#pragma once

#include <QtWidgets/QApplication>
#include <cassert>
#include <cmath>
#include <QDateTime>
#include "../CommonLib/Types.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800)
#endif
#include <QtWidgets/QDesktopWidget>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


class CUtils
{
public:

	static bool processDiagSignalMask(const QString& mask, const QString& str)
	{
		if (mask.isEmpty())
		{
			return true;
		}

		int maskLen = mask.length();
		int strLen = str.length();

		int maskPos = 0;	// текущая позиция маски
		int strPos = 0;		// текущая позиция строки

		while(true)
		{
			if (strPos == strLen && maskPos == maskLen)		// дошли до конца строки и маски строки совпали
			{
				return true;
			}

			if (strPos == strLen)		// дошли до конца строки строки не совпали
			{
				return false;
			}

			if (maskPos == maskLen)		// дошли до конца маски - строки не совпали
			{
				return false;
			}

			QChar m = mask.at(maskPos);

			if (m == '~')				// если в маске встретили тильду - вернуть
			{
				return true;
			}

			if (m == '?')				// ? - пропустить символ и начать сначала
			{
				maskPos++;
				strPos++;
				continue;
			}

			if (m == '*')				//* - пропустить все до первого символа "_"
			{
				while (str.at(strPos) != '_')
				{
					strPos++;

					if (strPos == strLen)		// дошли до конца строки - строки совпали
					{
						if (maskPos == maskLen - 1)
						{
							return true;		//строки совпали если это последняя звездочка
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

			if (str.at(strPos) != m)			// очередные символы не совпали
			{
				return false;
			}

			strPos++;
			maskPos++;
		}

		return true;
	}

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
	}
};



