#-------------------------------------------------
#
# Project created by QtCreator 2020-06-02T15:39:54
#
#-------------------------------------------------

QT       -= gui

TARGET = ChardetDetector
TEMPLATE = lib

DEFINES += CHARDETDETECTOR_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    #tables/LangArabicModel.cpp \
    tables/LangBulgarianModel.cpp \
    tables/LangCyrillicModel.cpp \
    #tables/LangDanishModel.cpp \
    #tables/LangEsperantoModel.cpp \
    #tables/LangFrenchModel.cpp \
    #tables/LangGermanModel.cpp \
    tables/LangGreekModel.cpp \
    tables/LangHebrewModel.cpp \
    tables/LangHungarianModel.cpp \
    #tables/LangSpanishModel.cpp \
    tables/LangThaiModel.cpp \
    #tables/LangTurkishModel.cpp \
    #tables/LangVietnameseModel.cpp \
    chardet.cpp \
    CharDistribution.cpp \
    JpCntx.cpp \
    nsBig5Prober.cpp \
    nsCharSetProber.cpp \
    nsEscCharsetProber.cpp \
    nsEscSM.cpp \
    nsEUCJPProber.cpp \
    nsEUCKRProber.cpp \
    nsEUCTWProber.cpp \
    nsGB2312Prober.cpp \
    nsHebrewProber.cpp \
    nsLatin1Prober.cpp \
    nsMBCSGroupProber.cpp \
    nsMBCSSM.cpp \
    nsSBCharSetProber.cpp \
    nsSBCSGroupProber.cpp \
    nsSJISProber.cpp \
    nsUniversalDetector.cpp \
    nsUTF8Prober.cpp

HEADERS += \
    chardet.h \
    CharDistribution.h \
    JpCntx.h \
    nsBig5Prober.h \
    nsCharSetProber.h \
    nsCodingStateMachine.h \
    nsEscCharsetProber.h \
    nsEUCJPProber.h \
    nsEUCKRProber.h \
    nsEUCTWProber.h \
    nsGB2312Prober.h \
    nsHebrewProber.h \
    nsLatin1Prober.h \
    nsMBCSGroupProber.h \
    nsPkgInt.h \
    nsSBCharSetProber.h \
    nsSBCSGroupProber.h \
    nsSJISProber.h \
    nsUniversalDetector.h \
    nsUTF8Prober.h \
    version.h \
    prmem.h \
    nscore.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
