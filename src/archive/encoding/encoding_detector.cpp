// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encoding_detector.h"


#include "encoding_tables.h"


#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace encdet {

namespace {

// ---- 与旧实现相同的常量 ----
constexpr float kShortcut = 0.95f;            // SHORTCUT_THRESHOLD
constexpr float kMinimum = 0.20f;             // MINIMUM_THRESHOLD
constexpr float kSureYes = 0.99f;             // SURE_YES
constexpr float kSureNo = 0.01f;              // SURE_NO
constexpr unsigned kEnoughData = 1024;        // ENOUGH_DATA_THRESHOLD
constexpr unsigned kContextMinData = 4;       // MINIMUM_DATA_THRESHOLD
constexpr unsigned kContextMaxRel = 1000;     // MAX_REL_THRESHOLD
constexpr int kNumOfCategory = 6;             // NUM_OF_CATEGORY
constexpr float kOneCharProb = 0.50f;         // ONE_CHAR_PROB

enum { SmStart = 0, SmError = 1, SmItsMe = 2 };          // nsSMState
enum { PsDetecting = 0, PsFoundIt = 1, PsNotMe = 2 };    // nsProbingState

// ---- 状态机：表驱动，查表运算与旧实现一致 ----

struct Dfa
{
    const SMModel *model = nullptr;
    int state = SmStart;
    PRUint32 charLen = 1;

    void reset()
    {
        state = SmStart;
        charLen = 1;
    }

    int next(unsigned char c)
    {
        const PRUint32 cls = GETFROMPCK(static_cast<PRUint32>(c), model->classTable);
        if (state == SmStart)
            charLen = model->charLenTable[cls];
        state = static_cast<int>(
                    GETFROMPCK(static_cast<PRUint32>(state * model->classFactor + cls),
                               model->stateTable));
        return state;
    }
};

// ---- 频率分布打分 ----

using OrderFn = int (*)(const unsigned char *);

struct Distribution
{
    const PRInt16 *table = nullptr;
    unsigned size = 0;
    float ratio = 1.0f;
    OrderFn order = nullptr;
    unsigned total = 0;
    unsigned freq = 0;

    void reset()
    {
        total = 0;
        freq = 0;
    }

    void one(const char *str, PRUint32 len)
    {
        if (len != 2)
            return;
        const int idx = order(reinterpret_cast<const unsigned char *>(str));
        if (idx < 0)
            return;
        ++total;
        if (static_cast<unsigned>(idx) < size && table[idx] < 512)
            ++freq;
    }

    bool enough() const { return total > kEnoughData; }

    float confidence() const
    {
        if (total == 0)
            return kSureNo;
        if (total != freq) {
            const float r = static_cast<float>(freq)
                    / (static_cast<float>(total - freq) * ratio);
            if (r < kSureYes)
                return r;
        }
        return kSureYes;
    }
};

// ---- 日文上下文分析（SJIS / EUC-JP 专用） ----

using CtxOrderFn = int (*)(const char *, PRUint32 *);

struct Context
{
    int relSample[kNumOfCategory] = {0, 0, 0, 0, 0, 0};
    unsigned totalRel = 0;
    int lastOrder = -1;
    unsigned needToSkip = 0;
    bool done = false;

    void reset()
    {
        for (int &v : relSample)
            v = 0;
        totalRel = 0;
        lastOrder = -1;
        needToSkip = 0;
        done = false;
    }

    void data(const char *buf, PRUint32 len, CtxOrderFn orderFn)
    {
        if (done)
            return;
        for (unsigned i = needToSkip; i < len;) {
            PRUint32 charLen = 0;
            const int order = orderFn(buf + i, &charLen);
            i += charLen;
            if (i > len) {
                needToSkip = i - len;
                lastOrder = -1;
            } else {
                if (order != -1 && lastOrder != -1) {
                    ++totalRel;
                    if (totalRel > kContextMaxRel) {
                        done = true;
                        break;
                    }
                    relSample[own_jp2CharContext[lastOrder][order]]++;
                }
                lastOrder = order;
            }
        }
    }

    bool enough() const { return totalRel > kContextMinData; }

    float confidence() const
    {
        if (totalRel > kContextMinData)
            return static_cast<float>(totalRel - relSample[0]) / static_cast<float>(totalRel);
        return kSureNo;   // DONT_KNOW
    }
};

// ---- MBCS 组的单个探测器 ----

struct MbcsProber
{
    const char *name = nullptr;
    Dfa dfa;
    Distribution dist;
    Context ctx;
    CtxOrderFn ctxOrder = nullptr;
    bool useContext = false;
    bool isUtf8 = false;
    unsigned mbChars = 0;
    int state = PsDetecting;
    char lastChar[2] = {0, 0};

    void reset()
    {
        dfa.reset();
        dist.reset();
        ctx.reset();
        mbChars = 0;
        state = PsDetecting;
        lastChar[0] = 0;
        lastChar[1] = 0;
    }

    float utf8Confidence() const
    {
        if (mbChars >= 6)
            return kSureYes;
        float unlike = 0.99f;
        for (unsigned i = 0; i < mbChars; ++i)
            unlike *= kOneCharProb;
        return 1.0f - unlike;
    }

    float confidence() const
    {
        if (isUtf8)
            return utf8Confidence();

        const float distribCf = dist.confidence();
        if (!useContext)
            return distribCf;
        const float contxtCf = ctx.confidence();
        return contxtCf > distribCf ? contxtCf : distribCf;
    }

    int feed(const char *buf, PRUint32 len)
    {
        for (PRUint32 i = 0; i < len; ++i) {
            const int codingState = dfa.next(static_cast<unsigned char>(buf[i]));
            if (codingState == SmError) {
                state = PsNotMe;
                break;
            }
            if (codingState == SmItsMe) {
                state = PsFoundIt;
                if (getenv("OWNDET_DEBUG"))
                    fprintf(stderr, "[dbg] %s: SM eItsMe at byte %u\n", name, i);
                break;
            }
            if (codingState == SmStart) {
                const PRUint32 charLen = dfa.charLen;
                if (isUtf8) {
                    if (charLen >= 2)
                        ++mbChars;
                    continue;
                }
                if (i == 0) {
                    lastChar[1] = buf[0];
                    dist.one(lastChar, charLen);
                    if (useContext)
                        ctx.data(lastChar, charLen, ctxOrder);
                } else {
                    dist.one(buf + i - 1, charLen);
                    if (useContext)
                        ctx.data(buf + i - 1, charLen, ctxOrder);
                }
            }
        }

        if (!isUtf8 && len > 0)
            lastChar[0] = buf[len - 1];

        if (state == PsDetecting) {
            if (isUtf8) {
                if (utf8Confidence() > kShortcut) {
                    state = PsFoundIt;
                    if (getenv("OWNDET_DEBUG"))
                        fprintf(stderr, "[dbg] UTF-8: shortcut\n");
                }
            } else {
                const bool enoughData = useContext ? (dist.enough() && ctx.enough()) : dist.enough();
                if (enoughData && confidence() > kShortcut) {
                    state = PsFoundIt;
                    if (getenv("OWNDET_DEBUG"))
                        fprintf(stderr, "[dbg] %s: shortcut conf=%.3f\n", name, confidence());
                }
            }
        }
        return state;
    }
};

// ---- 各编码的 GetOrder（与旧实现逐字一致） ----

int gbOrder(const unsigned char *c)
{
    if (c[0] >= 0xB0 && c[1] >= 0xA1)
        return 94 * (c[0] - 0xB0) + c[1] - 0xA1;
    return -1;
}

int big5Order(const unsigned char *c)
{
    if (c[0] >= 0xA4) {
        if (c[1] >= 0xA1)
            return 157 * (c[0] - 0xA4) + c[1] - 0xA1 + 63;
        return 157 * (c[0] - 0xA4) + c[1] - 0x40;
    }
    return -1;
}

int sjisOrder(const unsigned char *c)
{
    if (c[0] >= 0x81 && c[0] <= 0x9F)
        return 188 * (c[0] - 0x81) + (c[1] > 0x7F ? c[1] - 0x41 : c[1] - 0x40);
    if (c[0] >= 0xE0 && c[0] <= 0xEF)
        return 188 * (c[0] - 0xE0 + 31) + (c[1] > 0x7F ? c[1] - 0x41 : c[1] - 0x40);
    return -1;
}

int eucJpOrder(const unsigned char *c)
{
    if (c[0] >= 0xA0)
        return 94 * (c[0] - 0xA1) + c[1] - 0xA1;
    return -1;
}

int eucKrOrder(const unsigned char *c)
{
    if (c[0] >= 0xB0)
        return 94 * (c[0] - 0xB0) + c[1] - 0xA1;
    return -1;
}

int eucTwOrder(const unsigned char *c)
{
    if (c[0] >= 0xC4)
        return 94 * (c[0] - 0xC4) + c[1] - 0xA1;
    return -1;
}

// ---- 上下文分析的 GetOrder（SJIS / EUC-JP） ----

int sjisCtxOrder(const char *str, PRUint32 *charLen)
{
    const unsigned char *s = reinterpret_cast<const unsigned char *>(str);
    if ((s[0] >= 0x81 && s[0] <= 0x9F) || (s[0] >= 0xE0 && s[0] <= 0xFC))
        *charLen = 2;
    else
        *charLen = 1;

    // 只对平假名行（0x82 0x9F..0xF1）给序号，其余返回 -1
    if (s[0] == 0x82 && s[1] >= 0x9F && s[1] <= 0xF1)
        return s[1] - 0x9F;
    return -1;
}

int eucJpCtxOrder(const char *str, PRUint32 *charLen)
{
    const unsigned char *s = reinterpret_cast<const unsigned char *>(str);
    if (s[0] == 0x8E || (s[0] >= 0xA1 && s[0] <= 0xFE))
        *charLen = 2;
    else if (s[0] == 0x8F)
        *charLen = 3;
    else
        *charLen = 1;

    if (s[0] == 0xA4 && s[1] >= 0xA1 && s[1] <= 0xF3)
        return s[1] - 0xA1;
    return -1;
}

// ---- MBCS 组 ----

struct MbcsGroup
{
    MbcsProber probers[7];
    bool active[7] = {true, true, true, true, true, true, true};
    unsigned activeCount = 7;
    int bestGuess = 0;
    int state = PsDetecting;
    unsigned mKeepNext = 0;

    void init()
    {
        struct Def
        {
            const SMModel *sm;
            const char *name;
            const PRInt16 *table;
            unsigned size;
            float ratio;
            OrderFn order;
            CtxOrderFn ctxOrder;
            bool isUtf8;
        };
        // 顺序与旧实现一致：UTF8, SJIS, EUCJP, GB18030, EUCKR, Big5, EUCTW
        const Def defs[7] = {
            {&own_UTF8SMModel, "UTF-8", nullptr, 0, 1.0f, nullptr, nullptr, true},
            {&own_SJISSMModel, "Shift_JIS", JISCharToFreqOrder, static_cast<unsigned>(JIS_TABLE_SIZE),
             static_cast<float>(JIS_TYPICAL_DISTRIBUTION_RATIO), sjisOrder, sjisCtxOrder, false},
            {&own_EUCJPSMModel, "EUC-JP", JISCharToFreqOrder, static_cast<unsigned>(JIS_TABLE_SIZE),
             static_cast<float>(JIS_TYPICAL_DISTRIBUTION_RATIO), eucJpOrder, eucJpCtxOrder, false},
            {&own_GB18030SMModel, "gb18030", GB2312CharToFreqOrder, static_cast<unsigned>(GB2312_TABLE_SIZE),
             static_cast<float>(GB2312_TYPICAL_DISTRIBUTION_RATIO), gbOrder, nullptr, false},
            {&own_EUCKRSMModel, "EUC-KR", EUCKRCharToFreqOrder, static_cast<unsigned>(EUCKR_TABLE_SIZE),
             static_cast<float>(EUCKR_TYPICAL_DISTRIBUTION_RATIO), eucKrOrder, nullptr, false},
            {&own_Big5SMModel, "Big5", Big5CharToFreqOrder, static_cast<unsigned>(BIG5_TABLE_SIZE),
             static_cast<float>(BIG5_TYPICAL_DISTRIBUTION_RATIO), big5Order, nullptr, false},
            {&own_EUCTWSMModel, "x-euc-tw", EUCTWCharToFreqOrder, static_cast<unsigned>(EUCTW_TABLE_SIZE),
             static_cast<float>(EUCTW_TYPICAL_DISTRIBUTION_RATIO), eucTwOrder, nullptr, false},
        };

        for (int i = 0; i < 7; ++i) {
            MbcsProber &p = probers[i];
            p.name = defs[i].name;
            p.dfa.model = defs[i].sm;
            p.isUtf8 = defs[i].isUtf8;
            if (defs[i].table) {
                p.dist.table = defs[i].table;
                p.dist.size = defs[i].size;
                p.dist.ratio = defs[i].ratio;
                p.dist.order = defs[i].order;
            }
            p.ctxOrder = defs[i].ctxOrder;
            p.useContext = (defs[i].ctxOrder != nullptr);
            p.reset();
        }
    }

    void reset()
    {
        for (int i = 0; i < 7; ++i) {
            probers[i].reset();
            active[i] = true;
        }
        activeCount = 7;
        bestGuess = 0;
        state = PsDetecting;
        mKeepNext = 0;
    }

    int feed(const char *buf, PRUint32 len)
    {
        // 与旧实现一致：只把"含高位字节"的片段喂给各探测器，纯 ASCII 段跳过；
        // 片段边界跨调用保持（mKeepNext）
        unsigned start = 0;
        unsigned keepNext = mKeepNext;

        for (unsigned pos = 0; pos < len; ++pos) {
            if (buf[pos] & 0x80) {
                if (!keepNext)
                    start = pos;
                keepNext = 2;
            } else if (keepNext) {
                if (--keepNext == 0) {
                    const int st = feedSegment(buf + start, pos + 1 - start);
                    if (st != PsDetecting)
                        return st;
                }
            }
        }

        if (keepNext) {
            // 末尾仍是高位字节时，片段长度不能越过输入缓冲区。
            const int st = feedSegment(buf + start, len - start);
            if (st != PsDetecting) {
                mKeepNext = keepNext;
                return st;
            }
        }
        mKeepNext = keepNext;
        return state;
    }

    // 把一段字节交给所有仍然有效的探测器；命中或全灭时直接返回
    int feedSegment(const char *buf, unsigned len)
    {
        for (int i = 0; i < 7; ++i) {
            if (!active[i])
                continue;
            const int st = probers[i].feed(buf, len);
            if (st == PsFoundIt) {
                bestGuess = i;
                state = PsFoundIt;
                return state;
            }
            if (st == PsNotMe) {
                active[i] = false;
                --activeCount;
                if (activeCount == 0) {
                    state = PsNotMe;
                    return state;
                }
            }
        }
        return state;
    }

    float confidence()
    {
        if (state == PsFoundIt)
            return kSureYes;
        if (state == PsNotMe)
            return kSureNo;

        float best = 0.0f;
        for (int i = 0; i < 7; ++i) {
            if (!active[i])
                continue;
            const float cf = probers[i].confidence();
            if (best < cf) {
                best = cf;
                bestGuess = i;
            }
        }
        return best;
    }

    const char *charsetName() const { return probers[bestGuess].name; }
};



// ---- 过滤：保留英文的版本（Latin1 prober 用） ----
std::vector<char> filterWithEnglishLetters(const char *buf, unsigned len)
{
    std::vector<char> out;
    out.reserve(len);
    bool isInTag = false;
    const char *prev = buf;
    const char *cur = buf;
    for (; cur < buf + len; ++cur) {
        if (*cur == '>')
            isInTag = false;
        else if (*cur == '<')
            isInTag = true;

        if (!(*cur & 0x80) && (*cur < 'A' || (*cur > 'Z' && *cur < 'a') || *cur > 'z')) {
            if (cur > prev && !isInTag) {
                while (prev < cur)
                    out.push_back(*prev++);
                ++prev;
                out.push_back(' ');
            } else {
                prev = cur + 1;
            }
        }
    }
    if (!isInTag) {
        while (prev < cur)
            out.push_back(*prev++);
    }
    return out;
}

// ---- Latin1 prober ----
struct Latin1Prober
{
    int state = PsDetecting;
    char lastCharClass = 0;
    unsigned freqCounter[4] = {0, 0, 0, 0};

    void reset()
    {
        state = PsDetecting;
        lastCharClass = 1;   // 旧实现初始化为 OTH(=1)，不是 0
        for (unsigned &v : freqCounter)
            v = 0;
    }

    void feed(const char *buf, unsigned len)
    {
        // 旧实现：过滤成功就只用过滤后的缓冲区（即使为空），失败才回退原缓冲
        const std::vector<char> filtered = filterWithEnglishLetters(buf, len);
        const char *data = filtered.data();
        const unsigned n = static_cast<unsigned>(filtered.size());

        for (unsigned i = 0; i < n; ++i) {
            const unsigned char charClass = own_Latin1_CharToClass[static_cast<unsigned char>(data[i])];
            const unsigned char freq = own_Latin1ClassModel[lastCharClass * 8 + charClass];
            if (freq == 0) {
                state = PsNotMe;
                break;
            }
            freqCounter[freq]++;
            lastCharClass = static_cast<char>(charClass);
        }
    }

    float confidence() const
    {
        if (state == PsNotMe)
            return 0.01f;

        unsigned total = 0;
        for (int i = 0; i < 4; ++i)
            total += freqCounter[i];

        float confidence = 0.0f;
        if (total) {
            confidence = freqCounter[3] * 1.0f / static_cast<float>(total);
            confidence -= freqCounter[1] * 20.0f / static_cast<float>(total);
        }
        if (confidence < 0.0f)
            confidence = 0.0f;
        return confidence * 0.50f;   // 旧实现刻意压低，让更精确的探测器优先
    }

    const char *charsetName() const { return "windows-1252"; }
};

// ---- ISO-2022 系 Esc prober ----
struct EscProber
{
    Dfa sms[4];
    const SMModel *models[4] = {nullptr, nullptr, nullptr, nullptr};
    unsigned activeSm = 4;
    int state = PsDetecting;
    const char *detected = nullptr;

    void init()
    {
        models[0] = &own_HZSMModel;
        models[1] = &own_ISO2022CNSMModel;
        models[2] = &own_ISO2022JPSMModel;
        models[3] = &own_ISO2022KRSMModel;
        reset();
    }

    void reset()
    {
        for (int i = 0; i < 4; ++i) {
            sms[i].model = models[i];
            sms[i].reset();
        }
        activeSm = 4;
        state = PsDetecting;
        detected = nullptr;
    }

    void feed(const char *buf, unsigned len)
    {
        for (unsigned i = 0; i < len && state == PsDetecting; ++i) {
            for (int j = static_cast<int>(activeSm) - 1; j >= 0; --j) {
                const int codingState = sms[j].next(static_cast<unsigned char>(buf[i]));
                if (codingState == SmError) {
                    --activeSm;
                    if (activeSm == 0) {
                        state = PsNotMe;
                        return;
                    }
                    if (j != static_cast<int>(activeSm)) {
                        Dfa tmp = sms[activeSm];
                        sms[activeSm] = sms[j];
                        sms[j] = tmp;
                        const SMModel *mt = models[activeSm];
                        models[activeSm] = models[j];
                        models[j] = mt;
                    }
                } else if (codingState == SmItsMe) {
                    state = PsFoundIt;
                    detected = models[j] != nullptr ? models[j]->name : nullptr;
                    return;
                }
            }
        }
    }

    float confidence() const { return 0.99f; }
    const char *charsetName() const { return detected; }
};

// ================= SBCS 组（单字节语言模型） =================

constexpr unsigned kSampleSize = 64;
constexpr unsigned kSbEnoughRelThreshold = 1024;
constexpr float kPositiveShortcut = 0.95f;
constexpr float kNegativeShortcut = 0.05f;
constexpr unsigned kSymbolCatOrder = 250;
constexpr unsigned kNumOfSeqCat = 4;
constexpr unsigned kPositiveCat = kNumOfSeqCat - 1;
constexpr unsigned kNegativeCat = 0;

// 过滤：只保留含高位字节的片段，英文/符号段落压成单个空格
std::vector<char> filterWithoutEnglishLetters(const char *buf, unsigned len)
{
    std::vector<char> out;
    out.reserve(len);
    bool meetMsb = false;
    const char *prev = buf;
    const char *cur = buf;
    for (; cur < buf + len; ++cur) {
        if (*cur & 0x80) {
            meetMsb = true;
        } else if (*cur < 'A' || (*cur > 'Z' && *cur < 'a') || *cur > 'z') {
            if (meetMsb && cur > prev) {
                while (prev < cur)
                    out.push_back(*prev++);
                ++prev;
                out.push_back(' ');
                meetMsb = false;
            } else {
                prev = cur + 1;
            }
        }
    }
    if (meetMsb && cur > prev) {
        while (prev < cur)
            out.push_back(*prev++);
    }
    return out;
}

struct HebrewProber
{
    int finalLogical = 0;
    int finalVisual = 0;
    char prev = ' ';
    char beforePrev = ' ';
    // 逻辑/视觉模型的状态与置信度由组在调用时传入，避免与 SbProber 互相依赖

    static bool isFinal(unsigned char c)
    {
        return c == 0xEA || c == 0xED || c == 0xEF || c == 0xF3 || c == 0xF5;
    }

    static bool isNonFinal(unsigned char c)
    {
        return c == 0xEB || c == 0xEE || c == 0xF0 || c == 0xF4;
    }

    void reset()
    {
        finalLogical = 0;
        finalVisual = 0;
        prev = ' ';
        beforePrev = ' ';
    }

    int state(int logicalState, int visualState) const
    {
        if (logicalState == PsNotMe && visualState == PsNotMe)
            return PsNotMe;
        return PsDetecting;
    }

    void feed(const char *buf, unsigned len, int logicalState, int visualState)
    {
        if (state(logicalState, visualState) == PsNotMe)
            return;
        const char *cur = buf;
        const char *end = buf + len;
        for (; cur < end; ++cur) {
            const char c = *cur;
            if (c == ' ') {
                if (beforePrev != ' ') {
                    if (isFinal(static_cast<unsigned char>(prev)))
                        ++finalLogical;
                    else if (isNonFinal(static_cast<unsigned char>(prev)))
                        ++finalVisual;
                }
            } else {
                if (beforePrev == ' ' && isFinal(static_cast<unsigned char>(prev)) && c != ' ')
                    ++finalVisual;
            }
            beforePrev = prev;
            prev = c;
        }
    }

    const char *charsetName(float logicalCf, float visualCf) const
    {
        const int finalSub = finalLogical - finalVisual;
        if (finalSub >= 5)
            return "windows-1255";
        if (finalSub <= -5)
            return "ISO-8859-8";

        const float modelSub = logicalCf - visualCf;
        if (modelSub > 0.01f)
            return "windows-1255";
        if (modelSub < -0.01f)
            return "ISO-8859-8";

        if (finalSub < 0)
            return "ISO-8859-8";
        return "windows-1255";
    }
};

struct SbProber
{
    const SequenceModel *model = nullptr;
    bool reversed = false;
    HebrewProber *nameProber = nullptr;
    int state = PsDetecting;
    unsigned char lastOrder = 255;
    unsigned totalSeqs = 0;
    unsigned seqCounters[kNumOfSeqCat] = {0, 0, 0, 0};
    unsigned totalChar = 0;
    unsigned freqChar = 0;

    void reset()
    {
        state = PsDetecting;
        lastOrder = 255;
        for (unsigned &v : seqCounters)
            v = 0;
        totalSeqs = 0;
        totalChar = 0;
        freqChar = 0;
    }

    void feed(const char *buf, unsigned len)
    {
        for (unsigned i = 0; i < len; ++i) {
            const unsigned char order = model->charToOrderMap[static_cast<unsigned char>(buf[i])];

            if (order < kSymbolCatOrder)
                ++totalChar;
            if (order < kSampleSize) {
                ++freqChar;
                if (lastOrder < kSampleSize) {
                    ++totalSeqs;
                    if (!reversed)
                        ++seqCounters[static_cast<int>(model->precedenceMatrix[lastOrder * kSampleSize + order])];
                    else
                        ++seqCounters[static_cast<int>(model->precedenceMatrix[order * kSampleSize + lastOrder])];
                }
            }
            lastOrder = order;
        }

        if (state == PsDetecting && totalSeqs > kSbEnoughRelThreshold) {
            const float cf = confidence();
            if (cf > kPositiveShortcut)
                state = PsFoundIt;
            else if (cf < kNegativeShortcut)
                state = PsNotMe;
        }
    }

    float confidence() const
    {
        if (totalSeqs > 0) {
            float r = static_cast<float>(seqCounters[kPositiveCat])
                    / static_cast<float>(totalSeqs)
                    / model->mTypicalPositiveRatio;
            r = r * static_cast<float>(freqChar) / static_cast<float>(totalChar);
            return r >= 1.0f ? 0.99f : r;
        }
        return 0.01f;
    }

    const char *charsetName(const SbProber *logical, const SbProber *visual) const
    {
        if (!nameProber)
            return model->charsetName;
        const float logicalCf = logical ? logical->confidence() : 0.0f;
        const float visualCf = visual ? visual->confidence() : 0.0f;
        return nameProber->charsetName(logicalCf, visualCf);
    }
};

struct SbcsGroup
{
    SbProber probers[16];
    HebrewProber hebrew;
    bool active[16] = {true, true, true, true, true, true, true, true,
                       true, true, true, true, true, true, true, true};
    unsigned activeCount = 16;
    int bestGuess = 0;
    int state = PsDetecting;

    void init()
    {
        const SequenceModel *models[13] = {
            &own_Win1251Model, &own_Koi8rModel, &own_Latin5Model, &own_MacCyrillicModel,
            &own_Ibm866Model, &own_Ibm855Model, &own_Latin7Model, &own_Win1253Model,
            &own_Latin5BulgarianModel, &own_Win1251BulgarianModel,
            &own_Latin2HungarianModel, &own_Win1250HungarianModel, &own_TIS620ThaiModel,
        };
        for (int i = 0; i < 13; ++i) {
            probers[i].model = models[i];
            probers[i].reversed = false;
            probers[i].nameProber = nullptr;
        }
        // 13 = 希伯来判定器，14/15 = 逻辑/视觉希伯来模型
        probers[13].model = nullptr;
        probers[13].nameProber = nullptr;
        probers[14].model = &own_Win1255Model;
        probers[14].reversed = false;
        probers[14].nameProber = &hebrew;
        probers[15].model = &own_Win1255Model;
        probers[15].reversed = true;
        probers[15].nameProber = &hebrew;
        reset();
    }

    void reset()
    {
        for (int i = 0; i < 16; ++i) {
            probers[i].reset();
            active[i] = true;   // 13 号是希伯来判定器：参与判定，但置信度恒为 0
        }
        activeCount = 16;
        hebrew.reset();
        bestGuess = 0;
        state = PsDetecting;
    }

    int feed(const char *buf, unsigned len)
    {
        const std::vector<char> filtered = filterWithoutEnglishLetters(buf, len);
        if (filtered.empty())
            return state;

        for (int i = 0; i < 16; ++i) {
            if (!active[i])
                continue;
            int st = PsDetecting;
            if (i == 13) {
                // 希伯来判定器：只统计词尾字母，本身不报 charset
                hebrew.feed(filtered.data(), static_cast<unsigned>(filtered.size()),
                            probers[14].state, probers[15].state);
                st = hebrew.state(probers[14].state, probers[15].state);
            } else {
                probers[i].feed(filtered.data(), static_cast<unsigned>(filtered.size()));
                st = probers[i].state;
            }

            if (st == PsFoundIt) {
                bestGuess = i;
                state = PsFoundIt;
                return state;
            }
            if (st == PsNotMe) {
                active[i] = false;
                --activeCount;
                if (activeCount == 0) {
                    state = PsNotMe;
                    return state;
                }
            }
        }
        return state;
    }

    float confidence()
    {
        if (state == PsFoundIt)
            return 0.99f;
        if (state == PsNotMe)
            return 0.01f;

        float best = 0.0f;
        for (int i = 0; i < 16; ++i) {
            if (!active[i] || i == 13)
                continue;
            const float cf = probers[i].confidence();
            if (best < cf) {
                best = cf;
                bestGuess = i;
            }
        }
        return best;
    }

    const char *charsetName() const
    {
        return probers[bestGuess].charsetName(&probers[14], &probers[15]);
    }
};

} // namespace

Result detect(const char *data, size_t size)
{
    // 旧实现的 C 接口按 NUL 结尾处理，这里保持同样语义
    const size_t len = strnlen(data, size);
    if (len == 0) {
        // 旧实现在空输入上也会落到"纯 ASCII"分支（mInputState 默认值），返回 ASCII 1.0
        return {"ASCII", 1.0f};
    }

    const unsigned char *s = reinterpret_cast<const unsigned char *>(data);

    // BOM 优先（与旧实现一致，要求长度 > 3）
    if (len > 3) {
        if (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
            return {"UTF-8", 1.0f};
        if (s[0] == 0xFE && s[1] == 0xFF) {
            if (s[2] == 0x00 && s[3] == 0x00)
                return {"X-ISO-10646-UCS-4-3412", 1.0f};
            return {"UTF-16BE", 1.0f};
        }
        if (s[0] == 0xFF && s[1] == 0xFE) {
            if (s[2] == 0x00 && s[3] == 0x00)
                return {"UTF-32LE", 1.0f};
            return {"UTF-16LE", 1.0f};
        }
        if (s[0] == 0x00) {
            if (s[1] == 0x00 && s[2] == 0xFE && s[3] == 0xFF)
                return {"UTF-32BE", 1.0f};
            if (s[1] == 0x00 && s[2] == 0xFF && s[3] == 0xFE)
                return {"X-ISO-10646-UCS-4-2143", 1.0f};
        }
    }

    const char *buf = reinterpret_cast<const char *>(s);

    // 输入状态机：纯 ASCII -> 遇到 ESC 或 "~{" 转 Esc；遇到高位字节转 Highbyte
    enum { StatePureAscii = 0, StateEscAscii = 1, StateHighByte = 2 };
    int inputState = StatePureAscii;
    char lastChar = 0;

    MbcsGroup mbcs;
    SbcsGroup sbcs;
    Latin1Prober latin1;
    EscProber esc;
    bool groupsReady = false;
    bool escReady = false;

    for (size_t i = 0; i < len; ++i) {
        if ((s[i] & 0x80) && s[i] != 0xA0) {   // 0xA0（NBSP）按 ASCII 处理
            if (inputState != StateHighByte) {
                inputState = StateHighByte;
                // 高位字节出现后，Esc prober 直接作废
                escReady = false;
                if (!groupsReady) {
                    mbcs.init();
                    mbcs.reset();
                    sbcs.init();
                    sbcs.reset();
                    latin1.reset();
                    groupsReady = true;
                }
            }
        } else if (inputState == StatePureAscii
                   && (s[i] == '\033' || (s[i] == '{' && lastChar == '~'))) {
            inputState = StateEscAscii;
        }
        lastChar = static_cast<char>(s[i]);
    }

    switch (inputState) {
    case StateEscAscii: {
        if (!escReady) {
            esc.init();
            escReady = true;
        }
        esc.feed(buf, static_cast<unsigned>(len));
        if (esc.state == PsFoundIt && esc.charsetName() != nullptr)
            return {std::string(esc.charsetName()), 0.99f};
        // 与旧实现一致：Esc 状态下不做"三组取最高"那一步，判不出就是判不出
        return {};
    }
    case StateHighByte: {
        // 与旧实现一致：按 [MBCS, SBCS, Latin1] 顺序喂，任一 eFoundIt 立即定案
        if (mbcs.feed(buf, static_cast<unsigned>(len)) == PsFoundIt)
            return {std::string(mbcs.charsetName()), kSureYes};
        if (sbcs.feed(buf, static_cast<unsigned>(len)) == PsFoundIt)
            return {std::string(sbcs.charsetName()), kSureYes};
        latin1.feed(buf, static_cast<unsigned>(len));
        if (latin1.state == PsFoundIt)
            return {std::string(latin1.charsetName()), 0.99f};
        break;
    }
    default:
        return {"ASCII", 1.0f};
    }

    // DataEnd：三个组取最高置信度，超过 MINIMUM_THRESHOLD 才报
    float best = mbcs.confidence();
    std::string bestName = mbcs.charsetName();

    const float sbcsCf = sbcs.confidence();
    if (sbcsCf > best) {
        best = sbcsCf;
        bestName = sbcs.charsetName();
    }
    const float latinCf = latin1.confidence();
    if (latinCf > best) {
        best = latinCf;
        bestName = latin1.charsetName();
    }

    if (best > kMinimum)
        return {bestName, best};
    return {};
}

} // namespace encdet
