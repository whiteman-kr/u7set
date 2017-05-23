#include "../lib/CsvFile.h"

QString CsvFile::getCsvString(const QStringList& strings, bool replaceSeparatorsAndQuotes)
{
	static const QChar semicolon = ';';
	static const QChar comma = ',';
	static const QChar quotes = '"';
	static const QChar singleQuotes = '\'';
	static const QLatin1String doubleQuotes = QLatin1String("\"\"");

	QString result;

	int count = strings.size();

	for (QString s : strings)
	{
		if (replaceSeparatorsAndQuotes == true)
		{
			s = s.replace(quotes, singleQuotes);
			s = s.replace(semicolon, comma);
		}
		else
		{
			bool externalQuotes = false;

			if (s.contains(semicolon) == true)
			{
				externalQuotes = true;
			}

			if (s.contains(quotes) == true)
			{
				// replace quotes to double quotes
				//
				s = s.replace(quotes, doubleQuotes);
				externalQuotes = true;
			}

			if (externalQuotes == true)
			{
				// place the expression to external quotes
				//
				s = quotes + s + quotes;
			}
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

QString CsvFile::getCsvString(const QVariantList& strings, bool replaceSeparatorsAndQuotes)
{
	QStringList l;

	for (const QVariant& s : strings)
	{
		l << s.toString();
	}

	return getCsvString(l, replaceSeparatorsAndQuotes);
}
