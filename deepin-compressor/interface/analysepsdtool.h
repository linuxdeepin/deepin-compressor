#ifndef ANALYSEPSDTOOL_H
#define ANALYSEPSDTOOL_H

#include <QMap>

enum ENUMLINEINFO{
    RIGHTPSD,
    WRONGPSD,
    REPLACE
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
    void clean(){

    }

private:
    QMap<ENUMLINEINFO,LineInfo*>* pMapInfo;
    int lineCount = 0;
};


class AnalyseHelp{
public:
    explicit AnalyseHelp(QString destPath,QString subFolderName);

    ~AnalyseHelp();

    void analyseLine(const QString& line);

    // mark something for record
    void mark(ENUMLINEINFO id,QString line,bool read);

    LineInfo* getLineInfo(ENUMLINEINFO id);

    void setDestDir(const QString& path);

    QString getDestDir();

    bool hasReplace();

    bool isNotKnown();
    /**
     * @brief isRightPsd
     * @return 1:psd right;  2:psd wrong;  0:psd not checked;
     */
    int isRightPsd();

    void clearPath(QString path);

    QString getDestionFolderPath();

    QString getTempPath();

    bool isNeedRemoveTemp();
private:
    AnalyseTool* pTool = nullptr;
    QString destPath = "";
    QString destSubFolderName = "";
    QString tempPath = "";
    int lineCount = 0;
};


#endif // ANALYSEPSDTOOL_H
