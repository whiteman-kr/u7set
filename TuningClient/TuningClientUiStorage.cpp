#include "TuningClientUiStorage.h"
#include "MainWindow.h"
#include "TuningSourcesHelper.h"

TuningClientUiStorage::TuningClientUiStorage(const ClientLib::TuningSignalManager& tunigSignals,
											 const ClientLib::TuningConnection& tuningConnection,
											 const AppSignalLists::AppSignalListSet& appSignalListSet) :
	m_tunigSignals(tunigSignals),
	m_tuningConnection(tuningConnection),
	m_appSignalListSet(appSignalListSet)
{
}

void TuningClientUiStorage::updateCounters(const std::vector<ClientLib::TuningSource>& sourceStates,
										   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
										   TuningLib::TuningUiItem* uiItem)
{
	if (uiItem == nullptr)
	{
		uiItem = root();
	}

	TuningLib::TuningCounters filterCounters;

	if (uiItem->isRoot() == true)
	{
		// Root (total) Error and SOR counters
		//

		filterCounters.errorCounter = ClientLib::TuningSourcesHelper::sourcesErrorsCount(sourceStates);

		if (lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::SOR)
		{
			filterCounters.sorCounter =
				ClientLib::TuningSourcesHelper::sourcesSorCount(sourceStates, &filterCounters.sorActive, &filterCounters.sorValid);
		}
	}
	else
	{
		for (const QString& filterId : uiItem->filtersList())
		{
			auto appSignalList = m_appSignalListSet.get(filterId);
			if (appSignalList == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			// Error and SOR (equipment) counters
			//
			if (appSignalList->systemTags().contains(AppSignalLists::AppSignalList::tagEquipment) == true)
			{
				Hash equipmentHash = ::calcHash(appSignalList->id()); // AppSignalList ID for Equipment list is EquipmentId!
				filterCounters.errorCounter += ClientLib::TuningSourcesHelper::sourceErrorsCount(sourceStates, equipmentHash);

				if (lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::SOR)
				{
					bool sorIsActive = false;
					bool sorIsValid = false;

					filterCounters.sorCounter +=
						ClientLib::TuningSourcesHelper::sourceSorCount(sourceStates, equipmentHash, &sorIsActive, &sorIsValid);

					if (sorIsActive == true)
					{
						filterCounters.sorActive = true;
					}
					if (sorIsValid == true)
					{
						filterCounters.sorValid = true;
					}
				}
			}

			// Discrete counters
			//
			if (uiItem->hasDiscreteCounter() == true || uiItem->isCounter() == true)
			{
				bool found = false;

				const std::set<Hash>& appSignalsHashes = appSignalList->listHashesCache();
				for (const Hash& appSignalHash : appSignalsHashes)
				{
					TuningSignalState state = m_tunigSignals.queuedState(appSignalHash, &found);
					if (found == false)
					{
						continue;
					}

					if (state.valid() == true && state.value().type() == TuningValueType::Discrete && state.value().discreteValue() != 0)
					{
						filterCounters.discreteCounter++;
					}
				}
			}
		}
	}

	uiItem->setCounters(filterCounters);

	// Recursive call for all children\
	//
	for (int i = 0; i < uiItem->childCount(); i++)
	{
		updateCounters(sourceStates, lmStatusFlagMode, uiItem->child(i).get());

		// Add child filters' counters for all empty filters
		//
		if (uiItem->filters().isEmpty() == true)
		{
			TuningLib::TuningCounters childCounters = uiItem->child(i)->counters();

			filterCounters.discreteCounter += childCounters.discreteCounter;
			filterCounters.errorCounter += childCounters.errorCounter;
			filterCounters.sorCounter += childCounters.sorCounter;

			uiItem->setCounters(filterCounters);
		}
	}
}

