#include "MainWindow.h"
#include "TuningSourcesHelper.h"

TuningCountersManager::TuningCountersManager(const TuningLib::TuningUiStorage& tuningUi,
											 const ClientLib::TuningSignalManager& tunigSignals,
											 const ClientLib::TuningConnection& tuningConnection,
											 const AppSignalLists::AppSignalListSet& appSignalListSet) :
	m_tuningUi(tuningUi),
	m_tunigSignals(tunigSignals),
	m_tuningConnection(tuningConnection),
	m_appSignalListSet(appSignalListSet)
{
}

void TuningCountersManager::update(TuningClientSettings::LmStatusFlagMode lmStatusFlagMode)
{
	const std::vector<ClientLib::TuningSource> sourcesInfo = m_tuningConnection.tuningSourcesInfo();

	// Total counters
	//
	m_totalCounters.errorCounter = ClientLib::TuningSourcesHelper::sourcesErrorsCount(sourcesInfo);

	if (lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::SOR)
	{
		m_totalCounters.sorCounter =
			ClientLib::TuningSourcesHelper::sourcesSorCount(sourcesInfo, &m_totalCounters.sorActive, &m_totalCounters.sorValid);
	}

	// All UI counters
	//
	updateCounters(sourcesInfo, lmStatusFlagMode);
}

TuningCounters TuningCountersManager::totalCounters() const 
{
	return m_totalCounters;
}

TuningCounters TuningCountersManager::counters(const QString& filters) 
{
	auto it = m_countersBase.find(filters);
	if (it == m_countersBase.end()) 
	{
		// Create a new counter
		//
		TuningCountersData data;

		// Create unique set of filters
		//
		QStringList filtersList = filters.split(';', Qt::SkipEmptyParts);
		std::set<QString> filtersSet;
		for (const QString& s: filtersList) 
		{
			filtersSet.insert(s);
		}

		// Determine if counter for EquipmentId is added
		//
		const std::vector<ClientLib::TuningSource> sourcesInfo = m_tuningConnection.tuningSourcesInfo();
		for (const auto& si: sourcesInfo) 
		{
			if (si.equipmentId() == filters) 
			{
				data.isTuningSource = true;
				break;
			}
		}

		// Make filters hashes intersection for this counter
		//
		bool first = true;
		for (const QString& filterId : filtersSet)
		{
			AppSignalLists::AppSignalList* pageList = m_appSignalListSet.get(filterId).get();
			if (pageList == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			if (first == true)
			{
				first = false;
				data.tuningSignalHashes = pageList->tuningListHashesCache();
			}
			else
			{
				std::vector<Hash> v_intersection;
				std::set_intersection(data.tuningSignalHashes.begin(),
									  data.tuningSignalHashes.end(),
									  pageList->tuningListHashesCache().begin(),
									  pageList->tuningListHashesCache().end(),
									  std::back_inserter(v_intersection));
				data.tuningSignalHashes.clear();
				data.tuningSignalHashes.insert(v_intersection.begin(), v_intersection.end());
			}
		}

		m_countersBase[filters] = data;
		return data.counters;
	}
	else
	{
		return it->second.counters;
	}
}

void TuningCountersManager::slot_signalListsChanged() 
{
	m_countersBase.clear();
}

void TuningCountersManager::updateCounters(const std::vector<ClientLib::TuningSource>& sourceStates,
										   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode)
{

	for (auto& [filters, data] : m_countersBase)
	{
		data.counters = {};

		// Error and SOR (equipment) counters
		//
		if (data.isTuningSource == true)
		{
			Hash equipmentHash = ::calcHash(filters); // AppSignalList ID for Equipment list is EquipmentId!

			data.counters.errorCounter += ClientLib::TuningSourcesHelper::sourceErrorsCount(sourceStates, equipmentHash);

			if (lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::SOR)
			{
				bool sorIsActive = false;
				bool sorIsValid = false;

				data.counters.sorCounter +=
					ClientLib::TuningSourcesHelper::sourceSorCount(sourceStates, equipmentHash, &sorIsActive, &sorIsValid);

				if (sorIsActive == true)
				{
					data.counters.sorActive = true;
				}
				if (sorIsValid == true)
				{
					data.counters.sorValid = true;
				}
			}
		}

		// Discrete counters
		//
		bool found = false;

		for (const Hash& hash : data.tuningSignalHashes)
		{
			TuningSignalState state = m_tunigSignals.queuedState(hash, &found);
			if (found == false)
			{
				continue;
			}

			if (state.valid() == true && state.value().type() == TuningValueType::Discrete && state.value().discreteValue() != 0)
			{
				data.counters.discreteCounter++;
			}
		}
	}
}
