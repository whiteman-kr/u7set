#include "ApplicationLogicBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/VideoItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

namespace Builder
{

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
			m_log->writeMessage(tr("There is no appliction logic files in the project."));
			return true;
		}

		// Compile application logic
		//
		m_log->writeMessage(tr("Compiling..."));

		for (std::shared_ptr<VFrame30::LogicScheme> scheme : schemes)
		{
			m_log->writeMessage(scheme->caption());

			ok = compileApplicationLogicScheme(scheme.get());

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
			m_log->writeError(tr("Cannot get application logic file list."));
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
			m_log->writeMessage(tr("Loading %1").arg(fi.fileName()));

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
				m_log->writeError(tr("Cannot get application logic file instances."), true);
				return false;
			}

			// Read Appliaction logic files
			//
			std::shared_ptr<VFrame30::LogicScheme> ls(dynamic_cast<VFrame30::LogicScheme*>(VFrame30::Scheme::Create(file.get()->data())));

			if (ls == nullptr)
			{
				assert(ls != nullptr);
				m_log->writeError(tr("File loading error."), true);
				return false;
			}

			// Add LogicScheme to result
			//
			out->push_back(ls);
		}

		return true;
	}

	bool ApplicationLogicBuilder::compileApplicationLogicScheme(VFrame30::LogicScheme* logicScheme)
	{
		if (logicScheme == nullptr)
		{
			assert(false);
			return false;
		}

		// --
		logicScheme->BuildFblConnectionMap();

		// Find layer for compilation
		//
		bool layerFound = false;
		bool ok = false;

		for (std::shared_ptr<VFrame30::SchemeLayer> l : logicScheme->Layers)
		{
			qDebug() << Q_FUNC_INFO << " WARNING!!!! Compiling ALL layers, in future compile just l->compile() LAYER!!!!";

			//if (l->compile() == true)
			{
				layerFound = true;
				ok = compileApplicationLogicLayer(logicScheme, l.get());

				if (ok == false)
				{
					return false;
				}
			}
		}

		if (layerFound == false)
		{
			m_log->writeError(tr("There is no compile layer in the scheme."));
			return false;
		}

		return true;
	}

	bool ApplicationLogicBuilder::compileApplicationLogicLayer(VFrame30::LogicScheme* logicScheme, VFrame30::SchemeLayer* layer)
	{
		if (logicScheme == nullptr || layer == nullptr)
		{
			assert(logicScheme);
			assert(layer);
			return false;
		}

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
		std::list<std::set<QUuid>> branches;	// This list contains full branches

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

		// DEBUG
		//
		qDebug() << "";
		qDebug() << "Branches before joining";

		for (std::set<QUuid>& b : branches)
		{
			qDebug() << "--";
			for (const QUuid& q : b)
			{
				qDebug() << q;
			}
		}

		qDebug() << "";


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

		// DEBUG
		//
		qDebug() << "";
		qDebug() << "Branches after joining";

		for (std::set<QUuid>& b : branches)
		{
			qDebug() << "--";
			for (const QUuid& q : b)
			{
				qDebug() << q;
			}
		}

		qDebug() << "";


		return true;
	}


	DbController* ApplicationLogicBuilder::db()
	{
		return m_db;
	}

	OutputLog* ApplicationLogicBuilder::log()
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
