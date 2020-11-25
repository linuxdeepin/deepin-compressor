#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include <QModelIndex>
#include "analysepsdtool.h"
#include <QMimeDatabase>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <jobs.h>

TEST(AnalyseToolRar4_AnalyseToolRar4_UT, AnalyseToolRar4_AnalyseToolRar4_UT001)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->mark(ENUMLINEINFO::RIGHTPSD,"asdf",true);
    delete  toole;
}

TEST(AnalyseToolRar4_analyseLine_UT, AnalyseToolRar4_analyseLine_UT001)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->analyseLine("test%100");
    delete  toole;
}

TEST(AnalyseToolRar4_analyseLine_UT, AnalyseToolRar4_analyseLine_UT002)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->analyseLine("test%All OK");
    delete  toole;
}

TEST(AnalyseToolRar4_analyseLine_UT, AnalyseToolRar4_analyseLine_UT003)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->analyseLine("Extractingasd%adsOK ");
    delete  toole;
}

TEST(AnalyseToolRar4_getLineInfo_UT, AnalyseToolRar4_getLineInfo_UT001)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->getLineInfo(ENUMLINEINFO::RIGHTPSD);
    delete  toole;
}

TEST(AnalyseToolRar4_isRightPsd_UT, AnalyseToolRar4_isRightPsd_UT001)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->isRightPsd();
    delete  toole;
}

TEST(AnalyseToolRar4_isRightPsd_UT, AnalyseToolRar4_isRightPsd_UT002)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->analyseLine("Extractingasd%adsOK ");
    toole->isRightPsd();
    delete  toole;
}

TEST(AnalyseToolRar4_isRightPsd_UT, AnalyseToolRar4_isRightPsd_UT003)
{
    AnalyseToolRar4 * toole = new AnalyseToolRar4();
    toole->analyseLine("Extractingasd%adsOK ");
    (*toole->pMapInfo)[RIGHTPSD]->read =false;
    (*toole->pMapInfo)[WRONGPSD]->read =true;
    toole->isRightPsd();
    delete  toole;
}

TEST(AnalyseTool7Z_AnalyseTool7Z_UT, AnalyseTool7Z_AnalyseTool7Z_UT001)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();

    delete  toole;
}

TEST(AnalyseTool7Z_mark_UT, AnalyseTool7Z_mark_UT001)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    toole->mark(ENUMLINEINFO::RIGHTPSD,"asdf",true);
    delete  toole;
}

TEST(AnalyseTool7Z_analyseLine_UT, AnalyseTool7Z_analyseLine_UT001)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    toole->analyseLine("asdf");
    delete  toole;
}


TEST(AnalyseTool7Z_analyseLine_UT, AnalyseTool7Z_analyseLine_UT002)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    toole->analyseLine(DoubleBBBB);
    delete  toole;
}

TEST(AnalyseTool7Z_analyseLine_UT, AnalyseTool7Z_analyseLine_UT003)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    QString str =  "\b\b\b\b    \b\b\b\b ";
    QString str2 = ". Wrong password? ";
    QString line = str+str2;
    toole->analyseLine(line);
    delete  toole;
}


TEST(AnalyseTool7Z_analyseLine_UT, AnalyseTool7Z_analyseLine_UT004)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    QString str =  "\b\b\b\b    \b\b\b\b ";
    QString str2 = "Everything is Ok";
    QString line = str+str2;
    toole->analyseLine(line);
    delete  toole;
}

TEST(AnalyseTool7Z_analyseLine_UT, AnalyseTool7Z_analyseLine_UT005)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    QString str =  "\b\b\b\b    \b\b\b\b ";
    QString str2 = "sdfaafddddddddddddddddddd%dddddddsfa";
    QString line = str+str2;
    toole->analyseLine(line);
    delete  toole;
}

TEST(AnalyseTool7Z_analyseLine_UT, AnalyseTool7Z_analyseLine_UT006)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    QString str =  "Everything is Ok";
    QString str2 = "sdfaafddddddddddddddddddd%dddddddsfa";
    QString line = str+str2;
    toole->analyseLine(line);
    delete  toole;
}

TEST(AnalyseTool7Z_getLineInfo_UT, AnalyseTool7Z_getLineInfo_UT001)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    toole->getLineInfo(ENUMLINEINFO::RIGHTPSD);
    delete  toole;
}


TEST(AnalyseTool7Z_isRightPsd_UT, AnalyseTool7Z_isRightPsd_UT001)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
    toole->isRightPsd();
    delete  toole;
}

TEST(AnalyseTool7Z_isRightPsd_UT, AnalyseTool7Z_isRightPsd_UT002)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
   (*toole->pMapInfo)[RIGHTPSD]->read = true;
    toole->isRightPsd();
    delete  toole;
}

TEST(AnalyseTool7Z_isRightPsd_UT, AnalyseTool7Z_isRightPsd_UT003)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
     (*toole->pMapInfo)[WRONGPSD]->read = true;
    toole->isRightPsd();
    delete  toole;
}

TEST(AnalyseTool7Z_isRightPsd_UT, AnalyseTool7Z_isRightPsd_UT004)
{
    AnalyseTool7Z * toole = new AnalyseTool7Z();
     (*toole->pMapInfo)[WRONGPSD]->read = true;

    QMap<ENUMLINEINFO, LineInfo *>::iterator it = toole->pMapInfo->begin();
    while (it != toole->pMapInfo->end()) {
        delete it.value();
        it.value() = nullptr;
        it++;
    }
     toole->pMapInfo->clear();
    toole->isRightPsd();
    delete  toole;
}


TEST(AnalyseHelp_resetTempDir_UT, AnalyseHelp_resetTempDir_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->resetTempDir();
    delete  tool;
}

TEST(AnalyseHelp_clearPath_UT, AnalyseHelp_clearPath_UT002)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->clearPath("../UnitTest/regress/test.tst");
     delete  tool;
}

TEST(AnalyseHelp_getDestionFolderPath_UT, AnalyseHelp_getDestionFolderPath_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->getDestionFolderPath();
     delete  tool;
}

TEST(AnalyseHelp_getTempPath_UT, AnalyseHelp_getTempPath_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->getTempPath();
     delete  tool;
}


TEST(AnalyseHelp_analyseLine_UT, AnalyseHelp_analyseLine_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->analyseLine("UNRAR ../UnitTest/regress/s");
     delete  tool;
}

TEST(AnalyseHelp_analyseLine_UT, AnalyseHelp_analyseLine_UT002)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->analyseLine("7-Zip ../UnitTest/regress/s");
     delete  tool;
}

TEST(AnalyseHelp_analyseLine_UT, AnalyseHelp_analyseLine_UT003)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->analyseLine("7-Zip ../UnitTest/regress/s");
    tool->analyseLine("1../UnitTest/regress/s");
     delete  tool;
}

TEST(AnalyseHelp_mark_UT, AnalyseHelp_mark_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->analyseLine("7-Zip ../UnitTest/regress/s");
    tool->mark(ENUMLINEINFO::RIGHTPSD,"asdf",true);
     delete  tool;
}

TEST(AnalyseHelp_getLineInfo_UT, AnalyseHelp_getLineInfo_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->getLineInfo(ENUMLINEINFO::RIGHTPSD);
     delete  tool;
}

TEST(AnalyseHelp_getLineInfo_UT, AnalyseHelp_getLineInfo_UT002)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->analyseLine("7-Zip ../UnitTest/regress/s");
    tool->getLineInfo(ENUMLINEINFO::RIGHTPSD);
     delete  tool;
}

TEST(AnalyseHelp_setDestDir_UT, AnalyseHelp_setDestDir_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->setDestDir("7-Zip ../UnitTest/regress/s");
     delete  tool;
}

TEST(AnalyseHelp_getDestDir_UT, AnalyseHelp_getDestDir_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->getDestDir();
     delete  tool;
}


TEST(AnalyseHelp_isNotKnown_UT, AnalyseHelp_isNotKnown_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->isNotKnown();
     delete  tool;
}

TEST(AnalyseHelp_isNotKnown_UT, AnalyseHelp_isNotKnown_UT002)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->lineCount =2;
    tool->isNotKnown();
     delete  tool;
}

TEST(AnalyseHelp_isNotKnown_UT, AnalyseHelp_isNotKnown_UT003)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->lineCount =2;
    tool->analyseLine("7-Zip ../UnitTest/regress/s");
    tool->isNotKnown();
     delete  tool;
}


TEST(AnalyseHelp_checkReplaceTip_UT, AnalyseHelp_checkReplaceTip_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->checkReplaceTip(EXTRACT_REPLACE_TIP);
    delete  tool;
}

TEST(AnalyseHelp_isRightPsd_UT, AnalyseHelp_isRightPsd_UT001)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->isRightPsd();
    delete  tool;
}


TEST(AnalyseHelp_isRightPsd_UT, AnalyseHelp_isRightPsd_UT002)
{
    AnalyseHelp *tool = new AnalyseHelp("../UnitTest/regress/","test.tst");
    tool->analyseLine("7-Zip ../UnitTest/regress/s");
    tool->isRightPsd();
    delete  tool;
}
