#ifndef PROPERTYOBJECT
#define PROPERTYOBJECT

#include <type_traits>
#include <functional>
#include <map>
#include <list>
#include <vector>
#include <utility>
#include <assert.h>

#include <QObject>
#include <QString>
#include <QVariant>


// Add property which is stored in QVariant and does not have getter or setter
//
#define ADD_PROPERTY_QVARIANT(TYPE, NAME) \
	addProperty<TYPE>(tr(#NAME));

// Add property which has getter	QString caption() const;
void setCaption(QString value);
//
#define ADD_PROPERTY_GETTER(TYPE, NAME, GETTER) \
	addProperty<TYPE>(tr(#NAME), \
			(std::function<TYPE(void)>)std::bind(&GETTER, this));

// Add property which has getter and setter
//
#define ADD_PROPERTY_GETTER_SETTER(TYPE, NAME, GETTER, SETTER) \
	addProperty<TYPE>(tr(#NAME), \
			(std::function<TYPE(void)>)std::bind(&GETTER, this), \
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
	virtual bool isEnum() const = 0;
	virtual std::list<std::pair<int, QString>> enumValues() const = 0;

public:
	QString caption() const;
	void setCaption(QString value);

	QString description() const;
	void setDescription(QString value);

	bool readOnly() const;
	void setReadOnly(bool value);

	bool updateFromPreset() const;
	void setUpdateFromPreset(bool value);

	bool visible() const;
	void setVisible(bool value);

	int precision() const;
	void setPrecision(int value);

	virtual QVariant value() const = 0;
	virtual void setValue(const QVariant& value) = 0;

	virtual void setEnumValue(int value) = 0;
	virtual void setEnumValue(const char* value) = 0;

private:
	QString m_caption;
	QString m_description;
	QString m_category;
	bool m_readOnly = false;
	bool m_updateFromPreset = false;
	bool m_visible = false;
	int m_precision = 0;
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

	void setValue(const TYPE& value)				// Not virtual, is called from class ProprtyObject for direct access
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
				assert(m_setter);	// to control min value, and if getter set, setter must be also set

				if (m_getter() < m_lowLimit.value<TYPE>() && m_setter)
				{
					m_setter(m_lowLimit.value<TYPE>());
				}
			}
			else
			{
				// Theres is no setter or getter, it is dynamic property
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
				assert(m_setter);	// to control max value, and if getter set, setter must be also set

				if (m_getter() > m_highLimit.value<TYPE>() && m_setter)
				{
					m_setter(m_highLimit.value<TYPE>());
				}
			}
			else
			{
				// Theres is no setter or getter, it is dynamic property
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

private:
	QVariant m_value;
	QVariant m_lowLimit;
	QVariant m_highLimit;
	QVariant m_step;

	std::function<TYPE(void)> m_getter;
	std::function<void(TYPE)> m_setter;
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
									 std::function<TYPE(void)> getter = std::function<TYPE(void)>(),
									 std::function<void(TYPE)> setter = std::function<void(TYPE)>())
	{
		PropertyValue<TYPE>* property = new PropertyValue<TYPE>();

		property->setCaption(caption);
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
			delete alreadyExists->second;
			alreadyExists->second = property;
		}

		return property;
	}

	// Get all properties
	//
	std::vector<Property*> properties();

	// Get specific property by its caption,
	// return Property* or nullptr if property is not found
	//
	Property* propertyByCaption(QString caption);
	const Property* propertyByCaption(QString caption) const;

	// Get property value
	//
	QVariant propertyValue(QString caption) const;

	template <typename TYPE>
	bool setPropertyValue(QString caption, const TYPE& value)
	{
		auto it = m_properties.find(caption);

		if (it == m_properties.end())
		{
			return false;
		}

		Property* property = it->second;

		if (property->isEnum() == true)
		{
			assert(std::is_integral<TYPE>::value == true);

			property->setEnumValue(value);
			return true;
		}
		else
		{
			PropertyValue<TYPE>* propertyValue = dynamic_cast<PropertyValue<TYPE>*>(property);

			if (propertyValue == nullptr)
			{
				assert(propertyValue);
				return false;
			}

			propertyValue->setValue(value);
			return true;
		}

		return false;
	}

	bool setPropertyValue(QString caption, const char* value);
	bool setPropertyValue(QString caption, const QVariant& value);
	std::list<std::pair<int, QString>> enumValues(QString property);

private:
	std::map<QString, Property*> m_properties;
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
		ADD_PROPERTY_QVARIANT(int, varint);
		ADD_PROPERTY_GETTER(int, varintgetter, TestProp::someProp);
		ADD_PROPERTY_GETTER_SETTER(int, varintgs, TestProp::someProp, TestProp::setSomeProp);

		PropertyValue<int>* pi = addProperty<int>(tr("SomeProp"),
					(std::function<int(void)>)std::bind(&TestProp::someProp, this),
					std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
		pi->setLowLimit(-100);

		PropertyValue<int>* pdi = addProperty<int>(tr("SomeDynProp"));
		pdi->setLowLimit(-200);

		addProperty<bool>(tr("SomeBool"),
					(std::function<bool(void)>)std::bind(&TestProp::someBool, this),
					std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));

		addProperty<QString>(tr("SomeString"),
					std::bind(&TestProp::someString, this),
					std::bind(&TestProp::setSomeString, this, std::placeholders::_1));

		addProperty<TestEnum::Priority>(tr("Priority"),
					std::bind(&TestProp::priority, this),
					std::bind(&TestProp::setPriority, this, std::placeholders::_1));
	}

	int someProp() const				{		return m_someProp;		}
	void setSomeProp(int value)			{		m_someProp = value;		}

	bool someBool() const				{		return m_someBool;		}
	void setSomeBool(bool value)		{		m_someBool = value;		}

	QString someString() const			{		return m_someString;	}
	void setSomeString(QString value)	{		m_someString = value;	}

	void setPriority(TestEnum::Priority priority)	{	m_priority = priority;	}
	TestEnum::Priority priority() const				{	return m_priority;		}

private:
	int m_someProp = -1;
	bool m_someBool = false;
	QString m_someString;
	TestEnum::Priority m_priority = TestEnum::Priority::Low;
};


#endif // PROPERTYOBJECT



