#pragma once
#include <qobject.h>
#include <qtcpsocket.h>
#include <QCryptographicHash>

#define CTRL_MSG_NAME		"name"
#define CTRL_MSG_PORT		"port"
#define CTRL_MSG_LIST		"list"
#define CTRL_MSG_TEXT		"text"
#define CTRL_MSG_FILE		"offer"
#define CTRL_MSG_ACCEPT		"accept"
#define CTRL_MSG_DECLINE	"decline"
#define CTRL_MSG_FILE_DATA	"file_data"
#define CTRL_MSG_FILE_END	"file_end"
#define CTRL_MSG_FILE_RES	"file_res"
#define CTRL_MSG_HASH_REQ	"hash_req"
#define CTRL_MSG_HASH_RES	"hash_res"



class CChatSession : public QObject
{
	Q_OBJECT
public:
	CChatSession(QTcpSocket* pSocket = NULL, QObject* pParent = nullptr);
	CChatSession(const QString& FileName, QObject* pParent = nullptr);
	~CChatSession() {}

	void Start(const QUrl& url, const QString& Name = "");

	QString GetName() const {return m_Name;}

	QString GetErrorString() {return m_LastError;}

	bool SendText(const QString& Text);
	bool SendCtrl(const QString& id, const QString& param);

	bool IsConnected() const;

	bool IsFileOffer() const { return m_FileOffer; }
	bool IsFileCompleted() const { return m_FileCompleted; }

	void DeclineFile();
	bool AcceptFile(const QString& FileName);

	void Close();

	QUrl GetUrl() const { return m_Url; }

private slots:
	void OnConnected();
	void OnDisconnected();
	void OnReadyRead();
	void OnBytesWritten();

signals:
	void ChatSession();
	void FileSession(const QString& FileName, quint64 FileSize);
	void InfoChanged();
	void ReceivedText(const QString& Text);
	void Closed();
	void OnContact(const QString& Name, const QUrl& Url);
	void ListRequest(const QUrl& Url);

	void TransferProgress(quint64 pos, quint64 total, const QString& FileName);

protected:
	void SendHandShake();

	void SetSocket(QTcpSocket* pSocket);

	void StartSendFile();
	void ContinueSendFile();

	virtual void OnCtrlMsg(const QString& id, const QString& param);

	QUrl m_Url;
	QString m_Name;

	QString m_FileName;
	bool m_FileOffer = false;
	bool m_FileAccepted = false;
	bool m_FileDataPending = false;
	bool m_FileCompleted = false;
	QString m_LastError;
	quint64 m_uFileSize;
	QTcpSocket* m_pSocket = NULL;
	QByteArray m_Buffer;

	QFile* m_pFile = NULL;
	QCryptographicHash* m_pHash = NULL;
	QByteArray m_FileHash;
	QByteArray m_FileBuffer;
	QElapsedTimer m_ProgressTimer;
};

typedef QSharedPointer<CChatSession> CChatSessionPtr;
typedef QWeakPointer<CChatSession> CChatSessionRef;