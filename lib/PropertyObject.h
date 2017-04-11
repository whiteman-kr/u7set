#ifndef PROPERTYOBJECT
#define PROPERTYOBJECT

#include <type_traits>
#include <functional>
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <utility>
#include <cassert>
#include <memory>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMetaEnum>
#include <QHash>
#include <QTime>
#include <QTimer>
#include <QDebug>

#include "../lib/OrderedHash.h"
//#include "../lib/PlainObjectHeap.h"

//extern poh::PlainObjectHeap thePropertyObjectHeap;

class PropertyObject;

// Add property which is stored in QVariant and does not have getter or setter
//
#define ADD_PROPERTY_QVARIANT(TYPE, NAME, VISIBLE) \
	addProperty<TYPE>(NAME, VISIBLE);

// Add property which has getter	QString caption() const;
void setCaption(QString value);
//

#define ADD_PROPERTY_GETTER(TYPE, NAME, VISIBLE, GETTER) \
	addProperty<TYPE>(NAME, QString(), VISIBLE, \
			(std::function<TYPE(void)>)std::bind(&GETTER, this));

#define ADD_PROPERTY_GETTER_INDIRECT(TYPE, NAME, VISIBLE, GETTER, OWNER) \
	addProperty<TYPE>(NAME, QString(), VISIBLE, \
			(std::function<TYPE(void)>)std::bind(&GETTER, &OWNER));

// Add property which has getter and setter
//
#define ADD_PROPERTY_GETTER_SETTER(TYPE, NAME, VISIBLE, GETTER, SETTER) \
	addProperty<TYPE>(NAME, QString(), VISIBLE, \
			(std::function<TYPE(void)>)std::bind(&GETTER, this), \
			std::bind(&SETTER, this, std::placeholders::_1));

#define ADD_PROPERTY_GETTER_SETTER_INDIRECT(TYPE, NAME, VISIBLE, GETTER, SETTER, OWNER) \
	addProperty<TYPE>(NAME, QString(), VISIBLE, \
			(std::function<TYPE(void)>)std::bind(&GETTER, &OWNER), \
			std::bind(&SETTER, &OWNER, std::placeholders::_1));

// Add property which has getter, setter and category
//
#define ADD_PROPERTY_GET_SET_CAT(TYPE, NAME, CATEGORY, VISIBLE, GETTER, SETTER) \
	addProperty<TYPE>(\
			NAME, \
			CATEGORY,\
			VISIBLE,\
			(std::function<TYPE(void)>)std::bind(&GETTER, this), \
			std::bind(&SETTER, this, std::placeholders::_1));

// Add property which has getter and setter
//
#define ADD_PROPERTY_DYNAMIC_ENUM(NAME, VISIBLE, ENUMVALUES, GETTER, SETTER) \
	addDynamicEnumProperty(NAME, ENUMVALUES, VISIBLE, \
			(std::function<int(void)>)std::bind(&GETTER, this), \
			std::bind(&SETTER, this, std::placeholders::_1));

#define ADD_PROPERTY_DYNAMIC_ENUM_INDIRECT(NAME, VISIBLE, ENUMVALUES, GETTER, SETTER, OWNER) \
	addDynamicEnumProperty(NAME, ENUMVALUES, VISIBLE, \
			(std::function<int(void)>)std::bind(&GETTER, &OWNER), \
			std::bind(&SETTER, &OWNER, std::placeholders::_1));

//
//
//			Class Property
//
//
class Property
{
public:
	Property() : m_flags(0)
	{
	}

	virtual ~Property()
	{
	}

public:
	virtual bool isEnum() const = 0;
	virtual std::list<std::pair<int, QString>> enumValues() const = 0;

public:
	QString caption() const
	{
		return m_caption;
	}
	void setCaption(const QString& value)
	{
		m_caption = value;
	}

	QString description() const
	{
		return m_description;
	}
	void setDescription(const QString& value)
	{
		m_description = value;
	}

	QString category() const
	{
		return m_category;
	}
	void setCategory(const QString& value)
	{
		m_category = value;
	}

	QString validator() const
	{
		return m_validator;
	}
	void setValidator(const QString& value)
	{
		m_validator = value;
	}

	bool readOnly() const
	{
		return m_readOnly;
	}
	void setReadOnly(bool value)
	{
		m_readOnly = value;
	}

	bool updateFromPreset() const
	{
		return m_updateFromPreset;
	}
	void setUpdateFromPreset(bool value)
	{
		m_updateFromPreset = value;
	}

	bool specific() const
	{
		return m_specific;
	}
	void setSpecific(bool value)
	{
		m_specific = value;
	}

	bool visible() const
	{
		return m_visible;
	}
	void setVisible(bool value)
	{
		m_visible = value;
	}

	bool expert() const
	{
		return m_expert;
	}
	void setExpert(bool value)
	{
		m_expert = value;
	}

    bool password() const
    {
        return m_password;
    }
    void setPassword(bool value)
    {
        m_password = value;
    }

	bool isScript() const
	{
		return m_isScript;
	}
	void setIsScript(bool value)
	{
		m_isScript = value;
	}

    int precision() const
	{
		return m_precision;
	}
	void setPrecision(int value)
	{
		if (value < 0)
		{
			assert(value >= 0);
			value = 0;
		}

		m_precision = value;
	}

	virtual QVariant value() const = 0;
	virtual void setValue(const QVariant& value) = 0;

	virtual void setEnumValue(int value) = 0;
	virtual void setEnumValue(const char* value) = 0;

	virtual const QVariant& lowLimit() const = 0;
	virtual void setLowLimit(const QVariant& value) = 0;

	virtual const QVariant& highLimit() const = 0;
	virtual void setHighLimit(const QVariant& value) = 0;

	virtual bool isTheSameType(Property* property) = 0;
	virtual void updateFromPreset(Property* presetProperty, bool updateValue) = 0;

protected:
	void copy(const Property* source)
	{
		if (source == nullptr ||
			source == this)
		{
			assert(source);
			return;
		}

		m_caption = source->m_caption;
		m_description = source->m_description;
		m_category = source->m_category;
		m_validator = source->m_validator;
		m_flags = source->m_flags;
		m_precision = source->m_precision;

		return;
	}

private:
	// WARNING!!! If you add a field, do not forget to add it to void copy(const Property* source);
	//
	QString m_caption;
	QString m_description;
	QString m_category;
	QString m_validator;

	union
	{
		struct
		{
			bool m_readOnly : 1;
			bool m_updateFromPreset : 1;		// Update this property from preset, used in DeviceObject
			bool m_specific : 1;				// Specific property, used in DeviceObject
			bool m_visible : 1;
			bool m_expert : 1;
            bool m_password : 1;
			bool m_isScript : 1;
		};
		uint32_t m_flags;
	};

	int m_precision = 2;
};

//
//
//			Class PropertyValue
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
		if (std::is_enum<TYPE>::value)
		{
			return true;
		}

		return false;
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

		assert(false);
		return QVariant();
	}

	void setValueDirect(const TYPE& value)				// Not virtual, is called from class ProprtyObject for direct access
	{
		if (m_setter)
		{
			m_setter(value);
		}
		else
		{
			assert(false);
		}
	}

	void setValue(const QVariant& value) override	// Overriden from class Propery
	{
		if (!m_setter)
		{
			//assert(false);
			qDebug() << "Set property value for property without Setter: " << caption();
		}
		else
		{
			assert(value.canConvert<TYPE>());
			m_setter(value.value<TYPE>());
		}
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
			assert(false);
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

public:
	virtual const QVariant& lowLimit() const override
	{
		// Limits must be checked in getter/setter
		//
		assert(false);
static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setLowLimit(const QVariant&) override
	{
		// Limits must be checked in getter/setter
		assert(false);
	}

	virtual const QVariant& highLimit() const override
	{
		// Limits must be checked in getter/setter
		//
		assert(false);
static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setHighLimit(const QVariant&) override
	{
		// Limits must be checked in getter/setter
		assert(false);
	}

	void setGetter(const std::function<TYPE(void)>& getter)
	{
		m_getter = getter;
	}
	void setSetter(const std::function<void(TYPE)>& setter)
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

		if (source == nullptr)
		{
			assert(source);
			return;
		}

		if (updateValue == true)
		{
			setValue(presetProperty->value());
		}

		// Do not copy m_getter/m_setter, because the yare binded to own object instances
		//
		return;
	}

private:
	// WARNING!!! If you add a field, do not forget to add it to updateFromPreset();
	//
	std::function<TYPE(void)> m_getter;
	std::function<void(TYPE)> m_setter;
};


//
//
//			Class PropertyValue
//
//
class PropertyValueNoGetterSetter : public Property
{
public:
	PropertyValueNoGetterSetter()
	{
	}

	virtual ~PropertyValueNoGetterSetter()
	{
	}

public:
	virtual bool isEnum() const override
	{
		if (m_value.isValid() == false)
		{
			return false;
		}

		// Enum can be inside QVariant
		//
		const QMetaObject* mo = QMetaType::metaObjectForType(m_value.userType());
		if (mo == nullptr)
		{
			return false;
		}

		const char* typeName = m_value.typeName();
		QString typeStr(typeName);
		QStringRef typeNameRef(&typeStr);

		int doubleColumnIndex = typeNameRef.lastIndexOf("::");
		if (doubleColumnIndex != -1)
		{
			typeNameRef = typeNameRef.mid(doubleColumnIndex + 2);
		}

		int enumIndex = mo->indexOfEnumerator(typeNameRef.toLocal8Bit());

		if (enumIndex == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

public:
	virtual std::list<std::pair<int, QString>> enumValues() const override
	{
		std::list<std::pair<int, QString>> result;

		if (isEnum() == false)
		{
			assert(isEnum());
			return result;
		}

		if (m_value.isValid() == true)
		{
			// Enum can be inside QVariant
			//
			const QMetaObject* mo = QMetaType::metaObjectForType(m_value.userType());
			if (mo == nullptr)
			{
				assert(mo);
				return result;
			}

			const char* typeName = m_value.typeName();
			QString typeStr(typeName);
			QStringRef typeNameRef(&typeStr);

			int doubleColumnIndex = typeNameRef.lastIndexOf("::");
			if (doubleColumnIndex != -1)
			{
				typeNameRef = typeNameRef.mid(doubleColumnIndex + 2);
			}

			int enumIndex = mo->indexOfEnumerator(typeNameRef.toLocal8Bit());
			if (enumIndex != -1)
			{
				QMetaEnum me = mo->enumerator(enumIndex);

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
			}
		}

		return result;
	}

public:
	virtual QVariant value() const override
	{
		return m_value;
	}

	void setValue(const QVariant& value) override	// Overriden from class Propery
	{
		if (m_value.isValid() == false)
		{
			// It is initialization
			//
			m_value = value;
			return;
		}

		if (value.canConvert(m_value.type()) == true)
		{
			QVariant v(value);

			bool ok = v.convert(m_value.type());
			assert(ok);
			Q_UNUSED(ok);

			m_value.setValue(v);

			checkLimits();
			return;
		}

		if (value.canConvert(m_value.userType()) == true)
		{
			QVariant v(value);

			bool ok = v.convert(m_value.userType());
			assert(ok);
			Q_UNUSED(ok);

			m_value.setValue(v);
			checkLimits();
			return;
		}

		assert(m_value.type() == value.type());

		m_value = value;

		checkLimits();
	}

	class QVariantEx : public QVariant
	{
	public:
		void setEnumHack(int value)
		{
			this->d.data.i = value;
		}
	};

	virtual void setEnumValue(int value) override	// Overriden from class Propery
	{
		// cannot implement it now, but it's possible
		// The problem is, I dont see the way QVariant with enum inside can be set from integer and keep that enum type
		// QVariant::canConvert from int to enum returns false (for QString its ok)
		//
		QVariantEx* vex = static_cast<QVariantEx*>(&m_value);
		vex->setEnumHack(value);

		return;
	}

	virtual void setEnumValue(const char* value) override	// Overriden from class Propery
	{
		QVariant v(value);

		if (v.canConvert(m_value.userType()) == true)
		{
			QVariant v(value);

			bool ok = v.convert(m_value.userType());
			assert(ok);
			Q_UNUSED(ok);

			m_value.setValue(v);
		}
		else
		{
			assert(false);
		}
	}

private:
	void checkLimits()
	{
		if (m_lowLimit.isValid() == true)
		{
			assert(m_lowLimit.type() == m_value.type());

			if (m_value < m_lowLimit)
			{
				m_value = m_lowLimit;
			}
		}

		if (m_highLimit.isValid() == true)
		{
			assert(m_highLimit.type() == m_value.type());

			if (m_value > m_highLimit)
			{
				m_value = m_highLimit;
			}
		}
	}

public:
	void setLimits(const QVariant& low, const QVariant& high)
	{
		m_lowLimit = low;
		m_highLimit = high;
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

	virtual bool isTheSameType(Property* property) override
	{
		if (dynamic_cast<PropertyValueNoGetterSetter*>(property) == nullptr)
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

		PropertyValueNoGetterSetter* source = dynamic_cast<PropertyValueNoGetterSetter*>(presetProperty);
		if (source == nullptr)
		{
			assert(source);
			return;
		}

		if (updateValue == true)
		{
			m_value = source->m_value;
		}

		m_lowLimit = source->m_lowLimit;
		m_highLimit = source->m_highLimit;

		return;
	}

private:
	// WARNING!!! If you add a field, do not forget to add it to updateFromPreset();
	//
	QVariant m_value;
	QVariant m_lowLimit;
	QVariant m_highLimit;
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
static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	void setLowLimit(const QVariant& value)
	{
		Q_UNUSED(value);
	}

	const QVariant& highLimit() const
	{
static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	void setHighLimit(const QVariant& value)
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
	explicit PropertyObject(QObject* parent = nullptr)  :
		QObject(parent)
	{
	}

	PropertyObject(const PropertyObject& src) :
		QObject(src.parent())
	{
		// Shallow copy of properties
		//
		m_properties = src.m_properties;
	}

	virtual ~PropertyObject()
	{
	}

public:

	// Add new property
	// Defines can be used:
	//		ADD_PROPERTY_QVARIANT(int, varint);
	//		ADD_PROPERTY_GETTER(int, varintgetter, TestProp::someProp);
	//		ADD_PROPERTY_GETTER_SETTER(int, varintgs, TestProp::someProp, TestProp::setSomeProp);
	//
	template <typename TYPE>
	PropertyValue<TYPE>* addProperty(const QString& caption,
									 const QString& category,
									 bool visible,
									 std::function<TYPE(void)> getter,
									 std::function<void(TYPE)> setter = std::function<void(TYPE)>())
	{
		if (!getter)
		{
			assert(getter);
			return nullptr;
		}

		//std::shared_ptr<PropertyValue<TYPE>> property = thePropertyObjectHeap.alloc<PropertyValue<TYPE>>();
		std::shared_ptr<PropertyValue<TYPE>> property = std::make_shared<PropertyValue<TYPE>>();

		uint hash = qHash(caption);

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);
		property->setGetter(getter);
		property->setSetter(setter);

		if (!setter)
		{
			property->setReadOnly(true);
		}

		m_properties[hash] = property;

		return property.get();
	}

	PropertyValueNoGetterSetter* addProperty(const QString& caption,
											 const QString& category,
											 bool visible)
	{
		//std::shared_ptr<PropertyValueNoGetterSetter> property = thePropertyObjectHeap.alloc<PropertyValueNoGetterSetter>();
		std::shared_ptr<PropertyValueNoGetterSetter> property = std::make_shared<PropertyValueNoGetterSetter>();

		uint hash = qHash(caption);

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);

		m_properties[hash] = property;

		return property.get();
	}

	PropertyValue<OrderedHash<int, QString>>* addDynamicEnumProperty(
			const QString& caption,
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

		uint hash = qHash(caption);

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

		m_properties[hash] = property;

		return property.get();
	}

	// Get all properties
	//
	std::vector<std::shared_ptr<Property>> properties() const
	{
		std::vector<std::shared_ptr<Property>> result;
		result.reserve(m_properties.size());

		for (const std::pair<uint, std::shared_ptr<Property>>& p : m_properties)
		{
			result.push_back(p.second);
		}

		return result;
	}

	Q_INVOKABLE bool propertyExists(const QString& caption) const
	{
		uint hash = qHash(caption);
		auto it = m_properties.find(hash);
		return it != m_properties.end();
	}

	void removeAllProperties()
	{
		m_properties.clear();
	}

	bool removeProperty(const QString& caption)
	{
		uint hash = qHash(caption);
		size_t removed = m_properties.erase(hash);
		return removed > 0;
	}

	// Delete all specific properties
	//
	void removeSpecificProperties()
	{
		for(auto it = m_properties.begin(); it != m_properties.end();)
		{
			if(it->second->specific() == true)
			{
				it = m_properties.erase(it);
			}
			else
			{
				++it;
			}
		}
	}


	// Add properties
	// 1. If properties have getter or setter they must be added via PropertyObject::addProperty
	// because getter and setter are binded to this
	// 2. It is posible to use addProperties with getter and setter properties
	// if they were added via PropertyObject::addProperty and later removed by removeAllProperties
	//
	void addProperties(std::vector<std::shared_ptr<Property>> properties)
	{
		for (std::shared_ptr<Property> p : properties)
		{
			uint hash = qHash(p->caption());
			m_properties[hash] = p;
		}
	}


	// Get specific property by its caption,
	// return Property* or nullptr if property is not found
	//
	std::shared_ptr<Property> propertyByCaption(const QString& caption)
	{
		std::shared_ptr<Property> result = nullptr;

		uint hash = qHash(caption);
		auto it = m_properties.find(hash);

		if (it != m_properties.end())
		{
			result = it->second;
		}

		return result;
	}

	const std::shared_ptr<Property> propertyByCaption(const QString& caption) const
	{
		std::shared_ptr<Property> result = nullptr;

		uint hash = qHash(caption);
		auto it = m_properties.find(hash);

		if (it != m_properties.end())
		{
			result = it->second;
		}

		return result;
	}

	Q_INVOKABLE QVariant propertyValue(const QString& caption) const
	{
		uint hash = qHash(caption);
		auto it = m_properties.find(hash);

		if (it != m_properties.end())
		{
			QVariant result(it->second->value());
			return result;
		}
		else
		{
			qDebug() << "PropertyObject::propertyValue: property not found: " << caption;
			return QVariant();
		}
	}

	template <typename TYPE>
	bool setPropertyValue(const QString& caption, const TYPE& value)
	{
		uint hash = qHash(caption);
		auto it = m_properties.find(hash);

		if (it == m_properties.end())
		{
			return false;
		}

		Property* property = it->second.get();

		if (property->isEnum() == true)
		{
			assert(std::is_integral<TYPE>::value == true);

			property->setEnumValue(value);
			return true;
		}
		else
		{
			PropertyValue<TYPE>* propertyValue = dynamic_cast<PropertyValue<TYPE>*>(property);

			if (propertyValue != nullptr)
			{
				propertyValue->setValueDirect(value);
			}
			else
			{
				property->setValue(QVariant::fromValue(value));
			}

			return true;
		}

		return false;
	}

	bool setPropertyValue(const QString& caption, const char* value)
	{
		uint hash = qHash(caption);
		auto it = m_properties.find(hash);

		if (it == m_properties.end())
		{
			return false;
		}

		std::shared_ptr<Property> property = it->second;

		if (property->isEnum() == true)
		{
			property->setEnumValue(value);
			return true;
		}
		else
		{
			PropertyValue<QString>* propertyValue = dynamic_cast<PropertyValue<QString>*>(property.get());

			if (propertyValue == nullptr)
			{
				assert(propertyValue);
				return false;
			}

			propertyValue->setValueDirect(QString(value));

			return true;
		}

		return false;
	}

	Q_INVOKABLE bool setPropertyValue(const QString& caption, const QVariant& value)
	{
		uint hash = qHash(caption);
		auto it = m_properties.find(hash);

		if (it == m_properties.end())
		{
			return false;
		}

		it->second->setValue(value);

		return true;
	}

	std::list<std::pair<int, QString>> enumValues(const QString& property)
	{
		std::list<std::pair<int, QString>> result;

		uint hash = qHash(property);
		auto it = m_properties.find(hash);

		if (it != m_properties.end())
		{
			result = it->second->enumValues();
		}

		return result;
	}

private:
	std::map<uint, std::shared_ptr<Property>> m_properties;		// key is property caption hash qHash(QString)
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

//class TestProp : public PropertyObject
//{
//	Q_OBJECT

//public:
//	TestProp()
//	{
//		static QString BoolProp0_Caption("BoolProp0");
//		static QString BoolProp1_Caption("BoolProp1");
//		static QString BoolProp2_Caption("BoolProp2");
//		static QString BoolProp3_Caption("BoolProp3");
//		static QString BoolProp4_Caption("BoolProp4");
//		static QString BoolProp5_Caption("BoolProp5");
//		static QString BoolProp6_Caption("BoolProp6");
//		static QString BoolProp7_Caption("BoolProp7");
//		static QString BoolProp8_Caption("BoolProp8");
//		static QString BoolProp9_Caption("BoolProp9");

//		static QString IntProp0_Caption("IntProp0");
//		static QString IntProp1_Caption("IntProp1");
//		static QString IntProp2_Caption("IntProp2");
//		static QString IntProp3_Caption("IntProp3");
//		static QString IntProp4_Caption("IntProp4");
//		static QString IntProp5_Caption("IntProp5");
//		static QString IntProp6_Caption("IntProp6");
//		static QString IntProp7_Caption("IntProp7");
//		static QString IntProp8_Caption("IntProp8");
//		static QString IntProp9_Caption("IntProp9");

//		static QString StringProp0_Caption("StringProp0");
//		static QString StringProp1_Caption("StringProp1");
//		static QString StringProp2_Caption("StringProp2");
//		static QString StringProp3_Caption("StringProp3");
//		static QString StringProp4_Caption("StringProp4");
//		static QString StringProp5_Caption("StringProp5");
//		static QString StringProp6_Caption("StringProp6");
//		static QString StringProp7_Caption("StringProp7");
//		static QString StringProp8_Caption("StringProp8");
//		static QString StringProp9_Caption("StringProp9");

//		addProperty<bool>(BoolProp0_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp1_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp2_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp3_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp4_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp5_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp6_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp7_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp8_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));
//		addProperty<bool>(BoolProp9_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
//				std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));

//		addProperty<int>(IntProp0_Caption, true, (std::function<bool(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp1_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp2_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp3_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp4_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp5_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp6_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp7_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp8_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
//		addProperty<int>(IntProp9_Caption, true, (std::function<int(void)>)std::bind(&TestProp::someProp, this),
//				std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));

//		addProperty<QString>(StringProp0_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp1_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp2_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp3_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp4_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp5_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp6_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp7_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp8_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));
//		addProperty<QString>(StringProp9_Caption, true, (std::function<QString(void)>)std::bind(&TestProp::someString, this),
//				std::bind(&TestProp::setSomeString, this, std::placeholders::_1));

////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp0, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp1, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp2, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp3, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp4, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp5, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp6, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp7, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp8, true, TestProp::someBool, TestProp::setSomeBool);
////		ADD_PROPERTY_GETTER_SETTER(bool, BoolProp9, true, TestProp::someBool, TestProp::setSomeBool);

////		ADD_PROPERTY_GETTER_SETTER(int, IntProp1, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp2, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp0, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp3, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp4, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp5, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp6, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp7, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp8, true, TestProp::someProp, TestProp::setSomeProp);
////		ADD_PROPERTY_GETTER_SETTER(int, IntProp9, true, TestProp::someProp, TestProp::setSomeProp);

////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp0, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp1, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp2, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp3, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp4, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp5, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp6, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp7, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp8, true, TestProp::someString, TestProp::setSomeString);
////		ADD_PROPERTY_GETTER_SETTER(QString, StringProp9, true, TestProp::someString, TestProp::setSomeString);

////		ADD_PROPERTY_QVARIANT(int, varint, true);
////		ADD_PROPERTY_GETTER(int, varintgetter, true, TestProp::someProp);
////		ADD_PROPERTY_GETTER_SETTER(int, varintgs, true, TestProp::someProp, TestProp::setSomeProp);

////		PropertyValue<int>* pi = addProperty<int>(
////					tr("SomeProp"),
////					true,
////					(std::function<int(void)>)std::bind(&TestProp::someProp, this),
////					std::bind(&TestProp::setSomeProp, this, std::placeholders::_1));
////		pi->setLowLimit(-100);

////		PropertyValue<int>* pdi = addProperty<int>(tr("SomeDynProp"));
////		pdi->setLowLimit(-200);

////		addProperty<bool>(tr("SomeBool"),
////						  true,
////						  (std::function<bool(void)>)std::bind(&TestProp::someBool, this),
////						  std::bind(&TestProp::setSomeBool, this, std::placeholders::_1));

////		addProperty<QString>(tr("SomeString"),
////							 true,
////							 std::bind(&TestProp::someString, this),
////							 std::bind(&TestProp::setSomeString, this, std::placeholders::_1));

////		addProperty<TestEnum::Priority>(tr("Priority"),
////										true,
////										std::bind(&TestProp::priority, this),
////										std::bind(&TestProp::setPriority, this, std::placeholders::_1));

////		std::shared_ptr<OrderedHash<int, QString>> enumValues = std::make_shared<OrderedHash<int, QString>>();
////		enumValues->append(0, "Zero");
////		enumValues->append(1, "One");
////		enumValues->append(10, "Ten");

////		PropertyValue<OrderedHash<int, QString>>* pu = addDynamicEnumProperty(
////					tr("Units"),
////					enumValues,
////					true,
////					(std::function<int(void)>)std::bind(&TestProp::units, this),
////					std::bind(&TestProp::setUnits, this, std::placeholders::_1));

////		pu->setValue(2);

////		ADD_PROPERTY_DYNAMIC_ENUM(Units2, true, enumValues, TestProp::units, TestProp::setUnits);
//	}

//	int someProp() const				{		return m_someProp;		}
//	void setSomeProp(int value)			{		m_someProp = value;		}

//	int units() const					{		return m_units;		}
//	void setUnits(int value)			{		m_units = value;		}

//	bool someBool() const				{		return m_someBool;		}
//	void setSomeBool(bool value)		{		m_someBool = value;		}

//	QString someString() const			{		return m_someString;	}
//	void setSomeString(QString value)	{		m_someString = value;	}

//	void setPriority(TestEnum::Priority priority)	{	m_priority = priority;	}
//	TestEnum::Priority priority() const				{	return m_priority;		}

//private:
//	int m_someProp = -1;
//	int m_units = 0;
//	bool m_someBool = false;
//	QString m_someString;
//	TestEnum::Priority m_priority = TestEnum::Priority::Low;
//};


#endif // PROPERTYOBJECT



