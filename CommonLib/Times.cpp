#ifndef COMMON_LIB_DOMAIN
#error Don't include this file in the project! Link DbLib instead.
#endif

#include "Times.h"

QDateTime Times::systemToDateTime() const
{
	return system.toDateTime();
}

QDateTime Times::localToDateTime() const
{
	return local.toDateTime();
}

QDateTime Times::plantToDateTime() const
{
	return plant.toDateTime();
}

Times& Times::operator += (qint64 timeSpan)
{
	system += timeSpan;
	local += timeSpan;
	plant += timeSpan;

	return *this;
}
