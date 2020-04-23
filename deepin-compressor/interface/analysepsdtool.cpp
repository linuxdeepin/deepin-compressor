#include "analysepsdtool.h"
#include <stdio.h>

#include <DStandardPaths>
#include <QDir>
#include <QProcess>

DCORE_USE_NAMESPACE

lineInfo::lineInfo(QString l, bool b): line(l), read(b)
{

}

AnalyseTool::AnalyseTool()
{

}

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
    pMapInfo->insert(REPLACE, new LineInfo("", false));
    pMapInfo->insert(RIGHTPSD,new LineInfo("",false));
    lineCount = 0;
}

void AnalyseToolRar4::mark(ENUMLINEINFO id, QString line, bool read)
{
    if(pMapInfo->contains(id) == true){
        (*pMapInfo)[id]->line = line;
        (*pMapInfo)[id]->read = read;
    }
}

void AnalyseToolRar4::analyseLine(const QString &line)
{
//    if ((*pMapInfo)[REPLACE]->read == false) {
//        if (line.contains(EXTRACT_REPLACE_TIP) == true) {
//            (*pMapInfo)[REPLACE]->line = line;
//            (*pMapInfo)[REPLACE]->read = true;
//        }
//    }
    lineCount++;

    if(line.startsWith("Extracting") == true && line.endsWith("OK ")){
        (*pMapInfo)[RIGHTPSD]->line = line;
        (*pMapInfo)[RIGHTPSD]->read = true;
        return;
    }else if(line.contains(ALLOK) == true){
        (*pMapInfo)[RIGHTPSD]->line = line;
        (*pMapInfo)[RIGHTPSD]->read = true;
    }

//    if(lineCount>VALIDLINE){
//        if((*pMapInfo)[WRONGPSD]->read == false){
//            (*pMapInfo)[RIGHTPSD]->line = line;
//            (*pMapInfo)[RIGHTPSD]->read = true;
//        }
//    }else{

//    }
}

LineInfo *AnalyseToolRar4::getLineInfo(ENUMLINEINFO id)
{
    return (*pMapInfo)[id];
}

int AnalyseToolRar4::isRightPsd(){
    if(pMapInfo->contains(RIGHTPSD) == true){
        if((*pMapInfo)[RIGHTPSD]->read == true){
            return 1;
        }else if ((*pMapInfo)[WRONGPSD]->read == true){
            return 2;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

bool isDirExist(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
      return true;
    }
    else
    {
       bool ok = dir.mkpath(fullPath);//创建多级目录
       return ok;
    }
}

AnalyseHelp::AnalyseHelp(QString destinationPath,QString subFolderName)
{
    lineCount = 0;
    destPath = destinationPath;
    destSubFolderName = subFolderName;
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    tempPath = confDir + QDir::separator() + "tempExtractAAA";
    this->clearPath(tempPath);
    QDir Dir(tempPath);
    if(Dir.isEmpty())
    {
        printf("\ntemp dir %s is empty",tempPath.toUtf8().data());
        isDirExist(tempPath);
    }
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
    if(destSubFolderName == ""){
        return "";
    }
    if(destPath == ""){
        return "";
    }
    return destPath+"/"+destSubFolderName;
}

QString AnalyseHelp::getTempPath(){
    return this->tempPath;
}

AnalyseHelp::~AnalyseHelp(){
    this->clearPath(tempPath);
    delete pTool;
    pTool = nullptr;
    lineCount = 0;
    this->replaceTip = false;
}

void AnalyseHelp::analyseLine(const QString& line){
    if(pTool == nullptr){
        if(line.left(5) == "UNRAR"){
            pTool = new AnalyseToolRar4();
        }
    }else{
        int pos = line.indexOf(QLatin1Char('%'));
        if (pos > 1) {
            int percentage = line.midRef(pos - 3, 3).toInt();
            if(percentage>=0){
                pTool->analyseLine(line);
            }
        }
    }
    this->lineCount++;

}

void AnalyseHelp::mark(ENUMLINEINFO id,QString line,bool read)
{
    if(pTool != nullptr){
        pTool->mark(id,line,read);
    }
}

LineInfo* AnalyseHelp::getLineInfo(ENUMLINEINFO id){
    if(pTool != nullptr){
        return pTool->getLineInfo(id);
    }else{
        return nullptr;
    }
}

void AnalyseHelp::setDestDir(const QString &path){
    this->destPath = path;
}

QString AnalyseHelp::getDestDir(){
    return this->destPath;
}

bool AnalyseHelp::hasReplace(){
    LineInfo *pLineInfoRep = this->getLineInfo(ENUMLINEINFO::REPLACE);
    if (pLineInfoRep != nullptr) {
        return pLineInfoRep->read;
    }else{
        return false;
    }
}

bool AnalyseHelp::isNotKnown(){
    if(lineCount>=1){
        if(pTool == nullptr){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

bool AnalyseHelp::isNeedRemoveTemp(){
    if(this->hasReplace() == false){//tips replace;
        LineInfo* p = this->getLineInfo(ENUMLINEINFO::WRONGPSD);
        if(p != nullptr && p->read == true){
            return true;//psd is right
        }else{
            return false;//psd is wrong
        }
    }else{
        return false;
    }
}

void AnalyseHelp::checkReplaceTip(const QString &line){
    if(line.contains(EXTRACT_REPLACE_TIP) == true){
        this->replaceTip = true;
    }
}

int AnalyseHelp::isRightPsd(){
    if(this->pTool != nullptr){
        return this->pTool->isRightPsd();
    }else{
        return 0;//not known archive ,so return 0
    }

}


