#include "ApplicationLogicBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/VideoItemLink.h"
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/VideoItemFblElement.h"
#include "../../VFrame30/VideoItemSignal.h"
#include "../../VFrame30/HorzVertLinks.h"


namespace Builder
{

	Link::Link(const VFrame30::VideoItemPoint& point1, const VFrame30::VideoItemPoint& point2) :
		pt1(point1),
		pt2(point2)
	{
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
					return link.second.pt1 == pt || link.second.pt2 == pt;
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
	//		ApplicationLogicBranch
	//
	// ------------------------------------------------------------------------
//	ApplicationLogicBranch::ApplicationLogicBranch()
//	{
//	}

//	const std::list<std::shared_ptr<VFrame30::FblItemRect>>& ApplicationLogicBranch::items() const
//	{
//		return m_items;
//	}

//	std::list<std::shared_ptr<VFrame30::FblItemRect>>& ApplicationLogicBranch::items()
//	{
//		return m_items;
//	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicModule
	//
	// ------------------------------------------------------------------------
	ApplicationLogicModule::ApplicationLogicModule(QString moduleStrId) :
		m_moduleStrId(moduleStrId)
	{
	}

	struct ChangeOrder
	{
		struct HistoryItem
		{
			std::shared_ptr<VFrame30::FblItemRect> ChangeItem;
			int count;
		};

		std::shared_ptr<VFrame30::FblItemRect> item;
		std::list<HistoryItem> history;

		int getChangeCount(const std::shared_ptr<VFrame30::FblItemRect>& forItem)
		{
			auto it = std::find_if(history.begin(), history.end(),
				[&forItem](const HistoryItem& hi)
				{
					return hi.ChangeItem == forItem;
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

		void incChangeCount(const std::shared_ptr<VFrame30::FblItemRect>& forItem)
		{
			auto it = std::find_if(history.begin(), history.end(),
				[&forItem](const HistoryItem& hi)
				{
					return hi.ChangeItem == forItem;
				});

			if (it == history.end())
			{
				HistoryItem hi = {forItem, 1};
				history.push_back(hi);
			}
			else
			{
				it->count++;
			}
		}
	};


	bool ApplicationLogicModule::addBranch(const BushContainer& bushContainer, OutputLog* log)
	{
		if (bushContainer.bushes.empty() == true)
		{
			return false;
		}

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		// Go throuhg all bushes and make branches from them
		//

		// Get a COPY of the list, as the items will be moved to orderedList during algorithm work
		// Do not get reference!
		//
		std::set<std::shared_ptr<VFrame30::FblItemRect>> constFblItems;
		std::set<std::shared_ptr<VFrame30::FblItemRect>> fblItems;

		for (const Bush& bush : bushContainer.bushes)
		{
			for (const std::shared_ptr<VFrame30::FblItemRect>& f : bush.fblItems)
			{
				constFblItems.insert(f);
				fblItems.insert(f);
			}
		}

		// Add FblElement to branch in execution order
		//
		std::list<std::shared_ptr<VFrame30::FblItemRect>> orderedList;

		// Add all inputs and outputs
		// Warning:	Can be optimized by removing items from fblItems on the same
		//			loop
		//
		bool hasItemsWithouInputs = false;

		for (const std::shared_ptr<VFrame30::FblItemRect>& item : fblItems)
		{
			if (item->inputsCount() == 0)
			{
				orderedList.push_front(item);	// items without inputs must be at the begining of the list
				hasItemsWithouInputs = true;
				continue;
			}

			if (item->outputsCount() == 0)
			{
				orderedList.push_back(item);	// items without outputs must be at the end of the list
				continue;
			}
		}

		for (const std::shared_ptr<VFrame30::FblItemRect>& orderedItem : orderedList)
		{
			fblItems.erase(orderedItem);
		}

		if (hasItemsWithouInputs == false)
		{
			assert(hasItemsWithouInputs == true);

			// Imposible set exucution order for branch, there is no first item,
			// firts item can be item without inputs
			//
			log->writeError(tr("Imposible to set execution order for branch, there is no first item, it can be item without inputs"), false, true);

			result = false;
			return result;
		}

		std::list<ChangeOrder> changeOrderHistory;

		// Set other items
		//
		for (auto currentIt = orderedList.begin(); currentIt != orderedList.end(); ++currentIt)
		{
			const std::shared_ptr<VFrame30::FblItemRect>& currentItem = *currentIt;

			// Get dependant items
			//
			std::set<std::shared_ptr<VFrame30::FblItemRect>> dependantItems;

			const std::list<VFrame30::CFblConnectionPoint>& outputs = currentItem->outputs();

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
			for (std::shared_ptr<VFrame30::FblItemRect> dep : dependantItems)
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
			std::swap(items(), orderedList);
		}

		return result;
	}

	template<typename Iter>
	std::list<std::shared_ptr<VFrame30::FblItemRect>> ApplicationLogicModule::getItemsWithInput(
		Iter begin,
		Iter end,
		const QUuid& guid)
	{
		std::set<std::shared_ptr<VFrame30::FblItemRect>> result;	// set removes duplicats

		for (auto item = begin; item != end; ++item)
		{
			const std::list<VFrame30::CFblConnectionPoint>& inputs = item->get()->inputs();

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

		std::list<std::shared_ptr<VFrame30::FblItemRect>> resultList;
		resultList.assign(result.begin(), result.end());

		return resultList;
	}


	template<typename Iter>
	std::list<std::shared_ptr<VFrame30::FblItemRect>> ApplicationLogicModule::getItemsWithInput(
		Iter begin,
		Iter end,
		const std::list<QUuid>& guids)
	{
		std::set<std::shared_ptr<VFrame30::FblItemRect>> result;	// set removes duplicats

		for (auto item = begin; item != end; ++item)
		{
			const std::list<VFrame30::CFblConnectionPoint>& inputs = item->get()->inputs();

			for (auto in : inputs)
			{
				size_t inResultSize = result.size();

				const std::list<QUuid>& associatedOutputs = in.associatedIOs();

				for (const QUuid& id : guids)
				{
					auto foundAssociated = std::find(associatedOutputs.begin(), associatedOutputs.end(), id);

					if (foundAssociated != associatedOutputs.end())
					{
						result.insert(*item);
						break;
					}
				}

				if (inResultSize != result.size())
				{
					break;
				}
			}
		}

		std::list<std::shared_ptr<VFrame30::FblItemRect>> resultList;
		resultList.assign(result.begin(), result.end());

		return resultList;
	}


	QString ApplicationLogicModule::moduleStrId() const
	{
		return m_moduleStrId;
	}

	void ApplicationLogicModule::setModuleStrId(QString value)
	{
		m_moduleStrId = value;
	}

	const std::list<std::shared_ptr<VFrame30::FblItemRect>>& ApplicationLogicModule::items() const
	{
		return m_items;
	}

	std::list<std::shared_ptr<VFrame30::FblItemRect>>& ApplicationLogicModule::items()
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
			OutputLog* log)
	{
		if (bushContainer.bushes.empty() == true)
		{
			return false;
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
		bool result = module->addBranch(bushContainer, log);

		return result;
	}


//	const std::shared_ptr<VFrame30::LogicScheme> ApplicationLogicData::scheme() const
//	{
//		return m_scheme;
//	}

//	std::shared_ptr<VFrame30::LogicScheme> ApplicationLogicData::scheme()
//	{
//		return m_scheme;
//	}

//	const std::shared_ptr<VFrame30::SchemeLayer> ApplicationLogicData::layer() const
//	{
//		return m_layer;
//	}

//	std::shared_ptr<VFrame30::SchemeLayer> ApplicationLogicData::layer()
//	{
//		return m_layer;
//	}

//	std::list<std::shared_ptr<VFrame30::FblItemRect>> ApplicationLogicData::afbItems() const
//	{
//		return m_afbItems;
//	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------


	ApplicationLogicBuilder::ApplicationLogicBuilder(DbController* db, OutputLog* log, ApplicationLogicData *appLogicData,
		int changesetId, bool debug) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug),
		m_applicationData(appLogicData)
	{
		assert(m_db);
		assert(m_log);

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
		m_log->writeMessage(tr("Compiling..."), false);

		for (std::shared_ptr<VFrame30::LogicScheme> scheme : schemes)
		{
			m_log->writeMessage(scheme->caption(), false);

			ok = compileApplicationLogicScheme(scheme);

			if (ok == false)
			{
				return false;
			}
		}

		return true;
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

			// Add LogicScheme to result
			//
			out->push_back(ls);
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
			qDebug() << Q_FUNC_INFO << " WARNING!!!! Compiling ALL layers, in future compile just l->compile() LAYER!!!!";

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
		result = applicationData()->addData(bushContainer, logicScheme, layer, m_log);

		if (result == false)
		{
			log()->writeError(tr("Internal error: Cannot set data to ApplicationLogicData."), false, true);
			return false;
		}

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//
		// DEBUG
		//
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//		std::list<std::shared_ptr<VFrame30::FblItemRect>> items = data.afbItems();

//		qDebug() << "";
//		qDebug() << tr("Application Functional Blocks list, Scheme: %1, Layer %2").arg(logicScheme->caption()).arg(layer->name());

//		for (std::shared_ptr<VFrame30::FblItemRect> i : items)
//		{
//			qDebug() << "";

//			std::shared_ptr<VFrame30::VideoItemFblElement> fblElement = std::dynamic_pointer_cast<VFrame30::VideoItemFblElement>(i);
//			std::shared_ptr<VFrame30::VideoItemInputSignal> inputElement = std::dynamic_pointer_cast<VFrame30::VideoItemInputSignal>(i);
//			std::shared_ptr<VFrame30::VideoItemOutputSignal> outputElement = std::dynamic_pointer_cast<VFrame30::VideoItemOutputSignal>(i);

//			if (fblElement)
//			{
//				std::shared_ptr<Afbl::AfbElement> afb = logicScheme->afbCollection().get(fblElement->afbGuid());

//				qDebug() << afb->caption();

//				const std::list<VFrame30::CFblConnectionPoint>& inputs = fblElement->inputs();
//				const std::list<VFrame30::CFblConnectionPoint>& outputs = fblElement->outputs();

//				for (const VFrame30::CFblConnectionPoint& in : inputs)
//				{
//					QString str = QString("\tInput %1, associated pins: ").arg(in.guid().toString());

//					const std::list<QUuid>& assIos = in.associatedIOs();	// AssIos ))))

//					for (auto apid : assIos)
//					{
//						str.append(QString(" %1,").arg(apid.toString()));
//					}

//					qDebug() << str;
//				}

//				for (const VFrame30::CFblConnectionPoint& out : outputs)
//				{
//					QString str = QString("\tOutput %1, associated pins: ").arg(out.guid().toString());

//					const std::list<QUuid>& assIos = out.associatedIOs();	// AssIos ))))

//					for (auto apid : assIos)
//					{
//						str.append(QString(" %1,").arg(apid.toString()));
//					}

//					qDebug() << str;
//				}
//			}

//			if (inputElement)
//			{
//				const std::list<VFrame30::CFblConnectionPoint>& inputs = inputElement->inputs();
//				const std::list<VFrame30::CFblConnectionPoint>& outputs = inputElement->outputs();

//				assert(inputs.empty() == true);
//				assert(outputs.size() == 1);
//				assert(outputs.front().associatedIOs().size() > 0);

//				qDebug() << "Input Element: " << inputElement->signalStrIds();

//				for (const VFrame30::CFblConnectionPoint& out : outputs)
//				{
//					QString str = QString("\tOutput %1, associated pins: ").arg(out.guid().toString());

//					const std::list<QUuid>& assIos = out.associatedIOs();	// AssIos ))))

//					for (auto apid : assIos)
//					{
//						str.append(QString(" %1,").arg(apid.toString()));
//					}

//					qDebug() << str;
//				}
//			}

//			if (outputElement)
//			{
//				const std::list<VFrame30::CFblConnectionPoint>& inputs = outputElement->inputs();
//				const std::list<VFrame30::CFblConnectionPoint>& outputs = outputElement->outputs();

//				assert(inputs.size() == 1);
//				assert(inputs.front().associatedIOs().size() == 1);
//				assert(outputs.empty() == true);

//				qDebug() << "Output Element: " << outputElement->signalStrIds();

//				QString str = QString("\tInputPin %1, associated pin: %2")
//							  .arg(inputs.front().guid().toString())
//							  .arg(inputs.front().associatedIOs().front().toString());

//				qDebug() << str;
//			}
//		}

//		qDebug() << "";

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//
		// END OF DEBUG
		//
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


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

				VFrame30::VideoItemPoint pt1 = pointList.front();
				VFrame30::VideoItemPoint pt2 = pointList.back();

				newBranch.links[id] = Link(pt1, pt2);
			}

			bushContainer->bushes.push_back(newBranch);
		}

		// DEBUG
		for (Bush& eb : bushContainer->bushes)
		{
			qDebug() << "-----";
			for (auto& bl : eb.links)
			{
				qDebug() << bl.first << "--" <<
							bl.second.pt1.X <<
							"-" <<
							bl.second.pt1.Y <<
							"    " <<
							bl.second.pt2.X <<
							"-" <<
							bl.second.pt2.Y;
			}
		}
		// END OF DEBUG

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
				fblItem->SetConnectionsPos();

				std::list<VFrame30::CFblConnectionPoint>* inputs = fblItem->mutableInputs();
				std::list<VFrame30::CFblConnectionPoint>* outputs = fblItem->mutableOutputs();

				for (VFrame30::CFblConnectionPoint& in : *inputs)
				{
					VFrame30::VideoItemPoint pinPos = in.point();

					qDebug() << "input  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = branchContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						VFrame30::VideoItemInputSignal* inputSignal = dynamic_cast<VFrame30::VideoItemInputSignal*>(item.get());
						VFrame30::VideoItemOutputSignal* outputSignal = dynamic_cast<VFrame30::VideoItemOutputSignal*>(item.get());
						VFrame30::VideoItemFblElement* fblElement = dynamic_cast<VFrame30::VideoItemFblElement*>(item.get());

						if (inputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1 (layer %2): InputSignal element has unconnected pin.")
								.arg(scheme->caption())
								.arg(layer->name()),
								false, true);

							result = false;
							continue;
						}

						if (outputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1 (layer %2): OutputSignal element has unconnected pin.")
								.arg(scheme->caption())
								.arg(layer->name()),
								false, true);

							result = false;
							continue;
						}

						if (fblElement != nullptr)
						{
							std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(fblElement->afbGuid());

							log()->writeError(tr("LogicScheme %1 (layer %2): Item '%3' has unconnected pins.")
								.arg(scheme->caption())
								.arg(layer->name())
								.arg(afb->caption()),
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

					qDebug() << "output  " << pinPos.X << " -" << pinPos.Y;

					int branchIndex = branchContainer->getBranchByPinPos(pinPos);

					if (branchIndex == -1)
					{
						// Pin is not connectext to any link, this is error
						//
						VFrame30::VideoItemInputSignal* inputSignal = dynamic_cast<VFrame30::VideoItemInputSignal*>(item.get());
						VFrame30::VideoItemOutputSignal* outputSignal = dynamic_cast<VFrame30::VideoItemOutputSignal*>(item.get());
						VFrame30::VideoItemFblElement* fblElement = dynamic_cast<VFrame30::VideoItemFblElement*>(item.get());

						if (inputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1 (layer %2): InputSignal element has unconnected pin.")
								.arg(scheme->caption())
								.arg(layer->name())
								, false, true);

							result = false;
							continue;
						}

						if (outputSignal != nullptr)
						{
							log()->writeError(tr("LogicScheme %1 (layer %2): OutputSignal element has unconnected pin.")
								.arg(scheme->caption())
								.arg(layer->name())
								, false, true);

							result = false;
							continue;
						}

						if (fblElement != nullptr)
						{
							std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(fblElement->afbGuid());

							log()->writeError(tr("LogicScheme %1 (layer %2): Item '%3' has unconnected pins.")
								.arg(scheme->caption())
								.arg(layer->name())
								.arg(afb->caption())
								, false, true);

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

		for (auto& item : layer->Items)
		{
			VFrame30::FblItemRect* fblirect = dynamic_cast<VFrame30::FblItemRect*>(item.get());

			if(fblirect != nullptr)
			{
				std::shared_ptr<VFrame30::FblItemRect> fblElement =
						std::dynamic_pointer_cast<VFrame30::FblItemRect>(item);

				// VideoItem has inputs and outputs
				// Get coordinates for each input/output and
				// find branche with point on the pin
				//
				fblElement->ClearAssociatedConnections();
				fblElement->SetConnectionsPos();

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
						assert(bush.outputPin.isNull() == false);

						log()->writeError(tr("LogicScheme %1 (layer %2): Internalerror in function, output pin in brach suppose to be initialized, %1.")
							.arg(__FUNCTION__), false, true);

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
