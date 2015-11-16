#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <QThread>

class MultiThreadFileTest : public QThread
{

public:
	MultiThreadFileTest(int number,
					const char* dbHost,
					const char* dbUser,
					const char* dbUserPassword,
					const char* name,
					int amountOfFiles);

	virtual ~MultiThreadFileTest();

	virtual void run();

	int m_threadNumber = 0;
	int m_amountOfFileIds = 0;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;
};
