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

	BranchLink::BranchLink(const VFrame30::VideoItemPoint& point1, const VFrame30::VideoItemPoint& point2) :
		pt1(point1),
		pt2(point2)
	{
	}


	// Function finds branch with a point on it.
	// Returns branch index or -1 if a brach was not found
	//
	int BranchContainer::getBranchByPinPos(VFrame30::VideoItemPoint pt) const
	{
		for (size_t i = 0; i < branches.size(); i++)
		{
			const Branch& branch = branches[i];

			auto link = std::find_if(branch.links.cbegin(), branch.links.cend(),
				[&pt](const std::pair<QUuid, BranchLink>& link)
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

	int BranchContainer::getBranchByPinGuid(const QUuid& guid) const
	{
		for (size_t i = 0; i < branches.size(); i++)
		{
			const Branch& branch = branches[i];

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
	//		ApplicationLogicData
	//
	// ------------------------------------------------------------------------
	ApplicationLogicData::ApplicationLogicData()
	{
	}

	bool ApplicationLogicData::setData(std::shared_ptr<VFrame30::LogicScheme> scheme,
									 std::shared_ptr<VFrame30::SchemeLayer> layer)
	{
		if (!scheme ||
			!layer)
		{
			assert(scheme);
			assert(layer);
			return false;
		}

		m_scheme = scheme;
		m_layer = layer;

		bool result = true;

		for (std::shared_ptr<VFrame30::VideoItem> item : layer->Items)
		{
			std::shared_ptr<VFrame30::FblItemRect> fblItemRect = std::dynamic_pointer_cast<VFrame30::FblItemRect>(item);

			if (fblItemRect == nullptr)
			{
				continue;
			}

			m_afbItems.push_back(fblItemRect);
		}

		return result;
	}


	const std::shared_ptr<VFrame30::LogicScheme> ApplicationLogicData::scheme() const
	{
		return m_scheme;
	}

	std::shared_ptr<VFrame30::LogicScheme> ApplicationLogicData::scheme()
	{
		return m_scheme;
	}

	const std::shared_ptr<VFrame30::SchemeLayer> ApplicationLogicData::layer() const
	{
		return m_layer;
	}

	std::shared_ptr<VFrame30::SchemeLayer> ApplicationLogicData::layer()
	{
		return m_layer;
	}

	std::list<std::shared_ptr<VFrame30::FblItemRect>> ApplicationLogicData::afbItems() const
	{
		return m_afbItems;
	}


	// ------------------------------------------------------------------------
	//
	//		ApplicationLogicBuilder
	//
	// ------------------------------------------------------------------------


	ApplicationLogicBuilder::ApplicationLogicBuilder(DbController* db, OutputLog* log,
		int changesetId, bool debug) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug)
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

	bool ApplicationLogicBuilder::compileApplicationLogicLayer(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
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
		BranchContainer branchContainer;

		bool result = findBranches(logicScheme, layer, &branchContainer);

		if (result == false)
		{
			log()->writeError(tr("Finding branches error."), false, false);
			return false;
		}

		// Set pins' guids to branches
		//
		result = setBranchConnectionToPin(logicScheme, layer, &branchContainer);

		if (result == false)
		{
			log()->writeError("setBranchConnectionToPin function error.", false, false);
			return false;
		}

		// Associates input/outputs
		//
		result = setPinConnections(logicScheme, layer, &branchContainer);

		// Generate afb list, and set it to some container
		//
		ApplicationLogicData data;

		result = data.setData(logicScheme, layer);

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

		std::list<std::shared_ptr<VFrame30::FblItemRect>> items = data.afbItems();

		qDebug() << "";
		qDebug() << tr("Application Functional Blocks list, Scheme: %1, Layer %2").arg(logicScheme->caption()).arg(layer->name());

		for (std::shared_ptr<VFrame30::FblItemRect> i : items)
		{
			qDebug() << "";

			std::shared_ptr<VFrame30::VideoItemFblElement> fblElement = std::dynamic_pointer_cast<VFrame30::VideoItemFblElement>(i);
			std::shared_ptr<VFrame30::VideoItemInputSignal> inputElement = std::dynamic_pointer_cast<VFrame30::VideoItemInputSignal>(i);
			std::shared_ptr<VFrame30::VideoItemOutputSignal> outputElement = std::dynamic_pointer_cast<VFrame30::VideoItemOutputSignal>(i);

			if (fblElement)
			{
				std::shared_ptr<Afbl::AfbElement> afb = logicScheme->afbCollection().get(fblElement->afbGuid());

				qDebug() << afb->caption();

				const std::list<VFrame30::CFblConnectionPoint>& inputs = fblElement->inputs();
				const std::list<VFrame30::CFblConnectionPoint>& outputs = fblElement->outputs();

				for (const VFrame30::CFblConnectionPoint& in : inputs)
				{
					QString str = QString("\tInput %1, associated pins: ").arg(in.guid().toString());

					const std::list<QUuid>& assIos = in.associatedIOs();	// AssIos ))))

					for (auto apid : assIos)
					{
						str.append(QString(" %1,").arg(apid.toString()));
					}

					qDebug() << str;
				}

				for (const VFrame30::CFblConnectionPoint& out : outputs)
				{
					QString str = QString("\tOutput %1, associated pins: ").arg(out.guid().toString());

					const std::list<QUuid>& assIos = out.associatedIOs();	// AssIos ))))

					for (auto apid : assIos)
					{
						str.append(QString(" %1,").arg(apid.toString()));
					}

					qDebug() << str;
				}
			}

			if (inputElement)
			{
				const std::list<VFrame30::CFblConnectionPoint>& inputs = inputElement->inputs();
				const std::list<VFrame30::CFblConnectionPoint>& outputs = inputElement->outputs();

				assert(inputs.empty() == true);
				assert(outputs.size() == 1);
				assert(outputs.front().associatedIOs().size() > 0);

				qDebug() << "Input Element: " << inputElement->signalStrIds();

				for (const VFrame30::CFblConnectionPoint& out : outputs)
				{
					QString str = QString("\tOutput %1, associated pins: ").arg(out.guid().toString());

					const std::list<QUuid>& assIos = out.associatedIOs();	// AssIos ))))

					for (auto apid : assIos)
					{
						str.append(QString(" %1,").arg(apid.toString()));
					}

					qDebug() << str;
				}
			}

			if (outputElement)
			{
				const std::list<VFrame30::CFblConnectionPoint>& inputs = outputElement->inputs();
				const std::list<VFrame30::CFblConnectionPoint>& outputs = outputElement->outputs();

				assert(inputs.size() == 1);
				assert(inputs.front().associatedIOs().size() == 1);
				assert(outputs.empty() == true);

				qDebug() << "Output Element: " << outputElement->signalStrIds();

				QString str = QString("\tInputPin %1, associated pin: %2")
							  .arg(inputs.front().guid().toString())
							  .arg(inputs.front().associatedIOs().front().toString());

				qDebug() << str;
			}
		}

		qDebug() << "";

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//
		// END OF DEBUG
		//
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


		return true;
	}

	// Function connects all links, and compose them into branches
	//
	bool ApplicationLogicBuilder::findBranches(std::shared_ptr<VFrame30::LogicScheme> logicScheme,
											   std::shared_ptr<VFrame30::SchemeLayer> layer,
											   BranchContainer* branchContainer) const
	{
		if (logicScheme.get() == nullptr ||
			layer.get() == nullptr ||
			branchContainer == nullptr)
		{
			assert(logicScheme);
			assert(layer);
			assert(branchContainer);
			return false;
		}

		std::list<std::set<QUuid>> branches;

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

		// Enum all vert and horz links and compose branches
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
			auto foundBranch = std::find_if(branches.begin(), branches.end(),
											[link](const std::set<QUuid>& b)
			{
				auto foundBranch = b.find(link->guid());
				return foundBranch != b.end();
			});

			if (foundBranch == branches.end())
			{
				std::set<QUuid> newBranch;
				newBranch.insert(link->guid());

				branches.push_front(newBranch);

				foundBranch = branches.begin();
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

		for (auto currentBranch = branches.begin();
			 currentBranch != branches.end();
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
				for (; subBranch != branches.end();)
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

						branches.erase(tmp);

						wasJoining = true;
						continue;
					}

					++subBranch;
				}
			}
		}

		// For all links in branches get its end points
		//

		branchContainer->branches.reserve(branches.size());

		for (const std::set<QUuid>& b : branches)
		{
			Branch newBranch;

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

				newBranch.links[id] = BranchLink(pt1, pt2);
			}

			branchContainer->branches.push_back(newBranch);
		}

		// DEBUG
		for (Branch& eb : branchContainer->branches)
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
						BranchContainer* branchContainer) const
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
					branchContainer->branches[branchIndex].inputPins.insert(in.guid());
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

					if (branchContainer->branches[branchIndex].outputPin.isNull() == false)
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
						branchContainer->branches[branchIndex].outputPin = out.guid();
					}
				}
			}
		}

		return result;
	}


	bool ApplicationLogicBuilder::setPinConnections(std::shared_ptr<VFrame30::LogicScheme> scheme, std::shared_ptr<VFrame30::SchemeLayer> layer,
						   BranchContainer* branchContainer)
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
					const Branch& branch = branchContainer->branches[branchIndex];

					if (branch.outputPin.isNull() == true)
					{
						assert(branch.outputPin.isNull() == false);

						log()->writeError(tr("LogicScheme %1 (layer %2): Internalerror in function, output pin in brach suppose to be initialized, %1.")
							.arg(__FUNCTION__), false, true);

						result = false;
						return result;
					}

					// Set sourche pin guid for this pin
					//
					in.AddAssociattedIOs(branch.outputPin);
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
					const Branch& branch = branchContainer->branches[branchIndex];

					// Set destination pins guid for this pin
					//
					for (const QUuid& dstid : branch.inputPins)
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

}
