#pragma once

#include <QtWidgets/QMainWindow>
#include <qsplitter.h>
#include <qtreewidget.h>
#include <qgridlayout.h>
#include <qtabwidget.h>
#include "ChatManager.h"
#include "ChatTab.h"
#include "ContactManager.h"
#include <qsettings.h>


class QMessenger : public QMainWindow
{
    Q_OBJECT

public:
    QMessenger(QWidget *parent = nullptr);
    ~QMessenger();

private slots:

    void            OnSendFile();

    void            OnNewChat();

    void            OnShowConfig();
    void            OnInfoChanged();


    void            OnChatStarted(const CChatSessionPtr& pChat);
    void            OnChatEnded(const CChatSessionPtr& pChat);
    void            OnTransferEnded(const CChatSessionPtr& pChat);

    void            OnContactsChanged();

    void            OnContactDoubleClicked(QTreeWidgetItem* item, int column);
    void            OnContextMenuRequested(const QPoint& pos);

    void            OnTabChanged(int Index);
    void            OnCloseRequest(int Index);

    void            OnFileOffered(const QString& FileName, quint64 FileSize, const CChatSessionPtr& pChat);

    void            OnTransferProgress(quint64 pos, quint64 total, const QString& FileName);


private:
    //Ui::QMessagerClass ui;

    QWidget*        m_pMainWidget;
    QToolBar*       m_pToolBar;
    QVBoxLayout*    m_pMainLayout;
    QSplitter*      m_pMainSplitter;
    QTreeWidget*    m_pContacts;
    QTabWidget*     m_pChats;

    QLabel*         m_pEmpty;

    QMenu*          m_pMenu;
    QAction*        m_pNewChat;
    QAction*        m_pConfig;
    QAction*        m_pExit;

    QMenu*          m_pChatMenu;
    QAction*        m_pSendFile;
    QAction*        m_pReconnect;
    QAction*        m_pDisconnect;

protected:
    CChatManager*   m_pChatManager;
    CContactManager* m_pContactManager;

    QMap<CChatSession*, CChatTab*> m_pChatMap;

    QMap<CChatSession*, QLabel*> m_pLabels;
};
