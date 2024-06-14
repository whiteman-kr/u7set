#include "./include/TuningLib/TuningFilterToLists.h"
#include "./include/TuningLib/TuningFilter.h"
#include "./include/TuningLib/TuningUiItem.h"
#include <AppSignalLists/SignalList.h>

namespace TuningFilters
{
	bool TuningFilterToLists::convert(TuningFilters::TuningFilterStorage& tuningFilterStorage,
									  TuningLib::TuningUiStorage& uiStorage,
									  AppSignalLists::AppSignalListSet& appSignalLists,
									  const QStringList& appSignalListsSystemTags)
	{

		TuningFilter* rootFilter = tuningFilterStorage.root().get();
		TuningLib::TuningUiItem* rootUi = uiStorage.root();

		convertFilter(rootFilter, rootUi, appSignalLists, appSignalListsSystemTags);

		return true;
	}

	bool TuningFilterToLists::convertFilter(TuningFilter* parentFilter,
											TuningLib::TuningUiItem* parentUi,
											AppSignalLists::AppSignalListSet& appSignalLists,
											const QStringList& appSignalListsSystemTags)
	{
		for (int i = 0; i < parentFilter->childFiltersCount(); i++) 
		{
			TuningFilter* filter = parentFilter->childFilter(i).get();
			if (filter == nullptr) 
			{
				Q_ASSERT(filter);
				return false;
			}

			// Create Ui Item
			//
			std::shared_ptr<TuningLib::TuningUiItem> uiItem = std::make_shared<TuningLib::TuningUiItem>();
			uiItem->setUuid(QUuid::createUuid());	

			if (filter->customID().isEmpty() == true) 
			{
				uiItem->setID(uiItem->uuidString());
			}
			else 
			{
				uiItem->setID(filter->customID());
			}

			uiItem->setCaption(filter->caption());
			switch (filter->interfaceType())
			{
			case TuningFilter::InterfaceType::Tree:
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::Generic);
				break;
			case TuningFilter::InterfaceType::Tab:
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::Tab);
				break;
			case TuningFilter::InterfaceType::Button:
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::Button);
				break;
			case TuningFilter::InterfaceType::Counter:
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::Counter);
				break;
			case TuningFilter::InterfaceType::SchemasTab:
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::SchemasTab);
				break;
			default:
				Q_ASSERT(false);
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::Generic);
			}

			uiItem->setUseColors(filter->useColors());
			uiItem->setBackColor(filter->backColor());
			uiItem->setTextColor(filter->textColor());
			uiItem->setBackSelectedColor(filter->backSelectedColor());
			uiItem->setTextSelectedColor(filter->textSelectedColor());
			uiItem->setBackAlertedColor(filter->backAlertedColor());
			uiItem->setTextAlertedColor(filter->textAlertedColor());
			
			uiItem->setHasDiscreteCounter(filter->hasDiscreteCounter());

			switch (filter->counterType())
			{
			case TuningFilter::CounterType::FilterTree:
				uiItem->setCounterType(TuningLib::TuningUiItem::CounterType::FilterTree);
				break;
			case TuningFilter::CounterType::StatusBar:
				uiItem->setCounterType(TuningLib::TuningUiItem::CounterType::StatusBar);
				break;
			default:
				Q_ASSERT(false);
				uiItem->setCounterType(TuningLib::TuningUiItem::CounterType::FilterTree);
			}
			
			uiItem->setTags(filter->tags());

			switch(filter->tabType())
			{
			case TuningFilter::TabType::Generic:
				uiItem->setTabType(TuningLib::TuningUiItem::TabType::Generic);
				break;
			case TuningFilter::TabType::FiltersSwitch:
				uiItem->setTabType(TuningLib::TuningUiItem::TabType::FiltersSwitch);
				break;
			default:
				Q_ASSERT(false);
				uiItem->setTabType(TuningLib::TuningUiItem::TabType::Generic);
			}

			uiItem->setValuesColumnCount(filter->valuesColumnCount());
			uiItem->setValueColumnsAppSignalIdSuffixes(filter->valueColumnsAppSignalIdSuffixes());

			uiItem->setColumnCustomAppId(filter->columnCustomAppId());
			uiItem->setColumnAppId(filter->columnAppId());
			uiItem->setColumnEquipmentId(filter->columnEquipmentId());
			uiItem->setColumnCaption(filter->columnCaption());
			uiItem->setColumnUnits(filter->columnUnits());
			uiItem->setColumnType(filter->columnType());
			uiItem->setColumnLimits(filter->columnLimits());
			uiItem->setColumnDefault(filter->columnDefault());
			uiItem->setColumnValid(filter->columnValid());
			uiItem->setColumnOutOfRange(filter->columnOutOfRange());

			parentUi->addChild(uiItem);

			// Create AppSignalLists
			//
			if (filter->isEmpty() == false) 
			{
				std::shared_ptr<AppSignalLists::AppSignalList> appSignalList = std::make_shared<AppSignalLists::AppSignalList>();
				appSignalLists.add(appSignalList);

				// Copy list properties
				//
				appSignalList->setId(filter->ID());				// Signal list ID is the same as Filter ID! This is requred to keep links in UI item's Filters property!
				appSignalList->setCaption(filter->caption());
				switch(filter->signalType())
				{
				case TuningFilter::SignalType::All:
					appSignalList->setSignalType(AppSignalLists::AppSignalList::SignalType::All);
					break;
				case TuningFilter::SignalType::Analog:
					appSignalList->setSignalType(AppSignalLists::AppSignalList::SignalType::Analog);
					break;
				case TuningFilter::SignalType::Discrete:
					appSignalList->setSignalType(AppSignalLists::AppSignalList::SignalType::Discrete);
					break;
				default:
					Q_ASSERT(false);
					appSignalList->setSignalType(AppSignalLists::AppSignalList::SignalType::All);
				}
				
				appSignalList->systemTagsList() = appSignalListsSystemTags;
				/*
				if (filter->interfaceType() != TuningFilters::TuningFilter::InterfaceType::Tree) 
				{
					// This is UI list
					appSignalList->systemTagsList().push_back(AppSignalLists::AppSignalList::tagUi);
				}*/

				appSignalList->setUserTags({});
				appSignalList->setCustomAppSignalIDMask(filter->customAppSignalIDMask());
				appSignalList->setEquipmentIDMask(filter->equipmentIDMask());
				appSignalList->setAppSignalIDMask(filter->appSignalIDMask());
				appSignalList->setAppSignalTags(filter->appSignalTags());

				// Create recursive filters list
				//
				QStringList uiItemFilters;
				uiItemFilters.push_back(filter->ID());
				auto filtersListFunc = [&uiItemFilters](TuningFilter* filter, auto&& filtersListFunc)
				{
					TuningFilter* parentFilter = filter->parentFilter();
					if (parentFilter == nullptr || parentFilter->isRoot() == true)
					{
						return;	// End of recursion
					}

					if (parentFilter->isEmpty() == false)
					{
						uiItemFilters.insert(uiItemFilters.begin(), parentFilter->ID());
					}

					filtersListFunc(parentFilter, filtersListFunc);
				};
				filtersListFunc(filter, filtersListFunc);
				uiItem->setFilters(uiItemFilters.join(';'));

				// Add list signals
				//
				auto filterSignals = filter->getFilterSignals();
				for (const auto& fs : filterSignals) 
				{
					AppSignalLists::AppSignalListItem asi(fs.appSignalId());
					if (fs.useValue() == true) 
					{
						asi.setValue(fs.value());
					}
					appSignalList->add(asi);
				}
			}

			// Recursive call for all children
			//
			if (convertFilter(filter, uiItem.get(), appSignalLists, appSignalListsSystemTags) == false) 
			{
				Q_ASSERT(false);
				return false;
			}
		}

		return true;
	}

} // namespace TuningFilters

