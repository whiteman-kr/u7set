#ifndef STATISTICS_H
#define STATISTICS_H

#include <QMutex>


class Statistics
{
public:
	Statistics();
	~Statistics();

private:
	QMutex m_mutex;

};

#endif // STATISTICS_H
