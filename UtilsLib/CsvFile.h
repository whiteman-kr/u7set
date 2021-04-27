#pragma once

#include <QObject>

class CsvFile
{
public:
	static QString getCsvString(const QStringList& strings, bool replaceSeparatorsAndQuotes);
	static QString getCsvString(const QVariantList& strings, bool replaceSeparatorsAndQuotes);
};
