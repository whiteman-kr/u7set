#include "BusStorage.h"

//
// BusStorage
//

BusStorage::BusStorage(DbController* db, QWidget* parentWidget)
	:DbObjectStorage(db, parentWidget, db->busTypesFileId())
{

}

bool BusStorage::load()
{
	if (m_db == nullptr)
	{
		assert(m_db);
		return false;
	}

	// Get Busses
	//

	std::vector<DbFileInfo> fileList;

	bool ok = m_db->getFileList(&fileList, m_db->busTypesFileId(), BusFileExtension, true, nullptr);
	if (ok == false)
	{
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
		QString errorMessage;

		ok = bus.load(f->data(), &errorMessage);

		if (ok == false)
		{

			return false;

		}

		setFileInfo(bus.uuid(), *f);
		add(bus.uuid(), bus);
	}

	return true;
}

bool BusStorage::save(const QUuid& uuid)
{
	if (m_db == nullptr)
	{
		assert(m_db);
		return false;
	}

	VFrame30::Bus bus = get(uuid);

	QByteArray data;

	if (bus.save(&data) == false)
	{
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

		if (m_db->addFile(file, m_db->busTypesFileId(), m_parentWidget) == false)
		{
			return false;
		}

		setFileInfo(bus.uuid(), *file);
	}
	else
	{
		std::shared_ptr<DbFile> file = nullptr;

		// save to existing file
		//
		bool ok = m_db->getLatestVersion(fi, &file, m_parentWidget);
		if (ok == false || file == nullptr)
		{
			return false;
		}

		if (file->state() != VcsState::CheckedOut)
		{
			return false;
		}

		file->swapData(data);

		if (m_db->setWorkcopy(file, m_parentWidget) == false)
		{
			return false;
		}

		setFileInfo(bus.uuid(), *file);
	}

	return true;
}
