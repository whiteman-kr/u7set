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

void PropertyObjectTests::testMethods()
{
	// Test PropertyObject::properties() const
	//
	{
		PropertyClass p;
		std::vector<std::shared_ptr<Property>> props = p.properties();
		QVERIFY2(props.size() == 6, "PropertyObject::properties() failed");
	}

	// Test PropertyObject::propertyExists() const
	//
	{
		PropertyClass p;
		QVERIFY2(p.propertyExists("BoolStatProp") == true, "PropertyObject::propertyExists() failed");
		QVERIFY2(p.propertyExists("BoolStatPropNoneExists") == false, "PropertyObject::propertyExists() failed");
	}

	// Test PropertyObject::removeAllProperties() const
	//
	{
		PropertyClass p;
		p.removeAllProperties();
		std::vector<std::shared_ptr<Property>> props = p.properties();
		QVERIFY2(props.size() == 0, "PropertyObject::removeAllProperties() failed");
	}

	// Test PropertyObject::removeProperty() const
	//
	{
		PropertyClass p;
		QVERIFY2(p.propertyExists("BoolStatProp") == true, "PropertyObject::removeProperty() failed");
		p.removeProperty("BoolStatProp");
		QVERIFY2(p.propertyExists("BoolStatProp") == false, "PropertyObject::removeProperty() failed");
	}


	// Test PropertyObject::removeSpecificProperties() const
	//
	{
		PropertyClass p;

		QVERIFY(p.propertyExists("QStringStatProp") == true);
		QVERIFY(p.propertyExists("PriorityStatProp") == true);

		p.removeSpecificProperties();

		QVERIFY2(p.propertyExists("QStringStatProp") == false, "PropertyObject::removeSpecificProperties() failed");
		QVERIFY2(p.propertyExists("PriorityStatProp") == false, "PropertyObject::removeSpecificProperties() failed");
	}

	// Test PropertyObject::addProperties(std::vector<std::shared_ptr<Property>> properties)
	//
	{
		PropertyClass p;

		std::vector<std::shared_ptr<Property>> props = p.properties();
		p.removeAllProperties();

		QVERIFY2(p.properties().size() == 0, "PropertyObject::removeAllProperties() failed");

		p.addProperties(props);
		QVERIFY2(p.properties().size() == props.size(), "PropertyObject::addProperties() failed");
	}

	// Test PropertyObject::propertyByCaption()
	//
	{
		PropertyClass p;

		std::shared_ptr<Property> prop = p.propertyByCaption("QStringStatProp");

		QVERIFY2(prop.get() != nullptr, "PropertyObject::propertyByCaption() failed");
		QVERIFY2(prop->caption() == "QStringStatProp", "PropertyObject::propertyByCaption() failed");
	}

	// Test PropertyObject::propertyByCaption() const
	//
	{
		const PropertyClass p;

		const std::shared_ptr<Property> prop = p.propertyByCaption("QStringStatProp");

		QVERIFY2(prop.get() != nullptr, "PropertyObject::propertyByCaption() const failed");
		QVERIFY2(prop->caption() == "QStringStatProp", "PropertyObject::propertyByCaption() const failed");
	}

	// Test PropertyObject::propertyValue() const
	//
	{
		PropertyClass p;

		int val = 0x175522;
		p.setIntStatProp(val);
		QVariant result = p.propertyValue("IntStatProp");

		QVERIFY2(result.type() == QVariant::Int, "PropertyObject::propertyValue() const failed, type mismatch");
		QVERIFY2(result.toInt() == val, "PropertyObject::propertyValue() const failed, wrong value");
	}

	// Test:
	//	template <typename TYPE>
	//	bool setPropertyValue(const QString& caption, const TYPE& value)
	//
	{
		PropertyClass p;

		int val = 0x175522;
		bool result = p.setPropertyValue<int>("IntStatProp", val);

		QVERIFY2(result == true, "PropertyObject::setPropertyValue(), Property IntStatProp not found");
		QVERIFY2(p.intStatProp() == val, "PropertyObject::setPropertyValue() failed");
	}

	// Test: const char*
	//	bool setPropertyValue(const QString& caption, const char* value)
	//
	{
		PropertyClass p;

		const char* val = "0x175522 Some C-Style String";
		bool result = p.setPropertyValue("QStringStatProp", val);

		QVERIFY2(result == true, "PropertyObject::setPropertyValue(), Property QStringStatProp not found");
		QVERIFY2(p.qstringStatProp() == QLatin1String(val), "PropertyObject::setPropertyValue() failed");
	}

	// Test: const char*
	//	bool setPropertyValue(const QString& caption, const char* value)
	//
	{
		PropertyClass p;

		const char* val = "Extreme";
		bool result = p.setPropertyValue("PriorityStatProp", val);

		QVERIFY2(result == true, "PropertyObject::setPropertyValue(), Property QStringStatProp not found");
		QVERIFY2(p.priorityStatProp() == PropertyClass::Priority::Extreme, "PropertyObject::setPropertyValue() failed");
	}

	// Test: const char*
	//	bool setPropertyValue(const QString& caption, const char* value)
	//
	{
		PropertyClass p;

		const char* val = "0x175522 Some C-Style String";
		bool result = p.setPropertyValue("QStringStatProp", val);

		QVERIFY2(result == true, "PropertyObject::setPropertyValue(), Property QStringStatProp not found");
		QVERIFY2(p.qstringStatProp() == QLatin1String(val), "PropertyObject::setPropertyValue() failed");
	}

	// Test: const QVariant& value
	//	bool setPropertyValue(const QString& caption, const QVariant& value)
	//
	{
		PropertyClass p;

		QVariant val;
		val.setValue(0x175522);

		bool result = p.setPropertyValue("IntStatProp", val);

		QVERIFY2(result == true, "PropertyObject::setPropertyValue(), Property IntStatProp not found");
		QVERIFY2(p.intStatProp() == val.toInt(), "PropertyObject::setPropertyValue() failed");
	}

	// Test: const QVariant& value
	//	bool setPropertyValue(const QString& caption, const QVariant& value)
	//
	{
		PropertyClass p;

		QVariant val = QVariant::fromValue(PropertyClass::Priority::High);
		bool result = p.setPropertyValue("PriorityStatProp", val);

		QVERIFY2(result == true, "PropertyObject::setPropertyValue(), Property PriorityStatProp not found");
		QVERIFY2(p.priorityStatProp() == PropertyClass::Priority::High, "PropertyObject::setPropertyValue() failed");

		val = "Low";
		p.setPropertyValue("PriorityStatProp", val);
		QVERIFY2(p.priorityStatProp() == PropertyClass::Priority::Low, "PropertyObject::setPropertyValue() failed");
	}


	// Test: enumValues
	//
	{
		PropertyClass p;

		std::list<std::pair<int, QString>> result = p.enumValues("PriorityStatProp");

		QVERIFY2(result.size() == 6, "PropertyObject::enumValues(), Property PriorityStatProp not found");

		for (std::pair<int, QString> e : result)
		{
			switch (static_cast<PropertyClass::Priority>(e.first))
			{
			case PropertyClass::Priority::VeryLow:
				QVERIFY2(e.second == "VeryLow", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::Low:
				QVERIFY2(e.second == "Low", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::Normal:
				QVERIFY2(e.second == "Normal", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::High:
				QVERIFY2(e.second == "High", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::VeryHigh:
				QVERIFY2(e.second == "VeryHigh", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::Extreme:
				QVERIFY2(e.second == "Extreme", "PropertyObject::enumValues() result error");
				break;
			default:
				QVERIFY2(false, "PropertyObject::enumValues() result error");
			}
		}
	}

	// Test: enumValues
	//
	{
		PropertyClass p;

		p.setPropertyValue("DynamicPriority", PropertyClass::Priority::Low);		// Set enum type by this call

		std::list<std::pair<int, QString>> result = p.enumValues("DynamicPriority");

		QVERIFY2(result.size() == 6, "PropertyObject::enumValues(), Property PriorityStatProp not found");

		for (std::pair<int, QString> e : result)
		{
			switch (static_cast<PropertyClass::Priority>(e.first))
			{
			case PropertyClass::Priority::VeryLow:
				QVERIFY2(e.second == "VeryLow", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::Low:
				QVERIFY2(e.second == "Low", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::Normal:
				QVERIFY2(e.second == "Normal", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::High:
				QVERIFY2(e.second == "High", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::VeryHigh:
				QVERIFY2(e.second == "VeryHigh", "PropertyObject::enumValues() result error");
				break;
			case PropertyClass::Priority::Extreme:
				QVERIFY2(e.second == "Extreme", "PropertyObject::enumValues() result error");
				break;
			default:
				QVERIFY2(false, "PropertyObject::enumValues() result error");
			}
		}
	}

	return;
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
	// Test PropertyObject::setPropertyValue() const
	//
	{
		PropertyClass p;

		int ival = 0x11223344;

		p.setPropertyValue("DynamicInt", ival);
		QVariant value = p.propertyValue("DynamicInt");

		QVERIFY2(value.isValid() == true, "DynamicProperties setValue failed");
		QVERIFY2(value.type() == QVariant::Int, "DynamicProperties setValue failed, wrong type");
		QVERIFY2(value.toInt() == ival, "DynamicProperties setValue failed, wrong type");
	}

	// Test PropertyObject::setPropertyValue() const
	//
	{2 сек

		PropertyClass p;

		QVariant ival(0x11223344);

		p.setPropertyValue("DynamicInt", ival);
		QVariant value = p.propertyValue("DynamicInt");

		QVERIFY2(value.isValid() == true, "DynamicProperties setValue failed");
		QVERIFY2(value.type() == QVariant::Int, "DynamicProperties setValue failed, wrong type");
		QVERIFY2(value.toInt() == ival.toInt(), "DynamicProperties setValue failed, wrong type");
	}

	// Setting enum values, via type, int and char*
	//
	{
		PropertyClass p;

		p.setPropertyValue("DynamicPriority", PropertyClass::Priority::High);
		QVERIFY(p.propertyValue("DynamicPriority").value<PropertyClass::Priority>() == PropertyClass::Priority::High);

		p.setPropertyValue("DynamicPriority", "Low");
		QVERIFY(p.propertyValue("DynamicPriority").value<PropertyClass::Priority>() == PropertyClass::Priority::Low);

		p.setPropertyValue("DynamicPriority", static_cast<int>(PropertyClass::Priority::Extreme));
		QVERIFY(p.propertyValue("DynamicPriority").value<PropertyClass::Priority>() == PropertyClass::Priority::Extreme);
	}
}

//
//
//		PropertyClass -- test class
//
//
PropertyClass::PropertyClass()
{
	auto boolProp = ADD_PROPERTY_GETTER_SETTER(bool, "BoolStatProp", true, PropertyClass::boolStatProp, PropertyClass::setBoolStatProp)
	auto intProp = ADD_PROPERTY_GETTER_SETTER(int, "IntStatProp", true, PropertyClass::intStatProp, PropertyClass::setIntStatProp)
	auto qstringProp = ADD_PROPERTY_GETTER_SETTER(QString, "QStringStatProp", true, PropertyClass::qstringStatProp, PropertyClass::setQStringStatProp)
	auto priorityProp = ADD_PROPERTY_GETTER_SETTER(Priority, "PriorityStatProp", true, PropertyClass::priorityStatProp, PropertyClass::setPriorityStatProp)

	qstringProp->setSpecific(true);
	priorityProp->setSpecific(true);

	boolProp->setCategory("TestCategory");
	intProp->setDescription("This is static int property");

    /*auto intDynamicProp = addProperty("DynamicInt", "DynamicCat", true); */
    /*auto prioDynamicProp = addProperty("DynamicPriority", "DynamicCat", true);*/
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
