#include "Parser.h"

#include <typeindex>

#include "IssueLogger.h"
#include "GlobalMessanger.h"

#include "../../lib/DbController.h"

#include "../../VFrame30/LogicSchema.h"
#include "../../VFrame30/SchemaItemLink.h"
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/SchemaItemAfb.h"
#include "../../VFrame30/SchemaItemSignal.h"
#include "../../VFrame30/SchemaItemConst.h"
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

	QUuid Link::getNextId()
	{
		// The problem
		// Links has some id, and this id is used for maps, if we want all links have the same order from build to build for the same logic,
		// we must keep same id for same links
		//
		thread_local static uint64_t newid[2] = {0, 0};
		newid[0] ++;

		QByteArray ba = QByteArray::fromRawData(reinterpret_cast<const char*>(newid), 16);
		QUuid u = QUuid::fromRfc4122(ba);

		return u;
	}

	bool Link::isPinOnLink(VFrame30::SchemaPoint pt) const
	{
		VFrame30::CHorzVertLinks hvl;

		QUuid fakeId = Link::getNextId();
		hvl.AddLinks(m_points, fakeId);

		bool result = hvl.IsPinOnLink(pt, Link::getNextId());	// Must be othe Quuid, as if it the same return value always false

		return result;
	}

	VFrame30::FblItemRect* Bush::itemByPinGuid(QUuid pinId) const
	{
		for (auto item : fblItems)
		{
			VFrame30::AfbPin pin;
			bool found = item.second->GetConnectionPoint(pinId, &pin);

			if (found == true)
			{
				return item.second.get();
			}
		}

		return nullptr;
	}

	VFrame30::FblItemRect* Bush::itemByGuid(QUuid uuid) const
	{
		auto result = fblItems.find(uuid);

		if (result != fblItems.end())
		{
			assert(uuid == result->second->guid());
			return result->second.get();
		}
		else
		{
			return nullptr;
		}
	}

	VFrame30::AfbPin Bush::pinByGuid(QUuid pinId)
	{
		for (auto it = fblItems.begin(); it != fblItems.end(); ++it)
		{
			std::shared_ptr<VFrame30::FblItemRect> item = it->second;

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
		v.reserve(links.size() + fblItems.size());

		for (auto it : links)
		{
			v.push_back(it.first);
		}

		for (auto it : fblItems)
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

	bool Bush::hasCommonFbls(const Bush& bush) const
	{
		for (std::pair<QUuid, std::shared_ptr<VFrame30::FblItemRect>> fbl : bush.fblItems)
		{
			auto found = this->fblItems.find(fbl.first);

			if (found != this->fblItems.end())
			{
				return true;
			}
		}

		return false;
	}

//	bool Bush::hasInputOrOutput(const QUuid& uuid) const
//	{
//		if (outputPin == uuid)
//		{
//			return true;
//		}

//		auto findIt = inputPins.find(uuid);

//		if (findIt == inputPins.end())
//		{
//			return false;
//		}
//		else
//		{
//			return true;
//		}
//	}

//	bool Bush::hasJoinedInOuts(Bush& bush) const
//	{
//		if (bush.hasInputOrOutput(this->outputPin) == true)
//		{
//			return true;
//		}

//		for (const QUuid& uuid : inputPins)
//		{
//			if (bush.hasInputOrOutput(uuid) == true)
//			{
//				return true;
//			}
//		}

//		return false;
//	}

	void Bush::debugInfo() const
	{
		qDebug() << "Bush debug info:";
		qDebug() << "    OutputPin: "  << outputPin.toString();
		qDebug() << "    InputPins: ";

		for (QUuid u : inputPins)
		{
			qDebug() << "             " << u.toString();
		}

		if (itemByPinGuid(outputPin) != nullptr)
		{
			qDebug() << "    OutputItem: " << itemByPinGuid(outputPin)->buildName();
		}

		qDebug() << "    Links:";
		for (auto l = links.begin(); l != links.end(); l++)
		{
			qDebug() << "             " << l->first.toString();
		}

		qDebug() << "    FblItems:";
		for (auto it = fblItems.begin(); it != fblItems.end(); ++it)
		{
			std::shared_ptr<VFrame30::FblItemRect> item = it->second;
			assert(item);

			if (item != nullptr)
			{
				qDebug() << "           " << item->buildName();
			}
		}
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

	void BushContainer::debugInfo()
	{
		qDebug() << "---------------- BushContainer::debugInfo -----------------";

		for (const Bush& b : bushes)
		{
			b.debugInfo();
		}
	}


	// ------------------------------------------------------------------------
	//
	//		AppLogicItem
	//
	// ------------------------------------------------------------------------

	AppLogicItem::AppLogicItem(std::shared_ptr<VFrame30::FblItemRect> fblItem,
							   std::shared_ptr<VFrame30::LogicSchema> schema) :
		m_fblItem(fblItem),
		m_schema(schema)
	{
		assert(m_fblItem);
		assert(m_schema);

		if (m_fblItem->isAfbElement() == true)
		{
			m_afbElement = m_fblItem->toAfbElement()->afbElement();
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
			assert(this->m_schema == li.m_schema);
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
		m_moduleEquipmentId(moduleStrId)
	{
	}

	bool AppLogicModule::addBranch(std::shared_ptr<VFrame30::LogicSchema> logicSchema,
			const BushContainer& bushes,
			IssueLogger* log)
	{
		if (logicSchema == nullptr ||
			log == nullptr)
		{
			assert(logicSchema);
			assert(log);

			if (log != nullptr)
			{
				log->errINT1000(QString(__FUNCTION__) + QString(", schema %1, log %2.")
								.arg(reinterpret_cast<size_t>(logicSchema.get()))
								.arg(reinterpret_cast<size_t>(log)));
			}

			return false;
		}

		// Create a copy of bushcontainer, as in case of processing multisignlas in multichannel schemas
		// these multisignals will be created for all channels
		//
		BushContainer busheContainer = bushes;

		if (busheContainer.bushes.empty() == true)
		{
			//LOG_WARNING_OBSOLETE(log, Builder::IssueType::NotDefined, QObject::tr("Logic schema does no contains any correct links."));
			assert(false);	// if fires then add error to IussueLogger
			return true;
		}

		// Save all items to accumulator, they will be ordered in orderItems()
		//
		for (const Bush& bush : busheContainer.bushes)
		{
			for (auto it = bush.fblItems.begin(); it != bush.fblItems.end(); ++it)
			{
				const std::shared_ptr<VFrame30::FblItemRect>& f = it->second;

				if (f->isSignalElement() == true && f->toSignalElement()->multiChannel() == true)
				{
					log->errALP4030(logicSchema->schemaID(), f->buildName(), f->guid());
				}

				AppLogicItem li{f, logicSchema};

				m_fblItemsAcc[f->guid()] = li;
			}
		}

//		qDebug() << "----m_fblItemsAcc-------------------- AppLogicModule::addBranch ------------------------ ";
//		for (auto li : m_fblItemsAcc)
//		{
//			qDebug() << li.second.m_fblItem->buildName();
//		}

		return true;
	}

	bool AppLogicModule::orderItems(IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		qDebug() << "Order items for module " << m_moduleEquipmentId;

		bool result = true;

		// The last preparation - connect SchemaItemInput to SchemaItemOutput
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
		std::map<QUuid, AppLogicItem> constFblItems;
		std::map<QUuid, AppLogicItem> fblItems;

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

		for (auto item : fblItems)
		{
			if (item.second.m_fblItem->inputsCount() == 0)
			{
				orderedList.push_front(item.second);	// items without inputs must be at the begining of the list
				hasItemsWithouInputs = true;
				continue;
			}
		}

		for (auto item : fblItems)
		{
			if (item.second.m_fblItem->outputsCount() == 0)
			{
				orderedList.push_back(item.second);	// items without outputs must be at the end of the list
				continue;
			}
		}

		for (const AppLogicItem& orderedItem : orderedList)
		{
			fblItems.erase(orderedItem.m_fblItem->guid());
		}

//		if (hasItemsWithouInputs == false)
//		{
//			// Imposible set exucution order for module, there is no first item,
//			// firts item can be item without inputs
//			//
//			log->errALP4020(moduleEquipmentId());

//			result = false;
//			return result;
//		}

		int pass = 1;
		size_t checkRemainsCount = -1;			// it's ok to give a second change for setItemsOrder to remove some items form fblItems
		while (fblItems.empty() == false)
		{
			qDebug() << "Pass " << pass++;

			setItemsOrder(log, fblItems, orderedList, constFblItems);

			if (checkRemainsCount == fblItems.size())
			{
				// setItemsOrder did not processed any item
				//
				break;
			}
			else
			{
				checkRemainsCount = fblItems.size();	// fblItems.size() must be changed in setItemsOrder, if it did not happened
			}

			if (fblItems.empty() == false)
			{
				// some items in the accumulator
				//
				const AppLogicItem& item = fblItems.begin()->second;
				orderedList.push_back(item);

				fblItems.erase(item.m_fblItem->guid());
			}
		}

		// --
		//
		if (fblItems.empty() == false)
		{
			// Not all items were processes, it can happen if item with input pins does not have any connection
			// to these inputs
			//
			for (auto it = fblItems.begin(); it != fblItems.end(); ++it)
			{
				const AppLogicItem& item = it->second;

				LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
						  tr("%1 was not processed").arg(item.m_fblItem->buildName()));
			}

			result = false;
		}
		else
		{
			// Set complete data
			//

			std::swap(m_items, orderedList);
		}

		return result;
	}

	bool AppLogicModule::setItemsOrder(IssueLogger* log,
									   std::map<QUuid, AppLogicItem>& remainItems,
									   std::list<AppLogicItem>& orderedItems,
									   const std::map<QUuid, AppLogicItem>& constItems)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		std::list<ChangeOrder> changeOrderHistory;

		// Set other items
		//
		for (auto currentIt = orderedItems.begin(); currentIt != orderedItems.end(); ++currentIt)
		{
			AppLogicItem currentItem = *currentIt;		// NOT REFERENCE, ITEM CAN BE MOVED LATER

			//qDebug() << "Parsing -- order item " << currentItem.m_fblItem->buildName();

			// Get dependant items
			//
			std::map<QUuid, AppLogicItem> dependantItems;

			const std::list<VFrame30::AfbPin>& outputs = currentItem.m_fblItem->outputs();

			for (const VFrame30::AfbPin& out : outputs)
			{
				auto deps = getItemsWithInput(constItems.begin(), constItems.end(), out.guid());

				//qDebug() << "Dependant Items:";
				for (const AppLogicItem& di : deps)
				{
					//qDebug() << "\t" << di.m_fblItem->buildName();
					dependantItems[di.m_fblItem->guid()] = di;
				}
			}

			if (dependantItems.empty() == true)
			{
				// This item does not have influence on orderList, probably it is in the end and has no outputs
				//
				continue;
			}

			// Check dependencies
			//
			for (auto depIt = dependantItems.begin(); depIt != dependantItems.end(); ++depIt)
			{
				AppLogicItem dep = depIt->second;

				if (dep == currentItem)
				{
					// Loop for the same item, skip this dependance
					//
					continue;
				}

				// Check if dependant item is below current, if so, thats ok, don't do anything
				//
				auto dependantisBelow = std::find(currentIt, orderedItems.end(), dep);

				if (dependantisBelow != orderedItems.end())
				{
					// Dependant item already in orderedList, and it is under currentItem
					//
					continue;	// Process other dependtants, do not break!
				}

				// Check if the dependant above currentItem
				//
				auto dependantIsAbove = std::find(orderedItems.begin(), currentIt, dep);

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

					currentIt = orderedItems.insert(dependantIsAbove, currentItem);	// Upate currrentIt, it is important and part of the algorithm!

					orderedItems.erase(tempIter);

					continue;	// Process other dependtants, do not break!
				}

				// Obviusly dependant item is not in orderedList yet, add it right after currentItem
				//
				assert(std::find(orderedItems.begin(), orderedItems.end(), dep) == orderedItems.end());

				orderedItems.insert(std::next(currentIt), dep);
				remainItems.erase(dep.m_fblItem->guid());

				// process other dependtants, do not break!
			}
		}

		return true;
	}

	bool AppLogicModule::setInputOutputsElementsConnection(IssueLogger* log)
	{
		// Set connection between SchemaItemInput/SchemaItemOutput by StrIds
		//
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		QHash<QString, AppLogicItem> signalInputItems;
		QHash<QString, AppLogicItem> signalOutputItems;

		for (auto lipair : m_fblItemsAcc)
		{
			const AppLogicItem& li = lipair.second;

			if (li.m_fblItem->isInputSignalElement() == true)
			{
				VFrame30::SchemaItemSignal* signalElement = li.m_fblItem->toSignalElement();
				assert(signalElement);

				QString signalStrId = signalElement->appSignalIds();

				signalInputItems.insert(signalStrId, li);
				continue;
			}

			if (li.m_fblItem->isOutputSignalElement() == true ||
				li.m_fblItem->isInOutSignalElement() == true)
			{
				VFrame30::SchemaItemSignal* signalElement = li.m_fblItem->toSignalElement();
				assert(signalElement);

				QString signalStrId = signalElement->appSignalIds();

				auto duplicateItem = signalOutputItems.find(signalStrId);
				if (duplicateItem != signalOutputItems.end())
				{
					// Duplicate output signal %1, item '%2' on schema '%3', item '%4' on schema '%5' (Logic Module '%6').
					//
					std::vector<QUuid> itemsUuids;
					itemsUuids.push_back(li.m_fblItem->guid());
					itemsUuids.push_back(duplicateItem->m_fblItem->guid());

					log->errALP4021(moduleEquipmentId(),
									li.m_schema->schemaID(),
									duplicateItem->m_schema->schemaID(),
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
		std::map<QUuid, AppLogicItem> result;	// set removes duplicats

		for (auto it = begin; it != end; ++it)
		{
			const AppLogicItem& item = it->second;
			const std::list<VFrame30::AfbPin>& inputs = item.m_fblItem->inputs();

			for (auto in : inputs)
			{
				const std::list<QUuid>& associatedOutputs = in.associatedIOs();

				auto foundAssociated = std::find(associatedOutputs.begin(), associatedOutputs.end(), guid);

				if (foundAssociated != associatedOutputs.end())
				{
					result[item.m_fblItem->guid()] = item;
					break;
				}
			}
		}

		std::list<AppLogicItem> resultList;

		for (auto it = result.begin(); it != result.end(); ++it)
		{
			resultList.push_back(it->second);
		}

		return resultList;
	}

	QString AppLogicModule::moduleEquipmentId() const
	{
		return m_moduleEquipmentId;
	}

	void AppLogicModule::setModuleEquipmentId(QString value)
	{
		m_moduleEquipmentId = value;
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
			if (modulePtr->moduleEquipmentId() == moduleStrID)
			{
				return modulePtr;
			}
		}

		std::shared_ptr<AppLogicModule> nullPtr(nullptr);
		return nullPtr;
	}


	bool AppLogicData::addData(QString equipmentId,
							   const BushContainer& bushContainer,
							   std::shared_ptr<VFrame30::LogicSchema> schema,
							   IssueLogger* log)
	{
		if (equipmentId.isEmpty() == true)
		{
			log->errALP4001(schema->schemaID());
			return false;
		}

		if (bushContainer.bushes.empty() == true)
		{
			// It is not error, just algorithm does not contain any fbl elements
			//
			return true;
		}

		if (schema == nullptr ||
			log == nullptr)
		{
			assert(schema);
			assert(log);

			log->errINT1000(QString(__FUNCTION__) + QString(", schema %1, log %3")
							.arg(reinterpret_cast<size_t>(schema.get()))
							.arg(reinterpret_cast<size_t>(log)));
			return false;
		}

		// Get module, if it is not in the list, add it
		//
		bool result = true;

		std::shared_ptr<AppLogicModule> module;

		auto moduleIt = std::find_if(m_modules.begin(), m_modules.end(),
									 [&equipmentId](const std::shared_ptr<AppLogicModule>& m)
		{
			return m->moduleEquipmentId() == equipmentId;
		});

		if (moduleIt == m_modules.end())
		{
			// Module was not found, addit
			//
			module = std::make_shared<AppLogicModule>(equipmentId);
			m_modules.push_back(module);
		}
		else
		{
			module = *moduleIt;
		}

		assert(module);

		// add new branch to module
		//
		result &= module->addBranch(schema, bushContainer, log);

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
			LOG_MESSAGE(log, QObject::tr("Module: %1").arg(m->moduleEquipmentId()));

			ok = m->orderItems(log);

			if (ok == false)
			{
				result = false;
			}

			QString str = QString("%1 functional item(s) parsed").arg(m->items().size());
			LOG_MESSAGE(log, str);
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
				   Afb::AfbElementCollection* afbCollection,
				   Hardware::EquipmentSet* equipmentSet,
				   SignalSet* signalSet,
				   int changesetId,
				   bool debug) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug),
		m_applicationData(appLogicData),
		m_afbCollection(afbCollection),
		m_equipmentSet(equipmentSet),
		m_signalSet(signalSet)
	{
		assert(m_db);
		assert(m_log);
		assert(m_applicationData);
		assert(m_afbCollection);
		assert(m_equipmentSet);
		assert(m_signalSet);

		return;
	}

	Parser::~Parser()
	{
	}

	bool Parser::build()
	{
		// Get Application Logic
		//
		std::vector<std::shared_ptr<VFrame30::LogicSchema>> schemas;

		bool ok = loadAppLogicFiles(db(), &schemas);

		if (ok == false)
		{
			return ok;
		}

		if (schemas.empty() == true)
		{
			LOG_MESSAGE(m_log, tr("There is no appliction logic files in the project."));
			return true;
		}

		// Check schemas EquipmentIDs
		//
		for (std::shared_ptr<VFrame30::LogicSchema> schema : schemas)
		{
			checkEquipmentIds(schema.get());
		}

		// Check SchemaItemAfb.afbElement versions
		//
		for (std::shared_ptr<VFrame30::LogicSchema> schema : schemas)
		{
			checkAfbItemsVersion(schema.get());
		}

		// Parse application logic
		//
		LOG_MESSAGE(m_log, tr("Parsing schemas..."));

		bool result = true;

		for (std::shared_ptr<VFrame30::LogicSchema> schema : schemas)
		{
			LOG_MESSAGE(m_log, schema->caption());

			ok = parseAppLogicSchema(schema);

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

		//  In debug mode save/show item order for displaying on schemas
		//
		debugInfo();

		return result;
	}

	bool Parser::loadAppLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicSchema>>* out)
	{
		if (out == nullptr)
		{
			assert(out);
			m_log->errINT1000(QString(__FUNCTION__) + QString(", out %1").arg(reinterpret_cast<size_t>(out)));
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

			std::vector<DbFileInfo> markedAsDeletedRemoved;
			markedAsDeletedRemoved.reserve(applicationLogicFileList.size());

			for (DbFileInfo& fi : applicationLogicFileList)
			{
				if (fi.action() == VcsItemAction::Deleted)		// File is deleted
				{
					qDebug() << "Skip file " << fi.fileId() << ", " << fi.fileName() << ", it was marked as deleted";
					continue;
				}

				markedAsDeletedRemoved.push_back(fi);
			}

			std::swap(markedAsDeletedRemoved, applicationLogicFileList);
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
				// Schema is excluded from build (Schema '%1').
				//
				m_log->wrnALP4004(ls->schemaID());
				continue;
			}
			else
			{
				// Add LogicSchema to result
				//
				out->push_back(ls);
			}
		}

		return true;
	}

	bool Parser::checkEquipmentIds(VFrame30::LogicSchema* logicSchema)
	{
		if (logicSchema == nullptr ||
			m_equipmentSet == nullptr)
		{
			assert(logicSchema);
			assert(m_equipmentSet);

			m_log->errINT1000(QString(__FUNCTION__) + QString(", logicSchema %1, Parser::m_equipmentSet %2")
							  .arg(reinterpret_cast<size_t>(logicSchema))
							  .arg(reinterpret_cast<size_t>(m_equipmentSet)));
			return false;
		}

		QStringList equipmentIds = logicSchema->equipmentIdList();

		if (equipmentIds.isEmpty() == true)
		{
			// Property EquipmentIds is not set (LogicSchema '%1')
			//
			m_log->errALP4001(logicSchema->schemaID());
			return false;
		}

		bool ok = true;

		for (QString eqid : equipmentIds)
		{
			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(eqid);

			if (device == nullptr)
			{
				// EquipmentID '%1' is not found in the project equipment (Logic Schema '%2')
				//
				m_log->errALP4002(logicSchema->schemaID(), eqid);

				ok = false;
				continue;
			}

			if (device->isModule() == false)
			{
				// EquipmentID '%1' must be LM family module type (Logic Schema '%2').
				//
				m_log->errALP4003(logicSchema->schemaID(), eqid);

				ok = false;
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
					// EquipmentID '%1' must be LM family module type (Logic Schema '%2').
					//
					m_log->errALP4003(logicSchema->schemaID(), eqid);

					ok = false;
					continue;
				}
			}
		}

		return ok;
	}

	bool Parser::checkAfbItemsVersion(VFrame30::LogicSchema* logicSchema)
	{
		if (logicSchema == nullptr ||
			m_afbCollection == nullptr)
		{
			assert(logicSchema);
			assert(m_afbCollection);

			m_log->errINT1000(QString(__FUNCTION__) + QString(", logicSchema %1, Parser::m_afbCollection %2")
							  .arg(reinterpret_cast<size_t>(logicSchema))
							  .arg(reinterpret_cast<size_t>(m_afbCollection)));
			return false;
		}

		bool ok = true;

		for (std::shared_ptr<VFrame30::SchemaLayer> l : logicSchema->Layers)
		{
			if (l->compile() == true)
			{
				for (std::shared_ptr<VFrame30::SchemaItem> si : l->Items)
				{
					if (dynamic_cast<VFrame30::SchemaItemAfb*>(si.get()) != nullptr)
					{
						VFrame30::SchemaItemAfb* afbItem = dynamic_cast<VFrame30::SchemaItemAfb*>(si.get());

						std::shared_ptr<Afb::AfbElement> afbDescription = m_afbCollection->get(afbItem->afbStrID());

						if (afbDescription.get() == nullptr)
						{
							// AFB description '%1' is not found for schema item '%2' (Logic Schema '%3').
							//
							m_log->errALP4007(logicSchema->schemaID(), afbItem->buildName(), afbItem->afbStrID(), si->guid());

							ok = false;
							continue;
						}

						if (afbDescription->version() != afbItem->afbElement().version())
						{
							m_log->errALP4008(logicSchema->schemaID(),
											  afbItem->buildName(),
											  afbItem->afbElement().version(),
											  afbDescription->version(),
											  si->guid());

							ok = false;
							continue;
						}
					}
				}

				// We can parse only one layer
				//
				break;
			}
		}

		return ok;
	}

	bool Parser::parseAppLogicSchema(std::shared_ptr<VFrame30::LogicSchema> logicSchema)
	{
		if (logicSchema.get() == nullptr)
		{
			assert(false);
			return false;
		}

		// Find layer for compilation
		//
		bool layerFound = false;
		bool ok = false;

		for (std::shared_ptr<VFrame30::SchemaLayer> l : logicSchema->Layers)
		{
			if (l->compile() == true)
			{
				layerFound = true;
				ok = parseAppLogicLayer(logicSchema, l);

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
			// Schema does not have Logic layer (Logic Schema '%1').
			//
			m_log->errALP4022(logicSchema->schemaID());
			return false;
		}

		return true;
	}

	bool Parser::parseAppLogicLayer(
		std::shared_ptr<VFrame30::LogicSchema> logicSchema,
		std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		if (logicSchema == nullptr || layer == nullptr)
		{
			assert(logicSchema);
			assert(layer);
			return false;
		}

		QStringList equipmentIds = logicSchema->equipmentIdList();

		QByteArray layerData;
		layer->Save(layerData);

		// Parse layer for each LM
		//
		for (QString equipmentId : equipmentIds)
		{
			std::shared_ptr<VFrame30::SchemaLayer> moduleLayer(VFrame30::SchemaLayer::Create(layerData));

			if (moduleLayer.get() == nullptr)
			{
				assert(moduleLayer);
				m_log->errINT1001("Parser::parseAppLogicLayer, can't restore layer, assert(moduleLayer)");
				continue;
			}

			// Find all branches - connected links
			//
			BushContainer bushContainer;

			if (logicSchema->isMultichannelSchema() == true)
			{
				// Multischema checks. All multichannel signals must be transformed to singlechannel here
				//
				multichannelProcessing(logicSchema, moduleLayer, equipmentId);
			}

			bool result = findBushes(logicSchema, moduleLayer, &bushContainer);

			if (result == false)
			{
				LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined, tr("Finding bushes error."));
				return false;
			}

			//bushContainer.debugInfo();

			// Set pins' guids to bushes
			// All log errors should be reported in setBranchConnectionToPin
			//
			setBranchConnectionToPin(logicSchema, moduleLayer, &bushContainer);

			// Associates input/outputs
			//
			setPinConnections(logicSchema, moduleLayer, &bushContainer);

			// Filter singlechannel logic branches in multischema drawing
			//
			if (logicSchema->isMultichannelSchema() == true)
			{
				filterSingleChannelBranchesInMulischema(logicSchema, equipmentId, &bushContainer);
			}

			// Generate afb list, and set it to some container
			//
			applicationData()->addData(equipmentId, bushContainer, logicSchema, m_log);
		}

		return true;
	}

	bool Parser::multichannelProcessing(std::shared_ptr<VFrame30::LogicSchema> schema,
										std::shared_ptr<VFrame30::SchemaLayer> layer,
										QString equipmentId)
	{
		if (schema == nullptr ||
			layer == nullptr)
		{
			assert(schema);
			assert(layer);

			m_log->errINT1000(QString(__FUNCTION__) + QString(", schema %1, layer %2.")
							  .arg(reinterpret_cast<size_t>(schema.get()))
							  .arg(reinterpret_cast<size_t>(layer.get())));

			return false;
		}

		if (equipmentId.isEmpty() == true)
		{
			m_log->errALP4001(schema->schemaID());
			return false;
		}

		if (schema->isMultichannelSchema() == false)
		{
			assert(schema->isMultichannelSchema() == true);
			return true;
		}

		int signalIndexInBlocks = schema->equipmentIdList().indexOf(equipmentId);

		if (signalIndexInBlocks == -1)
		{
			// "this" AppLogicModule has LM's equipmentID  but Schema's equipmentIdList does not have any.
			// How did we end up here?
			//
			assert(signalIndexInBlocks != -1);
			m_log->errINT1001(QString("AppLogicModule::AppLogicModule(%1) signalIndexInBlocks == -1").arg(schema->schemaID()));
			return false;
		}


		for (std::shared_ptr<VFrame30::SchemaItem> item : layer->Items)
		{
			VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());

			if (signalItem == nullptr)
			{
				continue;
			}

			QStringList appSignalIds = signalItem->appSignalIdList();

			if (appSignalIds.size() == 1)
			{
				continue;
			}

			if (appSignalIds.size() == schema->channelCount())
			{
				// Get correct SignalID
				//
				const QStringList& signalIds = signalItem->appSignalIdList();

				QString signalId = signalIds[signalIndexInBlocks];

				signalItem->setAppSignalIds(signalId);

				continue;
			}
			else
			{
				// it's not singlechannel Signal and not enough or more then channel count
				//
				m_log->errALP4031(schema->schemaID(), signalItem->buildName(), signalItem->guid());
				continue;
			}

			assert(false);
		}

		return true;
	}

	bool Parser::filterSingleChannelBranchesInMulischema(std::shared_ptr<VFrame30::LogicSchema> schema,
														 QString equipmentId,
														 BushContainer* bushContainer)
	{
		if (bushContainer == nullptr)
		{
			assert(bushContainer);
			return false;
		}

		std::list<Bush> bushes(bushContainer->bushes.begin(), bushContainer->bushes.end());
		bushContainer->bushes.clear();

		while (bushes.empty() == false)
		{
			std::list<Bush> bushAcc;			// Whole jonedBranch Accumulator

			Bush startBush = bushes.front();
			bushAcc.push_back(startBush);		// Move front of bushes to the accumulator

			bushes.pop_front();					// Remove this bush from bushes

			// Get all bushes related to this bushAcc
			//
			auto accItertaor = bushAcc.begin();		// Just added one iterato, so it cannot be empty
			do
			{
				if (bushes.empty() == true)
				{
					// No more bushes to find
					//
					break;
				}

				auto findIt = bushes.begin();		// bushes is not empty!!!
				do
				{
					if (accItertaor->hasCommonFbls(*findIt) == true)
					{
						bushAcc.push_back(*findIt);

						findIt = bushes.erase(findIt);	// findIt points to the next item

						if (findIt == bushes.end())
						{
							break;
						}

						continue;		// findIt already incremente
					}
				}
				while (++findIt != bushes.end());
			}
			while (++accItertaor != bushAcc.end());

			// if bushAcc has all this channel signals, then add it
			//
			bool allSignalsFromThisChannel = true;
			QString lmEquipmnetId;
			for (const Bush& b : bushAcc)
			{
				for (std::pair<QUuid,std::shared_ptr<VFrame30::FblItemRect>> fbl : b.fblItems)
				{
					if (fbl.second->isSignalElement() == true)
					{
						const VFrame30::SchemaItemSignal* signalElement = fbl.second->toSignalElement();
						assert(signalElement);

						assert(signalElement->multiChannel() == false);

						QString appSignalId = signalElement->appSignalIds();

						// Check if appSignalid from this channel
						//
						Signal* signal = m_signalSet->getSignal(appSignalId);

						if (signal == nullptr ||
							signal->lm().get() == nullptr ||
							signal->lm()->equipmentId() != equipmentId)
						{
							allSignalsFromThisChannel = false;

							if (lmEquipmnetId.isEmpty() == true)
							{
								lmEquipmnetId = signal->lm()->equipmentId();
							}
							else
							{
								if (lmEquipmnetId != signal->lm()->equipmentId())
								{
									m_log->errALP4033(schema->schemaID(), appSignalId, signalElement->guid());
								}
							}
						}
					}
				}
			}

			if (allSignalsFromThisChannel == true)
			{
				// Add this bushAcc ti result bush container
				//
				for (const Bush& ac : bushAcc)
				{
					bushContainer->bushes.push_back(ac);
				}
			}
		}

		return true;
	}


	// Function connects all links, and compose them into bushes
	//
	bool Parser::findBushes(
		std::shared_ptr<VFrame30::LogicSchema> logicSchema,
		std::shared_ptr<VFrame30::SchemaLayer> layer,
		BushContainer* bushContainer) const
	{
		if (logicSchema.get() == nullptr ||
			layer.get() == nullptr ||
			bushContainer == nullptr)
		{
			assert(logicSchema);
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
				fblItem->SetConnectionsPos(logicSchema->gridSize(), logicSchema->pinGridStep());	// Calculate pins positions

				const std::list<VFrame30::AfbPin>& inputs = fblItem->inputs();
				const std::list<VFrame30::AfbPin>& outputs = fblItem->outputs();

				for (const VFrame30::AfbPin& pt : inputs)
				{
					std::shared_ptr<VFrame30::SchemaItemLink> fakeLink = std::make_shared<VFrame30::SchemaItemLink>(fblItem->itemUnit());

					fakeLink->setGuid(Link::getNextId());	// fake links must have the same uuid from build to build (if schema was not changed)

					VFrame30::SchemaPoint pos = pt.point();

					fakeLink->AddPoint(pos.X, pos.Y);
					fakeLink->AddPoint(pos.X, pos.Y);

					layer->Items.push_back(fakeLink);
				}

				for (const VFrame30::AfbPin& pt : outputs)
				{
					std::shared_ptr<VFrame30::SchemaItemLink> fakeLink = std::make_shared<VFrame30::SchemaItemLink>(fblItem->itemUnit());

					fakeLink->setGuid(Link::getNextId());	// fake links must have the same uuid from build to build (if schema was not changed)

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
			VFrame30::SchemaItemLink* link = dynamic_cast<VFrame30::SchemaItemLink*>(item->get());

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
			VFrame30::SchemaItemLink* link = dynamic_cast<VFrame30::SchemaItemLink*>(item->get());

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
			std::list<QUuid> itemsUnderFrontPoint = horzVertLinks.getSchemaItemsUnderPoint(pointList.front(), link->guid());
			std::list<QUuid> itemsUnderBackPoint = horzVertLinks.getSchemaItemsUnderPoint(pointList.back(), link->guid());

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
				// Get SchemaItemLink by this id,
				// save it's and points to newBranch
				//
				std::shared_ptr<VFrame30::SchemaItem> schemaItem = layer->getItemById(id);
				VFrame30::SchemaItemLink* link = dynamic_cast<VFrame30::SchemaItemLink*>(schemaItem.get());

				if (schemaItem == nullptr ||
					link == nullptr)
				{
					assert(schemaItem);
					assert(link);

					LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined, tr("%1 Internal error, expected VFrame30::SchemaItemLink").arg(__FUNCTION__));
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

	bool Parser::setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicSchema> schema, std::shared_ptr<VFrame30::SchemaLayer> layer,
						BushContainer* bushContainer) const
	{
		if (schema.get() == nullptr ||
			layer.get() == nullptr ||
			bushContainer == nullptr)
		{
			assert(schema);
			assert(layer);
			assert(bushContainer);

			log()->errINT1000(QString(__FUNCTION__) + QString(", schema %1, layer %2, bushContainer %3")
							  .arg(reinterpret_cast<size_t>(schema.get()))
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
				// SchemaItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblItem->ClearAssociatedConnections();
				fblItem->SetConnectionsPos(schema->gridSize(), schema->pinGridStep());

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
						log()->errALP4006(schema->schemaID(), fblItem->buildName(), in.caption(), item->guid());
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
						log()->errALP4006(schema->schemaID(), fblItem->buildName(), out.caption(), item->guid());

						result = false;
						continue;
					}

					// Branch was found for current pin
					//

					if (bushContainer->bushes[branchIndex].outputPin.isNull() == false)
					{
						// Branch has multiple outputs.
						//
						log()->errALP4000(schema->schemaID(), bushContainer->bushes[branchIndex].getAllUuid());

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
		std::shared_ptr<VFrame30::LogicSchema> schema,
		std::shared_ptr<VFrame30::SchemaLayer> layer,
		BushContainer* bushContainer)
	{
		if (schema.get() == nullptr ||
			layer.get() == nullptr ||
			bushContainer == nullptr)
		{
			assert(schema);
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

				// SchemaItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblElement->ClearAssociatedConnections();
				fblElement->SetConnectionsPos(schema->gridSize(), schema->pinGridStep());

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
								  tr("LogicSchema %1: Internalerror in function, branch suppose to be found, %2.")
								  .arg(schema->caption())
								  .arg(__FUNCTION__));

						result = false;
						return result;
					}

					// Branch was found for current pin
					//
					Bush& bush = bushContainer->bushes[branchIndex];

					bush.fblItems[fblElement->guid()] = fblElement;

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
								  tr("LogicSchema %1: Internalerror in function, branch suppose to be found, %2.")
								  .arg(schema->caption())
								  .arg(__FUNCTION__));

						result = false;
						return result;
					}				

					// Branch was found for current pin
					//
					Bush& bush = bushContainer->bushes[branchIndex];

					bush.fblItems[fblElement->guid()] = fblElement;

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
			// Logic Schema is empty, there are no any functional blocks in the compile layer (Logic Schema '%1')
			//
			m_log->wrnALP4005(schema->schemaID());
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
				for (auto it = bush.fblItems.begin(); it != bush.fblItems.end(); ++it)
				{
					const std::shared_ptr<VFrame30::FblItemRect>& item = it->second;
					// Schema item %1 has not linked pin %2 (Logic Schema '%3').
					//
					std::vector<VFrame30::AfbPin> inputs = bush.getInputPinsForItem(item->guid());

					QString inputsStr;
					for (auto input : inputs)
					{
						inputsStr += (inputsStr.isEmpty() == true) ? input.caption() : QString(", %1").arg(input.caption());
					}

					std::vector<QUuid> issuedItemsUuid = bush.getLinksUuids();
					issuedItemsUuid.push_back(item->guid());

					m_log->errALP4006(schema->schemaID(), item->buildName(), inputsStr, issuedItemsUuid);
				}
			}

			if (bush.inputPins.empty() == true)
			{
				// Look for item without associated inputs
				//
				for (auto it = bush.fblItems.begin(); it != bush.fblItems.end(); ++it)
				{
					const std::shared_ptr<VFrame30::FblItemRect>& item = it->second;
					const std::list<VFrame30::AfbPin>& outputs = item->outputs();

					for (const VFrame30::AfbPin& out : outputs)
					{
						if (out.associatedIOs().empty() == true)
						{
							// Schema item %1 has not linked pin %2 (Logic Schema '%3').
							//
							std::vector<QUuid> issuedItemsUuid = bush.getLinksUuids();
							issuedItemsUuid.push_back(item->guid());

							m_log->errALP4006(schema->schemaID(), item->buildName(), out.caption(), issuedItemsUuid);
						}
					}
				}
			}
		}

		return result;
	}

	void Parser::debugInfo()
	{
		//LOG_MESSAGE(m_log, "Debug Info:");

		// Set Schema Ityem Run Order for drawing on schemas
		//
		const AppLogicData* appLogicData = applicationData();
		const auto& logicModules = appLogicData->modules();

		for (std::shared_ptr<AppLogicModule> lm : logicModules)
		{
			const std::list<AppLogicItem>& items = lm->items();

			std::map<QUuid, int> schemaItemRunOrder;

			int index = 0;
			for (const AppLogicItem& it : items)
			{
				assert(schemaItemRunOrder.find(it.m_fblItem->guid()) == schemaItemRunOrder.end());

				schemaItemRunOrder[it.m_fblItem->guid()] = index;
				index ++;
			}

			GlobalMessanger::instance()->setRunOrder(lm->moduleEquipmentId(), schemaItemRunOrder);
		}

		return;
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
