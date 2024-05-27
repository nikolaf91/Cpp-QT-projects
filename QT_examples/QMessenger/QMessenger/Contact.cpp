#include "stdafx.h"
#include "Contact.h"

CContact::CContact(const QString& Name, QObject* pParent) :
	QObject(pParent)
{
	m_Name = Name;
}
