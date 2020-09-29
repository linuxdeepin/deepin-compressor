#include <gtest/gtest.h>

#include <gtest/src/stub.h>
#include "cliplugin.h"
#include "queries.h"

TEST(Archive_CliPluginFactory_UT, Archive_CliPluginFactory_UT001)
{
    CliPluginFactory *cliPlugin = new CliPluginFactory;
    //  ArchiveSortFilterModel *model = new ArchiveSortFilterModel();
    EXPECT_NE(cliPlugin, nullptr);
    delete cliPlugin;
}


TEST(CliPlugin_CliPlugin,CliPlugin_CliPlugin_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    ASSERT_NE(cliPlugin, nullptr);
    delete cliPlugin;
}


TEST(CliPlugin_list,CliPlugin_list_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    bool isOK = cliPlugin->list();
    EXPECT_TRUE(isOK);
    delete cliPlugin;
}
bool extractFiles1(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    return true;
}
TEST(CliPlugin_extractFiles,CliPlugin_extractFiles_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    Archive::Entry * entry = new Archive::Entry;
    QVector<Archive::Entry *>  files;
    files << entry;
    typedef bool (CliInterface::*fptr)(const QVector<Archive::Entry *>&, const QString&, const ExtractionOptions&);
    fptr A_foo = static_cast<fptr>(&CliInterface::extractFiles);
    Stub *stub = new Stub;
   // stub->set(A_foo,extractFiles1);
    ExtractionOptions option;
    bool isOK = cliPlugin->extractFiles(files,"1",option);
    EXPECT_TRUE(isOK);
    delete cliPlugin;
    delete stub;
}

TEST(CliPlugin_resetParsing,CliPlugin_resetParsing_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->resetParsing();
    delete cliPlugin;
}

TEST(CliPlugin_setupCliProperties,CliPlugin_setupCliProperties_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->setupCliProperties();
    delete cliPlugin;
}


TEST(CliPlugin_readListLine,CliPlugin_readListLine_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    QString line = "Failed! ";
    bool isOK = cliPlugin->readListLine(line);
    EXPECT_TRUE(isOK);
    delete cliPlugin;
}

bool myHasMatch()
{
    return true;
}

TEST(CliPlugin_readListLine,CliPlugin_readListLine_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    QString line = "Failed! ";
    Stub *stub = new Stub;
    stub->set(ADDR(QRegularExpressionMatch,hasMatch),myHasMatch);
    bool isOK = cliPlugin->readListLine(line);
    EXPECT_FALSE(isOK);
    delete cliPlugin;
    delete  stub;
}

TEST(CliPlugin_readExtractLine,CliPlugin_readExtractLine_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    QString line = "Failed! ";
    bool isOK = cliPlugin->readExtractLine(line);
    EXPECT_TRUE(isOK);
    delete cliPlugin;
}


TEST(CliPlugin_readExtractLine,CliPlugin_readExtractLine_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    QString line = "Failed! ";
    Stub *stub = new Stub;
    stub->set(ADDR(QRegularExpressionMatch,hasMatch),myHasMatch);
    bool isOK = cliPlugin->readExtractLine(line);
    EXPECT_FALSE(isOK);
    delete cliPlugin;
    delete  stub;
}

TEST(CliPlugin_setJsonOutput,CliPlugin_setJsonOutput_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    QString str  ="123";
    cliPlugin->setJsonOutput(str);
    delete cliPlugin;
}

void myreadStdout(void *,bool)
{

}

TEST(CliPlugin_readStdout,CliPlugin_readStdout_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    typedef void (CliInterface::*fptr)(bool);
    fptr A_Foo = static_cast<fptr>(&CliInterface::readStdout);
    Stub *stub = new Stub;
  //  stub->set(A_Foo,myreadStdout);
   // cliPlugin->readStdout(false);
    delete cliPlugin;
    delete stub;
}

TEST(CliPlugin_readStdout,CliPlugin_readStdout_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->readStdout(true);
    delete cliPlugin;
}

void mywaitForResponse()
{

}

TEST(CliPlugin_handleLine,CliPlugin_handleLine_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->m_operationMode = ReadWriteArchiveInterface::OperationMode::List;
    QString line= "This archive requires a password to unpack. Use the -p option to provide one.";
    Stub *stub = new Stub;
    stub->set(ADDR(PasswordNeededQuery,waitForResponse),mywaitForResponse);
    bool isOK = cliPlugin->handleLine(line);
    EXPECT_TRUE(isOK);
    delete cliPlugin;
    delete  stub;
}

TEST(CliPlugin_handleLine,CliPlugin_handleLine_UT002)
{

}

TEST(CliPlugin_processFinished,CliPlugin_processFinished_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->m_abortingOperation = true;
    cliPlugin->processFinished(1,QProcess::ExitStatus::CrashExit);
    delete cliPlugin;
}

TEST(CliPlugin_processFinished,CliPlugin_processFinished_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->m_abortingOperation = false;
    cliPlugin->m_password = "";
    cliPlugin->processFinished(2,QProcess::ExitStatus::CrashExit);
    delete cliPlugin;
}

TEST(CliPlugin_readJsonOutput,CliPlugin_readJsonOutput_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->m_jsonOutput = "{,}";
    cliPlugin->readJsonOutput();
    delete  cliPlugin;

}


TEST(CliPlugin_readJsonOutput,CliPlugin_readJsonOutput_UT002)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->m_jsonOutput = "{\"lsarProperties\":{\"XADVolumes\":[1,2]},"
                              "\"lsarFormatName\":\"RAR\","
                              "\"lsarContents\":"
                              "[{\"XADFileName\":\"/home/lx777/mydemo/\","
                              "\"XADIsDirectory\":true,"
                              "\"XADFileSize\":1024,"
                              "\"XADCompressedSize\":1000,"
                              "\"XADLastModificationDate\":\"2019-12-10 12:05:01\","
                              "\"XADFileSize\":800,"
                              "\"XADIsEncrypted\":1}]}";
    cliPlugin->readJsonOutput();
    delete cliPlugin;
}

TEST(CliPlugin_readJsonOutput,CliPlugin_readJsonOutput_UT003)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    cliPlugin->m_jsonOutput = "{\"lsarProperties\":{\"XADVolumes\":[1,2]},"
                              "\"lsarFormatName\":\"RAR 5\","
                              "\"lsarContents\":"
                              "[{\"XADFileName\":\"/home/lx777/mydemo/\","
                              "\"XADIsDirectory\":true,"
                              "\"XADFileSize\":1024,"
                              "\"XADCompressedSize\":1000,"
                              "\"XADLastModificationDate\":\"2019-12-10 12:05:01\","
                              "\"XADFileSize\":800,"
                              "\"XADIsEncrypted\":1}]}";
    cliPlugin->readJsonOutput();
    delete cliPlugin;
}


TEST(CliPlugin_isPasswordPrompt,CliPlugin_isPasswordPrompt_UT001)
{
    QVariantList list;
    list << "1" << "2";
    CliPlugin *cliPlugin = new CliPlugin(nullptr,list);
    const QString str = "12345";
    bool isOK = cliPlugin->isPasswordPrompt(str);
    EXPECT_FALSE(isOK);

}













