#include "DiagSignalTypesStorage.h"

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

std::shared_ptr<Hardware::DiagSignalType> DiagSignalTypesStorage::get(const QString& diagSignalTypeId) const
{
	for (const std::shared_ptr<Hardware::DiagSignalType>& dst : m_objectsVector)
	{
		Q_ASSERT(dst);

		if (dst != nullptr &&
			dst->signalTypeId() == diagSignalTypeId)
		{
			return dst;
		}
	}

	return std::shared_ptr<Hardware::DiagSignalType>();
}

bool DiagSignalTypesStorage::hasSignalTypeId(const QString& diagSignalTypeId) const
{
	return std::find_if(m_objectsVector.begin(), m_objectsVector.end(), [&diagSignalTypeId](const std::shared_ptr<Hardware::DiagSignalType>& dst) 
		{
							return dst != nullptr && dst->signalTypeId() == diagSignalTypeId;
		}
	) != m_objectsVector.end();
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
		std::shared_ptr<Hardware::DiagSignalType> diagSignalType = Hardware::DiagSignalType::Create(f->data());

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

	std::shared_ptr<Hardware::DiagSignalType> dst = get(uuid);
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

void DiagSignalTypesStorage::writeToXml(XmlWriteHelper& xml) const
{
	std::map<QString, std::shared_ptr<Hardware::DiagSignalType>> sortedTypes;	// DiagSignalTypeID => DiagSignalType

	for (const auto& diagSignalType : m_objectsVector)
	{
		TEST_PTR_CONTINUE(diagSignalType);

		sortedTypes.emplace(diagSignalType->signalTypeId(), diagSignalType);
	}

	xml.writeStartElement(XmlElement::DIAG_SIGNAL_TYPES);
	xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(sortedTypes.size()));

	for(const auto& [id, diagSignalType] : sortedTypes)
	{
		diagSignalType->writeToXml(xml);
	}

	xml.writeEndElement();		//	XmlElement::DIAG_SIGNAL_TYPES
}

bool DiagSignalTypesStorage::readFromXml(XmlReadHelper& xml)
{
	m_objectsVector.clear();

	bool result = true;

	result &= xml.findElement(XmlElement::DIAG_SIGNAL_TYPES);

	RETURN_IF_FALSE(result);

	int count = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

	for(int i = 0; i < count; i++)
	{

	}

	return result;
}

