#include "stdafx.h"
#include "ContactManager.h"
#include "Common.h"

CContactManager::CContactManager(QObject* pParent) :
	QObject(pParent)
{
}

void CContactManager::OnContact(const QString& Name, const QUrl& Url)
{
	QString sUrl = Url.toString();

	if (TryAddOrUpdate(Name, Url)) {
		QString Data = "contact:" + Name + "\n" + Url.toString();
		BroadcastData(Data.toUtf8(), QList<QUrl>() << Url);

		emit ContactsChanged();
	}
}

void CContactManager::OnListRequest(const QUrl& Url)
{
	foreach(const CContactPtr& pContact, m_Contacts)
	{
		if(Url == pContact->GetUrl())
			continue;
		QString Data = "contact:" + pContact->GetName() + "\n" + pContact->GetUrl().toString();
		m_pSocket->writeDatagram(Data.toUtf8(), QHostAddress(Url.host()), Url.port());
	}
}

bool CContactManager::TryAddOrUpdate(const QString& Name, const QUrl& Url)
{
	CContactPtr pContact = m_Contacts.value(Name);
	if (pContact.isNull() || pContact->GetUrl() != Url)
	{		
		if (pContact.isNull()) {
			pContact = CContactPtr(new CContact(Name));
			m_Contacts.insert(Name, pContact);
		}
		pContact->SetUrl(Url);
		return true;
	}
	return false;
}

void CContactManager::BroadcastData(const QByteArray& Data, const QList<QUrl>& SkipUrls)
{
	foreach(const CContactPtr& pContact, m_Contacts)
	{
		QUrl Url = pContact->GetUrl();
		if(SkipUrls.contains(Url))
			continue;
		m_pSocket->writeDatagram(Data, QHostAddress(Url.host()), Url.port());
	}
}

bool CContactManager::OpenPort(quint16 port)
{
	if(m_pSocket)
		return false;
	m_pSocket = new QUdpSocket(this);
	if (!m_pSocket->bind(port))
	{
		delete m_pSocket;
		m_pSocket = NULL;
		return false;
	}
	connect(m_pSocket, &QUdpSocket::readyRead, this, &CContactManager::OnReadyRead);
	return true;
}

void CContactManager::OnReadyRead()
{
	QByteArray buffer;
	buffer.resize(m_pSocket->pendingDatagramSize());

	QHostAddress sender;
	quint16 senderPort;

	m_pSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

	QStringList Data = QString::fromUtf8(buffer).split("\n");
	auto CmdName = Split2(Data[0],":");
	if (CmdName.first == "contact" && Data.size() >= 2)
	{
		QString sUrl = Data[1];
		QUrl Url = sUrl;
		if (TryAddOrUpdate(CmdName.second, Url)) {
			QUrl SkipUrl;
			SkipUrl.setScheme("tcp");
			SkipUrl.setHost(QHostAddress(sender.toIPv4Address()).toString());
			SkipUrl.setPort(senderPort);
			BroadcastData(buffer, QList<QUrl>() << Url << SkipUrl);
		}
	}

	emit ContactsChanged();
}

void CContactManager::RemoveContact(const QString& name)
{
	if (m_Contacts.contains(name)) {
		m_Contacts.remove(name);
		emit ContactsChanged();
	}
}