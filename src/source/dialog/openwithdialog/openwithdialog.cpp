// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog.h"
#include "dmimedatabase.h"
#include "processopenthread.h"
#include "properties.h"

#include <QStandardPaths>
#include <QDir>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScroller>
#include <QCheckBox>
#include <QFileDialog>
#include <QPainterPath>

OpenWithDialogListItem::OpenWithDialogListItem(const QIcon &icon, const QString &text, QWidget *parent)
    : QWidget(parent)
    , m_icon(icon)
{
    if (m_icon.isNull())
        m_icon = QIcon::fromTheme("application-x-desktop");

    m_pCheckBtn = new DIconButton(this);
    m_pCheckBtn->setFixedSize(10, 10);
    m_pCheckBtn->setFlat(true);

    m_pTextLbl = new DLabel(this);
    m_pTextLbl->setText(text);

    m_pIconLbl = new DLabel(this);
    m_pIconLbl->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(m_pCheckBtn);
    layout->addWidget(m_pIconLbl);
    layout->addWidget(m_pTextLbl);

    setMouseTracking(true);
}

OpenWithDialogListItem::~OpenWithDialogListItem()
{

}

void OpenWithDialogListItem::setChecked(bool checked)
{
    if (checked) {
        m_pCheckBtn->setIcon(DStyle::SP_MarkElement);
    } else {
        m_pCheckBtn->setIcon(QIcon());
    }
}

QString OpenWithDialogListItem::text() const
{
    return m_pTextLbl->text();
}

void OpenWithDialogListItem::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    m_pIconLbl->setFixedSize(e->size().height() - 20, e->size().height() - 20);
    m_pIconLbl->setPixmap(m_icon.pixmap(m_pIconLbl->size()));
}

void OpenWithDialogListItem::enterEvent(EnterEvent *e)
{
    Q_UNUSED(e)

    update();

    return QWidget::enterEvent(e);
}

void OpenWithDialogListItem::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)

    update();

    return QWidget::leaveEvent(e);
}

void OpenWithDialogListItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    if (!underMouse())
        return;

    QPainter pa(this);
    QPainterPath path;

    path.addRoundedRect(rect(), 6, 6);
    pa.setRenderHint(QPainter::Antialiasing);
    pa.fillPath(path, QColor(0, 0, 0, 13));
}





OpenWithDialogListSparerItem::OpenWithDialogListSparerItem(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_separator(new DHorizontalLine(this))
    , m_title(new QLabel(title, this))
{
    QFont font;
    font.setPixelSize(18);
    m_title->setFont(font);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(m_separator);
    layout->addWidget(m_title);
    layout->setContentsMargins(20, 0, 20, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

OpenWithDialogListSparerItem::~OpenWithDialogListSparerItem()
{

}





OpenWithDialog::OpenWithDialog(const QString &strFileName, QWidget *parent)
    : DAbstractDialog(parent)
    , m_strFileName(strFileName)
{
    qDebug() << "Creating OpenWithDialog for file:" << strFileName;
    m_showType = OpenType;
    setWindowFlags(windowFlags()
                   & ~ Qt::WindowMaximizeButtonHint
                   & ~ Qt::WindowMinimizeButtonHint
                   & ~ Qt::WindowSystemMenuHint);
    init();
    initConnections();
    initData();
    qDebug() << "OpenWithDialog initialized";
}

OpenWithDialog::~OpenWithDialog()
{
    qDebug() << "Destroying OpenWithDialog";
}

QList<DesktopFile> OpenWithDialog::getOpenStyle(const QString &strFileName)
{
    mimeAppsManager->initMimeTypeApps();
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(strFileName);
    const QStringList &recommendApps = mimeAppsManager->getRecommendedAppsByQio(mimeType);

    QList<DesktopFile> listType;;

    for (int i = 0; i < recommendApps.count(); ++i) {
        const DesktopFile &desktop_info = mimeAppsManager->DesktopObjs.value(recommendApps.at(i));

//        if (desktop_info.getFileName().contains("deepin-compressor")) {
//            continue;
//        }

        listType << desktop_info;
    }

    return listType;
}

void OpenWithDialog::resizeEvent(QResizeEvent *event)
{
    m_titlebar->setFixedWidth(event->size().width());

    DAbstractDialog::resizeEvent(event);
}

bool OpenWithDialog::eventFilter(QObject *obj, QEvent *event)
{
    // blumia: for m_scrollArea, to avoid touch screen scrolling cause window move
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonPress) {

        if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
            if (OpenWithDialogListItem *item = qobject_cast<OpenWithDialogListItem *>(obj))
                checkItem(item);

            return true;
        }
    }

    return false;
}

void OpenWithDialog::openWithProgram(const QString &strFileName, const QString &strExec)
{
    qDebug() << "Opening file:" << strFileName << "with program:" << strExec;
    // 在线程中执行外部应用打开的命令
    ProcessOpenThread *p = new ProcessOpenThread;
    p->setProgramPath(getProgramPathByExec(strExec));
    p->setArguments(QStringList() << strFileName);
    p->start();
    qDebug() << "Started process thread for opening file";
}

QString OpenWithDialog::showOpenWithDialog(OpenWithDialog::ShowType eType)
{
    qDebug() << "Showing OpenWithDialog with type:" << eType;
    exec();

    // 若未点击确定，不操作
    if (!m_bOk) {
        qDebug() << "Dialog cancelled, no action taken";
        return "";
    }
    const QString &app = m_pCheckedItem->property("app").toString();

    if (m_pSetToDefaultCheckBox->isChecked()) {
        qDebug() << "Setting default app for MIME type:" << m_mimeType.name() << "to:" << app;
        mimeAppsManager->setDefautlAppForTypeByGio(m_mimeType.name(), app);
    }

    if (eType == OpenType) {
        qDebug() << "Opening file with selected app";
        openWithProgram(m_strFileName, m_strAppExecName);
    }

    qDebug() << "Returning app exec name:" << m_strAppExecName;
    return m_strAppExecName;
}

QString OpenWithDialog::getProgramPathByExec(const QString &strExec)
{
    QString strProgramPath;

    if (strExec.isEmpty()) {
        // 应用路径为空时，使用默认应用程序
        strProgramPath = QStandardPaths::findExecutable("xdg-open"); //查询本地位置
    } else {
        // 以/开头时，做字符串分割处理
        QStringList list = strExec.split(" ");
        if (strExec.startsWith(QDir::separator())) {
            strProgramPath = list[0];
        } else {
            strProgramPath = QStandardPaths::findExecutable(list[0]); //查询本地位置
        }
    }

    return strProgramPath;
}

void OpenWithDialog::init()
{
    qDebug() << "Initializing OpenWithDialog UI";
    m_titlebar = new DTitlebar(this);
    m_titlebar->setBackgroundTransparent(true);

    setFixedSize(710, 450);
    m_titlebar->setTitle(tr("Open with"));
    qDebug() << "Dialog size set to 710x450";

    m_pScrollArea = new QScrollArea(this);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setWidgetResizable(true);
    QScroller::grabGesture(m_pScrollArea);
    m_pScrollArea->installEventFilter(this);
    m_pScrollArea->viewport()->setStyleSheet("background-color:transparent;"); //设置滚动区域与主窗体颜色一致

    QWidget *content_widget = new QWidget;
    content_widget->setObjectName("contentWidget");
    content_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pScrollArea->setWidget(content_widget);

    m_pRecommandLayout = new DFlowLayout;
    m_pOtherLayout = new DFlowLayout;
    qDebug() << "Created recommended and other apps layouts";

    m_pOpenFileChooseButton = new QCommandLinkButton(tr("Add other programs"));
    m_pSetToDefaultCheckBox = new QCheckBox(tr("Set as default"));
    m_pSetToDefaultCheckBox->setChecked(true);
    m_pCancelButton = new QPushButton(tr("Cancel", "button"));
    m_pChooseButton = new QPushButton(tr("Confirm", "button"));
    qDebug() << "Created all buttons and controls";

    QVBoxLayout *content_layout = new QVBoxLayout;
    content_layout->setContentsMargins(10, 0, 10, 0);
    content_layout->addWidget(new OpenWithDialogListSparerItem(tr("Recommended Applications"), this));
    content_layout->addLayout(m_pRecommandLayout);
    content_layout->addWidget(new OpenWithDialogListSparerItem(tr("Other Applications"), this));
    content_layout->addLayout(m_pOtherLayout);
    content_layout->addStretch();
    content_widget->setLayout(content_layout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_pOpenFileChooseButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_pSetToDefaultCheckBox);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(m_pCancelButton);
    buttonLayout->addWidget(m_pChooseButton);
    buttonLayout->setContentsMargins(10, 0, 10, 0);

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    QVBoxLayout *bottom_layout = new QVBoxLayout;
    bottom_layout->addWidget(new DHorizontalLine(this));
    bottom_layout->addSpacing(5);
    bottom_layout->addLayout(buttonLayout);
    bottom_layout->setContentsMargins(10, 0, 10, 0);

    main_layout->addWidget(m_pScrollArea);
    main_layout->addLayout(bottom_layout);
    main_layout->setContentsMargins(0, 35, 0, 10);
    qDebug() << "OpenWithDialog UI initialization completed";
}

void OpenWithDialog::initConnections()
{
    connect(m_pCancelButton, &QPushButton::clicked, this, &OpenWithDialog::close);
    connect(m_pChooseButton, &QPushButton::clicked, this, &OpenWithDialog::slotOpenFileByApp);
    connect(m_pOpenFileChooseButton, &QCommandLinkButton::clicked, this, &OpenWithDialog::slotUseOtherApplication);
}

void OpenWithDialog::initData()
{
    qDebug() << "Initializing OpenWithDialog data for file:" << m_strFileName;
    QString mineTypeStr(m_strFileName);

    DMimeDatabase db;
    m_mimeType = db.mimeTypeForFile(mineTypeStr);
    qDebug() << "Detected MIME type:" << m_mimeType.name();

    mimeAppsManager->initMimeTypeApps();
    const QString &default_app = mimeAppsManager->getDefaultAppByMimeType(m_mimeType);
    const QStringList &recommendApps = mimeAppsManager->getRecommendedAppsByQio(m_mimeType);

    qDebug() << "Found" << recommendApps.size() << "recommended apps";
    qDebug() << "Default app:" << (default_app.isEmpty() ? "None" : default_app);

    for (int i = 0; i < recommendApps.count(); ++i) {
        const DesktopFile &desktop_info = mimeAppsManager->DesktopObjs.value(recommendApps.at(i));
        qDebug() << "Adding recommended app:" << desktop_info.getDisplayName();

        OpenWithDialogListItem *item = createItem(QIcon::fromTheme(desktop_info.getIcon()), desktop_info.getDisplayName(), recommendApps.at(i), desktop_info.getExec());
        m_pRecommandLayout->addWidget(item);

        if (!default_app.isEmpty() && recommendApps.at(i).endsWith(default_app)) {
            qDebug() << "Setting default app as checked:" << desktop_info.getDisplayName();
            checkItem(item);
        }
    }

    QList<DesktopFile> other_app_list;
    qDebug() << "Processing other available applications";

    foreach (const QString &f, mimeAppsManager->DesktopObjs.keys()) {
        const DesktopFile &app = mimeAppsManager->DesktopObjs.value(f);
        if (recommendApps.contains(f))
            continue;

        if (mimeAppsManager->DesktopObjs.value(f).getNoShow()) {
            qDebug() << "Skipping hidden app:" << f;
            continue;
        }

        if (mimeAppsManager->DesktopObjs.value(f).getMimeType().isEmpty()) {
            qDebug() << "Skipping app with no MIME types:" << f;
            continue;
        }

        bool isSameDesktop = false;
        foreach (const DesktopFile &otherApp, other_app_list) {
            if (otherApp.getExec() == app.getExec() && otherApp.getLocalName() == app.getLocalName())
                isSameDesktop = true;
        }

        Properties desktop_info(f, "Desktop Entry");
        const QString &custom_open_desktop = desktop_info.value("X-DDE-File-Manager-Custom-Open").toString();

        if (!custom_open_desktop.isEmpty() && custom_open_desktop != m_mimeType.name()) {
            qDebug() << "Skipping app with custom open for different MIME type:" << f;
            continue;
        }

        if (isSameDesktop) {
            qDebug() << "Skipping duplicate app:" << f;
            continue;
        }

        other_app_list << mimeAppsManager->DesktopObjs.value(f);
        QString iconName = other_app_list.last().getIcon();
        OpenWithDialogListItem *item = createItem(QIcon::fromTheme(iconName), other_app_list.last().getDisplayName(), f, app.getExec());
        m_pOtherLayout->addWidget(item);
        qDebug() << "Added other app:" << other_app_list.last().getDisplayName();

        if (!default_app.isEmpty() && f.endsWith(default_app)) {
            qDebug() << "Setting default app as checked:" << other_app_list.last().getDisplayName();
            checkItem(item);
        }
    }
    qDebug() << "OpenWithDialog data initialization completed";
}

void OpenWithDialog::checkItem(OpenWithDialogListItem *item)
{
    if (m_pCheckedItem) {
        qDebug() << "Unchecking previous item:" << m_pCheckedItem->text();
        m_pCheckedItem->setChecked(false);
    }

    item->setChecked(true);
    m_pCheckedItem = item;
    qDebug() << "Checked item:" << item->text() << "app:" << item->property("app").toString();
}

OpenWithDialogListItem *OpenWithDialog::createItem(const QIcon &icon, const QString &name, const QString &filePath, const QString &strExec)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem(icon, name, this);

    item->setProperty("app", filePath);
    item->setProperty("exec", strExec);
    item->setFixedSize(220, 50);
    item->installEventFilter(this);

    return item;
}

void OpenWithDialog::slotUseOtherApplication()
{
    qDebug() << "User clicked 'Add other programs'";
    const QString &file_path = QFileDialog::getOpenFileName(this);

    if (file_path.isEmpty()) {
        qDebug() << "No file selected in file dialog";
        return;
    }

    QFileInfo info(file_path);
    QString target_desktop_file_name("%1/%2-custom-open-%3.desktop");

    target_desktop_file_name = target_desktop_file_name.arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).arg(qApp->applicationName()).arg(m_mimeType.name().replace("/", "-"));

    if (file_path.endsWith(".desktop")) {
        for (const OpenWithDialog *w : m_pRecommandLayout->parentWidget()->findChildren<OpenWithDialog *>()) {
            if (w->property("app").toString() == file_path)
                return;
        }

        Properties desktop(file_path, "Desktop Entry");

        if (desktop.value("MimeType").toString().isEmpty())
            return;

        if (!QFile::link(file_path, target_desktop_file_name))
            return;
    } else if (info.isExecutable()) {
        Properties desktop;

        desktop.set("Type", "Application");
        desktop.set("Name", info.fileName());
        desktop.set("Icon", "application-x-desktop");
        desktop.set("Exec", file_path);
        desktop.set("MimeType", "*/*");
        desktop.set("X-DDE-File-Manager-Custom-Open", m_mimeType.name());

        if (QFile::exists(target_desktop_file_name))
            QFile(target_desktop_file_name).remove();

        if (!desktop.save(target_desktop_file_name, "Desktop Entry"))
            return;
    }

    // remove old custom item
    for (int i = 0; i < m_pOtherLayout->count(); ++i) {
        QWidget *w = m_pOtherLayout->itemAt(i)->widget();

        if (!w)
            continue;

        if (w->property("app").toString() == target_desktop_file_name) {
            m_pOtherLayout->removeWidget(w);
            w->deleteLater();
        }
    }

    OpenWithDialogListItem *item = createItem(QIcon::fromTheme("application-x-desktop"), info.fileName(), target_desktop_file_name, "");

    int other_layout_sizeHint_height = m_pOtherLayout->sizeHint().height();
    m_pOtherLayout->addWidget(item);
    item->show();
    m_pOtherLayout->parentWidget()->setFixedHeight(m_pOtherLayout->parentWidget()->height() + m_pOtherLayout->sizeHint().height() - other_layout_sizeHint_height);
    checkItem(item);
}

void OpenWithDialog::slotOpenFileByApp()
{
    if (!m_pCheckedItem) {
        qWarning() << "No item selected for opening file";
        return;
    }

    qDebug() << "User confirmed opening with app:" << m_pCheckedItem->property("exec").toString();
    m_bOk = true;
    m_strAppExecName = m_pCheckedItem->property("exec").toString();
    close();
}
