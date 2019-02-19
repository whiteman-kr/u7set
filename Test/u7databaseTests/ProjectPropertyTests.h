#pragma once
#include <QTest>
#include "TestDbBase.h"

class ProjectPropertyTests : public TestDbBase
{
	Q_OBJECT

public:
	ProjectPropertyTests();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void set_project_property();
	void get_project_property();

public:
};
