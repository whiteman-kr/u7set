#include "SignalSetProvider.h"
#include "../DbLib/DbController.h"

#include <QMessageBox>


SignalPropertyManager* SignalPropertyManager::m_instance = nullptr;

SignalPropertyManager::SignalPropertyManager(DbController* dbController, QWidget* parentWidget) :
	m_dbController(dbController),
	m_parentWidget(parentWidget)
{
	assert (m_instance == nullptr);
	init();
	m_instance = this;
}

SignalPropertyManager* SignalPropertyManager::getInstance()
{
	assert (m_instance != nullptr);
	return m_instance;
}

int SignalPropertyManager::count() const
{
	return static_cast<int>(m_propertyDescription.size());
}

int SignalPropertyManager::index(const QString& name)
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

QString SignalPropertyManager::caption(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].caption;
}

QString SignalPropertyManager::name(int propertyIndex)
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QString();
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].name;
}

QVariant SignalPropertyManager::value(const AppSignal* signal, int propertyIndex, bool isExpert) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QVariant();
	}

	E::PropertyBehaviourType behaviour = getBehaviour(*signal, propertyIndex);
	if (isHidden(behaviour, isExpert))
	{
		return QVariant();
	}

	const AppSignalPropertyDescription& property = m_propertyDescription[static_cast<size_t>(propertyIndex)];
	if (property.enumValues.size() == 0)
	{
		return property.valueGetter(signal);
	}
	else
	{
		int value = property.valueGetter(signal).toInt();
		for (const auto& enumValue : property.enumValues)
		{
			if (value == enumValue.first)
			{
				return enumValue.second;
			}
		}
		return QString("Unknown value (%1)").arg(value);
	}
}

const std::vector<std::pair<int, QString> > SignalPropertyManager::values(int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return {};
	}

	return m_propertyDescription[static_cast<size_t>(propertyIndex)].enumValues;
}

void SignalPropertyManager::setValue(AppSignal* signal, int propertyIndex, const QVariant& value, bool isExpert)
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

QVariant::Type SignalPropertyManager::type(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return QVariant::Invalid;
	}
	return m_propertyDescription[static_cast<size_t>(propertyIndex)].type;
}

E::PropertyBehaviourType SignalPropertyManager::getBehaviour(const AppSignal& signal, const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return defaultBehaviour;
	}

	return getBehaviour(signal.signalType(), signal.inOutType(), propertyIndex);
}

E::PropertyBehaviourType SignalPropertyManager::getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const
{
	int behaviourIndex = m_propertyIndex2BehaviourIndexMap.value(propertyIndex, -1);
	if (behaviourIndex == -1)
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
				return m_propertyBehaviorDescription[static_cast<size_t>(behaviourIndex)].behaviourType[static_cast<size_t>(i * typeEnum.keyCount() + j)];
			}
		}
	}

	return defaultBehaviour;
}

bool SignalPropertyManager::dependsOnPrecision(const int propertyIndex) const
{
	if (isNotCorrect(propertyIndex))
	{
		assert(false);
		return false;
	}

	int behaviourIndex = m_propertyIndex2BehaviourIndexMap.value(propertyIndex, -1);
	if (behaviourIndex == -1)
	{
		return false;
	}

	return m_propertyBehaviorDescription[static_cast<size_t>(behaviourIndex)].dependsOnPrecision;
}

bool SignalPropertyManager::isHiddenFor(E::SignalType type, const int propertyIndex, bool isExpert) const
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

void SignalPropertyManager::detectNewProperties(const AppSignal &signal)
{
	PropertyObject propObject;

	std::pair<bool, QString> result = propObject.parseSpecificPropertiesStruct(signal.specPropStruct());

	if (result.first == false)
	{
		assert(false);
		return;
	}

	std::vector<std::shared_ptr<Property>> specificProperties = propObject.properties();

	AppSignalSpecPropValues spValues;

	for(std::shared_ptr<Property> specificProperty : specificProperties)
	{
		int index = m_propertyName2IndexMap.value(specificProperty->caption(), -1);
		if (index != -1)
		{
			continue;
		}

		AppSignalPropertyDescription newProperty;

		QString propertyName = specificProperty->caption();
		bool propertyIsEnum = specificProperty->isEnum();
		QVariant::Type type = specificProperty->value().type();

		newProperty.name = propertyName;
		newProperty.caption = AppSignalProperties::generateCaption(propertyName);
		newProperty.type = type;
		if (propertyIsEnum)
		{
			newProperty.enumValues = specificProperty->enumValues();
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

			assert(qv.type() == type);

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
		case QVariant::String:
		case QVariant::Double:
		case QVariant::Int:
		case QVariant::UInt:
		case QVariant::Bool:
			break;
		default:
			assert(false);
			continue;
		}

		addNewProperty(newProperty);
	}
}


// Loads properties that uninitialized signal contains
//
void SignalPropertyManager::loadNotSpecificProperties()
{
	AppSignal signal;
	AppSignalProperties signalProperties(signal, true);
	std::vector<AppSignalPropertyDescription> propetyDescription = signalProperties.getProperties();

	for (AppSignalPropertyDescription& property : propetyDescription)
	{
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

void SignalPropertyManager::reloadPropertyBehaviour()
{
	if (m_dbController == nullptr)
	{
		return;
	}

	int etcFileId = m_dbController->systemFileId(DbDir::EtcDir);

	DbFileInfo propertyBehaviorFile;
	m_dbController->getFileInfo(etcFileId, QString(Db::File::SignalPropertyBehaviorFileName), &propertyBehaviorFile, m_parentWidget);

	if (propertyBehaviorFile.isNull() == true)
	{
		QMessageBox::critical(m_parentWidget, "Error", QString("File \"%1\" is not found!").arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	std::shared_ptr<DbFile> file;
	bool result = m_dbController->getLatestVersion(propertyBehaviorFile, &file, m_parentWidget);
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

	int nameIndex = fieldNameList.indexOf("PropertyName");
	if (nameIndex < 0)
	{
		QMessageBox::critical(m_parentWidget, "Error", uncorrectFileMessage + ": PropertyName column not found");
		return;
	}

	int precisionIndex = fieldNameList.indexOf("DependsOnPrecision");
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
			typeIndexes[static_cast<size_t>(i * SIGNAL_TYPE_COUNT + j)] = fieldNameList.indexOf(typeName(i, j));
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

		int propertyIndex = m_propertyName2IndexMap.value(behaviour.name, -1);
		if (propertyIndex != -1)
		{
			int behaviourIndex = static_cast<int>(m_propertyBehaviorDescription.size()) - 1;
			assert(m_propertyDescription[propertyIndex].name == m_propertyBehaviorDescription[behaviourIndex].name);

			m_propertyIndex2BehaviourIndexMap[propertyIndex] = behaviourIndex;
		}
	}
}

void SignalPropertyManager::clear()
{
	if (m_propertyDescription.size() > m_basicPropertyDescription.size())
	{
		emit propertyCountWillDecrease(static_cast<int>(m_basicPropertyDescription.size()));
		m_propertyDescription = m_basicPropertyDescription;
		emit propertyCountDecreased();
	}
	if (m_propertyDescription.size() < m_basicPropertyDescription.size())
	{
		emit propertyCountWillIncrease(static_cast<int>(m_basicPropertyDescription.size()));
		m_propertyDescription = m_basicPropertyDescription;
		emit propertyCountIncreased();
	}
	m_propertyName2IndexMap.clear();
}

void SignalPropertyManager::init()
{
	clear();
	for (size_t i = 0; i < m_propertyDescription.size(); i++)
	{
		m_propertyName2IndexMap[m_propertyDescription[i].name] = static_cast<int>(i);
	}
	loadNotSpecificProperties();
}

bool SignalPropertyManager::isNotCorrect(int propertyIndex) const
{
	if (propertyIndex < 0 || propertyIndex >= static_cast<int>(m_propertyDescription.size()))
	{
		return true;
	}
	return false;
}

QString SignalPropertyManager::typeName(E::SignalType type, E::SignalInOutType inOutType)
{
	return E::valueToString<E::SignalType>(type) + E::valueToString<E::SignalInOutType>(inOutType);
}

QString SignalPropertyManager::typeName(int typeIndex, int inOutTypeIndex)
{
	return typeName(IntToEnum<E::SignalType>(QMetaEnum::fromType<E::SignalType>().value(typeIndex)),
					IntToEnum<E::SignalInOutType>(QMetaEnum::fromType<E::SignalInOutType>().value(inOutTypeIndex)));
}

TuningValue SignalPropertyManager::variant2TuningValue(const QVariant& variant, TuningValueType type)
{
	TuningValue value;
	value.setType(type);

	bool ok = false;
	value.fromString(variant.toString(), &ok);
	assert(ok == true);

	return value;
}

bool SignalPropertyManager::isHidden(E::PropertyBehaviourType behaviour, bool isExpert) const
{
	bool hidden = behaviour == E::PropertyBehaviourType::Hide;
	hidden |= behaviour == E::PropertyBehaviourType::Expert && isExpert == false;
	return hidden;
}

bool SignalPropertyManager::isReadOnly(E::PropertyBehaviourType behaviour, bool isExpert) const
{
	bool readOnly = behaviour != E::PropertyBehaviourType::Write;
	readOnly |= behaviour == E::PropertyBehaviourType::Expert && isExpert == false;
	return readOnly;
}

void SignalPropertyManager::addNewProperty(const AppSignalPropertyDescription& newProperty)
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
	m_propertyName2IndexMap.insert(newProperty.name, propertyIndex);
	emit propertyCountIncreased();

	for (size_t i = 0; i < m_propertyBehaviorDescription.size(); i++)
	{
		if (newProperty.name == m_propertyBehaviorDescription[i].name)
		{
			m_propertyIndex2BehaviourIndexMap[propertyIndex] = static_cast<int>(i);
			break;
		}
	}
}

void SignalPropertyManager::trimm(QStringList& stringList)
{
	for (QString& string : stringList)
	{
		string = string.trimmed();
	}
}


SignalSetProvider* SignalSetProvider::m_instance = nullptr;

SignalSetProvider::SignalSetProvider(DbController* dbController, QWidget* parentWidget) :
	QObject(parentWidget),
	m_dbController(dbController),
	m_propertyManager(dbController, parentWidget),
	m_parentWidget(parentWidget)
{
	assert(m_instance == nullptr);
	m_instance = this;

	connect(this, &SignalSetProvider::signalPropertiesChanged, &m_propertyManager, &SignalPropertyManager::detectNewProperties);
}

SignalSetProvider::~SignalSetProvider()
{
	if (m_lazyLoadSignalsTimer != nullptr)
	{
		m_lazyLoadSignalsTimer->stop();
		delete m_lazyLoadSignalsTimer;
	}
}

SignalSetProvider* SignalSetProvider::getInstance()
{
	assert(m_instance != nullptr);
	return m_instance;
}

void SignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
}


AppSignal* SignalSetProvider::getSignalByStrID(const QString signalStrID)
{
	if (m_signalSet.ID2IndexMapIsEmpty())
	{
		m_signalSet.buildID2IndexMap();
	}
	return m_signalSet.getSignal(signalStrID);
}

const AppSignal& SignalSetProvider::getLoadedSignal(int index)
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

AppSignalParam SignalSetProvider::getAppSignalParam(int index)
{
	AppSignal signal = getLoadedSignal(index);
	signal.cacheSpecPropValues();

	AppSignalParam param;
	param.load(signal);

	return param;
}

AppSignalParam SignalSetProvider::getAppSignalParam(QString appSignalId)
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

	return getAppSignalParam(m_signalSet.keyIndex(signal->ID()));
}

QVector<int> SignalSetProvider::getSameChannelSignals(int index)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		QVector<int> sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
		foreach (const int id, sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(m_signalSet.keyIndex(id));
		}
	}
	else
	{
		sameChannelSignalRows.append(index);
	}
	return sameChannelSignalRows;
}

void SignalSetProvider::loadUsers()
{
	std::vector<DbUser> list;
	m_dbController->getUserList(&list, m_parentWidget);

	m_usernameMap.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}
}

bool SignalSetProvider::isEditableSignal(const AppSignal& signal) const
{
	if (!signal.checkedOut() || (signal.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}
	return false;
}

bool SignalSetProvider::isCheckinableSignalForMe(const AppSignal& signal) const
{
	if (signal.checkedOut() && (signal.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}
	return false;
}

QString SignalSetProvider::getUserStr(int userId) const
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

bool SignalSetProvider::checkoutSignal(int index)
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
	m_dbController->checkoutSignals(&signalsIDs, &objectStates, m_parentWidget);
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
QString SignalSetProvider::errorMessage(const ObjectState& state)
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
void SignalSetProvider::showError(const ObjectState& state)
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
void SignalSetProvider::showErrors(const QVector<ObjectState>& states)
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


void SignalSetProvider::initLazyLoadSignals()
{
	loadUsers();

	m_propertyManager.init();
	m_propertyManager.reloadPropertyBehaviour();

	QVector<ID_AppSignalID> signalIds;
	dbController()->getSignalsIDAppSignalID(&signalIds, m_parentWidget);

	for (const ID_AppSignalID& id : signalIds)
	{
		m_signalSet.replaceOrAppendIfNotExists(id.ID, AppSignal(id));
	}

	emit signalCountChanged();
	m_partialLoading = true;

	if (m_lazyLoadSignalsTimer == nullptr)
	{
		m_lazyLoadSignalsTimer = new QTimer(this);
		connect(m_lazyLoadSignalsTimer, &QTimer::timeout, this, &SignalSetProvider::loadNextSignalsPortion);
	}

	m_lazyLoadSignalsTimer->start(100);
}

void SignalSetProvider::stopLoadingSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();
		m_partialLoading = false;
	}
}

void SignalSetProvider::finishLoadingSignals()
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

			dbController()->getLatestSignals(signalIds, &signalsToLoad, m_parentWidget);

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

void SignalSetProvider::loadNextSignalsPortion()
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

		dbController()->getLatestSignalsWithoutProgress(signalIds, &signalsToLoad, m_parentWidget);

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

bool SignalSetProvider::checkoutSignal(int index, QString& message)
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
	dbController()->checkoutSignals(&signalsIDs, &objectStates, m_parentWidget);
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


bool SignalSetProvider::undoSignal(int id)
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
		dbController()->undoSignalChanges(signalId, &state, m_parentWidget);
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

void SignalSetProvider::deleteSignal(int signalID)
{
	ObjectState state;
	dbController()->deleteSignal(signalID, &state, m_parentWidget);
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
}

void SignalSetProvider::addSignal(AppSignal& signal)
{
	m_signalSet.replaceOrAppendIfNotExists(signal.ID(), signal);
}

void SignalSetProvider::deleteSignals(const QSet<int>& signalIDs)
{
	for (const int signalID : signalIDs)
	{
		deleteSignal(signalID);
	}
	loadSignals();
}

void SignalSetProvider::loadSignalSet(QVector<int> keys)
{
	for (int i = 0; i < keys.count(); i++)
	{
		loadSignal(keys[i]);
	}
}

void SignalSetProvider::loadSignal(int signalId)
{
	int index = keyIndex(signalId);
	if (index == -1)
	{
		return;
	}
	dbController()->getLatestSignal(signalId, &m_signalSet[index], m_parentWidget);
	m_signalSet.updateID2IndexInMap(m_signalSet[index].appSignalID(), index);

	emit signalUpdated(index);
	emit signalPropertiesChanged(getLoadedSignal(index));
}

void SignalSetProvider::loadSignals()
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

	if (!dbController()->getSignals(&signalSetForReplacement, false, m_parentWidget))
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

void SignalSetProvider::saveSignal(AppSignal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);

	dbController()->setSignalWorkcopy(&signal, &state, m_parentWidget);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	loadSignal(signal.ID());
}

void SignalSetProvider::saveSignals(QVector<AppSignal*> signalVector)
{
	QVector<ObjectState> states;
	for (int i = 0; i < signalVector.count(); i++)
	{
		ObjectState state;
		trimSignalTextFields(*signalVector[i]);

		dbController()->setSignalWorkcopy(signalVector[i], &state, m_parentWidget);
		states.append(state);

		loadSignal(signalVector[i]->ID());
	}
	showErrors(states);
}

QVector<int> SignalSetProvider::cloneSignals(const QSet<int>& signalIDs)
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

		dbController()->addSignal(type, &groupSignals, m_parentWidget);

		int prevSize = resultSignalIDs.size();
		resultSignalIDs.resize(prevSize + groupSignals.count());

		for (int i = 0; i < groupSignals.count(); i++)
		{
			resultSignalIDs[prevSize + i] = groupSignals[i].ID();
		}
	}
	loadSignals();
	return resultSignalIDs;
}

void SignalSetProvider::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		m_propertyManager.clear();
		m_signalSet.clear();
		emit signalCountChanged();
	}
}

void SignalSetProvider::trimSignalTextFields(AppSignal& signal)
{
	signal.setAppSignalID(signal.appSignalID().trimmed());
	signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
	signal.setEquipmentID(signal.equipmentID().trimmed());
	signal.setBusTypeID(signal.busTypeID().trimmed());
	signal.setCaption(signal.caption().trimmed());
	signal.setUnit(signal.unit().trimmed());
}
