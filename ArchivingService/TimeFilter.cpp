#include "TimeFilter.h"


TimeFilter::Channel::Channel()
{
}

void TimeFilter::Channel::setTime(quint64 time)
{
	if (time < m_time)
	{
		m_errNomono++;
	}

	m_time = time;
}

TimeFilter::TimeFilter()
{
}

void TimeFilter::setTimes(Times times)
{
	m_channel[CHANNEL_PLANT].setTime(times.plant.timeStamp);
	m_channel[CHANNEL_SYSTEM].setTime(times.system.timeStamp);
	m_channel[CHANNEL_LOCAL].setTime(times.local.timeStamp);
}


void TimeFilter::getTimes(Times& times, qint64& serverTime)
{
	m_channel[CHANNEL_SERVER].setTime(QDateTime::currentMSecsSinceEpoch());

	times.plant.timeStamp = m_channel[CHANNEL_PLANT].time();
	times.system.timeStamp = m_channel[CHANNEL_SYSTEM].time();
	times.local.timeStamp = m_channel[CHANNEL_LOCAL].time();

	serverTime = m_channel[CHANNEL_SERVER].time();
}
