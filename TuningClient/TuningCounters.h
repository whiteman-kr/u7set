#pragma once

#include <Settings.h>
#include <TuningLib/TuningUiItem.h>

namespace ClientLib
{
	class TuningSource;
	class TuningConnection;
	class TuningSignalManager;
} // namespace ClientLib

namespace AppSignalLists
{
	class AppSignalListSet;
}

// Structs
//
struct TuningCounters
{
	int errorCounter = 0;
	int sorCounter = 0;
	bool sorActive = false;
	bool sorValid = false;
	int discreteCounter = 0;
};

struct TuningCountersData
{
	bool isTuningSource = false;
	std::set<Hash> tuningSignalHashes;
	TuningCounters counters;
};

class TuningCountersManager: public QObject
{
	Q_OBJECT
public:
	TuningCountersManager(const TuningLib::TuningUiStorage& tuningUi,
						  const ClientLib::TuningSignalManager& tunigSignals,
						  const ClientLib::TuningConnection& tuningConnection,
						  const TuningSignalListSet& appSignalListSet);

	void update(TuningClientSettings::LmStatusFlagMode lmStatusFlagMode);

	TuningCounters totalCounters() const;
	TuningCounters counters(const QString& filters);

public slots:
	void slot_signalListsChanged();

private:
	void updateCounters(const std::vector<ClientLib::TuningSource>& sourceStates,
						TuningClientSettings::LmStatusFlagMode lmStatusFlagMode);

private:
	const TuningLib::TuningUiStorage& m_tuningUi;
	const ClientLib::TuningSignalManager& m_tunigSignals;
	const ClientLib::TuningConnection& m_tuningConnection;
	const TuningSignalListSet& m_appSignalListSet;

	TuningCounters m_totalCounters;

	std::map<QString, TuningCountersData> m_countersBase;	// Key is filters configuration
};
