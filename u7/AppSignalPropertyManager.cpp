#include "AppSignalPropertyManager.h"
#include "../DbLib/DbController.h"
#include <QMessageBox>
#include "AppSignalSetProvider.h"

AppSignalPropertyManager* AppSignalPropertyManager::m_instance = nullptr;
const std::map<int, QString> AppSignalPropertyManager::m_emptyEnumValuesMap;
const AppSignalPropertyDescription AppSignalPropertyManager::m_notValidPropDescription;

const std::vector<AppSignalPropertyDescription> AppSignalPropertyManager::m_replacedPropertyDescription =
{
	{
		false,
		AppSignalPropNames::TYPE,
		"A/D/B",
		QMetaType::QString,
		[](const AppSignal* s){ return E::valueToString<E::SignalType>(s->signalType()).left(1); },
		nullptr,
		{},
		{}
	},

	{
		false,
		AppSignalPropNames::IN_OUT_TYPE,
		"Input-output type",
		QMetaType::QString,
		[](const AppSignal* s) { return E::valueToString<E::SignalInOutType>(s->inOutType()); },
		nullptr,
		{},
		{}
	},

	{
		false,
		AppSignalPropNames::BYTE_ORDER_PROP,
		"Byte order",
		QMetaType::QString,
		[](const AppSignal* s) { return E::valueToString<E::ByteOrder>(s->byteOrder()); },
		nullptr,
		{},
		{}
	},

	{
		false,
		AppSignalPropNames::ANALOG_SIGNAL_FORMAT,
		"Analog signal format",
		QMetaType::QString,
		[](const AppSignal* s) { return E::valueToString<E::AnalogAppSignalFormat>(s->analogSignalFormat()); },
		[](AppSignal* s, const QVariant& v) { s->setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(v.toInt())); },
		E::enumValuesMap<E::AnalogAppSignalFormat>(),
		{}
	},
};

AppSignalPropertyManager::AppSignalPropertyManager(DbController* dbController, QWidget* parentWidget) :
	m_dbController(dbController),
	m_parentWidget(parentWidget)
{
	assert (m_instance == nullptr);
	init();
	m_instance = this;
}

AppSignalPropertyManager* AppSignalPropertyManager::getInstance()
{
	assert (m_instance != nullptr);
	return m_instance;
}

int AppSignalPropertyManager::count() const
{
	return static_cast<int>(m_propertyDescription.size());
}

int AppSignalPropertyManager::index(const QString& name)
{
	for (size_t i = 0; i < m_propertyDescription.size(); i++)
	{
		if (m_propertyDescription[i].name == name)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

QString AppSignalPropertyManager::caption(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].caption;
}

QString AppSignalPropertyManager::name(int propertyIndex)
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].name;
}

bool AppSignalPropertyManager::getSignalEnumPropertyValues(const AppSignal& s, int propertyIndex,
														   std::vector<std::pair<int, QString>>* enumValues) const
{
	TEST_PTR_RETURN_FALSE(enumValues);

	enumValues->clear();

	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return false;
	}

	const AppSignalPropertyDescription& appSignalProperty = m_propertyDescription[static_cast<size_t>(propertyIndex)];

	Q_ASSERT(appSignalProperty.isEnumProperty());

	if (appSignalProperty.isSpecificProperty() == false)
	{
		Q_ASSERT(appSignalProperty.enumValues.size() > 0);

		return appSignalProperty.getEnumValuesVector(enumValues);
	}

	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(s.specPropStruct());

	if (result.first == false)
	{
		Q_ASSERT(false);
		return false;
	}

	std::shared_ptr<Property> property = propObject.propertyByCaption(appSignalProperty.name);

	TEST_PTR_RETURN_FALSE(property);

	*enumValues = property->enumValues();

	return true;
}

bool AppSignalPropertyManager::isEnumProperty(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return false;
	}

	return m_propertyDescription[static_cast<size_t>(propertyIndex)].isEnumProperty();
}

QVariant AppSignalPropertyManager::value(const AppSignal* signal, int propertyIndex, bool isExpert) const
{
	TEST_PTR_RETURN_VALUE(signal, QVariant());

	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return QVariant();
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);

	if (isHidden(behaviour, isExpert))
	{
		return QVariant();
	}

	const AppSignalPropertyDescription& property = m_propertyDescription[static_cast<size_t>(propertyIndex)];

	if (property.isSpecificProperty() == false)
	{
		return property.valueGetter(signal);
	}

	if (property.isSignalHaveProperty(signal->ID()) == false)
	{
		return QVariant();
	}

	if (property.isEnumProperty() == false)
	{
		return property.valueGetter(signal);
	}
	else
	{
		int value = property.valueGetter(signal).toInt();

		auto it = property.enumValues.find(value);

		if (it != property.enumValues.end())
		{
			return it->second;
		}

		return QString("Unknown value (%1)").arg(value);
	}
}

void AppSignalPropertyManager::setValue(AppSignal* signal, int propertyIndex, const QVariant& value, bool isExpert)
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);
	if (isHidden(behaviour, isExpert) || isReadOnly(behaviour, isExpert))
	{
		Q_ASSERT(false);
	}

	m_propertyDescription[static_cast<size_t>(propertyIndex)].valueSetter(signal, value);
}

const AppSignalPropertyDescription& AppSignalPropertyManager::getPropertyDescription(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return m_notValidPropDescription;
	}

	return m_propertyDescription[propertyIndex];
}

QMetaType::Type AppSignalPropertyManager::type(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return QMetaType::UnknownType;
	}

	return m_propertyDescription[static_cast<size_t>(propertyIndex)].type;
}

E::PropertyBehaviourType AppSignalPropertyManager::getBehaviour(const AppSignal& signal, const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return m_defaultBehaviour;
	}

	return getBehaviour(signal.signalType(), signal.inOutType(), propertyIndex);
}

E::PropertyBehaviourType AppSignalPropertyManager::getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const
{
	int bhIndex = behaviourIndex(propertyIndex);

	if (bhIndex == -1)
	{
		return m_defaultBehaviour;
	}

	auto typeEnum = QMetaEnum::fromType<E::SignalType>();
	auto inOutTypeEnum = QMetaEnum::fromType<E::SignalInOutType>();

	for (int i = 0; i < SIGNAL_TYPE_COUNT; i++)
	{
		if (type != typeEnum.value(i))
		{
			continue;
		}

		for (int j = 0; j < IN_OUT_TYPE_COUNT; j++)
		{
			if (directionType == static_cast<E::SignalInOutType>(inOutTypeEnum.value(j)))
			{
				return m_propertyBehaviorDescription[bhIndex].behaviourType[i * typeEnum.keyCount() + j];
			}
		}
	}

	return m_defaultBehaviour;
}

bool AppSignalPropertyManager::dependsOnPrecision(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		Q_ASSERT(false);
		return false;
	}

	int bhIndex = behaviourIndex(propertyIndex);

	if (bhIndex == -1)
	{
		return false;
	}

	return m_propertyBehaviorDescription[static_cast<size_t>(bhIndex)].dependsOnPrecision;
}

bool AppSignalPropertyManager::isHiddenFor(E::SignalType type, const int propertyIndex, bool isExpert) const
{
	auto inOutTypeEnum = QMetaEnum::fromType<E::SignalInOutType>();
	for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
	{
		E::SignalInOutType directionType = static_cast<E::SignalInOutType>(inOutTypeEnum.value(i));
		E::PropertyBehaviourType behaviour = getBehaviour(type, directionType, propertyIndex);
		if (isHidden(behaviour, isExpert) == false)
		{
			return false;
		}
	}
	return true;
}

void AppSignalPropertyManager::detectNewProperties(const AppSignal& signal)
{
	PropertyObject propObject;

	//
	// WMTD: create map with already processed signal->specPropStruct() to avoid repeated parsing!
	//

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(signal.specPropStruct());

	if (result.first == false)
	{
		Q_ASSERT(false);
		return;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	for(const std::shared_ptr<Property>& specificProperty : specificProperties)
	{
		bool propertyIsEnum = specificProperty->isEnum();

		int propIndex = propertyIndex(specificProperty->caption());

		if (propIndex != -1)
		{
			m_propertyDescription[propIndex].appendSignalID(signal.ID());

			if (propertyIsEnum == true)
			{
				m_propertyDescription[propIndex].joinEnumValues(specificProperty->enumValues());
			}

			continue;
		}

		AppSignalPropertyDescription newProperty;

		QString propertyName = specificProperty->caption();

		QMetaType::Type type = static_cast<QMetaType::Type>(specificProperty->value().typeId());

		newProperty.specificProperty = true;
		newProperty.name = propertyName;
		newProperty.caption = AppSignalProperties::generateCaption(propertyName);
		newProperty.type = type;
		newProperty.appendSignalID(signal.ID());

		if (propertyIsEnum)
		{
			newProperty.setEnumValues(specificProperty->enumValues());
		}

		newProperty.valueGetter = [propertyIsEnum, propertyName, type](const AppSignal* s)
		{
			QVariant qv;

			bool isEnum = propertyIsEnum;
			QString name = propertyName;

			bool result = s->getSpecPropValue(name, &qv, &isEnum, nullptr);

			if (result == false)
			{
				return QVariant();
			}

			Q_ASSERT(qv.typeId() == type);

			return qv;
		};

		newProperty.valueSetter = [propertyIsEnum, propertyName](AppSignal* s, const QVariant& v)
		{
			bool isEnum = propertyIsEnum;
			QString name = propertyName;

			bool result = s->setSpecPropValue(name, v, isEnum);

			assert(result == true);

			Q_UNUSED(result);
		};

		switch (newProperty.type)
		{
		case QMetaType::QString:
		case QMetaType::Double:
		case QMetaType::Int:
		case QMetaType::UInt:
		case QMetaType::Bool:
			break;
		default:
			Q_ASSERT(false);
			continue;
		}

		addNewProperty(newProperty);
	}
}

void AppSignalPropertyManager::updatePropertyName2IndexMap()
{
	m_propertyName2IndexMap.clear();

	for (size_t i = 0; i < m_propertyDescription.size(); i++)
	{
		m_propertyName2IndexMap.emplace(m_propertyDescription[i].name, static_cast<int>(i));
	}
}

int AppSignalPropertyManager::propertyIndex(const QString& propName) const
{
	auto it = m_propertyName2IndexMap.find(propName);

	return (it == m_propertyName2IndexMap.end() ? -1 : it->second);
}

int AppSignalPropertyManager::behaviourIndex(int propertyIndex) const
{
	auto it = m_propertyIndex2BehaviourIndexMap.find(propertyIndex);

	return (it == m_propertyIndex2BehaviourIndexMap.end() ? -1 : it->second);
}

void AppSignalPropertyManager::loadNotSpecificProperties()
{
	AppSignal signal;
	AppSignalProperties signalProperties(signal, true);
	std::vector<AppSignalPropertyDescription> propertyDescriptions = signalProperties.getProperties();

	for (AppSignalPropertyDescription& property : propertyDescriptions)
	{
		property.specificProperty = false;

		if (index(property.name) == -1)
		{
			auto propertyPtr = signalProperties.propertyByCaption(property.name);

			if (propertyPtr != nullptr && propertyPtr->category().isEmpty() == false)
			{
				addNewProperty(property);
			}
		}
	}
}

void AppSignalPropertyManager::reloadPropertyBehaviour()
{
	if (m_dbController == nullptr)
	{
		return;
	}

	int etcFileId = m_dbController->systemFileId(DbDir::EtcDir);

	DbFileInfo propertyBehaviorFile;
	m_dbController->getFileInfo(etcFileId, QString(Db::File::SignalPropertyBehaviorFileName), &propertyBehaviorFile, nullptr);

	if (propertyBehaviorFile.isNull() == true)
	{
		QMessageBox::critical(m_parentWidget, "Error", QString("File \"%1\" is not found!").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	std::shared_ptr<DbFile> file;
	bool result = m_dbController->getLatestVersion(propertyBehaviorFile, &file, nullptr);
	if (result == false)
	{
		QMessageBox::critical(m_parentWidget, "Error", QString("Could not load file \"%1\"").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}
	QString fileText = file->data();
	QStringList rows = fileText.split("\n", Qt::SkipEmptyParts);

	if (rows.isEmpty() == true)
	{
		QMessageBox::critical(m_parentWidget, "Error", QString("File \"%1\" is empty").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	QStringList fieldNameList = rows[0].split(';', Qt::KeepEmptyParts);
	trimm(fieldNameList);

	rows.removeFirst();

	QString uncorrectFileMessage =  QString("Uncorrect format of file \"%1\"").arg(Db::File::SignalPropertyBehaviorFileName);

	qsizetype nameIndex = fieldNameList.indexOf("PropertyName");
	if (nameIndex < 0)
	{
		QMessageBox::critical(m_parentWidget, "Error", uncorrectFileMessage + ": PropertyName column not found");
		return;
	}

	qsizetype precisionIndex = fieldNameList.indexOf("DependsOnPrecision");
	if (precisionIndex < 0)
	{
		QMessageBox::critical(m_parentWidget, "Error", uncorrectFileMessage + ": DependosOnPrecision column not found");
		return;
	}

	std::vector<int> typeIndexes(TOTAL_SIGNAL_TYPE_COUNT, -1);

	for (int i = 0; i < SIGNAL_TYPE_COUNT; i++)
	{
		for (int j = 0; j < IN_OUT_TYPE_COUNT; j++)
		{
			typeIndexes[i * SIGNAL_TYPE_COUNT + j] = static_cast<int>(fieldNameList.indexOf(typeName(i, j)));
		}
	}

	m_propertyBehaviorDescription.clear();
	m_propertyIndex2BehaviourIndexMap.clear();

	for (QString row : rows)
	{
		row = row.trimmed();

		if (row.isEmpty() == true)
		{
			continue;
		}

		QStringList fields = row.split(';', Qt::KeepEmptyParts);
		trimm(fields);

		if (nameIndex > fields.size())
		{
			continue;
		}

		PropertyBehaviourDescription behaviour;
		behaviour.name = fields[nameIndex];

		if (precisionIndex < fields.size())
		{
			behaviour.dependsOnPrecision = fields[precisionIndex].toLower() == "true";
		}

		for (size_t i = 0; i < static_cast<size_t>(TOTAL_SIGNAL_TYPE_COUNT); i++)
		{
			if (typeIndexes[i] < 0 && typeIndexes[i] >= fields.size())
			{
				continue;
			}

			bool ok = false;
			E::PropertyBehaviourType behaviourType = E::stringToValue<E::PropertyBehaviourType>(fields[typeIndexes[i]], &ok);
			if (ok == true)
			{
				behaviour.behaviourType[i] = behaviourType;
			}
		}

		m_propertyBehaviorDescription.push_back(behaviour);

		int propIndex = propertyIndex(behaviour.name);

		if (propIndex != -1)
		{
			int behaviourIndex = static_cast<int>(m_propertyBehaviorDescription.size()) - 1;

			Q_ASSERT(m_propertyDescription[propIndex].name == m_propertyBehaviorDescription[behaviourIndex].name);

			m_propertyIndex2BehaviourIndexMap.emplace(propIndex, behaviourIndex);
		}
	}
}

void AppSignalPropertyManager::clear()
{
	if (m_propertyDescription.size() > m_replacedPropertyDescription.size())
	{
		emit propertyCountWillDecrease(static_cast<int>(m_replacedPropertyDescription.size()));
		m_propertyDescription = m_replacedPropertyDescription;
		updatePropertyName2IndexMap();
		emit propertyCountDecreased();
		return;
	}

	if (m_propertyDescription.size() < m_replacedPropertyDescription.size())
	{
		emit propertyCountWillIncrease(static_cast<int>(m_replacedPropertyDescription.size()));
		m_propertyDescription = m_replacedPropertyDescription;
		updatePropertyName2IndexMap();
		emit propertyCountIncreased();
		return;
	}
}

void AppSignalPropertyManager::init()
{
	clear();
	loadNotSpecificProperties();
}

bool AppSignalPropertyManager::isNotCorrect(int propertyIndex) const
{
	if (propertyIndex < 0 || propertyIndex >= static_cast<int>(m_propertyDescription.size()))
	{
		return true;
	}
	return false;
}

QString AppSignalPropertyManager::typeName(E::SignalType type, E::SignalInOutType inOutType)
{
	return E::valueToString<E::SignalType>(type) + E::valueToString<E::SignalInOutType>(inOutType);
}

QString AppSignalPropertyManager::typeName(int typeIndex, int inOutTypeIndex)
{
	return typeName(IntToEnum<E::SignalType>(QMetaEnum::fromType<E::SignalType>().value(typeIndex)),
					IntToEnum<E::SignalInOutType>(QMetaEnum::fromType<E::SignalInOutType>().value(inOutTypeIndex)));
}

TuningValue AppSignalPropertyManager::variant2TuningValue(const QVariant& variant, TuningValueType type)
{
	TuningValue value;
	value.setType(type);

	bool ok = false;
	value.fromString(variant.toString(), &ok);
	assert(ok == true);

	return value;
}

bool AppSignalPropertyManager::isHidden(E::PropertyBehaviourType behaviour, bool isExpert) const
{
	bool hidden = behaviour == E::PropertyBehaviourType::Hide;
	hidden |= behaviour == E::PropertyBehaviourType::Expert && isExpert == false;
	return hidden;
}

bool AppSignalPropertyManager::isReadOnly(E::PropertyBehaviourType behaviour, bool isExpert) const
{
	bool readOnly = behaviour != E::PropertyBehaviourType::Write;
	readOnly |= behaviour == E::PropertyBehaviourType::Expert && isExpert == false;
	return readOnly;
}

void AppSignalPropertyManager::addNewProperty(const AppSignalPropertyDescription& newProperty)
{
	if (m_propertyName2IndexMap.contains(newProperty.name))
	{
		Q_ASSERT(false);
		return;
	}

	if (index(newProperty.name) != -1)
	{
		return;
	}

	emit propertyCountWillIncrease(static_cast<int>(m_propertyDescription.size() + 1));

	int propertyIndex = static_cast<int>(m_propertyDescription.size());

	m_propertyDescription.push_back(newProperty);

	m_propertyName2IndexMap.emplace(newProperty.name, propertyIndex);

	emit propertyCountIncreased();

	for (size_t i = 0; i < m_propertyBehaviorDescription.size(); i++)
	{
		if (newProperty.name == m_propertyBehaviorDescription[i].name)
		{
			m_propertyIndex2BehaviourIndexMap.emplace(propertyIndex, static_cast<int>(i));
			break;
		}
	}
}

void AppSignalPropertyManager::trimm(QStringList& stringList)
{
	for (QString& string : stringList)
	{
		string = string.trimmed();
	}
}

