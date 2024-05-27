#include "stdafx.h"
#include "ChatSession.h"
#include "Common.h"

extern quint16 g_ListenPort;

CChatSession::CChatSession(QTcpSocket* pSocket, QObject* pParent) : QObject(pParent)
{
	if (pSocket) {
		SetSocket(pSocket);
		m_Url.setScheme("tcp");
		m_Url.setHost(QHostAddress(pSocket->peerAddress().toIPv4Address()).toString());
		
		SendHandShake();
	}
}

CChatSession::CChatSession(const QString& FileName, QObject* pParent)
{
	m_FileOffer = true;
	m_FileName = FileName;
}

void CChatSession::Start(const QUrl& url, const QString& Name)
{
	qDebug() << url.toString();
	m_Url = url;
	m_Name = Name;

	QTcpSocket* pSocket = new QTcpSocket();

	pSocket->connectToHost(m_Url.host(), m_Url.port());

	SetSocket(pSocket);
}


void CChatSession::SetSocket(QTcpSocket* pSocket)
{
	m_pSocket = pSocket;
	m_pSocket->setParent(this);

	connect(m_pSocket, &QTcpSocket::connected, this, &CChatSession::OnConnected);
	connect(m_pSocket, &QTcpSocket::disconnected, this, &CChatSession::OnDisconnected);
	connect(m_pSocket, &QTcpSocket::readyRead, this, &CChatSession::OnReadyRead);
	connect(m_pSocket, &QTcpSocket::bytesWritten, this, &CChatSession::OnBytesWritten);

}

bool CChatSession::SendText(const QString& Text)
{
	m_pSocket->write(Text.toLatin1() + "\n");
	return true;
}

bool CChatSession::SendCtrl(const QString& id, const QString& param)
{
	m_pSocket->write("\\" + id.toLatin1() + " ");
	m_pSocket->write(param.toLatin1() + "\n");
	return true;
}

void CChatSession::OnConnected()
{
	SendHandShake();

	if (!m_FileName.isEmpty())
	{
		quint64 uFileSize = QFileInfo(m_FileName).size();
		SendCtrl(CTRL_MSG_FILE, m_FileName + "|" + QString::number(uFileSize));
	}
	else
	{
		SendCtrl(CTRL_MSG_LIST, "");
		SendCtrl(CTRL_MSG_TEXT, "");
	}
}

void CChatSession::SendHandShake()
{
	SendCtrl(CTRL_MSG_NAME, theConf->value("Chat/Name").toString());
	SendCtrl(CTRL_MSG_PORT, QString::number(g_ListenPort));
}

void CChatSession::OnDisconnected()
{
	if (m_FileOffer)
	{
		if(m_pFile) {
			delete m_pFile;
			m_pFile = NULL;
		}
		if (m_pHash) {
			delete m_pHash;
			m_pHash = NULL;
		}
	}
	emit Closed();
}

void CChatSession::OnReadyRead()
{
	m_Buffer.append(m_pSocket->readAll());

	for (;!m_FileDataPending;)
	{
		int pos = m_Buffer.indexOf('\n');
		if (pos == -1)
			break;

		QByteArray Message = m_Buffer.left(pos);
		m_Buffer.remove(0, pos + 1);

		if (Message.left(1) == "\\")
		{
			auto CtrlMsg = Split2(Message.mid(1), " ");
			OnCtrlMsg(CtrlMsg.first, CtrlMsg.second);
		}
		else
			emit ReceivedText(Message);
	}
	
	if (m_FileDataPending)
	{
		m_pHash->addData(m_Buffer);
		m_pFile->write(m_Buffer);
		m_Buffer.clear();

		if (m_ProgressTimer.elapsed() > 100) {
			m_ProgressTimer.restart();
			emit TransferProgress(m_pFile->pos(), m_uFileSize, m_FileName);
		}

		if (m_pFile->size() == m_uFileSize)
		{
			delete m_pFile;
			m_pFile = NULL;
			m_FileDataPending = false;

			m_FileHash = m_pHash->result().toHex();
			delete m_pHash;
			m_pHash = NULL;

			if (1) 
				SendCtrl(CTRL_MSG_HASH_REQ, "");
			else {
				m_FileCompleted = true;
				SendCtrl(CTRL_MSG_FILE_RES, "ok");
			}
		}
	}
}

void CChatSession::OnCtrlMsg(const QString& id, const QString& param)
{
	bool bInfoChanged = false;
	if (id == CTRL_MSG_NAME) {
		m_Name = param;
		bInfoChanged = true;
	}
	else if (id == CTRL_MSG_PORT) {
		m_Url.setPort(param.toUShort());
		bInfoChanged = true;
	}
	else if (id == CTRL_MSG_LIST) {
		emit ListRequest(m_Url);
	}
	else if (id == CTRL_MSG_TEXT) { 
		emit ChatSession();
		SendCtrl(CTRL_MSG_LIST, "");
	}
	else if (id == CTRL_MSG_FILE) {
		QStringList Params = param.split("|");
		emit FileSession(Params[0], Params.size() > 1 ? Params[1].toULongLong() : 0);
	}
	else if (id == CTRL_MSG_ACCEPT) {
		StartSendFile();
	}
	else if (id == CTRL_MSG_FILE_DATA) {
		if (!m_FileAccepted) {
			Close();
			return;
		}

		QStringList Params = param.split("|");

		m_ProgressTimer.start();
		m_uFileSize = Params[0].toULongLong();
		m_FileDataPending = true;
	}
	else if (id == CTRL_MSG_DECLINE) {
		if (!m_FileOffer) 
			return;
		m_LastError = param.split("|").first();
		Close();
	}
	else if (id == CTRL_MSG_FILE_RES)
	{
		if (!m_FileOffer)
			return;

		QStringList Params = param.split("|");
		if(Params[0] == "ok")
			m_FileCompleted = true;
		else
			m_LastError = Params[0];
		Close();
	}
	else if (id == CTRL_MSG_HASH_REQ)
	{
		SendCtrl(CTRL_MSG_HASH_RES, m_FileHash);
	}
	else if (id == CTRL_MSG_HASH_RES)
	{
		if (!m_FileAccepted) 
			return;

		QStringList Params = param.split("|");

		if (m_FileHash == Params[0])
		{
			m_FileCompleted = true;
			SendCtrl(CTRL_MSG_FILE_RES, "ok");
		}
		else
		{
			SendCtrl(CTRL_MSG_FILE_RES, "bad_hash");
		}
	}

	if (bInfoChanged) {
		emit InfoChanged();
		if(m_Url.port() != -1 && !m_Name.isEmpty())
			emit OnContact(m_Name, m_Url);
	}
}

void CChatSession::DeclineFile()
{
	SendCtrl(CTRL_MSG_DECLINE, "");
}

bool CChatSession::AcceptFile(const QString& FileName)
{
	m_FileName = FileName;
	m_FileAccepted = true;

	m_pFile = new QFile(m_FileName, this);
	m_pHash = new QCryptographicHash(QCryptographicHash::Sha256);
	if (!m_pFile->open(QFile::WriteOnly)) {
		return false;
	}
	SendCtrl(CTRL_MSG_ACCEPT, "");
	return true;
}

void CChatSession::StartSendFile()
{
	m_pFile = new QFile(m_FileName, this);
	m_pHash = new QCryptographicHash(QCryptographicHash::Sha256);
	quint64 uFileSize = m_pFile->size();
	SendCtrl(CTRL_MSG_FILE_DATA, QString::number(uFileSize));
	m_pFile->open(QFile::ReadOnly);

	m_ProgressTimer.start();
	ContinueSendFile();
}


void CChatSession::ContinueSendFile()
{
	for (;;)
	{
		if (m_FileBuffer.isEmpty()) 
		{
			m_FileBuffer.resize(0x1000);
			int Read = m_pFile->read(m_FileBuffer.data(), m_FileBuffer.size());
			if (Read < 0)
				break;
			m_pHash->addData(m_FileBuffer.data(), Read);
			m_FileBuffer.resize(Read);
			if (m_FileBuffer.isEmpty())
				break;
		}
		if(m_pSocket->bytesToWrite() > 0x100000)
			break;
		int Sent = m_pSocket->write(m_FileBuffer);
		if (Sent == m_FileBuffer.size())
			m_FileBuffer.clear();
		else
		{
			if(Sent >= 0)
				m_FileBuffer.remove(0, Sent);
			break;
		}
	}

	if (m_ProgressTimer.elapsed() > 100) {
		m_ProgressTimer.restart();
		qint64 pos = m_pFile->pos();
		qint64 size = m_pFile->size();
		qDebug() << "File Position: " << pos << ", Total Size: " << size;
		emit TransferProgress(pos, size, m_FileName);
	}
}



void CChatSession::OnBytesWritten()
{
	if (m_pFile) {
		ContinueSendFile();

		if (m_FileBuffer.isEmpty()) 
		{
			delete m_pFile;
			m_pFile = NULL;

			m_FileHash = m_pHash->result().toHex();
			delete m_pHash;
			m_pHash = NULL;
		}
	}
}

void CChatSession::Close()
{
	m_pSocket->flush();
	m_pSocket->close();
}

bool CChatSession::IsConnected() const
{
	switch (m_pSocket->state())
	{
	case QAbstractSocket::HostLookupState:
	case QAbstractSocket::ConnectingState:
	case QAbstractSocket::ConnectedState:
		return true;
	case QAbstractSocket::UnconnectedState:
	case QAbstractSocket::ClosingState:
	default:
		return false;
	}
}