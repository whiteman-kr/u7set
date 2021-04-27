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

		int maskPos = 0;	// ������� ������� �����
		int strPos = 0;		// ������� ������� ������

		while(true)
		{
			if (strPos == strLen && maskPos == maskLen)		// ����� �� ����� ������ � ����� ������ �������
			{
				return true;
			}

			if (strPos == strLen)		// ����� �� ����� ������ ������ �� �������
			{
				return false;
			}

			if (maskPos == maskLen)		// ����� �� ����� ����� - ������ �� �������
			{
				return false;
			}

			QChar m = mask.at(maskPos);

			if (m == '~')				// ���� � ����� ��������� ������ - �������
			{
				return true;
			}

			if (m == '?')				// ? - ���������� ������ � ������ �������
			{
				maskPos++;
				strPos++;
				continue;
			}

			if (m == '*')				//* - ���������� ��� �� ������� ������� "_"
			{
				while (str.at(strPos) != '_')
				{
					strPos++;

					if (strPos == strLen)		// ����� �� ����� ������ - ������ �������
					{
						if (maskPos == maskLen - 1)
						{
							return true;		//������ ������� ���� ��� ��������� ���������
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

			if (str.at(strPos) != m)			// ��������� ������� �� �������
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



