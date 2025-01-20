#pragma once
#include <QTest>
#include "TestDbBase.h"

class OtherTests : public TestDbBase
{
	Q_OBJECT

public:
	OtherTests();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void get_project_versionTest();
	void build_startTest();
	void build_finishTest();
	void add_log_recordTest();
};
