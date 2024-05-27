#include "stdafx.h"
#include "ChatManager.h"


CChatManager::CChatManager(QObject* pParent)
	:QObject(pParent)
{
}

bool CChatManager::Listen(int Port)
{
	if (m_pServer)
		return false;
	m_pServer = new QTcpServer();

	connect(m_pServer, &QTcpServer::newConnection, this, &CChatManager::OnIncommingChat);

	if (!m_pServer->listen(QHostAddress::Any, Port)) {
		delete m_pServer;
		m_pServer = NULL;
		return false;
	}
	return true;
}

void CChatManager::OnIncommingChat()
{
	QTcpSocket* pNewSocket = m_pServer->nextPendingConnection();

	CChatSessionPtr pChatSession = CChatSessionPtr(new CChatSession(pNewSocket));

	connect(pChatSession.data(), SIGNAL(Closed()), this, SLOT(OnSessionClosed()));
	connect(pChatSession.data(), SIGNAL(ChatSession()), this, SLOT(OnChatSession()));
	connect(pChatSession.data(), SIGNAL(FileSession(const QString&, quint64)), this, SLOT(OnFileSession(const QString&, quint64)));
	connect(pChatSession.data(), SIGNAL(ListRequest(const QUrl&)), this, SIGNAL(OnListRequest(const QUrl&)));

	m_Sessions.insert(pChatSession.data(), pChatSession);
}

CChatSessionPtr CChatManager::StartChat(const QUrl& url)
{
	CChatSessionPtr pChatSession = CChatSessionPtr(new CChatSession());

	connect(pChatSession.data(), SIGNAL(Closed()), this, SLOT(OnSessionClosed()));
	connect(pChatSession.data(), SIGNAL(ListRequest(const QUrl&)), this, SIGNAL(OnListRequest(const QUrl&)));

	pChatSession->Start(url);

	m_Sessions.insert(pChatSession.data(), pChatSession);
	m_ChatSessions.insert(url, pChatSession);
	emit ChatStarted(pChatSession);

	return pChatSession;
}

CChatSessionPtr CChatManager::SendFile(const QUrl& url, const QString& FileName)
{
	CChatSessionPtr pChatSession = CChatSessionPtr(new CChatSession(FileName));

	connect(pChatSession.data(), SIGNAL(Closed()), this, SLOT(OnSessionClosed()));
	connect(pChatSession.data(), SIGNAL(ListRequest(const QUrl&)), this, SIGNAL(OnListRequest(const QUrl&)));

	pChatSession->Start(url);

	m_Sessions.insert(pChatSession.data(), pChatSession);
	m_FileSessions.insert(url, pChatSession);

	return pChatSession;
}

void CChatManager::OnChatSession()
{
	CChatSession* pSender = qobject_cast<CChatSession*>(sender());
	CChatSessionPtr pChatSession = m_Sessions.value(pSender);

	emit ChatStarted(pChatSession);

	m_ChatSessions.insert(pChatSession->GetUrl(), pChatSession);

	emit Contact(pChatSession->GetName(), pChatSession->GetUrl());
}

void CChatManager::OnFileSession(const QString& FileName, quint64 FileSize)
{
	CChatSession* pSender = qobject_cast<CChatSession*>(sender());
	CChatSessionPtr pChatSession = m_Sessions.value(pSender); 

	emit FileOffered(FileName, FileSize, pChatSession);

	m_FileSessions.insert(pChatSession->GetUrl(), pChatSession);
}

void CChatManager::OnSessionClosed()
{
	CChatSession* pSender = qobject_cast<CChatSession*>(sender());
	CChatSessionPtr pChatSession = m_Sessions.value(pSender);
	if (pChatSession.isNull())
		return;

	m_Sessions.remove(pSender);
	if (m_FileSessions.remove(pChatSession->GetUrl(), pChatSession) > 0)
		emit TransferEnded(pChatSession);
	else if(m_ChatSessions.remove(pChatSession->GetUrl()) > 0)
		emit ChatEnded(pChatSession);

	// Cant use delete later as the smart pointer deletes instantly
	// So making a copy of the smart poitner and let it be released once the timer clicks

	QTimer::singleShot(10, [pChatSession]() {});
}

CChatSessionPtr CChatManager::GetChatSession(const QUrl& url)
{
	return m_ChatSessions.value(url);
}