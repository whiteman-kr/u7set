#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "CsvFile.h"


QString CsvFile::stringsToCSV(const QStringList& strings, bool replaceSeparatorsAndQuotes)
{
	static const QChar semicolon = ';';
	static const QChar comma = ',';
	static const QChar quotes = '"';
	static const QChar singleQuotes = '\'';
	static const QLatin1String singleQuotesStr = QLatin1String("\"");
	static const QLatin1String doubleQuotesStr = QLatin1String("\"\"");

	QString result;

	qsizetype count = strings.size();

	for (int i = 0; i < count; i++)
	{
		QString s = strings[i];

		s.replace('\n', "\\n");

		if (replaceSeparatorsAndQuotes == true)
		{
			s.replace(quotes, singleQuotes);
			s.replace(semicolon, comma);
			result += s;
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
				s.replace(singleQuotesStr, doubleQuotesStr);
				externalQuotes = true;
			}

			if (externalQuotes == true)
			{
				// place the expression to external quotes
				//
				result += quotes + s + quotes;
			}
			else
			{
				result += s;
			}
		}

		if (i != count - 1)
		{
			result += semicolon;
		}
	}

	return result;
}

QString CsvFile::stringsToCSV(const QVariantList& strings, bool replaceSeparatorsAndQuotes)
{
	QStringList l;

	for (const QVariant& s : strings)
	{
		l << s.toString();
	}

	return stringsToCSV(l, replaceSeparatorsAndQuotes);
}

QStringList CsvFile::csvToStrings(const QString& csvSting)
{
	static const QLatin1String singleQuotesStr = QLatin1String("\"");
	static const QLatin1String doubleQuotesStr = QLatin1String("\"\"");

	QStringList result;

	// Regular expression was taken from https://forum.qt.io/topic/119076/qregexp-to-parse-a-csv-file
	//
	const QRegularExpression regExp(R"x((\;|\n|^)(?:"([^"]*(?:""[^"]*)*)"|([^"\;\n]*)))x");

	QRegularExpressionMatchIterator matchIt = regExp.globalMatch(csvSting);
	while (matchIt.hasNext())
	{
		const QRegularExpressionMatch match = matchIt.next();

		QString s = match.capturedTexts().last();
		s.replace("\\n", "\n");
		s.replace(doubleQuotesStr, singleQuotesStr);
		result.push_back(s);
	}

	return result;
}
