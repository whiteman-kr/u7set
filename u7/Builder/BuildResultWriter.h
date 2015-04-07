#pragma once

#include <QObject>
#include "../include/OutputLog.h"


class DbController;

namespace Builder
{
	class BuildResultWriter : public QObject
	{
		Q_OBJECT

	private:
		QString m_userName;
		QString m_projectStrID;
		bool m_release = false;
		int m_buildNo = 0;
		OutputLog* m_log = nullptr;
		DbController* m_dbController;

	public:
		explicit BuildResultWriter(DbController *db, OutputLog *log, bool release, QObject *parent = 0);

		bool start();
		bool finish();

	signals:

	public slots:

	};
}


