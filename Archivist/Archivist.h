#pragma once

#include <QString>
#include <QDateTime>

#include "Print.h"

struct RequestParams
{
	QString archiveLocation;

	QDateTime begin;
	QDateTime end;
	QString signalsList;

	QString destLocation;

	bool fileArchive = true;
};

class Archivist
{
public:
	Archivist(const RequestParams& rp);

	virtual bool copyArchive() = 0;

protected:
	void printRequestParams();

protected:
	RequestParams m_reqParams;
};


