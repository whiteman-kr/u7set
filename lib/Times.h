#pragma once

#include <QDateTime>
#include "TimeStamp.h"

struct Times
{
	TimeStamp system;
	TimeStamp local;
	TimeStamp plant;

	QDateTime systemToDateTime() const;
	QDateTime localToDateTime() const;
	QDateTime plantToDateTime() const;

	Times& operator += (qint64 timeSpan);
};

