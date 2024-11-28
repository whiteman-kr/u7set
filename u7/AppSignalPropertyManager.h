#pragma once

#include "../Builder/AppSignalProperties.h"


class AppSignalPropertyManager : public QObject
{
	Q_OBJECT

public:
	AppSignalPropertyManager();

	static AppSignalPropertyManager* getInstance();

	int count() const;

	QString name(int propertyIndex);

	int propertyIndex(const QString& name);

	bool getSignalEnumPropertyValues(const AppSignal& s, int propertyIndex,
									 std::vector<std::pair<int, QString>>* enumValues) const;
	bool isEnumProperty(int propertyIndex) const;

	QVariant value(const AppSignal* signal, int propertyIndex, bool isExpert) const;
	bool setValue(AppSignal* signal, int propertyIndex, const QVariant& newValue, bool isExpert);	// returns true if value changed

	const AppSignalPropertyDescription& getPropertyDescription(int propertyIndex) const;
	QMetaType::Type type(const int propertyIndex) const;

	E::PropertyBehaviourType getBehaviour(const AppSignal& signal, const int propertyIndex) const;

	bool dependsOnPrecision(const QString& propName) const;
	bool dependsOnPrecision(int propIndex) const;
	bool isHiddenFor(E::SignalType type, const int propertyIndex, bool isExpert) const;
	bool isHidden(E::PropertyBehaviourType behaviour, bool isExpert) const;
	bool isReadOnly(E::PropertyBehaviourType behaviour, bool isExpert) const;

	void updatePropertiesBehaviour(const QString& propBehavoiurFile, QString* errMsg);
	void clear();

signals:
	void propertyCountWillIncrease(int newPropertyCount);
	void propertyCountWillDecrease(int newPropertyCount);
	void propertyCountIncreased();
	void propertyCountDecreased();

public slots:
	void slot_detectNewProperties(const std::vector<int>& signalIndexes);
	void detectNewProperties(const AppSignal& signal);

private:
	static const E::PropertyBehaviourType m_defaultBehaviour = E::PropertyBehaviourType::Write;
	static const std::map<int, QString> m_emptyEnumValuesMap;
	static const AppSignalPropertyDescription m_notValidPropDescription;

private:
	void initNotSpecificPropDescriptions();

	void updatePropNameToIndexMap();

	void updatePropDescriptionsBehaviour();

	int propertyIndex(const QString& propName) const;

	bool isValidPropIndex(int propertyIndex) const;
	QString typeName(E::SignalType type, E::SignalInOutType inOutType);
	QString typeName(int typeIndex, int inOutTypeIndex);

	static TuningValue variant2TuningValue(const QVariant& variant, TuningValueType type);

	void addNewProperty(const AppSignalPropertyDescription& newProperty, bool emitSignals);

	static void trimm(QStringList& stringList);

private:
	static AppSignalPropertyManager* m_instance;

	static const std::vector<AppSignalPropertyDescription> m_replacedPropDescriptions;
	static std::vector<AppSignalPropertyDescription> m_notSpecificPropDescriptions;	// with m_replacedPropDescriptions

	static const std::vector<E::SignalType> m_signalTypes;
	static const std::vector<E::SignalInOutType> m_inOutTypes;

	std::vector<AppSignalPropertyDescription> m_propDescriptions;
	std::map<QString, int> m_propNameToIndex;			// propertyName => index in m_propDescriptions

	// loaded from file SignalPropertyBehavior.csv
	// propertyName => AppSignalPropertyBehavior
	//
	std::map<QString, AppSignalPropertyBehavior> m_propertiesBehaviour;

	//

	std::map<Hash, PropertyObject> m_parsedSpecPropStruct;
};
