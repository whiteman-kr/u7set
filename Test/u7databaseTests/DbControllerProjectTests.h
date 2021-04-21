#pragma once
#include <QTest>
#include <memory>
#include "TestDbBase.h"
#include "../../DbLib/DbController.h"

class DbControllerProjectTests : public TestDbBase
{
	Q_OBJECT

public:
	DbControllerProjectTests();
	void setProjectVersion(int version);

private slots:
	virtual void initTestCase() override;
	virtual void cleanupTestCase() override;

	void createOpenUpgradeCloseDeleteProject();
	void getProjectListTest();
	void ProjectPropertyTest();
	void isProjectOpenedTest();
	void connectionInfoTest();

private:
	DbController m_db;
	int m_databaseVersion = -1;
};
