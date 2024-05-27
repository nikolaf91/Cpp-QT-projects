#pragma once
#include <qobject.h>
#include <qudpsocket.h>
#include "Contact.h"

class CContactManager :
    public QObject
{
    Q_OBJECT
public:
    CContactManager(QObject* pParent = nullptr);

    bool OpenPort(quint16 port);

    QMap<QString, CContactPtr> GetContacts() const { return m_Contacts; }
    void RemoveContact(const QString& name);

private slots:
    void OnReadyRead();

public slots:
    void OnContact(const QString& Name, const QUrl& Url);
    void OnListRequest(const QUrl& Url);

signals:
    void ContactsChanged();

protected:

    void BroadcastData(const QByteArray& Data, const QList<QUrl>& SkipUrls = QList<QUrl>());

    bool TryAddOrUpdate(const QString& Name, const QUrl& Url);

    QMap<QString, CContactPtr> m_Contacts;

    QUdpSocket* m_pSocket = NULL;
};

