#pragma once
#include <qobject.h>
#include "ChatSession.h"
#include <qtcpserver.h>
#include <qtcpsocket.h>

class CChatManager : public QObject
{
	Q_OBJECT
public:
	CChatManager(QObject* pParent = nullptr);

	CChatSessionPtr GetChatSession(const QUrl& url);

	bool Listen(int Port = 1234);

	CChatSessionPtr StartChat(const QUrl& url);
	CChatSessionPtr SendFile(const QUrl& url, const QString& FileName);

private slots:
	void OnSessionClosed();
	void OnIncommingChat();
	void OnChatSession();
	void OnFileSession(const QString& FileName, quint64 FileSize);

signals:
	void ChatStarted(const CChatSessionPtr& pChat);
	void ChatEnded(const CChatSessionPtr& pChat);
	void TransferEnded(const CChatSessionPtr& pChat);
	void FileOffered(const QString& FileName, quint64 FileSize, const CChatSessionPtr& pChat);
	void Contact(const QString& Name, const QUrl& Url);
	void OnListRequest(const QUrl& Url);

protected:

	QMap<CChatSession*, CChatSessionPtr> m_Sessions;
	QMap<QUrl, CChatSessionPtr> m_ChatSessions;
	QMultiMap<QUrl, CChatSessionPtr> m_FileSessions;

	QTcpServer* m_pServer = NULL;
};

