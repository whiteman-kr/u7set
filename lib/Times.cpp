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
