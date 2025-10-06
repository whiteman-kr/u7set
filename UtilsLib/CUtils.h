#pragma once

#include <QtWidgets/QApplication>
#include <cassert>
#include <cmath>
#include <QDateTime>


class CUtils
{
public:

	static bool processDiagSignalMask(const QString& mask, const QString& str)
	{
		if (mask.isEmpty())
		{
			return true;
		}

		int maskLen = static_cast<int>(mask.length());
		int strLen = static_cast<int>(str.length());

		int maskPos = 0;	// Curent mask position
		int strPos = 0;		// Currner row position

		while(true)
		{
			if (strPos == strLen && maskPos == maskLen)		// Came to the end of row and mask, matched rows
			{
				return true;
			}

			if (strPos == strLen)		// Came to the end of the row and raws are not mathcing
			{
				return false;
			}

			if (maskPos == maskLen)		// Came to the end of the mask - rows are not matching
			{
				return false;
			}

			QChar m = mask.at(maskPos);

			if (m == '~')				// If '~' in the mask - return
			{
				return true;
			}

			if (m == '?')				// ? - skip char and start from the begining
			{
				maskPos++;
				strPos++;
				continue;
			}

			if (m == '*')				// * - skip everything to "_"
			{
				while (str.at(strPos) != '_')
				{
					strPos++;

					if (strPos == strLen)		// Came to the end - raws matched
					{
						if (maskPos == maskLen - 1)
						{
							return true;		// Rows are matched and this is the last *
						}
						else
						{
							return false;
						}
					}
				}

				maskPos++;
				continue;
			}

			if (str.at(strPos) != m)			// Again chars are not matched
			{
				return false;
			}

			strPos++;
			maskPos++;
		}

		return true;
	}
	/*
	static QString dateTimeToStringTime(const QDateTime& dt, bool milliseconds)
	{
		QTime tm = dt.time();
		QString result;

		if (milliseconds == true)
		{
			result = QString("%1:%2:%3.%4")
						.arg(tm.hour(), 2, 10, QChar('0'))
						.arg(tm.minute(), 2, 10, QChar('0'))
						.arg(tm.second(), 2, 10, QChar('0'))
						.arg(tm.msec(), 3, 10, QChar('0'));
		}
		else
		{
			result = QString("%1:%2:%3")
						.arg(tm.hour(), 2, 10, QChar('0'))
						.arg(tm.minute(), 2, 10, QChar('0'))
						.arg(tm.second(), 2, 10, QChar('0'));
		}

		return result;
	}

	static QString dateTimeToStringDate(const QDateTime& dt)
	{
		QDate date = dt.date();
		QString result;

		result = QString("%1.%2.%3")
					.arg(date.day(), 2, 10, QChar('0'))
					.arg(date.month(), 2, 10, QChar('0'))
					.arg(date.year(), 4, 10, QChar('0'));

		return result;
	}*/
};



