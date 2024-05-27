#include "stdafx.h"
#include "Common.h"


QPair<QString, QString> Split2(const QString& str, const QString& sep, bool Reverse)
{
	int pos = Reverse ? str.lastIndexOf(sep) : str.indexOf(sep);
	if (pos == -1)
		pos = str.length();
	return qMakePair(str.left(pos).trimmed(), str.mid(pos + 1).trimmed());
}