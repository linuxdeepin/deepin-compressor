#ifndef ACCESSIBLE_H
#define ACCESSIBLE_H

#include "accessibledefine.h"

#include "mainwindow.h"
#include "homepage.h"
#include "uncompresspage.h"
#include "compresspage.h"
#include "compresssetting.h"
#include "compressor_success.h"
#include "compressor_fail.h"
#include "progressdialog.h"
#include "encryptionpage.h"
#include "openloadingpage.h"
#include "settingdialog.h"
#include "customwidget.h"

#include <DFrame>

DWIDGET_USE_NAMESPACE
//using namespace DCC_NAMESPACE;

SET_FORM_ACCESSIBLE(HomePage, "HomePage")
SET_FORM_ACCESSIBLE(CompressPage, "CompressPage")
SET_FORM_ACCESSIBLE(CompressSetting, "CompressSetting")
SET_FORM_ACCESSIBLE(Compressor_Success, "Compressor_Success")
SET_FORM_ACCESSIBLE(Compressor_Fail, "Compressor_Fail")
SET_FORM_ACCESSIBLE(UnCompressPage, "UnCompressPage")
SET_FORM_ACCESSIBLE(ProgressDialog, "ProgressDialog")
SET_FORM_ACCESSIBLE(MainWindow, "MainWindow")
SET_FORM_ACCESSIBLE(EncryptionPage, "EncryptionPage")
SET_FORM_ACCESSIBLE(OpenLoadingPage, "OpenLoadingPage")
SET_FORM_ACCESSIBLE(Progress, "Progress")
SET_FORM_ACCESSIBLE(SettingDialog, "SettingDialog")

// Qt控件
SET_FORM_ACCESSIBLE(QFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton, m_w->text().isEmpty() ? "qpushbutton" : m_w->text())
SET_SLIDER_ACCESSIBLE(QSlider, "qslider")
SET_FORM_ACCESSIBLE(QMenu, "qmenu")

// Dtk控件
SET_FORM_ACCESSIBLE(DFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(DWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DSwitchButton, m_w->text().isEmpty() ? "switchbutton" : m_w->text())
SET_BUTTON_ACCESSIBLE(DPushButton, m_w->objectName().isEmpty() ? "DPushButton" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DIconButton, m_w->objectName().isEmpty() ? "DIconButton" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DCheckBox, m_w->objectName().isEmpty() ? "DCheckBox" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DCommandLinkButton, "DCommandLinkButton")
SET_FORM_ACCESSIBLE(DTitlebar, m_w->objectName().isEmpty() ? "DTitlebar" : m_w->objectName())
SET_FORM_ACCESSIBLE(DDialog, m_w->objectName().isEmpty() ? "DDialog" : m_w->objectName())
SET_FORM_ACCESSIBLE(DFileDialog, m_w->objectName().isEmpty() ? "DFileDialog" : m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    // 应用主窗口
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), HomePage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CompressPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CompressSetting);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), Compressor_Success);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), Compressor_Fail);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), UnCompressPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ProgressDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MainWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), EncryptionPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), OpenLoadingPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), Progress);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), SettingDialog);

    //      自定义控件
    //    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CustomPushButton);

    //  Qt 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QSlider);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QMenu);

    //  Dtk 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DSwitchButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DIconButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DCheckBox);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DCommandLinkButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DTitlebar);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DFileDialog);

    return interface;
}

#endif // ACCESSIBLE_H
