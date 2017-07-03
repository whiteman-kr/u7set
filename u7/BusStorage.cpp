#include "BusStorage.h"

//
// BusStorage
//

BusStorage::BusStorage(DbController* db)
	: DbObjectStorage(db, db->busTypesFileId())
{

}

bool BusStorage::load(QString* errorMessage)
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

	bool ok = m_db->getFileList(&fileList, m_db->busTypesFileId(), BusFileExtension, true, nullptr);
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

	// Parse files, create actual Busses
	//
	std::vector<VFrame30::Bus> busses;
	busses.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->deleted() == true ||
			f->action() == VcsItemAction::Deleted)
		{
			continue;
		}

		VFrame30::Bus bus;
		QString loadBusErrorMessage;

		ok = bus.load(f->data(), &loadBusErrorMessage);
		if (ok == false)
		{
			*errorMessage = QString("Load bus %1 error ").arg(f->fileName()) + loadBusErrorMessage;
			return false;
		}

		setFileInfo(bus.uuid(), *f);
		add(bus.uuid(), bus);
	}

	return true;
}

bool BusStorage::save(const QUuid& uuid, QString* errorMessage)
{
	if (m_db == nullptr ||
		errorMessage == nullptr)
	{
		assert(m_db);
		assert(errorMessage);
		return false;
	}

	VFrame30::Bus bus = get(uuid);
	QByteArray data;

	bool ok = bus.save(&data);

	if (ok == false)
	{
		*errorMessage = QString("Error saving bus %1 to xml.").arg(bus.busTypeId());
		return false;
	}

	// save to db
	//
	DbFileInfo fi = fileInfo(bus.uuid());

	if (fi.isNull() == true)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		QString fileName = QString("bustype-%1.%2").arg(bus.uuid().toString()).arg(BusFileExtension);
		fileName = fileName.remove('{');
		fileName = fileName.remove('}');

		file->setFileName(fileName);
		file->swapData(data);

		ok = m_db->addFile(file, m_db->busTypesFileId(), nullptr);

		if (ok == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		setFileInfo(bus.uuid(), *file);
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

		if (file->state() != VcsState::CheckedOut)
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

		setFileInfo(bus.uuid(), *file);
	}

	return true;
}
