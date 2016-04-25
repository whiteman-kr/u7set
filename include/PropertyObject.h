#ifndef PROPERTYOBJECT
#define PROPERTYOBJECT

#include <type_traits>
#include <functional>
#include <map>
#include <list>
#include <vector>
#include <utility>
#include <assert.h>
#include <memory>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMetaEnum>

#include "../include/OrderedHash.h"

namespace Proto
{
	class Property;
}

// Add property which is stored in QVariant and does not have getter or setter
//
#define ADD_PROPERTY_QVARIANT(TYPE, NAME, VISIBLE) \
	addProperty<TYPE>(tr(#NAME), VISIBLE);

// Add property which has getter	QString caption() const;
void setCaption(QString value);
//
#define ADD_PROPERTY_GETTER(TYPE, NAME, VISIBLE, GETTER) \
	addProperty<TYPE>(tr(#NAME), VISIBLE, \
			(std::function<TYPE(void)>)std::bind(&GETTER, this));

// Add property which has getter and setter
//
#define ADD_PROPERTY_GETTER_SETTER(TYPE, NAME, VISIBLE, GETTER, SETTER) \
	addProperty<TYPE>(tr(#NAME), VISIBLE, \
			(std::function<TYPE(void)>)std::bind(&GETTER, this), \
			std::bind(&SETTER, this, std::placeholders::_1));

// Add property which has getter, setter and category
//
#define ADD_PROPERTY_GET_SET_CAT(TYPE, NAME, CATEGORY, VISIBLE, GETTER, SETTER) \
	addProperty<TYPE>(\
			tr(#NAME), \
			CATEGORY,\
			VISIBLE,\
			(std::function<TYPE(void)>)std::bind(&GETTER, this), \
			std::bind(&SETTER, this, std::placeholders::_1));

// Add property which has getter and setter
//
#define ADD_PROPERTY_DYNAMIC_ENUM(NAME, VISIBLE, ENUMVALUES, GETTER, SETTER) \
	addDynamicEnumProperty(tr(#NAME), ENUMVALUES, VISIBLE, \
			(std::function<int(void)>)std::bind(&GETTER, this), \
			std::bind(&SETTER, this, std::placeholders::_1));

//
//
//			Class Property
//
//
class Property
{
public:
	Property();
	virtual ~Property();

public:
	void saveValue(::Proto::Property* protoProperty) const;
	bool loadValue(const ::Proto::Property& protoProperty);

	virtual bool isEnum() const = 0;
	virtual std::list<std::pair<int, QString>> enumValues() const = 0;

public:
	QString caption() const;
	void setCaption(QString value);

	QString description() const;
	void setDescription(QString value);

	QString category() const;
	void setCategory(QString value);

	QString validator() const;
	void setValidator(QString value);

	bool readOnly() const;
	void setReadOnly(bool value);

	bool updateFromPreset() const;
	void setUpdateFromPreset(bool value);

	bool specific() const;
	void setSpecific(bool value);

	bool visible() const;
	void setVisible(bool value);

	int precision() const;
	void setPrecision(int value);

	virtual QVariant value() const = 0;
	virtual void setValue(const QVariant& value) = 0;

	virtual void setEnumValue(int value) = 0;
	virtual void setEnumValue(const char* value) = 0;

    virtual const QVariant& lowLimit() const = 0;
    virtual void setLowLimit(const QVariant& value) = 0;

    virtual const QVariant& highLimit() const = 0;
    virtual void setHighLimit(const QVariant& value) = 0;

    virtual const QVariant& step() const = 0;
    virtual void setStep(const QVariant& value) = 0;

	virtual bool isTheSameType(Property* property) = 0;
	virtual void updateFromPreset(Property* presetProperty, bool updateValue) = 0;

protected:
	void copy(const Property* source);

private:
	// WARNING!!! If you add a field, do not forget to add it to void copy(const Property* source);
	//
	QString m_caption;
	QString m_description;
	QString m_category;
	QString m_validator;
	bool m_readOnly = false;
	bool m_updateFromPreset = false;		// Update this property from preset, used in DeviceObject
	bool m_specific = false;				// Specific property, used in DeviceObject
	bool m_visible = false;
	int m_precision = 2;
};

//
//
//			Class Property
//
//

template <typename TYPE>
class PropertyValue : public Property
{
public:
	PropertyValue()
	{
	}

	virtual ~PropertyValue()
	{
	}

public:
	virtual bool isEnum() const override
	{
		return std::is_enum<TYPE>::value;
	}

	// Tag Dispatch method for enum/not enum type, it does not allow to instantiate metaEnum for not enums
	// Example is in question:
	// http://stackoverflow.com/questions/6917079/tag-dispatch-versus-static-methods-on-partially-specialised-classes
	//
private:
	template <bool> struct enumness {};
	typedef enumness<true> enum_tag;
	typedef enumness<false> non_enum_tag;

	template <typename ENUM>
	const QMetaEnum metaEnum(enum_tag) const
	{
		QMetaEnum me = QMetaEnum::fromType<ENUM>();		// static_assert is here, that is why tag dispatch is used
		return me;
	}

	template <typename NOT_ENUM>
	const QMetaEnum metaEnum(non_enum_tag) const
	{
		assert(std::is_enum<NOT_ENUM>::value);			// Try to get QMetaEnum for not enum type
		return QMetaEnum();
	}

public:
	virtual std::list<std::pair<int, QString>> enumValues() const override
	{
		assert(std::is_enum<TYPE>::value == true);

		std::list<std::pair<int, QString>> result;

		QMetaEnum me = metaEnum<TYPE>(enumness<std::is_enum<TYPE>::value>());
		if (me.isValid() == false)
		{
			assert(me.isValid() == true);
			return result;
		}

		int keyCount = me.keyCount();
		for (int i = 0; i < keyCount; i++)
		{
			result.push_back(std::make_pair(me.value(i), QString::fromLocal8Bit(me.key(i))));
		}

		return result;
	}

public:
	virtual QVariant value() const override
	{
		if (m_getter)
		{
			QVariant result(QVariant::fromValue(m_getter()));
			return result;
		}
		else
		{
			return m_value;
		}
	}

	void setValueDirect(const TYPE& value)				// Not virtual, is called from class ProprtyObject for direct access
	{
		if (m_setter)
		{
			m_setter(value);
		}
		else
		{
			//assert(m_value.type() == value.type());
			m_value = QVariant::fromValue(value);
		}

		checkLimits();
	}

	void setValue(const QVariant& value) override	// Overriden from class Propery
	{
		if (m_setter)
		{
			assert(value.canConvert<TYPE>());
			m_setter(value.value<TYPE>());
		}
		else
		{
			if (m_value.isValid() == true)			// if m_value is not valid then it is initialization
			{
				assert(m_value.type() == value.type());
			}

			m_value = value;
		}

		checkLimits();
	}

	virtual void setEnumValue(int value) override	// Overriden from class Propery
	{
		setEnumValueInternal<TYPE>(value, enumness<std::is_enum<TYPE>::value>());
	}
	virtual void setEnumValue(const char* value) override	// Overriden from class Propery
	{
		return setEnumValueInternal<TYPE>(value, enumness<std::is_enum<TYPE>::value>());
	}

private:
	template <typename ENUM>
	void setEnumValueInternal(int value, enum_tag)				// Overriden from class Propery
	{
		assert(std::is_enum<TYPE>::value == true);

		if (m_setter)
		{
			m_setter(static_cast<TYPE>(value));
		}
		else
		{
			m_value = QVariant::fromValue(static_cast<TYPE>(value));
		}
	}
	template <typename NON_ENUM>
	void setEnumValueInternal(int value, non_enum_tag)
	{
		Q_UNUSED(value)
		assert(false);
	}

	template <typename ENUM>
	void setEnumValueInternal(const char* value, enum_tag)				// Overriden from class Propery
	{
		assert(std::is_enum<TYPE>::value == true);

		QVariant v(value);
		setValue(v);

		return;
	}
	template <typename NON_ENUM>
	void setEnumValueInternal(const char* value, non_enum_tag)
	{
		Q_UNUSED(value)
		assert(false);
	}

private:

	void checkLimits()
	{
		if (lowLimit().isValid() == true)
		{
			if (m_getter)
			{
				// Setter suppose to check limits
				//
			}
			else
			{
				// Theres is no setter or getter, it is specific property
				//
				assert(m_lowLimit.type() == m_value.type());

				if (m_value < m_lowLimit)
				{
					m_value = m_lowLimit;
				}
			}
		}

		if (highLimit().isValid() == true)
		{
			if (m_getter)
			{
				// Setter suppose to check limits
				//
			}
			else
			{
				// Theres is no setter or getter, it is specific property
				//
				assert(m_highLimit.type() == m_value.type());

				if (m_value > m_highLimit)
				{
					m_value = m_highLimit;
				}
			}
		}
	}

public:

	void setLimits(const QVariant& low, const QVariant& high)
	{
		setLowLimit(low);
		setHighLimit(high);
	}

	const QVariant& lowLimit() const
	{
		return m_lowLimit;
	}
	void setLowLimit(const QVariant& value)
	{
		m_lowLimit = value;
	}

	const QVariant& highLimit() const
	{
		return m_highLimit;
	}
	void setHighLimit(const QVariant& value)
	{
		m_highLimit = value;
	}

	const QVariant& step() const
	{
		return m_step;
	}
	void setStep(const QVariant& value)
	{
		m_step = value;
	}

	void setGetter(std::function<TYPE(void)> getter)
	{
		m_getter = getter;
	}
	void setSetter(std::function<void(TYPE)> setter)
	{
		m_setter = setter;
	}

	virtual bool isTheSameType(Property* property) override
	{
		if (dynamic_cast<PropertyValue<TYPE>*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override
	{
		if (presetProperty == nullptr ||
			isTheSameType(presetProperty) == false)
		{
			assert(presetProperty != nullptr);
			assert(isTheSameType(presetProperty) == true);
			return;
		}

		Property::copy(presetProperty);	// Copy data from the base class

		PropertyValue<TYPE>* source = dynamic_cast<PropertyValue<TYPE>*>(presetProperty);
		assert(source);

		if (updateValue == true)
		{
			setValue(presetProperty->value());
		}

		m_lowLimit = source->m_lowLimit;
		m_highLimit = source->m_highLimit;
		m_step = source->m_step;

		// Do not copy m_getter/m_setter, because the yare binded to own object instances
		//
		return;
	}

private:
	// WARNING!!! If you add a field, do not forget to add it to updateFromPreset();
	//
	QVariant m_value;
	QVariant m_lowLimit;
	QVariant m_highLimit;
	QVariant m_step;

	std::function<TYPE(void)> m_getter;
	std::function<void(TYPE)> m_setter;
};

//
//			Dynamic Enum Property
//			Class PropertyValue specialization for OrderedHash<int, QString>,
//			class behave like enum
//
//
template <>
class PropertyValue<OrderedHash<int, QString>> : public Property
{
public:
	PropertyValue(std::shared_ptr<OrderedHash<int, QString>> enumValues) :
		m_enumValues(enumValues)
	{
		assert(m_enumValues);
	}

	virtual ~PropertyValue()
	{
	}

public:
	virtual bool isEnum() const override
	{
		return true;	// This is dynamic enumeration
	}

	virtual std::list<std::pair<int, QString>> enumValues() const override
	{
		std::list<std::pair<int, QString>> result;

		auto values = m_enumValues->getKeyValueVector();

		for (auto i : values)
		{
			result.push_back(std::make_pair(i.first, i.second));
		}

		return result;
	}

public:
	virtual QVariant value() const override
	{
		if (m_getter)
		{
			QVariant result(QVariant::fromValue(m_getter()));
			return result;
		}
		else
		{
			return QVariant(m_value);
		}
	}

	void setValueDirect(const int& value)				// Not virtual, is called from class ProprtyObject for direct access
	{
		// setValueDirect is used for non enum types only
		//
		Q_UNUSED(value);
		return;
	}

	void setValue(const QVariant& value) override		// Overriden from class Propery
	{
		if (value.type() == QVariant::Int)
		{
			setEnumValue(value.value<int>());
			return;
		}

		if (value.type() == QVariant::String)
		{
			int key = m_enumValues->key(value.toString());
			setEnumValue(key);
			return;
		}

		assert(false);
	}

	virtual void setEnumValue(int value) override			// Overriden from class Propery
	{
		if (m_setter)
		{
			m_setter(value);
		}
		else
		{
			m_value = value;
		}
	}

	virtual void setEnumValue(const char* value) override	// Overriden from class Propery
	{
		int key = m_enumValues->key(QString(value));
		setEnumValue(key);
	}

private:

	void checkLimits()
	{
	}

public:

	void setLimits(const QVariant& low, const QVariant& high)
	{
		Q_UNUSED(low);
		Q_UNUSED(high);
	}

	const QVariant& lowLimit() const
	{
		return m_interanalUse;
	}
	void setLowLimit(const QVariant& value)
	{
		Q_UNUSED(value);
	}

	const QVariant& highLimit() const
	{
		return m_interanalUse;
	}
	void setHighLimit(const QVariant& value)
	{
		Q_UNUSED(value);
	}

	const QVariant& step() const
	{
		return m_interanalUse;
	}
	void setStep(const QVariant& value)
	{
		Q_UNUSED(value);
	}

	void setGetter(std::function<int(void)> getter)
	{
		m_getter = getter;
	}
	void setSetter(std::function<void(int)> setter)
	{
		m_setter = setter;
	}

	virtual bool isTheSameType(Property* property) override
	{
		if (dynamic_cast<PropertyValue<OrderedHash<int, QString>>*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override
	{
		// Implementation is not requiered yet
		// To do if requeired
		//
		Q_UNUSED(presetProperty);
		Q_UNUSED(updateValue);
	}

private:
	// WARNING!!! If you add a field, do not forget to add it to updateFromPreset();
	//
	std::shared_ptr<OrderedHash<int, QString>> m_enumValues;

	int m_value = 0;
	QVariant m_interanalUse;	//	for return from lowLimt, hughLimt, step

	std::function<int(void)> m_getter;
	std::function<void(int)> m_setter;
};



//
//
//			Class PropertyObject
//
//
class PropertyObject : public QObject
{
	Q_OBJECT

public:
	explicit PropertyObject(QObject* parent = nullptr);
	virtual ~PropertyObject();

public:

	// Add new property
	// Defines can be used:
	//		ADD_PROPERTY_QVARIANT(int, varint);
	//		ADD_PROPERTY_GETTER(int, varintgetter, TestProp::someProp);
	//		ADD_PROPERTY_GETTER_SETTER(int, varintgs, TestProp::someProp, TestProp::setSomeProp);
	//
	template <typename TYPE>
	PropertyValue<TYPE>* addProperty(QString caption,
									 bool visible = false,
									 std::function<TYPE(void)> getter = std::function<TYPE(void)>(),
									 std::function<void(TYPE)> setter = std::function<void(TYPE)>())
	{
        std::shared_ptr<PropertyValue<TYPE>> property = std::make_shared<PropertyValue<TYPE>>();

		property->setCaption(caption);
		property->setVisible(visible);
		property->setGetter(getter);
		property->setSetter(setter);

		if (!getter)
		{
			property->setValue(QVariant::fromValue(TYPE()));
		}

		if (!setter)
		{
			property->setReadOnly(true);
		}

		auto alreadyExists = m_properties.find(caption);

		if (alreadyExists == m_properties.end())
		{
			m_properties[caption] = property;
		}
		else
		{
			alreadyExists->second = property;
		}

        return property.get();
	}

	template <typename TYPE>
	PropertyValue<TYPE>* addProperty(QString caption,
									 QString category,
									 bool visible = false,
									 std::function<TYPE(void)> getter = std::function<TYPE(void)>(),
									 std::function<void(TYPE)> setter = std::function<void(TYPE)>())
	{
		std::shared_ptr<PropertyValue<TYPE>> property = std::make_shared<PropertyValue<TYPE>>();

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);
		property->setGetter(getter);
		property->setSetter(setter);

		if (!getter)
		{
			property->setValue(QVariant::fromValue(TYPE()));
		}

		if (!setter)
		{
			property->setReadOnly(true);
		}

		auto alreadyExists = m_properties.find(caption);

		if (alreadyExists == m_properties.end())
		{
			m_properties[caption] = property;
		}
		else
		{
			alreadyExists->second = property;
		}

		return property.get();
	}

	PropertyValue<OrderedHash<int, QString>>* addDynamicEnumProperty(
			QString caption,
			std::shared_ptr<OrderedHash<int, QString>> enumValues,
			bool visible = false,
			std::function<int(void)> getter = std::function<int(void)>(),
			std::function<void(int)> setter = std::function<void(int)>())
	{
		if (enumValues.get() == nullptr)
		{
			assert(enumValues);
			return nullptr;
		}

		std::shared_ptr<PropertyValue<OrderedHash<int, QString>>> property = std::make_shared<PropertyValue<OrderedHash<int, QString>>>(enumValues);

		property->setCaption(caption);
		property->setVisible(visible);
		property->setGetter(getter);
		property->setSetter(setter);

		if (!getter)
		{
			property->setValue(QVariant::fromValue(int()));
		}

		if (!setter)
		{
			property->setReadOnly(true);
		}

		auto alreadyExists = m_properties.find(caption);

		if (alreadyExists == m_properties.end())
		{
			m_properties[caption] = property;
		}
		else
		{
			alreadyExists->second = property;
		}

		return property.get();
	}

	// Get all properties
	//
	std::vector<std::shared_ptr<Property>> properties() const;

	// Delete all properties
	//
	void removeAllProperties();

	// Add properties
	// 1. If properties have getter or setter the must be added via PropertyObject::addProperty
	// because getter and setter are binded to this
	// 2. It is posible to use addProperties with getter and setter properties
	// if they were added via PropertyObject::addProperty and later removed by removeAllProperties
	//
	void addProperties(std::vector<std::shared_ptr<Property>> properties);

	// Delete all deynamic properties
	//
	void removeSpecificProperties();

    Q_INVOKABLE bool propertyExists(QString caption) const;

    // Get specific property by its caption,
	// return Property* or nullptr if property is not found
	//
    std::shared_ptr<Property> propertyByCaption(QString caption);
    const std::shared_ptr<Property> propertyByCaption(QString caption) const;

    // Get property value
	//
    Q_INVOKABLE QVariant propertyValue(QString caption) const;

	template <typename TYPE>
	bool setPropertyValue(QString caption, const TYPE& value)
	{
		auto it = m_properties.find(caption);

		if (it == m_properties.end())
		{
			return false;
		}

		std::shared_ptr<Property> property = it->second;

		if (property->isEnum() == true)
		{
			assert(std::is_integral<TYPE>::value == true);

			property->setEnumValue(value);
			return true;
		}
		else
		{
			PropertyValue<TYPE>* propertyValue = dynamic_cast<PropertyValue<TYPE>*>(property.get());

			if (propertyValue == nullptr)
			{
				assert(propertyValue);
				return false;
			}

			propertyValue->setValueDirect(value);
			return true;
		}

		return false;
	}

    bool setPropertyValue(QString caption, const char* value);
	Q_INVOKABLE bool setPropertyValue(QString caption, const QVariant& value);
	std::list<std::pair<int, QString>> enumValues(QString property);

private:
    std::map<QString, std::shared_ptr<Property>> m_properties;
};


//
//
//		Test and examples
//
//
class TestEnum : public QObject
{
	Q_OBJECT
public:

	enum Priority { VeryLow, Low, High, VeryHigh = 10 , Extreme = 12 };
	Q_ENUM(Priority)

};

class TestProp : public PropertyObject
{
	Q_OBJECT

public:
	TestProp()
	{
		ADD_PROPERTY_QVARIANT(int, varint, true);
		ADD_PROPERTY_GETTER(int, varintgetter, true, TestProp::someProp);
		ADD_PROPERTY_GETTER_SETTER(int, varintgs, true, TestProp::someProp, TestProp::setSomeProp);

		PropertyValue<int>* pi = addProperty<int>(
					tr("SomeProp"),
					true,
					(std::function<int(void)>)std::bind(&TestProp::someProp, this),
					std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
		pi->setLowLimit(-100);

		PropertyValue<int>* pdi = addProperty<int>(tr("SomeDynProp"));
		pdi->setLowLimit(-200);

		addProperty<bool>(tr("SomeBool"),
						  true,
						  (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
						  std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));

		addProperty<QString>(tr("SomeString"),
							 true,
							 std::bind(&TestProp::someString, this),
							 std::bind(&TestProp::setSomeString, this, std::placeholders::_1));

		addProperty<TestEnum::Priority>(tr("Priority"),
										true,
										std::bind(&TestProp::priority, this),
										std::bind(&TestProp::setPriority, this, std::placeholders::_1));

		std::shared_ptr<OrderedHash<int, QString>> enumValues = std::make_shared<OrderedHash<int, QString>>();
		enumValues->append(0, "Zero");
		enumValues->append(1, "One");
		enumValues->append(10, "Ten");

		PropertyValue<OrderedHash<int, QString>>* pu = addDynamicEnumProperty(
					tr("Units"),
					enumValues,
					true,
					(std::function<int(void)>)std::bind(&TestProp::units, this),
					std::bind(&TestProp::setUnits, this, std::placeholders::_1));

		pu->setValue(2);

		ADD_PROPERTY_DYNAMIC_ENUM(Units2, true, enumValues, TestProp::units, TestProp::setUnits);
	}

	int someProp() const				{		return m_someProp;		}
	void setSomeProp(int value)			{		m_someProp = value;		}

	int units() const					{		return m_units;		}
	void setUnits(int value)			{		m_units = value;		}

	bool someBool() const				{		return m_someBool;		}
	void setSomeBool(bool value)		{		m_someBool = value;		}

	QString someString() const			{		return m_someString;	}
	void setSomeString(QString value)	{		m_someString = value;	}

	void setPriority(TestEnum::Priority priority)	{	m_priority = priority;	}
	TestEnum::Priority priority() const				{	return m_priority;		}

private:
	int m_someProp = -1;
	int m_units = 0;
	bool m_someBool = false;
	QString m_someString;
	TestEnum::Priority m_priority = TestEnum::Priority::Low;
};


#endif // PROPERTYOBJECT



