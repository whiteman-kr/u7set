#pragma once


#include <QString>

#include "../../AppSignalLib/ComparatorSet.h"
#include "../../AppSignalLib/AppSignal.h"
#include <CommonLib/Hash.h>
#include <CommonLib/Types.h>

#include "OutputLog.h"

enum class FilterType
{
	BySchemaID = 0,
	ByCaseID = 1,
	ByAlgorithm = 2,
	Count = 3
};


struct ComparatorData
{
	Hash hash = UNDEFINED_HASH;

	QString algorithm;

	QString schemaID;

	QString inputAppSignalID;
	QString inputCustomAppSignalID;
	QString inputSignalCaption;

	QString caseID;

	QString type;

	double defaultValue = 0;	// default setpoint value from the project
	double currentValue = 0;	// current setpoint value
	int precision = 0;

	bool isDynamic = false;         // true - dynamic setpoint, false - static setpoint

	QString setpointAppSignalID;
	QString setpointCustomAppSignalID;
	QString setpointAppSignalCaption;

	QString outputAppSignalID;
	QString outputCustomAppSignalID;
	QString outputSignalCaption;

	bool isValueOverriden() const
	{
		double delta = 1e-4;
		return std::abs(currentValue - defaultValue) > delta;
	}
};

struct ImportResult
{
	int newComparatorCount = 0;
	int removedComparatorCount = 0;
	int overriddenValuesComparatorCount = 0;
};

class ComparatorsStorage
{
public:
	ComparatorsStorage();
	~ComparatorsStorage();

	void clear();

	// Serialization
	//
	bool loadFromFile(const QString& fileName, QString* errorMsg);
	bool saveToFile(const QString& fileName);
	bool saveToFile(const QString& fileName, const std::vector<Hash>& comparators);

	// Import
	//
	std::pair <bool, ImportResult> importComparatorsSet(const QString& comparatorsFile, const QString& appSignalsFile, QString* errorMsg);

	// Data access
	//
	std::vector<Hash> comparatorsHashes() const;
	std::vector<Hash> filteredComparatorsHashes(QList<QString> id, FilterType sortType) const;

	const ComparatorData& comparator(Hash hash, bool* ok) const;

	QList<QString> schemaIds() const;
	QList<QString> caseIds() const;
	QList<QString> algorithms() const;

	// Update comparator
	//
	void setComparatorCriteria(Hash hash, double criteria);
	void setComparatorAlgorithm(Hash hash, const QString& algorithm);
	void swapComparatorPlace(int index1, int index2);

	QString FilterTypeStr[static_cast<int>(FilterType::Count)] = {QObject::tr("Schema ID"), QObject::tr("Case ID"), QObject::tr("Algorithm")};

private:
	bool loadComparators(const QString& comparatorsFile, ComparatorSet& comparatorSet) const;
	bool loadAppSignals(const QString& appSignalsFile, AppSignalSet& appSignalSet) const;

private:
	QStringList m_csvHeaders;

	std::vector<Hash> m_comparatorData;
	std::unordered_map<Hash, ComparatorData> m_comparatorDataMap;
};