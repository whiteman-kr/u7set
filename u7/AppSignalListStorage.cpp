#include "AppSignalListStorage.h"

//
// AppSignalListStorage
//

AppSignalListStorage::AppSignalListStorage(DbController* db)
	: DbObjectStorage(db, db->systemFileId(DbDir::AppSignalListsDir))
{
}

bool AppSignalListStorage::load(QString* errorMessage)
{
	if (m_db == nullptr ||
		errorMessage == nullptr)
	{
		assert(m_db);
		assert(errorMessage);
		return false;
	}

	// Get Busses
	//
	std::vector<DbFileInfo> fileList;

	bool ok = m_db->getFileList(&fileList, DbDir::AppSignalListsDir, Db::File::AppSignalListFileExtension, false /*removeDeleted*/, nullptr);
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
		if ((f->deleted() == true || f->action() == E::VcsItemAction::Deleted) && 
			f->state() == E::VcsState::CheckedIn)
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
	if (m_db == nullptr ||
		errorMessage == nullptr)
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
	bool ok = list->SaveData(&message);
	if (ok == false)
	{
		*errorMessage = QString("Error saving list %1.").arg(list->id());
		return false;
	}

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

		ok = m_db->addFile(file, DbDir::AppSignalListsDir, nullptr);

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
		ok = m_db->getLatestVersion(fi, &file, nullptr);
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
