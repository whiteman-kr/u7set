#include "AppSignalSetProvider.h"
#include "../DbLib/DbController.h"

#include <QMessageBox>

AppSignalPropertyManager* AppSignalPropertyManager::m_instance = nullptr;
const std::map<int, QString> AppSignalPropertyManager::m_emptyEnumValuesMap;

// is initialized by non specific properties
//
const std::vector<AppSignalPropertyDescription> AppSignalPropertyManager::m_replacedPropertyDescription =
{
/*	{
		true,
		AppSignalPropNames::APP_SIGNAL_ID,
		AppSignalPropNames::APP_SIGNAL_ID,
		QMetaType::QString,
		[](const AppSignal* s){ return s->appSignalID(); },
		[](AppSignal* s, QVariant v){ s->setAppSignalID(v.toString()); },
		{},
		{}
	},

	{
		true,
		AppSignalPropNames::CUSTOM_APP_SIGNAL_ID,
		AppSignalPropNames::CUSTOM_APP_SIGNAL_ID,
		QMetaType::QString,
		[](const AppSignal* s){ return s->customAppSignalID(); },
		[](AppSignal* s, QVariant v){ s->setCustomAppSignalID(v.toString()); },
		{},
		{}
	},

	{
		true,
		AppSignalPropNames::EQUIPMENT_ID,
		AppSignalPropNames::EQUIPMENT_ID,
		QMetaType::QString,
		[](const AppSignal* s){ return s->equipmentID(); },
		[](AppSignal* s, QVariant v){ s->setEquipmentID(v.toString()); },
		{},
		{}
	},

	{
		true,
		AppSignalPropNames::BUS_TYPE_ID,
		AppSignalPropNames::BUS_TYPE_ID,
		QMetaType::QString,
		[](const AppSignal* s){ return s->busTypeID(); },
		[](AppSignal* s, QVariant v){ s->setBusTypeID(v.toString()); },
		{},
		{}
	}, */

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
		assert(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].caption;
}

QString AppSignalPropertyManager::name(int propertyIndex)
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].name;
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

	if (signal->appSignalID() == "#SYSTEMID_RACK01_FSCC01_MD00_CTRLIN_INH03B" && property.name == "ElectricUnit")
	{
		DEBUG_STOP;
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

/*const std::map<int, QString>& AppSignalPropertyManager::propertyEnumValues(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return m_emptyEnumValuesMap;
	}

	return m_propertyDescription[static_cast<size_t>(propertyIndex)].enumValues;
}*/

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

void AppSignalPropertyManager::setValue(AppSignal* signal, int propertyIndex, const QVariant& value, bool isExpert)
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);
	if (isHidden(behaviour, isExpert) || isReadOnly(behaviour, isExpert))
	{
		assert(false);
	}

	m_propertyDescription[static_cast<size_t>(propertyIndex)].valueSetter(signal, value);
}

QMetaType::Type AppSignalPropertyManager::type(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QMetaType::UnknownType;
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].type;
}

E::PropertyBehaviourType AppSignalPropertyManager::getBehaviour(const AppSignal& signal, const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return defaultBehaviour;
	}

	return getBehaviour(signal.signalType(), signal.inOutType(), propertyIndex);
}

E::PropertyBehaviourType AppSignalPropertyManager::getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const
{
	int bhIndex = behaviourIndex(propertyIndex);

	if (bhIndex == -1)
	{
		return defaultBehaviour;
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
				return m_propertyBehaviorDescription[static_cast<size_t>(bhIndex)].behaviourType[static_cast<size_t>(i * typeEnum.keyCount() + j)];
			}
		}
	}

	return defaultBehaviour;
}

bool AppSignalPropertyManager::dependsOnPrecision(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
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

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(signal.specPropStruct());

	if (result.first == false)
	{
		assert(false);
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

		if (propertyName == "SensorType")
		{
			DEBUG_STOP;
		}

		QMetaType::Type type = static_cast<QMetaType::Type>(specificProperty->value().typeId());

		newProperty.specificProperty = false;
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
			assert(false);
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

// Loads properties that uninitialized signal contains
//
void AppSignalPropertyManager::loadNotSpecificProperties()
{
	AppSignal signal;
	AppSignalProperties signalProperties(signal, true);
	std::vector<AppSignalPropertyDescription> propertyDescriptions = signalProperties.getProperties();

	for (AppSignalPropertyDescription& property : propertyDescriptions)
	{
		if (property.name == "InOutType")
		{
			DEBUG_STOP;
		}

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

	std::vector<int> typeIndexes(static_cast<size_t>(TOTAL_SIGNAL_TYPE_COUNT), -1);
	for (int i = 0; i < SIGNAL_TYPE_COUNT; i++)
	{
		for (int j = 0; j < IN_OUT_TYPE_COUNT; j++)
		{
			typeIndexes[static_cast<size_t>(i * SIGNAL_TYPE_COUNT + j)] = static_cast<int>(fieldNameList.indexOf(typeName(i, j)));
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
		assert(false);
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


AppSignalSetProvider* AppSignalSetProvider::m_instance = nullptr;

AppSignalSetProvider::AppSignalSetProvider(DbController* dbController, QWidget* parentWidget) :
	QObject(parentWidget),
	m_dbController(dbController),
	m_propertyManager(dbController, parentWidget)
{
	assert(m_instance == nullptr);
	m_instance = this;

	connect(this, &AppSignalSetProvider::signalPropertiesChanged, &m_propertyManager, &AppSignalPropertyManager::detectNewProperties);
}

AppSignalSetProvider::~AppSignalSetProvider()
{
	if (m_lazyLoadSignalsTimer != nullptr)
	{
		m_lazyLoadSignalsTimer->stop();
		delete m_lazyLoadSignalsTimer;
	}
}

AppSignalSetProvider* AppSignalSetProvider::getInstance()
{
	assert(m_instance != nullptr);
	return m_instance;
}

void AppSignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
}


AppSignal* AppSignalSetProvider::getSignalByStrID(const QString signalStrID)
{
	if (m_signalSet.ID2IndexMapIsEmpty())
	{
		m_signalSet.buildID2IndexMap();
	}
	return m_signalSet.getSignal(signalStrID);
}

const AppSignal& AppSignalSetProvider::getLoadedSignal(int index)
{
	if (m_signalSet[index].isLoaded() == false)
	{
		int oldIndex = m_middleVisibleSignalIndex;
		m_middleVisibleSignalIndex = index;

		loadNextSignalsPortion();	// force loading this signal

		m_middleVisibleSignalIndex = oldIndex;

	}
	return m_signalSet[index];
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(int index)
{
	AppSignal signal = getLoadedSignal(index);
	signal.cacheSpecPropValues();

	AppSignalParam param;
	param.load(signal);

	return param;
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(QString appSignalId)
{
	AppSignalParam param;

	AppSignal* signal = getSignalByStrID(appSignalId);
	if (signal == nullptr)
	{
		assert(false);
		return param;
	}

	if (signal->isLoaded())
	{
		signal->cacheSpecPropValues();
		param.load(*signal);
		return param;
	}

	return getAppSignalParam(static_cast<int>(m_signalSet.keyIndex(signal->ID())));
}

QVector<int> AppSignalSetProvider::getSameChannelSignals(int index)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		QVector<int> sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
		foreach (const int id, sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(static_cast<int>(m_signalSet.keyIndex(id)));
		}
	}
	else
	{
		sameChannelSignalRows.append(index);
	}
	return sameChannelSignalRows;
}

void AppSignalSetProvider::loadUsers()
{
	std::vector<DbUser> list;
	m_dbController->getUserList(&list, nullptr);

	m_usernameMap.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}
}

bool AppSignalSetProvider::isEditableSignal(const AppSignal& signal) const
{
	if (!signal.checkedOut() || (signal.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}
	return false;
}

bool AppSignalSetProvider::isCheckinableSignalForMe(const AppSignal& signal) const
{
	if (signal.checkedOut() && (signal.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}
	return false;
}

QString AppSignalSetProvider::getUserStr(int userId) const
{
	if (m_usernameMap.contains(userId))
	{
		return m_usernameMap[userId];
	}
	else
	{
		return "";
	}
}

bool AppSignalSetProvider::checkoutSignal(int index)
{
	AppSignal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == m_dbController->currentUser().userId() || dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	m_dbController->checkoutSignals(&signalsIDs, &objectStates, nullptr);
	if (objectStates.count() == 0)
	{
		return false;
	}
	showErrors(objectStates);
	for (const ObjectState& objectState : objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			return false;
		}
	}
	for (int id : signalsIDs)
	{
		loadSignal(id);
	}
	return true;
}

// Converts ObjectState to human readable message
//
QString AppSignalSetProvider::errorMessage(const ObjectState& state)
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT: return tr("Signal %1 is not checked out").arg(state.id);
		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER: return tr("Signal %1 is checked out by \"%2\"").arg(state.id).arg(m_usernameMap[state.userId]);
		case ERR_SIGNAL_DELETED: return tr("Signal %1 was deleted already").arg(state.id);
		case ERR_SIGNAL_NOT_FOUND: return tr("Signal %1 not found").arg(state.id);
		case ERR_SIGNAL_EXISTS: return "";				// error message is displayed by PGSql driver
		default:
			return tr("Unknown error %1").arg(state.errCode);
	}
}

// Throws error signal with human readable message for single ObjectState
//
void AppSignalSetProvider::showError(const ObjectState& state)
{
	if (state.errCode != ERR_SIGNAL_OK)
	{
		QString message = errorMessage(state);
		if (!message.isEmpty())
		{
			emit error(message);
		}
	}
}

// Throws single error signal with human readable message for set of ObjectState
//
void AppSignalSetProvider::showErrors(const QVector<ObjectState>& states)
{
	QString message;

	foreach (const ObjectState& state, states)
	{
		if (state.errCode != ERR_SIGNAL_OK)
		{
			if (message.isEmpty() == false)
			{
				message += "\n";
			}

			message += errorMessage(state);
		}
	}

	if (message.isEmpty() == false)
	{
		emit error(message);
	}
}


void AppSignalSetProvider::initLazyLoadSignals()
{
	loadUsers();

	m_propertyManager.init();
	m_propertyManager.reloadPropertyBehaviour();

	QVector<ID_AppSignalID> signalIds;
	dbController()->getSignalsIDAppSignalID(&signalIds, nullptr);

	for (const ID_AppSignalID& id : signalIds)
	{
		m_signalSet.replaceOrAppendIfNotExists(id.ID, AppSignal(id));
	}

	emit signalCountChanged();
	m_partialLoading = true;

	if (m_lazyLoadSignalsTimer == nullptr)
	{
		m_lazyLoadSignalsTimer = new QTimer(this);
		connect(m_lazyLoadSignalsTimer, &QTimer::timeout, this, &AppSignalSetProvider::loadNextSignalsPortion);
	}

	m_lazyLoadSignalsTimer->start(100);
}

void AppSignalSetProvider::stopLoadingSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();
		m_partialLoading = false;
	}
}

void AppSignalSetProvider::finishLoadingSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();

		QVector<int> signalIds;
		for (int i = 0; i < m_signalSet.count(); i++)
		{
			if (m_signalSet[i].isLoaded() == false)
			{
				signalIds.push_back(m_signalSet.key(i));
			}
		}

		if (signalIds.count() > 0)
		{
			QVector<AppSignal> signalsToLoad;
			signalsToLoad.reserve(signalIds.count());

			dbController()->getLatestSignals(signalIds, &signalsToLoad, nullptr);

			for (const AppSignal& loadedSignal: signalsToLoad)
			{
				m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

				emit signalUpdated(keyIndex(loadedSignal.ID()));
				emit signalPropertiesChanged(loadedSignal);
			}
		}
	}

	m_partialLoading = false;
}

void AppSignalSetProvider::loadNextSignalsPortion()
{
	if (m_partialLoading == false)
	{
		return;
	}
	QVector<int> signalIds;
	signalIds.reserve(250);
	int low = m_middleVisibleSignalIndex - 1;
	int high = m_middleVisibleSignalIndex;

	if (m_middleVisibleSignalIndex == -1)
	{
		high = 0;
	}

	while ((low >= 0 || high < signalCount()) && signalIds.count() <= 248)
	{
		while (low >= 0 && m_signalSet[low].isLoaded() == true)
		{
			low--;
		}

		if (low >= 0)
		{
			signalIds.push_back(m_signalSet.key(low));
			low--;
		}

		while (high < signalCount() && m_signalSet[high].isLoaded() == true)
		{
			high++;
		}

		if (high < signalCount())
		{
			signalIds.push_back(m_signalSet.key(high));
			high++;
		}
	}

	if (signalIds.count() > 0)
	{
		QVector<AppSignal> signalsToLoad;
		signalsToLoad.reserve(signalIds.count());

		dbController()->getLatestSignalsWithoutProgress(signalIds, &signalsToLoad, nullptr);

		for (const AppSignal& loadedSignal : signalsToLoad)
		{
			m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);
		}

		for (const AppSignal& loadedSignal : signalsToLoad)
		{
			emit signalUpdated(keyIndex(loadedSignal.ID()));
			emit signalPropertiesChanged(loadedSignal);
		}
	}
	else
	{
		m_partialLoading = false;
	}
}

bool AppSignalSetProvider::checkoutSignal(int index, QString& message)
{
	AppSignal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	dbController()->checkoutSignals(&signalsIDs, &objectStates, nullptr);
	if (objectStates.count() == 0)
	{
		return false;
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(objectState) + "\n";
		}
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			return false;
		}
	}
	for (int id : signalsIDs)
	{
		loadSignal(id);
	}
	return true;
}


bool AppSignalSetProvider::undoSignal(int id)
{
	const AppSignal& s = m_signalSet[m_signalSet.keyIndex(id)];
	if (!s.checkedOut())
	{
		return false;
	}

	QVector<int> signalsIDs;
	if (s.signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(s.signalGroupID());
	}
	else
	{
		signalsIDs << id;
	}
	QVector<ObjectState> states;

	for (int signalId : signalsIDs)
	{
		ObjectState state;
		dbController()->undoSignalChanges(signalId, &state, nullptr);
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}

	if (!states.isEmpty())
	{
		showErrors(states);
	}

	for (int signalId : signalsIDs)
	{
		loadSignal(signalId);
	}

	return true;
}

void AppSignalSetProvider::deleteSignal(int signalID)
{
	ObjectState state;
	dbController()->deleteSignal(signalID, &state, nullptr);
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
}

void AppSignalSetProvider::addSignal(AppSignal& signal)
{
	m_signalSet.replaceOrAppendIfNotExists(signal.ID(), signal);
}

void AppSignalSetProvider::deleteSignals(const QSet<int>& signalIDs)
{
	for (const int signalID : signalIDs)
	{
		deleteSignal(signalID);
	}
	loadSignals();
}

void AppSignalSetProvider::loadSignalSet(QVector<int> keys)
{
	for (int i = 0; i < keys.count(); i++)
	{
		loadSignal(keys[i]);
	}
}

const AppSignal* AppSignalSetProvider::loadSignal(int signalId)
{
	int index = keyIndex(signalId);

	if (index == -1)
	{
		return nullptr;
	}

	dbController()->getLatestSignal(signalId, &m_signalSet[index], nullptr);
	m_signalSet.updateID2IndexInMap(m_signalSet[index].appSignalID(), index);

	emit signalUpdated(index);
	emit signalPropertiesChanged(getLoadedSignal(index));

	return &m_signalSet[index];
}

void AppSignalSetProvider::loadSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();
		m_partialLoading = false;
	}

	m_propertyManager.init();
	m_propertyManager.reloadPropertyBehaviour();

	loadUsers();

	AppSignalSet signalSetForReplacement;

	if (!dbController()->getSignals(&signalSetForReplacement, false, nullptr))
	{
		emit error(tr("Could not load signals"));
	}

	for (int i = 0; i < signalSetForReplacement.count(); i++)
	{
		m_propertyManager.detectNewProperties(signalSetForReplacement[i]);
	}

	m_signalSet.clear();

	m_signalSet = std::move(signalSetForReplacement);
	signalSetForReplacement.forget();	// Destructor will delete all Signal pointers which should be kept for m_signalSet

	emit signalCountChanged();
}

void AppSignalSetProvider::saveSignal(AppSignal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);

	dbController()->setSignalWorkcopy(&signal, &state, nullptr);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	loadSignal(signal.ID());
}

void AppSignalSetProvider::saveSignals(QVector<AppSignal*> signalVector)
{
	QVector<ObjectState> states;
	for (int i = 0; i < signalVector.count(); i++)
	{
		ObjectState state;
		trimSignalTextFields(*signalVector[i]);

		dbController()->setSignalWorkcopy(signalVector[i], &state, nullptr);
		states.append(state);

		loadSignal(signalVector[i]->ID());
	}
	showErrors(states);
}

QVector<int> AppSignalSetProvider::cloneSignals(const QSet<int>& signalIDs)
{
	QVector<int> resultSignalIDs;
	m_signalSet.buildID2IndexMap();

	QSet<int> clonedSignalIDs;
	QList<int> signalIDsList = signalIDs.values();
	std::sort(signalIDsList.begin(), signalIDsList.end());
	for (const int signalID : signalIDsList)
	{
		if (clonedSignalIDs.contains(signalID))
		{
			continue;
		}

		const AppSignal&& signal = m_signalSet.value(signalID);
		E::SignalType type = signal.signalType();
		QVector<int> groupSignalIDs;

		if (signal.signalGroupID() == 0)
		{
			groupSignalIDs.append(signal.ID());
		}
		else
		{
			groupSignalIDs = m_signalSet.getChannelSignalsID(signal);

			if (groupSignalIDs.size() == 0)
			{
				Q_ASSERT(false);
				continue;
			}
		}
		std::sort(groupSignalIDs.begin(), groupSignalIDs.end());

		for (int groupSignalID : groupSignalIDs)
		{
			clonedSignalIDs.insert(groupSignalID);
		}

		QString suffix = "_CLONE";
		int suffixNumerator = 1;
		bool hasConflict;
		do
		{
			hasConflict = false;
			for (int groupSignalID : groupSignalIDs)
			{
				if (m_signalSet.contains(m_signalSet.value(groupSignalID).appSignalID() + suffix))
				{
					hasConflict = true;
					break;
				}
			}
			if (hasConflict)
			{
				suffixNumerator++;
				suffix = QString("_CLONE%1").arg(suffixNumerator);
			}
		}
		while (hasConflict && suffixNumerator < 1000);

		if (suffixNumerator >= 1000)
		{
			assert(false);
			return QVector<int>();
		}

		QVector<AppSignal> groupSignals(groupSignalIDs.count());
		for (int i = 0; i < groupSignalIDs.count(); i++)
		{
			const AppSignal&& groupSignal = m_signalSet.value(groupSignalIDs[i]);
			groupSignals[i] = groupSignal;
			trimSignalTextFields(groupSignals[i]);

			groupSignals[i].setAppSignalID(groupSignal.appSignalID() + suffix);
			groupSignals[i].setCustomAppSignalID(groupSignal.customAppSignalID() + suffix);
		}

		dbController()->addSignal(type, &groupSignals, nullptr);

		qsizetype prevSize = resultSignalIDs.size();
		resultSignalIDs.resize(prevSize + groupSignals.count());

		for (int i = 0; i < groupSignals.count(); i++)
		{
			resultSignalIDs[prevSize + i] = groupSignals[i].ID();
		}
	}
	loadSignals();
	return resultSignalIDs;
}

void AppSignalSetProvider::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		m_propertyManager.clear();
		m_signalSet.clear();
		emit signalCountChanged();
	}
}

void AppSignalSetProvider::trimSignalTextFields(AppSignal& signal)
{
	signal.setAppSignalID(signal.appSignalID().trimmed());
	signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
	signal.setEquipmentID(signal.equipmentID().trimmed());
	signal.setBusTypeID(signal.busTypeID().trimmed());
	signal.setCaption(signal.caption().trimmed());
	signal.setUnit(signal.unit().trimmed());
}
