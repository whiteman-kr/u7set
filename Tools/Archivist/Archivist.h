#pragma once

#include <QString>
#include <QDateTime>
#include <iostream>
#include <QDebug>

//#include "../OnlineLib/CircularLogger.h"

struct RequestParams
{
	QString archiveLocation;

	QDateTime begin;
	QDateTime end;
	QString signalsList;

	QString destLocation;
};

class Archivist
{
public:
	Archivist(int argc, char* argv[]);

	bool start();

	virtual void copyArchive() = 0;

private:
	QString m_sourceDir;
	QString m_beginTime;
	QString m_endTime;
	QString m_destFile;
	int m_partSize = 0;
};

//extern std::shared_ptr<CircularLogger> logger;
