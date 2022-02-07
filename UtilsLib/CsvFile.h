#pragma once

#include <QObject>

class CsvFile
{
public:
	static QString stringsToCSV(const QStringList& strings, bool replaceSeparatorsAndQuotes = false);
	static QString stringsToCSV(const QVariantList& strings, bool replaceSeparatorsAndQuotes = false);
	static QStringList csvToStrings(const QString& csvSting);
};
