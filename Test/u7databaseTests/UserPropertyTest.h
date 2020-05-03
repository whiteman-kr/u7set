#pragma once
#include <QTest>
#include "TestDbBase.h"

class UserPropertyTests : public TestDbBase
{
	Q_OBJECT

public:
	UserPropertyTests();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void set_user_property();
	void set_user_property_check_user_influence();
	void get_user_property();

public:
};
