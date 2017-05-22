#pragma once

#include <QObject>

class CsvFile
{
public:
	static QString getCsvString(const QStringList& strings);
	static QString getCsvString(const QVariantList& strings);
};
