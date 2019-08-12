#pragma once

#include <type_traits>
#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <utility>
#include <memory>
#include <algorithm>

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QMetaEnum>
#include <QHash>
#include <QTime>
#include <QTimer>
#include <QDebug>

#include "../lib/OrderedHash.h"
#include "../lib/Types.h"

class PropertyObject;

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
	Property() = default;
	Property(const Property&) = default;
	Property(Property&&) = default;
	Property& operator=(const Property&) = default;
	Property& operator=(Property&&) = default;

	virtual ~Property()
	{
	}

public:
	virtual bool isEnum() const = 0;
	virtual std::list<std::pair<int, QString>> enumValues() const = 0;

	virtual QVariant enumValue() const = 0;		// if is enum, returns QVariant with TYPE or QVariant::string if type
												// cannot be get (dynamic enum properties)

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

	bool essential() const
	{
		return m_essential;
	}
	void setEssential(bool value)
	{
		m_essential = value;
	}

	bool disableTableEditor() const
	{
		return m_disableTableEditor;
	}
	void setDisableTableEditor(bool value)
	{
		m_disableTableEditor = value;
	}


	E::PropertySpecificEditor specificEditor()
	{
		return m_specificEditor;
	}

	void setSpecificEditor(E::PropertySpecificEditor value)
	{
		m_specificEditor = value;
	}

	bool password() const
	{
		return m_specificEditor == E::PropertySpecificEditor::Password;
	}
	void setPassword(bool value)
	{
		m_specificEditor = value ? E::PropertySpecificEditor::Password : E::PropertySpecificEditor::None;
	}

	bool isScript() const
	{
		return m_specificEditor == E::PropertySpecificEditor::Script ||
				caption().contains("Script") == true;
	}
	void setIsScript(bool value)
	{
		m_specificEditor = value ? E::PropertySpecificEditor::Script : E::PropertySpecificEditor::None;
	}

	int precision() const
	{
		return m_precision;
	}
	void setPrecision(int value)
	{
		m_precision = std::clamp<qint16>(static_cast<qint16>(value), 0, 128);
	}

	quint16 viewOrder() const
	{
		return m_viewOrder;
	}
	void setViewOrder(quint16 value)
	{
		m_viewOrder = value;
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
			Q_ASSERT(source);
			return;
		}

		m_caption = source->m_caption;
		m_description = source->m_description;
		m_category = source->m_category;
		m_validator = source->m_validator;
		m_flags = source->m_flags;
		m_specificEditor = source->m_specificEditor;
		m_precision = source->m_precision;
		m_viewOrder = source->m_viewOrder;

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
			bool m_essential : 1;
			bool m_disableTableEditor : 1;
		};
		uint16_t m_flags = 0;
	};

	E::PropertySpecificEditor m_specificEditor = E::PropertySpecificEditor::None;
	qint16 m_precision = 2;
	quint16 m_viewOrder = 0xFFFF;
};

//
//
//			Class PropertyTypedValue
//
//
template <typename TYPE,
		  typename CLASS,
		  TYPE(CLASS::*get)() const,
		  void(CLASS::*set)(const TYPE&)>
class PropertyTypedValue : public Property
{
public:
	PropertyTypedValue() = delete;
	PropertyTypedValue(CLASS* object) :
		Property(),
		m_object(object)
	{
		Q_ASSERT(object);
	}

	PropertyTypedValue(const PropertyTypedValue&) = default;
	PropertyTypedValue(PropertyTypedValue&&) = default;
	PropertyTypedValue& operator=(const PropertyTypedValue&) = default;
	PropertyTypedValue& operator=(PropertyTypedValue&&) = default;

public:
	virtual bool isEnum() const override final
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
		Q_ASSERT(std::is_enum<NOT_ENUM>::value);			// Try to get QMetaEnum for not enum type
		return QMetaEnum();
	}

public:
	virtual std::list<std::pair<int, QString>> enumValues() const override final
	{
		Q_ASSERT(std::is_enum<TYPE>::value);

		std::list<std::pair<int, QString>> result;

		QMetaEnum me = metaEnum<TYPE>(enumness<std::is_enum<TYPE>::value>());
		if (me.isValid() == false)
		{
			Q_ASSERT(me.isValid() == true);
			return result;
		}

		int keyCount = me.keyCount();
		for (int i = 0; i < keyCount; i++)
		{
			result.push_back(std::make_pair(me.value(i), QString::fromLocal8Bit(me.key(i))));
		}

		return result;
	}

	virtual QVariant enumValue() const override final
	{
		if constexpr (std::is_enum<TYPE>::value == false)
		{
			Q_ASSERT(isEnum());
			return {};
		}
		else
		{
			return value();
		}
	}

public:
	virtual QVariant value() const override final
	{
		QVariant result(QVariant::fromValue((m_object->*get)()));
		return result;
	}

	void setValueDirect(const TYPE& value)				// Not virtual, is called from class ProprtyObject for direct access
	{
		static_assert(set != nullptr);

		if (set != nullptr)
		{
			(m_object->*set)(value);
		}
		else
		{
			Q_ASSERT(false);
		}
	}

	void setValue(const QVariant& value) override final	// Overriden from class Propery
	{
		if (set == nullptr)
		{
			qDebug() << "Set property value for property without Setter: " << caption();
		}
		else
		{
			Q_ASSERT(value.canConvert<TYPE>());
			(m_object->*set)(value.value<TYPE>());
		}
	}

	virtual void setEnumValue(int value) override final			// Overriden from class Property
	{
		setEnumValueInternal<TYPE>(value, enumness<std::is_enum<TYPE>::value>());
	}
	virtual void setEnumValue(const char* value) override final	// Overriden from class Property
	{
		return setEnumValueInternal<TYPE>(value, enumness<std::is_enum<TYPE>::value>());
	}

private:
	template <typename ENUM>
	void setEnumValueInternal(int value, enum_tag)				// Overriden from class Property
	{
		static_assert(set);
		static_assert(std::is_enum<TYPE>::value);

		(m_object->*set)(static_cast<TYPE>(value));
	}
	template <typename NON_ENUM>
	void setEnumValueInternal(int value, non_enum_tag)
	{
		Q_UNUSED(value)
		Q_ASSERT(false);
	}

	template <typename ENUM>
	void setEnumValueInternal(const char* value, enum_tag)				// Overriden from class Property
	{
		static_assert(std::is_enum<TYPE>::value);

		QVariant v(value);
		setValue(v);

		return;
	}
	template <typename NON_ENUM>
	void setEnumValueInternal(const char* value, non_enum_tag)
	{
		Q_UNUSED(value)
		Q_ASSERT(false);
	}

public:
	virtual const QVariant& lowLimit() const override final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setLowLimit(const QVariant&) override final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	virtual const QVariant& highLimit() const override final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setHighLimit(const QVariant&) override final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	virtual bool isTheSameType(Property* property) override final
	{
		if (dynamic_cast<decltype(this)>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override final
	{
		if (presetProperty == nullptr ||
			isTheSameType(presetProperty) == false)
		{
			Q_ASSERT(presetProperty != nullptr);
			Q_ASSERT(isTheSameType(presetProperty) == true);
			return;
		}

		Property::copy(presetProperty);	// Copy data from the base class

		if (auto source = dynamic_cast<decltype(this)>(presetProperty);
			source == nullptr)
		{
			Q_ASSERT(source);
			return;
		}

		if (updateValue == true)
		{
			setValue(presetProperty->value());
		}

		return;
	}

private:
	CLASS* const m_object = nullptr;		// Cannot change m_object pointer
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
	PropertyValue() = default;
	PropertyValue(const PropertyValue&) = default;
	PropertyValue(PropertyValue&&) = default;
	PropertyValue& operator=(const PropertyValue&) = default;
	PropertyValue& operator=(PropertyValue&&) = default;

public:
	virtual bool isEnum() const override final
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
		Q_ASSERT(std::is_enum<NOT_ENUM>::value);			// Try to get QMetaEnum for not enum type
		return QMetaEnum();
	}

public:
	virtual std::list<std::pair<int, QString>> enumValues() const override final
	{
		Q_ASSERT(std::is_enum<TYPE>::value);

		std::list<std::pair<int, QString>> result;

		QMetaEnum me = metaEnum<TYPE>(enumness<std::is_enum<TYPE>::value>());
		if (me.isValid() == false)
		{
			Q_ASSERT(me.isValid() == true);
			return result;
		}

		int keyCount = me.keyCount();
		for (int i = 0; i < keyCount; i++)
		{
			result.push_back(std::make_pair(me.value(i), QString::fromLocal8Bit(me.key(i))));
		}

		return result;
	}

	virtual QVariant enumValue() const override final
	{
		if constexpr (std::is_enum<TYPE>::value == false)
		{
			Q_ASSERT(isEnum());
			return {};
		}
		else
		{
			return value();
		}
	}

public:
	virtual QVariant value() const override final
	{
		if (m_getter)
		{
			return QVariant::fromValue(m_getter());
		}

		Q_ASSERT(m_getter);
		return {};
	}

	void setValueDirect(const TYPE& value)				// Not virtual, is called from class ProprtyObject for direct access
	{
		if (m_setter)
		{
			m_setter(value);
		}
		else
		{
			Q_ASSERT(false);
		}
	}

	void setValue(const QVariant& value) override final	// Overriden from class Propery
	{
		if (!m_setter)
		{
			//assert(false);
			qDebug() << "Set property value for property without Setter: " << caption();
		}
		else
		{
			Q_ASSERT(value.canConvert<TYPE>());
			m_setter(value.value<TYPE>());
		}
	}

	virtual void setEnumValue(int value) override final			// Overriden from class Propery
	{
		setEnumValueInternal<TYPE>(value, enumness<std::is_enum<TYPE>::value>());
	}
	virtual void setEnumValue(const char* value) override final	// Overriden from class Propery
	{
		return setEnumValueInternal<TYPE>(value, enumness<std::is_enum<TYPE>::value>());
	}

private:
	template <typename ENUM>
	void setEnumValueInternal(int value, enum_tag)				// Overriden from class Propery
	{
		static_assert(std::is_enum<TYPE>::value);

		if (m_setter)
		{
			m_setter(static_cast<TYPE>(value));
		}
		else
		{
			Q_ASSERT(false);
		}
	}
	template <typename NON_ENUM>
	void setEnumValueInternal(int value, non_enum_tag)
	{
		Q_UNUSED(value)
		Q_ASSERT(false);
	}

	template <typename ENUM>
	void setEnumValueInternal(const char* value, enum_tag)				// Overriden from class Propery
	{
		static_assert(std::is_enum<TYPE>::value);

		QVariant v(value);
		setValue(v);

		return;
	}
	template <typename NON_ENUM>
	void setEnumValueInternal(const char* value, non_enum_tag)
	{
		Q_UNUSED(value)
		Q_ASSERT(false);
	}

public:
	virtual const QVariant& lowLimit() const override final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setLowLimit(const QVariant&) override final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	virtual const QVariant& highLimit() const override final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setHighLimit(const QVariant&) override final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	void setGetter(const std::function<TYPE(void)>& getter)
	{
		m_getter = getter;
	}
	void setSetter(const std::function<void(TYPE)>& setter)
	{
		m_setter = setter;
	}

	virtual bool isTheSameType(Property* property) override final
	{
		if (dynamic_cast<PropertyValue<TYPE>*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override final
	{
		if (presetProperty == nullptr ||
			isTheSameType(presetProperty) == false)
		{
			Q_ASSERT(presetProperty != nullptr);
			Q_ASSERT(isTheSameType(presetProperty) == true);
			return;
		}

		Property::copy(presetProperty);	// Copy data from the base class

		PropertyValue<TYPE>* source = dynamic_cast<PropertyValue<TYPE>*>(presetProperty);

		if (source == nullptr)
		{
			Q_ASSERT(source);
			return;
		}

		if (updateValue == true)
		{
			setValue(presetProperty->value());
		}

		// Do not copy m_getter/m_setter, because they are binded to their own object instances
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
//			Class PropertyValueNoGetterSetter
//
//
class PropertyValueNoGetterSetter : public Property
{
public:
	PropertyValueNoGetterSetter() = default;
	PropertyValueNoGetterSetter(const PropertyValueNoGetterSetter&) = default;
	PropertyValueNoGetterSetter(PropertyValueNoGetterSetter&&) = default;
	PropertyValueNoGetterSetter& operator=(const PropertyValueNoGetterSetter&) = default;
	PropertyValueNoGetterSetter& operator=(PropertyValueNoGetterSetter&&) = default;

	virtual ~PropertyValueNoGetterSetter()
	{
	}

public:
	virtual bool isEnum() const override final
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

	virtual QVariant enumValue() const override final
	{
#ifdef _DEBUG
		if (isEnum() == false)		// Commented for perfomance reasone
		{
			Q_ASSERT(isEnum());
			return QVariant();
		}
#endif

		return value();
	}

public:
	virtual std::list<std::pair<int, QString>> enumValues() const override final
	{
		std::list<std::pair<int, QString>> result;

		if (isEnum() == false)
		{
			Q_ASSERT(isEnum());
			return result;
		}

		if (m_value.isValid() == true)
		{
			// Enum can be inside QVariant
			//
			const QMetaObject* mo = QMetaType::metaObjectForType(m_value.userType());
			if (mo == nullptr)
			{
				Q_ASSERT(mo);
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
					Q_ASSERT(me.isValid() == true);
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
	virtual QVariant value() const override final
	{
		return m_value;
	}

	void setValue(const QVariant& value) override final	// Overriden from class Propery
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
			Q_ASSERT(ok);
			Q_UNUSED(ok);

			m_value.setValue(v);

			checkLimits();
			return;
		}

		if (value.canConvert(m_value.userType()) == true)
		{
			QVariant v(value);

			bool ok = v.convert(m_value.userType());
			Q_ASSERT(ok);
			Q_UNUSED(ok);

			m_value.setValue(v);
			checkLimits();
			return;
		}

		Q_ASSERT(m_value.type() == value.type());

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

	virtual void setEnumValue(int value) override final	// Overriden from class Propery
	{
		// cannot implement it now, but it's possible
		// The problem is, I dont see the way QVariant with enum inside can be set from integer and keep that enum type
		// QVariant::canConvert from int to enum returns false (for QString its ok)
		//
		QVariantEx* vex = static_cast<QVariantEx*>(&m_value);
		vex->setEnumHack(value);

		return;
	}

	virtual void setEnumValue(const char* value) override final	// Overriden from class Propery
	{
		if (QVariant v(value);
			v.canConvert(m_value.userType()) == true)
		{
			bool ok = v.convert(m_value.userType());
			if (Q_LIKELY(ok == true))
			{
				m_value.setValue(v);
			}
			else
			{
				Q_ASSERT(ok);
			}
		}
		else
		{
			Q_ASSERT(false);
		}
	}

private:
	void checkLimits()
	{
		if (m_lowLimit.isValid() == true)
		{
			Q_ASSERT(m_lowLimit.type() == m_value.type());

			if (m_value < m_lowLimit)
			{
				m_value = m_lowLimit;
			}
		}

		if (m_highLimit.isValid() == true)
		{
			Q_ASSERT(m_highLimit.type() == m_value.type());

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

	virtual const QVariant& lowLimit() const override final
	{
		return m_lowLimit;
	}
	virtual void setLowLimit(const QVariant& value) override final
	{
		m_lowLimit = value;
	}

	virtual const QVariant& highLimit() const override final
	{
		return m_highLimit;
	}
	virtual void setHighLimit(const QVariant& value) override final
	{
		m_highLimit = value;
	}

	virtual bool isTheSameType(Property* property) override final
	{
		if (dynamic_cast<PropertyValueNoGetterSetter*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override final
	{
		if (presetProperty == nullptr ||
			isTheSameType(presetProperty) == false)
		{
			Q_ASSERT(presetProperty != nullptr);
			Q_ASSERT(isTheSameType(presetProperty) == true);
			return;
		}

		Property::copy(presetProperty);	// Copy data from the base class

		PropertyValueNoGetterSetter* source = dynamic_cast<PropertyValueNoGetterSetter*>(presetProperty);
		if (source == nullptr)
		{
			Q_ASSERT(source);
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


//			Dynamic Enum Property
//			Class PropertyValue specialization for OrderedHash<int, QString>,
//			class behaves like enum
//
template <>
class PropertyValue<OrderedHash<int, QString>> : public Property
{
public:
	PropertyValue(std::shared_ptr<OrderedHash<int, QString>> enumValues) :
		m_enumValues(enumValues)
	{
		Q_ASSERT(m_enumValues);
	}

	virtual ~PropertyValue()
	{
	}

public:
	virtual bool isEnum() const override final
	{
		return true;	// This is dynamic enumeration
	}

	virtual std::list<std::pair<int, QString>> enumValues() const override final
	{
		std::list<std::pair<int, QString>> result;

		auto values = m_enumValues->getKeyValueVector();

		for (auto i : values)
		{
			result.push_back(std::make_pair(i.first, i.second));
		}

		return result;
	}

	virtual QVariant enumValue() const override final
	{
		return value();
	}

public:
	virtual QVariant value() const override final
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

	void setValue(const QVariant& value) override final	// Overriden from class Propery
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

		Q_ASSERT(false);
	}

	virtual void setEnumValue(int value) override final	// Overriden from class Propery
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

	virtual void setEnumValue(const char* value) override final	// Overriden from class Propery
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

	virtual const QVariant& lowLimit() const override final
	{
		static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	virtual void setLowLimit(const QVariant& value) override final
	{
		Q_UNUSED(value);
	}

	virtual const QVariant& highLimit() const override final
	{
		static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	virtual void setHighLimit(const QVariant& value) override final
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

	virtual bool isTheSameType(Property* property) override final
	{
		if (dynamic_cast<PropertyValue<OrderedHash<int, QString>>*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override final
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


//			Dynamic Enum Property based on std::vector, it's slower then PropertyValue<OrderedHash> but consumes
//			less memory and in plain way.
//			class behaves like enum
//
template <>
class PropertyValue<std::vector<std::pair<QString, int>>> : public Property
{
public:
	PropertyValue(const std::vector<std::pair<QString, int>>& enumValues) :
		m_enumValues(enumValues)
	{
		if (m_enumValues.empty() == false)
		{
			m_value = m_enumValues.front().second;
		}
		else
		{
			Q_ASSERT(m_enumValues.empty() == false);
			m_value = 0;
		}

		return;
	}

public:
	virtual bool isEnum() const override final
	{
		return true;	// This is dynamic enumeration
	}

	virtual std::list<std::pair<int, QString>> enumValues() const override final
	{
		std::list<std::pair<int, QString>> result;

		for (auto[str, key] : m_enumValues)
		{
			result.push_back({key, str});
		}

		return result;
	}

	virtual QVariant enumValue() const override final
	{
		for (auto[str, key] : m_enumValues)
		{
			if (key == m_value)
			{
				return QVariant(str);
			}
		}

		Q_ASSERT(false);
		return QVariant();
	}

public:
	virtual QVariant value() const override final
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

	virtual void setValue(const QVariant& value) override final		// Overriden from class Propery
	{
		if (value.type() == QVariant::Int)
		{
			setEnumValue(value.value<int>());
			return;
		}

		if (value.type() == QVariant::String)
		{
			setEnumValue(value.toString().toStdString().data());
			return;
		}

		Q_ASSERT(false);
		return;
	}

	virtual void setEnumValue(int value) override final				// Overriden from class Propery
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

	virtual void setEnumValue(const char* value) override final		// Overriden from class Propery
	{
		QString strvalue(value);

		for (auto[str, key] : m_enumValues)
		{
			if (strvalue == str)
			{
				setEnumValue(key);
				return;
			}
		}

		// Str not found
		//
		Q_ASSERT(false);
		return;
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

	virtual const QVariant& lowLimit() const override final
	{
		static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	virtual void setLowLimit(const QVariant& value) override final
	{
		Q_UNUSED(value);
	}

	virtual const QVariant& highLimit() const override final
	{
		static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	virtual void setHighLimit(const QVariant& value) override final
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

	virtual bool isTheSameType(Property* property) override final
	{
		return dynamic_cast<PropertyValue<std::vector<std::pair<QString, int>>>*>(property) != nullptr;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) override final
	{
		if (presetProperty == nullptr ||
			isTheSameType(presetProperty) == false)
		{
			Q_ASSERT(presetProperty != nullptr);
			Q_ASSERT(isTheSameType(presetProperty) == true);
			return;
		}

		Property::copy(presetProperty);	// Copy data from the base class
		auto source = dynamic_cast<PropertyValue<std::vector<std::pair<QString, int>>>*>(presetProperty);

		if (source == nullptr)
		{
			Q_ASSERT(source);
			return;
		}

		if (updateValue == true)
		{
			setValue(presetProperty->value());
		}

		// Do not copy m_getter/m_setter, because they are binded to their own object instances
		//
		return;
	}

private:
	// WARNING!!! If you add a field, do not forget to add it to updateFromPreset();
	//
	std::vector<std::pair<QString, int>> m_enumValues;

	int m_value = 0;	// m_value is not index is a value from m_enumValues

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

protected:
	// Override in child to postpone property creation
	//
	virtual void propertyDemand(const QString& prop)
	{
		Q_UNUSED(prop);
	}

	void demandAllProperties()
	{
		if (m_allPropsAlreadyDemanded == false)
		{
			propertyDemand(QString());
			m_allPropsAlreadyDemanded = true;
		}

		return;
	}

	void demandAllProperties() const
	{
		if (m_allPropsAlreadyDemanded == false)
		{
			const_cast<PropertyObject*>(this)->propertyDemand(QString());
			m_allPropsAlreadyDemanded = true;
		}

		return;
	}


public:
	// Add new property
	//
	template <typename TYPE,
			  typename CLASS,
			  TYPE(CLASS::*get)() const,
			  void(CLASS::*set)(const TYPE&)>
	auto addProperty(const QString& caption, const QString& category, bool visible)
	{
		static_assert(get != nullptr);

		auto property = std::make_shared<PropertyTypedValue<TYPE, CLASS, get, set>>(dynamic_cast<CLASS*>(this));

		uint hash = qHash(caption);

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);

		if (set == nullptr)
		{
			property->setReadOnly(true);
		}

		m_properties[hash] = std::dynamic_pointer_cast<Property>(property);

		emit propertyListChanged();

		return property.get();
	}


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
			Q_ASSERT(getter);
			return nullptr;
		}

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

		emit propertyListChanged();

		return property.get();
	}

	PropertyValueNoGetterSetter* addProperty(const QString& caption,
											 const QString& category,
											 bool visible,
											 const QVariant& value)
	{
		std::shared_ptr<PropertyValueNoGetterSetter> property = std::make_shared<PropertyValueNoGetterSetter>();

		uint hash = qHash(caption);

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);
		property->setValue(value);

		m_properties[hash] = property;

		emit propertyListChanged();

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
			Q_ASSERT(enumValues);
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

		emit propertyListChanged();

		return property.get();
	}

	PropertyValue<std::vector<std::pair<QString, int>>>* addDynamicEnumProperty(
			const QString& caption,
			const std::vector<std::pair<QString, int>>& enumValues,
			bool visible = false,
			std::function<int(void)> getter = std::function<int(void)>(),
			std::function<void(int)> setter = std::function<void(int)>())
	{
		uint hash = qHash(caption);

		auto property = std::make_shared<PropertyValue<std::vector<std::pair<QString, int>>>>(enumValues);

		property->setCaption(caption);
		property->setVisible(visible);
		property->setGetter(getter);
		property->setSetter(setter);

		m_properties[hash] = property;

		emit propertyListChanged();

		return property.get();
	}

	// Get all properties
	//
	std::vector<std::shared_ptr<Property>> properties() const
	{
		demandAllProperties();

		std::vector<std::shared_ptr<Property>> result;
		result.reserve(m_properties.size());

		for (const std::pair<uint, std::shared_ptr<Property>>& p : m_properties)
		{
			result.push_back(p.second);
		}

		return result;
	}

	std::vector<std::shared_ptr<Property>> specificProperties() const
	{
		// Specific properties cannot be demanded,
		// they are not in propertyDemand
		//

		std::vector<std::shared_ptr<Property>> result;
		result.reserve(m_properties.size());

		for (const auto[key, prop] : m_properties)
		{
			Q_UNUSED(key);

			if (prop->specific() == true)
			{
				result.push_back(prop);
			}
		}

		return result;
	}

	Q_INVOKABLE bool propertyExists(const QString& caption, bool demandIfNotExists) const
	{
		uint hash = qHash(caption);

		if (demandIfNotExists == false)
		{
			return m_properties.find(hash) != m_properties.end();
		}
		else
		{
			return propertyByCaption(caption) != nullptr;
		}
	}

	Q_INVOKABLE bool propertyExists(const QString& caption) const
	{
		return propertyByCaption(caption) != nullptr;
	}

	void removeAllProperties()
	{
		m_properties.clear();
		m_allPropsAlreadyDemanded = false;

		emit propertyListChanged();
	}

	bool removeProperty(const QString& caption)
	{
		uint hash = qHash(caption);
		size_t removed = m_properties.erase(hash);

		if (removed > 0)
		{
			emit propertyListChanged();
			return true;
		}
		else
		{
			return false;
		}
	}

	// Delete all specific properties
	//
	void removeSpecificProperties()
	{
		bool someRemoved = false;

		for(auto it = m_properties.begin(); it != m_properties.end();)
		{
			if(it->second->specific() == true)
			{
				it = m_properties.erase(it);
				someRemoved = true;
			}
			else
			{
				++it;
			}
		}

		if (someRemoved == true)
		{
			emit propertyListChanged();
		}

		return;
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

		if (properties.empty() == false)
		{
			emit propertyListChanged();
		}

		return;
	}

	void addProperty(std::shared_ptr<Property> property)
	{
		uint hash = qHash(property->caption());
		m_properties[hash] = property;

		emit propertyListChanged();

		return;
	}


	// Get specific property by its caption,
	// return Property* or nullptr if property is not found
	//
	std::shared_ptr<Property> propertyByCaption(const QString& caption)
	{
		uint hash = qHash(caption);

		if (auto it = m_properties.find(hash);
			it == m_properties.end())
		{
			propertyDemand(caption);

			if (auto itt = m_properties.find(hash);
				itt == m_properties.end())
			{
				return std::shared_ptr<Property>();
			}
			else
			{
				return itt->second;
			}
		}
		else
		{
			return it->second;
		}
	}

	const std::shared_ptr<Property> propertyByCaption(const QString& caption) const
	{
		uint hash = qHash(caption);

		if (auto it = m_properties.find(hash);
			it == m_properties.end())
		{
			const_cast<PropertyObject*>(this)->propertyDemand(caption);

			if (auto itt = m_properties.find(hash);
				itt == m_properties.end())
			{
				return std::shared_ptr<Property>();
			}
			else
			{
				return itt->second;
			}
		}
		else
		{
			return it->second;
		}
	}

	Q_INVOKABLE QVariant propertyValue(const QString& caption) const
	{
		auto prop = propertyByCaption(caption);

		if (prop != nullptr)
		{
			return prop->value();
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
		auto prop = propertyByCaption(caption);
		if (prop == nullptr)
		{
			return false;
		}

		Property* property = prop.get();

		if (property->isEnum() == true)
		{
			Q_ASSERT(std::is_integral<TYPE>::value == true);

			property->setEnumValue(value);
			return true;
		}
		else
		{
			if (PropertyValue<TYPE>* propertyValue = dynamic_cast<PropertyValue<TYPE>*>(property);
				propertyValue != nullptr)
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
		auto property = propertyByCaption(caption);
		if (property == nullptr)
		{
			return false;
		}

		if (property->isEnum() == true)
		{
			property->setEnumValue(value);
			return true;
		}
		else
		{
			if (PropertyValue<QString>* propertyValue = dynamic_cast<PropertyValue<QString>*>(property.get());
				propertyValue != nullptr)
			{
				propertyValue->setValueDirect(QString(value));
				return true;
			}
			else
			{
				property->setValue(QVariant::fromValue(QString(value)));	// Try to get luck
				return true;
			}
		}

		return false;
	}

	Q_INVOKABLE bool setPropertyValue(const QString& caption, const QVariant& value)
	{
		auto property = propertyByCaption(caption);
		if (property == nullptr)
		{
			return false;
		}

		property->setValue(value);

		return true;
	}

	std::list<std::pair<int, QString>> enumValues(const QString& caption)
	{
		std::list<std::pair<int, QString>> result;

		auto property = propertyByCaption(caption);
		if (property == nullptr)
		{
			return result;
		}

		result = property->enumValues();

		return result;
	}

	static const int m_lastSpecificPropertiesVersion = 6;

	static QString createSpecificPropertyStruct(const QString& name,
										   const QString& category,
										   const QString& description,
										   const QString& strType,
										   const QString& strMin,
										   const QString& strMax,
										   const QString& strDefaultValue,
										   int precision,
										   bool updateFromPreset,
										   bool expert,
										   bool visible,
										   const E::PropertySpecificEditor editor,
										   quint16 viewOrder,
											bool essential)
	{
		static_assert(PropertyObject::m_lastSpecificPropertiesVersion >= 1 && PropertyObject::m_lastSpecificPropertiesVersion <= 6);	// Function must be reviewed if version is raised

		QString result = QString("%1;").arg(m_lastSpecificPropertiesVersion);

		result += name + ";";
		result += category + ";";
		result += strType + ";";
		result += strMin + ";";
		result += strMax + ";";
		result += strDefaultValue + ";";
		result += QString("%1;").arg(precision),
		result += updateFromPreset ? "true;" : "false;";
		result += expert ? "true;" : "false;";
		result += description + ";";
		result += visible ? "true;" : "false;";
		result += E::valueToString<E::PropertySpecificEditor>(editor) + ";";
		result += QString("%1;").arg(viewOrder);
		result += essential ? "true" : "false";

		result = result.replace(QChar::CarriageReturn, "\\r");
		result = result.replace(QChar::LineFeed, "\\n");

		return result;

	}

	// Specific properties
	//
	std::pair<bool, QString> parseSpecificPropertiesStruct(const QString& specificProperties)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		// Save all specific properties values
		//
		std::vector<std::shared_ptr<Property>> oldProperties = this->specificProperties();

		// Delete all previous object's specific properties
		//
		removeSpecificProperties();

		// Parse struct (rows, divided by semicolon) and create new properties
		//

		/*
		Example:

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset
		1;          IP;		Server;		string;		0;			0;			192.168.75.254;     0           false
		1;          Port;	Server;		uint32_t;	1;			65535;		2345;               0           false

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description
		2;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description		Visible
		3;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;		true

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description		Visible		Editor
		4;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;		true		None

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description		Visible		Editor	ViewOrder
		5;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;		true		None	65535

		version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description		Visible		Editor	ViewOrder	Essential
		6;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;		true		None	65535		false

		version:            record version
		name:               property name
		category:           category name

		type:               property type, can by one of:
							qint32  (4 bytes signed integral),
							quint32 (4 bytes unsigned integer)
							bool (true, false),
							double,
							E::Channel,
							string,
							DynamicEnum [EnumValue1 = 1, EnumValue2 = 2 , EnumValue7 = 12, ...]

		min:                property minimum value (ignored for bool, string)
		max:                property maximim value (ignored for bool, string)
		default:            can be any value of the specified type
		precision:          property precision
		updateFromPreset:   property will be updated from preset

		expert:				[Added in version 2] expert property
		description:		[Added in version 2] property description

		visible:			[Added in version 3] property is visible

		Editor				[Added in version 4] Property specific editor (emun E::PropertySpecificEditor )
							can have values: None, Password, Script, TuningFilter, SpecificProperties

		ViewOrder			[Added in version 5] View order for displaying in PropertyEditor
		Essential			[Added in version 6] Property is highlighted by color in PropertyEditor
		*/
		QString m_specificPropertiesStructTrimmed = specificProperties;

		QStringList rows = m_specificPropertiesStructTrimmed.split(QChar::LineFeed, QString::SkipEmptyParts);

		for (QString row : rows)
		{
			row = row.trimmed();
			if (row.isEmpty() == true)
			{
				continue;
			}

			QStringList columns = row.split(';');

			for (QString& col : columns)
			{
				col = col.trimmed();

				col = col.replace(QString("\\r"), QString(QChar::CarriageReturn));
				col = col.replace(QString("\\n"), QString(QChar::LineFeed));
			}

			QString strVersion(columns[0]);
			bool ok = false;
			int version = strVersion.toInt(&ok);

			if (ok == false)
			{
				result.first = false;
				result.second += "SpecificProperties: failed to parse specific prop version filed: " + row;
				continue;
			}

			switch (version)
			{
			case 1:
				{
					auto parseResult = parseSpecificPropertiesStructV1(columns);

					result.first &= parseResult.first;
					result.second += parseResult.second;
				}
				break;
			case 2:
				{
					auto parseResult = parseSpecificPropertiesStructV2(columns);

					result.first &= parseResult.first;
					result.second += parseResult.second;
				}
				break;
			case 3:
				{
					auto parseResult = parseSpecificPropertiesStructV3(columns);

					result.first &= parseResult.first;
					result.second += parseResult.second;
				}
				break;
			case 4:
				{
					auto parseResult = parseSpecificPropertiesStructV4(columns);

					result.first &= parseResult.first;
					result.second += parseResult.second;
				}
				break;
			case 5:
				{
					auto parseResult = parseSpecificPropertiesStructV5(columns);

					result.first &= parseResult.first;
					result.second += parseResult.second;
				}
				break;
			case 6:
				{
					auto parseResult = parseSpecificPropertiesStructV6(columns);

					result.first &= parseResult.first;
					result.second += parseResult.second;
				}
				break;
			default:
				result.first = false;
				result.second += "SpecificProperties: Unsupported version: " + version;

				Q_ASSERT(false);
				qDebug() << "Object has spec prop with unsuported version: " << row;
			}
		}

		std::vector<std::shared_ptr<Property>> newProperties = this->specificProperties();

		bool someValuesWereRestored = false;

		// Set to parsed properties old value
		//
		for (const std::shared_ptr<Property>& p : oldProperties)
		{
			auto it = std::find_if(newProperties.begin(), newProperties.end(),
								   [p](const std::shared_ptr<Property>& np)
			{
				return np->caption() == p->caption();
			});

			if (it != newProperties.end() &&
				(*it)->value().type() == p->value().type() &&
				p != (*it))
			{
				someValuesWereRestored = true;

				setPropertyValue(p->caption(), p->value());
			}
			else
			{
				// Default value already was set
				//
				continue;
			}
		}

		if (someValuesWereRestored == true)
		{
			emit propertyListChanged();
		}

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesStructV1(const QStringList& columns)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (columns.count() != 9)
		{
			result.first = false;
			result.second += " Wrong proprty struct version 1! Expected: version;name;category;type;min;max;default;precision;updateFromPreset\n";

			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 1!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset";
			return result;
		}

		const QString& name = columns.at(1);
		const QString& category = columns.at(2);
		const QString& type = columns.at(3);
		const QString& min = columns.at(4);
		const QString& max = columns.at(5);
		const QString& defaultValue = columns.at(6);
		const QString& strPrecision = columns.at(7);
		const QString& strUpdateFromPreset = columns.at(8);

		result = parseSpecificPropertiesCreate(1,
											   name,
											   category,
											   QString(),
											   type,
											   min,
											   max,
											   defaultValue,
											   strPrecision,
											   strUpdateFromPreset,
											   QStringLiteral("false"),
											   QStringLiteral("true"),
											   QStringLiteral("None"),
											   QStringLiteral("65535"),
											   QStringLiteral("false"));

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesStructV2(const QStringList& columns)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (columns.count() != 11)
		{
			result.first = false;
			result.second = "Wrong proprty struct version 2!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description\n";

			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 2!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description";
			return result;
		}

		const QString& name = columns.at(1);
		const QString& category = columns.at(2);
		const QString& type = columns.at(3);
		const QString& min = columns.at(4);
		const QString& max = columns.at(5);
		const QString& defaultValue = columns.at(6);
		const QString& strPrecision = columns.at(7);
		const QString& strUpdateFromPreset = columns.at(8);
		const QString& strExpert = columns.at(9);
		const QString& strDescription = columns.at(10);

		result = parseSpecificPropertiesCreate(2,
											   name,
											   category,
											   strDescription,
											   type,
											   min,
											   max,
											   defaultValue,
											   strPrecision,
											   strUpdateFromPreset,
											   strExpert,
											   QStringLiteral("true"),
											   QStringLiteral("None"),
											   QStringLiteral("65535"),
											   QStringLiteral("false"));

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesStructV3(const QStringList& columns)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (columns.count() != 12)
		{
			result.first = false;
			result.second = "Wrong proprty struct version 3!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible\n";

			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 3!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible";
			return result;
		}

		const QString& name = columns.at(1);
		const QString& category = columns.at(2);
		const QString& type = columns.at(3);
		const QString& min = columns.at(4);
		const QString& max = columns.at(5);
		const QString& defaultValue = columns.at(6);
		const QString& strPrecision = columns.at(7);
		const QString& strUpdateFromPreset = columns.at(8);
		const QString& strExpert = columns.at(9);
		const QString& strDescription= columns.at(10);
		const QString& strVisible = columns.at(11);

		result = parseSpecificPropertiesCreate(3,
											   name,
											   category,
											   strDescription,
											   type,
											   min,
											   max,
											   defaultValue,
											   strPrecision,
											   strUpdateFromPreset,
											   strExpert,
											   strVisible,
											   QStringLiteral("None"),
											   QStringLiteral("65535"),
											   QStringLiteral("false"));

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesStructV4(const QStringList& columns)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (columns.count() != 13)
		{
			result.first = false;
			result.second = "Wrong proprty struct version 4!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor\n";

			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 3!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor";
			return result;
		}

		const QString& name = columns.at(1);
		const QString& category = columns.at(2);
		const QString& type = columns.at(3);
		const QString& min = columns.at(4);
		const QString& max = columns.at(5);
		const QString& defaultValue = columns.at(6);
		const QString& strPrecision = columns.at(7);
		const QString& strUpdateFromPreset = columns.at(8);
		const QString& strExpert = columns.at(9);
		const QString& description = columns.at(10);
		const QString& strVisible = columns.at(11);
		const QString& strEditor = columns.at(12);

		result = parseSpecificPropertiesCreate(4,
											   name,
											   category,
											   description,
											   type,
											   min,
											   max,
											   defaultValue,
											   strPrecision,
											   strUpdateFromPreset,
											   strExpert,
											   strVisible,
											   strEditor,
											   QStringLiteral("65535"),
											   QStringLiteral("false"));

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesStructV5(const QStringList& columns)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (columns.count() != 14)
		{
			result.first = false;
			result.second = "Wrong proprty struct version 5!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder\n";

			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 3!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder";
			return result;
		}

		const QString& name = columns.at(1);
		const QString& category = columns.at(2);
		const QString& type = columns.at(3);
		const QString& min = columns.at(4);
		const QString& max = columns.at(5);
		const QString& defaultValue = columns.at(6);
		const QString& strPrecision = columns.at(7);
		const QString& strUpdateFromPreset = columns.at(8);
		const QString& strExpert = columns.at(9);
		const QString& description = columns.at(10);
		const QString& strVisible = columns.at(11);
		const QString& strEditor = columns.at(12);
		const QString& strViewOrder = columns.at(13);

		result = parseSpecificPropertiesCreate(5,
											   name,
											   category,
											   description,
											   type,
											   min,
											   max,
											   defaultValue,
											   strPrecision,
											   strUpdateFromPreset,
											   strExpert,
											   strVisible,
											   strEditor,
											   strViewOrder,
											   QStringLiteral("false"));

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesStructV6(const QStringList& columns)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (columns.count() != 15)
		{
			result.first = false;
			result.second = "Wrong proprty struct version 6!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder;essential\n";

			qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 6!";
			qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder;essential";
			return result;
		}

		const QString& name = columns.at(1);
		const QString& category = columns.at(2);
		const QString& type = columns.at(3);
		const QString& min = columns.at(4);
		const QString& max = columns.at(5);
		const QString& defaultValue = columns.at(6);
		const QString& strPrecision = columns.at(7);
		const QString& strUpdateFromPreset = columns.at(8);
		const QString& strExpert = columns.at(9);
		const QString& description = columns.at(10);
		const QString& strVisible = columns.at(11);
		const QString& strEditor = columns.at(12);
		const QString& strViewOrder = columns.at(13);
		const QString& strEssential = columns.at(14);

		result = parseSpecificPropertiesCreate(6,
											   name,
											   category,
											   description,
											   type,
											   min,
											   max,
											   defaultValue,
											   strPrecision,
											   strUpdateFromPreset,
											   strExpert,
											   strVisible,
											   strEditor,
											   strViewOrder,
											   strEssential);

		return result;
	}

	std::pair<bool, QString> parseSpecificPropertiesCreate(int version,
														   const QString& name,
														   const QString& category,
														   const QString& description,
														   const QString& strType,
														   const QString& strMin,
														   const QString& strMax,
														   const QString& strDefaultValue,
														   const QString& strPrecision,
														   const QString& strUpdateFromPreset,
														   const QString& strExpert,
														   const QString& strVisible,
														   const QString& strEditor,
														   const QString& strViewOrder,
														   const QString& strEssential)
	{
		std::pair<bool, QString> result = std::make_pair(true, "");

		if (version < 0 || version > m_lastSpecificPropertiesVersion)
		{
			Q_ASSERT(false);

			result.first = false;
			result.second += "SpecificProperties: Unsupported version: " + QString::number(version);
			return result;
		}

		int precision = strPrecision.toInt();
		bool updateFromPreset = strUpdateFromPreset.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
		bool expert = strExpert.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
		bool visible = strVisible.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
		bool essential = strEssential.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;

		if (name.isEmpty() == true || name.size() > 1024)
		{
			result.first = false;
			result.second = "SpecificProperties: filed name must have size  from 1 to 1024, name: " + name + "\n";

			qDebug() << Q_FUNC_INFO << " SpecificProperties: filed name must have size  from 1 to 1024, name: " << name;
			return result;
		}

		// Get E::PropertySpecificEditor value
		//
		auto[editorType, editorOk] = E::stringToValue<E::PropertySpecificEditor>(strEditor);

		if (editorOk == false)
		{
			result.first = false;
			result.second = "SpecificProperties: Specific property editor is not recognized: " + strEditor + "\n";

			qDebug() << Q_FUNC_INFO << "SpecificProperties: Specific propertye editor is not recognized: " << strEditor;
			return result;
		}

		// ViewOrder
		//
		bool viewOrderOk = true;
		quint16 viewOrder = static_cast<quint16>(strViewOrder.toInt(&viewOrderOk, 10));

		if (viewOrderOk == false)
		{
			result.first = false;
			result.second = "SpecificProperties: Specific propertye ViewOrder is not recognized: " + strViewOrder + "\n";

			qDebug() << Q_FUNC_INFO << "SpecificProperties: Specific propertye ViewOrder is not recognized: " + strViewOrder;
			return result;
		}

		// Type
		//

		// Check if strType is like
		// DynamicEnum [EnumValue1 = 1, EnumValue2 = 2 , EnumValue7 = 12, ...]
		//
		Property* addedProperty = nullptr;

		if (bool startedFromDynamicEnum = strType.trimmed().startsWith(QLatin1String("DynamicEnum"), Qt::CaseInsensitive);
				startedFromDynamicEnum == true)
		{
			// Parse String - Key pairs:
			// [EnumValue1 = 1, EnumValue2 = 2 , EnumValue7 = 12, ...]
			//
			bool propertyOk = false;

			auto enumValues = PropertyObject::parseSpecificPropertyTypeDynamicEnum(strType, &propertyOk);

			if (propertyOk == false)
			{
				// Error, unknown type
				//
				result.first = false;
				result.second = " SpecificProperties: dynamic enum parsing error: " + strType + "\n";

				qDebug() << Q_FUNC_INFO << " SpecificProperties: dynamic enum parsing error: " << strType;
				return result;
			}


			// Add property with default value
			//
			auto p = addDynamicEnumProperty(name, enumValues, true);
			p->setCategory(category);
			p->setValue(strDefaultValue);

			addedProperty = p;
		}
		else
		{
			auto [pt, propertyOk] = parseSpecificPropertyType(strType);
			if (propertyOk == false)
			{
				// Error, unknown type
				//
				result.first = false;
				result.second = " SpecificProperties: wrong type: " + strType + "\n";

				qDebug() << Q_FUNC_INFO << " SpecificProperties: wrong type: " << strType;
				return result;
			}

			switch (pt)
			{
			case E::SpecificPropertyType::pt_int32:
				{
					// Min
					//
					bool ok = false;
					qint32 minInt = strMin.toInt(&ok);
					if (ok == false)
					{
						minInt = std::numeric_limits<qint32>::lowest();
					}

					// Max
					//
					qint32 maxInt = strMax.toInt(&ok);
					if (ok == false)
					{
						maxInt = std::numeric_limits<qint32>::max();
					}

					// Default Value
					//
					qint32 defaultInt = strDefaultValue.toInt();

					auto p = addProperty(name, category, true, QVariant(defaultInt));
					addedProperty = p;

					p->setLimits(QVariant(minInt), QVariant(maxInt));
				}
				break;
			case E::SpecificPropertyType::pt_uint32:
				{
					// Min
					//
					bool ok = false;
					quint32 minUInt = strMin.toUInt(&ok);
					if (ok == false)
					{
						minUInt = std::numeric_limits<quint32>::lowest();
					}

					// Max
					//
					quint32 maxUInt = strMax.toUInt(&ok);
					if (ok == false)
					{
						maxUInt = std::numeric_limits<quint32>::max();
					}

					// Default Value
					//
					quint32 defaultUInt = strDefaultValue.toUInt();

					// Add property with default value
					//
					auto p = addProperty(name, category, true, QVariant(defaultUInt));
					addedProperty = p;

					p->setLimits(QVariant(minUInt), QVariant(maxUInt));
				}
				break;
			case E::SpecificPropertyType::pt_double:
				{
					// Min
					//
					bool ok = false;
					double minDouble = strMin.toDouble(&ok);
					if (ok == false)
					{
						minDouble = std::numeric_limits<double>::lowest();
					}

					// Max
					//
					double maxDouble = strMax.toDouble(&ok);
					if (ok == false)
					{
						maxDouble = std::numeric_limits<double>::max();
					}

					// Default Value
					//
					double defaultDouble = strDefaultValue.toDouble();

					// Add property with default value
					//
					auto p = addProperty(name, category, true, QVariant(defaultDouble));
					addedProperty = p;

					p->setLimits(QVariant(minDouble), QVariant(maxDouble));
				}
				break;
			case E::SpecificPropertyType::pt_bool:
				{
					// Default Value
					//
					bool defaultBool = strDefaultValue.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;

					// Add property with default value
					//
					auto p = addProperty(name, category, true, QVariant(defaultBool));
					addedProperty = p;
				}
				break;
			case E::SpecificPropertyType::pt_e_channel:
				{
					// Default Value
					//
					QString defaultString = strDefaultValue;

					// Add property with default value
					//
					auto p = addProperty(name, category, true, QVariant::fromValue(E::Channel::A));
					addedProperty = p;

					p->setValue(defaultString.toStdString().c_str());
				}
				break;
			case E::SpecificPropertyType::pt_string:
				{
					// Add property with default value
					//
					auto p = addProperty(name, category, true, QVariant(strDefaultValue));
					addedProperty = p;
				}
				break;

			default:
				Q_ASSERT(false);

				// Error, unknown type
				//
				result.first = false;
				result.second = " SpecificProperties: wrong type: " + strType + "\n";

				qDebug() << Q_FUNC_INFO << " SpecificProperties: wrong type: " << strType;
				return result;
			}
		}

		// Set common for all properties
		//
		if (addedProperty == nullptr)
		{
			Q_ASSERT(addedProperty);

			result.first = false;
			result.second = " Property was not created: " + strType + "\n";
			return result;
		}

		// Set command properties
		//
		addedProperty->setSpecific(true);
		addedProperty->setReadOnly(false);
		addedProperty->setPrecision(precision);
		addedProperty->setUpdateFromPreset(updateFromPreset);
		addedProperty->setExpert(expert);
		addedProperty->setDescription(description);
		addedProperty->setVisible(visible);
		addedProperty->setSpecificEditor(editorType);
		addedProperty->setViewOrder(viewOrder);
		addedProperty->setEssential(essential);

		return result;
	}

	static std::pair<E::SpecificPropertyType, bool> parseSpecificPropertyType(const QString& strType)
	{
		static const QHash<QString, E::SpecificPropertyType> typeMap =
			{
				{QStringLiteral("qint32"), E::SpecificPropertyType::pt_int32},
				{QStringLiteral("signed int"), E::SpecificPropertyType::pt_int32},
				{QStringLiteral("int32"), E::SpecificPropertyType::pt_int32},
				{QStringLiteral("int"), E::SpecificPropertyType::pt_int32},
				{QStringLiteral("int32_t"), E::SpecificPropertyType::pt_int32},

				{QStringLiteral("quint32"), E::SpecificPropertyType::pt_uint32},
				{QStringLiteral("unsigned int"), E::SpecificPropertyType::pt_uint32},
				{QStringLiteral("uint32"), E::SpecificPropertyType::pt_uint32},
				{QStringLiteral("uint"), E::SpecificPropertyType::pt_uint32},
				{QStringLiteral("uint32_t"), E::SpecificPropertyType::pt_uint32},

				{QStringLiteral("double"), E::SpecificPropertyType::pt_double},
				{QStringLiteral("Double"), E::SpecificPropertyType::pt_double},

				{QStringLiteral("bool"), E::SpecificPropertyType::pt_bool},
				{QStringLiteral("Bool"), E::SpecificPropertyType::pt_bool},
				{QStringLiteral("boolean"), E::SpecificPropertyType::pt_bool},
				{QStringLiteral("Boolean"), E::SpecificPropertyType::pt_bool},

				{QStringLiteral("E::Channel"), E::SpecificPropertyType::pt_e_channel},
				{QStringLiteral("e::channel"), E::SpecificPropertyType::pt_e_channel},
				{QStringLiteral("channel"), E::SpecificPropertyType::pt_e_channel},

				{QStringLiteral("string"), E::SpecificPropertyType::pt_string},
				{QStringLiteral("String"), E::SpecificPropertyType::pt_string},
				{QStringLiteral("QString"), E::SpecificPropertyType::pt_string},
			};

		// Check for one of standard types from typeMap
		//
		auto typeIt = typeMap.find(strType);
		if (typeIt == typeMap.end())
		{
			return {E::SpecificPropertyType::pt_int32, false};
		}

		return {*typeIt, true};
	}

	static std::vector<std::pair<QString, int>> parseSpecificPropertyTypeDynamicEnum(const QString& strType, bool* ok)
	{
		std::vector<std::pair<QString, int>> enumValues;

		if (ok == nullptr)
		{
			Q_ASSERT(false);
			return enumValues;
		}

		int openBrace = strType.indexOf('[');
		int closeBrace = strType.lastIndexOf(']');

		if (openBrace == -1 || closeBrace == -1 || openBrace >= closeBrace)
		{
			*ok = false;
			return enumValues;
		}

		QString valuesString = strType.mid(openBrace + 1, closeBrace - openBrace - 1);
		valuesString.remove(' ');

		QStringList valueStringList = valuesString.split(',', QString::SkipEmptyParts);	// split value pairs
		if (valueStringList.empty() == true)
		{
			*ok = false;
			return enumValues;
		}

		enumValues.reserve(valueStringList.size());

		for (QString str : valueStringList)
		{
			// str is like:
			// EnumValue = 1
			//
			QStringList str2intList = str.split('=', QString::SkipEmptyParts);
			if (str2intList.size() != 2)
			{
				*ok = false;
				return enumValues;
			}

			QString enumStr = str2intList.at(0);
			bool conversionOk = false;
			int enumVal = str2intList.at(1).toInt(&conversionOk);

			if (conversionOk == false)
			{
				*ok = false;
				return enumValues;
			}

			// Pair is good
			//
			enumValues.push_back({enumStr, enumVal});
		}

		*ok = true;

		return enumValues;
	}

signals:
	void propertyListChanged();		// One or more properties were added or deleted

private:
	std::map<uint, std::shared_ptr<Property>> m_properties;		// key is property caption hash qHash(QString)
	mutable bool m_allPropsAlreadyDemanded = false;
};


//
// PropertyVector can be used in QVariant, full copy is made if assign operator called, so OBJECT_TYPE must have copy constructor
//
template <typename OBJECT_TYPE>
class PropertyVectorBase : public std::vector<std::shared_ptr<OBJECT_TYPE>>
{
public:
	virtual ~PropertyVectorBase() {};
	[[nodiscard]] virtual std::shared_ptr<OBJECT_TYPE> createItem() const = 0;
};


template <typename OBJECT_TYPE>
class PropertyVector : public PropertyVectorBase<OBJECT_TYPE>
{
public:
	PropertyVector()
	{
		static_assert(std::is_base_of<PropertyObject, OBJECT_TYPE>::value);
	}
	PropertyVector(const PropertyVector& src)
	{
		operator=(src);
	}
	PropertyVector(PropertyVector&& src) = default;

	~PropertyVector() = default;

	PropertyVector& operator= (const PropertyVector& src)
	{
		if (this == &src)
		{
			return *this;
		}

		this->clear();
		if (this->capacity() < src.size())
		{
			this->reserve(src.size());
		}

		for (const std::shared_ptr<OBJECT_TYPE>& item : src)
		{
			// Make a copy of the object
			//
			this->push_back(std::make_shared<OBJECT_TYPE>(*item));
		}

		return *this;
	}
	PropertyVector& operator= (PropertyVector&& src) = default;

	virtual std::shared_ptr<OBJECT_TYPE> createItem() const override
	{
		return std::make_shared<OBJECT_TYPE>();
	}
};

//
// PropertyList can be used in QVariant, full copy is made if assign operator called, so OBJECT_TYPE must have copy constructor
//
template <typename OBJECT_TYPE>
class PropertyListBase : public std::list<std::shared_ptr<OBJECT_TYPE>>
{
public:
	virtual ~PropertyListBase() {};
	[[nodiscard]] virtual std::shared_ptr<OBJECT_TYPE> createItem() const = 0;
};

template <typename OBJECT_TYPE>
class PropertyList : public PropertyListBase<OBJECT_TYPE>
{
public:
	PropertyList()
	{
		static_assert(std::is_base_of<PropertyObject, OBJECT_TYPE>::value);
	}
	PropertyList(const PropertyList& src)
	{
		operator=(src);
	}
	PropertyList(PropertyList&& src) = default;

	~PropertyList() = default;

	PropertyList& operator= (const PropertyList& src)
	{
		if (this == &src)
		{
			return *this;
		}

		this->clear();
		for (const std::shared_ptr<OBJECT_TYPE>& item : src)
		{
			// Make a copy of the object
			//
			this->push_back(std::make_shared<OBJECT_TYPE>(*item));
		}

		return *this;
	}
	PropertyList& operator= (PropertyList&& src) = default;

	virtual std::shared_ptr<OBJECT_TYPE> createItem() const override
	{
		return std::make_shared<OBJECT_TYPE>();
	}

};


// PropertyVector to QVariant functions
//
inline bool variantIsPropertyVector(const QVariant& v)
{
	QString type{v.typeName()};
	return type.startsWith(QStringLiteral("PropertyVector<"), Qt::CaseSensitive);
}

inline PropertyVectorBase<PropertyObject>* variantToPropertyVector(QVariant& v)
{
	if (variantIsPropertyVector(v) == false)
	{
		Q_ASSERT(variantIsPropertyVector(v));
		return nullptr;
	}

	// Dangerous hack! Kids, don't do it
	// It is Undefined Behavior
	// It' is better to create another vector and copy ther ptr's
	// Now leave it as is
	//
	return reinterpret_cast<PropertyVectorBase<PropertyObject>*>(v.data());
}

template<>
inline PropertyVectorBase<PropertyObject>* qvariant_cast<PropertyVectorBase<PropertyObject>*>(const QVariant& v)
{
	return variantToPropertyVector(const_cast<QVariant&>(v));
}

// PropertyList to QVariant functions
//
inline bool variantIsPropertyList(const QVariant& v)
{
	QString type{v.typeName()};
	return type.startsWith(QStringLiteral("PropertyList<"), Qt::CaseSensitive);
}

inline PropertyListBase<PropertyObject>* variantToPropertyList(QVariant& v)
{
	if (variantIsPropertyList(v) == false)
	{
		Q_ASSERT(variantIsPropertyList(v));
		return nullptr;
	}

	// Dangerous hack! Kids, don't do it
	// It is Undefined Behavior
	// It' is better to create another list and copy ther ptr's
	// Now leave it as is
	//
	return reinterpret_cast<PropertyListBase<PropertyObject>*>(v.data());
}

template<>
inline PropertyListBase<PropertyObject>* qvariant_cast<PropertyListBase<PropertyObject>*>(const QVariant& v)
{
	return variantToPropertyList(const_cast<QVariant&>(v));
}


