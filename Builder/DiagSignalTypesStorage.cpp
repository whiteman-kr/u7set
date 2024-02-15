#include "DiagSignalTypesStorage.h"
#include "../UtilsLib/WUtils.h"

//
//
// DiagSignalTypesStorage
//
//

DiagSignalTypesStorage::DiagSignalTypesStorage(DbController* db) :
	DbObjectStorage(db, db->systemFileId(DbDir::DiagSignalTypesDir))
{
}

DiagSignalTypesStorage::~DiagSignalTypesStorage()
{
}

void DiagSignalTypesStorage::get(std::vector<Hardware::DiagSignalType>* types) const
{
	TEST_PTR_RETURN(types);

	types->clear();
	types->reserve(count());

	for(const auto& object : m_objectsVector)
	{
		types->emplace_back(object->diagSignalType());
	}
}

std::shared_ptr<Hardware::DiagSignalTypeObject> DiagSignalTypesStorage::get(const QString& diagSignalTypeId) const
{
	for (const std::shared_ptr<Hardware::DiagSignalTypeObject>& dst : m_objectsVector)
	{
		Q_ASSERT(dst);

		if (dst != nullptr &&
			dst->signalTypeId() == diagSignalTypeId)
		{
			return dst;
		}
	}

	return std::shared_ptr<Hardware::DiagSignalTypeObject>();
}

bool DiagSignalTypesStorage::hasSignalTypeId(const QString& diagSignalTypeId) const
{
	return std::find_if(m_objectsVector.begin(), m_objectsVector.end(), [&diagSignalTypeId](const auto& dst)
						{
							return dst != nullptr && dst->signalTypeId() == diagSignalTypeId;
						}) != m_objectsVector.end();
}

int DiagSignalTypesStorage::count() const
{
	return static_cast<int>(m_objectsVector.size());
}

bool DiagSignalTypesStorage::load(QString* errorMessage)
{
	if (m_db == nullptr ||
		errorMessage == nullptr)
	{
		Q_ASSERT(m_db);
		Q_ASSERT(errorMessage);
		return false;
	}

	clear();

	// Load the file from the database
	//
	std::vector<DbFileInfo> fileList;

	bool ok = m_db->getFileList(&fileList, DbDir::DiagSignalTypesDir, Db::File::DiagSignalTypeFileExtension, true, nullptr);
	if (ok == false)
	{
		*errorMessage = m_db->lastError();
		return false;
	}

	if (fileList.empty() == true)
	{
		return true;
	}

	std::vector<std::shared_ptr<DbFile>> files;
	ok = m_db->getLatestVersion(fileList, &files, nullptr);

	if (ok == false)
	{
		*errorMessage = m_db->lastError();
		return false;
	}

	for (const auto& f : files)
	{
		auto diagSignalType = Hardware::DiagSignalTypeObject::Create(f->data());

		setFileInfo(diagSignalType->uuid(), *f);
		add(diagSignalType->uuid(), diagSignalType);
	}

	return true;
}

bool DiagSignalTypesStorage::save(const QUuid& uuid, QString* errorMessage)
{
	if (m_db == nullptr ||
		errorMessage == nullptr)
	{
		Q_ASSERT(m_db);
		Q_ASSERT(errorMessage);
		return false;
	}

	auto dst = get(uuid);
	if (dst == nullptr)
	{
		Q_ASSERT(dst);
		return false;
	}

	Proto::Envelope message;
	dst->Save(&message);

	QByteArray data;

	int size = static_cast<int>(message.ByteSizeLong());
	data.resize(size);

	message.SerializeToArray(data.data(), size);

	// save to db
	//
	DbFileInfo fi = fileInfo(dst->uuid());

	const QString& fileName = QString("%1.%2").arg(dst->signalTypeId().toLower()).arg(Db::File::DiagSignalTypeFileExtension);

	if (fi.isNull() == true)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
		file->setFileName(fileName);
		file->swapData(data);

		if (m_db->addFile(file, DbDir::DiagSignalTypesDir, nullptr) == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		setFileInfo(dst->uuid(), *file);
	}
	else
	{
		// save to existing file
		//
		if (fi.fileName() != fileName)
		{
			// Filename was changed, rename the file
			//
			DbFileInfo fiUpdated;

			if (m_db->renameFile(fi, fileName, &fiUpdated, nullptr) == false)
			{
				*errorMessage = m_db->lastError();
				return false;
			}

			fi = fiUpdated;
		}

		std::shared_ptr<DbFile> file;

		bool ok = m_db->getLatestVersion(fi, &file, nullptr);
		if (ok == false || file == nullptr)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		if (file->state() != E::VcsState::CheckedOut)
		{
			*errorMessage = QString("File %1 state is not CheckedOut").arg(file->fileName());
			return false;
		}

		file->swapData(data);

		if (m_db->setWorkcopy(file, nullptr) == false)
		{
			*errorMessage = m_db->lastError();
			return false;
		}

		setFileInfo(dst->uuid(), *file);
	}

	return true;
}
