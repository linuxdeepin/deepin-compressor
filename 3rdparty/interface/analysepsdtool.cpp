#include "analysepsdtool.h"

#include <DStandardPaths>

#include <QDir>
#include <QProcess>
#include <QDebug>

#include <stdio.h>

DCORE_USE_NAMESPACE

lineInfo::lineInfo(QString l, bool b)
    : line(l)
    , read(b)
{
}

AnalyseTool::AnalyseTool()
{
}

//AnalyseToolRar4 begin
AnalyseToolRar4::~AnalyseToolRar4()
{
    QMap<ENUMLINEINFO, LineInfo *>::iterator it = pMapInfo->begin();
    while (it != pMapInfo->end()) {
        delete it.value();
        it.value() = nullptr;
        it++;
    }

    pMapInfo->clear();
    lineCount = 0;
}

AnalyseToolRar4::AnalyseToolRar4()
{
    pMapInfo = new QMap<ENUMLINEINFO, LineInfo *>();
    pMapInfo->insert(WRONGPSD, new LineInfo("", false));
    pMapInfo->insert(RIGHTPSD, new LineInfo("", false));
    lineCount = 0;
}

void AnalyseToolRar4::mark(ENUMLINEINFO id, QString line, bool read)
{
    if (pMapInfo->contains(id) == true) {
        (*pMapInfo)[id]->line = line;
        (*pMapInfo)[id]->read = read;
    }
}

void AnalyseToolRar4::analyseLine(const QString &line)
{
    int pos = line.indexOf(QLatin1Char('%'));
    if (pos > 1) {
        int percentage = line.midRef(pos - 3, 3).toInt();
        if (percentage >= 0) {
            lineCount++;
            if ((line.startsWith("Extracting") == true && line.endsWith("OK ")) || line.startsWith("...")) { // 解压分卷文件命令行输出以"..."开头
                (*pMapInfo)[RIGHTPSD]->line = line;
                (*pMapInfo)[RIGHTPSD]->read = true;
                return;
            } else if (line.contains(ALLOK) == true) {
                (*pMapInfo)[RIGHTPSD]->line = line;
                (*pMapInfo)[RIGHTPSD]->read = true;
            }
        }
    }
}

LineInfo *AnalyseToolRar4::getLineInfo(ENUMLINEINFO id)
{
    return (*pMapInfo)[id];
}

int AnalyseToolRar4::isRightPsd()
{
    if (pMapInfo->contains(RIGHTPSD) == true) {
        if ((*pMapInfo)[RIGHTPSD]->read == true) {
            return 1;
        } else if ((*pMapInfo)[WRONGPSD]->read == true) {
            return 2;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
//AnalyseToolRar4 end

//AnalyseTool7Z begin
AnalyseTool7Z::~AnalyseTool7Z()
{
    QMap<ENUMLINEINFO, LineInfo *>::iterator it = pMapInfo->begin();
    while (it != pMapInfo->end()) {
        delete it.value();
        it.value() = nullptr;
        it++;
    }

    pMapInfo->clear();
    lineCount = 0;
}

AnalyseTool7Z::AnalyseTool7Z()
{
    pMapInfo = new QMap<ENUMLINEINFO, LineInfo *>();
    pMapInfo->insert(WRONGPSD, new LineInfo("", false));
    pMapInfo->insert(RIGHTPSD, new LineInfo("", false));
    lineCount = 0;
}

void AnalyseTool7Z::mark(ENUMLINEINFO id, QString line, bool read)
{
    if (pMapInfo->contains(id) == true) {
        (*pMapInfo)[id]->line = line;
        (*pMapInfo)[id]->read = read;
    }
}

void AnalyseTool7Z::analyseLine(const QString &line)
{
    qDebug() << line << line.length();
    lineCount++;
    if (line.contains(DoubleBBBB)) {
        if (line.contains(WRONGPSD7Z)) {
            (*pMapInfo)[WRONGPSD]->line = line;
            (*pMapInfo)[WRONGPSD]->read = true;
        } else if (line.right(16) == EVERYOK) { //right psd
            (*pMapInfo)[RIGHTPSD]->line = line;
            (*pMapInfo)[RIGHTPSD]->read = true;
            return;
        } else if (line.length() > 24) { //right psd
            int pos = line.indexOf(QLatin1Char('%'));
            if (pos > 1) {
                int percentage = line.midRef(pos - 3, 3).toInt();
                if (percentage <= 100) {
                    (*pMapInfo)[RIGHTPSD]->line = line;
                    (*pMapInfo)[RIGHTPSD]->read = true;
                }
            }
        }
    } else if (line.left(16) == EVERYOK) { //right psd
        (*pMapInfo)[RIGHTPSD]->line = line;
        (*pMapInfo)[RIGHTPSD]->read = true;
        return;
    }
}

LineInfo *AnalyseTool7Z::getLineInfo(ENUMLINEINFO id)
{
    return (*pMapInfo)[id];
}

int AnalyseTool7Z::isRightPsd()
{
    if (pMapInfo->contains(RIGHTPSD) == true) {
        if ((*pMapInfo)[RIGHTPSD]->read == true) {
            return 1;
        } else if ((*pMapInfo)[WRONGPSD]->read == true) {
            return 2;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
//AnalyseTool7Z end

bool isDirExist(QString fullPath)
{
    QDir dir(fullPath);
    if (dir.exists()) {
        return true;
    } else {
        bool ok = dir.mkpath(fullPath); //创建多级目录
        return ok;
    }
}

void AnalyseHelp::resetTempDir()
{
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    tempPath = confDir + QDir::separator() + "tempExtractAAA";
    this->clearPath(tempPath);
    QDir Dir(tempPath);
    if (Dir.isEmpty()) {
        printf("temp dir %s is empty\n", tempPath.toUtf8().data());
        isDirExist(tempPath);
    }
}

AnalyseHelp::AnalyseHelp(QString destinationPath, QString subFolderName)
{
    lineCount = 0;
    destPath = destinationPath;
    destSubFolderName = subFolderName;
    this->resetTempDir();
    this->replaceTip = false;
}

void AnalyseHelp::clearPath(QString path)
{
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-fr");
    args.append(path);
    p.execute(command, args);
    p.waitForFinished();
}

QString AnalyseHelp::getDestionFolderPath()
{
    if (destSubFolderName == "") {
        return "";
    }

    if (destPath == "") {
        return "";
    }

    return destPath + "/" + destSubFolderName;
}

QString AnalyseHelp::getTempPath()
{
    return this->tempPath;
}

AnalyseHelp::~AnalyseHelp()
{
    this->clearPath(tempPath);
    delete pTool;
    pTool = nullptr;
    lineCount = 0;
    this->replaceTip = false;
}

void AnalyseHelp::analyseLine(const QString &line)
{
    if (pTool == nullptr) {
        if (line.left(5) == "UNRAR") {
            pTool = new AnalyseToolRar4();
        } else if (line.left(5) == "7-Zip") {
            pTool = new AnalyseTool7Z();
        }
    } else {
        //        int pos = line.indexOf(QLatin1Char('%'));
        //        if (pos > 1) {
        //            int percentage = line.midRef(pos - 3, 3).toInt();
        //            if(percentage>=0){
        pTool->analyseLine(line);
        //            }
        //        }
    }

    this->lineCount++;
}

void AnalyseHelp::mark(ENUMLINEINFO id, QString line, bool read)
{
    if (pTool != nullptr) {
        pTool->mark(id, line, read);
    }
}

LineInfo *AnalyseHelp::getLineInfo(ENUMLINEINFO id)
{
    if (pTool != nullptr) {
        return pTool->getLineInfo(id);
    } else {
        return nullptr;
    }
}

void AnalyseHelp::setDestDir(const QString &path)
{
    this->destPath = path;
}

QString AnalyseHelp::getDestDir()
{
    return this->destPath;
}

bool AnalyseHelp::isNotKnown()
{
    if (lineCount >= 1) {
        if (pTool == nullptr) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void AnalyseHelp::checkReplaceTip(const QString &line)
{
    if (line.contains(EXTRACT_REPLACE_TIP) == true) {
        this->replaceTip = true;
    }
}

int AnalyseHelp::isRightPsd()
{
    if (this->pTool != nullptr) {
        return this->pTool->isRightPsd();
    } else {
        return 0; //not known archive ,so return 0
    }
}

///////密码正确
//"\b\b\b\b    \b\b\b\bEverything is Ok"
//"\b\b\b\b    \b\b\b\b 66% 59 - 压缩小文件/深度_压缩工具_20191226/标注/index.html"
//"\b\b\b\b    \b\b\b\b 93% 49 - 压缩小文件/深度_压缩工具_201 . w/page-1-导入-新2.png"
//"\b\b\b\b    \b\b\b\b  0% 132 - 装机常用软件/EasyConnect_x64.deb"
///////密码错误
//"\b\b\b\b    \b\b\b\b"
//"\b\b\b\b    \b\b\b\bERROR: Data Error in encrypted file. Wrong password? : imp/importantinfo.txt"
//"\b\b\b\b    \b\b\b\bERROR: Data Error in encrypted file. Wrong password? : 压缩小文件/Deepin 压力测试方法_张成忠.docx"
//"\b\b\b\b    \b\b\b\bERROR: Data Error in encrypted file. Wrong password? : 压缩小文件/内存泄漏测试方法.docx"
