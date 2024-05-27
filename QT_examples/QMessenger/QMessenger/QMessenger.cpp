#include "stdafx.h"
#include "QMessenger.h"
#include "Common.h"
#include "ConfigWnd.h"

QSettings* theConf;
quint16 g_ListenPort = 0;

QMessenger::QMessenger(QWidget* parent)
    : QMainWindow(parent)
{
    //ui.setupUi(this);

    srand(QDateTime::currentDateTime().toMSecsSinceEpoch());

    QString ConfigFile = "Config.ini";

    QStringList args = QApplication::arguments();
    int iPos = args.indexOf("instance");
    if (iPos != -1)
        ConfigFile.prepend(args.at(iPos + 1) + "-");

    ConfigFile.prepend(QApplication::applicationDirPath() + "/");
    theConf = new QSettings(ConfigFile, QSettings::IniFormat);
    if(theConf->value("Chat/Name").toString().isEmpty())
        theConf->setValue("Chat/Name", QString("User %1").arg(rand()));

    m_pChatManager = new CChatManager(this);
    connect(m_pChatManager, &CChatManager::ChatStarted, this, &QMessenger::OnChatStarted);
    connect(m_pChatManager, &CChatManager::ChatEnded, this, &QMessenger::OnChatEnded);
    connect(m_pChatManager, &CChatManager::TransferEnded, this, &QMessenger::OnTransferEnded);

    connect(m_pChatManager, &CChatManager::FileOffered, this, &QMessenger::OnFileOffered);

    m_pContactManager = new CContactManager(this);

    connect(m_pChatManager, &CChatManager::Contact, m_pContactManager, &CContactManager::OnContact);
    connect(m_pChatManager, &CChatManager::OnListRequest, m_pContactManager, &CContactManager::OnListRequest);
    connect(m_pContactManager, &CContactManager::ContactsChanged, this, &QMessenger::OnContactsChanged);

    int Port = 1234;
    for (; Port < 0xffff; Port++) {
        if (m_pChatManager->Listen(Port))
            break;
    }
    if (Port != 0xffff) {
        m_pContactManager->OpenPort(Port);
        g_ListenPort = Port;
        this->setWindowTitle(tr("QMessenger listening on port %1").arg(Port));
    } else
        this->setWindowTitle(tr("QMessenger faild to listen"));

    m_pMenu = menuBar()->addMenu(tr("Menu"));
    m_pNewChat = m_pMenu->addAction(QIcon(":/Resources/NewMsg.png"), tr("New Chat"), this, SLOT(OnNewChat()));
    m_pMenu->addSeparator();
    m_pConfig = m_pMenu->addAction(tr("Config"), this, SLOT(OnShowConfig()));
    m_pMenu->addSeparator();
    m_pExit = m_pMenu->addAction(tr("Exit"), this, SLOT(close()));

    m_pChatMenu = menuBar()->addMenu(tr("Chat"));
    m_pSendFile = m_pChatMenu->addAction(tr("Send File"), this, SLOT(OnSendFile()));
    m_pChatMenu->setEnabled(false);

    m_pMainWidget = new QWidget();
    m_pMainLayout = new QVBoxLayout(m_pMainWidget);
    m_pMainLayout->setContentsMargins(2,2,2,2);
    setCentralWidget(m_pMainWidget);

    m_pToolBar = new QToolBar();
    m_pMainLayout->addWidget(m_pToolBar);

    m_pToolBar->addAction(m_pNewChat);
    m_pToolBar->addSeparator();

    m_pMainSplitter = new QSplitter();
    m_pMainLayout->addWidget(m_pMainSplitter);
    m_pContacts = new QTreeWidget();
    m_pContacts->setHeaderLabels(tr("Name|Address").split("|"));
    m_pMainSplitter->addWidget(m_pContacts);
    m_pChats = new QTabWidget();
    m_pChats->setTabsClosable(true);
    m_pMainSplitter->addWidget(m_pChats);

    // Set the context menu policy and connect the customContextMenuRequested signal to the OnContextMenuRequested slot
    m_pContacts->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pContacts, &QTreeWidget::customContextMenuRequested, this, &QMessenger::OnContextMenuRequested);

    m_pEmpty = new QLabel(tr("empty"));
    //m_pEmpty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pChats->addTab(m_pEmpty, tr("empty"));

    connect(m_pChats, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));
    connect(m_pChats, SIGNAL(tabCloseRequested(int)), this, SLOT(OnCloseRequest(int)));

    statusBar()->showMessage(tr("ready"), 3000);

    this->restoreGeometry(QByteArray::fromBase64(theConf->value("UI/MainWnd").toByteArray()));
    m_pMainSplitter->restoreState(QByteArray::fromBase64(theConf->value("UI/MainSplit").toByteArray()));
}

QMessenger::~QMessenger()
{
    theConf->setValue("UI/MainWnd", QString(this->saveGeometry().toBase64()));
    theConf->setValue("UI/MainSplit", QString(m_pMainSplitter->saveState().toBase64()));
    theConf->sync();
}

void QMessenger::OnNewChat()
{
    QString Target = QInputDialog::getText(this, tr("Start Chat"), tr("Please enter the IP:Port of your ratget."), QLineEdit::Normal, "127.0.0.1:1234");
    if (Target.isEmpty())
        return;

    auto IpPort = Split2(Target, ":", true);

    QUrl url;
    url.setScheme("tcp");
    url.setHost(IpPort.first);
    url.setPort(IpPort.second.toLong());

    m_pChatManager->StartChat(url);
}

void QMessenger::OnShowConfig()
{
    CConfigWnd* wnd = new CConfigWnd(this);
    wnd->show();
}

void QMessenger::OnChatStarted(const CChatSessionPtr& pChat)
{
    if (m_pEmpty) {
        //m_pChats->removeTab()
        delete m_pEmpty;
        m_pEmpty = NULL;
    }

    connect(pChat.data(),SIGNAL(InfoChanged()), this, SLOT(OnInfoChanged()));

    CChatTab* pNewChat = new CChatTab(pChat, this);

    m_pChatMap.insert(pChat.data(), pNewChat);
    m_pChats->addTab(pNewChat, pChat->GetUrl().toString());
}

void QMessenger::OnInfoChanged()
{
    CChatSession* pChat = (CChatSession*)sender();

    CChatTab* pChatTab = m_pChatMap.value(pChat);
    if(!pChatTab) return;

    for (int i = 0; i < m_pChats->count(); i++) 
    {
        if (m_pChats->widget(i) == pChatTab)
        {
            QString sUrl = pChat->GetUrl().toString();
            if(!sUrl.isEmpty())
                m_pChats->setTabText(i, tr("%1 (%2)").arg(pChat->GetName()).arg(pChat->GetUrl().toString()));
            else
                m_pChats->setTabText(i, pChat->GetName());
            break;
        }
    }
}


void QMessenger::OnChatEnded(const CChatSessionPtr& pChat)
{
    OnContactsChanged();
}

void QMessenger::OnTransferEnded(const CChatSessionPtr& pChat)
{
    QLabel* pLabel = m_pLabels.take(pChat.data());
    if(pLabel) pLabel->deleteLater();

    if (!pChat->IsFileCompleted()) {
        QString Error = pChat->GetErrorString();
        if(Error.isEmpty())
            Error = tr("user declined");
        QMessageBox::critical(this, tr("Transfer Failed"), tr("Failed transfer file, reason: %1").arg(Error));
    }
}

void QMessenger::OnContactsChanged()
{
    QMap<QString, QTreeWidgetItem*> OldMap;
    for (int i = 0; i < m_pContacts->topLevelItemCount(); i++) {
        QTreeWidgetItem* pItem = m_pContacts->topLevelItem(i);
        OldMap[pItem->text(0)] = pItem;
    }

    QMap<QString, CContactPtr> Contacts = m_pContactManager->GetContacts();
    foreach(const CContactPtr & pContact, Contacts)
    {
        QTreeWidgetItem* pItem = OldMap.take(pContact->GetName());
        if (!pItem) {
            pItem = new QTreeWidgetItem();
            pItem->setText(0, pContact->GetName());
            m_pContacts->addTopLevelItem(pItem);
        }

        QFont font = pItem->font(0);
        font.setBold(!m_pChatManager->GetChatSession(pContact->GetUrl()).isNull());
        pItem->setFont(0, font);

        pItem->setText(1, pContact->GetUrl().toString());
        pItem->setData(1, Qt::UserRole, pContact->GetUrl());
    }

    foreach(QTreeWidgetItem * pItem, OldMap)
        delete pItem;
}

// Double click functionality on contacts to activate new chat
void QMessenger::OnContactDoubleClicked(QTreeWidgetItem *item, int column)
{
    
    QString name = item->text(0);
    QMap<QString, CContactPtr> contacts = m_pContactManager->GetContacts();

    // check for safety if the entry exists in the list
    if (contacts.contains(name)) {

        CContactPtr contact = contacts.value(name);
        QUrl contactUrl = contact->GetUrl();

        // Check if a chat is already open with this contact
        if (m_pChatManager->GetChatSession(contactUrl)) {

            // If a chat is already open, bring it to the front
            for(int i = 0; i < m_pChats->count(); i++) {
                CChatTab* pChatTab = qobject_cast<CChatTab*>(m_pChats->widget(i));
                if(pChatTab && pChatTab->GetSession()->GetUrl() == contactUrl) {
                    m_pChats->setCurrentIndex(i);
                    break;
                }
            }
        } else {
            // If not, start a new chat with the contact
            m_pChatManager->StartChat(contactUrl);
        }
    }
}

// context menu on the contact
void QMessenger::OnContextMenuRequested(const QPoint &pos)
{
    QMenu contextMenu(this);

    QAction *startChatAction = contextMenu.addAction("Start Chat");
    QAction *removeContactAction = contextMenu.addAction("Remove Contact");

    QAction *selectedAction = contextMenu.exec(m_pContacts->mapToGlobal(pos));

    if (selectedAction == startChatAction) {
        // Start a chat with the selected contact
        OnContactDoubleClicked(m_pContacts->itemAt(pos), 0);
    } else if (selectedAction == removeContactAction) {
        // Remove the selected contact
        QString name = m_pContacts->itemAt(pos)->text(0);
        m_pContactManager->RemoveContact(name);
    } 
}

void QMessenger::OnTabChanged(int Index)
{
    if (Index == -1) {
        m_pChatMenu->setEnabled(false);
        return;
    }

    CChatTab* pNewChat = qobject_cast<CChatTab*>(m_pChats->widget(Index));
    if(!pNewChat)
        return;

    m_pChatMenu->setEnabled(true);
    CChatSessionPtr pSession = pNewChat->GetSession();
    int bConnected = pSession->IsConnected();
    m_pSendFile->setEnabled(bConnected);
}

void QMessenger::OnCloseRequest(int Index)
{
    CChatTab* pNewChat = qobject_cast<CChatTab*>(m_pChats->widget(Index));
    if(!pNewChat)
        return;

    pNewChat->GetSession()->Close();
    m_pChats->removeTab(Index);
    m_pChatMap.remove(pNewChat->GetSession().data());
    pNewChat->deleteLater();
}

void QMessenger::OnSendFile()
{
    QString FileName = QFileDialog::getOpenFileName(this, tr("Send File"));
    if(FileName.isEmpty())
        return;

    CChatTab* pChat = qobject_cast<CChatTab*>(m_pChats->currentWidget());
    if(!pChat)
        return;

    // Start the file transfer and get the file transfer session
    CChatSessionPtr pFileChat = m_pChatManager->SendFile(pChat->GetSession()->GetUrl(), FileName);

    // Show progress dialog for file transfer
    QProgressDialog progressDialog("Sending File", "Cancel", 0, 100, this);
    progressDialog.setWindowTitle("Sending File");
    progressDialog.setWindowModality(Qt::WindowModal);

    QLabel* pLabel = new QLabel();
    progressDialog.setLabel(pLabel);

    // Connect the progress dialog cancellation to close the chat session
    connect(&progressDialog, &QProgressDialog::canceled, pFileChat.data(), &CChatSession::Close);

    // Connect the TransferProgress signal to update the progress bar and label
    connect(pFileChat.data(), &CChatSession::TransferProgress, [&](quint64 pos, quint64 total, const QString& FileName) {
        if (total > 0) { // Ensure Filesize is greater than 0 to avoid division by zero
            progressDialog.setValue(static_cast<int>((pos * 100) / total));
        }
        // Update the label with transfer information
        pLabel->setText(tr("%1 %2 \n%3 \nsent to:%4").arg(Split2(FileName, "/", true).second).arg(pos).arg(total).arg(pFileChat->GetName()));

        if (pos >= total && pFileChat->IsFileCompleted()) {
            // File transfer is complete, close the progress dialog
            progressDialog.close();
        }
        });

    progressDialog.exec();
}

void QMessenger::OnFileOffered(const QString& FileName, quint64 FileSize, const CChatSessionPtr& pChat)
{
    if (QMessageBox::question(this, tr("Receive File"), tr("%3 is trying to send you a file %1 which has a size of %2 bytes. Do you want to receive this file?").arg(FileName).arg(FileSize).arg(pChat->GetName()), QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
        pChat->DeclineFile();

        // Close any active progress dialog if it exists
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            QProgressDialog* progressDialog = qobject_cast<QProgressDialog*>(widget);
            if (progressDialog && progressDialog->windowTitle() == "Sending File") {
                progressDialog->close();
                break;
            }
        }
        return;
    }

    // If user accepts the file transfer
    QString SaveFileName = QFileDialog::getSaveFileName(this, tr("Save File"));
    if (SaveFileName.isEmpty()) {
        pChat->DeclineFile();
        return;
    }

    QLabel* pLabel = new QLabel();
    statusBar()->addWidget(pLabel);
    m_pLabels.insert(pChat.data(), pLabel);
    connect(pChat.data(), &CChatSession::TransferProgress, this, &QMessenger::OnTransferProgress);

    if (!pChat->AcceptFile(SaveFileName)) {
        QMessageBox::critical(this, tr("Receive File"), tr("Failed to open file to save data, transfer aborted"));
    }
}


void QMessenger::OnTransferProgress(quint64 pos, quint64 total, const QString& FileName)
{
    CChatSession* pChat = (CChatSession*)sender();

    QLabel* pLabel = m_pLabels.value(pChat);
    if(!pLabel)
        return;
    pLabel->setText(tr("%1 %2/%3").arg(Split2(FileName, "/", true).second).arg(pos).arg(total));
}

