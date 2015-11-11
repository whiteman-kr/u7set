#pragma once
#include <QTest>
#include "../../include/PropertyObject.h"

class PropertyClass;

class PropertyObjectTests : public QObject
{
	Q_OBJECT

public:
	PropertyObjectTests();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void testSttaticProperties();
	void testDynamicProperties();

private:
	PropertyObject* m_propertyObject = nullptr;
};


// Class for test
//
class PropertyClass : public PropertyObject
{
	Q_OBJECT

public:
	PropertyClass();
	virtual ~PropertyClass();

	enum Priority
	{
		VeryLow = 0,
		Low = 1,
		Normal = 2,
		High = 4,
		VeryHigh = 5,
		Extreme = 6
	};
	Q_ENUM(Priority)

public:
	bool boolStatProp() const;
	void setBoolStatProp(bool value);

	int intStatProp() const;
	void setIntStatProp(int value);

	QString qstringStatProp() const;
	void setQStringStatProp(QString value);

	Priority priorityStatProp() const;
	void setPriorityStatProp(Priority value);

	// Data
	//
public:
	bool m_boolStatProp = false;
	int m_intStatProp = 0;
	QString m_qstringStatProp;
	Priority m_priorityStatProp;
};
