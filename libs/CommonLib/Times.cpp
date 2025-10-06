#ifndef COMMON_LIB_DOMAIN
#error Do not include this file in the project! Link CommonLib instead.
#endif

#include <CommonLib/Times.h>


QString DateTimeFormat::fileName(const QLocale* locale)
{
	QString dateFmt = locale->dateFormat(QLocale::ShortFormat);
	QString timeFmt = locale->timeFormat(QLocale::ShortFormat);

	dateFmt.replace(QRegularExpression("[^dMy]"), "_");
	timeFmt.replace(QRegularExpression("[^hHmsaH]"), "_");

	return dateFmt + "_" + timeFmt;
}

QString DateTimeFormat::dateTime(bool withSeconds, bool withMilliseconds, const QLocale* locale)
{
	// Try to found data in cache
	//
	int seed = withSeconds + withMilliseconds;
	size_t hash = qHash(locale, seed);
	{
		QReadLocker rl(&lock());
		auto it = m_dateTimeCache.find(hash);
		if (it != m_dateTimeCache.end())
		{
			return it->second; // Return value from map
		}
	}

	// No data found in cache
	//
	QString dateFmt = locale->dateFormat(QLocale::ShortFormat);
	QString timeFmt = locale->timeFormat(QLocale::ShortFormat);

	if (withSeconds == false)
	{
		timeFmt.remove(QRegularExpression("[:]?s{1,2}"));
	}
	else
	{
		// ensure seconds exist
		if (timeFmt.contains(QRegularExpression("s{1,2}")) == false)
		{
			int apIdx = timeFmt.indexOf("AP");
			if (apIdx == -1)
				apIdx = timeFmt.indexOf("ap");

			if (apIdx != -1)
			{
				timeFmt.insert(apIdx - 1, ":ss ");
			}
			else
			{
				timeFmt.append(":ss");
			}
		}
	}

	if (withMilliseconds && timeFmt.contains('z') == false)
	{
		int idx = timeFmt.lastIndexOf('s');
		if (idx != -1)
		{
			timeFmt.insert(idx + 1, ".zzz");
		}
		else
		{
			int apIdx = timeFmt.indexOf("AP");
			if (apIdx == -1)
			{
				apIdx = timeFmt.indexOf("ap");
			}

			if (apIdx != -1)
			{
				timeFmt.insert(apIdx - 1, ".zzz ");
			}
			else
			{
				timeFmt.append(".zzz");
			}
		}
	}

	QString result = dateFmt + " " + timeFmt;
	{
		QWriteLocker rl(&lock());
		m_dateTimeCache[hash] = result;
	}

	return result;
}

QString DateTimeFormat::date(const QLocale* locale)
{
	// Try to found data in cache
	//
	size_t hash = qHash(locale);
	{
		QReadLocker rl(&lock());
		auto it = m_dateCache.find(hash);
		if (it != m_dateCache.end())
		{
			return it->second; // Return value from map
		}
	}

	// No data found in cache
	//

	QString dateFmt = locale->dateFormat(QLocale::ShortFormat);
	{
		QWriteLocker rl(&lock());
		m_dateCache[hash] = dateFmt;
	}

	return dateFmt;
}

QString DateTimeFormat::time(bool withSeconds, bool withMilliseconds, const QLocale* locale)
{
	// Try to found data in cache
	//
	int seed = withSeconds + withMilliseconds;
	size_t hash = qHash(locale, seed);

	{
		QReadLocker rl(&lock());
		auto it = m_timeCache.find(hash);
		if (it != m_timeCache.end())
		{
			return it->second; // Return value from map
		}
	}

	// No data found in cache
	//
	QString timeFmt = locale->timeFormat(QLocale::ShortFormat);
	
	if (withSeconds == false)
	{
		timeFmt.remove(QRegularExpression("[:]?s{1,2}"));
	}
	else
	{
		// ensure seconds exist
		if (timeFmt.contains(QRegularExpression("s{1,2}")) == false)
		{
			int apIdx = timeFmt.indexOf("AP");
			if (apIdx == -1)
			{
				apIdx = timeFmt.indexOf("ap");
			}

			if (apIdx != -1)
			{
				timeFmt.insert(apIdx - 1, ":ss");
			}
			else
			{
				timeFmt.append(":ss");
			}
		}
	}

	if (withMilliseconds && timeFmt.contains('z') == false)
	{
		int idx = timeFmt.lastIndexOf('s');
		if (idx != -1)
		{
			timeFmt.insert(idx + 1, ".zzz");
		}
		else
		{
			int apIdx = timeFmt.indexOf("AP");
			if (apIdx == -1)
			{
				apIdx = timeFmt.indexOf("ap");
			}

			if (apIdx != -1)
			{
				timeFmt.insert(apIdx - 1, ".zzz ");
			}
			else
			{
				timeFmt.append(".zzz");
			}
		}
	}

	{
		QWriteLocker rl(&lock());
		m_timeCache[hash] = timeFmt;
	}
	return timeFmt;
};

//
// DateTimeToString
//

QString DateTimeToString::dateTimeMin(const QDateTime& time, const QLocale* locale)
{
	return dateTime(time, false, false, locale);
}
QString DateTimeToString::dateTimeSec(const QDateTime& time, const QLocale* locale)
{
	return dateTime(time, true, false, locale);
}
QString DateTimeToString::dateTimeMs(const QDateTime& time, const QLocale* locale)
{
	return dateTime(time, true, true, locale);
}

QString DateTimeToString::timeMin(const QTime& tm, const QLocale* locale)
{
	return time(tm, false, false, locale);
}
QString DateTimeToString::timeSec(const QTime& tm, const QLocale* locale)
{
	return time(tm, true, false, locale);
}
QString DateTimeToString::timeMs(const QTime& tm, const QLocale* locale)
{
	return time(tm, true, true, locale);
}

QString DateTimeToString::dateTime(const QDateTime& time, bool withSeconds, bool withMilliseconds, const QLocale* locale)
{
	// TODO: What to output when isValid == false (e.g. 2002, 2, 30 - Feb 30 does not exist) or toMSecsSinceEpoch == 0 (01.01.1970)?

	if (time.isNull() == true)
	{
		return {"?"};
	}

	return locale->toString(time, DateTimeFormat::dateTime(withSeconds, withMilliseconds, locale));
}

QString DateTimeToString::time(const QTime& time, bool withSeconds, bool withMilliseconds, const QLocale* locale)
{
	if (time.isNull() == true)
	{
		return {"?"};
	}

	return locale->toString(time, DateTimeFormat::time(withSeconds, withMilliseconds, locale));
}

QString DateTimeToString::fileName(const QDateTime& time, const QLocale* locale)
{
	if (time.isNull() == true)
	{
		return {"?"};
	}

	return locale->toString(time, DateTimeFormat::fileName(locale));
}

QString DateTimeToString::date(const QDate& date, const QLocale* locale)
{
	if (date.isNull() == true)
	{
		return {"?"};
	}

	return locale->toString(date, DateTimeFormat::date(locale));
}

QString DateTimeToString::timeDuration(qint64 timeStamp)
{
	int secs = static_cast<int>(timeStamp / 1_sec) % 60;
	int mins = static_cast<int>(timeStamp / 1_min) % 60;
	int hours = static_cast<int>(timeStamp / 1_hour);

	return QString::asprintf("%02d:%02d:%02d", hours, mins, secs);
}

QString DateTimeToString::timeDurationMs(qint64 timeStamp)
{
	int msecs = static_cast<int>(timeStamp % 1000_ms);
	int secs = static_cast<int>(timeStamp / 1_sec) % 60;
	int mins = static_cast<int>(timeStamp / 1_min) % 60;
	int hours = static_cast<int>(timeStamp / 1_hour);

	return QString::asprintf("%02d:%02d:%02d.%03d", hours, mins, secs, msecs);
}

QString DateTimeToString::dateTimeDuration(qint64 timeStamp)
{
	int days = static_cast<int>(timeStamp / 1_day) % 24;
	timeStamp -= days * 1_day;
	int secs = static_cast<int>(timeStamp / 1_sec) % 60;
	int mins = static_cast<int>(timeStamp / 1_min) % 60;
	int hours = static_cast<int>(timeStamp / 1_hour) % 60;

	QString distanceText;

	if (days > 0)
	{
		distanceText = QString::asprintf("%1dd, %02d:%02d:%02d", days, hours, mins, secs);
	}
	else
	{
		distanceText = QString::asprintf("%02d:%02d:%02d", hours, mins, secs);
	}

	return distanceText;
}

QString DateTimeToString::dateTimeDurationMs(qint64 timeStamp)
{
	int days = static_cast<int>(timeStamp / 1_day) % 24;
	timeStamp -= days * 1_day;
	int msecs = static_cast<int>(timeStamp % 1000_ms);
	int secs = static_cast<int>(timeStamp / 1_sec) % 60;
	int mins = static_cast<int>(timeStamp / 1_min) % 60;
	int hours = static_cast<int>(timeStamp / 1_hour) % 60;

	QString distanceText;

	if (days > 0)
	{
		distanceText = QString::asprintf("%1dd, %02d:%02d:%02d.%03d", days, hours, mins, secs, msecs);
	}
	else
	{
		distanceText = QString::asprintf("%02d:%02d:%02d.%03d", hours, mins, secs, msecs);
	}

	return distanceText;
}