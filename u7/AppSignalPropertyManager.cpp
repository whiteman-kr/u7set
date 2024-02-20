#include "AppSignalPropertyManager.h"
#include "AppSignalSetProvider.h"
#include "../UtilsLib/WUtils.h"

// this replacement for properties required to change getter return type from E::* to QString
//
const std::vector<AppSignalPropertyDescription> AppSignalPropertyManager::m_replacedPropDescriptions =
{
	{
		AppSignalPropNames::TYPE,
		QMetaType::QString,
		false,
		[](const AppSignal* s){ return E::valueToString<E::SignalType>(s->signalType()); },
		nullptr,
		AppSignalProperties::NON_SPECIFIC_PROP_HASH,
		E::enumValuesMap<E::SignalType>(),
	},

	{
		AppSignalPropNames::IN_OUT_TYPE,
		QMetaType::QString,
		false,
		[](const AppSignal* s) { return E::valueToString<E::SignalInOutType>(s->inOutType()); },
		nullptr,
		AppSignalProperties::NON_SPECIFIC_PROP_HASH,
		E::enumValuesMap<E::SignalInOutType>(),
	},

	{
		AppSignalPropNames::BYTE_ORDER_PROP,
		QMetaType::QString,
		false,
		[](const AppSignal* s) { return E::valueToString<E::ByteOrder>(s->byteOrder()); },
		nullptr,
		AppSignalProperties::NON_SPECIFIC_PROP_HASH,
		E::enumValuesMap<E::ByteOrder>(),
	},

	{
		AppSignalPropNames::CHECKOUT_BY_USER,
		QMetaType::QString,
		false,
		[](const AppSignal* s) {
									return (s->checkedOut() ?
												AppSignalSetProvider::getInstance()->getUserName(s->userID()) :
												QString());
								},
		[](AppSignal*, const QVariant&) {
													// no assigns
										},
	},

	{
		AppSignalPropNames::ANALOG_SIGNAL_FORMAT,
		QMetaType::QString,
		false,
		[](const AppSignal* s) { return E::valueToString<E::AnalogAppSignalFormat>(s->analogSignalFormat()); },
		[](AppSignal* s, const QVariant& v) { s->setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(v.toInt())); },
		AppSignalProperties::NON_SPECIFIC_PROP_HASH,
		E::enumValuesMap<E::AnalogAppSignalFormat>(),
	},

	{
		AppSignalPropNames::APERTURE_TYPE,
		QMetaType::QString,
		false,
		[](const AppSignal* s) { return E::valueToString<E::ApertureType>(s->apertureType()); },
		[](AppSignal* s, const QVariant& v) { s->setApertureType(static_cast<E::ApertureType>(v.toInt())); },
		AppSignalProperties::NON_SPECIFIC_PROP_HASH,
		E::enumValuesMap<E::ApertureType>()
	},
};

AppSignalPropertyManager* AppSignalPropertyManager::m_instance = nullptr;

const std::map<int, QString> AppSignalPropertyManager::m_emptyEnumValuesMap;
const AppSignalPropertyDescription AppSignalPropertyManager::m_notValidPropDescription;

std::vector<AppSignalPropertyDescription> AppSignalPropertyManager::m_notSpecificPropDescriptions;

const std::vector<E::SignalType> AppSignalPropertyManager::m_signalTypes = E::values<E::SignalType>();
const std::vector<E::SignalInOutType> AppSignalPropertyManager::m_inOutTypes = E::values<E::SignalInOutType>();

AppSignalPropertyManager::AppSignalPropertyManager()
{
	Q_ASSERT(m_instance == nullptr);
	m_instance = this;

	initNotSpecificPropDescriptions();
}

AppSignalPropertyManager* AppSignalPropertyManager::getInstance()
{
	Q_ASSERT(m_instance != nullptr);
	return m_instance;
}

int AppSignalPropertyManager::count() const
{
	return static_cast<int>(m_propDescriptions.size());
}

QString AppSignalPropertyManager::name(int propertyIndex)
{
	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return QString();
	}

	return m_propDescriptions[propertyIndex].name();
}

int AppSignalPropertyManager::propertyIndex(const QString& name)
{
	auto it = m_propNameToIndex.find(name);

	if (it == m_propNameToIndex.end())
	{
		return -1;
	}

	return it->second;
}

bool AppSignalPropertyManager::getSignalEnumPropertyValues(const AppSignal& s, int propertyIndex,
														   std::vector<std::pair<int, QString>>* enumValues) const
{
	TEST_PTR_RETURN_FALSE(enumValues);

	enumValues->clear();

	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return false;
	}

	const AppSignalPropertyDescription& appSignalProperty = m_propDescriptions[propertyIndex];

	Q_ASSERT(appSignalProperty.isEnumProperty());

	if (appSignalProperty.isSpecificProperty() == false)
	{
		return appSignalProperty.getEnumValuesVector(AppSignalProperties::NON_SPECIFIC_PROP_HASH, enumValues);
	}

	return appSignalProperty.getEnumValuesVector(s.specPropStructHash(), enumValues);

/*	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(s.specPropStruct());

	if (result.first == false)
	{
		Q_ASSERT(false);
		return false;
	}

	std::shared_ptr<Property> property = propObject.propertyByCaption(appSignalProperty.name);

	TEST_PTR_RETURN_FALSE(property);

	*enumValues = property->enumValues();

	return true; */
}

bool AppSignalPropertyManager::isEnumProperty(int propertyIndex) const
{
	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return false;
	}

	return m_propDescriptions[propertyIndex].isEnumProperty();
}

QVariant AppSignalPropertyManager::value(const AppSignal* signal, int propertyIndex, bool isExpert) const
{
	TEST_PTR_RETURN_VALUE(signal, QVariant());

	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return QVariant();
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);

	if (isHidden(behaviour, isExpert))
	{
		return QVariant();
	}

	const AppSignalPropertyDescription& property = m_propDescriptions[propertyIndex];

	TEST_PTR_RETURN_VALUE(property.getter(), QVariant());

	if (property.isSpecificProperty() == false)
	{
		return property.getter()(signal);
	}

	if (property.isSignalHaveProperty(signal->ID()) == false)
	{
		return QVariant();
	}

	if (property.isEnumProperty() == false)
	{
		return property.getter()(signal);
	}

	int value = property.getter()(signal).toInt();

	if (property.isSpecificProperty() == true)
	{
		return property.getEnumValueStr(signal->specPropStructHash(), value);
	}

	return property.getEnumValueStr(AppSignalProperties::NON_SPECIFIC_PROP_HASH, value);

/*	auto it = property.enumValues.find(value);

	if (it != property.enumValues.end())
	{
		return it->second;
	}

	return QString("Unknown value (%1)").arg(value); */
}

bool AppSignalPropertyManager::setValue(AppSignal* signal, int propertyIndex, const QVariant& newValue, bool isExpert)
{
	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return false;
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);

	if (isHidden(behaviour, isExpert) || isReadOnly(behaviour, isExpert))
	{
		Q_ASSERT(false);
		return false;
	}

	AppSignalPropertyDescription& propDesc = m_propDescriptions[propertyIndex];

	QVariant prevValue = value(signal, propertyIndex, isExpert);

	if (propDesc.isEnumProperty() == true)
	{
		// for enum properties prevValue returnes as string, ex. "SignedInt32", but newValue is a number
		// so, convertion of newValue from number to enum value String is required!
		//
		QVariant newValueStr = propDesc.getEnumValueStr(signal->specPropStructHash(),newValue.toInt());

		if (prevValue == newValueStr)
		{
			return false;
		}
	}
	else
	{
		if (prevValue == newValue)
		{
			return false;
		}
	}

	TEST_PTR_RETURN_FALSE(propDesc.setter());

	propDesc.setter()(signal, newValue);

	return true;
}

const AppSignalPropertyDescription& AppSignalPropertyManager::getPropertyDescription(int propertyIndex) const
{
	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return m_notValidPropDescription;
	}

	return m_propDescriptions[propertyIndex];
}

QMetaType::Type AppSignalPropertyManager::type(const int propertyIndex) const
{
	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return QMetaType::UnknownType;
	}

	return m_propDescriptions[propertyIndex].type();
}

E::PropertyBehaviourType AppSignalPropertyManager::getBehaviour(const AppSignal& signal, const int propertyIndex) const
{
	if (isValidPropIndex(propertyIndex) == false)
	{
		Q_ASSERT(false);
		return m_defaultBehaviour;
	}

	return m_propDescriptions[propertyIndex].getBehaviour(signal);
}

bool AppSignalPropertyManager::dependsOnPrecision(const QString& propName) const
{
	return dependsOnPrecision(propertyIndex(propName));
}

bool AppSignalPropertyManager::dependsOnPrecision(int propIndex) const
{
	ASSERT_RETURN_IF_FALSE(isValidPropIndex(propIndex));

	return m_propDescriptions[propIndex].dependsOnPrecision();
}

bool AppSignalPropertyManager::isHiddenFor(E::SignalType type, const int propertyIndex, bool isExpert) const
{
	ASSERT_RETURN_IF_FALSE(isValidPropIndex(propertyIndex));

	const AppSignalPropertyDescription& propDescription = m_propDescriptions[propertyIndex];

	for (E::SignalInOutType inOutType : m_inOutTypes)
	{
		E::PropertyBehaviourType behaviour = propDescription.getBehaviour(type, inOutType);

		if (isHidden(behaviour, isExpert) == false)
		{
			return false;
		}
	}

	return true;
}

void AppSignalPropertyManager::slot_detectNewProperties(const std::vector<int>& signalIndexes)
{
	const AppSignalSetProvider* provider = AppSignalSetProvider::getInstance();

	for(int index : signalIndexes)
	{
		const AppSignal* s = provider->getSignalByIndex(index);

		TEST_PTR_CONTINUE(s);

		detectNewProperties(*s);
	}
}

void AppSignalPropertyManager::detectNewProperties(const AppSignal& signal)
{
	if (signal.specPropStruct().isEmpty() == true)
	{
		return;
	}

	Hash specPropStructHash = signal.specPropStructHash();

	auto it = m_parsedSpecPropStruct.find(specPropStructHash);

	const PropertyObject* propObject = nullptr;

	if (it == m_parsedSpecPropStruct.end())
	{
		auto [newIt, b] = m_parsedSpecPropStruct.emplace(specPropStructHash, PropertyObject{});

		std::pair<bool, QString> result = newIt->second.parseSpecificPropertiesStruct(signal.specPropStruct());

		if (result.first == false)
		{
			Q_ASSERT(false);
			return;
		}

		propObject = &newIt->second;
	}
	else
	{
		propObject = &it->second;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject->properties();

	for(const std::shared_ptr<Property>& specificProperty : specificProperties)
	{
		bool propertyIsEnum = specificProperty->isEnum();

		int propIndex = propertyIndex(specificProperty->caption());

		if (propIndex != -1)
		{
			m_propDescriptions[propIndex].appendSignalID(signal.ID());

			if (propertyIsEnum == true)
			{
				m_propDescriptions[propIndex].checkEnumValues(specPropStructHash, specificProperty->enumValues());
			}

			continue;
		}

		QMetaType::Type type = static_cast<QMetaType::Type>(specificProperty->value().typeId());

		switch (type)
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

		const QString& propertyName = specificProperty->caption();

		auto getter = [propertyIsEnum, propertyName, type](const AppSignal* s)
						{
							QVariant qv;

							bool isEnum = propertyIsEnum;

							bool result = s->getSpecPropValue(propertyName, &qv, &isEnum, nullptr);

							if (result == false)
							{
								return QVariant();
							}

							Q_ASSERT(qv.typeId() == type);

							return qv;
						};

		auto setter = [propertyIsEnum, propertyName](AppSignal* s, const QVariant& v)
						{
							bool isEnum = propertyIsEnum;

							bool result = s->setSpecPropValue(propertyName, v, isEnum);

							assert(result == true);

							Q_UNUSED(result);
						};

		AppSignalPropertyDescription newProperty;

		if (propertyIsEnum == true)
		{
			auto valuesVector = specificProperty->enumValues();

			newProperty.initEnumProp(specificProperty->caption(), type, true, getter, setter,
									 specPropStructHash, std::map<int, QString>(valuesVector.begin(), valuesVector.end()));
		}
		else
		{
			newProperty.initNonEnumProp(specificProperty->caption(), type, true, getter, setter);
		}

		addNewProperty(newProperty, true);
	}
}

void AppSignalPropertyManager::initNotSpecificPropDescriptions()
{
	m_notSpecificPropDescriptions.clear();

	m_notSpecificPropDescriptions = m_replacedPropDescriptions;

	std::set<QString> existPropNames;

	for(const AppSignalPropertyDescription& propDesc : m_notSpecificPropDescriptions)
	{
		existPropNames.emplace(propDesc.name());
	}

	AppSignal signal;
	AppSignalProperties signalProperties(signal, true);
	std::vector<AppSignalPropertyDescription> propertyDescriptions = signalProperties.getProperties();

	for (const AppSignalPropertyDescription& propDesc : propertyDescriptions)
	{
		if (existPropNames.contains(propDesc.name()))
		{
			continue;
		}

		auto propPtr = signalProperties.propertyByCaption(propDesc.name());

		if (propPtr != nullptr && propPtr->isCategorized() == true)
		{
			m_notSpecificPropDescriptions.emplace_back(propDesc);
		}
	}
}

void AppSignalPropertyManager::updatePropNameToIndexMap()
{
	m_propNameToIndex.clear();

	for (size_t i = 0; i < m_propDescriptions.size(); i++)
	{
		m_propNameToIndex.emplace(m_propDescriptions[i].name(), static_cast<int>(i));
	}
}

void AppSignalPropertyManager::updatePropDescriptionsBehaviour()
{
	for(AppSignalPropertyDescription& propDesc : m_propDescriptions)
	{
		auto it = m_propertiesBehaviour.find(propDesc.name());

		if (it == m_propertiesBehaviour.end())
		{
			propDesc.clearBehaviour();
		}
		else
		{
			propDesc.setBehaviour(it->second);
		}
	}
}

int AppSignalPropertyManager::propertyIndex(const QString& propName) const
{
	auto it = m_propNameToIndex.find(propName);

	return (it == m_propNameToIndex.end() ? -1 : it->second);
}

void AppSignalPropertyManager::updatePropertiesBehaviour(const QString& propBehavoiurFile, QString* errMsg)
{
	TEST_PTR_RETURN(errMsg);

	QStringList rows = propBehavoiurFile.split("\n", Qt::SkipEmptyParts);

	if (rows.isEmpty() == true)
	{
		*errMsg = "File SignalPropertyBehavior.csv is empty";
		return;
	}

	QStringList fieldNameList = rows[0].split(';', Qt::KeepEmptyParts);

	trimm(fieldNameList);

	rows.removeFirst();				// header row removed!

	QString uncorrectFileMsg = "Uncorrect format of file SignalPropertyBehavior.csv: ";

	qsizetype nameIndex = fieldNameList.indexOf("PropertyName");

	if (nameIndex < 0)
	{
		*errMsg = uncorrectFileMsg + " PropertyName column not found";
		return;
	}

	qsizetype precisionIndex = fieldNameList.indexOf("DependsOnPrecision");

	if (precisionIndex < 0)
	{
		*errMsg = uncorrectFileMsg + " DependosOnPrecision column not found";
		return;
	}

	bool isSafetyProject = AppSignalSetProvider::getInstance()->isSafetyProject();

	m_propertiesBehaviour.clear();

	for (QString row : rows)
	{
		row = row.trimmed();

		if (row.isEmpty() == true)
		{
			continue;
		}

		QStringList fields = row.split(';', Qt::KeepEmptyParts);
		trimm(fields);

		if (nameIndex >= fields.size() ||
			precisionIndex >= fields.size())
		{
			Q_ASSERT(false);
			continue;
		}

		QString propName = fields[nameIndex];

		if (m_propertiesBehaviour.contains(propName) == true)
		{
			Q_ASSERT(false);
			continue;
		}

		bool hidePropery = false;

		if (propName == AppSignalPropNames::INVERT_SIGNAL &&
			isSafetyProject == true)
		{
			hidePropery = true;
		}

		AppSignalPropertyBehavior behaviour;

		behaviour.setDependsOnPrecision(fields[precisionIndex].toLower() == "true");

		int fieldIndex = precisionIndex + 1;

		for(E::SignalType signalType : m_signalTypes)
		{
			for(E::SignalInOutType inOutType : m_inOutTypes)
			{
				if (fieldIndex >= 0 && fieldIndex <= fields.size())
				{
					bool ok = false;

					E::PropertyBehaviourType behaviourType = E::stringToValue<E::PropertyBehaviourType>(fields[fieldIndex], &ok);

					if (ok == true)
					{
						if (hidePropery == true)
						{
							behaviourType = E::PropertyBehaviourType::Hide;
						}

						behaviour.set(signalType, inOutType, behaviourType);
					}
					else
					{
						Q_ASSERT(false);
					}
				}
				else
				{
					Q_ASSERT(false);
				}

				fieldIndex++;
			}
		}

		m_propertiesBehaviour.emplace(propName, behaviour);
	}

	updatePropDescriptionsBehaviour();
}

void AppSignalPropertyManager::clear()
{
	bool increased = false;
	bool decreased = false;

	if (m_notSpecificPropDescriptions.size() > m_propDescriptions.size())
	{
		increased = true;
		emit propertyCountWillIncrease(static_cast<int>(m_notSpecificPropDescriptions.size()));
	}
	else
	{
		if (m_notSpecificPropDescriptions.size() < m_propDescriptions.size())
		{
			decreased = true;
			emit propertyCountWillDecrease(static_cast<int>(m_notSpecificPropDescriptions.size()));
		}
	}

	m_propDescriptions = m_notSpecificPropDescriptions;
	updatePropNameToIndexMap();

	m_parsedSpecPropStruct.clear();

	if (increased == true)
	{
		emit propertyCountIncreased();
	}
	else
	{
		if (decreased == true)
		{
			emit propertyCountDecreased();
		}
	}
}

bool AppSignalPropertyManager::isValidPropIndex(int propertyIndex) const
{
	return (propertyIndex >= 0 && propertyIndex < static_cast<int>(m_propDescriptions.size()));
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

void AppSignalPropertyManager::addNewProperty(const AppSignalPropertyDescription& newProperty, bool emitSignals)
{
	if (m_propNameToIndex.contains(newProperty.name()))
	{
		Q_ASSERT(false);
		return;
	}

	if (emitSignals == true)
	{
		emit propertyCountWillIncrease(static_cast<int>(m_propDescriptions.size() + 1));
	}

	m_propNameToIndex.emplace(newProperty.name(), static_cast<int>(m_propDescriptions.size()));
	m_propDescriptions.emplace_back(newProperty);

	if (emitSignals == true)
	{
		emit propertyCountIncreased();
	}
}

void AppSignalPropertyManager::trimm(QStringList& stringList)
{
	for (QString& string : stringList)
	{
		string = string.trimmed();
	}
}
