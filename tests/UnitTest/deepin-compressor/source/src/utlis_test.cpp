#include <gtest/gtest.h>
#include "utils.h"
#include <gtest/src/stub.h>
#include <iostream>
#include <QList>
#include <QSize>
#include <QPixmap>
#include <QTextCodec>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QMimeType>
#include <QRegularExpressionMatch>
#include <QLocale>

QString strCaptured = "charset";
QLocale::Script myScript = QLocale::ArabicScript;

TEST(Utils_GetConfigPath_UT, Utils_GetConfigPath_UT001)
{
    QString temp = Utils::getConfigPath();
    std::cout << temp.toStdString() << std::endl;
    ASSERT_STREQ(temp.toStdString().c_str(), "/home/lx777/.config/deepin/deepin-compressor");
}

TEST(Utils_SuffixList_UT, Utils_SuffixList_UT001)
{
  //  QString str = Utils::suffixList();
 //   ASSERT_STREQ(str.toStdString().c_str(), "");
}

TEST(Utils_renderSVG_UT, Utils_renderSVG_UT001)
{
    QPixmap pixmap = Utils::renderSVG("", QSize(0, 0));
}

TEST(Utils_isCompressed_file_UT, Utils_isCompressed_file_UT001)
{
    bool ret = Utils::isCompressed_file("test.deb");
    ASSERT_EQ(ret, false);
}
TEST(Utils_isCompressed_file_UT, Utils_isCompressed_file_UT002)
{
    bool ret = Utils::isCompressed_file("test.crx");
    ASSERT_EQ(ret, true);
}

TEST(Utils_humanReadableSize_UT, Utils_humanReadableSize_UT001)
{
    QString str = Utils::humanReadableSize(10240, 50);
    // std::cout << str.toStdString() << std::endl;
    ASSERT_STREQ(str.toStdString().c_str(), "10.00000000000000000000000000000000000000000000000000 KB");
}

TEST(Utils_humanReadableSize_UT, Utils_humanReadableSize_UT002)
{
    QString str = Utils::humanReadableSize(0, 50);
    // std::cout << str.toStdString() << std::endl;
    ASSERT_STREQ(str.toStdString().c_str(), "-");
}

TEST(Utils_humanReadableSize_UT, Utils_humanReadableSize_UT003)
{
    QString str = Utils::humanReadableSize(102400, 50);
    // std::cout << str.toStdString() << std::endl;
    ASSERT_STREQ(str.toStdString().c_str(), "100.00000000000000000000000000000000000000000000000000 KB");
}

TEST(Utils_humanReadableToSize_UT, Utils_humanReadableToSize_UT001)
{
    qint64 num = Utils::humanReadableToSize("10 KB");
    ASSERT_EQ(num, 10240);
}
TEST(Utils_humanReadableToSize_UT, Utils_humanReadableToSize_UT002)
{
    qint64 num = Utils::humanReadableToSize("10NB");
    ASSERT_EQ(num, 0);
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT001)
{
//    std::string str = Utils::detectEncode("", "").toStdString();
//    ASSERT_STREQ(str.c_str(), "UTF-8");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT002)
{
//    std::string str = Utils::detectEncode(QString("你好").toLatin1(), "").toStdString();
//    ASSERT_STREQ(str.c_str(), "UTF-8");
}

QTextCodec *mycodecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec)
{
    return QTextCodec::codecForName("Big5");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT003)
{
//    typedef QTextCodec *(*fptr)(const QByteArray &, QTextCodec *);
//    fptr A_foo = (fptr)(&QTextCodec::codecForUtfText); //obtaining an address
//    Stub *stub = new Stub;
//    stub->set(A_foo, mycodecForUtfText);
//    std::string str = Utils::detectEncode(QString("你好").toLatin1(), "").toStdString();
//    stub->reset(A_foo);
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "Big5");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT004)
{
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    ASSERT_STREQ(str.c_str(), "UTF-8");
}

QString mycaptured(void *obj, const QString &name)
{
    if (strCaptured == "charset")
        return "test";
    else if (strCaptured == "language" && name == "language")
        return "test";
    else if (strCaptured == "coding" && name == "coding")
        return "coding";
    return "";
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT005)
{
//    strCaptured = "charset";
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "test");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT006)
{
//    strCaptured = "language";
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "UTF-8");
}

QLocale::Script myscript()
{
    return myScript;
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT007)
{
//    strCaptured = "language";
//    myScript = QLocale::ArabicScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}
TEST(Utils_detectEncode_UT, Utils_detectEncode_UT008)
{
//    strCaptured = "language";
//    myScript = QLocale::SimplifiedChineseScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}
TEST(Utils_detectEncode_UT, Utils_detectEncode_UT009)
{
//    strCaptured = "language";
//    myScript = QLocale::TraditionalChineseScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT010)
{
//    strCaptured = "language";
//    myScript = QLocale::CyrillicScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT011)
{
//    strCaptured = "language";
//    myScript = QLocale::GreekScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}
TEST(Utils_detectEncode_UT, Utils_detectEncode_UT012)
{
//    strCaptured = "language";
//    myScript = QLocale::HebrewScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT013)
{
//    strCaptured = "language";
//    myScript = QLocale::JapaneseScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT014)
{
//    strCaptured = "language";
//    myScript = QLocale::KoreanScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT015)
{
//    strCaptured = "language";
//    myScript = QLocale::ThaiScript;
//    Stub *stub = new Stub;
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    stub->set(ADDR(QLocale, script), myscript);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QLocale, script));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "gb18030");
}

QString myname()
{
    return "text/x-python";
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT016)
{
//    Stub *stub = new Stub;
//    stub->set(ADDR(QMimeType, name), myname);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QMimeType, name));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "UTF-8");
}

TEST(Utils_detectEncode_UT, Utils_detectEncode_UT017)
{
//    strCaptured = "coding";
//    Stub *stub = new Stub;
//    stub->set(ADDR(QMimeType, name), myname);
//    stub->set((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured), mycaptured);
//    std::string str = Utils::detectEncode(QString("<html></html>").toLatin1(), "").toStdString();
//    stub->reset(ADDR(QMimeType, name));
//    stub->reset((QString(QRegularExpressionMatch::*)(const QString &) const)ADDR(QRegularExpressionMatch, captured));
//    delete stub;
//    ASSERT_STREQ(str.c_str(), "UTF-8");
}

TEST(Utils_toShortString_UT, Utils_toShortString_UT001)
{
    QString str = Utils::toShortString("test.rar");
    ASSERT_STREQ(str.toLocal8Bit().data(), "test.rar");
}

TEST(Utils_checkAndDeleteDir_UT, Utils_checkAndDeleteDir_UT001)
{
    bool ret = Utils::checkAndDeleteDir("test");
    ASSERT_FALSE(ret);
}

TEST(Utils_checkAndDeleteDir_UT, Utils_checkAndDeleteDir_UT002)
{
    QDir dir;
    if (!dir.exists("/home/lx777/test111")) {
        dir.mkdir("/home/lx777/test111");
    }
    bool ret = Utils::checkAndDeleteDir("/home/lx777/test111");
    ASSERT_TRUE(ret);
}

TEST(Utils_checkAndDeleteDir_UT, Utils_checkAndDeleteDir_UT003)
{
    QFile file("/home/lx777/123.txt");
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.close();
    }
    bool ret = Utils::checkAndDeleteDir("/home/lx777/123.txt");
    ASSERT_TRUE(ret);
}
TEST(Utils_deleteDir_UT, Utils_deleteDir_UT001)
{
    bool ret = Utils::deleteDir("test");
    ASSERT_TRUE(ret);
}

TEST(Utils_deleteDir_UT, Utils_deleteDir_UT002)
{
    QDir dir;
    if (!dir.exists("/home/lx777/test222")) {
        dir.mkdir("/home/lx777/test222");
    }
    bool ret = Utils::deleteDir("/home/lx777/test222");
    ASSERT_TRUE(ret);
}

TEST(Utils_deleteDir_UT, Utils_deleteDir_UT003)
{
    QFile file("/home/lx777/1111.txt");
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.close();
    }
    bool ret = Utils::deleteDir("/home/lx777/1111.txt");
    ASSERT_TRUE(ret);
}

bool myFileRemove(const QString &fileName)
{
    return false;
}

TEST(Utlis_deleteDir_UT, Utils_deleteDir_UT004)
{
    QDir dir;
    if (!dir.exists("/home/lx777/test222")) {
        dir.mkdir("/home/lx777/test222");
        QFile file("/home/lx777/test222/1111.txt");
        if (!file.exists()) {
            file.open(QIODevice::ReadWrite | QIODevice::Text);
            file.close();
        }
    }
    typedef bool (*fptr)(const QString &);
    fptr A_foo = (fptr)(&QFile::remove); //obtaining an address
    Stub *stub = new Stub;
    stub->set(A_foo, myFileRemove);

    bool ret = Utils::deleteDir("/home/lx777/test222");
    stub->reset(A_foo);
    delete stub;
    ASSERT_FALSE(ret);
}

TEST(Utlis_createRandomString_UT, Utlis_createRandomString_UT001)
{
    QString str = Utils::createRandomString();
    ASSERT_STRNE(str.toStdString().c_str(), "");
}

TEST(Utlis_zipPasswordIsChinese_UT, Utlis_zipPasswordIsChinese_UT001)
{
    QMap<QString, QString> temp;
    temp.insert("fixedMimeType", "application/zip");
    temp.insert("encryptionPassword", "123456");
    bool ret = Utils::zipPasswordIsChinese(temp);
    ASSERT_FALSE(ret);
}
TEST(Utlis_zipPasswordIsChinese_UT, Utlis_zipPasswordIsChinese_UT002)
{
    QMap<QString, QString> temp;
    temp.insert("fixedMimeType", "application/zip");
    temp.insert("encryptionPassword", "统信软件");
    bool ret = Utils::zipPasswordIsChinese(temp);
    ASSERT_TRUE(ret);
}

TEST(Utils_existMimeType_UT, Utils_existMimeType_UT001)
{
    bool ret = Utils::existMimeType("");
    ASSERT_FALSE(ret);
}

TEST(Utils_existMimeType_UT, Utils_existMimeType_UT002)
{
    bool ret = Utils::existMimeType("gzip");
    ASSERT_TRUE(ret);
}
QString myreadConf()
{
    return "file_association.file_association_type.gzip:false";
}
TEST(Utils_existMimeType_UT, Utils_existMimeType_UT003)
{
    Stub *stub = new Stub;
    stub->set(ADDR(Utils, readConf), myreadConf);
    bool ret = Utils::existMimeType("gzip");
    stub->reset(ADDR(Utils, readConf));
    delete stub;
    ASSERT_FALSE(ret);
}

TEST(Utils_readConf_UT, Utils_readConf_UT001)
{
    QString str = Utils::readConf();
    std::cout << str.toStdString() << std::endl;
}
class Utils_judgeFileMime_UT_Param
{
public:
    Utils_judgeFileMime_UT_Param(const QString &str, const QString &val)
    {
        file = str;
        returnValue = val;
    }
    QString file;
    QString returnValue;
};

class Utils_judgeFileMime_UT : public ::testing::TestWithParam<Utils_judgeFileMime_UT_Param>
{
};
INSTANTIATE_TEST_SUITE_P(Utils, Utils_judgeFileMime_UT, ::testing::Values(Utils_judgeFileMime_UT_Param("a.7z", "x-7z-compressed"), Utils_judgeFileMime_UT_Param("a.cpio.gz", "x-cpio-compressed"), Utils_judgeFileMime_UT_Param("a.tar.bz2", "x-bzip-compressed-tar"), Utils_judgeFileMime_UT_Param("a.tar.gz", "x-compressed-tar"), Utils_judgeFileMime_UT_Param("a.tar.lz", "x-lzip-compressed-tar"), Utils_judgeFileMime_UT_Param("a.tar.lzma", "x-lzma-compressed-tar"), Utils_judgeFileMime_UT_Param("a.tar.lzo", "x-tzo"), Utils_judgeFileMime_UT_Param("a.tar.xz", "x-xz-compressed-tar"), Utils_judgeFileMime_UT_Param("a.tar.Z", "x-tarz"), Utils_judgeFileMime_UT_Param("a.src.rpm", "x-source-rpm"), Utils_judgeFileMime_UT_Param("a.ar", "x-archive"), Utils_judgeFileMime_UT_Param("a.bcpio", "x-bcpio"), Utils_judgeFileMime_UT_Param("a.bz2", "x-bzip"), Utils_judgeFileMime_UT_Param("a.cpio", "x-cpio"), Utils_judgeFileMime_UT_Param("a.deb", "vnd.debian.binary-package"), Utils_judgeFileMime_UT_Param("a.gz", "gzip"), Utils_judgeFileMime_UT_Param("a.jar", "x-java-archive"), Utils_judgeFileMime_UT_Param("a.lzma", "x-lzma"), Utils_judgeFileMime_UT_Param("a.cab", "vnd.ms-cab-compressed"), Utils_judgeFileMime_UT_Param("a.rar", "vnd.rar"), Utils_judgeFileMime_UT_Param("a.rpm", "x-rpm"), Utils_judgeFileMime_UT_Param("a.sv4cpio", "x-sv4cpio"), Utils_judgeFileMime_UT_Param("a.sc4crc", "x-sc4crc"), Utils_judgeFileMime_UT_Param("a.tar", "x-tar"), Utils_judgeFileMime_UT_Param("a.xar", "x-xar"), Utils_judgeFileMime_UT_Param("a.xz", "x-xz"), Utils_judgeFileMime_UT_Param("a.zip", "zip"), Utils_judgeFileMime_UT_Param("a.iso", "x-cd-image"), Utils_judgeFileMime_UT_Param("a.appimage", "x-iso9660-appimage")));
TEST_P(Utils_judgeFileMime_UT, Utils_judgeFileMime_UT)
{
    Utils_judgeFileMime_UT_Param param = GetParam();
    QString str = Utils::judgeFileMime(param.file);
    // std::cout << param.file .toStdString()<< "             " << param.returnValue.toStdString() << std::endl;
    ASSERT_STREQ(str.toStdString().c_str(), param.returnValue.toStdString().c_str());
}
