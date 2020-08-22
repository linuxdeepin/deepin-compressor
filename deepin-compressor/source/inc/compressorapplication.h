#pragma once

#include <DApplication>

DWIDGET_USE_NAMESPACE

class MainWindow;

class CompressorApplication: public DApplication
{
    Q_OBJECT
public:
    CompressorApplication(int &argc, char **argv);
    void setMainWindow(MainWindow *wd);

private:
    void handleQuitAction() Q_DECL_OVERRIDE;

    MainWindow *mainWindow_ = nullptr;

public:
    bool notify(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
};
