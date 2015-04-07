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
		int m_changesetID = 0;
		int m_buildNo = -1;
		OutputLog* m_log = nullptr;
		DbController* m_dbController = nullptr;

		bool m_runBuild = true;

	private:
		bool createBuildDirectory();

	public:
		explicit BuildResultWriter(QObject *parent = 0);

		bool start(DbController *db, OutputLog *log, bool release, int changesetID);
		bool finish();

	signals:

	public slots:

	};
}


