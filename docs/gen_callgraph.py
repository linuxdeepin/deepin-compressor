#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 生成 docs/architecture-callgraph.svg —— 单列时序：编号严格自上而下
import os

W, H = 1620, 1640
S = []
S.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}" '
         'font-family="Noto Sans CJK SC, Source Han Sans SC, sans-serif">')
S.append('''<defs>
<marker id="a" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="#2563eb"/></marker>
<marker id="ad" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="#dc2626"/></marker>
</defs>
<style>
 .t{font-size:15px;font-weight:600;fill:#0f172a}
 .s{font-size:12px;fill:#475569}
 .x{font-size:11.5px;fill:#64748b}
 .src{fill:#eff6ff;stroke:#2563eb;stroke-width:1.5}
 .api{fill:#f5f3ff;stroke:#7c3aed;stroke-width:1.5}
 .plg{fill:#ecfdf5;stroke:#0f766e;stroke-width:1.5}
 .e{stroke:#2563eb;stroke-width:1.6;fill:none}
 .ed{stroke:#dc2626;stroke-width:1.4;fill:none;stroke-dasharray:5 4}
 .num{font-size:13px;font-weight:700;fill:#fff}
 .layer{font-size:10.5px;font-weight:700;letter-spacing:.4px}
 .tag{font-size:11px;fill:#334155}
</style>
<rect x="0" y="0" width="1620" height="1560" fill="#ffffff"/>''')

STEPS = [
    ("main()", "MLogger → 解析 argv → 构造 DApplication", "src", 56),
    ("CompressorApplication（DApplication）", "翻译 / 图标 / 单实例 / DBus 适配器", "src", 56),
    ("MainWindow", "slotHandleArguments → loadArchive / extract / compress", "src", 56),
    ("ArchiveManager::loadArchive(path)", "单例；判断操作种类，准备参数", "src", 56),
    ("UiTools::createInterface(path, false)", "按 MIME + 业务指定的 eType（APT_Libzip / APT_Cli7z …）选后端", "src", 64),
    ("PluginManager::preferredPluginsFor(mime)", "按 X-KDE-Priority 排序，再用 Name 子串过滤（7zip / libzip / pzip …）", "interface", 64),
    ("KPluginLoader：扫目录 + 读元数据", "/usr/lib/deepin-compressor/plugins/*.so 内嵌的 kerfuffle_*.json", "interface", 64),
    ("KPluginFactory::create() → 插件实例（dlopen）", "构造参数 { 压缩包路径, 插件元数据, CustomMimeType }", "plugin", 64),
    ("返回 ReadOnlyArchiveInterface*", "从这一刻起 src 只认这个指针，不再关心背后是哪个实现", "interface", 56),
    ("new LoadJob(iface) → start()", "若 waitForFinished()==true 直接执行，否则起 SingleJobThread", "src", 56),
    ("iface->list()（虚函数分派）", "契约方法：list / testArchive / extractFiles / addFiles / deleteFiles …", "interface", 64),
    ("插件实现执行", "", "plugin", 140),
    ("数据与交互回传", "", "interface", 104),
    ("Job 收尾 → UI 刷新", "signalFinished 结束 Job；progress 刷新进度条；DataManager 供树模型读取", "src", 56),
]

LAYER = {"src": ("src 自有", "#2563eb"), "interface": ("3rdparty/interface", "#7c3aed"), "plugin": ("3rdparty/插件", "#0f766e")}
CLS = {"src": "src", "interface": "api", "plugin": "plg"}

X, BW = 290, 720
CX = X + BW / 2
TOP, GAP = 128, 26

S.append('<text x="34" y="38" class="t" font-size="19">deepin-compressor 调用时序（从 main 开始）</text>')
S.append('<text x="34" y="62" class="s">一个框 = 一步；编号严格自上而下读。左列标注该步属于哪一层。</text>')
S.append('<text x="34" y="82" class="s">蓝 = src 自有　紫 = 3rdparty/interface（契约层）　绿 = 插件 .so（运行时加载）</text>')

y = TOP
coords = []
for i, (title, sub, layer, h) in enumerate(STEPS, start=1):
    S.append(f'<rect x="{X}" y="{y}" width="{BW}" height="{h}" rx="9" class="{CLS[layer]}"/>')
    ty = y + (h / 2 + 5 if not sub else 24)
    if i in (12, 13):
        S.append(f'<text x="{X + 24}" y="{y + 24}" class="t">{title}</text>')
    else:
        S.append(f'<text x="{CX}" y="{ty}" text-anchor="middle" class="t">{title}</text>')
    if sub:
        S.append(f'<text x="{CX}" y="{ty + 20}" text-anchor="middle" class="s">{sub}</text>')

    if i == 12:
        rows = [("库调用", "libarchiveplugin → libarchive C API　·　libzipplugin → libzip C API　·　minizip"),
                ("命令行子进程", "CliInterface → QProcess / KPTY　·　7z　·　unrar　·　tar + pigz"),
                ("外部可执行", "clipzipplugin → /usr/lib/deepin-compressor/pzip（--ui-events 推进度）")]
        for k, (tag, txt) in enumerate(rows):
            yy = y + 52 + k * 27
            S.append(f'<text x="{X + 24}" y="{yy}" class="tag" font-weight="700">{tag}</text>')
            S.append(f'<text x="{X + 140}" y="{yy}" class="x">{txt}</text>')

    if i == 13:
        rows = [("DataManager", "list() 把条目写进全局单例；UI 从这里读（src 里 18 处）"),
                ("Query 对话框", "密码 / 覆盖 / 损坏包：emit signalQuery → 主线程弹框 → setResponse 唤醒工作线程"),
                ("信号", "signalprogress / signalCurFileName / signalFinished / signalCancel")]
        for k, (tag, txt) in enumerate(rows):
            yy = y + 50 + k * 24
            S.append(f'<text x="{X + 24}" y="{yy}" class="tag" font-weight="700">{tag}</text>')
            S.append(f'<text x="{X + 150}" y="{yy}" class="x">{txt}</text>')

    nb = y + h / 2
    S.append(f'<circle cx="70" cy="{nb - 6}" r="15" fill="#475569"/>')
    S.append(f'<text x="70" y="{nb - 1}" text-anchor="middle" class="num">{i}</text>')
    name, color = LAYER[layer]
    S.append(f'<rect x="96" y="{nb - 12}" width="4" height="24" rx="2" fill="{color}"/>')
    S.append(f'<text x="108" y="{nb + 4}" class="layer" fill="{color}">{name}</text>')

    coords.append((y, h))
    y += h + GAP

for i in range(len(coords) - 1):
    y0, h0 = coords[i]
    y1, _ = coords[i + 1]
    S.append(f'<path d="M {CX},{y0 + h0} L {CX},{y1}" class="e" marker-end="url(#a)"/>')
    if i in (2, 5):
        S.append(f'<text x="{CX + 10}" y="{y1 - 9}" class="x">调用</text>')

# 第 9 步：指针回传（相邻两步，不跨层）
y9, h9 = coords[8]
y10, _ = coords[9]
S.append(f'<path d="M {X + BW},{y9 + h9 / 2} L {X + BW + 54},{y9 + h9 / 2} L {X + BW + 54},{y10 + 30} L {X + BW},{y10 + 30}" '
         'class="ed" marker-end="url(#ad)"/>')
S.append(f'<text x="{X + BW + 60}" y="{y9 + h9 / 2 - 10}" class="x" fill="#dc2626">返回值</text>')

# 右侧附注（贴着对应步骤）
NOTES = {
    4: ["整个流程在此分岔：先拿后端，", "再交给 Job；两步顺序固定。"],
    5: ["eType 是业务硬编码策略：zip 注释", "固定 libzip；分卷 zip / tar.7z 固定 cli7z。"],
    7: ["扫目录只得到候选；能否启用还看", ".so 声明的外部命令在不在。"],
    11: ["同步或异步由后端自己决定，src 只是", "照 waitForFinished() 的返回值执行。"],
}
for idx, lines in NOTES.items():
    y0, h0 = coords[idx - 1]
    for k, line in enumerate(lines):
        S.append(f'<text x="{X + BW + 24}" y="{y0 + 26 + k * 17}" class="x">{line}</text>')

BY = y + 4
S.append(f'<rect x="34" y="{BY}" width="1552" height="92" rx="10" fill="#f8fafc" stroke="#e2e8f0"/>')
S.append(f'<text x="54" y="{BY + 24}" class="t" font-size="13.5">结构上的三个事实</text>')
facts = ["src 的可执行文件只链接 compressor-interface 一个库；libarchive / libzip / minizip 只出现在插件 .so 的链接行里。",
         "src 从不 include 插件头、不实例化插件类；第 9 步之后它手里只有一个 ReadOnlyArchiveInterface*。",
         "8 个后端都链接同一个 compressor-interface，接口类在进程内只有一份，靠虚函数分派到具体实现。"]
for k, f in enumerate(facts):
    S.append(f'<text x="54" y="{BY + 47 + k * 19}" class="s">· {f}</text>')

S.append('</svg>')
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "architecture-callgraph.svg")
open(out, "w", encoding="utf-8").write("\n".join(S))
print("written", out, "chars", len("\n".join(S)), "| content bottom", BY + 92)
