#pragma once
#include <qobject.h>
class CContact :
    public QObject
{
    Q_OBJECT
public:
    CContact(const QString& Name, QObject* pParent = nullptr);

    QString GetName() const {return m_Name;}

    void SetUrl(const QUrl& Url) {m_Url = Url;}
    QUrl GetUrl() const {return m_Url;}


protected:
    QString m_Name;
    QUrl m_Url;
};

typedef QSharedPointer<CContact> CContactPtr;