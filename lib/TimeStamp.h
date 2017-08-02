#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QDateTime>

enum class TimeType
{
	Plant,
	System,
	Local,
	ArchiveId
};

Q_DECLARE_METATYPE(TimeType)

// TimeStamp
//
struct TimeStamp
{
	qint64 timeStamp = 0;

	// --
	//
	TimeStamp() = default;
	TimeStamp(const TimeStamp&) = default;
	TimeStamp(qint64 value) : timeStamp(value)
	{
	}
	explicit TimeStamp(const QDateTime& dateTime) : timeStamp(dateTime.toMSecsSinceEpoch() + dateTime.offsetFromUtc() * 1000)
	{
	}

	TimeStamp& operator=(const TimeStamp& src) = default;

	QDateTime toDateTime() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, Qt::UTC);
	}

	QDate toDate() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, Qt::UTC).date();
	}

	QTime toTime() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, Qt::UTC).time();
	}


	bool operator> (const TimeStamp& value) const
	{
		return this->timeStamp > value.timeStamp;
	}

	bool operator>= (const TimeStamp& value) const
	{
		return this->timeStamp >= value.timeStamp;
	}

	bool operator< (const TimeStamp& value) const
	{
		return this->timeStamp < value.timeStamp;
	}

	bool operator<= (const TimeStamp& value) const
	{
		return this->timeStamp <= value.timeStamp;
	}

	bool operator== (const TimeStamp& value) const
	{
		return this->timeStamp == value.timeStamp;
	}

	bool operator!= (const TimeStamp& value) const
	{
		return this->timeStamp != value.timeStamp;
	}
};

Q_DECLARE_METATYPE(TimeStamp)

// TimeSpan
//
struct TimeSpan
{
	qint64 timeSpan = 0;			// milliseconds
};

Q_DECLARE_METATYPE(TimeSpan)


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

#endif //TIMESTAMP_H
