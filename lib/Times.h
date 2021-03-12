#pragma once

#include <QDateTime>
#include "TimeStamp.h"

struct Times
{
	TimeStamp system;
	TimeStamp local;
	TimeStamp plant;

	[[nodiscard]] QDateTime systemToDateTime() const;
	[[nodiscard]] QDateTime localToDateTime() const;
	[[nodiscard]] QDateTime plantToDateTime() const;

	Times& operator += (qint64 timeSpan);
};

