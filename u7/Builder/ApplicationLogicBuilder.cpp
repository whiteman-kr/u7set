#include "ApplicationLogicBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/VideoItemLink.h"
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/VideoItemFblElement.h"
#include "../../VFrame30/VideoItemSignal.h"
#include "../../VFrame30/SchemeItemConst.h"
#include "../../VFrame30/HorzVertLinks.h"

#include "../../VFrame30/Fbl.h"


namespace Builder
{

	Link::Link(const std::list<VFrame30::VideoItemPoint>& points) :
		m_points(points)
	{
		assert(points.size() >= 2);
	}

	VFrame30::VideoItemPoint Link::ptBegin() const
	{
		if (m_points.empty() == true)
		{
			assert(m_points.empty() == false);
			return VFrame30::VideoItemPoint();
		}

		return m_points.front();
	}

	VFrame30::VideoItemPoint Link::ptEnd() const
	{
		if (m_points.empty() == true)
		{
			assert(m_points.empty() == false);
			return VFrame30::VideoItemPoint();
		}

		return m_points.back();
	}

	bool Link::isPinOnLink(VFrame30::VideoItemPoint pt) const
	{
		VFrame30::CHorzVertLinks hvl;

		QUuid fakeId = QUuid::createUuid();
		hvl.AddLinks(m_points, fakeId);

		bool result = hvl.IsPinOnLink(pt, QUuid::createUuid());	// Must be othe Quuid, as if it the same return value always false

		return result;
	}


	// Function finds branch with a point on it.
	// Returns branch index or -1 if a brach was not found
	//
	int BushContainer::getBranchByPinPos(VFrame30::VideoItemPoint pt) const
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


	// ------------------------------------------------------------------------
	//
	//		AppLogicItem
	//
	// ------------------------------------------------------------------------

	AppLogicItem::AppLogicItem(std::shared_ptr<VFrame30::FblItemRect> fblItem,
							   std::shared_ptr<VFrame30::LogicScheme> scheme,
							   std::shared_ptr<Afbl::AfbElement> afbElement) :
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
	ApplicationLogicModule::ApplicationLogicModule(QString moduleStrId) :
		m_moduleStrId(moduleStrId)
	{
	}

	bool ApplicationLogicModule::addBranch(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
			const BushContainer& bushContainer,
			Afbl::AfbElementCollection* afbCollection,
			OutputLog* log)
	{
		if (logicScheme == nullptr ||
			log == nullptr ||
			afbCollection == nullptr)
		{
			assert(logicScheme);
			assert(afbCollection);
			assert(log);
			return false;
		}

		if (bushContainer.bushes.empty() == true)
		{
			log->writeWarning(QObject::tr("Logic scheme does no contains any correct links."), false, true);
			return true;
		}

		bool result = true;

		// Save all items to accumulator, they will be ordered in orderItems()
		//
		for (const Bush& bush : bushContainer.bushes)
		{
			for (const std::shared_ptr<VFrame30::FblItemRect>& f : bush.fblItems)
			{
				std::shared_ptr<Afbl::AfbElement> afbElement;

				if (f->isFblElement())
				{
					afbElement = afbCollection->get(f->toFblElement()->afbStrID());

					if (afbElement == nullptr)
					{
						assert(afbElement != nullptr);
						log->writeError(QObject::tr("Fbl element does not have Afb description, scheme: %1, element: %2")
										.arg(logicScheme->strID())
										.arg(f->guid().toString()),
										false, true);

						return false;
					}
				}

				AppLogicItem li{f, logicScheme, afbElement};

				m_fblItemsAcc.insert(li);
			}
		}

		return result;
	}

	bool ApplicationLogicModule::orderItems(OutputLog* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		// The last preparation - connect VideoItemInputSignal to VideoItemOutputSignal.
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
			// Imposible set exucution order for branch, there is no first item,
			// firts item can be item without inputs
			//
			log->writeError(tr("There is no start point for the logic scheme branch"), false, true);

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

			const std::list<VFrame30::CFblConnectionPoint>& outputs = currentItem.m_fblItem->outputs();

			for (const VFrame30::CFblConnectionPoint& out : outputs)
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
			assert(fblItems.empty() == true);
			log->writeError(tr("Internal error, not all items were proceded. %s").arg(Q_FUNC_INFO), false, true);

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

			// ApplicationLogicScheme appScheme;
			// appScheme.setData(logicScheme, orderedList);

			// m_schemes.push_back(appScheme);

		}

		return result;
	}

	bool ApplicationLogicModule::setInputOutputsElementsConnection(OutputLog* log)
	{
		// Set connection between VideoItemInputSignal/VideoItemOutputSignal by StrIds
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
				VFrame30::VideoItemSignal* signalElement = li.m_fblItem->toSignalElement();
				assert(signalElement);

				// !!!! IN FUTURE, POSSIBLE WE WILL RECEIVE STRING ARRAY HERE, NOW WE ASSUME ONLY ONE STRID IS HERE
				//
				QString signalStrId = signalElement->signalStrIds();

				signalInputItems.insert(signalStrId, li);
				continue;
			}

			if (li.m_fblItem->isOutputSignalElement())
			{
				VFrame30::VideoItemSignal* signalElement = li.m_fblItem->toSignalElement();
				assert(signalElement);

				// !!!! IN FUTURE, POSSIBLE WE WILL RECEIVE STRING ARRAY HERE, NOW WE ASSUME ONLY ONE STRID IS HERE
				//
				QString signalStrId = signalElement->signalStrIds();

				if (signalOutputItems.contains(signalStrId) == true)
				{
					log->writeError(QObject::tr("Ouput element has duplicate StrId, element1: %1, element2:%2, StrId: %3")
									.arg(signalElement->guid().toString())
									.arg(signalOutputItems[signalStrId].m_fblItem->guid().toString())
									.arg(signalStrId)
									, false, true);

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
	std::list<AppLogicItem> ApplicationLogicModule::getItemsWithInput(
		Iter begin,
		Iter end,
		const QUuid& guid)
	{
		std::set<AppLogicItem> result;	// set removes duplicats

		for (auto item = begin; item != end; ++item)
		{
			const std::list<VFrame30::CFblConnectionPoint>& inputs = item->m_fblItem->inputs();

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


	QString ApplicationLogicModule::moduleStrId() const
	{
		return m_moduleStrId;
	}

	void ApplicationLogicModule::setModuleStrId(QString value)
	{
		m_moduleStrId = value;
	}

	const std::list<AppLogicItem>& ApplicationLogicModule::items() const
	{
		return m_items;
	}

	std::list<AppLogicItem>& ApplicationLogicModule::items()
	{
		return m_items;
	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	ApplicationLogicData::ApplicationLogicData()
	{
	}


	std::shared_ptr<ApplicationLogicModule> ApplicationLogicData::getModuleLogicData(QString moduleStrID)
	{
		for(std::shared_ptr<ApplicationLogicModule> modulePtr : m_modules)
		{
			if (modulePtr->moduleStrId() == moduleStrID)
			{
				return modulePtr;
			}
		}

		std::shared_ptr<ApplicationLogicModule> nullPtr(nullptr);
		return nullPtr;
	}


	bool ApplicationLogicData::addData(
			const BushContainer& bushContainer,
			std::shared_ptr<VFrame30::LogicScheme> scheme,
			std::shared_ptr<VFrame30::SchemeLayer> layer,
			Afbl::AfbElementCollection* afbCollection,
			OutputLog* log)
	{
		if (bushContainer.bushes.empty() == true)
		{
			// It is not error, just algorithm does not contain any fbl elements
			//
			return true;
		}

		if (scheme == nullptr ||
			layer == nullptr ||
			log == nullptr)
		{
			assert(scheme);
			assert(layer);
			assert(log);
			return false;
		}

		// Get module, if it is not in the list, add it
		//
		std::shared_ptr<ApplicationLogicModule> module;

		QString moduleStrId = scheme->hardwareStrIds();

		auto moduleIt = std::find_if(m_modules.begin(), m_modules.end(),
			[&moduleStrId](const std::shared_ptr<ApplicationLogicModule>& m)
			{
				return m->moduleStrId() == moduleStrId;
			});

		if (moduleIt == m_modules.end())
		{
			// Module was not found, addit
			//
			module = std::make_shared<ApplicationLogicModule>(moduleStrId);
			m_modules.push_back(module);
		}
		else
		{
			module = *moduleIt;
		}

		assert(module);

		// add new branch to module
		//
		bool result = module->addBranch(scheme, bushContainer, afbCollection,log);

		return result;
	}


	bool ApplicationLogicData::orderItems(OutputLog* log)
	{
		if (log == nullptr)
		{
			assert(nullptr);
			return false;
		}

		bool ok = true;
		bool result = true;

		for (std::shared_ptr<ApplicationLogicModule> m : m_modules)
		{
			log->writeMessage(QObject::tr("Module: %1").arg(m->moduleStrId()), false);

			ok = m->orderItems(log);

			if (ok == false)
			{
				result = false;
			}
		}

		return result;
	}

	const std::list<std::shared_ptr<ApplicationLogicModule>>& ApplicationLogicData::modules() const
	{
		return m_modules;
	}

	std::list<std::shared_ptr<ApplicationLogicModule>>& ApplicationLogicData::modules()
	{
		return m_modules;
	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------


	ApplicationLogicBuilder::ApplicationLogicBuilder(DbController* db,
													 OutputLog* log,
													 ApplicationLogicData* appLogicData,
													 Afbl::AfbElementCollection* afbCollection,
													 int changesetId,
													 bool debug) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug),
		m_applicationData(appLogicData),
		m_afbCollection(afbCollection)
	{
		assert(m_db);
		assert(m_log);
		assert(m_applicationData);
		assert(afbCollection);

		return;
	}

	ApplicationLogicBuilder::~ApplicationLogicBuilder()
	{
	}

	bool ApplicationLogicBuilder::build()
	{
		// Get Application Logic
		//
		std::vector<std::shared_ptr<VFrame30::LogicScheme>> schemes;

		bool ok = loadApplicationLogicFiles(db(), &schemes);

		if (ok == false)
		{
			return ok;
		}

		if (schemes.empty() == true)
		{
			m_log->writeMessage(tr("There is no appliction logic files in the project."), false);
			return true;
		}

		// Compile application logic
		//
		m_log->writeMessage(tr("Parsing schemes..."), false);

		bool result = true;

		for (std::shared_ptr<VFrame30::LogicScheme> scheme : schemes)
		{
			m_log->writeMessage(scheme->caption(), false);

			ok = compileApplicationLogicScheme(scheme);

			if (ok == false)
			{
				result = false;
			}
		}

		// The result is set of ApplicationLogicModule (m_modules), but items are not ordered yet
		// Order itmes in all modules
		//
		m_log->writeMessage(tr("Ordering items..."), false);

		ok = m_applicationData->orderItems(m_log);

		if (ok == false)
		{
			result = false;
		}

		return result;
	}

	bool ApplicationLogicBuilder::loadApplicationLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicScheme>>* out)
	{
		if (out == nullptr)
		{
			assert(out);
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
			ok = db->getFileList(&applicationLogicFileList, db->alFileId(), "%.als", nullptr);
		}

		if (ok == false)
		{
			m_log->writeError(tr("Cannot get application logic file list."), false, true);
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
			m_log->writeMessage(tr("Loading %1").arg(fi.fileName()), false);

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
				m_log->writeError(tr("Cannot get application logic file instances."), true, true);
				return false;
			}

			// Read Appliaction logic files
			//
			std::shared_ptr<VFrame30::LogicScheme> ls(dynamic_cast<VFrame30::LogicScheme*>(VFrame30::Scheme::Create(file.get()->data())));

			if (ls == nullptr)
			{
				assert(ls != nullptr);
				m_log->writeError(tr("File loading error."), true, true);
				return false;
			}

			if (ls->excludeFromBuild() == true)
			{
				m_log->writeWarning(tr("Scheme %1 excluded from build.").arg(ls->strID()), false, false);
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

	bool ApplicationLogicBuilder::compileApplicationLogicScheme(std::shared_ptr<VFrame30::LogicScheme> logicScheme)
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

		for (std::shared_ptr<VFrame30::SchemeLayer> l : logicScheme->Layers)
		{
			if (l->compile() == true)
			{
				layerFound = true;
				ok = compileApplicationLogicLayer(logicScheme, l);

				if (ok == false)
				{
					return false;
				}

				// We can compile only one layer
				//
				break;
			}
		}

		if (layerFound == false)
		{
			m_log->writeError(tr("There is no compile layer in the scheme."), false, true);
			return false;
		}

		return true;
	}

	bool ApplicationLogicBuilder::compileApplicationLogicLayer(
		std::shared_ptr<VFrame30::LogicScheme> logicScheme,
		std::shared_ptr<VFrame30::SchemeLayer> layer)
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
			log()->writeError(tr("Finding bushes error."), false, false);
			return false;
		}

		// Set pins' guids to bushes
		//
		result = setBranchConnectionToPin(logicScheme, layer, &bushContainer);

		if (result == false)
		{
			log()->writeError("setBranchConnectionToPin function error.", false, false);
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
			log()->writeError(tr("Internal error: Cannot set data to ApplicationLogicData."), false, true);
			return false;
		}

		return true;
	}

	// Function connects all links, and compose them into bushes
	//
	bool ApplicationLogicBuilder::findBushes(
		std::shared_ptr<VFrame30::LogicScheme> logicScheme,
		std::shared_ptr<VFrame30::SchemeLayer> layer,
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

		std::list<std::set<QUuid>> bushes;

		// Enum all links and get all horzlinks and vertlinks
		//
		VFrame30::CHorzVertLinks horzVertLinks;

		for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
		{
			VFrame30::VideoItemLink* link = dynamic_cast<VFrame30::VideoItemLink*>(item->get());

			if (link != nullptr)
			{
				const std::list<VFrame30::VideoItemPoint>& pointList = link->GetPointList();

				if (pointList.size() < 2)
				{
					assert(pointList.size() >= 2);
					continue;
				}

				// Decompose link on different parts and put them to horzlinks and vertlinks
				//
				horzVertLinks.AddLinks(pointList, link->guid());
			}
		}

		// Enum all vert and horz links and compose bushes
		//
		for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
		{
			VFrame30::VideoItemLink* link = dynamic_cast<VFrame30::VideoItemLink*>(item->get());

			if (link == nullptr)
			{
				continue;
			}

			const std::list<VFrame30::VideoItemPoint>& pointList = link->GetPointList();

			if (pointList.size() < 2)
			{
				assert(pointList.size() >= 2);
				continue;
			}

			// Check if end points on some link
			//
			std::list<QUuid> videoItemsUnderFrontPoint = horzVertLinks.getVideoItemsUnderPoint(pointList.front(), link->guid());
			std::list<QUuid> videoItemsUnderBackPoint = horzVertLinks.getVideoItemsUnderPoint(pointList.back(), link->guid());

			// Find item branch, if branch does not exists, make a new brach
			//
			auto foundBranch = std::find_if(bushes.begin(), bushes.end(),
											[link](const std::set<QUuid>& b)
			{
				auto foundBranch = b.find(link->guid());
				return foundBranch != b.end();
			});

			if (foundBranch == bushes.end())
			{
				std::set<QUuid> newBranch;
				newBranch.insert(link->guid());

				bushes.push_front(newBranch);

				foundBranch = bushes.begin();
			}

			// Add to foundBranch everything from  videoItemsUnderFrontPoint, videoItemsUnderBackPoint
			//
			for (QUuid& q : videoItemsUnderFrontPoint)
			{
				foundBranch->insert(q);
			}

			for (QUuid& q : videoItemsUnderBackPoint)
			{
				foundBranch->insert(q);
			}
		}

		// branches can contain same items,
		// all such branches must be united
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
			Bush newBranch;

			for (QUuid id : b)
			{
				// Get VideoItemLink by this id,
				// save it's and points to newBranch
				//
				std::shared_ptr<VFrame30::VideoItem> videoItem = layer->getItemById(id);
				VFrame30::VideoItemLink* link = dynamic_cast<VFrame30::VideoItemLink*>(videoItem.get());

				if (videoItem == nullptr ||
					link == nullptr)
				{
					assert(videoItem);
					assert(link);

					log()->writeError(tr("%1Internal error, expected VFrame30::VideoItemLink").arg(__FUNCTION__), false, true);
					return false;
				}

				const std::list<VFrame30::VideoItemPoint>& pointList = link->GetPointList();

				if (pointList.size() < 2)
				{
					assert(pointList.size() >= 2);
					log()->writeError(tr("%1Internal error, Link has less the two points").arg(__FUNCTION__), false, true);
					return false;
				}

				newBranch.links[id] = Link(pointList);
			}

			bushContainer->bushes.push_back(newBranch);
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

	bool ApplicationLogicBuilder::setBranchConnectionToPin(std::shared_ptr<VFrame30::LogicScheme> scheme, std::shared_ptr<VFrame30::SchemeLayer> layer,
						BushContainer* branchContainer) const
	{
		if (scheme.get() == nullptr ||
			layer.get() == nullptr ||
			branchContainer == nullptr)
		{
			assert(scheme);
			assert(layer);
			assert(branchContainer);
			return false;
		}

		bool result = true;

		for (auto& item : layer->Items)
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
				// VideoItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblItem->ClearAssociatedConnections();
				fblItem->SetConnectionsPos(scheme->gridSize(), scheme->pinGridStep());

				std::list<VFrame30::CFblConnectionPoint>* inputs = fblItem->mutableInputs();
				std::list<VFrame30::CFblConnectionPoint>* outputs = fblItem->mutableOutputs();

				for (VFrame30::CFblConnectionPoint& in : *inputs)
				{
					VFrame30::VideoItemPoint pinPos = in.point();

					//qDebug() << "input  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = branchContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						VFrame30::VideoItemInputSignal* inputSignal = dynamic_cast<VFrame30::VideoItemInputSignal*>(item.get());
						VFrame30::VideoItemOutputSignal* outputSignal = dynamic_cast<VFrame30::VideoItemOutputSignal*>(item.get());
						VFrame30::SchemeItemConst* schemeItemConst = dynamic_cast<VFrame30::SchemeItemConst*>(item.get());
						VFrame30::VideoItemFblElement* fblElement = dynamic_cast<VFrame30::VideoItemFblElement*>(item.get());

						if (inputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1: Input %2 has unconnected pin")
								.arg(scheme->caption())
								.arg(inputSignal->signalStrIds()),
								false, true);

							result = false;
							continue;
						}

						if (outputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1: Output %2 has unconnected pin")
								.arg(scheme->caption())
								.arg(outputSignal->signalStrIds()),
								false, true);

							result = false;
							continue;
						}

						if (schemeItemConst != nullptr)
						{
							log()->writeError(tr("LogicScheme %1: Constant element %2 has unconnected pin")
								.arg(scheme->caption())
								.arg(schemeItemConst->valueToString()),
								false, true);

							result = false;
							continue;
						}

						if (fblElement != nullptr)
						{
							std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(fblElement->afbStrID());

							log()->writeError(tr("LogicScheme %1: Item '%2', pin '%3' is unconnected")
								.arg(scheme->caption())
								.arg(afb->caption())
								.arg(in.caption()),
								false, true);

							result = false;
							continue;
						}

						assert(false);		// What the item is it?
					}

					// Branch was found for current pin
					//
					branchContainer->bushes[branchIndex].inputPins.insert(in.guid());
				}

				for (const VFrame30::CFblConnectionPoint& out : *outputs)
				{
					VFrame30::VideoItemPoint pinPos = out.point();

					//qDebug() << "output  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = branchContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						VFrame30::VideoItemInputSignal* inputSignal = dynamic_cast<VFrame30::VideoItemInputSignal*>(item.get());
						VFrame30::VideoItemOutputSignal* outputSignal = dynamic_cast<VFrame30::VideoItemOutputSignal*>(item.get());
						VFrame30::SchemeItemConst* schemeItemConst = dynamic_cast<VFrame30::SchemeItemConst*>(item.get());
						VFrame30::VideoItemFblElement* fblElement = dynamic_cast<VFrame30::VideoItemFblElement*>(item.get());

						if (inputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1: Input %2 has unconnected pin.")
								.arg(scheme->caption())
								.arg(inputSignal->signalStrIds()),
								false, true);

							result = false;
							continue;
						}

						if (outputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1: Output %2 has unconnected pin.")
								.arg(scheme->caption())
								.arg(outputSignal->signalStrIds()),
								false, true);

							result = false;
							continue;
						}

						if (schemeItemConst != nullptr)
						{
							log()->writeError(tr("LogicScheme %1: Constant element %2 has unconnected pin.")
								.arg(scheme->caption())
								.arg(schemeItemConst->valueToString()),
								false, true);

							result = false;
							continue;
						}

						if (fblElement != nullptr)
						{
							std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(fblElement->afbStrID());

							log()->writeError(tr("LogicScheme %1: Item '%2', pin '%3' is unconnected")
								.arg(scheme->caption())
								.arg(afb->caption())
								.arg(out.caption()),
								false, true);

							result = false;
							continue;
						}

						assert(false);		// What the item is it?
					}

					// Branch was found for current pin
					//

					if (branchContainer->bushes[branchIndex].outputPin.isNull() == false)
					{
						log()->writeError(tr("LogicScheme %1 (layer %2): Branch has multiple outputs.")
							.arg(scheme->caption())
							.arg(layer->name())
							, false, true);

						result = false;
						continue;
					}
					else
					{
						branchContainer->bushes[branchIndex].outputPin = out.guid();
					}
				}
			}
		}

		return result;
	}


	bool ApplicationLogicBuilder::setPinConnections(
		std::shared_ptr<VFrame30::LogicScheme> scheme,
		std::shared_ptr<VFrame30::SchemeLayer> layer,
		BushContainer* branchContainer)
	{
		if (scheme.get() == nullptr ||
			layer.get() == nullptr ||
			branchContainer == nullptr)
		{
			assert(scheme);
			assert(layer);
			assert(branchContainer);
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

				// VideoItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblElement->ClearAssociatedConnections();
				fblElement->SetConnectionsPos(scheme->gridSize(), scheme->pinGridStep());

				std::list<VFrame30::CFblConnectionPoint>* inputs = fblElement->mutableInputs();
				std::list<VFrame30::CFblConnectionPoint>* outputs = fblElement->mutableOutputs();

				for (VFrame30::CFblConnectionPoint& in : *inputs)
				{
					int branchIndex = branchContainer->getBranchByPinGuid(in.guid());

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						assert(false);

						log()->writeError(tr("LogicScheme %1 (layer %2): Internalerror in function, branch suppose to be found, %1.")
							.arg(__FUNCTION__), false, true);

						result = false;
						return result;
					}

					// Branch was found for current pin
					//
					Bush& bush = branchContainer->bushes[branchIndex];

					bush.fblItems.insert(fblElement);

					if (bush.outputPin.isNull() == true)
					{
						log()->writeError(
							tr("LogicScheme %1: There is no input pin for branch.").arg(scheme->caption()),
							false, true);

						result = false;
						return result;
					}

					// Set sourche pin guid for this pin
					//
					in.AddAssociattedIOs(bush.outputPin);
				}

				for (VFrame30::CFblConnectionPoint& out : *outputs)
				{
					int branchIndex = branchContainer->getBranchByPinGuid(out.guid());

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						assert(false);

						log()->writeError(tr("LogicScheme %1 (layer %2): Internalerror in function, branch suppose to be found, %1.")
							.arg(__FUNCTION__), false, true);

						result = false;
						return result;
					}				

					// Branch was found for current pin
					//
					Bush& bush = branchContainer->bushes[branchIndex];

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
			log()->writeWarning("Empty logic scheme, functional blocks were not found.", false, true);
			return true;
		}

		return result;
	}


	DbController* ApplicationLogicBuilder::db()
	{
		return m_db;
	}

	OutputLog* ApplicationLogicBuilder::log() const
	{
		return m_log;
	}

	int ApplicationLogicBuilder::changesetId() const
	{
		return m_changesetId;
	}

	bool ApplicationLogicBuilder::debug() const
	{
		return m_debug;
	}

	bool ApplicationLogicBuilder::release() const
	{
		return !debug();
	}

	const ApplicationLogicData* ApplicationLogicBuilder::applicationData() const
	{
		return m_applicationData;
	}

	ApplicationLogicData* ApplicationLogicBuilder::applicationData()
	{
		return m_applicationData;
	}

}
