#include "AppSignalListStorage.h"
#include "../AppSignalLib/AppSignal.h"

namespace Builder
{
	//
	// AppSignalListStorage
	//

	AppSignalListStorage::AppSignalListStorage(DbController* db) :
		DbObjectStorage(db, db->systemFileId(DbDir::AppSignalListsDir))
	{
	}

	std::vector<std::pair<QString, QString>> AppSignalListStorage::checkForSameIds() const
	{
		std::set<QString> ids;
		std::vector<std::pair<QString, QString>> result;

		for (auto it = begin(); it != end(); it++)
		{
			const AppSignalLists::AppSignalList* list = it->get();
			if (list == nullptr) 
			{
				Q_ASSERT(list);
				return {};
			}

			if (ids.find(list->id()) != ids.end())
			{
				// Duplicate found
				//
				result.push_back({list->id(), list->caption()});
			}
			else
			{
				ids.insert(list->id());
			}
		}

		return result;
	}
	
	bool AppSignalListStorage::load(QString* errorMessage)
	{
		if (m_db == nullptr || errorMessage == nullptr)
		{
			assert(m_db);
			assert(errorMessage);
			return false;
		}

		// Get Busses
		//
		std::vector<DbFileInfo> fileList;

		bool ok =
			m_db->getFileList(&fileList, DbDir::AppSignalListsDir, Db::File::AppSignalListFileExtension, false /*removeDeleted*/, nullptr);
		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		if (fileList.empty() == true)
		{
			return true;
		}

		// Get Busses latest version from the DB
		//
		std::vector<std::shared_ptr<DbFile>> files;

		ok = m_db->getLatestVersion(fileList, &files, nullptr);
		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		for (const std::shared_ptr<DbFile>& f : files)
		{
			if ((f->deleted() == true || f->action() == E::VcsItemAction::Deleted) && f->state() == E::VcsState::CheckedIn)
			{
				continue;
			}

			std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();

			Proto::Envelope envelope;
			if (envelope.ParseFromArray(f->data().constData(), static_cast<int>(f->data().size())) == false)
			{
				Q_ASSERT(false);
				return false;
			}

			ok = list->LoadData(envelope);
			if (ok == false)
			{
				*errorMessage = QString("Load AppSignalList %1 error").arg(f->fileName());
				return false;
			}

			setFileInfo(list->uuid(), *f);

			add(list->uuid(), list);
		}

		return true;
	}

	bool AppSignalListStorage::save(const QUuid& uuid, QString* errorMessage)
	{
		if (m_db == nullptr || errorMessage == nullptr)
		{
			Q_ASSERT(m_db);
			assert(errorMessage);
			return false;
		}

		std::shared_ptr<AppSignalLists::AppSignalList> list = get(uuid);
		if (list == nullptr)
		{
			assert(list);
			return false;
		}

		Proto::Envelope message;
		list->SaveData(&message);

		QByteArray data;
		data.resize(static_cast<int>(message.ByteSizeLong()));

		bool result = message.SerializeToArray(data.data(), static_cast<int>(message.ByteSizeLong()));
		if (result == false)
		{
			Q_ASSERT(result);
			return false;
		}

		// save to db
		//
		DbFileInfo fi = fileInfo(list->uuid());

		if (fi.isNull() == true)
		{
			// create a file, if it does not exists
			//
			std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

			QString fileName = QString("appsignallist-%1.%2").arg(list->uuid().toString()).arg(Db::File::AppSignalListFileExtension);
			fileName = fileName.remove('{');
			fileName = fileName.remove('}');

			file->setFileName(fileName);
			file->swapData(data);

			bool ok = m_db->addFile(file, DbDir::AppSignalListsDir, nullptr);
			if (ok == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			setFileInfo(uuid, *file);
		}
		else
		{
			std::shared_ptr<DbFile> file = nullptr;

			// Save to existing file
			//
			bool ok = m_db->getLatestVersion(fi, &file, nullptr);
			if (ok == false || file == nullptr)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			if (file->state() != E::VcsState::CheckedOut)
			{
				*errorMessage = QString("file %1 is not checked out.").arg(file->fileName());
				return false;
			}

			file->swapData(data);

			if (m_db->setWorkcopy(file, nullptr) == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			setFileInfo(uuid, *file);
		}

		return true;
	}


	//
	//
	// AppSignalListsProvider - this class is used to provide app signals for editing signal lists
	//
	//

	AppSignalListsProvider::AppSignalListsProvider(const std::vector<AppSignal*>& signalsVector)
	{
		for (const AppSignal* s : signalsVector)
		{
			// Remove bus and non-analog/discrete signals
			//
			if (s->isAnalog() == false && s->isDiscrete() == false)
			{
				continue;
			}

			Hash hash = ::calcHash(s->appSignalID());

			m_params[hash] = *s;
			m_params[hash].setHash(hash);
		}
	}

	int AppSignalListsProvider::signalsCount() const
	{
		return static_cast<int>(m_params.size());
	}

	std::vector<Hash> AppSignalListsProvider::signalHashes() const
	{
		std::vector<Hash> result;
		result.reserve(m_params.size());

		for (const auto& [hash, param] : m_params)
		{
			result.push_back(hash);
		}
		return result;
	}

	std::vector<AppSignalParam> AppSignalListsProvider::signalList() const
	{
		std::vector<AppSignalParam> result;
		result.reserve(m_params.size());

		for (const auto& [hash, param] : m_params)
		{
			result.push_back(param);
		}
		return result;
	}

	bool AppSignalListsProvider::signalExists(Hash hash) const
	{
		return m_params.find(hash) != m_params.end();
	}

	bool AppSignalListsProvider::signalExists(const QString& appSignalId) const
	{
		return signalExists(::calcHash(appSignalId));
	}

	bool AppSignalListsProvider::signalsExist(const QStringList& signalIds) const
	{
		return std::all_of(signalIds.begin(),
						   signalIds.end(),
						   [this](const QString& appSignalId)
						   {
							   return signalExists(appSignalId);
						   });
	}

	AppSignalParam AppSignalListsProvider::signalParam(Hash signalHash, bool* found) const
	{
		AppSignalParam result;

		auto it = m_params.find(signalHash);

		if (found != nullptr)
		{
			*found = it != m_params.end();
		}

		if (it != m_params.end())
		{
			result = it->second;
		}

		return result;
	}

	AppSignalParam AppSignalListsProvider::signalParam(const QString& appSignalId, bool* found) const
	{
		return signalParam(::calcHash(appSignalId), found);
	}
}
