#ifndef EXTRACTLINESANALYZE_H
#define EXTRACTLINESANALYZE_H

#include <QMap>
#include <QThread>
#include "extractres.h"

#define MAXLINE_AFTER_PHYSICALSIZE 8            //check extract password whether right at most 3times after read line contains "Physical Size"
#define MAXLINE_RAR_CHECK 5

struct LineID
{
public:
    LineID(int id = 0):id(id)
    {

    }
    ~LineID()
    {

    }

    bool has = false;
    int id;
};

struct ExtractInfo
{
public:
    ExtractInfo(QString str,bool b):type(str),bReplaceTip(b)
    {

    }
    QString type = "";
    bool bReplaceTip = false;
};

/**
 * @brief 解压分析工具基类
 */
class ExtractAnalyzeBase
{

public:

    ExtractAnalyzeBase()
    {
       pInfo = new ExtractInfo("",false);
    }

    virtual ~ExtractAnalyzeBase()
    {

    }

    void analyseLine(QString line)
    {
        this->checkWrongPsd(line);
    }

    void setToolName(QString toolName)
    {
        if(this->pInfo != nullptr)
        {
            this->pInfo->type = toolName;
        }

    }

    QString getToolName()
    {
        return this->pInfo->type;
    }

    int getPsdStatus()
    {
        if(this->pInfo->type != nullptr && this->pInfo->type != "")
        {
            return this->isRightPsdNow();
        }
        return 0;
    }

//    virtual void checkIfReplace(QString line) = 0;
    void checkIfReplace(QString line)
    {
        if(this->pInfo->bReplaceTip == false && line.startsWith(EXTRACT_REPLACE_TIP) == true)
        {
            this->pInfo->bReplaceTip = true;
            printf(">>>>>>>>>>>>>>>>>>>>>>>>Would you like to replace the existing file<<<<<<<<<<<<<<<<<<<<<<<<\n");
        }
    }

    bool ifReplace()
    {
        return this->pInfo->bReplaceTip;
    }

    void clear()
    {
        if(pInfo != nullptr)
        {
            delete pInfo;
            pInfo = nullptr;
        }
    }

protected:

    virtual void checkWrongPsd(QString line) = 0;


    /**
     * @brief checkIfWrongPsd
     * @param line
     * @return 1:wrong password
     *         2:right password
     *         0:not known,continue read
     */
    virtual int isRightPsdNow() = 0;


private:
    ExtractInfo* pInfo;
};




class ExtractAnalyze7Z : public ExtractAnalyzeBase
{
public:
    ExtractAnalyze7Z()
    {
        this->setToolName(EXTRACT_TOOL_7Z);

        map_Lines = new QMap<QString,LineID>();
        map_Lines->insert(GlobalStrTable[IDS_PHYSICSIZE],0);
        map_Lines->insert(GlobalStrTable[IDS_WRONGPSDERROR],0);
        map_Lines->insert(GlobalStrTable[IDS_EVERYTHINGISOK],0);

    }

    ~ExtractAnalyze7Z() override
    {
        this->setToolName("");
        this->clear();
        if(map_Lines != nullptr)
        {
            map_Lines->clear();
            this->setToolName("");
            delete map_Lines;
            map_Lines = nullptr;
        }
    }

protected:

    void checkWrongPsd(QString line) override
    {
        if((*map_Lines)[GlobalStrTable[IDS_PHYSICSIZE]].has == true)
        {
            if(line.contains(GlobalStrTable[IDS_EVERYTHINGISOK]) == true)
            {
                (*map_Lines)[GlobalStrTable[IDS_EVERYTHINGISOK]].has = true;//密码正确
                return;
            }
            if(line.contains(GlobalStrTable[IDS_WRONGPSDERROR]) == true)
            {
                (*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].has = true;
            }
            else
            {
                if((*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].id > MAXLINE_AFTER_PHYSICALSIZE)
                {

                }
                else
                {
                    (*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].id++;
                }
            }
        }
        else
        {
            if(line.contains(GlobalStrTable[IDS_PHYSICSIZE]) == true)
            {
                (*map_Lines)[GlobalStrTable[IDS_PHYSICSIZE]].has = true;
            }
        }
    }

    /**
     * @brief checkIfWrongPsd
     * @param line
     * @return 1:wrong password
     *         2:right password
     *         0:not known,continue read
     */
    int isRightPsdNow() override
    {
        if((*map_Lines)[GlobalStrTable[IDS_EVERYTHINGISOK]].has == true)
        {
            return 2;
        }

        if((*map_Lines)[GlobalStrTable[IDS_PHYSICSIZE]].has == true)
        {
            if((*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].has == true)
            {
                return 1;
            }
            else
            {
                if((*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].id > MAXLINE_AFTER_PHYSICALSIZE)
                {
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        }
        else
        {
            return 0;
        }
    }

private:

    QMap<QString,LineID> *map_Lines;
};


class ExtractAnalyzeZip : public ExtractAnalyzeBase
{
public:
    explicit ExtractAnalyzeZip()
    {
        this->setToolName(EXTRACT_TOOL_ZIP);

        map_Lines = new QMap<QString,LineID>();
        map_Lines->insert(GlobalStrTable[IDS_PHYSICSIZE],0);
        map_Lines->insert(GlobalStrTable[IDS_WRONGPSDERROR],0);
        map_Lines->insert(GlobalStrTable[IDS_EVERYTHINGISOK],0);

    }

    ~ExtractAnalyzeZip() override
    {
        this->setToolName("");
        this->clear();
        if(map_Lines != nullptr)
        {
            if(map_Lines->isEmpty() == false)
            {
               map_Lines->clear();
            }

            this->setToolName("");
            delete map_Lines;
            map_Lines = nullptr;
        }
    }


protected:
    void checkWrongPsd(QString line) override
    {
        if((*map_Lines)[GlobalStrTable[IDS_PHYSICSIZE]].has == true)
        {
            if(line.contains(GlobalStrTable[IDS_EVERYTHINGISOK]) == true)
            {
                (*map_Lines)[GlobalStrTable[IDS_EVERYTHINGISOK]].has = true;//密码正确
                return;
            }
            if(line.contains(GlobalStrTable[IDS_WRONGPSDERROR]) == true)
            {
                (*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].has = true;
            }
            else
            {
                if((*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].id > MAXLINE_AFTER_PHYSICALSIZE)
                {

                }
                else
                {
                    (*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].id++;
                }
            }
        }
        else
        {
            if(line.contains(GlobalStrTable[IDS_PHYSICSIZE]) == true)
            {
                (*map_Lines)[GlobalStrTable[IDS_PHYSICSIZE]].has = true;
            }
        }
    }

    /**
     * @brief checkIfWrongPsd
     * @param line
     * @return 1:wrong password
     *         2:right password
     *         0:not known,continue read
     */
    int isRightPsdNow() override
    {
        if((*map_Lines)[GlobalStrTable[IDS_EVERYTHINGISOK]].has == true)
        {
            return 2;
        }

        if((*map_Lines)[GlobalStrTable[IDS_PHYSICSIZE]].has == true)
        {
            if((*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].has == true)
            {
                return 1;
            }
            else
            {
                if((*map_Lines)[GlobalStrTable[IDS_WRONGPSDERROR]].id > MAXLINE_AFTER_PHYSICALSIZE)
                {
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        }
        else
        {
            return 0;
        }
    }


private:
    QMap<QString,LineID> *map_Lines;

};

class ExtractAnalyzeRar : public ExtractAnalyzeBase
{
public:
    ExtractAnalyzeRar()
    {
        this->setToolName(EXTRACT_TOOL_RAR);

        map_Lines = new QMap<QString,LineID>();
        map_Lines->insert(GlobalStrTable[IDS_RAR_ERRORPSD],0);
        map_Lines->insert(GlobalStrTable[IDS_RAR_ALLOK],0);

    }

    ~ExtractAnalyzeRar() override
    {
        this->setToolName("");
        this->clear();
        if(map_Lines != nullptr)
        {
            if(map_Lines->isEmpty() == false)
            {
               map_Lines->clear();
            }

            this->setToolName("");
            delete map_Lines;
            map_Lines = nullptr;
        }
    }


protected:

    /**
     * @brief checkIfWrongPsd
     * @param line
     * @return 1:wrong password
     *         2:right password
     *         0:not known,continue read
     */
    void checkWrongPsd(QString line) override
    {
        if(line.contains(GlobalStrTable[IDS_RAR_ALLOK]) == true)
        {
            (*map_Lines)[GlobalStrTable[IDS_RAR_ALLOK]].has = true;//此时密码一定正确
            return;
        }

        if((*map_Lines)[GlobalStrTable[IDS_RAR_ERRORPSD]].has == true)
        {   //此时一定是密码错误
            return;
        }
        else
        {
            if(line.contains(GlobalStrTable[IDS_RAR_ERRORPSD]) == true
                    || line.contains(GlobalStrTable[IDS_RAR_WRONGPSD]) == true)
            {
                //此时一定是密码错误
                (*map_Lines)[GlobalStrTable[IDS_RAR_ERRORPSD]].has = true;
                return;
            }
            else
            {
                (*map_Lines)[GlobalStrTable[IDS_RAR_ERRORPSD]].id++;
            }
        }
    }

    int isRightPsdNow() override
    {
        if((*map_Lines)[GlobalStrTable[IDS_RAR_ALLOK]].has == true)
        {
            return 2;
        }
        if((*map_Lines)[GlobalStrTable[IDS_RAR_ERRORPSD]].has == true)
        {
            return 1;
        }
        else
        {
            if((*map_Lines)[GlobalStrTable[IDS_RAR_ERRORPSD]].id >MAXLINE_RAR_CHECK)
            {
                return 2;
            }
            else
            {
                return 0;
            }
        }
    }


private:

    QMap<QString,LineID> *map_Lines;

};




/**
 * @brief The ExtractAnalyzeHelper class
 * 帮助解压前的信息分析
 */


class ExtractAnalyzeHelper
{

public:
    ExtractAnalyzeHelper()
    {
        extractAnalyzeTool = nullptr;
    }
    ~ExtractAnalyzeHelper()
    {
        this->clean();
    }

    void clean()
    {
        if(extractAnalyzeTool != nullptr)
        {
            if(this->extractAnalyzeTool->getToolName() == EXTRACT_TOOL_7Z)
            {
                ExtractAnalyze7Z* p = dynamic_cast<ExtractAnalyze7Z*>(this->extractAnalyzeTool);
                delete p;
                p = nullptr;
            }
            else if(this->extractAnalyzeTool->getToolName() == EXTRACT_TOOL_RAR)
            {
                ExtractAnalyzeRar* p = dynamic_cast<ExtractAnalyzeRar*>(this->extractAnalyzeTool);
                delete p;
                p = nullptr;
            }
            else if(this->extractAnalyzeTool->getToolName() == EXTRACT_TOOL_ZIP)
            {
                ExtractAnalyzeZip* p = dynamic_cast<ExtractAnalyzeZip*>(this->extractAnalyzeTool);
                delete p;
                p = nullptr;
            }
        }
    }

    void checkLine(const QString& line)
    {
        if(extractAnalyzeTool == nullptr)
        {
            if(line.contains(GlobalStrTable[IDS_ISRAR]))
            {
                extractAnalyzeTool = new ExtractAnalyzeRar();
            }
            else if(line.contains(GlobalStrTable[IDS_IS7Z]))
            {
                extractAnalyzeTool = new ExtractAnalyze7Z();
            }
            else if(line.contains(GlobalStrTable[IDS_ISZIP]))
            {
                extractAnalyzeTool = new ExtractAnalyzeZip();
            }
        }

        if(extractAnalyzeTool != nullptr)
        {
            extractAnalyzeTool->analyseLine(line);
        }
    }

    void checkIfReplace(const QString& line)
    {
        if(extractAnalyzeTool == nullptr)
        {
            if(line.contains(GlobalStrTable[IDS_ISRAR]))
            {
                extractAnalyzeTool = new ExtractAnalyzeRar();
            }
            else if(line.contains(GlobalStrTable[IDS_IS7Z]))
            {
                extractAnalyzeTool = new ExtractAnalyze7Z();
            }
            else if(line.contains(GlobalStrTable[IDS_ISZIP]))
            {
                extractAnalyzeTool = new ExtractAnalyzeZip();
            }
        }

        if(extractAnalyzeTool != nullptr)
        {
            extractAnalyzeTool->checkIfReplace(line);
        }

    }

    bool ifReplace()
    {
        if(extractAnalyzeTool != nullptr)
        {
            return extractAnalyzeTool->ifReplace();
        }
        return false;
    }

    /**
     * @brief checkIfWrongPsd
     * @param line
     * @return 1:wrong password
     *         2:right password
     *         0:not known,continue read
     */
    int getPsdStatus()
    {
        if(this->extractAnalyzeTool != nullptr)
        {
            return this->extractAnalyzeTool->getPsdStatus();
        }
        return 0;
    }

private:
    ExtractAnalyzeBase* extractAnalyzeTool;
};








class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};



#endif // EXTRACTLINESANALYZE_H
