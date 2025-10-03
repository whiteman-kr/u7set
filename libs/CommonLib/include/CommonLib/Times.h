#pragma once

#include <QDateTime>
#include <QTimeZone>
#include <QMetaType>
#include <QString>
#include <QReadWriteLock>


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


// Time formats
// 
class DateTimeFormat
{
public:
	static QString fileName(const QLocale* locale = &m_systemLocale);
	static QString dateTime(bool withMilliseconds = false, const QLocale* locale = &m_systemLocale);
	static QString date(const QLocale* locale = &m_systemLocale);
	static QString time(bool withMilliseconds = false, const QLocale* locale = &m_systemLocale);

private:
	// key is a hash of a locale, counted by qHash function with seed equal to useMilliseconds (0 or 1)
	//
	inline static QReadWriteLock m_lock;
	inline static std::unordered_map<size_t, QString> m_dateCache;
	inline static std::unordered_map<size_t, QString> m_timeCache;
	inline static std::unordered_map<size_t, QString> m_dateTimeCache;

	inline static const QLocale m_systemLocale = QLocale::system();
};

// Time to string formatting class
//
class DateTimeToString
{
public:
	static QString dateTime(const QDateTime& time, bool withMilliseconds = false, const QLocale* locale = &m_systemLocale);
	static QString time(const QTime& time, bool withMilliseconds = false, const QLocale* locale = &m_systemLocale);
	static QString date(const QDate& date, const QLocale* locale = &m_systemLocale);
	static QString fileName(const QDateTime& time, const QLocale* locale = &m_systemLocale);

private:
	inline static const QLocale m_systemLocale = QLocale::system();
};