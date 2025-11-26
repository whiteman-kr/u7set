#pragma once

#include <CommonStdLib/TimesStd.h>

#include <QDateTime>
#include <QMetaType>
#include <QTimeZone>


inline static const QTimeZone TIME_ZONE_UTC(QTimeZone::UTC);
inline static const QTimeZone TIME_ZONE_LOCAL(QTimeZone::LocalTime);

//
// TimeStamp
//
struct TimeStamp
{
	qint64 timeStamp = 0; // ms

	// --
	//
	TimeStamp() = default;
	TimeStamp(const TimeStamp&) = default;
	TimeStamp(TimeStamp&&) noexcept = default;
	TimeStamp(qint64 value) :
		timeStamp(value)
	{
	}
	explicit TimeStamp(const QDateTime& dateTime) :
		timeStamp(dateTime.toMSecsSinceEpoch() + static_cast<qint64>(dateTime.offsetFromUtc()) * 1000ll)
	{
	}

	TimeStamp& operator=(const TimeStamp& src) = default;
	TimeStamp& operator=(TimeStamp&& src) noexcept = default;

	[[nodiscard]] QDateTime toDateTime() const { return QDateTime::fromMSecsSinceEpoch(timeStamp, TIME_ZONE_UTC); }

	[[nodiscard]] QDate toDate() const { return QDateTime::fromMSecsSinceEpoch(timeStamp, TIME_ZONE_UTC).date(); }

	[[nodiscard]] QTime toTime() const { return QDateTime::fromMSecsSinceEpoch(timeStamp, TIME_ZONE_UTC).time(); }

	[[nodiscard]] TimeStamp roundedToHour() const { return TimeStamp{(timeStamp / 1_hour) * 1_hour}; }

	auto operator<=>(const TimeStamp&) const = default;

	TimeStamp& operator+=(qint64 timeSpan)
	{
		timeStamp += timeSpan;
		return *this;
	}

	TimeStamp& operator-=(qint64 timeSpan)
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
	qint64 timeSpan = 0; // milliseconds
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

	[[nodiscard]] QDateTime systemToDateTime() const { return system.toDateTime(); }

	[[nodiscard]] QDateTime localToDateTime() const { return local.toDateTime(); }

	[[nodiscard]] QDateTime plantToDateTime() const { return plant.toDateTime(); }

	Times& operator+=(qint64 timeSpan)
	{
		system += timeSpan;
		local += timeSpan;
		plant += timeSpan;

		return *this;
	}

	void clear() { *this = {}; }

	bool operator==(const Times& t2) const { return system == t2.system && local == t2.local && plant == t2.plant; }
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
