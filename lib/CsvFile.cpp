#include "../lib/CsvFile.h"

QString CsvFile::getCsvString(const QStringList& strings)
{
	static const QChar semicolon = ';';
	static const QChar quotes = '"';
	static const QLatin1String doubleQuotes = QLatin1String("\"\"");

	QString result;

	int count = strings.size();

	for (QString s : strings)
	{
		bool externalQuotes = false;

		if (s.contains(semicolon) == true)
		{
			externalQuotes = true;
		}

		if (s.contains(quotes) == true)
		{
			s = s.replace(quotes, doubleQuotes);
			externalQuotes = true;
		}

		if (externalQuotes == true)
		{
			s = quotes + s + quotes;
		}

		result += s;

		if (count > 1)
		{
			result += semicolon;
			count--;
		}
	}

	return result;
}

QString CsvFile::getCsvString(const QVariantList& strings)
{
	QStringList l;

	for (const QVariant& s : strings)
	{
		l << s.toString();
	}

	return getCsvString(l);
}
