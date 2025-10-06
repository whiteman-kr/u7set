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
		QReadLocker rl(&m_lock);
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
		QWriteLocker rl(&m_lock);
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
		QReadLocker rl(&m_lock);
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
		QWriteLocker rl(&m_lock);
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
		QReadLocker rl(&m_lock);
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
		QWriteLocker rl(&m_lock);
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


QString DateTimeToString::timeDuration(const QTime& time)
{
	if (time.isNull() == true)
	{
		return {"?"};
	}
	return time.toString("HHh MMm SSs");
}

QString DateTimeToString::timeDurationMs(const QTime& time)
{
	if (time.isNull() == true)
	{
		return {"?"};
	}
	return time.toString("HHh MMm SSs ZZZms");
}
QString DateTimeToString::dateTimeDuration(const QDateTime& time)
{
	if (time.isNull() == true)
	{
		return {"?"};
	}

	if (time.date().day() == 0)
	{
		return time.toString("HHh MMm SSs");
	}

	return time.toString("DDd, HHh MMm SSs");
}
QString DateTimeToString::dateTimeDurationMs(const QDateTime& time)
{
	if (time.isNull() == true)
	{
		return {"?"};
	}

	if (time.date().day() == 0)
	{
		return time.toString("HHh MMm SSs ZZZms");
	}

	return time.toString("DDd, HHh MMm SSs ZZZms");
}