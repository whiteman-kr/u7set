#include <QString>
#include <QtTest>

class File2pgsqlTestTest : public QObject
{
	Q_OBJECT

public:
	File2pgsqlTestTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void testCase1_data();
	void testCase1();
};

File2pgsqlTestTest::File2pgsqlTestTest()
{
}

void File2pgsqlTestTest::initTestCase()
{
}

void File2pgsqlTestTest::cleanupTestCase()
{
}

void File2pgsqlTestTest::testCase1_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("output");

	QTest::newRow("0") << QString("") << QString("");;
	QTest::newRow("1") << QString("") << QString("");;
	QTest::newRow("2") << QString("") << QString("");;
}

void File2pgsqlTestTest::testCase1()
{
	QFETCH(QString, data);
	QVERIFY2(true, "Failure");
}


QTEST_APPLESS_MAIN(File2pgsqlTestTest)

#include "tst_File2pgsqlTestTest.moc"
