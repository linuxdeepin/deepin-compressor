#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include "cliplugin.h"
#include <memory>

TEST(CliPluginFactory_CliPluginFactory,CliPluginFactory_CliPluginFactory_UT001)
{
   CliPluginFactory *fc = new CliPluginFactory;
   ASSERT_NE(fc, nullptr);
   delete fc;
}

TEST(Archive_CliRarPlugin_UT, Archive_CliRarPlugin_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *cliRarPlugin = new CliRarPlugin(nullptr,list);
    ASSERT_NE(cliRarPlugin, nullptr);
    delete cliRarPlugin;
}
TEST(CliRarPlugin_resetParsing,CliRarPlugin_resetParsing_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *cliRarPlugin = new CliRarPlugin(nullptr,list);
    cliRarPlugin->resetParsing();
    EXPECT_EQ(cliRarPlugin->m_numberOfVolumes,0);
    delete cliRarPlugin;
}

TEST(CliRarPlugin_setupCliProperties,CliRarPlugin_setupCliProperties_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *cliRarPlugin = new CliRarPlugin(nullptr,list);
    cliRarPlugin->setupCliProperties();
    delete cliRarPlugin;
}

TEST(CliRarPlugin_readListLine,CliRarPlugin_readListLine_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString lines = "123";
    crar->m_remainingIgnoreLines = 2;
    bool isOK = crar->readListLine(lines);
    EXPECT_TRUE(isOK);
    delete  crar;
}

TEST(CliRarPlugin_readListLine,CliRarPlugin_readListLine_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateTitle;
    crar->m_remainingIgnoreLines = 0;
    QString lines = "123";
    bool isOK = crar->readListLine(lines);
    EXPECT_FALSE(isOK);
    delete  crar;

}

TEST(CliRarPlugin_readListLine,CliRarPlugin_readListLine_UT003)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateTitle;
    crar->m_remainingIgnoreLines = 0;
    QString lines = "UNRAR 6.0 beta 6.0";
   // Stub *stub = new Stub;

    bool isOK = crar->readListLine(lines);
    EXPECT_TRUE(isOK);
    delete  crar;

}

TEST(CliRarPlugin_readListLine,CliRarPlugin_readListLine_UT004)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
    crar->m_remainingIgnoreLines = 0;
    QString lines = "UNRAR 6.0 beta 6.0";
   // Stub *stub = new Stub;
    crar->m_isUnrar5 = true;
    bool isOK = crar->readListLine(lines);
    EXPECT_TRUE(isOK);
    delete  crar;

}

TEST(CliRarPlugin_readListLine,CliRarPlugin_readListLine_UT005)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
    crar->m_remainingIgnoreLines = 0;
    QString lines = "UNRAR 6.0 beta 6.0";
   // Stub *stub = new Stub;
    crar->m_isUnrar5 = false;
    bool isOK = crar->readListLine(lines);
    EXPECT_TRUE(isOK);
    delete  crar;

}

TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "Cannot find volume ";
    bool isOk = crar->handleUnrar5Line(line);
    EXPECT_FALSE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "Archive: ";
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}


TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT003)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "Archive11: ";
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}


TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT004)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateHeader;
    QString line = "Details: volume solid RAR 4 lock";
    crar->m_isMultiVolume = true;
    crar->m_isSolid = false;
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT005)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateHeader;
    QString line = "Details: volume solid RAR 5 lock";
    crar->m_isMultiVolume = true;
    crar->m_isSolid = false;
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT006)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "Archive: ";
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT007)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "Archive:";
    QHash<QString,QString> strHash;
    strHash.insert("1","1");
    crar->m_unrar5Details = strHash;
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}



TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT008)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "";
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}



TEST(CliRarPlugin_handleUnrar5Line,CliRarPlugin_handleUnrar5Line_UT009)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateLinkTarget;
    QString line = "";
    bool isOk= crar->handleUnrar5Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar5Entry,CliRarPlugin_handleUnrar5Entry_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_unrar5Details.insert("ratio","ratio");
    crar->m_unrar5Details.insert("mtime","mtime");
    crar->m_unrar5Details.insert("type","Directory");
    crar->m_unrar5Details.insert("name","test.7z");
    crar->m_unrar5Details.insert("compression","test.7z");
    crar->m_unrar5Details.insert("flags","encrypted");
    crar->m_unrar5Details.insert("fullPath","/home/lx777");
    crar->m_unrar5Details.insert("size","1024");
    crar->m_unrar5Details.insert("packed size","2048");
    crar->m_unrar5Details.insert("attributes","l 777");
    crar->handleUnrar5Entry();

}


TEST(CliRarPlugin_handleUnrar5Entry,CliRarPlugin_handleUnrar5Entry_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_unrar5Details.insert("ratio","ratio");
    crar->m_unrar5Details.insert("mtime","mtime");
    crar->m_unrar5Details.insert("type","Directory");
    crar->m_unrar5Details.insert("name","test.7z");
    crar->m_unrar5Details.insert("compression","te-st.7z");
    crar->m_unrar5Details.insert("flags","encrypted");
    crar->m_unrar5Details.insert("fullPath","/home/lx777");
    crar->m_unrar5Details.insert("size","1024");
    crar->m_unrar5Details.insert("packed size","2048");
    crar->m_unrar5Details.insert("attributes","l 777");
    crar->handleUnrar5Entry();
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "Cannot find volume ";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_FALSE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
    QString line = "Unsupported archive format. Please update RAR to a newer version.";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_FALSE(isOk);
    delete  crar;
}
bool myhasMatch()
{
    return true;
}
//TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT003)
//{
//    QVariantList list;
//    list << "1" << "2";
//    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
//    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
//    crar->m_isMultiVolume = false;
//    QString line = "Volume (Solid Volume) ";
//    Stub *stub = new Stub;
//    stub->set(ADDR(QRegularExpressionMatch,hasMatch),myhasMatch);
//    bool isOk = crar->handleUnrar4Line(line);
//    EXPECT_TRUE(isOk);
//    delete  crar;
//    delete  stub;
//}
//TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT004)
//{
//    QVariantList list;
//    list << "1" << "2";
//    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
//    crar->m_parseState = CliRarPlugin::ParseState::ParseStateComment;
//    crar->m_isMultiVolume = false;
//    QString line = "Solid archive Volume (Solid Volume) ";
//    Stub *stub = new Stub;
//    stub->set(ADDR(QRegularExpressionMatch,hasMatch),myhasMatch);
//    bool isOk = crar->handleUnrar4Line(line);
//    EXPECT_TRUE(isOk);
//    delete  crar;
//    delete  stub;
//}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT005)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateHeader;
    QString line = "--------------------";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT006)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateHeader;
    QString line = "Volume ";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT007)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateHeader;
    QString line = "Lock is present";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT008)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

QString str = "STM";

QString mycaptured1()
{
    return str;
}


TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT009)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "123";
    Stub *stub = new Stub;
    str = "STM";
    stub->set((QString (QRegularExpressionMatch::*)(int)const)ADDR(QRegularExpressionMatch,captured),mycaptured1);
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
    delete  stub;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT010)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "123";
    Stub *stub = new Stub;
    str = "CMT";
    stub->set((QString (QRegularExpressionMatch::*)(int)const)ADDR(QRegularExpressionMatch,captured),mycaptured1);
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
    delete  stub;
}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT011)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "123";
    Stub *stub = new Stub;
    str = "RR";
    stub->set((QString (QRegularExpressionMatch::*)(int)const)ADDR(QRegularExpressionMatch,captured),mycaptured1);
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
    delete  stub;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT012)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "123";
    Stub *stub = new Stub;
    str = "RR";
    stub->set((QString (QRegularExpressionMatch::*)(int)const)ADDR(QRegularExpressionMatch,captured),mycaptured1);
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
    delete  stub;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0013)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "-----------------";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0014)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    crar->m_isEncrypted = false;
    crar->m_isPasswordProtected = true;
    QString line = "*  ";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}


TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0015)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = "1234565";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0016)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryFileName;
    QString line = " 1234565";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0017)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "-----------------";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0018)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "------- ------ ----";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0019)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "- -- - - -- l-- l-- -- -- --";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}

TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0020)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateEntryDetails;
    QString line = "- -- - - -- -- -- -- -- --";
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
}
void myhandleUnrar4Entry()
{

}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0021)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = CliRarPlugin::ParseState::ParseStateLinkTarget;
    QString line = "- -- - - -- -- -- -- -- --";
    Stub *stub = new Stub();
    stub->set(ADDR(CliRarPlugin,handleUnrar4Entry),myhandleUnrar4Entry);
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
    delete  stub;
}
TEST(CliRarPlugin_handleUnrar4Line,CliRarPlugin_handleUnrar4Line_UT0022)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_parseState = (CliRarPlugin::ParseState)-1;
    QString line = "- -- - - -- -- -- -- -- --";
    Stub *stub = new Stub();
    stub->set(ADDR(CliRarPlugin,handleUnrar4Entry),myhandleUnrar4Entry);
    bool isOk = crar->handleUnrar4Line(line);
    EXPECT_TRUE(isOk);
    delete  crar;
     delete  stub;
}

TEST(CliRarPlugin_handleUnrar4Entry,CliRarPlugin_handleUnrar4Entry_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_unrar4Details.append("/home/lxzzz/");
    crar->m_unrar4Details.append("1");
    crar->m_unrar4Details.append("2");
    crar->m_unrar4Details.append("<--");
    crar->m_unrar4Details.append("1920-02-03");
    crar->m_unrar4Details.append("19:20:02");
    crar->m_unrar4Details.append("Dir");
    crar->m_unrar4Details.append("Dir");
    crar->m_unrar4Details.append("Dir");
    crar->m_unrar4Details.append("Dir");
    crar->handleUnrar4Entry();
}
TEST(CliRarPlugin_handleUnrar4Entry,CliRarPlugin_handleUnrar4Entry_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_unrar4Details.append("/home/lxzzz/");
    crar->m_unrar4Details.append("1");
    crar->m_unrar4Details.append("2");
    crar->m_unrar4Details.append("<<--");
    crar->m_unrar4Details.append("1920-02-03");
    crar->m_unrar4Details.append("19:20:02");
    crar->m_unrar4Details.append("lDir");
    crar->m_unrar4Details.append("Dir");
    crar->m_unrar4Details.append("Dir");
    crar->m_unrar4Details.append("Dir");
    crar->m_unrar4Details.append("Dir");
    crar->handleUnrar4Entry();
}

TEST(CliRarPlugin_readExtractLine,CliRarPlugin_readExtractLine_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "CRC failed";
    bool isOK = crar->readExtractLine(line);
    EXPECT_FALSE(isOK);
    delete crar;
}

TEST(CliRarPlugin_readExtractLine,CliRarPlugin_readExtractLine_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "Cannot find volume ";
    bool isOK = crar->readExtractLine(line);
    EXPECT_FALSE(isOK);
    delete crar;
}

TEST(CliRarPlugin_readExtractLine,CliRarPlugin_readExtractLine_UT003)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "Cannot 111 ";
    bool isOK = crar->readExtractLine(line);
    EXPECT_TRUE(isOK);
    delete crar;
}

TEST(CliRarPlugin_hasBatchExtractionProgress,CliRarPlugin_hasBatchExtractionProgress_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    bool isOK = crar->hasBatchExtractionProgress();
    EXPECT_TRUE(isOK);
    delete crar;
}

TEST(CliRarPlugin_ignoreLines,CliRarPlugin_ignoreLines_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->ignoreLines(10,CliRarPlugin::ParseState::ParseStateTitle);
    EXPECT_EQ(crar->m_remainingIgnoreLines,10);
    delete crar;
}

TEST(CliRarPlugin_emitEntryForIndex,CliRarPlugin_emitEntryForIndex_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    crar->m_isEncrypted = false;
    QHash<QString,QString> hashInfo;
    hashInfo.insert("ratio","ratio");
    hashInfo.insert("mtime","2020-09-16 10:30:10");
    hashInfo.insert("type","Directory");
    hashInfo.insert("name","/home/lx777/");
    hashInfo.insert("compression","compression");
    hashInfo.insert("flags","encrypted");
    hashInfo.insert("size","1024");
    hashInfo.insert("packed size","10240");
    hashInfo.insert("attributes","l777");
    crar->m_unrar5Details = hashInfo;
    bool isOK = crar->emitEntryForIndex(archive);
    EXPECT_TRUE(isOK);
    delete crar;
}

TEST(CliRarPlugin_emitEntryForIndex,CliRarPlugin_emitEntryForIndex_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    crar->m_isEncrypted = false;
    QHash<QString,QString> hashInfo;
    hashInfo.insert("ratio","ratio");
    hashInfo.insert("mtime","2020-09-16 10:30:10");
    hashInfo.insert("type","Directory");
    hashInfo.insert("name","/home/lx777/");
    hashInfo.insert("compression","compre-ssion");
    hashInfo.insert("flags","encrypted");
    hashInfo.insert("size","1024");
    hashInfo.insert("packed size","10240");
    hashInfo.insert("attributes","l777");
    crar->m_unrar5Details = hashInfo;
    bool isOK = crar->emitEntryForIndex(archive);
    EXPECT_TRUE(isOK);
    delete crar;
}

TEST(CliRarPlugin_setEntryVal,CliRarPlugin_setEntryVal_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    archive.archive_fullPath = "home";
    crar->setEntryVal(archive);
    delete crar;
}

TEST(CliRarPlugin_setEntryData,CliRarPlugin_setEntryData_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    archive.archive_fullPath = "home";
    crar->setEntryData(archive);
    delete crar;
}
TEST(CliRarPlugin_setEntryData,CliRarPlugin_setEntryData_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    archive.archive_fullPath = "home";
    crar->setEntryData(archive,false);
    delete crar;
}

TEST(CliRarPlugin_setEntryDataA,CliRarPlugin_setEntryDataA_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    auto a = crar->setEntryDataA(archive);
    EXPECT_NE(a,nullptr);
    delete crar;
}

TEST(CliRarPlugin_extractSize,CliRarPlugin_extractSize_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    Archive::Entry *entry = new Archive::Entry(nullptr,"/home/lx777/mydemo/");
    entry->rootNode = "/home/lx777/mydemo/";
    Archive::Entry *entry1 = new Archive::Entry(nullptr,"/home/lx777/mydemo/123/");
    entry1->rootNode = "/home/lx777/mydemo/";
    Archive::Entry *entry2 = new Archive::Entry(nullptr,"/home/lx777/mydemo/test.zip");
    entry2->rootNode = "/home/lx777/mydemo/123/";
    Archive::Entry *entry3 = new Archive::Entry(nullptr,"/home/lx777/mydemo/1test.zip");
    QVector<Archive::Entry *> files;
    files << entry << entry1 << entry2 << entry3;
    ReadOnlyArchiveInterface::archive_stat archive;
    crar->m_listMap.insert("/home/lx777/mydemo/",archive);
    ReadOnlyArchiveInterface::archive_stat archive1;
    crar->m_listMap.insert("/home/lx777/mydemo/123/",archive1);
    ReadOnlyArchiveInterface::archive_stat archive2;
    crar->m_listMap.insert("/home/lx777/mydemo/test.zip",archive2);

    crar->extractSize(files);
    delete crar;


}

TEST(CliRarPlugin_isPasswordPrompt,CliRarPlugin_isPasswordPrompt_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "123";
    bool isOK = crar->isPasswordPrompt(line);
    EXPECT_FALSE(isOK);
    delete crar;
}
TEST(CliRarPlugin_isWrongPasswordMsg,CliRarPlugin_isWrongPasswordMsg_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "123";
    bool isOK = crar->isWrongPasswordMsg(line);
    EXPECT_FALSE(isOK);
    delete crar;
}
TEST(CliRarPlugin_isCorruptArchiveMsg,CliRarPlugin_isCorruptArchiveMsg_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "123";
    bool isOK = crar->isCorruptArchiveMsg(line);
    EXPECT_FALSE(isOK);
    delete crar;
}
TEST(CliRarPlugin_isDiskFullMsg,CliRarPlugin_isDiskFullMsg_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "123";
    bool isOK = crar->isDiskFullMsg(line);
    EXPECT_FALSE(isOK);
    delete crar;
}
TEST(CliRarPlugin_isFileExistsMsg,CliRarPlugin_isFileExistsMsg_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "123";
    bool isOK = crar->isFileExistsMsg(line);
    EXPECT_FALSE(isOK);
    delete crar;
}
TEST(CliRarPlugin_isFileExistsFileName,CliRarPlugin_isFileExistsFileName_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    QString line = "123";
    bool isOK = crar->isFileExistsFileName(line);
    EXPECT_FALSE(isOK);
    delete crar;
}


TEST(CliRarPlugin_isLocked,CliRarPlugin_isLocked_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    crar->m_isLocked = false;
    bool isOK = crar->isLocked();
    EXPECT_FALSE(isOK);
    delete crar;
}


TEST(CliRarPlugin_showEntryListFirstLevel,CliRarPlugin_showEntryListFirstLevel_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);

    crar->showEntryListFirstLevel("");
}

TEST(CliRarPlugin_showEntryListFirstLevel,CliRarPlugin_showEntryListFirstLevel_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    ReadOnlyArchiveInterface::archive_stat archive;
    crar->m_listMap.insert("/home/lx777/mydemo/",archive);
    ReadOnlyArchiveInterface::archive_stat archive1;
    crar->m_listMap.insert("/home/lx777/mydemo/123/",archive1);
    ReadOnlyArchiveInterface::archive_stat archive2;
    crar->m_listMap.insert("/home/lx777/mydemo/test.zip",archive2);
    ReadOnlyArchiveInterface::archive_stat archive3;
    crar->m_listMap.insert("/home/ax777/",archive3);
    QString strDir = "/home/lx777/mydemo/";
    crar->showEntryListFirstLevel(strDir);


}

TEST(CliRarPlugin_RefreshEntryFileCount,CliRarPlugin_RefreshEntryFileCount_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    Archive::Entry *file = new Archive::Entry(nullptr,"/home/lx777/mydemo/test.zip");
    file->m_isDirectory = false;
    crar->RefreshEntryFileCount(file);
}

TEST(CliRarPlugin_RefreshEntryFileCount,CliRarPlugin_RefreshEntryFileCount_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliRarPlugin *crar = new CliRarPlugin(nullptr,list);
    Archive::Entry *file = new Archive::Entry(nullptr,"/home/lx777/mydemo/");
    file->m_isDirectory = true;
    ReadOnlyArchiveInterface::archive_stat archive;
    crar->m_listMap.insert("/home/lx777/mydemo/",archive);
    ReadOnlyArchiveInterface::archive_stat archive1;
    crar->m_listMap.insert("/home/lx777/mydemo/123/",archive1);
    ReadOnlyArchiveInterface::archive_stat archive2;
    crar->m_listMap.insert("/home/lx777/mydemo/test.zip",archive2);
    crar->RefreshEntryFileCount(file);
}


















