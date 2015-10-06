#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <QThread>

class MultiThreadTest : public QThread
{

public:
	MultiThreadTest(int number,
					const char* dbHost,
					const char* dbUser,
					const char* dbUserPassword,
					const char* name,
					int amountOfFiles);
	virtual ~MultiThreadTest();

	void run();

	int m_threadNumber;
	int m_amountOfFileIds;

	const char* m_databaseHost;
	const char* m_databaseUser;
	const char* m_databaseUserPassword;
	const char* m_projectName;
};
