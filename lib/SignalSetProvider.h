#pragma once
#include "../Builder/SignalSet.h"
#include "SignalProperties.h"
#include "AppSignalParam.h"
#include "../DbLib/DbStruct.h"

#define SIGNAL_TYPE_COUNT (QMetaEnum::fromType<E::SignalType>().keyCount())
#define IN_OUT_TYPE_COUNT (QMetaEnum::fromType<E::SignalInOutType>().keyCount())
#define TOTAL_SIGNAL_TYPE_COUNT (SIGNAL_TYPE_COUNT * IN_OUT_TYPE_COUNT)

class DbController;

class SignalPropertyManager : public QObject
{
	Q_OBJECT
public:
	struct PropertyBehaviourDescription
	{
		QString name;
		bool dependsOnPrecision = false;
		std::vector<E::PropertyBehaviourType> behaviourType = std::vector<E::PropertyBehaviourType>(
					static_cast<size_t>(TOTAL_SIGNAL_TYPE_COUNT),
					E::PropertyBehaviourType::Write);
	};

	static const E::PropertyBehaviourType defaultBehaviour = E::PropertyBehaviourType::Write;

signals:
	void propertyCountWillIncrease(int newPropertyCount);
	void propertyCountWillDecrease(int newPropertyCount);
	void propertyCountIncreased();
	void propertyCountDecreased();

public:
	SignalPropertyManager(DbController* dbController, QWidget* parentWidget);

	static SignalPropertyManager* getInstance();

	// Data for models
	//

	int count() const;

	int index(const QString& name);
	QString caption(int propertyIndex) const;
	QString name(int propertyIndex);

	QVariant value(const AppSignal* signal, int propertyIndex, bool isExpert) const;
	const std::vector<std::pair<int, QString>> values(int propertyIndex) const;

	void setValue(AppSignal* signal, int propertyIndex, const QVariant& value, bool isExpert);

	QVariant::Type type(const int propertyIndex) const;
	E::PropertyBehaviourType getBehaviour(const AppSignal& signal, const int propertyIndex) const;
	E::PropertyBehaviourType getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const;
	bool dependsOnPrecision(const int propertyIndex) const;
	bool isHiddenFor(E::SignalType type, const int propertyIndex, bool isExpert) const;
	bool isHidden(E::PropertyBehaviourType behaviour, bool isExpert) const;
	bool isReadOnly(E::PropertyBehaviourType behaviour, bool isExpert) const;

	void loadNotSpecificProperties();
	void reloadPropertyBehaviour();
	void clear();
	void init();

public slots:
	void detectNewProperties(const AppSignal& signal);

private:
	bool isNotCorrect(int propertyIndex) const;
	QString typeName(E::SignalType type, E::SignalInOutType inOutType);
	QString typeName(int typeIndex, int inOutTypeIndex);

	static TuningValue variant2TuningValue(const QVariant& variant, TuningValueType type);

	void addNewProperty(const SignalPropertyDescription& newProperty);

	static void trimm(QStringList& stringList);

	std::vector<PropertyBehaviourDescription> m_propertyBehaviorDescription;
	QHash<QString, int> m_propertyName2IndexMap;
	QHash<int, int> m_propertyIndex2BehaviourIndexMap;

	DbController* m_dbController;
	QWidget* m_parentWidget;
	static SignalPropertyManager* m_instance;

	// is initialized by non specific properties
	//
	std::vector<SignalPropertyDescription> m_basicPropertyDescription = {
		{ AppSignalPropNames::APP_SIGNAL_ID,
		  AppSignalPropNames::APP_SIGNAL_ID,
		  QVariant::String, {},
		  [](const AppSignal* s){ return s->appSignalID(); },
		  [](AppSignal* s, QVariant v){ s->setAppSignalID(v.toString()); }, },

		{ AppSignalPropNames::CUSTOM_APP_SIGNAL_ID,
		  AppSignalPropNames::CUSTOM_APP_SIGNAL_ID,
		  QVariant::String, {},
		  [](const AppSignal* s){ return s->customAppSignalID(); },
		  [](AppSignal* s, QVariant v){ s->setCustomAppSignalID(v.toString()); }, },

		{ AppSignalPropNames::EQUIPMENT_ID,
		  AppSignalPropNames::EQUIPMENT_ID,
		  QVariant::String, {},
		  [](const AppSignal* s){ return s->equipmentID(); },
		  [](AppSignal* s, QVariant v){ s->setEquipmentID(v.toString()); }, },

		{ AppSignalPropNames::BUS_TYPE_ID,
		  AppSignalPropNames::BUS_TYPE_ID,
		  QVariant::String, {},
		  [](const AppSignal* s){ return s->busTypeID(); },
		  [](AppSignal* s, QVariant v){ s->setBusTypeID(v.toString()); }, },

		{ AppSignalPropNames::TYPE,
		  "A/D/B",
		  QVariant::String, {},
		  [](const AppSignal* s){ return E::valueToString<E::SignalType>(s->signalType()).left(1); },
		  nullptr },

		{ AppSignalPropNames::IN_OUT_TYPE,
		  "Input-output type",
		  QVariant::Int, E::enumValues<E::SignalInOutType>(),
		  [](const AppSignal* s){ return TO_INT(s->inOutType()); },
		  [](AppSignal* s, QVariant v){ s->setInOutType(IntToEnum<E::SignalInOutType>(v.toInt())); }, },
	};
	std::vector<SignalPropertyDescription> m_propertyDescription;
};

class SignalSetProvider : public QObject
{
	Q_OBJECT

public:
	SignalSetProvider(DbController* dbController, QWidget* parentWidget);
	virtual ~SignalSetProvider();

	static SignalSetProvider* getInstance();
	SignalPropertyManager& signalPropertyManager() { return m_propertyManager; }

	void setMiddleVisibleSignalIndex(int signalIndex);

	void clearSignals();

	const AppSignalSet& signalSet() const	{ return m_signalSet; }
	static void trimSignalTextFields(AppSignal& signal);

	int signalCount() { return m_signalSet.count(); }
	AppSignal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	AppSignal* getSignalByStrID(const QString signalStrID);
	QVector<int> getChannelSignalsID(int signalGroupID) { return m_signalSet.getChannelSignalsID(signalGroupID); }
	int key(int index) const { return m_signalSet.key(index); }
	int keyIndex(int key) { return m_signalSet.keyIndex(key); }
	QVector<int> getSameChannelSignals(int index);

	const AppSignal& getLoadedSignal(int index);

	AppSignalParam getAppSignalParam(int index);
	AppSignalParam getAppSignalParam(QString appSignalId);

	bool isEditableSignal(int index) const { return isEditableSignal(m_signalSet[index]); }
	bool isEditableSignal(const AppSignal& signal) const;
	bool isCheckinableSignalForMe(int index) const{ return isCheckinableSignalForMe(m_signalSet[index]); }
	bool isCheckinableSignalForMe(const AppSignal& signal) const;

	QString getUserStr(int userId) const;

	DbController* dbController() { return m_dbController; }
	const DbController* dbController() const { return m_dbController; }

	bool checkoutSignal(int index);
	bool checkoutSignal(int index, QString& message);
	bool undoSignal(int id);

	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const QSet<int>& signalIDs);
	void deleteSignal(int signalID);

	void addSignal(AppSignal& signal);
	void saveSignal(AppSignal& signal);
	void saveSignals(QVector<AppSignal*> signalVector);
	QVector<int> cloneSignals(const QSet<int>& signalIDs);

	void showError(const ObjectState& state);
	void showErrors(const QVector<ObjectState>& states);

signals:
	void error(const QString& message) const;	// for throwing message boxes
	void signalCountChanged() const;	// for reloading entire signal model content
	void signalUpdated(int signalIndex) const;	// for updating row in signal view (throwing models DataChanged signal)
	void signalPropertiesChanged(const AppSignal& signal) const; // for updating property list if new properties exist in signal

public slots:
	void initLazyLoadSignals();
	void finishLoadingSignals();
	void stopLoadingSignals();
	void loadNextSignalsPortion();
	void loadUsers();
	void loadSignals();
	void loadSignalSet(QVector<int> keys);
	void loadSignal(int signalId);

private:
	QString errorMessage(const ObjectState& state);	// Converts ObjectState to human readable text

	static SignalSetProvider* m_instance;

	DbController* m_dbController = nullptr;
	SignalPropertyManager m_propertyManager;
	QTimer* m_lazyLoadSignalsTimer = nullptr;
	int m_middleVisibleSignalIndex = 0;
	QWidget* m_parentWidget = nullptr;	//used by DbController
	AppSignalSet m_signalSet;
	QMap<int, QString> m_usernameMap;
	bool m_partialLoading = false;
};
