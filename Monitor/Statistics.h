#pragma once

class Statistics
{
public:
	Statistics();
	~Statistics();

private:
	QMutex m_mutex;
};

