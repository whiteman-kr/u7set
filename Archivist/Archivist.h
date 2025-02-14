#pragma once

#include <QString>
#include <QDateTime>

#include "Print.h"

struct RequestParams
{
	QString archiveLocation;

	bool checkonly = false;

	QDateTime begin;
	QDateTime end;
	QStringList signalsList;

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
