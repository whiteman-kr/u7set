#pragma once

#include "../lib/TimeStamp.h"
#include "../lib/AppSignal.h"


class TimeFilter
{
private:

	class Channel
	{
	public:
		Channel();

		void setTime(quint64 time);

		qint64 time() const { return m_time; }

	private:
		qint64 m_time = 0;
		qint64 m_errNomono = 0;				//	nonmonotonicity errors count
	};

	static const int CHANNELS_COUNT = 4;

	static const int CHANNEL_PLANT = 0;
	static const int CHANNEL_SYSTEM = 1;
	static const int CHANNEL_LOCAL = 2;
	static const int CHANNEL_SERVER = 3;			// system time of archive server

public:
	TimeFilter();

	void setTimes(Times times);
	void getTimes(Times& times, qint64 &serverTime);

private:
	TimeFilter::Channel m_channel[CHANNELS_COUNT];
};

