#include "LogicModuleSet.h"

LogicModuleSet::LogicModuleSet()
{

}

bool LogicModuleSet::loadFile(DbController* db, QString fileName, QString* errorString)
{
	if (errorString != nullptr)
	{
		errorString->clear();
	}

	if (db == nullptr ||
		fileName.isEmpty() == true)
	{
		assert(db);
		assert(fileName.isEmpty() == false);

		if (errorString != nullptr)
		{
			*errorString = tr("Function input parameters error");
		}

		return false;
	}

	std::vector<DbFileInfo> fileList;

	bool result = db->getFileList(&fileList, db->afblFileId(), fileName, true, nullptr);
	if (result == false)
	{
		if (errorString != nullptr)
		{
			*errorString = tr("Error of getting file list from the database, parent file ID %1, filter '%2', database message '%3'.")
							  .arg(db->afblFileId())
							  .arg(fileName)
							  .arg(db->lastError());
		}

		return false;
	}

	if (fileList.size() != 1)
	{
		if (errorString != nullptr)
		{
			*errorString = tr("File LmDescriptionFile %1 is not found.").arg(fileName);
		}

		return false;
	}

	// Get description file from the DB
	//
	std::shared_ptr<DbFile> file;
	result = db->getLatestVersion(fileList[0], &file, nullptr);
	if (result == false)
	{
		if (errorString != nullptr)
		{
			*errorString = tr("Getting file instance error, file ID %1, file name '%2', database message '%3'.")
						   .arg(fileList[0].fileId())
						   .arg(fileList[0].fileName())
						   .arg(db->lastError());
		}

		return false;
	}

	// Parse file
	//
	QString parseErrorMessage;
	std::shared_ptr<LmDescription> lmd = std::make_shared<LmDescription>();

	result = lmd->load(file->data(), &parseErrorMessage);

	if (result == false)
	{
		if (errorString != nullptr)
		{
			*errorString = tr("Cannot parse file %1. Error message: %2").arg(fileName).arg(parseErrorMessage);
		}
		return false;
	}

	add(fileName, lmd);

	return true;
}

bool LogicModuleSet::has(QString fileName) const
{
	return m_lmDescriptions.count(fileName) > 0;
}

void LogicModuleSet::add(QString fileName, std::shared_ptr<LmDescription> lm)
{
	assert(lm);
	m_lmDescriptions[fileName] = lm;
}

std::shared_ptr<LmDescription> LogicModuleSet::get(QString fileName) const
{
	auto it = m_lmDescriptions.find(fileName);

	if (it == std::end(m_lmDescriptions))
	{
		return std::shared_ptr<LmDescription>();
	}
	else
	{
		return it->second;
	}
}

std::shared_ptr<LmDescription> LogicModuleSet::get(QString fileName)
{
	auto it = m_lmDescriptions.find(fileName);

	if (it == std::end(m_lmDescriptions))
	{
		return std::shared_ptr<LmDescription>();
	}
	else
	{
		return it->second;
	}
}

std::shared_ptr<LmDescription> LogicModuleSet::get(const Hardware::DeviceModule* logicModule) const
{
	if (logicModule == nullptr ||
		logicModule->isFSCConfigurationModule() == false)
	{
		assert(logicModule);
		assert(logicModule->isFSCConfigurationModule());
		return std::shared_ptr<LmDescription>();
	}

	auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
	if (lmDescriptionFileProp == nullptr)
	{
		assert(lmDescriptionFileProp);
		return std::shared_ptr<LmDescription>();
	}

	QString lmDescriptionFile = lmDescriptionFileProp->value().toString();
	if (lmDescriptionFile.isEmpty() == true)
	{
		assert(lmDescriptionFile.isEmpty() == false);
		return std::shared_ptr<LmDescription>();
	}

	return get(lmDescriptionFile);
}

std::shared_ptr<LmDescription> LogicModuleSet::get(Hardware::DeviceModule* logicModule)
{
	if (logicModule == nullptr ||
		logicModule->isFSCConfigurationModule() == false)
	{
		assert(logicModule);
		assert(logicModule->isFSCConfigurationModule());
		return std::shared_ptr<LmDescription>();
	}

	auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
	if (lmDescriptionFileProp == nullptr)
	{
		assert(lmDescriptionFileProp);
		return std::shared_ptr<LmDescription>();
	}

	QString lmDescriptionFile = lmDescriptionFileProp->value().toString();
	if (lmDescriptionFile.isEmpty() == true)
	{
		assert(lmDescriptionFile.isEmpty() == false);
		return std::shared_ptr<LmDescription>();
	}

	return get(lmDescriptionFile);
}

QString LogicModuleSet::lmDescriptionFile(const Hardware::DeviceModule* logicModule)
{
	assert(logicModule);
	assert(logicModule->isFSCConfigurationModule());
	return LmDescription::lmDescriptionFile(logicModule);
}
