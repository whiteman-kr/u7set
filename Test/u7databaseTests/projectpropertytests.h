#pragma once
#include <QTest>

class ProjectPropertyTests : public QObject
{
	Q_OBJECT

public:
	ProjectPropertyTests();

private slots:
	void set_project_property();
	void get_project_property();
};
