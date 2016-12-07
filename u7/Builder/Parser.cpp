#include "Parser.h"

#include <typeindex>
#include <functional>
#include <QtConcurrent/QtConcurrent>

#include "IssueLogger.h"
#include "GlobalMessanger.h"

#include "../../lib/DbController.h"

#include "../../VFrame30/LogicSchema.h"
#include "../../VFrame30/SchemaItemLink.h"
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/SchemaItemAfb.h"
#include "../../VFrame30/SchemaItemSignal.h"
#include "../../VFrame30/SchemaItemConst.h"
#include "../../VFrame30/SchemaItemUfb.h"
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
		for (const auto& item : fblItems)
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

		for (const auto& it : links)
		{
			v.push_back(it.first);
		}

		for (const auto& it : fblItems)
		{
			v.push_back(it.first);
		}

		return v;
	}

	std::vector<QUuid> Bush::getLinksUuids() const
	{
		std::vector<QUuid> v;
		v.reserve(links.size());

		for (const auto& it : links)
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

		const std::vector<VFrame30::AfbPin>& itemInputs = item->inputs();

		result.reserve(itemInputs.size());

		for (const auto& id : inputPins)
		{
			auto foundPin = std::find_if(itemInputs.begin(), itemInputs.end(),
				[&id](const VFrame30::AfbPin& itemInput)
				{
					return itemInput.guid() == id;
				});

			if (foundPin != itemInputs.end())
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
		size_t bushesSize = bushes.size();
		for (size_t i = 0; i < bushesSize; ++i)
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
							   std::shared_ptr<VFrame30::Schema> schema) :
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
	AppLogicModule::AppLogicModule(QString moduleId) :
		m_equipmentId(moduleId)
	{
	}

	bool AppLogicModule::addBranch(std::shared_ptr<VFrame30::Schema> schema,
			const BushContainer& bushes,
			IssueLogger* log)
	{
		if (schema == nullptr ||
			log == nullptr)
		{
			assert(schema);
			assert(log);

			if (log != nullptr)
			{
				log->errINT1000(QString(__FUNCTION__) + QString(", schema %1, log %2.")
								.arg(reinterpret_cast<size_t>(schema.get()))
								.arg(reinterpret_cast<size_t>(log)));
			}

			return false;
		}

		// Create a copy of bushcontainer, as in case of processing multisignlas in multichannel schemas
		// these multisignals will be created for all channels
		//
		BushContainer bushContainer = bushes;

		if (bushContainer.bushes.empty() == true)
		{
			//LOG_WARNING_OBSOLETE(log, Builder::IssueType::NotDefined, QObject::tr("Logic schema does no contains any correct links."));
			assert(false);	// if fires then add error to IussueLogger
			return true;
		}

		// Save all items to accumulator, they will be ordered in orderItems()
		//
		for (const Bush& bush : bushContainer.bushes)
		{
			for (auto it = bush.fblItems.begin(); it != bush.fblItems.end(); ++it)
			{
				const std::shared_ptr<VFrame30::FblItemRect>& f = it->second;

				if (f->isSignalElement() == true && f->toSignalElement()->multiChannel() == true)
				{
					log->errALP4030(schema->schemaId(), f->buildName(), f->guid());
				}

				AppLogicItem li{f, schema};

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

	bool AppLogicModule::orderItems(IssueLogger* log, bool* interruptProcess)
	{
		if (log == nullptr ||
			interruptProcess == nullptr)
		{
			assert(log);
			assert(interruptProcess);
			return false;
		}

		qDebug() << "Order items for module " << m_equipmentId;

		LOG_MESSAGE(log, QObject::tr("Started OrderItems for module: %1").arg(equipmentId()));

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

		for (const auto& item : fblItems)
		{
			if (item.second.m_fblItem->inputsCount() == 0)
			{
				orderedList.push_front(item.second);	// items without inputs must be at the begining of the list
				hasItemsWithouInputs = true;
				continue;
			}
		}

		for (const auto& item : fblItems)
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
			if (*interruptProcess == true)
			{
				break;
			}

			qDebug() << "Pass " << pass++;

			setItemsOrder(log, fblItems, orderedList, constFblItems, interruptProcess);

			if (*interruptProcess == true)
			{
				break;
			}


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

		if (*interruptProcess == true)
		{
			return false;
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

		if (result == true)
		{
			QString str = QString("Finished OrderItems for module %1, %2 functional item(s) were parsed")
						  .arg(equipmentId())
						  .arg(m_items.size());

			LOG_MESSAGE(log, str);
		}

		return result;
	}

	std::shared_ptr<AppLogicModule> AppLogicModule::deepCopy(QUuid groupId, const QString& label) const
	{
		// Params:
		//	groupId: is used for marking all UFB items as a single group, for AppLogic it can be null
		//	label: is used to make new labels for UFB
		//

		// Make a copy of ordered items
		// pin guids will be changed in this copy
		//
		std::list<AppLogicItem> itemsCopy;

		for (const AppLogicItem& ali : m_items)
		{
			QByteArray buffer;
			buffer.reserve(1024);

			ali.m_fblItem->Save(buffer);

			AppLogicItem aliCopy = ali;
			aliCopy.m_fblItem = std::dynamic_pointer_cast<VFrame30::FblItemRect>(VFrame30::SchemaItem::Create(buffer));
			assert(aliCopy.m_fblItem);

			aliCopy.m_groupId = groupId;

			itemsCopy.push_back(aliCopy);
		}

		// Set new inputs/ouputs guids to the copied items
		// Keep connections to in<->out (Associated IOs)
		//
		std::map<QUuid, QUuid> oldToNewPins;			// key is old, value in new QUuid of the pin

		// Set new values to all input and output pins, save changes in oldNewPinGuids
		// from oldNewPinGuids assocIOs will be restored
		//
		for (AppLogicItem& ali : itemsCopy)
		{
			assert(ali.m_fblItem);

			// Set new item guid
			//
			ali.m_fblItem->setGuid(QUuid::createUuid());

			// Set new label
			//
			if (label.isEmpty() == false)
			{
				ali.m_fblItem->setLabel(label + "_" + ali.m_fblItem->label());
			}

			// Set new guids to in/outs
			//
			for (VFrame30::AfbPin& pin : ali.m_fblItem->inputs())
			{
				QUuid newPinGuid = QUuid::createUuid();
				oldToNewPins[pin.guid()] = newPinGuid;

				pin.setGuid(newPinGuid);
			}

			for (VFrame30::AfbPin& pin : ali.m_fblItem->outputs())
			{
				QUuid newPinGuid = QUuid::createUuid();
				oldToNewPins[pin.guid()] = newPinGuid;

				pin.setGuid(newPinGuid);
			}
		}

		// Restore associatedIOs after assigning new pin guids
		//
		for (AppLogicItem& ali : itemsCopy)			// set new pin guid to associatedIOs
		{
			assert(ali.m_fblItem);

			for (VFrame30::AfbPin& pin : ali.m_fblItem->inputs())
			{
				for (QUuid& assocIO : pin.associatedIOs())
				{
					auto newGuidIt = oldToNewPins.find(assocIO);

					if (newGuidIt == oldToNewPins.end())
					{
						assert(newGuidIt != oldToNewPins.end());
						return nullptr;
					}

					assocIO = newGuidIt->second;
				}
			}

			for (VFrame30::AfbPin& pin : ali.m_fblItem->outputs())
			{
				for (QUuid& assocIO : pin.associatedIOs())
				{
					auto newGuidIt = oldToNewPins.find(assocIO);

					if (newGuidIt == oldToNewPins.end())
					{
						assert(newGuidIt != oldToNewPins.end());
						return nullptr;
					}

					assocIO = newGuidIt->second;
				}
			}
		}

		std::shared_ptr<AppLogicModule> copy = std::make_shared<AppLogicModule>(this->m_equipmentId);

		copy->m_items = itemsCopy;

		return copy;
	}

	bool AppLogicModule::debugCheckItemsRelationsConsistency(IssueLogger* log) const
	{
		return AppLogicModule::debugCheckItemsRelationsConsistency(m_equipmentId, m_items, log);
	}

	bool AppLogicModule::debugCheckItemsRelationsConsistency(QString equipmentId,
															 const std::list<AppLogicItem>& items,
															 IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		try
		{
			std::map<QUuid, AppLogicItem> itemMap;
			std::map<QUuid, std::pair<AppLogicItem, VFrame30::AfbPin>> pins;

			for (const AppLogicItem& ali : items)
			{
				assert(ali.m_fblItem);

				if (itemMap.count(ali.m_fblItem->guid()) != 0)
				{
					throw QString("Two or more items have duplicate guids. item %1, item.guid %2")
							.arg(ali.m_fblItem->label())
							.arg(ali.m_fblItem->guid().toString());
				}

				for (const VFrame30::AfbPin& pin : ali.m_fblItem->inputs())
				{
					if (pin.IsInput() == false)
					{
						throw QString("Pin is in iutput list but has an output type. item %1, pin.caption %2")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption());
					}

					if (pins.count(pin.guid()) != 0)
					{
						throw QString("Duplicate pin guids. item %1, pin.caption %2, pin.guid %3")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption())
								.arg(pin.guid().toString());
					}

					pins[pin.guid()] = std::make_pair(ali, pin);
				}

				for (const VFrame30::AfbPin& pin : ali.m_fblItem->outputs())
				{
					if (pin.IsOutput() == false)
					{
						throw QString("Pin is in output list but has an input type. item %1, pin.caption %2")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption());
					}

					if (pins.count(pin.guid()) != 0)
					{
						throw QString("Duplicate pin guids. item %1, pin.caption %2, pin.guid %3")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption())
								.arg(pin.guid().toString());
					}

					pins[pin.guid()] = std::make_pair(ali, pin);
				}
			}

			// Check associated IOs
			//
			for (const std::pair<QUuid, std::pair<AppLogicItem, VFrame30::AfbPin>>& pinPair : pins)
			{
				const AppLogicItem& ali = pinPair.second.first;
				const VFrame30::AfbPin& pin = pinPair.second.second;

				if (pin.IsInput() == true)
				{
					if (pin.associatedIOs().size() != 1)
					{
						throw QString("Input pin does not have associated output. item %1, pin.caption %2")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption());
					}

					QUuid associatedOutput = pin.associatedIOs().front();

					auto associatedPinIt = pins.find(associatedOutput);
					if (associatedPinIt == pins.end())
					{
						throw QString("Input pin has an associated output which cannot be found. item %1, pin.caption %2, not found assocIO %3")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption())
								.arg(associatedOutput.toString());
					}

					// Reverse check, found ouput pin must have associated current input pin
					//
					const VFrame30::AfbPin& associatedOutputPin = associatedPinIt->second.second;

					if (associatedOutputPin.IsOutput() == false)
					{
						throw QString("Input pin has an associated pin which is expected to be an output but it is an input. item %1, pin.caption %2, assocPin.caption %3, assocPin.guid %4")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption())
								.arg(associatedOutputPin.caption())
								.arg(associatedOutputPin.guid().toString());
					}

					auto foundInputAssocIt = std::find(associatedOutputPin.associatedIOs().begin(),
													   associatedOutputPin.associatedIOs().end(),
													   pin.guid());

					if (foundInputAssocIt == associatedOutputPin.associatedIOs().end())
					{
						throw QString("There is no back association for input. Input has valid output which also must have initial input but it does not."
									  "item %1, pin.caption %2, assocPin.caption %3, assocPin.guid %4")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption())
								.arg(associatedOutputPin.caption())
								.arg(associatedOutputPin.guid().toString());
					}

					continue;
				}

				if (pin.IsOutput() == true)
				{
					if (pin.associatedIOs().empty() == true)
					{
						throw QString("Ouput pin does not have associated inputs. item %1, pin.caption %2")
								.arg(ali.m_fblItem->label())
								.arg(pin.caption());
					}

					const std::vector<QUuid>& assocInputs = pin.associatedIOs();
					for (const QUuid& assocInput : assocInputs)
					{
						auto associatedPinIt = pins.find(assocInput);
						if (associatedPinIt == pins.end())
						{
							throw QString("Output pin has an associated input which cannot be found. item %1, pin.caption %2, not found assocIO %3")
									.arg(ali.m_fblItem->label())
									.arg(pin.caption())
									.arg(assocInput.toString());
						}

						const VFrame30::AfbPin& assocInputPin = associatedPinIt->second.second;
						assert(assocInputPin.guid() == assocInput);

						if (assocInputPin.IsInput() == false)
						{
							throw QString("Output pin has an associated pin which is expected to be an intput but it is an output. item %1, pin.caption %2, assocPin.caption %3, assocPin.guid %4")
									.arg(ali.m_fblItem->label())
									.arg(pin.caption())
									.arg(assocInputPin.caption())
									.arg(assocInputPin.guid().toString());
						}

						// Check for the back association
						//
						if (assocInputPin.associatedIOs().size() != 1)
						{
							throw QString("Output pin has an associated input pin which is expected to have one assoc IO (back to output)."
										  "item %1, pin.caption %2, assocPin.caption %3, assocPin.guid %4")
									.arg(ali.m_fblItem->label())
									.arg(pin.caption())
									.arg(assocInputPin.caption())
									.arg(assocInputPin.guid().toString());
						}

						if (assocInputPin.associatedIOs().front() != pin.guid())
						{
							throw QString("There is no back association for output. Output has valid assoc input which also must have initial output but it does not."
										  "item %1, pin.caption %2, assocPin.caption %3, assocPin.guid %4")
									.arg(ali.m_fblItem->label())
									.arg(pin.caption())
									.arg(assocInputPin.caption())
									.arg(assocInputPin.guid().toString());
						}
					}

					continue;
				}

				throw QString("pin is neither inpur nor output. item %1, pin.caption %2")
						.arg(ali.m_fblItem->label())
						.arg(pin.caption());
			}
		}
		catch (QString message)
		{
			QString errorMessage = QString("Please, report to developers: Checking items relations consistency error: module %1, error message %2")
								   .arg(equipmentId)
								   .arg(message);

			qDebug() << "-------------------------- Checking items relations consistency error dump----------------------------------";
			qDebug() << errorMessage;

			for (const AppLogicItem& ali : items)
			{
				ali.m_fblItem->dump();
			}
			qDebug() << "------------------- END OF Checking items relations consistency error dump ---------------------------------";

			log->errINT1001(errorMessage);
			return false;
		}

		return true;
	}

	bool AppLogicModule::removeInOutItemKeepAssoc(const QUuid& itemGuid)
	{
		// Find item iteself
		//
		auto itemIt = std::find_if(m_items.begin(), m_items.end(),
								   [&itemGuid](const AppLogicItem& ali)
								   {
										return ali.m_fblItem->guid() == itemGuid;
								   });

		if (itemIt == m_items.end())
		{
			// Item for removing is not found
			//
			assert(itemIt != m_items.end());
			return false;
		}

		const AppLogicItem& item = *itemIt;

		if (item.m_fblItem->isInOutSignalElement() == false ||
			item.m_fblItem->inputsCount() != 1 ||
			item.m_fblItem->outputsCount() != 1)
		{
			assert(item.m_fblItem->isInOutSignalElement() == true);
			assert(item.m_fblItem->inputsCount() == 1);
			assert(item.m_fblItem->outputsCount() == 1);
			return false;
		}

		// --
		//
		VFrame30::AfbPin& in = item.m_fblItem->inputs().front();
		VFrame30::AfbPin& out = item.m_fblItem->outputs().front();

		// Find sourceItem
		//
		if (in.associatedIOs().size() != 1)
		{
			assert(in.associatedIOs().size() == 1);
			return false;
		}

		const QUuid& sourceOutputUuid = in.associatedIOs().front();
		auto sourceItemIt = std::find_if(m_items.begin(), m_items.end(),
								   [&sourceOutputUuid](const AppLogicItem& ali)
								   {
										return ali.m_fblItem->hasOutput(sourceOutputUuid);
								   });
		if (sourceItemIt == m_items.end())
		{
			// Schema is not consistent?
			//
			assert(sourceItemIt != m_items.end());
			return false;
		}

		AppLogicItem& sourceItem = *sourceItemIt;
		VFrame30::AfbPin& sourceItemOutPin = sourceItem.m_fblItem->output(sourceOutputUuid);

		// Find all target items
		//
		if (out.associatedIOs().empty() == true)
		{
			assert(out.associatedIOs().empty() == false);
			return false;
		}

		for (const QUuid& targetInputUuid : out.associatedIOs())
		{
			auto targetItemIt = std::find_if(m_items.begin(), m_items.end(),
									   [&targetInputUuid](const AppLogicItem& ali)
									   {
											return ali.m_fblItem->hasInput(targetInputUuid);
									   });
			if (targetItemIt == m_items.end())
			{
				continue;
			}

			AppLogicItem& targetItem = *targetItemIt;
			VFrame30::AfbPin& targetItemInputPin = targetItem.m_fblItem->input(targetInputUuid);

			AppLogicData::bindTwoPins(sourceItemOutPin, targetItemInputPin);
			targetItemInputPin.removeFromAssociatedIo(out.guid());
		}

		sourceItemOutPin.removeFromAssociatedIo(in.guid());

		// Remove item from the item list
		//
		m_items.erase(itemIt);

		return true;
	}

	void AppLogicModule::dump() const
	{
		qDebug() << "-------------------------- AppLogicModule::dump----------------------------------";
		qDebug() << "equipmentId()" << equipmentId();

		for (const AppLogicItem& ali : m_items)
		{
			ali.m_fblItem->dump();
		}

		qDebug() << "------------------------ END OF AppLogicModule::dump -----------------------------";
	}

	bool AppLogicModule::setItemsOrder(IssueLogger* log,
									   std::map<QUuid, AppLogicItem>& remainItems,
									   std::list<AppLogicItem>& orderedItems,
									   const std::map<QUuid, AppLogicItem>& constItems,
									   bool* interruptProcess)
	{
		if (log == nullptr ||
			interruptProcess == nullptr)
		{
			assert(log);
			assert(interruptProcess);
			return false;
		}

		// --
		//
		std::map<QUuid, std::vector<AppLogicItem>> itemsWithInputs;		// Key is QUuid of output
		auto constItemsBegin = constItems.begin();
		auto constItemsEnd = constItems.end();

		for (const std::pair<QUuid, AppLogicItem>& currentItem : constItems)
		{
			const std::vector<VFrame30::AfbPin>& outputs = currentItem.second.m_fblItem->outputs();

			for (const VFrame30::AfbPin& out : outputs)
			{
				std::vector<AppLogicItem> deps = getItemsWithInput(constItemsBegin, constItemsEnd, out.guid());
				itemsWithInputs[out.guid()] = deps;
			}
		}

		// --
		//
		std::list<ChangeOrder> changeOrderHistory;

		// Set other items
		//
		for (auto currentIt = orderedItems.begin(); currentIt != orderedItems.end(); ++currentIt)
		{
			if (*interruptProcess == true)
			{
				return false;
			}

			AppLogicItem currentItem = *currentIt;		// NOT REFERENCE, ITEM CAN BE MOVED LATER

			//qDebug() << "Parsing -- order item " << currentItem.m_fblItem->buildName();

			// Get dependant items
			//
			std::map<QUuid, AppLogicItem> dependantItems;

			const std::vector<VFrame30::AfbPin>& outputs = currentItem.m_fblItem->outputs();

			for (const VFrame30::AfbPin& out : outputs)
			{
				auto foundDepIterator = itemsWithInputs.find(out.guid());
				if (foundDepIterator == itemsWithInputs.end())
				{
					assert(foundDepIterator != itemsWithInputs.end());
					log->errINT1001("Output was not fount in itemsWithInputs map, assert(foundDepIterator != itemsWithInputs.end())");
					continue;
				}

				const std::vector<AppLogicItem>& deps = foundDepIterator->second;

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

		for (const auto& lipair : m_fblItemsAcc)
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

					log->errALP4021(equipmentId(),
									li.m_schema->schemaId(),
									duplicateItem->m_schema->schemaId(),
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
	std::vector<AppLogicItem> AppLogicModule::getItemsWithInput(
			const Iter& begin,
			const Iter& end,
			const QUuid& guid)
	{
		std::map<QUuid, AppLogicItem> result;	// set removes duplicats

		for (auto it = begin; it != end; ++it)
		{
			const AppLogicItem& item = it->second;
			const std::vector<VFrame30::AfbPin>& inputs = item.m_fblItem->inputs();

			//for (const VFrame30::AfbPin& in : inputs)
			size_t inputCount = inputs.size();
			for (size_t inputIndex = 0; inputIndex < inputCount; ++inputIndex)
			{
				const VFrame30::AfbPin& in = inputs[inputIndex];

				const std::vector<QUuid>& associatedOutputs = in.associatedIOs();

				// !!!
				// This find is very slow in debug mode, so it was changed to for with pointer
				//
//				auto associatedOutputsBegin = associatedOutputs.begin();
//				auto associatedOutputsEnd = associatedOutputs.end();
//
//				auto foundAssociated = std::find(associatedOutputsBegin, associatedOutputsEnd, guid);
//
//				if (foundAssociated != associatedOutputsEnd)
//				{
//					result[item.m_fblItem->guid()] = item;
//					break;
//				}

				// Low level optimization instead of std::find(associatedOutputsBegin, associatedOutputsEnd, guid);
				//
				bool found = false;

				const QUuid* assocPtr = associatedOutputs.data();
				size_t associatedOutputsSize = associatedOutputs.size();

				for (size_t ait = 0; ait < associatedOutputsSize; ++ait, ++assocPtr)
				//for (const QUuid& a : associatedOutputs)
				{
					if (*assocPtr == guid)
					{
						result[item.m_fblItem->guid()] = item;
						found = true;
						break;
					}
				}

				if (found == true)
				{
					break;
				}
			}
		}

		std::vector<AppLogicItem> resultVector;
		resultVector.reserve(8);

		for (const auto& item : result)
		{
			resultVector.push_back(item.second);
		}

		return resultVector;
	}

	QString AppLogicModule::equipmentId() const
	{
		return m_equipmentId;
	}

	void AppLogicModule::setEquipmentId(QString value)
	{
		m_equipmentId = value;
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


	bool AppLogicData::addLogicModuleData(QString equipmentId,
										  const BushContainer& bushContainer,
										  std::shared_ptr<VFrame30::LogicSchema> schema,
										  IssueLogger* log)
	{
		if (equipmentId.isEmpty() == true)
		{
			log->errALP4001(schema->schemaId());
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
			return m->equipmentId() == equipmentId;
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

	bool AppLogicData::addUfbData(const BushContainer& bushContainer,
								  std::shared_ptr<VFrame30::UfbSchema> schema,
								  IssueLogger* log)
	{
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

		// Get module (m_ufb), if it is not in the list, add it
		//
		bool result = true;
		auto moduleIt = std::find_if(m_ufbs.begin(), m_ufbs.end(),
									[&schema](const std::pair<QString, std::shared_ptr<AppLogicModule>>& m)
									{
										return m.second->equipmentId() == schema->schemaId();
									});

		std::shared_ptr<AppLogicModule> module;

		if (moduleIt == m_ufbs.end())
		{
			// Module was not found, addit
			//
			module = std::make_shared<AppLogicModule>(schema->schemaId());
			m_ufbs[schema->schemaId()] = module;
		}
		else
		{
			module = moduleIt->second;
		}

		assert(module);

		// add new branch to module
		//
		result &= module->addBranch(schema, bushContainer, log);

		return result;
	}

	bool AppLogicData::orderLogicModuleItems(IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(nullptr);
			return false;
		}

		bool result = true;

		std::vector<QFuture<bool>> orderTasks;
		orderTasks.reserve(m_modules.size());

		bool iterruptRequest = false;

		for (std::shared_ptr<AppLogicModule> m : m_modules)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			QFuture<bool> task =  QtConcurrent::run(std::bind(&AppLogicModule::orderItems, m, log, &iterruptRequest));
			orderTasks.push_back(task);
		}

		// Wait for finish and process interrupt request
		//
		do
		{
			bool allFinished = true;
			for (QFuture<bool>& task : orderTasks)
			{
				QThread::yieldCurrentThread();
				if (task.isRunning() == true)
				{
					allFinished = false;
					break;
				}
			}

			if (allFinished == true)
			{
				break;
			}
			else
			{
				// Set iterruptRequest, so work threads can get it and exit
				//
				iterruptRequest = QThread::currentThread()->isInterruptionRequested();
				QThread::yieldCurrentThread();
			}
		}
		while (1);

		for (QFuture<bool>& task : orderTasks)
		{
			result &= task.result();
		}

		return result;
	}

	bool AppLogicData::orderUfbItems(IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(nullptr);
			return false;
		}

		bool result = true;
		bool iterruptRequest = false;

		for (std::pair<QString, std::shared_ptr<AppLogicModule>> ufbModule : m_ufbs)
		{
			iterruptRequest = QThread::currentThread()->isInterruptionRequested();
			if (iterruptRequest == true)
			{
				return false;
			}

			ufbModule.second->orderItems(log, &iterruptRequest);
		}

		return result;
	}

	bool AppLogicData::expandUfbs(IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(nullptr);
			return false;
		}

		bool result = true;
		bool iterruptRequest = false;

		for (std::shared_ptr<AppLogicModule> module : m_modules)
		{
			iterruptRequest = QThread::currentThread()->isInterruptionRequested();
			if (iterruptRequest == true)
			{
				return false;
			}

			bool checkResult = module->debugCheckItemsRelationsConsistency(log);
			if (checkResult == false)
			{
				return false;
			}

			// Fake Items are used in special cases:
			// 1. UFB has direct connect from input to output
			//    [in_1]-+-----------------+-[out_1]
			// 2. SchemaItemUfb has loop withot any items
			//        +-[UFB]-+
			//        |       |
			//        +-------+
			// In these cases FakeItems (SchemaItemInOut) are added to these connections
			// later these fake items must be removed and their connections must be rebind
			//
			//
			std::vector<std::shared_ptr<VFrame30::SchemaItem>> fakeItems;
			fakeItems.reserve(128);

			// Find VFrame30::SchemaItemUfb and insert AFTER them actual ufbs
			//
			for (auto itemIt = module->items().begin(); itemIt != module->items().end(); ++itemIt)
			{
				const AppLogicItem& item = *itemIt;

				if (item.m_fblItem->isType<VFrame30::SchemaItemUfb>() == false)
				{
					continue;
				}

				VFrame30::SchemaItemUfb* ufbItem = item.m_fblItem->toType<VFrame30::SchemaItemUfb>();
				assert(ufbItem);

				std::shared_ptr<AppLogicModule> parsedUfb = ufb(ufbItem->ufbSchemaId());

				if (parsedUfb == nullptr)
				{
					log->errALP4009(item.m_schema->schemaId(), ufbItem->label(), ufbItem->ufbSchemaId(), ufbItem->guid());
					result = false;
					continue;
				}

				// Make a copy of ufb, so all the guid will be unique in
				//
				std::shared_ptr<AppLogicModule> ufbCopy = parsedUfb->deepCopy(ufbItem->guid(), ufbItem->label());
				if (ufbCopy == nullptr)
				{
					assert(ufbCopy);
					continue;
				}

				std::list<AppLogicItem> ufbItemsCopy = ufbCopy->items();		// Prefer to work with a list copy

				// Intermidiate check, to make sure all the new guids were set right and relations were kept in consistency
				//
				checkResult = ufbCopy->debugCheckItemsRelationsConsistency(log);
				if (checkResult == false)
				{
					return false;
				}

//				qDebug() << "================= UFB Copy itmes dump ====================";
//				for (const AppLogicItem& ali : ufbItemsCopy)
//				{
//					ali.m_fblItem->dump();
//				}
//				qDebug() << "============- END OF UFB Copy itmes dump =================";


				// Check for the special case, input is connected to output of the same item
				// SchemaItemUfb has loop withot any items
				//        +-[UFB]-+
				//        |       |
				//        +-------+
				//
				for (VFrame30::AfbPin& ufbInputPin : ufbItem->inputs())
				{
					for (VFrame30::AfbPin& ufbOutputPin : ufbItem->outputs())
					{
						if (ufbOutputPin.hasAssociatedIo(ufbInputPin.guid()) == true)
						{
							assert(ufbInputPin.hasAssociatedIo(ufbOutputPin.guid()));	// Check for back association

							std::shared_ptr<VFrame30::SchemaItemInOut> inOutItem = std::make_shared<VFrame30::SchemaItemInOut>(ufbItem->itemUnit());
							inOutItem->setAppSignalIds(QString("#FAKEITEM_%1_%2_%3")
													   .arg(ufbItem->label())
													   .arg(ufbInputPin.caption())
													   .arg(ufbOutputPin.caption()));

							assert(inOutItem->inputsCount() == 1);
							assert(inOutItem->outputsCount() == 1);

							ufbInputPin.ClearAssociattdIOs();
							ufbOutputPin.removeFromAssociatedIo(ufbInputPin.guid());

							AppLogicData::bindTwoPins(inOutItem->outputs().front(), ufbInputPin);
							AppLogicData::bindTwoPins(ufbOutputPin, inOutItem->inputs().front());

							fakeItems.push_back(inOutItem);

							AppLogicItem fakeAli(inOutItem, item.m_schema);
							module->items().push_back(fakeAli);	// Order is not important here, this item will be removed later
						}
					}
				}

				// !!!!!!!!!!!!!!!!!!!!!!!!!
				module->dump();

				// Bind LogicSchema outputû to UFB items inputs
				//
				for (const VFrame30::AfbPin& ufbItemPin : ufbItem->inputs())
				{
					// [schemaOutputItem]-+------+-[UFB:ufbItem] .... [ufbInputBlock]-+---+---+-[ufbIntItem0]
					//                                                                    |         ...
					//                                                                    +---+-[ufbIntItemN]
					// Bind schemaOutputItem <--> ufbIntItem0...ufbIntItemN
					//

					//qDebug() << ufbItemPin.caption() << " " << ufbItemPin.guid();		// !!!!!!!!!!!!!!!!!!!!!!!!!

					if (ufbItemPin.IsInput() == false ||
						ufbItemPin.associatedIOs().size() != 1)
					{
						assert(ufbItemPin.IsInput() == true);
						assert(ufbItemPin.associatedIOs().size() == 1);
						result = false;
						continue;
					}

					QUuid outputGuid = ufbItemPin.associatedIOs().front();	// This is the real output guid, set it to all
																			// corrsponding inputs in ufb

					//qDebug() << "outputGuid" << " " << outputGuid;	// !!!!!!!!!!!!!!!!!!

					// Find schemaOutputItem
					//
					AppLogicItem schemaOutputItem;

					for (AppLogicItem& ali : module->items())
					{
						bool found = ali.m_fblItem->hasOutput(outputGuid);
						if (found == true)
						{
							schemaOutputItem = ali;
							break;
						}
					}

					if (schemaOutputItem.m_fblItem == nullptr)
					{
						assert(schemaOutputItem.m_fblItem);
						log->errINT1001("Cant find item on schema, which has output.guid == outputGuid");
						continue;
					}

					VFrame30::AfbPin& schemaOutputItemPin = schemaOutputItem.m_fblItem->output(outputGuid);

					// Find ufbInputBlock
					//
					auto foundInputIt = std::find_if(ufbItemsCopy.begin(), ufbItemsCopy.end(),
							[&ufbItemPin](const AppLogicItem& ali)
							{
								return
									ali.m_fblItem != nullptr &&
									ali.m_fblItem->isInputSignalElement() == true &&
									ali.m_fblItem->toInputSignalElement() != nullptr &&
									ali.m_fblItem->toInputSignalElement()->appSignalIds().trimmed() == ufbItemPin.caption();
							});

					if (foundInputIt == ufbItemsCopy.end())
					{
						log->errALP4012(item.m_schema->schemaId(), ufbItem->label(), ufbItemPin.caption(), ufbItem->guid());
						result = false;
						continue;
					}

					AppLogicItem& ufbInputBlock = *foundInputIt;

					if (ufbInputBlock.m_fblItem->outputs().size() != 1)
					{
						assert(ufbInputBlock.m_fblItem->outputs().size() == 1);
						result = false;
						continue;
					}

					//qDebug() << "inputBlock";				// !!!!!!!!!!!!!!!!!!!!!!!!
					//ufbInputBlock.m_fblItem->dump();		// !!!!!!!!!!!!!!!!!!!!!!!!!!

					VFrame30::AfbPin& ufbInputBlockPin = ufbInputBlock.m_fblItem->outputs().front();

					// Bind schemaOutputItem to [ufbIntItem0...ufbIntItemN]
					//

					// Get ufbIntItem0... ufbIntItemN
					// Set pins association
					//
					for (AppLogicItem& ufbItem : ufbItemsCopy)
					{
						std::vector<VFrame30::AfbPin>& ufbItemInputs = ufbItem.m_fblItem->inputs();

						for (VFrame30::AfbPin& ufbItemPin : ufbItemInputs)
						{
							if (ufbItemPin.hasAssociatedIo(ufbInputBlockPin.guid()) == true)
							{
								AppLogicData::bindTwoPins(schemaOutputItemPin, ufbItemPin);
							}
						}
					}

					// Remove from associated ufbItemPin
					//
					schemaOutputItemPin.removeFromAssociatedIo(ufbItemPin.guid());
				}

				// Binding outputs
				//
				for (const VFrame30::AfbPin& ufbItemPin : ufbItem->outputs())
				{
					if (ufbItemPin.IsOutput() == false)
					{
						assert(ufbItemPin.IsOutput() == true);
						continue;
					}

					// [ufbIntItem]-+------+-[ufbOutputBlock] .... [UFB:ufbItem]-+---+---+-[schemaInputItem1]
					//                                                               |         ...
					//                                                               +---+-[schemaInputItemN]

					//qDebug() << "ufbItemPin" << " " << ufbItemPin.guid() << " " << ufbItemPin.caption();		// !!!!!!!!!!!

					// ufbOutputBlock
					//
					auto foundUfbOutputIt = std::find_if(ufbItemsCopy.begin(), ufbItemsCopy.end(),
							[&ufbItemPin](const AppLogicItem& ali)
							{
								return
									ali.m_fblItem != nullptr &&
									ali.m_fblItem->isOutputSignalElement() == true &&
									ali.m_fblItem->toOutputSignalElement() != nullptr &&
									ali.m_fblItem->toOutputSignalElement()->appSignalIds().trimmed() == ufbItemPin.caption();
							});

					if (foundUfbOutputIt == ufbItemsCopy.end())
					{
						log->errALP4012(item.m_schema->schemaId(), ufbItem->label(), ufbItemPin.caption(), ufbItem->guid());
						result = false;
						continue;
					}

					AppLogicItem& ufbOutputBlock = *foundUfbOutputIt;

					if (ufbOutputBlock.m_fblItem->inputs().size() != 1)
					{
						assert(ufbOutputBlock.m_fblItem->inputs().size() == 1);
						result = false;
						continue;
					}

					//qDebug() << "ufbOutputBlock";			// !!!!!!!!!!!!!!!!!!!!!!!!
					//ufbOutputBlock.m_fblItem->dump();		// !!!!!!!!!!!!!!!!!!!!!!!!

					VFrame30::AfbPin& ufbOutputBlockPin = ufbOutputBlock.m_fblItem->inputs().front();
					if (ufbOutputBlockPin.associatedIOs().size() != 1)
					{
						assert(ufbOutputBlockPin.associatedIOs().size() == 1);
						result = false;
						continue;
					}

					// schemaInputItem1....schemaInputItemN
					//
					std::vector<VFrame30::AfbPin*> schemaInputItems;
					schemaInputItems.reserve(16);

					for (AppLogicItem& ali : module->items())
					{
						std::vector<VFrame30::AfbPin>& inputs = ali.m_fblItem->inputs();

						for (VFrame30::AfbPin& pin : inputs)
						{
							if (pin.IsInput() == true &&
								pin.associatedIOs().size() == 1 &&
								pin.associatedIOs().front() == ufbItemPin.guid())
							{
								schemaInputItems.push_back(&pin);
							}
						}
					}

					// ufbIntItem
					//
					QUuid ufbOutputBlockPinAssoc = ufbOutputBlockPin.associatedIOs().front();
					bool foundUfbIntItem = false;
					for (AppLogicItem& ufbItem : ufbItemsCopy)
					{
						if (ufbItem.m_fblItem->hasOutput(ufbOutputBlockPinAssoc) == true)
						{
							foundUfbIntItem = true;

							VFrame30::AfbPin& ufbIntItemPin = ufbItem.m_fblItem->output(ufbOutputBlockPinAssoc);

							for (VFrame30::AfbPin* schemaInputItemPin : schemaInputItems)
							{
								AppLogicData::bindTwoPins(ufbIntItemPin, *schemaInputItemPin);
							}

							ufbIntItemPin.removeFromAssociatedIo(ufbOutputBlockPin.guid());

							break;
						}
					}

					if (foundUfbIntItem == false)
					{
						// Cant bind ufbIntItem to schemaInputItems
						//
						assert(foundUfbIntItem);
						result = false;
						continue;
					}

					// Remove unwanted association from UFB:ufbItem
					//
					for (VFrame30::AfbPin* schemaInputItemPin : schemaInputItems)
					{
						schemaInputItemPin->removeFromAssociatedIo(ufbItemPin.guid());
					}
				}

				// Remove all Signal elements
				//
				ufbItemsCopy.remove_if(
							[](const AppLogicItem& ali)
							{
								return ali.m_fblItem->isSignalElement();
							});

				// Inject ufb schema items
				//
				module->items().insert(itemIt, ufbItemsCopy.begin(), ufbItemsCopy.end());

				//module->dump();
			}

			// Remove all VFrame30::SchemaItemUfb, as they have already beeen expanded
			//
			module->items().remove_if(
						[](const AppLogicItem& ali)
						{
							return ali.m_fblItem->isType<VFrame30::SchemaItemUfb>();
						});

			// Remove Fake Items
			//
			for (std::shared_ptr<VFrame30::SchemaItem> fakeItem : fakeItems)
			{
				module->removeInOutItemKeepAssoc(fakeItem->guid());
			}

			// --
			//

			qDebug() << "-------------------------- AFTER UFB EXPANDING ----------------------------------";
			module->dump();		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			checkResult = module->debugCheckItemsRelationsConsistency(log);
			if (checkResult == false)
			{
				return false;
			}
		}

		return result;
	}

	bool AppLogicData::bindTwoPins(VFrame30::AfbPin& outPint, VFrame30::AfbPin& inputPin)
	{
		// Function binds two pins to each other, it is done by setting assocation vector
		//
		if (outPint.IsOutput() == false ||
			inputPin.IsInput() == false)
		{
			assert(outPint.IsOutput() == true);
			assert(inputPin.IsInput() == true);
			return false;
		}

		// Just add associations to output and replace association for input
		//
		outPint.AddAssociattedIOs(inputPin.guid());
		inputPin.associatedIOs() = {outPint.guid()};

		return true;
	}

	const std::list<std::shared_ptr<AppLogicModule>>& AppLogicData::modules() const
	{
		return m_modules;
	}

	std::list<std::shared_ptr<AppLogicModule>>& AppLogicData::modules()
	{
		return m_modules;
	}

	std::shared_ptr<AppLogicModule> AppLogicData::module(QString moduleStrID)
	{
		for(std::shared_ptr<AppLogicModule> modulePtr : m_modules)
		{
			if (modulePtr->equipmentId() == moduleStrID)
			{
				return modulePtr;
			}
		}

		return std::shared_ptr<AppLogicModule>();
	}

	const std::map<QString, std::shared_ptr<AppLogicModule>>& AppLogicData::ufbs() const
	{
		return m_ufbs;
	}

	std::shared_ptr<AppLogicModule> AppLogicData::ufb(QString ufbId) const
	{
		auto it = m_ufbs.find(ufbId);

		if (it == m_ufbs.end())
		{
			return std::shared_ptr<AppLogicModule>();
		}
		else
		{
			return it->second;
		}
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

	bool Parser::parse()
	{
		// Get User Functional Blocks
		//
		std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;

		bool ok = loadUfbFiles(db(), &ufbs);
		if (ok == false)
		{
			return ok;
		}

		// Check Ufbs SchemaItemAfb.afbElement versions
		//
		for (std::shared_ptr<VFrame30::UfbSchema> schema : ufbs)
		{
			checkAfbItemsVersion(schema.get());
		}

		// Parse User Functional Blocks
		//
		if (ufbs.empty() == false)
		{
			LOG_MESSAGE(m_log, tr("Parsing User Functional Blocks..."));

			bool result = true;
			for (std::shared_ptr<VFrame30::UfbSchema> ufbSchema : ufbs)
			{
				if (QThread::currentThread()->isInterruptionRequested() == true)
				{
					return false;
				}

				LOG_MESSAGE(m_log, tr("Parsing ") + ufbSchema->schemaId());

				ok = parsUfbSchema(ufbSchema);

				if (ok == false)
				{
					result = false;
				}
			}
		}

		// The result is set of AppLogicModule (m_modules), but items are not ordered yet
		// Order itmes in all modules
		//
		bool result = true;

		LOG_MESSAGE(m_log, tr("Ordering User Functional Blocks items..."));

		ok = m_applicationData->orderUfbItems(m_log);

		if (ok == false)
		{
			result = false;
		}

		// Get Application Logic
		//
		std::vector<std::shared_ptr<VFrame30::LogicSchema>> schemas;

		ok = loadAppLogicFiles(db(), &schemas);

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

		// Check SchemaItemUfb versions
		//
		for (std::shared_ptr<VFrame30::LogicSchema> schema : schemas)
		{
			checkUfbItemsVersion(schema.get(), ufbs);
		}

		// Parse application logic
		//
		LOG_MESSAGE(m_log, tr("Parsing schemas..."));

		for (std::shared_ptr<VFrame30::LogicSchema> schema : schemas)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			LOG_MESSAGE(m_log, tr("Parsing ") + schema->schemaId());

			ok = parseAppLogicSchema(schema);

			if (ok == false)
			{
				result = false;
			}
		}

		// The result is set of AppLogicModule (m_modules), but items are not ordered yet
		// Order itmes in all modules
		//
		LOG_MESSAGE(m_log, tr("Ordering AppLogic items..."));

		ok = m_applicationData->orderLogicModuleItems(m_log);

		if (ok == false)
		{
			result = false;
		}

		// Check for relations conistency
		//
		LOG_MESSAGE(m_log, tr("Checking relations consistency..."));

		bool checkResult = true;
		for (std::shared_ptr<AppLogicModule> module : m_applicationData->modules())
		{
			checkResult &= module->debugCheckItemsRelationsConsistency(m_log);
		}

		for (std::pair<QString, std::shared_ptr<AppLogicModule>> ufb : m_applicationData->ufbs())
		{
			checkResult &= ufb.second->debugCheckItemsRelationsConsistency(m_log);
		}

		if (checkResult == false)
		{
			result = false;
			return result;
		}

		// Expand User Functioanl Block on places of SchemaIntemUfb
		//
		ok = m_applicationData->expandUfbs(m_log);

		if (ok == false)
		{
			result = false;
		}

		//  In debug mode save/show item order for displaying on schemas
		//
		setDebugInfo();

		return result;
	}

	bool Parser::loadUfbFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::UfbSchema>>* out)
	{
		return loadSchemaFiles<VFrame30::UfbSchema>(db, out, db->ufblFileId(), QLatin1String("%.") + ::UfbFileExtension);
	}

	bool Parser::loadAppLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicSchema>>* out)
	{
		return loadSchemaFiles<VFrame30::LogicSchema>(db, out, db->alFileId(), QLatin1String("%.") + ::AlFileExtension);
	}

	template<typename SchemaType>
	bool Parser::loadSchemaFiles(DbController* db, std::vector<std::shared_ptr<SchemaType>>* out, int parentFileId, QString filter)
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
		std::vector<DbFileInfo> fileList;

		if (release() == true)
		{
			ok = db->getFileList(&fileList, parentFileId, filter, true, nullptr);

			std::vector<DbFileInfo> markedAsDeletedRemoved;
			markedAsDeletedRemoved.reserve(fileList.size());

			for (DbFileInfo& fi : fileList)
			{
				if (fi.action() == VcsItemAction::Deleted)		// File is deleted
				{
					qDebug() << "Skip file " << fi.fileId() << ", " << fi.fileName() << ", it was marked as deleted";
					continue;
				}

				markedAsDeletedRemoved.push_back(fi);
			}

			std::swap(markedAsDeletedRemoved, fileList);
		}
		else
		{
			ok = db->getFileList(&fileList, parentFileId, filter, true, nullptr);

			std::vector<DbFileInfo> markedAsDeletedRemoved;
			markedAsDeletedRemoved.reserve(fileList.size());

			for (DbFileInfo& fi : fileList)
			{
				if (fi.action() == VcsItemAction::Deleted)		// File is deleted
				{
					qDebug() << "Skip file " << fi.fileId() << ", " << fi.fileName() << ", it was marked as deleted";
					continue;
				}

				markedAsDeletedRemoved.push_back(fi);
			}

			std::swap(markedAsDeletedRemoved, fileList);
		}

		if (ok == false)
		{
			// Error of getting file list from the database, parent file ID %1, filter '%2', database message %3.
			//
			m_log->errPDB2001(db->alFileId(), filter, db->lastError());
			return false;
		}

		if (fileList.empty() == true)
		{
			return true;		// it is not a error
		}

		out->reserve(fileList.size());

		// Get file data and read it
		//
		for (DbFileInfo& fi : fileList)
		{
			// Check for cancel
			//
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			// --
			//
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

			// Read schema files
			//
			std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file.get()->data());

			std::shared_ptr<SchemaType> ls = std::dynamic_pointer_cast<SchemaType>(schema);

			if (ls == nullptr)
			{
				assert(ls != nullptr);

				// File loading/parsing error, file is damaged or has incompatible format, file name '%1'.
				//
				m_log->errCMN0010(file->fileName());
				return false;
			}

			// Remove all commented items from the schema
			//
			for (std::shared_ptr<VFrame30::SchemaLayer> layer :  schema->Layers)
			{
				std::list<std::shared_ptr<VFrame30::SchemaItem>> newItemList;

				for (std::shared_ptr<VFrame30::SchemaItem> item :  layer->Items)
				{
					if (item->isCommented() == false)
					{
						newItemList.push_back(item);
					}
				}

				layer->Items.swap(newItemList);
			}

			// Add to schema list
			//
			if (ls->excludeFromBuild() == true)
			{
				// Schema is excluded from build (Schema '%1').
				//
				m_log->wrnALP4004(ls->schemaId());
				continue;
			}
			else
			{
				// Add schema to result
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
			m_log->errALP4001(logicSchema->schemaId());
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
				m_log->errALP4002(logicSchema->schemaId(), eqid);

				ok = false;
				continue;
			}

			if (device->isModule() == false)
			{
				// EquipmentID '%1' must be LM family module type (Logic Schema '%2').
				//
				m_log->errALP4003(logicSchema->schemaId(), eqid);

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
					m_log->errALP4003(logicSchema->schemaId(), eqid);

					ok = false;
					continue;
				}
			}
		}

		return ok;
	}

	bool Parser::checkAfbItemsVersion(VFrame30::Schema* schema)
	{
		if (schema == nullptr ||
			m_afbCollection == nullptr)
		{
			assert(schema);
			assert(m_afbCollection);

			m_log->errINT1000(QString(__FUNCTION__) + QString(", logicSchema %1, Parser::m_afbCollection %2")
							  .arg(reinterpret_cast<size_t>(schema))
							  .arg(reinterpret_cast<size_t>(m_afbCollection)));
			return false;
		}

		bool ok = true;

		for (std::shared_ptr<VFrame30::SchemaLayer> l : schema->Layers)
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
							m_log->errALP4007(schema->schemaId(), afbItem->buildName(), afbItem->afbStrID(), si->guid());

							ok = false;
							continue;
						}

						if (afbDescription->version() != afbItem->afbElement().version())
						{
							m_log->errALP4008(schema->schemaId(),
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

	bool Parser::checkUfbItemsVersion(VFrame30::LogicSchema* logicSchema,
									  const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs)
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

		std::map<QString, std::shared_ptr<VFrame30::UfbSchema>> ufbsMap;
		for (auto ufb : ufbs)
		{
			ufbsMap[ufb->schemaId()] = ufb;
		}

		bool ok = true;

		for (std::shared_ptr<VFrame30::SchemaLayer> l : logicSchema->Layers)
		{
			if (l->compile() == true)
			{
				for (std::shared_ptr<VFrame30::SchemaItem> si : l->Items)
				{
					if (si->isType<VFrame30::SchemaItemUfb>() == true)
					{
						VFrame30::SchemaItemUfb* ufbItem = si->toType<VFrame30::SchemaItemUfb>();
						assert(ufbItem);

						auto ufbsIt = ufbsMap.find(ufbItem->ufbSchemaId());

						if (ufbsIt == ufbsMap.end())
						{
							// UFB schema '%1' is not found for schema item '%2' (Logic Schema '%3').
							//
							m_log->errALP4009(logicSchema->schemaId(), ufbItem->buildName(), ufbItem->ufbSchemaId(), si->guid());
							ok = false;
							continue;
						}

						std::shared_ptr<VFrame30::UfbSchema> ufbSchema = ufbsIt->second;

						if (ufbSchema->version() != ufbItem->ufbSchemaVersion())
						{
							m_log->errALP4010(logicSchema->schemaId(),
											  ufbItem->buildName(),
											  ufbItem->ufbSchemaVersion(),
											  ufbSchema->version(),
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

	bool Parser::parsUfbSchema(std::shared_ptr<VFrame30::UfbSchema> ufbSchema)
	{
		if (ufbSchema.get() == nullptr)
		{
			assert(false);
			return false;
		}

		// Find layer for compilation
		//
		bool layerFound = false;
		bool ok = false;

		for (std::shared_ptr<VFrame30::SchemaLayer> l : ufbSchema->Layers)
		{
			if (l->compile() == true)
			{
				layerFound = true;
				ok = parseUfbLayer(ufbSchema, l);

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
			// Schema does not have logic layer (Schema '%1').
			//
			m_log->errALP4022(ufbSchema->schemaId());
			return false;
		}

		return true;
	}

	bool Parser::parseUfbLayer(std::shared_ptr<VFrame30::UfbSchema> ufbSchema,
							   std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		if (ufbSchema == nullptr || layer == nullptr)
		{
			assert(ufbSchema);
			assert(layer);
			return false;
		}

		// Check for nested UFBs
		//
		for (std::shared_ptr<VFrame30::SchemaItem> item : layer->Items)
		{
			if (item->isType<VFrame30::SchemaItemUfb>() == true)
			{
				assert(item->isFblItemRect() == true);

				// User Functional Block cannot have nested another UFB, SchemaItem %1 (UfbSchema '%2').
				//
				m_log->errALP4011(ufbSchema->schemaId(), item->toFblItemRect()->label(), item->guid());
				return false;
			}
		}

		// Find all branches - connected links
		//
		BushContainer bushContainer;

		bool result = true;
		result = findBushes(ufbSchema, layer, &bushContainer);

		if (result == false)
		{
			LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined, tr("Finding bushes error."));
			return false;
		}

		//bushContainer.debugInfo();

		// Set pins' guids to bushes
		// All log errors should be reported in setBranchConnectionToPin
		//
		result = setBranchConnectionToPin(ufbSchema, layer, &bushContainer);
		if (result == false)
		{
			return false;
		}

		// Associates input/outputs
		//
		result = setPinConnections(ufbSchema, layer, &bushContainer);
		if (result == false)
		{
			return false;
		}

		// Generate afb list, and set it to some container
		//
		applicationData()->addUfbData(bushContainer, ufbSchema, m_log);

		return true;
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
			m_log->errALP4022(logicSchema->schemaId());
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
			bool result = true;

			BushContainer bushContainer;

			if (logicSchema->isMultichannelSchema() == true)
			{
				// Multischema checks. All multichannel signals must be transformed to singlechannel here
				//
				result = multichannelProcessing(logicSchema, moduleLayer, equipmentId);
			}

			if (result == false)
			{
				// Something wron in multichannelProcessing for this schema, stop parsing it
				//
				continue;
			}

			result = findBushes(logicSchema, moduleLayer, &bushContainer);

			if (result == false)
			{
				LOG_ERROR_OBSOLETE(log(), Builder::IssueType::NotDefined, tr("Finding bushes error."));
				return false;
			}

			//bushContainer.debugInfo();

			// Set pins' guids to bushes
			// All log errors should be reported in setBranchConnectionToPin
			//
			result = setBranchConnectionToPin(logicSchema, moduleLayer, &bushContainer);
			if (result == false)
			{
				return false;
			}

			// Associates input/outputs
			//
			result = setPinConnections(logicSchema, moduleLayer, &bushContainer);
			if (result == false)
			{
				return false;
			}

			// Filter singlechannel logic branches in multischema drawing
			//
			if (logicSchema->isMultichannelSchema() == true)
			{
				filterSingleChannelBranchesInMulischema(logicSchema, equipmentId, &bushContainer);
			}

			// Generate afb list, and set it to some container
			//
			applicationData()->addLogicModuleData(equipmentId, bushContainer, logicSchema, m_log);
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
			m_log->errALP4001(schema->schemaId());
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
			m_log->errINT1001(QString("AppLogicModule::AppLogicModule(%1) signalIndexInBlocks == -1").arg(schema->schemaId()));
			return false;
		}

		bool result = true;

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
				result = false;
				m_log->errALP4031(schema->schemaId(), signalItem->buildName(), signalItem->guid());
				continue;
			}

			assert(false);
			result = false;
		}

		return result;
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

						if (signal == nullptr)
						{
							allSignalsFromThisChannel = false;
							m_log->errALP4034(schema->schemaId(), fbl.second->buildName(), appSignalId, fbl.second->guid());
							continue;
						}

						if (signal->lm() == nullptr)
						{
							allSignalsFromThisChannel = false;
							m_log->errALP4035(schema->schemaId(), fbl.second->buildName(), appSignalId, fbl.second->guid());
							continue;
						}

						if (signal->lm()->equipmentId() != equipmentId)
						{
							allSignalsFromThisChannel = false;

							if (lmEquipmnetId.isEmpty() == true)
							{
								// The first occurance LM, init lmEquipmnetId
								//
								lmEquipmnetId = signal->lm()->equipmentId();
							}
							else
							{
								if (lmEquipmnetId != signal->lm()->equipmentId())
								{
									// Single channel branch contains signals (%1) from different channels (LogicSchema '%2').
									//
									m_log->errALP4033(schema->schemaId(), appSignalId, signalElement->guid());
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
	bool Parser::findBushes(std::shared_ptr<VFrame30::Schema> schema,
							std::shared_ptr<VFrame30::SchemaLayer> layer,
							BushContainer* bushContainer) const
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


		// FblItems can conatain input or output pins, if input connects to output directly (without link)
		// then create fake link for it.
		// This is for special case, when input connects to output without link.
		//
		for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
		{
			VFrame30::FblItemRect* fblItem = dynamic_cast<VFrame30::FblItemRect*>(item->get());

			if (fblItem != nullptr)
			{
				fblItem->SetConnectionsPos(schema->gridSize(), schema->pinGridStep());	// Calculate pins positions

				const std::vector<VFrame30::AfbPin>& inputs = fblItem->inputs();
				const std::vector<VFrame30::AfbPin>& outputs = fblItem->outputs();

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

	bool Parser::setBranchConnectionToPin(
			std::shared_ptr<VFrame30::Schema> schema,
			std::shared_ptr<VFrame30::SchemaLayer> layer,
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

				std::vector<VFrame30::AfbPin>* inputs = fblItem->mutableInputs();
				std::vector<VFrame30::AfbPin>* outputs = fblItem->mutableOutputs();

				for (VFrame30::AfbPin& in : *inputs)
				{
					VFrame30::SchemaPoint pinPos = in.point();

					//qDebug() << "input  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = bushContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						log()->errALP4006(schema->schemaId(), fblItem->buildName(), in.caption(), item->guid());
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
						log()->errALP4006(schema->schemaId(), fblItem->buildName(), out.caption(), item->guid());

						result = false;
						continue;
					}

					// Branch was found for current pin
					//

					if (bushContainer->bushes[branchIndex].outputPin.isNull() == false)
					{
						// Branch has multiple outputs.
						//
						log()->errALP4000(schema->schemaId(), bushContainer->bushes[branchIndex].getAllUuid());

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
			std::shared_ptr<VFrame30::Schema> schema,
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

				std::vector<VFrame30::AfbPin>* inputs = fblElement->mutableInputs();
				std::vector<VFrame30::AfbPin>* outputs = fblElement->mutableOutputs();

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
                        //assert(false);

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
			m_log->wrnALP4005(schema->schemaId());
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
					const std::vector<VFrame30::AfbPin>& inputs = bush.getInputPinsForItem(item->guid());

					QString inputsStr;
					inputsStr.reserve(1024);
					for (const VFrame30::AfbPin& input : inputs)
					{
						inputsStr += (inputsStr.isEmpty() == true) ? input.caption() : QString(", %1").arg(input.caption());
					}

					std::vector<QUuid> issuedItemsUuid = bush.getLinksUuids();
					issuedItemsUuid.push_back(item->guid());

					m_log->errALP4006(schema->schemaId(), item->buildName(), inputsStr, issuedItemsUuid);
					result = false;
				}
			}

			if (bush.inputPins.empty() == true)
			{
				// Look for item without associated inputs
				//
				for (auto it = bush.fblItems.begin(); it != bush.fblItems.end(); ++it)
				{
					const std::shared_ptr<VFrame30::FblItemRect>& item = it->second;
					const std::vector<VFrame30::AfbPin>& outputs = item->outputs();

					for (const VFrame30::AfbPin& out : outputs)
					{
						if (out.associatedIOs().empty() == true)
						{
							// Schema item %1 has not linked pin %2 (Logic Schema '%3').
							//
							std::vector<QUuid> issuedItemsUuid = bush.getLinksUuids();
							issuedItemsUuid.push_back(item->guid());

							m_log->errALP4006(schema->schemaId(), item->buildName(), out.caption(), issuedItemsUuid);
							result = false;
						}
					}
				}
			}
		}

		return result;
	}

	void Parser::setDebugInfo()
	{
		//LOG_MESSAGE(m_log, "Debug Info:");

		// Set UFBs run order for drawing on schemas
		//
		const AppLogicData* appLogicData = applicationData();
		const auto& ufbs = appLogicData->ufbs();

		for (std::pair<QString, std::shared_ptr<AppLogicModule>> ufb : ufbs)
		{
			const std::list<AppLogicItem>& items = ufb.second->items();

			std::map<QUuid, std::pair<int, int>> schemaItemRunOrder;

			int index = 0;
			for (const AppLogicItem& it : items)
			{
				assert(schemaItemRunOrder.find(it.m_fblItem->guid()) == schemaItemRunOrder.end());

				schemaItemRunOrder[it.m_fblItem->guid()] = std::make_pair(index, index);
				index ++;
			}

			GlobalMessanger::instance()->setRunOrder(ufb.second->equipmentId(), schemaItemRunOrder);
		}

		// Set Schema Ityem Run Order for drawing on schemas
		//
		const auto& logicModules = appLogicData->modules();

		for (std::shared_ptr<AppLogicModule> lm : logicModules)
		{
			const std::list<AppLogicItem>& items = lm->items();

			std::map<QUuid, std::pair<int, int>> schemaItemRunOrder;

			int index = 0;
			for (const AppLogicItem& it : items)
			{
				if (it.m_groupId.isNull() == true)
				{
					assert(schemaItemRunOrder.find(it.m_fblItem->guid()) == schemaItemRunOrder.end());

					schemaItemRunOrder[it.m_fblItem->guid()] = std::make_pair(index, index);
				}
				else
				{
					// it.m_groupId is SchemaItemUfb.guid()
					//
					if (schemaItemRunOrder.count(it.m_groupId) == 0)
					{
						// This is the first group item
						//
						schemaItemRunOrder[it.m_groupId] = std::make_pair(index, index);
					}
					else
					{
						schemaItemRunOrder[it.m_groupId].second = index;
					}
				}

				index ++;
			}

			GlobalMessanger::instance()->setRunOrder(lm->equipmentId(), schemaItemRunOrder);
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
