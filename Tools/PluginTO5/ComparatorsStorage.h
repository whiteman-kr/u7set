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
	Count = 2
};

static inline QString FilterTypeStr[static_cast<int>(FilterType::Count)] = {"Schema ID", "Case ID"};

struct ComparatorData
{
	Hash hash = UNDEFINED_HASH;

	QString schemaID;

	QString inputAppSignalID;
	QString inputCustomAppSignalID;
	QString inputSignalCaption;

	QString caseID;

	QString type;

	double defaultValue = 0;	// default setpoint value from the project
	double currentValue = 0;	// current setpoint value

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

	// Update comparator
	//
	void setComparatorCriteria(Hash hash, double criteria);

private:
	bool loadComparators(const QString& comparatorsFile, ComparatorSet& comparatorSet) const;
	bool loadAppSignals(const QString& appSignalsFile, AppSignalSet& appSignalSet) const;

private:
	QStringList m_csvHeaders;

	std::vector<Hash> m_comparatorData;
	std::unordered_map<Hash, ComparatorData> m_comparatorDataMap;
};