#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <QThread>

class MultiThreadFileTest : public QThread
{

public:
	MultiThreadFileTest(int number, QString dbHost, QString dbUser, QString dbUserPassword, QString name, int amountOfFiles);
	virtual ~MultiThreadFileTest();

	virtual void run();

	int m_threadNumber = 0;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;

	int m_amountOfFileIds = 0;
};
