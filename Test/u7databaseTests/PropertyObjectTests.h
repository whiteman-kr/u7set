#pragma once
#include <QTest>

class PropertyObjectTests : public QObject
{
	Q_OBJECT

public:
	PropertyObjectTests();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void firstTest();
	void secondTest();
};

