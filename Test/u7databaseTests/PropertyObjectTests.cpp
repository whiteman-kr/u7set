#include <QtSql>
#include <QTest>
#include "PropertyObjectTests.h"
#include "../../lib/PropertyObject.h"

PropertyObjectTests::PropertyObjectTests()
{
}

void PropertyObjectTests::initTestCase()
{
	// Actions before test starts
	//
	m_propertyObject = new PropertyObject;
}

void PropertyObjectTests::cleanupTestCase()
{
	// Actions after test finish
	//
	delete m_propertyObject;
	m_propertyObject = nullptr;
}

void PropertyObjectTests::testSttaticProperties()
{
	PropertyClass p;

	// Test if static propertie present
	//

	// bool test
	//
	QVERIFY2(p.propertyExists("BoolStatProp") == true, "Property BoolStatProp is not exists");
	QVERIFY2(p.propertyExists("IntStatProp") == true, "Property BoolStatProp is not exists");
	QVERIFY2(p.propertyExists("QStringStatProp") == true, "Property QStringStatProp is not exists");
	QVERIFY2(p.propertyExists("PriorityStatProp") == true, "PriorityStatProp QStringStatProp is not exists");

	p.setBoolStatProp(true);
	QVERIFY2(p.propertyValue("BoolStatProp").toBool() == true, "Statuic property bool fail");
	p.setBoolStatProp(false);
	QVERIFY2(p.propertyValue("BoolStatProp").toBool() == false, "Statuic property bool fail");

	p.setPropertyValue("BoolStatProp", true);
	QVERIFY2(p.propertyValue("BoolStatProp").toBool() == true, "Static property bool fail");
	p.setPropertyValue("BoolStatProp", QVariant(false));
	QVERIFY2(p.propertyValue("BoolStatProp").toBool() == false, "Static property bool fail");

	// int test
	//
	p.setIntStatProp(0);
	QVERIFY2(p.propertyValue("IntStatProp").toInt() == 0, "Static int property fail");
	p.setIntStatProp(254);
	QVERIFY2(p.propertyValue("IntStatProp").toInt() == 254, "Static int property fail");

	p.setPropertyValue("IntStatProp", -1367);
	QVERIFY2(p.propertyValue("IntStatProp").toInt() == -1367, "Static int property fail");
	p.setPropertyValue("IntStatProp", QVariant(2400));
	QVERIFY2(p.propertyValue("IntStatProp").toInt() == 2400, "Static int property fail");

	// QString test
	//
	p.setQStringStatProp("Hello!");
	QVERIFY2(p.propertyValue("QStringStatProp").toString() == "Hello!", "Static string property fail");

	p.setPropertyValue("QStringStatProp", "PassQString");
	QVERIFY2(p.propertyValue("QStringStatProp").toString() == "PassQString", "Static string property fail");
	p.setPropertyValue("QStringStatProp", QVariant("QVariant"));
	QVERIFY2(p.propertyValue("QStringStatProp").toString() == "QVariant", "Static string property fail");

	// enum test
	//
	p.setPriorityStatProp(PropertyClass::Priority::High);
	QVERIFY2(p.propertyValue("PriorityStatProp").toString() == "High", "Static enum property fail");

	p.setPropertyValue("PriorityStatProp", "Low");
	QVERIFY2(p.propertyValue("PriorityStatProp").toString() == "Low", "Static enum property fail");
	p.setPropertyValue("PriorityStatProp", QVariant("Extreme"));
	QVERIFY2(p.propertyValue("PriorityStatProp").toString() == "Extreme", "Static enum property fail");

	auto enumValues = p.enumValues("PriorityStatProp");
	QVERIFY2(enumValues.size() == 6, "Static enum property fail");

	return;
}

void PropertyObjectTests::testDynamicProperties()
{

}

//
//
//		PropertyClass
//
//
PropertyClass::PropertyClass()
{
	auto boolProp = ADD_PROPERTY_GETTER_SETTER(bool, "BoolStatProp", true, PropertyClass::boolStatProp, PropertyClass::setBoolStatProp)
	auto intProp = ADD_PROPERTY_GETTER_SETTER(int, "IntStatProp", true, PropertyClass::intStatProp, PropertyClass::setIntStatProp)
	auto qstringProp = ADD_PROPERTY_GETTER_SETTER(QString, "QStringStatProp", true, PropertyClass::qstringStatProp, PropertyClass::setQStringStatProp)
	auto priorityProp = ADD_PROPERTY_GETTER_SETTER(Priority, "PriorityStatProp", true, PropertyClass::priorityStatProp, PropertyClass::setPriorityStatProp)

	Q_UNUSED(qstringProp);
	Q_UNUSED(priorityProp);

	boolProp->setCategory("TestCategory");
	intProp->setDescription("This is static int property");

}

PropertyClass::~PropertyClass()
{
}

bool PropertyClass::boolStatProp() const
{
	return m_boolStatProp;
}

void PropertyClass::setBoolStatProp(bool value)
{
	m_boolStatProp = value;
}


int PropertyClass::intStatProp() const
{
	return m_intStatProp;
}

void PropertyClass::setIntStatProp(int value)
{
	m_intStatProp = value;
}

QString PropertyClass::qstringStatProp() const
{
	return m_qstringStatProp;
}

void PropertyClass::setQStringStatProp(QString value)
{
	m_qstringStatProp = value;
}

PropertyClass::Priority PropertyClass::priorityStatProp() const
{
	return m_priorityStatProp;
}

void PropertyClass::setPriorityStatProp(PropertyClass::Priority value)
{
	m_priorityStatProp = value;
}
