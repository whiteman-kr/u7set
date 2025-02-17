#ifndef COMMON_LIB_DOMAIN
#error Do not include this file in the project! Link DbLib instead.
#endif

#include <CommonLib/PropertyObject.h>
#include <QRegularExpression>

//
//
//	PropertyObject
//
//
PropertyObject::PropertyObject(QObject* parent /*= nullptr*/) noexcept :
	QObject(parent)
{
	m_properties.reserve(16);
}

PropertyObject::PropertyObject(const PropertyObject& src) noexcept :
	QObject(src.parent()),
	m_properties(src.m_properties)
{
	// Shallow copy of properties
	//
}

PropertyObject& PropertyObject::operator=(const PropertyObject& src) noexcept
{
	// Shallow copy of properties
	//
	QObject::setParent(src.parent());
	m_properties = src.m_properties;

	return *this;
}

void PropertyObject::propertyDemand(const QString& prop)
{
	Q_UNUSED(prop);
}

void PropertyObject::demandAllProperties()
{
	if (m_allPropsAlreadyDemanded == false)
	{
		propertyDemand(QString());
		m_allPropsAlreadyDemanded = true;
	}

	return;
}

void PropertyObject::demandAllProperties() const
{
	if (m_allPropsAlreadyDemanded == false)
	{
		const_cast<PropertyObject*>(this)->propertyDemand(QString());
		m_allPropsAlreadyDemanded = true;
	}

	return;
}

PropertyValueNoGetterSetter* PropertyObject::addProperty(const QString& caption,
														 const QString& category,
														 bool visible,
														 const QVariant& value)
{
	std::shared_ptr<PropertyValueNoGetterSetter> property = std::make_shared<PropertyValueNoGetterSetter>();

	property->setCaption(caption);
	property->setCategory(category);
	property->setVisible(visible);
	property->setValue(value);

	m_properties[caption] = property;

	emit propertyListChanged();

	return property.get();
}

PropertyValue<std::vector<std::pair<QString, int>>>* PropertyObject::addDynamicEnumProperty(
		const QString& caption,
		const std::vector<std::pair<QString, int>>& enumValues,
		bool visible /*= false*/,
		const std::function<int(void)>& getter /*= std::function<int(void)>()*/,
		const std::function<void(int)>& setter /*= std::function<void(int)>()*/)
{
	auto property = std::make_shared<PropertyValue<std::vector<std::pair<QString, int>>>>(enumValues);

	property->setCaption(caption);
	property->setVisible(visible);
	property->setGetter(getter);
	property->setSetter(setter);

	m_properties[caption] = property;

	emit propertyListChanged();

	return property.get();
}

// Get all properties
//
std::vector<std::shared_ptr<Property>> PropertyObject::properties() const
{
	demandAllProperties();

	std::vector<std::shared_ptr<Property>> result;
	result.reserve(m_properties.size());

	for( auto it = m_properties.cbegin(); it != m_properties.cend(); ++it )
	{
		result.push_back(it->second);
	}

	return result;
}

std::vector<std::shared_ptr<Property>> PropertyObject::specificProperties() const
{
	// Specific properties cannot be demanded,
	// they are not in propertyDemand
	//
	std::vector<std::shared_ptr<Property>> result;
	result.reserve(m_properties.size());

	for(auto it = m_properties.cbegin(); it != m_properties.cend(); ++it )
	{
		if (it->second->specific() == true)
		{
			result.push_back(it->second);
		}
	}

	return result;
}

bool PropertyObject::propertyExists(const QString& caption, bool demandIfNotExists) const
{
	if (demandIfNotExists == false)
	{
		return m_properties.find(caption) != m_properties.end();
	}
	else
	{
		return propertyByCaption(caption) != nullptr;
	}
}

bool PropertyObject::propertyExists(const QString& caption) const
{
	return propertyByCaption(caption) != nullptr;
}

void PropertyObject::removeAllProperties()
{
	m_properties.clear();
	m_allPropsAlreadyDemanded = false;

	emit propertyListChanged();
}

bool PropertyObject::removeProperty(const QString& caption)
{
	size_t removed = m_properties.erase(caption);

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

void PropertyObject::removeCategoryProperties(const QString& category)
{
	bool someRemoved = false;

	for (auto it = m_properties.begin(), end = m_properties.end(); it != end;)
	{
		auto copy_it = it++;
		if (copy_it->second->category() == category)
		{
			m_properties.erase(copy_it);
			someRemoved = true;
		}
	}

	if (someRemoved == true)
	{
		emit propertyListChanged();
	}

	return;
}

// Delete all specific properties
//
void PropertyObject::removeSpecificProperties()
{
	bool someRemoved = false;

	for(auto it = m_properties.begin(); it != m_properties.end();)
	{
		auto copy_it = it++;
		if (copy_it->second->specific() == true)
		{
			m_properties.erase(copy_it);
			someRemoved = true;
		}
	}

	if (someRemoved == true)
	{
		emit propertyListChanged();
	}

	return;
}

void PropertyObject::hideCategoryProperties(const QString& category)
{
	bool someChanged = false;

	for(auto it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		if (it->second->category() == category)
		{
			it->second->setVisible(false);
			someChanged = true;
		}
	}

	if (someChanged == true)
	{
		emit propertyListChanged();
	}

	return;
}

void PropertyObject::showCategoryProperties(const QString& category)
{
	bool someChanged = false;

	for(auto it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		if (it->second->category() == category)
		{
			it->second->setVisible(true);
			someChanged = true;
		}
	}

	if (someChanged == true)
	{
		emit propertyListChanged();
	}

	return;
}

// Add properties
// 1. If properties have getter or setter they must be added via PropertyObject::addProperty
// because getter and setter are binded to this
// 2. It is possible to use addProperties with getter and setter properties
// if they were added via PropertyObject::addProperty and later removed by removeAllProperties
//
void PropertyObject::addProperties(const std::vector<std::shared_ptr<Property>>& properties)
{
	for (const std::shared_ptr<Property>& p : properties)
	{
		m_properties[p->caption()] = p;
	}

	if (properties.empty() == false)
	{
		emit propertyListChanged();
	}

	return;
}

void PropertyObject::addProperty(std::shared_ptr<Property> property)
{
	m_properties[property->caption()] = std::move(property);

	emit propertyListChanged();

	return;
}

std::shared_ptr<Property> PropertyObject::propertyByCaption(const QString& caption)
{
	if (auto it = m_properties.find(caption);
		it == m_properties.end())
	{
		propertyDemand(caption);

		if (auto itt = m_properties.find(caption);
			itt == m_properties.end())
		{
			return {};
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

const std::shared_ptr<Property> PropertyObject::propertyByCaption(const QString& caption) const
{
	if (auto it = m_properties.find(caption);
		it == m_properties.end())
	{
		const_cast<PropertyObject*>(this)->propertyDemand(caption);

		if (auto itt = m_properties.find(caption);
			itt == m_properties.end())
		{
			return {};
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

QVariant PropertyObject::propertyValue(const QString& caption) const
{
	auto prop = propertyByCaption(caption);

	if (prop != nullptr)
	{
		return prop->value();
	}
	else
	{
		qDebug() << "PropertyObject::propertyValue: property not found: " << caption;
		return {};
	}
}

bool PropertyObject::setPropertyValue(const QString& caption, const char* value)
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
			propertyValue->setValueDirect(QString::fromLatin1(value));
			return true;
		}
		else
		{
			property->setValue(QVariant::fromValue(QString::fromLatin1(value)));	// Try to get luck
			return true;
		}
	}

	return false;
}

bool PropertyObject::setPropertyValue(const QString& caption, const QVariant& value)
{
	auto property = propertyByCaption(caption);
	if (property == nullptr)
	{
		return false;
	}

	property->setValue(value);

	return true;
}

std::vector<std::pair<int, QString>> PropertyObject::enumValues(const QString& caption)
{
	auto property = propertyByCaption(caption);
	if (property == nullptr)
	{
		return {};
	}

	return property->enumValues();
}

QString PropertyObject::createSpecificPropertyStruct(const QString& name,
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
													 bool readOnly,
													 const QString& validator)
{
	static_assert(PropertyObject::m_lastSpecificPropertiesVersion >= 1 && PropertyObject::m_lastSpecificPropertiesVersion <= 8);	// Function must be reviewed if version is raised

	QLatin1String trueString{"true"};
	QLatin1String falseString{"false"};

	QStringList resultStrings;

	resultStrings.push_back(QStringLiteral("%1").arg(m_lastSpecificPropertiesVersion));
	resultStrings.push_back(name);
	resultStrings.push_back(category);
	resultStrings.push_back(strType);
	resultStrings.push_back(strMin);
	resultStrings.push_back(strMax);
	resultStrings.push_back(strDefaultValue);
	resultStrings.push_back(QStringLiteral("%1").arg(precision));
	resultStrings.push_back(updateFromPreset ? trueString : falseString);
	resultStrings.push_back(expert ? trueString : falseString);
	resultStrings.push_back(description);
	resultStrings.push_back(visible ? trueString : falseString);
	resultStrings.push_back(E::valueToString<E::PropertySpecificEditor>(editor));
	resultStrings.push_back(QStringLiteral("%1").arg(viewOrder));
	resultStrings.push_back(essential ? trueString: falseString);
	resultStrings.push_back(readOnly ? trueString : falseString);
	resultStrings.push_back(validator);

	static const QChar semicolon = ';';
	static const QChar quotes = '"';
	static const QLatin1String singleQuotesStr = QLatin1String("\"");
	static const QLatin1String doubleQuotesStr = QLatin1String("\"\"");

	for (QString& s : resultStrings)
	{
		s.replace(QChar::CarriageReturn, QLatin1String("\\r"));
		s.replace(QChar::LineFeed, QLatin1String("\\n"));

		// CSV-specific formatting
		//
		bool externalQuotes = false;

		if (s.contains(semicolon) == true)
		{
			externalQuotes = true;
		}

		if (s.contains(quotes) == true)
		{
			s.replace(singleQuotesStr, doubleQuotesStr);
			externalQuotes = true;
		}

		if (externalQuotes == true)
		{
			s = quotes + s + quotes;
		}
	}

	return resultStrings.join(';');
}

// Specific properties
//
std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStruct(const QString& specificProperties)
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

	version;    name; 	category;	type;		min;		max;		default             precision   updateFromPreset	Expert		Description		Visible		Editor	ViewOrder	Essential	ReadOnly
	7;          Port;	Server;		uint32_t;	1;			65535;		2345;               0;          false;				false;		IP Address;		true		None	65535		false		false

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
	ReadOnly			[Added in version 7] Property is read-only
	*/
	const QString& m_specificPropertiesStructTrimmed = specificProperties;

	QStringList rows = m_specificPropertiesStructTrimmed.split(QChar::LineFeed, Qt::SkipEmptyParts);

	for (QString row : rows)
	{
		row = row.trimmed();
		if (row.isEmpty() == true)
		{
			continue;
		}

		// Parse row to columns using CSV regular expression
		//
		QStringList columns;

		static const QLatin1String singleQuotesStr = QLatin1String("\"");
		static const QLatin1String doubleQuotesStr = QLatin1String("\"\"");

		// Regular expression was taken from https://forum.qt.io/topic/119076/qregexp-to-parse-a-csv-file
		//
		thread_local const QRegularExpression regExp(R"x((\;|\n|^)(?:"([^"]*(?:""[^"]*)*)"|([^"\;\n]*)))x");

		QRegularExpressionMatchIterator matchIt = regExp.globalMatch(row);
		while (matchIt.hasNext())
		{
			const QRegularExpressionMatch match = matchIt.next();

			QString s = match.capturedTexts().last();
			s.replace(doubleQuotesStr, singleQuotesStr);
			columns.push_back(s);
		}

		for (QString& col : columns)
		{
			col = col.trimmed();

			col = col.replace(QStringLiteral("\\r"), QString(QChar::CarriageReturn));
			col = col.replace(QStringLiteral("\\n"), QString(QChar::LineFeed));
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
		case 7:
			{
				auto parseResult = parseSpecificPropertiesStructV7(columns);

				result.first &= parseResult.first;
				result.second += parseResult.second;
			}
			break;
		case 8:
			{
				auto parseResult = parseSpecificPropertiesStructV8(columns);

				result.first &= parseResult.first;
				result.second += parseResult.second;
			}
			break;
		default:
			result.first = false;
			result.second += "SpecificProperties: Unsupported version: " + QString::number(version);

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
			(*it)->value().typeId() == p->value().typeId() &&
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

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV1(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 9)
	{
		result.first = false;
		result.second += QLatin1String(" Wrong property struct version 1! Expected: version;name;category;type;min;max;default;precision;updateFromPreset\n");

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
										   QStringLiteral("false"),
										   QStringLiteral("false"),
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV2(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 11)
	{
		result.first = false;
		result.second = QStringLiteral(
							"Wrong proprty struct version 2!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description\n");

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
										   QStringLiteral("false"),
										   QStringLiteral("false"),
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV3(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 12)
	{
		result.first = false;
		result.second = QStringLiteral(
							"Wrong proprty struct version 3!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible\n");

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
										   QStringLiteral("false"),
										   QStringLiteral("false"),
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV4(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 13)
	{
		result.first = false;
		result.second = QStringLiteral(
							"Wrong proprty struct version 4!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor\n");

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
										   QStringLiteral("false"),
										   QStringLiteral("false"),
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV5(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 14)
	{
		result.first = false;
		result.second = QStringLiteral(
							"Wrong proprty struct version 5!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder\n");

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
										   QStringLiteral("false"),
										   QStringLiteral("false"),
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV6(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 15)
	{
		result.first = false;
		result.second = QStringLiteral(
								"Wrong proprty struct version 6!\n"
								"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder;essential\n");

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
										   strEssential,
										   QStringLiteral("false"),
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV7(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 16)
	{
		result.first = false;
		result.second = QStringLiteral(
							"Wrong proprty struct version 7!\n"
							"Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder;essential;readOnly\n");

		qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 7!";
		qDebug() << Q_FUNC_INFO << " Expected: version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder;essential;readOnly";
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
	const QString& strReadOnly = columns.at(15);

	result = parseSpecificPropertiesCreate(7,
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
										   strEssential,
										   strReadOnly,
										   QString());

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesStructV8(const QStringList& columns)
{
	std::pair<bool, QString> result = std::make_pair(true, "");

	if (columns.count() != 17)
	{
		result.first = false;
		result.second = QStringLiteral("Wrong proprty struct version 8!\n"
									   "Expected: "
									   "version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;"
									   "editor;viewOrder;essential;readOnly;validator\n");

		qDebug() << Q_FUNC_INFO << " Wrong proprty struct version 8!";
		qDebug() << Q_FUNC_INFO
				 << " Expected: "
					"version;name;category;type;min;max;default;precision;updateFromPreset;expert;description;visible;editor;viewOrder;"
					"essential;readOnly;validator";
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
	const QString& strReadOnly = columns.at(15);
	const QString& strValidator = columns.at(16);

	result = parseSpecificPropertiesCreate(8,
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
										   strEssential,
										   strReadOnly,
										   strValidator);

	return result;
}

std::pair<bool, QString> PropertyObject::parseSpecificPropertiesCreate(int version,
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
																	   const QString& strReadOnly,
																	   const QString& validator)
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
	bool readOnly = strReadOnly.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;

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
				// Add property with default value
				//
				auto p = addProperty(name, category, true, QVariant::fromValue(E::Channel::A));
				addedProperty = p;

				p->setValue(strDefaultValue.toStdString().c_str());
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
	addedProperty->setReadOnly(readOnly);
	addedProperty->setPrecision(precision);
	addedProperty->setUpdateFromPreset(updateFromPreset);
	addedProperty->setExpert(expert);
	addedProperty->setDescription(description);
	addedProperty->setVisible(visible);
	addedProperty->setSpecificEditor(editorType);
	addedProperty->setViewOrder(viewOrder);
	addedProperty->setEssential(essential);
	
	if (validator.isEmpty() == false)
	{
		addedProperty->setValidator(validator);
	}

	return result;
}

std::pair<E::SpecificPropertyType, bool> PropertyObject::parseSpecificPropertyType(const QString& strType)
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

std::vector<std::pair<QString, int>> PropertyObject::parseSpecificPropertyTypeDynamicEnum(const QString& strType, bool* ok)
{
	std::vector<std::pair<QString, int>> enumValues;

	if (ok == nullptr)
	{
		Q_ASSERT(false);
		return enumValues;
	}

	qsizetype openBrace = strType.indexOf('[');
	qsizetype closeBrace = strType.lastIndexOf(']');

	if (openBrace == -1 || closeBrace == -1 || openBrace >= closeBrace)
	{
		*ok = false;
		return enumValues;
	}

	QString valuesString = strType.mid(openBrace + 1, closeBrace - openBrace - 1);
	valuesString.remove(' ');

	QStringList valueStringList = valuesString.split(',', Qt::SkipEmptyParts);	// split value pairs
	if (valueStringList.empty() == true)
	{
		*ok = false;
		return enumValues;
	}

	enumValues.reserve(valueStringList.size());

	for (const QString& str : valueStringList)
	{
		// str is like:
		// EnumValue = 1
		//
		QStringList str2intList = str.split('=', Qt::SkipEmptyParts);
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
		enumValues.emplace_back(enumStr, enumVal);
	}

	*ok = true;

	return enumValues;
}

