#pragma once
#include <QTest>

class OtherTests : public QObject
{
	Q_OBJECT

public:
	OtherTests();

private slots:
	void get_project_versionTest();
	void get_unitsTest();
	void build_startTest();
	void build_finishTest();
	void add_unitTest();
};
