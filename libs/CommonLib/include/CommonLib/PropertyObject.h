#pragma once

#include "AfbParamValue.h"
#include <QVariant>
#include <functional>
#include <list>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// --
//
class PropertyObject;

#define ADD_PROPERTY_GETTER(TYPE, NAME, VISIBLE, GETTER) \
	addProperty<TYPE>(NAME, QString(), VISIBLE, \
	[this](){return GETTER();})


#define ADD_PROPERTY_GETTER_INDIRECT(TYPE, NAME, VISIBLE, GETTER, OWNER) \
	{\
		auto OWNER_PTR = &OWNER;\
		addProperty<TYPE>(NAME, QString(), VISIBLE, \
		[OWNER_PTR](){return OWNER_PTR->GETTER();});\
	}

// Add property which has getter and setter
//
#define ADD_PROPERTY_GETTER_SETTER(TYPE, NAME, VISIBLE, GETTER, SETTER) \
	addProperty<TYPE>(NAME, QString(), VISIBLE, \
	[this](){return GETTER();}, \
	[this](const TYPE& value){return SETTER(value);})


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
	[this](){return GETTER();}, \
	[this](const TYPE& value){return SETTER(value);})


// Add property which has dirrect access to the variable, by creating lambdas fo getter and setter
//
#define ADD_PROPERTY_CAT_VAR(TYPE, NAME, CATEGORY, VISIBLE, VARNAME) \
	addProperty<TYPE>(\
	NAME, \
	CATEGORY,\
	VISIBLE,\
	[this](){return VARNAME;}, \
	[this](const auto& v){VARNAME = v;})

// Add property which has getter and setter
//
#define ADD_PROPERTY_DYNAMIC_ENUM(NAME, VISIBLE, ENUMVALUES, GETTER, SETTER) \
	addDynamicEnumProperty(NAME, ENUMVALUES, VISIBLE, \
	[this](){return GETTER();}, \
	[this](const TYPE& value){return SETTER(value);});

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
protected:
	Property() noexcept = default;
	Property(const Property&) noexcept = default;
	Property(Property&&) noexcept = default;
	Property& operator=(const Property&) noexcept = default;
	Property& operator=(Property&&) noexcept = default;
	virtual ~Property() = default;

public:
	[[nodiscard]] virtual bool isEnum() const = 0;
	[[nodiscard]] virtual std::vector<std::pair<int, QString>> enumValues() const = 0;

	[[nodiscard]] virtual QVariant enumValue() const = 0;	// if is enum, returns QVariant with TYPE or QVariant::string if type
															// cannot be get (dynamic enum properties)

public:
	[[nodiscard]] const QString& caption() const noexcept
	{
		return m_caption;
	}
	Property& setCaption(const QString& value) noexcept
	{
		m_caption = value;
		return *this;
	}

	[[nodiscard]] const QString& description() const noexcept
	{
		return m_description;
	}
	Property& setDescription(const QString& value) noexcept
	{
		m_description = value;
		return *this;
	}

	[[nodiscard]] const QString& category() const noexcept
	{
		return m_category;
	}
	Property& setCategory(const QString& value) noexcept
	{
		m_category = value;
		return *this;
	}

	[[nodiscard]] bool isCategorized() const noexcept
	{
		return !m_category.isEmpty();
	}

	[[nodiscard]] const QString& validator() const noexcept
	{
		return m_validator;
	}
	Property& setValidator(const QString& value) noexcept
	{
		m_validator = value;
		return *this;
	}
	Property& setValidator(const QLatin1String value) noexcept
	{
		m_validator = value;
		return *this;
	}

	[[nodiscard]] bool readOnly() const noexcept
	{
		return m_readOnly;
	}
	Property& setReadOnly(bool value) noexcept
	{
		m_readOnly = value;
		return *this;
	}

	[[nodiscard]] bool updateFromPreset() const noexcept
	{
		return m_updateFromPreset;
	}
	Property& setUpdateFromPreset(bool value) noexcept
	{
		m_updateFromPreset = value;
		return *this;
	}

	[[nodiscard]] bool specific() const noexcept
	{
		return m_specific;
	}
	Property& setSpecific(bool value) noexcept
	{
		m_specific = value;
		return *this;
	}

	[[nodiscard]] bool isVisible() const noexcept
	{
		return m_visible;
	}
	[[nodiscard]] bool visible() const noexcept
	{
		return m_visible;
	}
	Property& setVisible(bool value) noexcept
	{
		m_visible = value;
		return *this;
	}

	[[nodiscard]] bool expert() const noexcept
	{
		return m_expert;
	}
	Property& setExpert(bool value) noexcept
	{
		m_expert = value;
		return *this;
	}

	[[nodiscard]] bool essential() const noexcept
	{
		return m_essential;
	}
	Property& setEssential(bool value) noexcept
	{
		m_essential = value;
		return *this;
	}

	[[nodiscard]] bool disableTableEditor() const noexcept
	{
		return m_disableTableEditor;
	}
	Property& setDisableTableEditor(bool value) noexcept
	{
		m_disableTableEditor = value;
		return *this;
	}

	[[nodiscard]] E::PropertySpecificEditor specificEditor() noexcept
	{
		return m_specificEditor;
	}
	Property& setSpecificEditor(E::PropertySpecificEditor value) noexcept
	{
		m_specificEditor = value;
		return *this;
	}

	[[nodiscard]] bool password() const noexcept
	{
		return m_specificEditor == E::PropertySpecificEditor::Password;
	}
	Property& setPassword(bool value) noexcept
	{
		m_specificEditor = value ? E::PropertySpecificEditor::Password : E::PropertySpecificEditor::None;
		return *this;
	}

	[[nodiscard]] bool isScript() const noexcept
	{
		return m_specificEditor == E::PropertySpecificEditor::Script ||
				caption().contains(QLatin1String("Script")) == true;
	}
	Property& setIsScript(bool value) noexcept
	{
		m_specificEditor = value ? E::PropertySpecificEditor::Script : E::PropertySpecificEditor::None;
		return *this;
	}

	[[nodiscard]] int precision() const noexcept
	{
		return m_precision;
	}
	Property& setPrecision(int value) noexcept
	{
		m_precision = std::clamp<qint16>(static_cast<qint16>(value), 0, 128);
		return *this;
	}

	[[nodiscard]] int viewOrder() const noexcept
	{
		return m_viewOrder;
	}
	Property& setViewOrder(int value) noexcept
	{
		m_viewOrder = static_cast<quint16>(value);
		return *this;
	}

	[[nodiscard]] virtual QVariant value() const noexcept = 0;
	virtual void setValue(const QVariant& value) noexcept = 0;

	virtual void setEnumValue(int value) noexcept = 0;
	virtual void setEnumValue(const char* value) noexcept = 0;

	[[nodiscard]] virtual const QVariant& lowLimit() const noexcept = 0;
	virtual void setLowLimit(const QVariant& value) noexcept = 0;

	[[nodiscard]] virtual const QVariant& highLimit() const noexcept = 0;
	virtual void setHighLimit(const QVariant& value) noexcept = 0;

	[[nodiscard]] virtual bool isTheSameType(Property* property) noexcept = 0;
	virtual void updateFromPreset(Property* presetProperty, bool updateValue) noexcept = 0;

protected:
	void copy(const Property* source) noexcept
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
		  void(CLASS::*set)(TYPE)>
class PropertyTypedValue final : public Property
{

public:
	PropertyTypedValue() = delete;
	PropertyTypedValue(CLASS* object) noexcept :
		Property(),
		m_object(object)
	{
		Q_ASSERT(object);
	}

	PropertyTypedValue(const PropertyTypedValue&) noexcept = default;
	PropertyTypedValue(PropertyTypedValue&&) noexcept = default;
	PropertyTypedValue& operator=(const PropertyTypedValue&) noexcept = default;
	PropertyTypedValue& operator=(PropertyTypedValue&&) noexcept = default;

public:
	[[nodiscard]] virtual bool isEnum() const noexcept final
	{
		return std::is_enum<TYPE>::value;
	}

	[[nodiscard]] virtual std::vector<std::pair<int, QString>> enumValues() const noexcept final
	{
		if constexpr (std::is_enum<TYPE>::value)
		{
			QMetaEnum me = QMetaEnum::fromType<TYPE>();
			Q_ASSERT(me.isValid() == true);

			int keyCount = me.keyCount();

			std::vector<std::pair<int, QString>> result;
			result.reserve(keyCount);

			for (int i = 0; i < keyCount; i++)
			{
				result.emplace_back(me.value(i), QString::fromLocal8Bit(me.key(i)));
			}

			return result;
		}
		else
		{
			Q_ASSERT(std::is_enum<TYPE>::value);
			return {};
		}
	}

	[[nodiscard]] virtual QVariant enumValue() const noexcept final
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
	[[nodiscard]] virtual QVariant value() const noexcept final
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

	void setValue(const QVariant& value) noexcept final	// Overriden from class Propery
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

	virtual void setEnumValue(int value) noexcept final			// Overriden from class Property
	{
		if constexpr (std::is_enum<TYPE>::value)
		{
			static_assert(set);
			static_assert(std::is_enum<TYPE>::value);

			(m_object->*set)(static_cast<TYPE>(value));
		}
		else
		{
			Q_ASSERT(std::is_enum<TYPE>::value);
		}

		return;
	}
	virtual void setEnumValue(const char* value) noexcept final	// Overriden from class Property
	{
		if constexpr (std::is_enum<TYPE>::value)
		{
			static_assert(std::is_enum<TYPE>::value);
			setValue(QVariant{value});
		}
		else
		{
			Q_ASSERT(std::is_enum<TYPE>::value);
		}
	}

public:
	[[nodiscard]] virtual const QVariant& lowLimit() const noexcept final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setLowLimit(const QVariant&) noexcept final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	[[nodiscard]] virtual const QVariant& highLimit() const noexcept final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setHighLimit(const QVariant&) noexcept final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	[[nodiscard]] virtual bool isTheSameType(Property* property) noexcept final
	{
		if (dynamic_cast<decltype(this)>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) noexcept final
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
class PropertyValue final : public Property
{
public:
	PropertyValue() noexcept = default;
	PropertyValue(const PropertyValue&) = default;
	PropertyValue(PropertyValue&&) noexcept = default;
	PropertyValue& operator=(const PropertyValue&) = default;
	PropertyValue& operator=(PropertyValue&&) noexcept = default;

public:
	[[nodiscard]] virtual bool isEnum() const noexcept final
	{
		return std::is_enum<TYPE>::value;
	}

	[[nodiscard]] virtual std::vector<std::pair<int, QString>> enumValues() const noexcept final
	{
		if constexpr (std::is_enum<TYPE>::value)
		{
			QMetaEnum me = QMetaEnum::fromType<TYPE>();
			Q_ASSERT(me.isValid());

			int keyCount = me.keyCount();

			std::vector<std::pair<int, QString>> result;
			result.reserve(keyCount);

			for (int i = 0; i < keyCount; i++)
			{
				result.emplace_back(me.value(i), QString::fromLocal8Bit(me.key(i)));
			}

			return result;
		}
		else
		{
			Q_ASSERT(std::is_enum<TYPE>::value);
			return {};
		}
	}

	[[nodiscard]] virtual QVariant enumValue() const noexcept final
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
	[[nodiscard]] virtual QVariant value() const noexcept final
	{
		if (m_getter)
		{
			return QVariant::fromValue(m_getter());
		}

		Q_ASSERT(m_getter);
		return {};
	}

	void setValueDirect(const TYPE& value) noexcept 			// Not virtual, is called from class ProprtyObject for direct access
	{
		if (m_setter)
		{
			m_setter(value);
		}
		else
		{
			Q_ASSERT(m_setter);
		}
	}

	void setValue(const QVariant& value) noexcept final	// Overriden from class Propery
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

	virtual void setEnumValue(int value) noexcept final			// Overriden from class Propery
	{
		if constexpr (std::is_enum<TYPE>::value)
		{
			Q_ASSERT(false);

			if (m_setter)
			{
				m_setter(static_cast<TYPE>(value));
			}
		}
		else
		{
			Q_ASSERT(std::is_enum<TYPE>::value);
		}

		return;
	}

	virtual void setEnumValue(const char* value) noexcept final	// Overriden from class Propery
	{
		if constexpr (std::is_enum<TYPE>::value)
		{
			setValue(QVariant{value});
		}
		else
		{
			Q_ASSERT(std::is_enum<TYPE>::value);
		}

		return;
	}

public:
	[[nodiscard]] virtual const QVariant& lowLimit() const noexcept final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setLowLimit(const QVariant&) noexcept final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	[[nodiscard]] virtual const QVariant& highLimit() const noexcept final
	{
		// Limits must be checked in getter/setter
		//
		Q_ASSERT(false);
		static QVariant staticQVariant;
		return staticQVariant;
	}
	virtual void setHighLimit(const QVariant&) noexcept final
	{
		// Limits must be checked in getter/setter
		Q_ASSERT(false);
	}

	void setGetter(const std::function<TYPE(void)>& getter) noexcept
	{
		m_getter = getter;
	}
	void setSetter(const std::function<void(TYPE)>& setter) noexcept
	{
		m_setter = setter;
	}

	[[nodiscard]] virtual bool isTheSameType(Property* property) noexcept final
	{
		if (dynamic_cast<PropertyValue<TYPE>*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) noexcept final
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
class PropertyValueNoGetterSetter final : public Property
{
public:
	PropertyValueNoGetterSetter() noexcept = default;
	PropertyValueNoGetterSetter(const PropertyValueNoGetterSetter&) = default;
	PropertyValueNoGetterSetter(PropertyValueNoGetterSetter&&) noexcept = default;
	PropertyValueNoGetterSetter& operator=(const PropertyValueNoGetterSetter&) = default;
	PropertyValueNoGetterSetter& operator=(PropertyValueNoGetterSetter&&) noexcept = default;
	virtual ~PropertyValueNoGetterSetter() = default;


public:
	[[nodiscard]] virtual bool isEnum() const noexcept final
	{
		if (m_value.isValid() == false)
		{
			return false;
		}

		// Enum can be inside QVariant
		//
		const QMetaObject* mo = m_value.metaType().metaObject();
		if (mo == nullptr)
		{
			return false;
		}

		QString typeStr(QString::fromLatin1(m_value.typeName()));
		QStringView typeNameRef(typeStr);

		if (qsizetype doubleColumnIndex = typeNameRef.lastIndexOf(QLatin1String("::"));
			doubleColumnIndex != -1)
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

	[[nodiscard]] virtual QVariant enumValue() const noexcept final
	{
#ifdef _DEBUG
		if (isEnum() == false)		// Commented for perfomance reasone
		{
			Q_ASSERT(isEnum());
			return {};
		}
#endif

		return value();
	}

public:
	[[nodiscard]] virtual std::vector<std::pair<int, QString>> enumValues() const noexcept final
	{
		std::vector<std::pair<int, QString>> result;

		if (isEnum() == false)
		{
			Q_ASSERT(isEnum());
			return result;
		}

		if (m_value.isValid() == true)
		{
			// Enum can be inside QVariant
			//
			const QMetaObject* mo = m_value.metaType().metaObject();
			if (mo == nullptr)
			{
				Q_ASSERT(mo);
				return result;
			}

			QString typeStr(QString::fromLatin1(m_value.typeName()));
			QStringView typeNameRef(typeStr);


			if (qsizetype doubleColumnIndex = typeNameRef.lastIndexOf(QLatin1String("::"));
				doubleColumnIndex != -1)
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
				result.reserve(keyCount);

				for (int i = 0; i < keyCount; i++)
				{
					result.emplace_back(me.value(i), QString::fromLocal8Bit(me.key(i)));
				}
			}
		}

		return result;
	}

public:
	[[nodiscard]] virtual QVariant value() const noexcept final
	{
		return m_value;
	}

	void setValue(const QVariant& value) noexcept final		// Overriden from class Propery
	{
		if (m_value.isValid() == false)
		{
			// It is initialization
			//
			m_value = value;
			return;
		}

		if (value.canConvert(m_value.metaType()) == true)
		{
			QVariant v(value);

			bool ok = v.convert(m_value.metaType());
			Q_ASSERT(ok);
			Q_UNUSED(ok);

			m_value.setValue(v);
			checkLimits();

			return;
		}


		if (m_value.metaType().id() == qMetaTypeId<Afb::AfbParamValue>())
		{
			auto afbParamValue = m_value.value<Afb::AfbParamValue>();

			if (afbParamValue.value().canConvert(value.metaType()) == true)
			{
				QVariant v(value);

				bool ok = v.convert(value.metaType());
				Q_ASSERT(ok);
				Q_UNUSED(ok);

				bool checkResult = afbParamValue.setValue(v);

				if (checkResult == true)
				{
					m_value.setValue(afbParamValue);
					checkLimits();
				}
			}

			return;
		}

		Q_ASSERT(m_value.metaType().id() == value.metaType().id());

		m_value = value;
		checkLimits();

		return;
	}

	class QVariantEx final : public QVariant
	{
	public:
		void setEnumHack(int value)
		{
			QVariant vint(value);

			[[maybe_unused]] bool ok = vint.convert(this->metaType());
			Q_ASSERT(ok);

			this->setValue(vint);
		}
	};

	virtual void setEnumValue(int value) noexcept final	// Overriden from class Propery
	{
		// cannot implement it now, but it's possible
		// The problem is, I dont see the way QVariant with enum inside can be set from integer and keep that enum type
		// QVariant::canConvert from int to enum returns false (for QString its ok)
		//
		QVariantEx* vex = static_cast<QVariantEx*>(&m_value);
		vex->setEnumHack(value);

		return;
	}

	virtual void setEnumValue(const char* value) noexcept final	// Overriden from class Propery
	{
		if (QVariant v(value);
			v.canConvert(m_value.metaType()) == true)
		{
			bool ok = v.convert(m_value.metaType());
			if (ok == true)
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
	void checkLimits() noexcept
	{
		QVariant value;

		if (m_value.metaType().id() == qMetaTypeId<Afb::AfbParamValue>())
		{
			auto afbParamValue = m_value.value<Afb::AfbParamValue>();

			if (afbParamValue.reference().isEmpty())
			{
				value = afbParamValue.value();
			}
			else
			{
				return;		// AfbParamValue is cannot check limits for variable
			}
		}
		else
		{
			value = m_value;
		}


		if (m_lowLimit.isValid() == true)
		{
			Q_ASSERT(value.canConvert(m_lowLimit.metaType()) == true);

            auto operatorLs =
                [](auto op1, auto op2) -> bool
                {
                    return op1 < op2;
                };

			bool less = false;

			switch (static_cast<QMetaType::Type>(value.metaType().id()))
			{
			case QMetaType::Int:
				assert(value.canConvert<int>());
				less = operatorLs(value.toInt(), m_lowLimit.toInt());
				break;
			case QMetaType::UInt:
				assert(value.canConvert<unsigned int>());
				less = operatorLs(value.toUInt(), m_lowLimit.toUInt());
				break;
			case QMetaType::LongLong:
				assert(value.canConvert<qlonglong>());
				less = operatorLs(value.toLongLong(), m_lowLimit.toLongLong());
				break;
			case QMetaType::ULongLong:
				assert(value.canConvert<qulonglong>());
				less = operatorLs(value.toULongLong(), m_lowLimit.toULongLong());
				break;
			case QMetaType::Double:
				assert(value.canConvert<double>());
				less = operatorLs(value.toDouble(), m_lowLimit.toDouble());
				break;
			case QMetaType::Long:
				assert(value.canConvert<long>());
				less = operatorLs(value.toInt(), m_lowLimit.toInt());
				break;
			case QMetaType::Short:
				assert(value.canConvert<short>());
				less = operatorLs(value.value<short>(), m_lowLimit.value<short>());
				break;
			case QMetaType::ULong:
				assert(value.canConvert<unsigned long>());
				less = operatorLs(value.value<unsigned long>(), m_lowLimit.value<unsigned long>());
				break;
			case QMetaType::UShort:
				assert(value.canConvert<unsigned short>());
				less = operatorLs(value.value<unsigned short>(), m_lowLimit.value<unsigned short>());
				break;
			case QMetaType::Float:
				assert(value.canConvert<float>());
				less = operatorLs(value.toFloat(), m_lowLimit.toFloat());
				break;
			default:
				less = false;
				break;
			}

			if (less == true)
			{
				if (m_value.metaType().id() == qMetaTypeId<Afb::AfbParamValue>())
				{
					Afb::AfbParamValue afbParamValue = m_value.value<Afb::AfbParamValue>();

					if (afbParamValue.reference().isEmpty() == true)
					{
						afbParamValue.setValue(m_lowLimit);
						m_value = afbParamValue.toVariant();
					}
				}
				else
				{
					m_value = m_lowLimit;
				}
			}
		}

		if (m_highLimit.isValid() == true)
		{
			Q_ASSERT(value.canConvert(m_highLimit.metaType()) == true);

            auto operatorGt =
                [](auto op1, auto op2) -> bool
                {
                    return op1 > op2;
                };

			bool gt = false;

			switch (value.metaType().id())
			{
			case QMetaType::Int:
				assert(value.canConvert<int>());
				gt = operatorGt(value.toInt(), m_highLimit.toInt());
				break;
			case QMetaType::UInt:
				assert(value.canConvert<unsigned int>());
				gt = operatorGt(value.toUInt(), m_highLimit.toUInt());
				break;
			case QMetaType::LongLong:
				assert(value.canConvert<qlonglong>());
				gt = operatorGt(value.toLongLong(), m_highLimit.toLongLong());
				break;
			case QMetaType::ULongLong:
				assert(value.canConvert<qulonglong>());
				gt = operatorGt(value.toULongLong(), m_highLimit.toULongLong());
				break;
			case QMetaType::Double:
				assert(value.canConvert<double>());
				gt = operatorGt(value.toDouble(), m_highLimit.toDouble());
                break;
			case QMetaType::Long:
				assert(value.canConvert<long>());
				gt = operatorGt(value.toInt(), m_highLimit.toInt());
                break;
			case QMetaType::Short:
				assert(value.canConvert<short>());
				gt = operatorGt(value.value<short>(), m_highLimit.value<short>());
                break;
			case QMetaType::ULong:
				assert(value.canConvert<unsigned long>());
				gt = operatorGt(value.value<unsigned long>(), m_highLimit.value<unsigned long>());
                break;
			case QMetaType::UShort:
				assert(value.canConvert<unsigned short>());
				gt = operatorGt(value.value<unsigned short>(), m_highLimit.value<unsigned short>());
                break;
			case QMetaType::Float:
				assert(value.canConvert<float>());
				gt = operatorGt(value.toFloat(), m_highLimit.toFloat());
                break;
			default:
				gt = false;
				break;
			}

			if (gt == true)
			{
				if (m_value.metaType().id() == qMetaTypeId<Afb::AfbParamValue>())
				{
					Afb::AfbParamValue afbParamValue = m_value.value<Afb::AfbParamValue>();

					if (afbParamValue.reference().isEmpty() == true)
					{
						afbParamValue.setValue(m_highLimit);
						m_value = afbParamValue.toVariant();
					}
				}
				else
				{
					m_value = m_highLimit;
				}
			}
		}
	}

public:
	void setLimits(const QVariant& low, const QVariant& high) noexcept
	{
		m_lowLimit = low;
		m_highLimit = high;
	}

	[[nodiscard]] virtual const QVariant& lowLimit() const noexcept final
	{
		return m_lowLimit;
	}
	virtual void setLowLimit(const QVariant& value) noexcept final
	{
		m_lowLimit = value;
	}

	[[nodiscard]] virtual const QVariant& highLimit() const noexcept final
	{
		return m_highLimit;
	}
	virtual void setHighLimit(const QVariant& value) noexcept final
	{
		m_highLimit = value;
	}

	virtual bool isTheSameType(Property* property) noexcept final
	{
		if (dynamic_cast<PropertyValueNoGetterSetter*>(property) == nullptr)
		{
			return false;
		}

		return true;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) noexcept final
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

//			Dynamic Enum Property based on std::vector, it's slower then PropertyValue<OrderedHash> but consumes
//			less memory and in plain way.
//			class behaves like enum
//
template <>
class PropertyValue<std::vector<std::pair<QString, int>>> final : public Property
{
public:
	PropertyValue(std::vector<std::pair<QString, int>> enumValues) noexcept :
		m_enumValues(std::move(enumValues))
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
	[[nodiscard]] virtual bool isEnum() const noexcept final
	{
		return true;	// This is dynamic enumeration
	}

	[[nodiscard]] virtual std::vector<std::pair<int, QString>> enumValues() const noexcept final
	{
		std::vector<std::pair<int, QString>> result;
		result.reserve(m_enumValues.size());

		for (const auto&[str, key] : m_enumValues)
		{
			result.emplace_back(key, str);
		}

		return result;
	}

	[[nodiscard]] virtual QVariant enumValue() const noexcept final
	{
		for (const auto&[str, key] : m_enumValues)
		{
			if (key == m_value)
			{
				return {str};
			}
		}

		Q_ASSERT(false);
		return {};
	}

public:
	[[nodiscard]] virtual QVariant value() const noexcept final
	{
		if (m_getter)
		{
			QVariant result(QVariant::fromValue(m_getter()));
			return result;
		}
		else
		{
			return {m_value};
		}
	}

	virtual void setValue(const QVariant& value) noexcept final		// Overriden from class Propery
	{
		if (value.metaType().id() == QMetaType::Int)
		{
			setEnumValue(value.toInt());
			return;
		}

		if (value.metaType().id() == QMetaType::QString)
		{
			setEnumValue(value.toString().toStdString().data());
			return;
		}

		Q_ASSERT(false);
		return;
	}

	virtual void setEnumValue(int value) noexcept final				// Overriden from class Propery
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

	virtual void setEnumValue(const char* value) noexcept final		// Overriden from class Propery
	{
		QString strvalue = QString::fromLatin1(value);

		for (const auto&[str, key] : m_enumValues)
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
	void checkLimits() noexcept
	{
	}

public:
	void setLimits(const QVariant& low, const QVariant& high) noexcept
	{
		Q_UNUSED(low);
		Q_UNUSED(high);
	}

	[[nodiscard]] virtual const QVariant& lowLimit() const noexcept final
	{
		static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	virtual void setLowLimit(const QVariant& value) noexcept final
	{
		Q_UNUSED(value);
	}

	[[nodiscard]] virtual const QVariant& highLimit() const noexcept final
	{
		static const QVariant dummy;			//	for return from lowLimt, hughLimt
		return dummy;
	}
	virtual void setHighLimit(const QVariant& value) noexcept final
	{
		Q_UNUSED(value);
	}

	void setGetter(std::function<int(void)> getter) noexcept
	{
		m_getter = std::move(getter);
	}
	void setSetter(std::function<void(int)> setter) noexcept
	{
		m_setter = std::move(setter);
	}

	virtual bool isTheSameType(Property* property) noexcept final
	{
		return dynamic_cast<PropertyValue<std::vector<std::pair<QString, int>>>*>(property) != nullptr;
	}

	virtual void updateFromPreset(Property* presetProperty, bool updateValue) noexcept final
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
//			Class PropertyObject to dervice from
//
//
class PropertyObject : public QObject
{
	Q_OBJECT

public:
	explicit PropertyObject(QObject* parent = nullptr) noexcept;
	PropertyObject(const PropertyObject& src) noexcept;

	PropertyObject(PropertyObject&& src) = delete;		// Forbidden to move properties as some (like PropertyTypedValue) has build in parent, which cannot be moved

	PropertyObject& operator=(const PropertyObject& src) noexcept;
	PropertyObject& operator=(PropertyObject&& src) = delete;			// Forbidden to move properties as some (like PropertyTypedValue) has build in parent, which cannot be moved

	virtual ~PropertyObject() = default;

protected:
	// Override in child to postpone property creation
	//
	virtual void propertyDemand(const QString& prop);
	void demandAllProperties();
	void demandAllProperties() const;

public:
	// Add new property
	//
	template <typename TYPE,
			  typename CLASS,
			  TYPE(CLASS::*get)() const,
			  void(CLASS::*set)(TYPE)>
	auto addProperty(const QString& caption, const QString& category, bool visible)
	{
#if !defined(__GNUC__) || (__GNUC___ > 10)
		static_assert(get != nullptr);
#endif

		auto property = std::make_shared<PropertyTypedValue<TYPE, CLASS, get, set>>(dynamic_cast<CLASS*>(this));

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);

		if (set == nullptr)
		{
			property->setReadOnly(true);
		}

		m_properties[caption] = std::dynamic_pointer_cast<Property>(property);

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

		property->setCaption(caption);
		property->setCategory(category);
		property->setVisible(visible);
		property->setGetter(getter);
		property->setSetter(setter);

		if (!setter)
		{
			property->setReadOnly(true);
		}

		m_properties[caption] = property;

		emit propertyListChanged();

		return property.get();
	}

	PropertyValueNoGetterSetter* addProperty(const QString& caption,
											 const QString& category,
											 bool visible,
											 const QVariant& value);

	PropertyValue<std::vector<std::pair<QString, int>>>* addDynamicEnumProperty(
			const QString& caption,
			const std::vector<std::pair<QString, int>>& enumValues,
			bool visible = false,
			const std::function<int(void)>& getter = std::function<int(void)>(),
			const std::function<void(int)>& setter = std::function<void(int)>());

	// Get all properties
	//
	std::vector<std::shared_ptr<Property>> properties() const;
	std::vector<std::shared_ptr<Property>> specificProperties() const;

	Q_INVOKABLE bool propertyExists(const QString& caption, bool demandIfNotExists) const;
	Q_INVOKABLE bool propertyExists(const QString& caption) const;

	void removeAllProperties();
	bool removeProperty(const QString& caption);

	void removeCategoryProperties(const QString& category);		// Delete all properties in category
	void removeSpecificProperties();							// Delete all specific properties

	void hideCategoryProperties(const QString& category);		// Hide all properties in category
	void showCategoryProperties(const QString& category);		// Show all properties in category

	// Add properties
	// 1. If properties have getter or setter they must be added via PropertyObject::addProperty
	// because getter and setter are binded to this
	// 2. It is posible to use addProperties with getter and setter properties
	// if they were added via PropertyObject::addProperty and later removed by removeAllProperties
	//
	void addProperties(const std::vector<std::shared_ptr<Property>>& properties);
	void addProperty(std::shared_ptr<Property> property);

	// Get specific property by its caption,
	// return Property* or nullptr if property is not found
	//
	std::shared_ptr<Property> propertyByCaption(const QString& caption);
	const std::shared_ptr<Property> propertyByCaption(const QString& caption) const;

	Q_INVOKABLE QVariant propertyValue(const QString& caption) const;

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

	bool setPropertyValue(const QString& caption, const char* value);

	Q_INVOKABLE bool setPropertyValue(const QString& caption, const QVariant& value);

	std::vector<std::pair<int, QString>> enumValues(const QString& caption);

	bool setPropertyVisible(const QString& caption, bool visible) 
	{
		auto prop = propertyByCaption(caption);
		if (prop == nullptr)
		{
			return false;
		}
		prop->setVisible(visible);
		return true;
	}

	//
	// Parse specific properties
	//
	static const int m_lastSpecificPropertiesVersion = 7;

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
										   bool essential,
										   bool readOnly);
	// Specific properties
	//
	std::pair<bool, QString> parseSpecificPropertiesStruct(const QString& specificProperties);
	std::pair<bool, QString> parseSpecificPropertiesStructV1(const QStringList& columns);
	std::pair<bool, QString> parseSpecificPropertiesStructV2(const QStringList& columns);
	std::pair<bool, QString> parseSpecificPropertiesStructV3(const QStringList& columns);
	std::pair<bool, QString> parseSpecificPropertiesStructV4(const QStringList& columns);
	std::pair<bool, QString> parseSpecificPropertiesStructV5(const QStringList& columns);
	std::pair<bool, QString> parseSpecificPropertiesStructV6(const QStringList& columns);
	std::pair<bool, QString> parseSpecificPropertiesStructV7(const QStringList& columns);
private:
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
														   const QString& strEssential,
														   const QString& strReadOnly);

public:
	static std::pair<E::SpecificPropertyType, bool> parseSpecificPropertyType(const QString& strType);
	static std::vector<std::pair<QString, int>> parseSpecificPropertyTypeDynamicEnum(const QString& strType, bool* ok);

signals:
	void propertyListChanged();		// One or more properties were added or deleted

private:
	std::unordered_map<QString, std::shared_ptr<Property>> m_properties;		// key is property Caption
	mutable bool m_allPropsAlreadyDemanded = false;
};


//
// PropertyVector can be used in QVariant, full copy is made if assign operator called, so OBJECT_TYPE must have copy constructor
//
template <typename OBJECT_TYPE>
class PropertyVectorBase : public std::vector<std::shared_ptr<OBJECT_TYPE>>
{
public:
	virtual ~PropertyVectorBase() = default;
	[[nodiscard]] virtual std::shared_ptr<OBJECT_TYPE> createItem() const = 0;
};


template <typename OBJECT_TYPE>
class PropertyVector final : public PropertyVectorBase<OBJECT_TYPE>
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
	PropertyVector(PropertyVector&& src) noexcept = default;

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
	PropertyVector& operator= (PropertyVector&& src) noexcept = default;

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
	virtual ~PropertyListBase() = default;
	[[nodiscard]] virtual std::shared_ptr<OBJECT_TYPE> createItem() const = 0;
};

template <typename OBJECT_TYPE>
class PropertyList final : public PropertyListBase<OBJECT_TYPE>
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
	PropertyList(PropertyList&& src) noexcept = default;

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
	PropertyList& operator= (PropertyList&& src) noexcept = default;

	virtual std::shared_ptr<OBJECT_TYPE> createItem() const override
	{
		return std::make_shared<OBJECT_TYPE>();
	}

};


// PropertyVector to QVariant functions
//
inline bool variantIsPropertyVector(const QVariant& v)
{
	QString type{QString::fromLatin1(v.typeName())};
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
	// Update: in c++20 coming something like reinterpret cast for shared pointers, it will not help, but...
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
	QString type{QString::fromLatin1(v.typeName())};
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


