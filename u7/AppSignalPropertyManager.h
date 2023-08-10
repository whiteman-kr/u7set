#pragma once

#include "../Builder/AppSignalProperties.h"

class DbController;

class AppSignalPropertyManager : public QObject
{
	Q_OBJECT

public:
	AppSignalPropertyManager(DbController* dbController, QWidget* parentWidget);

	static AppSignalPropertyManager* getInstance();

	// Data for models
	//

	int count() const;

	int index(const QString& name);
	QString caption(int propertyIndex) const;
	QString name(int propertyIndex);

	bool getSignalEnumPropertyValues(const AppSignal& s, int propertyIndex,
									 std::vector<std::pair<int, QString>>* enumValues) const;
	bool isEnumProperty(int propertyIndex) const;

	QVariant value(const AppSignal* signal, int propertyIndex, bool isExpert) const;
	void setValue(AppSignal* signal, int propertyIndex, const QVariant& value, bool isExpert);

	const AppSignalPropertyDescription& getPropertyDescription(int propertyIndex) const;
	QMetaType::Type type(const int propertyIndex) const;

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

signals:
	void propertyCountWillIncrease(int newPropertyCount);
	void propertyCountWillDecrease(int newPropertyCount);
	void propertyCountIncreased();
	void propertyCountDecreased();

public slots:
	void detectSignalsNewProperties(const std::vector<const AppSignal *>& signalsArray);
	void detectNewProperties(const AppSignal* signal);

private:
	static inline const int SIGNAL_TYPE_COUNT = QMetaEnum::fromType<E::SignalType>().keyCount();
	static inline const int IN_OUT_TYPE_COUNT = QMetaEnum::fromType<E::SignalInOutType>().keyCount();
	static inline const int TOTAL_SIGNAL_TYPE_COUNT = SIGNAL_TYPE_COUNT * IN_OUT_TYPE_COUNT;

	struct PropertyBehaviourDescription
	{
		QString name;
		bool dependsOnPrecision = false;
		std::vector<E::PropertyBehaviourType> behaviourType = std::vector<E::PropertyBehaviourType>(
																	TOTAL_SIGNAL_TYPE_COUNT,
																	E::PropertyBehaviourType::Write);
	};

	static const E::PropertyBehaviourType m_defaultBehaviour = E::PropertyBehaviourType::Write;
	static const std::map<int, QString> m_emptyEnumValuesMap;
	static const AppSignalPropertyDescription m_notValidPropDescription;

private:
	void updatePropertyName2IndexMap();

	int propertyIndex(const QString& propName) const;
	int behaviourIndex(int propertyIndex) const;

	bool isNotCorrect(int propertyIndex) const;
	QString typeName(E::SignalType type, E::SignalInOutType inOutType);
	QString typeName(int typeIndex, int inOutTypeIndex);

	static TuningValue variant2TuningValue(const QVariant& variant, TuningValueType type);

	void addNewProperty(const AppSignalPropertyDescription& newProperty);

	static void trimm(QStringList& stringList);

private:
	DbController* m_dbController = nullptr;
	QWidget* m_parentWidget = nullptr;
	static AppSignalPropertyManager* m_instance;

	std::set<Hash> m_parsedSpecPropStruct;

	static const std::vector<AppSignalPropertyDescription> m_replacedPropertyDescription;

	std::vector<AppSignalPropertyDescription> m_propertyDescription;
	std::map<QString, int> m_propertyName2IndexMap;		// propName => index in m_propertyDescription

	std::vector<PropertyBehaviourDescription> m_propertyBehaviorDescription;
	std::map<int, int> m_propertyIndex2BehaviourIndexMap;
};
