#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QDateTime>

struct TimeStamp
{
	qint64 timeStamp = 0;

	// --
	//
	TimeStamp() = default;
	explicit TimeStamp(qint64 value) : timeStamp(value)
	{
	}
	explicit TimeStamp(const QDateTime& dateTime) : timeStamp(dateTime.toMSecsSinceEpoch())
	{
	}

	QDateTime toDateTime() const
	{
		return QDateTime::fromMSecsSinceEpoch(timeStamp, Qt::UTC);
	}
};

struct TimeSpan
{
	qint64 timeSpan = 0;			// milliseconds
};


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

#endif TIMESTAMP_H
