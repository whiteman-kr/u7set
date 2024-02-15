#ifndef DATABASETESTS_H
#define DATABASETESTS_H

#include <QTest>

#include "../Metrology/Database.h"

// ==============================================================================================

class DatabaseTests : public QObject
{
	Q_OBJECT

public:

	DatabaseTests();

private slots:

	void initTestCase();
	void cleanupTestCase();

	void openDatabase();

private:

	Database m_db;

	QString m_databaseName = "TestMetrology";

};

// ==============================================================================================

#endif // DATABASETESTS_H
