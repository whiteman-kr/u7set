#include "Parser.h"

#include "IssueLogger.h"

#include "../../include/DbController.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicSchema.h"
#include "../../VFrame30/SchemeItemLink.h"
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/SchemeItemAfb.h"
#include "../../VFrame30/SchemeItemSignal.h"
#include "../../VFrame30/SchemeItemConst.h"
#include "../../VFrame30/HorzVertLinks.h"


namespace Builder
{

	Link::Link(const std::list<VFrame30::SchemaPoint>& points) :
		m_points(points)
	{
		assert(points.size() >= 2);
	}

	VFrame30::SchemaPoint Link::ptBegin() const
	{
		if (m_points.empty() == true)
		{
			assert(m_points.empty() == false);
			return VFrame30::SchemaPoint();
		}

		return m_points.front();
	}

	VFrame30::SchemaPoint Link::ptEnd() const
	{
		if (m_points.empty() == true)
		{
			assert(m_points.empty() == false);
			return VFrame30::SchemaPoint();
		}

		return m_points.back();
	}

	bool Link::isPinOnLink(VFrame30::SchemaPoint pt) const
	{
		VFrame30::CHorzVertLinks hvl;

		QUuid fakeId = QUuid::createUuid();
		hvl.AddLinks(m_points, fakeId);

		bool result = hvl.IsPinOnLink(pt, QUuid::createUuid());	// Must be othe Quuid, as if it the same return value always false

		return result;
	}

	VFrame30::FblItemRect* Bush::itemByPinGuid(QUuid pinId)
	{
		for (std::shared_ptr<VFrame30::FblItemRect> item : fblItems)
		{
			VFrame30::AfbPin pin;
			bool found = item->GetConnectionPoint(pinId, &pin);

			if (found == true)
			{
				return item.get();
			}
		}

		return nullptr;
	}

	VFrame30::FblItemRect* Bush::itemByGuid(QUuid uuid) const
	{
		for (std::shared_ptr<VFrame30::FblItemRect> item : fblItems)
		{
			if (item->guid() == uuid)
			{
				return item.get();
			};
		}

		return nullptr;
	}

	VFrame30::AfbPin Bush::pinByGuid(QUuid pinId)
	{
		for (std::shared_ptr<VFrame30::FblItemRect> item : fblItems)
		{
			VFrame30::AfbPin pin;
			bool found = item->GetConnectionPoint(pinId, &pin);

			if (found == true)
			{
				return pin;
			}
		}

		return VFrame30::AfbPin();
	}

	std::vector<QUuid> Bush::getAllUuid() const
	{
		std::vector<QUuid> v;
		v.reserve(links.size() + 1);

		for (auto it : links)
		{
			v.push_back(it.first);
		}

		return v;
	}

	std::vector<QUuid> Bush::getLinksUuids() const
	{
		std::vector<QUuid> v;
		v.reserve(links.size());

		for (auto it : links)
		{
			v.push_back(it.first);
		}

		return v;
	}

	std::vector<VFrame30::AfbPin> Bush::getInputPinsForItem(QUuid fblItemUuid) const
	{
		// Get all input pins (for this bush) for an item
		//
		std::vector<VFrame30::AfbPin> result;

		VFrame30::FblItemRect* item = itemByGuid(fblItemUuid);
		if (item == nullptr)
		{
			assert(item);
			return result;
		}

		for (auto id : inputPins)
		{
			auto foundPin = std::find_if(std::begin(item->inputs()), std::end(item->inputs()),
				[&id](const VFrame30::AfbPin& itemInput)
				{
					return itemInput.guid() == id;
				});

			if (foundPin != std::end(item->inputs()))
			{
				result.push_back(*foundPin);
			}
		}

		return result;
	}

	// Function finds branch with a point on it.
	// Returns branch index or -1 if a brach was not found
	//
	int BushContainer::getBranchByPinPos(VFrame30::SchemaPoint pt) const
	{
		for (size_t i = 0; i < bushes.size(); i++)
		{
			const Bush& branch = bushes[i];

			auto link = std::find_if(branch.links.cbegin(), branch.links.cend(),
				[&pt](const std::pair<QUuid, Link>& link)
				{
					if (link.second.ptBegin() == pt || link.second.ptEnd() == pt)
					{
						return true;
					}

					return link.second.isPinOnLink(pt);
				});

			if (link != branch.links.end())
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	int BushContainer::getBranchByPinGuid(const QUuid& guid) const
	{
		for (size_t i = 0; i < bushes.size(); i++)
		{
			const Bush& branch = bushes[i];

			if (branch.outputPin == guid)
			{
				return static_cast<int>(i);
			}

			if (branch.inputPins.find(guid) != branch.inputPins.end())
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	void BushContainer::removeEmptyBushes()
	{
		// Remove bushes without fbls
		//
		bushes.erase(std::remove_if(bushes.begin(), bushes.end(),
					[](const Bush& b)
					{
						return b.fblItems.empty();
					}),	bushes.end());

	}


	// ------------------------------------------------------------------------
	//
	//		AppLogicItem
	//
	// ------------------------------------------------------------------------

	AppLogicItem::AppLogicItem(std::shared_ptr<VFrame30::FblItemRect> fblItem,
							   std::shared_ptr<VFrame30::LogicSchema> scheme,
							   std::shared_ptr<Afb::AfbElement> afbElement) :
		m_fblItem(fblItem),
		m_scheme(scheme)
	{
		assert(m_fblItem);
		assert(m_scheme);

		if (m_fblItem->isFblElement() == true)
		{
			if (afbElement != nullptr)
			{
				m_afbElement = (*afbElement.get());

				m_fblItem->toFblElement()->setAfbElementParams(&m_afbElement);
			}
			else
			{
				assert(afbElement != nullptr);
			}
		}

		return;
	}

	bool AppLogicItem::operator < (const AppLogicItem& li) const
	{
		return this->m_fblItem.get() < li.m_fblItem.get();
	}

	bool AppLogicItem::operator == (const AppLogicItem& li) const
	{
		if (this == &li)
		{
			return true;
		}

		bool result = this->m_fblItem == li.m_fblItem;

		if (result == true)
		{
			assert(this->m_scheme == li.m_scheme);
		}

		return result;
	}


	//
	// ChangeOrder struct -- used only in ApplicationLogicModule::orderItems
	//
	struct ChangeOrder
	{
		struct HistoryItem
		{
			AppLogicItem ChangeItem;
			int count;
		};

		AppLogicItem item;
		std::list<HistoryItem> history;

		int getChangeCount(const AppLogicItem& forItem)
		{
			auto it = std::find_if(history.begin(), history.end(),
				[&forItem](const HistoryItem& hi)
				{
					return hi.ChangeItem.m_fblItem == forItem.m_fblItem;
				});

			if (it == history.end())
			{
				return 0;
			}
			else
			{
				return it->count;
			}
		}

		void incChangeCount(const AppLogicItem& forItem)
		{
			auto it = std::find_if(history.begin(), history.end(),
				[&forItem](const HistoryItem& hi)
				{
					return hi.ChangeItem.m_fblItem == forItem.m_fblItem;
				});

			if (it == history.end())
			{
				HistoryItem hi{forItem, 1};
				history.push_back(hi);
			}
			else
			{
				it->count++;
			}
		}
	};



	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicModule
	//
	// ------------------------------------------------------------------------
	AppLogicModule::AppLogicModule(QString moduleStrId) :
		m_moduleStrId(moduleStrId)
	{
	}

	bool AppLogicModule::addBranch(std::shared_ptr<VFrame30::LogicSchema> logicScheme,
			const BushContainer& bushContainer,
			Afb::AfbElementCollection* afbCollection,
			IssueLogger* log)
	{
		if (logicScheme == nullptr ||
			log == nullptr ||
			afbCollection == nullptr)
		{
			assert(logicScheme);
			assert(afbCollection);
			assert(log);

			if (log != nullptr)
			{
				log->errINT1000(QString(__FUNCTION__ ", scheme %1, log %2, afbCollection %3")
								.arg(reinterpret_cast<size_t>(logicScheme.get()))
								.arg(reinterpret_cast<size_t>(log))
								.arg(reinterpret_cast<size_t>(afbCollection)));
			}

			return false;
		}

		if (bushContainer.bushes.empty() == true)
		{
			//LOG_WARNING_OBSOLETE(log, Builder::IssueType::NotDefined, QObject::tr("Logic scheme does no contains any correct links."));
			assert(false);	// if fires then add error to IussueLogger
			return true;
		}

		// Save all items to accumulator, they will be ordered in orderItems()
		//
		for (const Bush& bush : bushContainer.bushes)
		{
			for (const std::shared_ptr<VFrame30::FblItemRect>& f : bush.fblItems)
			{
				std::shared_ptr<Afb::AfbElement> afbElement;

				if (f->isFblElement())
				{
					afbElement = afbCollection->get(f->toFblElement()->afbStrID());

					if (afbElement == nullptr)
					{
						// AFB description '%1' is not found for scheme item '%2' (Logic Scheme '%3').
						//
						log->errALP4007(logicScheme->strID(), f->buildName(), f->toFblElement()->afbStrID(), f->guid());
						return false;
					}
				}

				AppLogicItem li{f, logicScheme, afbElement};

				m_fblItemsAcc.insert(li);
			}
		}

		return true;
	}

	bool AppLogicModule::orderItems(IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		// The last preparation - connect SchemeItemInput to SchemeItemOutput
		// Get all signals and put it to hash tables.
		//
		result = setInputOutputsElementsConnection(log);

		if (result == false)
		{
			return false;
		}

		// The end of the last preparation
		//

		// Get a COPY of the list, as the items will be moved to orderedList during algorithm work
		// Do not get reference!
		//
		std::set<AppLogicItem> constFblItems;
		std::set<AppLogicItem> fblItems;

		constFblItems = m_fblItemsAcc;
		fblItems = m_fblItemsAcc;

		m_fblItemsAcc.clear();					// Don't need it anymore, release items
		m_items.clear();

		// Add FblElement to branch in execution order
		//
		std::list<AppLogicItem> orderedList;

		// Add all inputs and outputs
		// Warning:	Can be optimized by removing items from fblItems on the same
		//			loop
		//
		bool hasItemsWithouInputs = false;

		for (const AppLogicItem& item : fblItems)
		{
			if (item.m_fblItem->inputsCount() == 0)
			{
				orderedList.push_front(item);	// items without inputs must be at the begining of the list
				hasItemsWithouInputs = true;
				continue;
			}

			if (item.m_fblItem->outputsCount() == 0)
			{
				orderedList.push_back(item);	// items without outputs must be at the end of the list
				continue;
			}
		}

		for (const AppLogicItem& orderedItem : orderedList)
		{
			fblItems.erase(orderedItem);
		}

		if (hasItemsWithouInputs == false)
		{
			// Imposible set exucution order for module, there is no first item,
			// firts item can be item without inputs
			//
			log->errALP4008(moduleStrId());

			result = false;
			return result;
		}

		std::list<ChangeOrder> changeOrderHistory;

		// Set other items
		//
		for (auto currentIt = orderedList.begin(); currentIt != orderedList.end(); ++currentIt)
		{
			AppLogicItem currentItem = *currentIt;		// NOT REFERENCE, ITEM CAN BE MOVED LATER

			// Get dependant items
			//
			std::set<AppLogicItem> dependantItems;

			const std::list<VFrame30::AfbPin>& outputs = currentItem.m_fblItem->outputs();

			for (const VFrame30::AfbPin& out : outputs)
			{
				auto deps = getItemsWithInput(constFblItems.begin(), constFblItems.end(), out.guid());

				dependantItems.insert(deps.begin(), deps.end());
			}

			if (dependantItems.empty() == true)
			{
				// This item does not have influence on orderList, probably it is in the end and has no outputs
				//
				continue;
			}

			// Check dependencies
			//
			for (AppLogicItem dep : dependantItems)
			{
				if (dep == currentItem)
				{
					// Loop for the same item, skip this dependance
					//
					continue;
				}

				// Check if dependant item is below current, if so, thats ok, don't do anything
				//
				auto dependantisBelow = std::find(currentIt, orderedList.end(), dep);

				if (dependantisBelow != orderedList.end())
				{
					// Dependant item already in orderedList, and it is under currentItem
					//
					continue;	// Process other dependtants, do not break!
				}

				// Check if the dependant above currentItem
				//
				auto dependantIsAbove = std::find(orderedList.begin(), currentIt, dep);

				if (dependantIsAbove != currentIt)
				{
					// Save hostory, if this is the third switch item, then skip it
					//
					auto histForCurrentItem = std::find_if(
												  changeOrderHistory.begin(),
												  changeOrderHistory.end(),
												  [&currentItem](const ChangeOrder& co)
					{
						return co.item == currentItem;
					});

					if (histForCurrentItem == changeOrderHistory.end())
					{
						ChangeOrder co;
						co.item = currentItem;
						co.incChangeCount(*dependantIsAbove);

						changeOrderHistory.push_back(co);
					}
					else
					{
						int switchCounter = histForCurrentItem->getChangeCount(*dependantIsAbove);

						if (switchCounter == 2)
						{
							continue;
						}
						else
						{
							histForCurrentItem->incChangeCount(*dependantIsAbove);
						}
					}

					// Dependant item is above currentItem, so let's move currentItem right before dependand one
					// in orderedList
					//
					auto tempIter = currentIt;

					currentIt = orderedList.insert(dependantIsAbove, currentItem);	// Upate currrentIt, it is important and part of the algorithm!

					orderedList.erase(tempIter);

					continue;	// Process other dependtants, do not break!
				}

				// Obviusly dependant item not in orderedList yet, add it right after currentItem
				//
				assert(std::find(orderedList.begin(), orderedList.end(), dep) == orderedList.end());

				orderedList.insert(std::next(currentIt), dep);
				fblItems.erase(dep);

				// process other dependtants, do not break!
			}
		}

		if (fblItems.empty() == false)
		{
			// Not all items were processes, it can happen if item with input pins does not have any connection
			// to these inputs
			//
			for (const AppLogicItem& item : fblItems)
			{
				LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
						  tr("%1 was not processed").arg(item.m_fblItem->buildName()));
			}

			result = false;
		}
		else
		{
			// -- debug
			qDebug() << "";
			qDebug() << "ORDERED LIST FOR MODULE " << m_moduleStrId;
			qDebug() << "";

			for (const AppLogicItem& item : orderedList)
			{
				if (item.m_fblItem->isInputSignalElement())
				{
					qDebug() << "Input " << item.m_fblItem->toInputSignalElement()->signalStrIds();
					continue;
				}

				if (item.m_fblItem->isOutputSignalElement())
				{
					qDebug() << "Output " << item.m_fblItem->toOutputSignalElement()->signalStrIds();
					continue;
				}

				if (item.m_fblItem->isConstElement())
				{
					qDebug() << "Constant " << item.m_fblItem->toSchemeItemConst()->valueToString();
					continue;
				}


				if (item.m_fblItem->isFblElement())
				{
					qDebug() << "Fbl " << item.m_afbElement.caption();
					continue;
				}

				qDebug() << "ERROR, UNKWNOW element " << item.m_fblItem->metaObject()->className();
			}

			// -- end of debug

			// Set complete data
			//

			std::swap(m_items, orderedList);
		}

		return result;
	}

	bool AppLogicModule::setInputOutputsElementsConnection(IssueLogger* log)
	{
		// Set connection between SchemeItemInput/SchemeItemOutput by StrIds
		//
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		QHash<QString, AppLogicItem> signalInputItems;
		QHash<QString, AppLogicItem> signalOutputItems;

		for (AppLogicItem li : m_fblItemsAcc)
		{
			if (li.m_fblItem->isInputSignalElement())
			{
				VFrame30::SchemeItemSignal* signalElement = li.m_fblItem->toSignalElement();
				assert(signalElement);

				// !!!! IN FUTURE, POSSIBLE WE WILL RECEIVE STRING ARRAY HERE, NOW WE ASSUME ONLY ONE STRID IS HERE
				//
				QString signalStrId = signalElement->signalStrIds();

				signalInputItems.insert(signalStrId, li);
				continue;
			}

			if (li.m_fblItem->isOutputSignalElement())
			{
				VFrame30::SchemeItemSignal* signalElement = li.m_fblItem->toSignalElement();
				assert(signalElement);

				// !!!! IN FUTURE, POSSIBLE WE WILL RECEIVE STRING ARRAY HERE, NOW WE ASSUME ONLY ONE STRID IS HERE
				//
				QString signalStrId = signalElement->signalStrIds();

				auto duplicateItem = signalOutputItems.find(signalStrId);
				if (duplicateItem != signalOutputItems.end())
				{
					// Duplicate output signal %1, item '%2' on scheme '%3', item '%4' on scheme '%5' (Logic Module '%6').
					//
					std::vector<QUuid> itemsUuids;
					itemsUuids.push_back(li.m_fblItem->guid());
					itemsUuids.push_back(duplicateItem->m_fblItem->guid());

					log->errALP4009(moduleStrId(),
									li.m_scheme->strID(),
									duplicateItem->m_scheme->strID(),
									li.m_fblItem->buildName(),
									duplicateItem->m_fblItem->buildName(),
									signalStrId,
									itemsUuids);
					continue;
				}

				signalOutputItems.insert(signalStrId, li);
				continue;
			}
		}

		// Continue the last preparation, look for same strIds in inputs/outputs
		//
		for (auto lit = signalInputItems.begin(); lit != signalInputItems.end(); ++lit)
		{
			QString inputStrId = lit.key();

			auto outputIt = signalOutputItems.find(inputStrId);

			if (outputIt == signalOutputItems.end())
			{
				// This input does not have pair with output, do not process it
				//
				continue;
			}

			AppLogicItem& inputLogicItem = lit.value();
			AppLogicItem& outputLogicItem = outputIt.value();

			// Add input pin to input element
			//
			assert(inputLogicItem.m_fblItem->inputsCount() == 0);
			inputLogicItem.m_fblItem->addInput();

			// if output element does not have fake output pin then add it
			//
			if (outputLogicItem.m_fblItem->outputsCount() == 0)
			{
				outputLogicItem.m_fblItem->addOutput();
			}

			// Associate items to each other
			//

			assert(inputLogicItem.m_fblItem->inputsCount() == 1);		// Check fake input
			assert(outputLogicItem.m_fblItem->outputsCount() == 1);		// Check fake output

			QUuid inputGuid = inputLogicItem.m_fblItem->inputs().front().guid();
			QUuid outputGuid = outputLogicItem.m_fblItem->outputs().front().guid();

			inputLogicItem.m_fblItem->mutableInputs()->front().AddAssociattedIOs(outputGuid);
			outputLogicItem.m_fblItem->mutableOutputs()->front().AddAssociattedIOs(inputGuid);
		}

		return true;
	}

	template<typename Iter>
	std::list<AppLogicItem> AppLogicModule::getItemsWithInput(
		Iter begin,
		Iter end,
		const QUuid& guid)
	{
		std::set<AppLogicItem> result;	// set removes duplicats

		for (auto item = begin; item != end; ++item)
		{
			const std::list<VFrame30::AfbPin>& inputs = item->m_fblItem->inputs();

			for (auto in : inputs)
			{
				const std::list<QUuid>& associatedOutputs = in.associatedIOs();

				auto foundAssociated = std::find(associatedOutputs.begin(), associatedOutputs.end(), guid);

				if (foundAssociated != associatedOutputs.end())
				{
					result.insert(*item);
					break;
				}
			}
		}

		std::list<AppLogicItem> resultList;
		resultList.assign(result.begin(), result.end());

		return resultList;
	}


//	template<typename Iter>
//	std::list<std::shared_ptr<VFrame30::FblItemRect>> ApplicationLogicModule::getItemsWithInput(
//		Iter begin,
//		Iter end,
//		const std::list<QUuid>& guids)
//	{
//		std::set<std::shared_ptr<VFrame30::FblItemRect>> result;	// set removes duplicats

//		for (auto item = begin; item != end; ++item)
//		{
//			const std::list<VFrame30::CFblConnectionPoint>& inputs = item->get()->inputs();

//			for (auto in : inputs)
//			{
//				size_t inResultSize = result.size();

//				const std::list<QUuid>& associatedOutputs = in.associatedIOs();

//				for (const QUuid& id : guids)
//				{
//					auto foundAssociated = std::find(associatedOutputs.begin(), associatedOutputs.end(), id);

//					if (foundAssociated != associatedOutputs.end())
//					{
//						result.insert(*item);
//						break;
//					}
//				}

//				if (inResultSize != result.size())
//				{
//					break;
//				}
//			}
//		}

//		std::list<std::shared_ptr<VFrame30::FblItemRect>> resultList;
//		resultList.assign(result.begin(), result.end());

//		return resultList;
//	}


	QString AppLogicModule::moduleStrId() const
	{
		return m_moduleStrId;
	}

	void AppLogicModule::setModuleStrId(QString value)
	{
		m_moduleStrId = value;
	}

	const std::list<AppLogicItem>& AppLogicModule::items() const
	{
		return m_items;
	}

	std::list<AppLogicItem>& AppLogicModule::items()
	{
		return m_items;
	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	AppLogicData::AppLogicData()
	{
	}


	std::shared_ptr<AppLogicModule> AppLogicData::getModuleLogicData(QString moduleStrID)
	{
		for(std::shared_ptr<AppLogicModule> modulePtr : m_modules)
		{
			if (modulePtr->moduleStrId() == moduleStrID)
			{
				return modulePtr;
			}
		}

		std::shared_ptr<AppLogicModule> nullPtr(nullptr);
		return nullPtr;
	}


	bool AppLogicData::addData(const BushContainer& bushContainer,
			std::shared_ptr<VFrame30::LogicSchema> scheme,
			std::shared_ptr<VFrame30::SchemaLayer> layer,
			Afb::AfbElementCollection* afbCollection,
			IssueLogger* log)
	{
		if (bushContainer.bushes.empty() == true)
		{
			// It is not error, just algorithm does not contain any fbl elements
			//
			return true;
		}

		if (scheme == nullptr ||
			layer == nullptr ||
			log == nullptr ||
			afbCollection == nullptr)
		{
			assert(scheme);
			assert(layer);
			assert(log);
			assert(afbCollection);

			log->errINT1000(QString(__FUNCTION__ ", scheme %1, layer %2, log %3, afbCollection %4")
							.arg(reinterpret_cast<size_t>(scheme.get()))
							.arg(reinterpret_cast<size_t>(layer.get()))
							.arg(reinterpret_cast<size_t>(log))
							.arg(reinterpret_cast<size_t>(afbCollection)));
			return false;
		}

		// Get module, if it is not in the list, add it
		//
		QStringList moduleStrIdList = scheme->hardwareStrIdList();
		bool result = true;

		for (QString moduleStrId : moduleStrIdList)
		{
			std::shared_ptr<AppLogicModule> module;

			auto moduleIt = std::find_if(m_modules.begin(), m_modules.end(),
										 [&moduleStrId](const std::shared_ptr<AppLogicModule>& m)
			{
				return m->moduleStrId() == moduleStrId;
			});

			if (moduleIt == m_modules.end())
			{
				// Module was not found, addit
				//
				module = std::make_shared<AppLogicModule>(moduleStrId);
				m_modules.push_back(module);
			}
			else
			{
				module = *moduleIt;
			}

			assert(module);

			// add new branch to module
			//
			result &= module->addBranch(scheme, bushContainer, afbCollection, log);
		}

		return result;
	}


	bool AppLogicData::orderItems(IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(nullptr);
			return false;
		}

		bool ok = true;
		bool result = true;

		for (std::shared_ptr<AppLogicModule> m : m_modules)
		{
			LOG_MESSAGE(log, QObject::tr("Module: %1").arg(m->moduleStrId()));

			ok = m->orderItems(log);

			if (ok == false)
			{
				result = false;
			}
		}

		return result;
	}

	const std::list<std::shared_ptr<AppLogicModule>>& AppLogicData::modules() const
	{
		return m_modules;
	}

	std::list<std::shared_ptr<AppLogicModule>>& AppLogicData::modules()
	{
		return m_modules;
	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------


	Parser::Parser(DbController* db,
				   IssueLogger* log,
				   AppLogicData* appLogicData,
				   Afb::AfbElementCollection* afbCollection, Hardware::EquipmentSet* equipmentSet,
				   int changesetId,
				   bool debug) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug),
		m_applicationData(appLogicData),
		m_afbCollection(afbCollection),
		m_equipmentSet(equipmentSet)
	{
		assert(m_db);
		assert(m_log);
		assert(m_applicationData);
		assert(m_afbCollection);
		assert(m_equipmentSet);

		return;
	}

	Parser::~Parser()
	{
	}

	bool Parser::build()
	{
		// Get Application Logic
		//
		std::vector<std::shared_ptr<VFrame30::LogicSchema>> schemes;

		bool ok = loadAppLogicFiles(db(), &schemes);

		if (ok == false)
		{
			return ok;
		}

		if (schemes.empty() == true)
		{
			LOG_MESSAGE(m_log, tr("There is no appliction logic files in the project."));
			return true;
		}

		// Check schemes hardwareSetrIds
		//
		assert(m_equipmentSet);

		if (m_equipmentSet != nullptr)
		{
			for (std::shared_ptr<VFrame30::LogicSchema> scheme : schemes)
			{
				QStringList deviceStrIds = scheme->hardwareStrIdList();

				if (deviceStrIds.isEmpty() == true)
				{
					// Property DeviceStrIds is not set (LogicScheme '%1')
					//
					m_log->wrnALP4001(scheme->strID());
					continue;
				}

				for (QString strid : deviceStrIds)
				{
					Hardware::DeviceObject* device = m_equipmentSet->deviceObject(strid);

					if (device == nullptr)
					{
						// HardwareStrId '%1' is not found in the project equipment (Logic Scheme '%2')
						//
						m_log->wrnALP4002(scheme->strID(), strid);
						continue;
					}

					if (device->isModule() == false)
					{
						// HardwareStrId '%1' must be LM family module type (Logic Scheme '%2').
						//
						m_log->wrnALP4003(scheme->strID(), strid);
						continue;
					}
					else
					{
						// Is module, check if it is LM family
						//
						Hardware::DeviceModule* module = device->toModule();
						assert(module);

						if (module != nullptr && module->moduleFamily() != Hardware::DeviceModule::FamilyType::LM)
						{
							// HardwareStrId '%1' must be LM family module type (Logic Scheme '%2').
							//
							m_log->wrnALP4003(scheme->strID(), strid);
							continue;
						}
					}
				}
			}
		}


		// Compile application logic
		//
		LOG_MESSAGE(m_log, tr("Parsing schemes..."));

		bool result = true;

		for (std::shared_ptr<VFrame30::LogicSchema> scheme : schemes)
		{
			LOG_MESSAGE(m_log, scheme->caption());

			ok = parseAppLogicScheme(scheme);

			if (ok == false)
			{
				result = false;
			}
		}

		// The result is set of AppLogicModule (m_modules), but items are not ordered yet
		// Order itmes in all modules
		//
		LOG_MESSAGE(m_log, tr("Ordering items..."));

		ok = m_applicationData->orderItems(m_log);

		if (ok == false)
		{
			result = false;
		}

		return result;
	}

	bool Parser::loadAppLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicSchema>>* out)
	{
		if (out == nullptr)
		{
			assert(out);
			m_log->errINT1000(QString(__FUNCTION__ ", out %1").arg(reinterpret_cast<size_t>(out)));
			return false;
		}

		out->clear();

		// Get application logic file list from the DB
		//
		bool ok = false;
		std::vector<DbFileInfo> applicationLogicFileList;

		if (release() == true)
		{
			assert(false);
		}
		else
		{
			ok = db->getFileList(&applicationLogicFileList, db->alFileId(), "%.als", true, nullptr);
		}

		if (ok == false)
		{
			// Error of getting file list from the database, parent file ID %1, filter '%2', database message %3.
			//
			m_log->errPDB2001(db->alFileId(), "%.als", db->lastError());
			return false;
		}

		if (applicationLogicFileList.empty() == true)
		{
			return true;		// it is not a error
		}

		out->reserve(applicationLogicFileList.size());

		// Get file data and read it
		//
		for (DbFileInfo& fi : applicationLogicFileList)
		{
			LOG_MESSAGE(m_log, tr("Loading %1").arg(fi.fileName()));

			std::shared_ptr<DbFile> file;

			if (release() == true)
			{
				assert(false);			// get specific files
			}
			else
			{
				ok = db->getLatestVersion(fi, &file, nullptr);
			}

			if (ok == false)
			{
				// Getting file instance error, file ID %1, file name '%2', database message '%3'.
				//
				m_log->errPDB2002(fi.fileId(), fi.fileName(), db->lastError());
				return false;
			}

			// Read Appliaction logic files
			//
			std::shared_ptr<VFrame30::LogicSchema> ls(dynamic_cast<VFrame30::LogicSchema*>(VFrame30::Schema::Create(file.get()->data())));

			if (ls == nullptr)
			{
				assert(ls != nullptr);
				// File loading/parsing error, file is damaged or has incompatible format, file name '%1'.
				//
				m_log->errCMN0010(file->fileName());
				return false;
			}

			if (ls->excludeFromBuild() == true)
			{
				// Scheme is excluded from build (Scheme '%1').
				//
				m_log->wrnALP4004(ls->strID());
				continue;
			}
			else
			{
				// Add LogicScheme to result
				//
				out->push_back(ls);
			}
		}

		return true;
	}

	bool Parser::parseAppLogicScheme(std::shared_ptr<VFrame30::LogicSchema> logicScheme)
	{
		if (logicScheme.get() == nullptr)
		{
			assert(false);
			return false;
		}

		// Find layer for compilation
		//
		bool layerFound = false;
		bool ok = false;

		for (std::shared_ptr<VFrame30::SchemaLayer> l : logicScheme->Layers)
		{
			if (l->compile() == true)
			{
				layerFound = true;
				ok = parseAppLogicLayer(logicScheme, l);

				if (ok == false)
				{
					return false;
				}

				// We can parse only one layer
				//
				break;
			}
		}

		if (layerFound == false)
		{
			// Scheme does not have Logic layer (Logic Scheme '%1').
			//
			m_log->errALP4010(logicScheme->strID());
			return false;
		}

		return true;
	}

	bool Parser::parseAppLogicLayer(
		std::shared_ptr<VFrame30::LogicSchema> logicScheme,
		std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		if (logicScheme == nullptr || layer == nullptr)
		{
			assert(logicScheme);
			assert(layer);
			return false;
		}

		// Find all branches - connected links
		//
		BushContainer bushContainer;

		bool result = findBushes(logicScheme, layer, &bushContainer);

		if (result == false)
		{
			LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined, tr("Finding bushes error."));
			return false;
		}

		// Set pins' guids to bushes
		//
		result = setBranchConnectionToPin(logicScheme, layer, &bushContainer);

		if (result == false)
		{
			// All log errors should be reported in setBranchConnectionToPin
			//
			return false;
		}

		// Associates input/outputs
		//
		result = setPinConnections(logicScheme, layer, &bushContainer);

		// Generate afb list, and set it to some container
		//
		result = applicationData()->addData(bushContainer, logicScheme, layer, m_afbCollection, m_log);

		if (result == false)
		{
			// function addData will report about errors
			//
			return false;
		}

		return true;
	}

	// Function connects all links, and compose them into bushes
	//
	bool Parser::findBushes(
		std::shared_ptr<VFrame30::LogicSchema> logicScheme,
		std::shared_ptr<VFrame30::SchemaLayer> layer,
		BushContainer* bushContainer) const
	{
		if (logicScheme.get() == nullptr ||
			layer.get() == nullptr ||
			bushContainer == nullptr)
		{
			assert(logicScheme);
			assert(layer);
			assert(bushContainer);
			return false;
		}


		// FblItems can conatain input or output pins, if input connects to output directly (without link)
		// then create fake link for it.
		// This is for special case, when input connects to output without link.
		//
		for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
		{
			VFrame30::FblItemRect* fblItem = dynamic_cast<VFrame30::FblItemRect*>(item->get());

			if (fblItem != nullptr)
			{
				fblItem->SetConnectionsPos(logicScheme->gridSize(), logicScheme->pinGridStep());	// Calculate pins positions

				const std::list<VFrame30::AfbPin>& inputs = fblItem->inputs();
				const std::list<VFrame30::AfbPin>& outputs = fblItem->outputs();

				for (const VFrame30::AfbPin& pt : inputs)
				{
					std::shared_ptr<VFrame30::SchemeItemLink> fakeLink = std::make_shared<VFrame30::SchemeItemLink>(fblItem->itemUnit());

					VFrame30::SchemaPoint pos = pt.point();

					fakeLink->AddPoint(pos.X, pos.Y);
					fakeLink->AddPoint(pos.X, pos.Y);

					layer->Items.push_back(fakeLink);
				}

				for (const VFrame30::AfbPin& pt : outputs)
				{
					std::shared_ptr<VFrame30::SchemeItemLink> fakeLink = std::make_shared<VFrame30::SchemeItemLink>(fblItem->itemUnit());

					VFrame30::SchemaPoint pos = pt.point();

					fakeLink->AddPoint(pos.X, pos.Y);
					fakeLink->AddPoint(pos.X, pos.Y);

					layer->Items.push_back(fakeLink);
				}

				continue;
			}
		}

		// Enum all links and get all horzlinks and vertlinks
		//
		VFrame30::CHorzVertLinks horzVertLinks;

		for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
		{
			// Decompose link to parts
			//
			VFrame30::SchemeItemLink* link = dynamic_cast<VFrame30::SchemeItemLink*>(item->get());

			if (link != nullptr)
			{
				const std::list<VFrame30::SchemaPoint>& pointList = link->GetPointList();

				if (pointList.size() < 2)
				{
					assert(pointList.size() >= 2);
					continue;
				}

				// Decompose link on different parts and put them to horzlinks and vertlinks
				//
				horzVertLinks.AddLinks(pointList, link->guid());

				continue;
			}
		}

		// Enum all vert and horz links and compose bushes
		//
		std::list<std::set<QUuid>> bushes;

		for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
		{
			VFrame30::SchemeItemLink* link = dynamic_cast<VFrame30::SchemeItemLink*>(item->get());

			if (link == nullptr)
			{
				continue;
			}

			const std::list<VFrame30::SchemaPoint>& pointList = link->GetPointList();

			if (pointList.size() < 2)
			{
				assert(pointList.size() >= 2);
				continue;
			}

			// Check if end points on some link
			//
			std::list<QUuid> itemsUnderFrontPoint = horzVertLinks.getSchemeItemsUnderPoint(pointList.front(), link->guid());
			std::list<QUuid> itemsUnderBackPoint = horzVertLinks.getSchemeItemsUnderPoint(pointList.back(), link->guid());

			// Find item branch, if branch is not exists, make a new brach
			//
			auto foundBranch = std::find_if(bushes.begin(), bushes.end(),
				[link](const std::set<QUuid>& b)
				{
					auto foundBranch = b.find(link->guid());
					return foundBranch != b.end();
				});

			if (foundBranch == bushes.end())
			{
				std::set<QUuid> newBush;
				newBush.insert(link->guid());

				bushes.push_front(newBush);

				foundBranch = bushes.begin();
			}

			// Add to foundBranch everything from  itemsUnderFrontPoint, itemsUnderBackPoint
			//
			for (QUuid q : itemsUnderFrontPoint)
			{
				foundBranch->insert(q);
			}

			for (QUuid q : itemsUnderBackPoint)
			{
				foundBranch->insert(q);
			}
		}

		// branches can contain same items,
		// all such branches must be joined
		//
		bool wasJoining = false;	// if branch was joinedto other branch, then process currentBranch one more time

		for (auto currentBranch = bushes.begin();
			 currentBranch != bushes.end();
			 std::advance(currentBranch, wasJoining ? 0 : 1))
		{
			wasJoining = false;

			// currentBranch is std::set<QUuid>
			//

			// Take each id from the currentBranch,
			// try to find such branch where this id is present,
			// and join this branch to currentBranch
			//
			for (auto id = currentBranch->begin(); id != currentBranch->end(); ++id)
			{
				auto subBranch = currentBranch;
				++subBranch;
				for (; subBranch != bushes.end();)
				{
					if (std::find(subBranch->begin(), subBranch->end(), *id) != subBranch->end())
					{
						// Join found branch to currentBranch
						//
						for (auto& subBranchId : *subBranch)
						{
							currentBranch->insert(subBranchId);
						}

						// Delete subBrach, make tmp to delete it after interator increment
						//
						auto tmp = subBranch;
						++subBranch;

						bushes.erase(tmp);

						wasJoining = true;
						continue;
					}

					++subBranch;
				}
			}
		}

		// For all links in branches get its end points
		//

		bushContainer->bushes.reserve(bushes.size());

		for (const std::set<QUuid>& b : bushes)
		{
			Bush newBush;

			for (QUuid id : b)
			{
				// Get SchemeItemLink by this id,
				// save it's and points to newBranch
				//
				std::shared_ptr<VFrame30::SchemaItem> schemeItem = layer->getItemById(id);
				VFrame30::SchemeItemLink* link = dynamic_cast<VFrame30::SchemeItemLink*>(schemeItem.get());

				if (schemeItem == nullptr ||
					link == nullptr)
				{
					assert(schemeItem);
					assert(link);

					LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined, tr("%1 Internal error, expected VFrame30::SchemeItemLink").arg(__FUNCTION__));
					return false;
				}

				const std::list<VFrame30::SchemaPoint>& pointList = link->GetPointList();

				if (pointList.size() < 2)
				{
					assert(pointList.size() >= 2);
					LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined,
							  tr("%1 Internal error, Link has less the two points").arg(__FUNCTION__));
					return false;
				}

				newBush.links[id] = Link(pointList);
			}

			// Add bush to container
			//
			bushContainer->bushes.push_back(newBush);
		}

//		// DEBUG
//		for (Bush& eb : bushContainer->bushes)
//		{
//			qDebug() << "-----";
//			for (auto& bl : eb.links)
//			{
//				qDebug() << bl.first << "--" <<
//							bl.second.pt1.X <<
//							"-" <<
//							bl.second.pt1.Y <<
//							"    " <<
//							bl.second.pt2.X <<
//							"-" <<
//							bl.second.pt2.Y;
//			}
//		}
//		// END OF DEBUG

		return true;
	}

	bool Parser::setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicSchema> scheme, std::shared_ptr<VFrame30::SchemaLayer> layer,
						BushContainer* bushContainer) const
	{
		if (scheme.get() == nullptr ||
			layer.get() == nullptr ||
			bushContainer == nullptr)
		{
			assert(scheme);
			assert(layer);
			assert(bushContainer);

			log()->errINT1000(QString(__FUNCTION__ ", scheme %1, layer %2, bushContainer %3")
							  .arg(reinterpret_cast<size_t>(scheme.get()))
							  .arg(reinterpret_cast<size_t>(layer.get()))
							  .arg(reinterpret_cast<size_t>(bushContainer)));

			return false;
		}

		bool result = true;

		for (std::shared_ptr<VFrame30::SchemaItem> item : layer->Items)
		{
			if (dynamic_cast<VFrame30::FblItemLine*>(item.get()) != nullptr)
			{
				// This is link, it is already processed by composing branches
				//
				continue;
			}

			std::shared_ptr<VFrame30::FblItem> fblItem =
					std::dynamic_pointer_cast<VFrame30::FblItem>(item);

			if(fblItem != nullptr)
			{
				// SchemeItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblItem->ClearAssociatedConnections();
				fblItem->SetConnectionsPos(scheme->gridSize(), scheme->pinGridStep());

				std::list<VFrame30::AfbPin>* inputs = fblItem->mutableInputs();
				std::list<VFrame30::AfbPin>* outputs = fblItem->mutableOutputs();

				for (VFrame30::AfbPin& in : *inputs)
				{
					VFrame30::SchemaPoint pinPos = in.point();

					//qDebug() << "input  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = bushContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined,
								  tr("LogicScheme %1: %2 has unconnected pin %3")
								  .arg(scheme->caption())
								  .arg(fblItem->buildName())
								  .arg(in.caption()));

						result = false;
						continue;
					}

					// Branch was found for current pin
					//
					bushContainer->bushes[branchIndex].inputPins.insert(in.guid());
				}

				for (const VFrame30::AfbPin& out : *outputs)
				{
					VFrame30::SchemaPoint pinPos = out.point();

					//qDebug() << "output  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = bushContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						log()->errALP4006(scheme->strID(), fblItem->buildName(), out.caption(), item->guid());

						result = false;
						continue;
					}

					// Branch was found for current pin
					//

					if (bushContainer->bushes[branchIndex].outputPin.isNull() == false)
					{
						// Branch has multiple outputs.
						//
						log()->errALP4000(scheme->strID(), bushContainer->bushes[branchIndex].getAllUuid());

						result = false;
						continue;
					}
					else
					{
						bushContainer->bushes[branchIndex].outputPin = out.guid();
					}
				}
			}
		}

		return result;
	}


	bool Parser::setPinConnections(
		std::shared_ptr<VFrame30::LogicSchema> scheme,
		std::shared_ptr<VFrame30::SchemaLayer> layer,
		BushContainer* bushContainer)
	{
		if (scheme.get() == nullptr ||
			layer.get() == nullptr ||
			bushContainer == nullptr)
		{
			assert(scheme);
			assert(layer);
			assert(bushContainer);
			return false;
		}

		bool result = true;
		int hasFblItems = false;

		for (auto& item : layer->Items)
		{
			VFrame30::FblItemRect* fblirect = dynamic_cast<VFrame30::FblItemRect*>(item.get());

			if(fblirect != nullptr)
			{
				hasFblItems = true;

				std::shared_ptr<VFrame30::FblItemRect> fblElement =
						std::dynamic_pointer_cast<VFrame30::FblItemRect>(item);

				// SchemeItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblElement->ClearAssociatedConnections();
				fblElement->SetConnectionsPos(scheme->gridSize(), scheme->pinGridStep());

				std::list<VFrame30::AfbPin>* inputs = fblElement->mutableInputs();
				std::list<VFrame30::AfbPin>* outputs = fblElement->mutableOutputs();

				for (VFrame30::AfbPin& in : *inputs)
				{
					int branchIndex = bushContainer->getBranchByPinGuid(in.guid());

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						assert(false);

						LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined,
								  tr("LogicScheme %1: Internalerror in function, branch suppose to be found, %2.")
								  .arg(scheme->caption())
								  .arg(__FUNCTION__));

						result = false;
						return result;
					}

					// Branch was found for current pin
					//
					Bush& bush = bushContainer->bushes[branchIndex];

					bush.fblItems.insert(fblElement);

					if (bush.outputPin.isNull() == true)
					{
						// Error message @Input is not defined, for items: %1" is later in this function
						//
						result = false;
						continue;
					}

					// Set sourche pin guid for this pin
					//
					in.AddAssociattedIOs(bush.outputPin);
				}

				for (VFrame30::AfbPin& out : *outputs)
				{
					int branchIndex = bushContainer->getBranchByPinGuid(out.guid());

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						assert(false);

						LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined,
								  tr("LogicScheme %1: Internalerror in function, branch suppose to be found, %2.")
								  .arg(scheme->caption())
								  .arg(__FUNCTION__));

						result = false;
						return result;
					}				

					// Branch was found for current pin
					//
					Bush& bush = bushContainer->bushes[branchIndex];

					bush.fblItems.insert(fblElement);

					// Set destination pins guid for this pin
					//
					for (const QUuid& dstid : bush.inputPins)
					{
						out.AddAssociattedIOs(dstid);
					}
				}
			}
		}

		if (hasFblItems == false)
		{
			// Logic Scheme is empty, there are no any functional blocks in the compile layer (Logic Scheme '%1')
			//
			m_log->wrnALP4005(scheme->strID());
			return true;
		}

		// Delete all empty bushes (bushes without fblitems)
		//
		bushContainer->removeEmptyBushes();

		// Check bushes, bush must have output pin and one or several input pins
		//
		for (const Bush& bush : bushContainer->bushes)
		{

			if (bush.outputPin.isNull() == true)
			{
				for (std::shared_ptr<VFrame30::FblItemRect> i : bush.fblItems)
				{
					// Scheme item %1 has not linked pin %2 (Logic Scheme '%3').
					//
					std::vector<VFrame30::AfbPin> inputs = bush.getInputPinsForItem(i->guid());

					QString inputsStr;
					for (auto input : inputs)
					{
						inputsStr += (inputsStr.isEmpty() == true) ? input.caption() : QString(", %1").arg(input.caption());
					}

					std::vector<QUuid> issuedItemsUuid = bush.getLinksUuids();
					issuedItemsUuid.push_back(i->guid());

					m_log->errALP4006(scheme->strID(), i->buildName(), inputsStr, issuedItemsUuid);
				}
			}

			if (bush.inputPins.empty() == true)
			{
				// Look for item without associated inputs
				//
				for (std::shared_ptr<VFrame30::FblItemRect> item : bush.fblItems)
				{
					const std::list<VFrame30::AfbPin>& outputs = item->outputs();

					for (const VFrame30::AfbPin& out : outputs)
					{
						if (out.associatedIOs().empty() == true)
						{
							// Scheme item %1 has not linked pin %2 (Logic Scheme '%3').
							//
							std::vector<QUuid> issuedItemsUuid = bush.getLinksUuids();
							issuedItemsUuid.push_back(item->guid());

							m_log->errALP4006(scheme->strID(), item->buildName(), out.caption(), issuedItemsUuid);
						}
					}
				}
			}
		}

		return result;
	}


	DbController* Parser::db()
	{
		return m_db;
	}

	IssueLogger* Parser::log() const
	{
		return m_log;
	}

	int Parser::changesetId() const
	{
		return m_changesetId;
	}

	bool Parser::debug() const
	{
		return m_debug;
	}

	bool Parser::release() const
	{
		return !debug();
	}

	const AppLogicData* Parser::applicationData() const
	{
		return m_applicationData;
	}

	AppLogicData* Parser::applicationData()
	{
		return m_applicationData;
	}

}
