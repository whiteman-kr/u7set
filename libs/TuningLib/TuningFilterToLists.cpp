#include "./include/TuningLib/TuningFilterToLists.h"
#include "./include/TuningLib/TuningFilter.h"
#include "./include/TuningLib/TuningUiItem.h"
#include <AppSignalLists/SignalList.h>

namespace TuningFilters
{
	bool TuningFilterToLists::convertUi(const QString& softwareEquipmentId,
										const QString& softwareTag,
										TuningFilters::TuningFilterStorage& tuningFilterStorage,
										TuningLib::TuningUiStorage& uiStorage,
										AppSignalLists::AppSignalListSet& appSignalLists)
	{
		TuningFilter* rootFilter = tuningFilterStorage.root().get();
		TuningLib::TuningUiItem* rootUi = uiStorage.root();

		int filterNumber = 0;

		convertUiFilter(softwareEquipmentId, softwareTag, filterNumber, rootFilter, rootUi, appSignalLists);

		return true;
	}

	bool TuningFilterToLists::convertGeneric(TuningFilters::TuningFilterStorage& tuningFilterStorage,
											 AppSignalLists::AppSignalListSet& appSignalLists)
	{
		TuningFilter* rootFilter = tuningFilterStorage.root().get();

		int filterNumber = 0;

		convertGenericFilter(filterNumber, rootFilter, appSignalLists);

		return true;
	}

	bool TuningFilterToLists::convertUiFilter(const QString& softwareEquipmentId,
											  const QString& softwareTag,
											  int& filterNumber,
											  TuningFilters::TuningFilter* parentFilter,
											  TuningLib::TuningUiItem* parentUi,
											  AppSignalLists::AppSignalListSet& appSignalLists)
	{
		for (int i = 0; i < parentFilter->childFiltersCount(); i++)
		{
			TuningFilter* filter = parentFilter->childFilter(i).get();
			if (filter == nullptr)
			{
				Q_ASSERT(filter);
				return false;
			}

			if (filter->isTree() == true) // They are in lists
			{
				continue;
			}

			QString id;
			do
			{
				id = QString("%1_UI_%2").arg(softwareEquipmentId).arg(QString::number(filterNumber++).rightJustified(4, '0'));
			} while (appSignalLists.get(id) != nullptr);

			filter->setID(id);

			// Create Ui Item
			//
			std::shared_ptr<TuningLib::TuningUiItem> uiItem = std::make_shared<TuningLib::TuningUiItem>();
			uiItem->setUuid(QUuid::createUuid());
			uiItem->setCaption(filter->caption());
			switch (filter->interfaceType())
			{
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
				uiItem->setInterfaceType(TuningLib::TuningUiItem::InterfaceType::Tab);
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

			switch (filter->tabType())
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
			uiItem->setStartSchemaId(filter->startSchemaId());

			parentUi->addChild(uiItem);

			// Create AppSignalLists
			//
			if (filter->isEmpty() == false)
			{
				std::shared_ptr<AppSignalLists::AppSignalList> appSignalList = std::make_shared<AppSignalLists::AppSignalList>();
				appSignalLists.add(appSignalList);

				// Copy list properties
				//
				appSignalList->setId(
					filter->ID()); // Signal list ID is the same as Filter ID! This is requred to keep links in UI item's Filters property!
				appSignalList->setCaption(filter->caption());
				switch (filter->signalType())
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

				if (appSignalList->userTagsList().contains(softwareTag) == false)
				{
					QStringList tags = appSignalList->userTagsList();
					tags.push_back(softwareTag);
					appSignalList->setUserTags(tags.join(' '));
				}

				if (appSignalList->systemTagsList().contains(AppSignalLists::AppSignalList::tagUi) == false)
				{
					QStringList tags = appSignalList->systemTagsList();
					tags.push_back(AppSignalLists::AppSignalList::tagUi);
					appSignalList->setSystemTags(tags.join(' '));
				}
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
						return; // End of recursion
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
			if (convertUiFilter(softwareEquipmentId, softwareTag, filterNumber, filter, uiItem.get(), appSignalLists) == false)
			{
				Q_ASSERT(false);
				return false;
			}
		}

		return true;
	}

	bool TuningFilterToLists::convertGenericFilter(int& filterNumber,
												   TuningFilters::TuningFilter* parentFilter,
												   AppSignalLists::AppSignalListSet& appSignalLists)
	{
		for (int i = 0; i < parentFilter->childFiltersCount(); i++)
		{
			TuningFilter* filter = parentFilter->childFilter(i).get();
			if (filter == nullptr)
			{
				Q_ASSERT(filter);
				return false;
			}

			if (filter->isTree() == false)
			{
				continue;
			}


			// Create AppSignalLists
			//
			std::shared_ptr<AppSignalLists::AppSignalList> appSignalList = std::make_shared<AppSignalLists::AppSignalList>();
			appSignalLists.add(appSignalList);

			// Copy list properties
			//
			QString id;
			do
			{
				id = QString("LOCAL_%1").arg(QString::number(filterNumber++).rightJustified(4, '0'));
			} while (appSignalLists.get(id) != nullptr);
			appSignalList->setId(id);

			appSignalList->setCaption(filter->caption());

			switch (filter->signalType())
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

			appSignalList->setCustomAppSignalIDMask(filter->customAppSignalIDMask());
			appSignalList->setEquipmentIDMask(filter->equipmentIDMask());
			appSignalList->setAppSignalIDMask(filter->appSignalIDMask());
			appSignalList->setAppSignalTags(filter->appSignalTags());

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

			// Recursive call for all children
			//
			if (convertGenericFilter(filterNumber, filter, appSignalLists) == false)
			{
				Q_ASSERT(false);
				return false;
			}
		}

		return true;
	}
} // namespace TuningFilters
