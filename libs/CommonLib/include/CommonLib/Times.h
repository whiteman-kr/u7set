#pragma once

#include <QDateTime>
#include <QTimeZone>
#include <QMetaType>
#include <QString>


// Time literals converts to ms
//
constexpr int64_t operator "" _ms(unsigned long long int value)
{
	return value;
}

constexpr int64_t operator "" _sec(unsigned long long int value)
{
	return value * 1000;
}

constexpr int64_t operator "" _min(unsigned long long int value)
{
	return value * 60 * 1000;
}

constexpr int64_t operator "" _hour(unsigned long long int value)
{
	return value * 3600 * 1000;
}

constexpr int64_t operator "" _hours(unsigned long long int value)
{
	return value * 3600 * 1000;
}

constexpr int64_t operator "" _day(unsigned long long int value)
{
	return value * 24 * 3600 * 1000;
}

inline static const QTimeZone TIME_ZONE_UTC(QTimeZone::UTC);
inline static const QTimeZone TIME_ZONE_LOCAL(QTimeZone::LocalTime);

//
// TimeStamp
//
struct TimeStamp
{
	qint64 timeStamp = 0;	// ms

	// --
	//
	TimeStamp() = default;
	TimeStamp(const TimeStamp&) = default;
	TimeStamp(TimeStamp&&) noexcept = default;
	TimeStamp(qint64 value) : timeStamp(value)
	{
	}
	explicit TimeStamp(const QDateTime& dateTime) : 
		timeStamp(dateTime.toMSecsSinceEpoch() + static_cast<qint64>(dateTime.offsetFromUtc()) * 1000ll)
	{
	}

	TimeStamp& operator=(const TimeStamp& src) = default;
	TimeStamp& operator=(TimeStamp&& src) noexcept = default;

	[[nodiscard]] QDateTime toDateTime() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, TIME_ZONE_UTC);
	}

	[[nodiscard]] QDate toDate() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, TIME_ZONE_UTC).date();
	}

	[[nodiscard]] QTime toTime() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, TIME_ZONE_UTC).time();
	}

	[[nodiscard]] TimeStamp roundedToHour() const
	{
		return TimeStamp{(timeStamp / 1_hour) * 1_hour};
	}

	auto operator<=>(const TimeStamp&) const = default;

	TimeStamp& operator+= (qint64 timeSpan)
	{
		timeStamp += timeSpan;
		return *this;
	}

	TimeStamp& operator-= (qint64 timeSpan)
	{
		timeStamp -= timeSpan;
		return *this;
	}
};

Q_DECLARE_METATYPE(TimeStamp)


//
// TimeSpan
//
struct TimeSpan
{
	qint64 timeSpan = 0;			// milliseconds
};

Q_DECLARE_METATYPE(TimeSpan)


//
// Times
//
struct Times
{
	TimeStamp system;
	TimeStamp local;
	TimeStamp plant;

	[[nodiscard]] QDateTime systemToDateTime() const
	{
		return system.toDateTime();
	}

	[[nodiscard]] QDateTime localToDateTime() const
	{
		return local.toDateTime();
	}

	[[nodiscard]] QDateTime plantToDateTime() const
	{
		return plant.toDateTime();
	}

	Times& operator += (qint64 timeSpan)
	{
		system += timeSpan;
		local += timeSpan;
		plant += timeSpan;

		return *this;
	}

	void clear()
	{
		*this = {};
	}
};

// Time to string formatting function
//
inline QString timeToString(const QDateTime& time)
{
	if (time.isNull() == true || time.toMSecsSinceEpoch() == 0)
	{
		return "Not Available";
	}
	if (time.isValid() == false)
	{
		return time.toString("dd.MM.yyyy hh:mm:ss.zzz (Invalid)");
	}
	return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
}

struct DateTimeFormat
{
	static QString forFileName() 
	{ 
		return "dd_MM_yyyy_HH_mm_ss"; 
	}

	static QString dateTimeFormat(bool withMilliseconds = false,
								  const QLocale& locale = QLocale::system(),
								  QLocale::FormatType fmt = QLocale::ShortFormat)
	{
		QString dateFmt = locale.dateFormat(fmt);
		QString timeFmt = locale.timeFormat(fmt);

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

		return dateFmt + " " + timeFmt;
	}

	static QString dateFormat(const QLocale& locale = QLocale::system(),
								  QLocale::FormatType fmt = QLocale::ShortFormat)
	{
		QString dateFmt = locale.dateFormat(fmt);
		return dateFmt;
	}

	static QString timeFormat(bool withMilliseconds = false,
							  const QLocale& locale = QLocale::system(),
							  QLocale::FormatType fmt = QLocale::ShortFormat)
	{
		QString timeFmt = locale.timeFormat(fmt);

		if (withMilliseconds && timeFmt.contains('z') != false)
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
					timeFmt.insert(apIdx - 1, ".zzz");
				}
				else
				{
					timeFmt.append(".zzz");
				}
			}
		}

		return timeFmt;
	}
};



namespace DateTimeToString
{
	inline QString stringDateTime(const QDateTime& time, bool withMilliseconds = false, const QLocale& locale = QLocale::system())
	{
		if (time.isValid() == false)
		{
			return QString();
		}

		return locale.toString(time, DateTimeFormat::dateTimeFormat(withMilliseconds, locale));
	}

	inline QString stringTime(const QTime& time, bool withMilliseconds = false, const QLocale& locale = QLocale::system())
	{
		if (time.isValid() == false)
		{
			return QString();
		}

		return locale.toString(time, DateTimeFormat::timeFormat(withMilliseconds, locale));
	}

	inline QString stringDate(const QDate& date, const QLocale& locale = QLocale::system())
	{
		if (date.isValid() == false)
		{
			return QString();
		}

		return locale.toString(date, DateTimeFormat::dateFormat(locale));
	}

	inline QString stringDateTimeToFile(const QDateTime& time, const QLocale& locale = QLocale::system())
	{
		if (time.isValid() == false)
		{
			return QString();
		}

		return locale.toString(time, DateTimeFormat::forFileName());
	}
} // namespace DateTimeToString