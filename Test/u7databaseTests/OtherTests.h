#pragma once
#include <QTest>

class OtherTests : public QObject
{
	Q_OBJECT

public:
	OtherTests();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void get_project_versionTest();

public:
	static bool get_project_version();
};
