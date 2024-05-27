#pragma once
#include <qmainwindow.h>
#include "ui_ConfigWnd.h"

class CConfigWnd : public QMainWindow
{
    Q_OBJECT
public:
    CConfigWnd(QWidget* pParent = nullptr);
    ~CConfigWnd();

private slots:
    void OnOk();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:

    Ui::ConfigWndClass ui;
};

