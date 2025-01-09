#pragma once
#include <QTest>
#include "TestDbBase.h"


class DbControllerUserTests : public TestDbBase
{
	Q_OBJECT

public:
	DbControllerUserTests();

private slots:
	virtual void initTestCase() override;
	virtual void cleanupTestCase() override;

	void createUserTest();
	void updateUserTest();
	void getUserListTest();

private:
	DbController m_db;
};
