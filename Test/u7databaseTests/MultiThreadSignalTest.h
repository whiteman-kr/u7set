#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <QThread>

class MultiThreadSignalTest : public QThread
{
public:
	MultiThreadSignalTest(int number,
						  const char* dbHost,
						  const char* dbUser,
						  const char* dbUserPassword,
						  const char* name,
						  int amountOfFiles);

	virtual ~MultiThreadSignalTest();

	virtual void run();

	int m_threadNumber;
	int m_amountOfSignalIds;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;
};

