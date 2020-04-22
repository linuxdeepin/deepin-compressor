#ifndef ANALYSEPSDTOOL_H
#define ANALYSEPSDTOOL_H

#include <QMap>

enum ExtractPsdStatus{
    NotChecked,     //需要先分析
    Reextract,      //可以解压
    NoneedAnalyse,  //不需要AnalyseTool分析的
    Checked,
    Completed,
    Canceled
};


#define VALIDLINE 0
#define EXTRACT_REPLACE_TIP "Would you like to replace the existing file"
#define ALLOK "All OK"

typedef struct lineInfo{
    lineInfo(QString l,bool b);
    QString line;
    bool read;
}LineInfo;

class AnalyseTool{
public:
    enum ENUMLINEINFO{
        RIGHTPSD,
        WRONGPSD,
        REPLACE
    };

    AnalyseTool();

    virtual ~AnalyseTool() {

    }

    virtual void analyseLine(const QString &line) = 0;
    // mark something for record
    virtual void mark(ENUMLINEINFO id,QString line,bool read) = 0;

    virtual LineInfo* getLineInfo(ENUMLINEINFO) = 0;

    virtual int isRightPsd() = 0;
};


class AnalyseToolRar4:public AnalyseTool{
public:
    explicit AnalyseToolRar4();

    ~AnalyseToolRar4()override;

    void mark(ENUMLINEINFO id,QString line,bool read)override;

    void analyseLine(const QString &line)override;

    LineInfo* getLineInfo(ENUMLINEINFO id)override;

    /**
     * @brief isRightPsd
     * @return 1:psd right;  2:psd wrong;  0:psd not checked;
     */
    int isRightPsd();

private:
    QMap<ENUMLINEINFO,LineInfo*>* pMapInfo;
    int lineCount = 0;
};


class AnalyseTool7Z:public AnalyseTool{
    explicit AnalyseTool7Z();

    ~AnalyseTool7Z()override;

    void mark(ENUMLINEINFO id,QString line,bool read)override;

    void analyseLine(const QString &line)override;

    /**
     * @brief isRightPsd
     * @return 1:psd right;  2:psd wrong;  0:psd not checked;
     */
    int isRightPsd();

private:
    QMap<ENUMLINEINFO,LineInfo*>* pMapInfo;
    int lineCount = 0;
};

// some info can help analyse lines
struct AnalyseInfo{
    QString destUserPath = "";
    QString destSubFolderName = "";
    QString tempPath = "";
    int step = 0;
    ExtractPsdStatus curStatus;
};

class AnalyseHelp{
public:
    explicit AnalyseHelp(ExtractPsdStatus status = NoneedAnalyse,QString destPath = "",QString subFolderName = "");

    ~AnalyseHelp();

    void analyseLine(const QString& line);

    // mark something for record
    void mark(AnalyseTool::ENUMLINEINFO id,QString line,bool read);

    bool hasReplace();

    bool ifNotBind();
    /**
     * @brief isRightPsd
     * @return 1:psd right;  2:psd wrong;  0:psd not checked;  3:default(不需要AnalyseHelp分析判断的）;
     */
    int isRightPsd();

    void clearPath(QString path);

    QString getDestionFolderPath();

    QString getPath();

    bool isNeedRemoveTemp();

private:
    void init();
    LineInfo* getLineInfo(AnalyseTool::ENUMLINEINFO id);
    void bind(const QString& line);
private:
    AnalyseTool* pTool = nullptr;
    AnalyseInfo* pInfo = nullptr;
};


#endif // ANALYSEPSDTOOL_H
