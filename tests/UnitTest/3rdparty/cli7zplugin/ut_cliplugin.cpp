#include <gtest/gtest.h>

#include <gtest/src/stub.h>

#include <QFileInfo>
#include "kprocess.h"
#include "kprocess_p.h"
#include "cliplugin.h"
#include "mimetypes.h"
#include "pluginmanager.h"
#include "kpluginloader.h"
#include "kpluginfactory.h"
#include "archiveinterface.h"
#include <QMimeDatabase>
#include <QRegularExpression>

Q_DECLARE_METATYPE(KPluginMetaData)
TEST(Cli7zPlugin_Cli7zPlugin_UT, Cli7zPlugin_Cli7zPlugin_UT001)
{
    Cli7zPlugin *Cli7zPlugin = nullptr;
    ASSERT_EQ(Cli7zPlugin, nullptr);
}

TEST(Cli7zPlugin_Cli7zPlugin_UT, Cli7zPlugin_Cli7zPlugin_UT002)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    CliInterface *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    ASSERT_NE(Cli7zPlugin1, nullptr);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_CliPluginFactory_UT, Cli7zPlugin_CliPluginFactory_UT003)
{

    CliPluginFactory *cliPluginFactory = new CliPluginFactory();
    ASSERT_NE(cliPluginFactory, nullptr);
    delete cliPluginFactory;
}


TEST(Cli7zPlugin_resetParsing_UT, Cli7zPlugin_resetParsing_UT003)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->resetParsing();
    ASSERT_EQ(Cli7zPlugin1->m_listMap.count(), 0);
    ASSERT_EQ(Cli7zPlugin1->m_parseState, Cli7zPlugin::ParseState::ParseStateTitle);
    ASSERT_EQ(Cli7zPlugin1->m_comment.count(), 0);
    ASSERT_EQ(Cli7zPlugin1->m_numberOfVolumes, 0);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setupCliProperties_UT, Cli7zPlugin_setupCliProperties_UT004)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->setupCliProperties();

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("captureProgress"),QVariant (false));

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("addProgram"),QStringLiteral("7z"));
    QStringList temp = QStringList{QStringLiteral("a"),QStringLiteral("-l")};
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("addSwitch"),temp);

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("deleteProgram"),QStringLiteral("7z"));
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("deleteSwitch"),QStringLiteral("d"));

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("extractProgram"),QStringLiteral("7z"));
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("extractSwitch"), QStringList{QStringLiteral("x")});
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("progressarg"),QStringList{QStringLiteral("-bsp1")});
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("extractSwitchNoPreserve"),QStringList{QStringLiteral("e")});

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("listProgram"),QStringLiteral("7z"));
    temp = QStringList{QStringLiteral("l"),QStringLiteral("-slt")};
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("listSwitch"),temp);

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("moveProgram"),QStringLiteral("7z"));
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("moveSwitch"),QStringLiteral("rn"));

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("testProgram"),QStringLiteral("7z"));
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("testSwitch"),QStringLiteral("t"));

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("testProgram"),QStringLiteral("7z"));
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("testSwitch"),QStringLiteral("t"));

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("passwordSwitch"),QStringList{QStringLiteral("-p$Password")});
    temp = QStringList{QStringLiteral("-p$Password"),QStringLiteral("-mhe=on")};
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("passwordSwitchHeaderEnc"),temp);

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("compressionLevelSwitch"),QStringLiteral("-mx=$CompressionLevel"));

//    QHash<QString, QVariant> temp1= QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QStringLiteral("-m0=$CompressionMethod")},
//                                                              {QStringLiteral("application/zip"), QStringLiteral("-mm=$CompressionMethod")}
//                                                          };
//    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("compressionMethodSwitch"),temp1);


//    QHash<QString, QVariant> temp2 = QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QString()},
//            {QStringLiteral("application/zip"), QStringLiteral("-mem=$EncryptionMethod")}
//        };

//    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("encryptionMethodSwitch"),temp2);


    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("multiVolumeSwitch"),QStringLiteral("-v$VolumeSizek"));

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("testPassedPatterns"),QStringList{QStringLiteral("^Everything is Ok$")});

    QStringList temp3= QStringList{QStringLiteral("^file \\./(.*)$"),
            QStringLiteral("^  Path:     \\./(.*)$")};

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("fileExistsFileNameRegExp"),temp3);

    QStringList temp4=QStringList{QStringLiteral("Y"),   //Overwrite
            QStringLiteral("N"),   //Skip
            QStringLiteral("A"),   //Overwrite all
            QStringLiteral("S"),   //Autoskip
            QStringLiteral("Q")};
    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("fileExistsInput"),temp4);

    ASSERT_EQ(Cli7zPlugin1->m_cliProps->property("multiVolumeSuffix"),QStringList{QStringLiteral("$Suffix.001")});


    delete Cli7zPlugin1;

}


TEST(Cli7zPlugin_fixDirectoryFullName_UT, Cli7zPlugin_fixDirectoryFullName_UT005)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
    Cli7zPlugin1->fixDirectoryFullName();

    delete Cli7zPlugin1;
}



TEST(Cli7zPlugin_emitEntryForIndex_UT, Cli7zPlugin_emitEntryForIndex_UT006)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
    Cli7zPlugin1->emitEntryForIndex(archinve);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT007)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
    Cli7zPlugin1->setEntryVal(archinve,index,"",dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT008)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
     QString name = "/";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT009)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
     QString name = "//home/tmp";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT010)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
     QString name = "///";
     Cli7zPlugin1->m_SigDirRecord = "/";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT011)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
     QString name = "///";
     Cli7zPlugin1->m_SigDirRecord = "";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT012)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
     QString name = "///g";
     Cli7zPlugin1->m_SigDirRecord = "///g";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT013)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "";
     QString name = "///g";
     Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal_UT, Cli7zPlugin_setEntryVal_UT014)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "/home";
     QString name = "///g";
     Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryVal(archinve,index,name,dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    Cli7zPlugin1->setEntryVal1(archinve, index, "", dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT002)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    QString name = "/";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT003)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    QString name = "//home/tmp";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT004)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    QString name = "///";
    Cli7zPlugin1->m_SigDirRecord = "/";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT005)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    QString name = "///";
    Cli7zPlugin1->m_SigDirRecord = "";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT006)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    QString name = "///g";
    Cli7zPlugin1->m_SigDirRecord = "///g";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT007)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "";
    QString name = "///g";
    Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryVal1_UT, Cli7zPlugin_setEntryVal1_UT008)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath = "/home/test";
    ReadOnlyArchiveInterface::archive_stat archinve;
    int index = 0;
    QString dirRecord = "/home";
    QString name = "///g";
    Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryVal1(archinve, index, name, dirRecord);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryData_UT, Cli7zPlugin_setEntryData_UT015)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "/home";
     QString name = "///g";
     Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryData(archinve,0,name,true);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryData_UT, Cli7zPlugin_setEntryData_UT016)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "/home";
     QString name = "///g/";
     Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryData(archinve,0,name,false);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_setEntryDataA_UT, Cli7zPlugin_setEntryDataA_UT017)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
     Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
     ReadOnlyArchiveInterface::archive_stat  archinve;
     int index =0;
     QString dirRecord  = "/home";
     QString name = "///g/";
     Cli7zPlugin1->m_DirRecord = "///g";
    Cli7zPlugin1->setEntryDataA(archinve,name);
    delete Cli7zPlugin1;
}


TEST(Cli7zPlugin_setEntryDataA_UT, Cli7zPlugin_setEntryDataA_UT018)
{

  // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("//home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                           QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_fileStat.archive_isDirectory = true;
    Cli7zPlugin1->m_fileStat.archive_fullPath  = "/home/test";
    ReadOnlyArchiveInterface::archive_stat  archinve;
    int index =0;
    QString dirRecord  = "/home";
    QString name = "///g";
    Cli7zPlugin1->m_DirRecord = "///g";
    Archive::Entry *temp = Cli7zPlugin1->setEntryDataA(archinve,name);
    ASSERT_EQ(temp->property("fullPath"),name);
    ASSERT_EQ(temp->property("isDirectory"),false);
    delete temp;
    delete Cli7zPlugin1;

}



TEST(Cli7zPlugin_extractSize_UT, Cli7zPlugin_extractSize_UT019)
{

    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    QVector<Archive::Entry *> files ;
    Archive::Entry *file1 = new Archive::Entry;
    Archive::Entry *file2 = new Archive::Entry;
    Archive::Entry *file3 = new Archive::Entry;

    ReadOnlyArchiveInterface::archive_stat f;
    Cli7zPlugin1->m_listMap["/home"] = f;
    Cli7zPlugin1->m_listMap["/home/tmp/"] = f;
    Cli7zPlugin1->m_listMap["/home/tmp/a.php"] = f;
    file1->setFullPath("/home");
    file1->setSize(100);
    files.push_back(file1);

    file2->setFullPath("/home/tmp/");
    file2->setSize(100);
    files.push_back(file2);

    file3->setFullPath("/home/tmp/a.php");
    file3->setSize(100);
    files.push_back(file3);

    qint64 rsize = Cli7zPlugin1->extractSize(files);
    delete file3;
    delete file2;
    delete file1;
    delete Cli7zPlugin1;

}

TEST(Cli7zPlugin_isPasswordList_UT, Cli7zPlugin_isPasswordList_UT020)
{

    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    KProcess *p = new KProcess;
    KProcessPrivate *d = new  KProcessPrivate(p);
    d->args<<"ff";
    d->prog = "";
    Cli7zPlugin1->m_process  = new KProcess;
    bool rsize = Cli7zPlugin1->isPasswordList();
    ASSERT_EQ(rsize,false);
    delete d;
    delete p;
    delete Cli7zPlugin1;

}

TEST(Cli7zPlugin_isPasswordList_UT, Cli7zPlugin_isPasswordList_UT021)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    KProcess *p = new KProcess;

    KProcessPrivate *d = new  KProcessPrivate(p);
    d->args<<"ff";
    d->prog = "-p";
    Cli7zPlugin1->m_process  = new KProcess(d,nullptr);

    //Cli7zPlugin1->m_process->setProgram(programPath, arguments);
    bool rsize = Cli7zPlugin1->isPasswordList();
    ASSERT_EQ(rsize,true);
    delete d;
    delete p;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    KProcess *p = new KProcess;
    KProcessPrivate *d = new  KProcessPrivate(p);
    d->args<<"ff";
    d->prog = "-p";
    Cli7zPlugin1->m_process  = new KProcess(d,nullptr);

   bool r = Cli7zPlugin1->readListLine("Open ERROR: Can not open the file as [7z] archive");
    ASSERT_EQ(r,true);
    delete d;
    delete p;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT002)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    KProcess *p = new KProcess;
    KProcessPrivate *d = new  KProcessPrivate(p);
    d->args<<"ff";
    d->prog = "s";
    Cli7zPlugin1->m_process  = new KProcess(d,nullptr);

    bool r = Cli7zPlugin1->readListLine("Open ERROR: Can not open the file as [7z] archive");
    ASSERT_EQ(r,false);
    delete d;
    delete p;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT003)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    KProcess *p = new KProcess;
    KProcessPrivate *d = new  KProcessPrivate(p);
    d->args<<"ff";
    d->prog = "-p";
    Cli7zPlugin1->m_process  = new KProcess(d,nullptr);

    bool r=Cli7zPlugin1->readListLine("ERROR:Can not open the file as archive");
    ASSERT_EQ(r,true);
    delete d;
    delete p;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT004)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    KProcess *p = new KProcess;
    KProcessPrivate *d = new  KProcessPrivate(p);
    d->args<<"ff";
    d->prog = "s";
    Cli7zPlugin1->m_process  = new KProcess(d,nullptr);

    bool r= Cli7zPlugin1->readListLine("ERROR:Can not open the file as archive");
    ASSERT_EQ(r,false);
    delete d;
    delete p;
    delete Cli7zPlugin1;
}

// ParseStateTitleff分支
TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT005)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateTitle;
     bool r= Cli7zPlugin1->readListLine("p7zip Version 4.58 gfgfg");
    delete Cli7zPlugin1;
}
// ParseStateHeader 分支
TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT006)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateHeader;
    bool r= Cli7zPlugin1->readListLine("----");
    ASSERT_EQ(r,true);
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateHeader;
    r= Cli7zPlugin1->readListLine("Listing archive:");
    ASSERT_EQ(r,true);
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateHeader;
    r= Cli7zPlugin1->readListLine("Error:");
    ASSERT_EQ(r,true);
    delete Cli7zPlugin1;
}

// ParseStateArchiveInformation 分支
TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT007)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r1= Cli7zPlugin1->readListLine("----------");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r2= Cli7zPlugin1->readListLine("Type = 7z");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r3= Cli7zPlugin1->readListLine("Type = bzip2");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r4= Cli7zPlugin1->readListLine("Type = gzip");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r5= Cli7zPlugin1->readListLine("Type = xz");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r6= Cli7zPlugin1->readListLine("Type = tar");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r7= Cli7zPlugin1->readListLine("Type = zip");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r8= Cli7zPlugin1->readListLine("Type = Rar");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r9= Cli7zPlugin1->readListLine("Type = Split");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r10= Cli7zPlugin1->readListLine("Type = Udf");
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r11= Cli7zPlugin1->readListLine("Type = Iso");

    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r12 = Cli7zPlugin1->readListLine("Type = Iso1");

    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r13 = Cli7zPlugin1->readListLine("Volumes = Iso1");

    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r14 = Cli7zPlugin1->readListLine("Method = Iso1");

    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateArchiveInformation;
    bool r15 = Cli7zPlugin1->readListLine("Comment = Iso1");

    ASSERT_EQ(r1,true);
    ASSERT_EQ(r2,true);
    ASSERT_EQ(r3,true);
    ASSERT_EQ(r4,true);
    ASSERT_EQ(r5,true);
    ASSERT_EQ(r6,true);
    ASSERT_EQ(r7,true);
    ASSERT_EQ(r8,true);
    ASSERT_EQ(r9,true);
    ASSERT_EQ(r10,true);
    ASSERT_EQ(r11,true);
    ASSERT_EQ(r12,false);

    ASSERT_EQ(r13,true);
    ASSERT_EQ(r14,true);
    ASSERT_EQ(r15,true);
    delete Cli7zPlugin1;
}

// ParseStateComment 分支
TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT008)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateComment;
    Cli7zPlugin1->m_comment = "asdf";
    bool r1= Cli7zPlugin1->readListLine("----------");


    Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateComment;
    Cli7zPlugin1->m_comment = "asdf";
    bool r2= Cli7zPlugin1->readListLine("--------asd");

    ASSERT_EQ(r1,true);
    ASSERT_EQ(r2,true);
    delete Cli7zPlugin1;
}


// ParseStateEntryInformation 分支
TEST(Cli7zPlugin_readListLine_UT, Cli7zPlugin_readListLine_UT009)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/111.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
     Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
    Cli7zPlugin1->m_isFirstInformationEntry = true;
    bool r1= Cli7zPlugin1->readListLine("Path = dd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   bool r2= Cli7zPlugin1->readListLine("Size = dd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   Cli7zPlugin1-> m_archiveType = Cli7zPlugin::ArchiveTypeTar;
   bool r3= Cli7zPlugin1->readListLine("Packed Size = dd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   Cli7zPlugin1-> m_archiveType = Cli7zPlugin::ArchiveTypeIso;
   bool r4= Cli7zPlugin1->readListLine("Modified = dd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   bool r5= Cli7zPlugin1->readListLine("Folder = +");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   bool r6= Cli7zPlugin1->readListLine("Attributes = D_");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   bool r7= Cli7zPlugin1->readListLine("Attributes = asd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isFirstInformationEntry = true;
   bool r8= Cli7zPlugin1->readListLine("CRC =  asd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_archiveType = Cli7zPlugin::ArchiveTypeZip;
   bool r9= Cli7zPlugin1->readListLine("Method = asd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_isEncrypted = false;
   Cli7zPlugin1->m_fileStat.archive_isPasswordProtected = true;
   bool r10 = Cli7zPlugin1->readListLine("Encrypted = +sdf");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   bool r11 = Cli7zPlugin1->readListLine("Block =  asd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_archiveType = Cli7zPlugin::ArchiveTypeUdf;
   bool r12 = Cli7zPlugin1->readListLine("Accessed =  asd");

   Cli7zPlugin1->m_parseState = Cli7zPlugin::ParseStateEntryInformation;
   Cli7zPlugin1->m_archiveType = Cli7zPlugin::ArchiveTypeTar;
   bool r13 = Cli7zPlugin1->readListLine("Hard Link = asd");
   ASSERT_EQ(r1,true);

   ASSERT_EQ(r1,true);
   ASSERT_EQ(r2,true);
   ASSERT_EQ(r3,true);
   ASSERT_EQ(r4,true);
   ASSERT_EQ(r5,true);
   ASSERT_EQ(r6,true);
   ASSERT_EQ(r7,true);
   ASSERT_EQ(r8,true);
   ASSERT_EQ(r9,true);
   ASSERT_EQ(r10,true);
   ASSERT_EQ(r11,true);
   ASSERT_EQ(r12,true);
   ASSERT_EQ(r13,true);
   delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readExtractLine_UT, Cli7zPlugin_readExtractLine_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);


    bool r1= Cli7zPlugin1->readExtractLine("ERROR: E_FAIL");

    ASSERT_EQ(r1,false);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readExtractLine_UT, Cli7zPlugin_readExtractLine_UT002)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);


    bool r1= Cli7zPlugin1->readExtractLine("ERROR: CRC Failed");

    ASSERT_EQ(r1,false);

    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readExtractLine_UT, Cli7zPlugin_readExtractLine_UT003)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    bool r1= Cli7zPlugin1->readExtractLine("asdfsdf");
    ASSERT_EQ(r1,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readDeleteLine_UT, Cli7zPlugin_readDeleteLine_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    bool r1= Cli7zPlugin1->readDeleteLine("Error:  is not supported archive");
    ASSERT_EQ(r1,false);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_readDeleteLine_UT, Cli7zPlugin_readDeleteLine_UT002)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    bool r1= Cli7zPlugin1->readDeleteLine("fdfd");
    ASSERT_EQ(r1,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_handleMethods_UT, Cli7zPlugin_handleMethods_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    QStringList methods;

    methods<<"7zAES"<<"AES-128"<<"AES"<<"LZMA2s"<<"LZMA"<<"xz";
    Cli7zPlugin1->handleMethods(methods);
    //ASSERT_EQ(r1,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_isPasswordPrompt_UT, Cli7zPlugin_isPasswordPrompt_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);

    bool r1 = Cli7zPlugin1->isPasswordPrompt("Enter password (will not be echoed):asdsads");
    ASSERT_EQ(r1,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_isWrongPasswordMsg_UT, Cli7zPlugin_isWrongPasswordMsg_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);

    bool r1 = Cli7zPlugin1->isWrongPasswordMsg("Wrong password");
    ASSERT_EQ(r1,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_isCorruptArchiveMsg_UT, Cli7zPlugin_isCorruptArchiveMsg_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);

    bool r1 = Cli7zPlugin1->isCorruptArchiveMsg("Unexpected end of archive");
    ASSERT_EQ(r1,true);
    bool r2 = Cli7zPlugin1->isCorruptArchiveMsg("Headers Error");
    ASSERT_EQ(r2,true);
    delete Cli7zPlugin1;
}


TEST(Cli7zPlugin_isDiskFullMsg_UT, Cli7zPlugin_isDiskFullMsg_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);

    bool r1 = Cli7zPlugin1->isDiskFullMsg("No space left on device");
    ASSERT_EQ(r1,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_isFileExistsMsg_UT, Cli7zPlugin_isFileExistsMsg_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);

    bool r1 = Cli7zPlugin1->isFileExistsMsg("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ");
    ASSERT_EQ(r1,true);
    bool r2 = Cli7zPlugin1->isFileExistsMsg("? (Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ");
    ASSERT_EQ(r2,true);
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_isFileExistsFileName_UT, Cli7zPlugin_isFileExistsFileName_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);

    bool r1 = Cli7zPlugin1->isFileExistsFileName("file ./");
    ASSERT_EQ(r1,true);
    bool r2 = Cli7zPlugin1->isFileExistsFileName("  Path:     ./");
    ASSERT_EQ(r2,true);
    delete Cli7zPlugin1;
}


TEST(Cli7zPlugin_watchFileList_UT, Cli7zPlugin_watchFileList_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    QStringList *strList = new QStringList;

    Cli7zPlugin1->watchFileList(strList);
    delete strList;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_showEntryListFirstLevel_UT, Cli7zPlugin_showEntryListFirstLevel_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    ReadOnlyArchiveInterface::archive_stat f;
    Cli7zPlugin1->m_listMap["asdfasf"] = f;
    Cli7zPlugin1->showEntryListFirstLevel("asdfasf");

    Cli7zPlugin1->m_listMap["/home/"] = f;
    Cli7zPlugin1->m_listMap["asdfasf"] = f;
    Cli7zPlugin1->m_listMap["/home/asdfasf"] = f;
    Cli7zPlugin1->m_listMap["/home/asdfasf/"] = f;
    Cli7zPlugin1->m_listMap["/home/asdfasf/temp"] = f;
    Cli7zPlugin1->m_listMap["/home/asdfasf/temp/"] = f;
    Cli7zPlugin1->m_listMap["/home/asdfasf/temp/a.php"] = f;
    Cli7zPlugin1->m_listMap["/home/asdfasf/temp/a/"] = f;
    Cli7zPlugin1->showEntryListFirstLevel("/home/asdfasf");

    Cli7zPlugin1->showEntryListFirstLevel("/home/");
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_RefreshEntryFileCount_UT, Cli7zPlugin_RefreshEntryFileCount_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                       QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr,args);
    ReadOnlyArchiveInterface::archive_stat f;
    Cli7zPlugin1->m_listMap["/home"]= f;
    Cli7zPlugin1->m_listMap["/home/"]= f;
    Cli7zPlugin1->m_listMap["/home/temp"]= f;
    Cli7zPlugin1->m_listMap["/home/temp/"]= f;
    Cli7zPlugin1->m_listMap["/home/temp/a.php"]= f;
    Cli7zPlugin1->m_listMap["/home/temp/s"]= f;
    Archive::Entry * fil0 = nullptr;
    Cli7zPlugin1->RefreshEntryFileCount(fil0);

    Archive::Entry * file1 =new Archive::Entry;
    file1->setIsDirectory(false);
    Cli7zPlugin1->RefreshEntryFileCount(file1);

    Archive::Entry * file2 =new Archive::Entry;
    file1->setIsDirectory(true);
    Cli7zPlugin1->RefreshEntryFileCount(file2);

    Archive::Entry * file3 =new Archive::Entry;
    file2->setIsDirectory(true);
    file2->setFullPath("/home");

    Cli7zPlugin1->RefreshEntryFileCount(file3);


    delete file3;
    delete file2;
    delete file1;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_updateListMap_UT, Cli7zPlugin_updateListMap_UT001)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Archive::Entry *entry = new Archive::Entry();
    Cli7zPlugin1->updateListMap(entry, 1);
    delete entry;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_updateListMap_UT, Cli7zPlugin_updateListMap_UT002)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Archive::Entry *entry = new Archive::Entry();
    entry->setFullPath("/home");
    ReadOnlyArchiveInterface::archive_stat temp;
    Cli7zPlugin1->m_listMap["/home"] = temp;
    Cli7zPlugin1->updateListMap(entry, 2);
    delete entry;
    delete Cli7zPlugin1;
}

TEST(Cli7zPlugin_updateListMap_UT, Cli7zPlugin_updateListMap_UT003)
{
    // QWidget *qman = new QWidget();
    KPluginMetaData ss;
    const QVariantList args = {QVariant(QFileInfo("/home/lx777/Desktop/googletest.7z").absoluteFilePath()),
                               QVariant().fromValue(ss)};
    Cli7zPlugin *Cli7zPlugin1 = new Cli7zPlugin(nullptr, args);
    Archive::Entry *entry = new Archive::Entry();
    Archive::Entry *sub = new Archive::Entry(entry);
    sub->setIsDirectory(true);
    entry->setIsDirectory(true);
    entry->entries().push_back(sub);

    entry->setFullPath("/home");
    ReadOnlyArchiveInterface::archive_stat temp;
    Cli7zPlugin1->m_listMap["/home"] = temp;
    Cli7zPlugin1->updateListMap(entry, 100);
    delete sub;
    delete entry;
    delete Cli7zPlugin1;
}
