#include "LmDescriptionSet.h"

namespace Builder
{
	//
	//
	//		LmDescriptionSet
	//
	bool LmDescriptionSet::loadFile(IssueLogger* log, DbController* db, QString objectId, QString fileName)
	{
		Q_ASSERT(log);
		Q_ASSERT(db);
		Q_ASSERT(objectId.isEmpty() == false);
		Q_ASSERT(fileName.isEmpty() == false);

		std::vector<DbFileInfo> fileList;

		bool result = db->getFileList(&fileList, db->afblFileId(), fileName, true, nullptr);
		if (result == false)
		{
			log->errPDB2001(db->afblFileId(), fileName, db->lastError());
			return false;
		}

		if (fileList.size() != 1)
		{
			log->errEQP6004(objectId, fileName, QUuid());
			return false;
		}

		// Get description file from the DB
		//
		std::shared_ptr<DbFile> file;
		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false)
		{
			log->errPDB2002(fileList[0].fileId(), fileList[0].fileName(), db->lastError());
			return false;
		}

		// Parse file
		//
		QString parseErrorMessage;

		std::shared_ptr<LmDescription> lmd = std::make_shared<LmDescription>();

		result = lmd->load(file->data(), &parseErrorMessage);

		if (result == false)
		{
			QString errorMsg = QString("Cannot parse file %1. Error message: %2").arg(fileName).arg(parseErrorMessage);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined, errorMsg);
			return false;
		}

		add(fileName, lmd);

		m_rawLmDescriptions[fileName] = file->data();

		return true;
	}

	std::pair<QString, bool> LmDescriptionSet::rowFile(QString fileName) const
	{
		auto it = m_rawLmDescriptions.find(fileName);
		if (it == m_rawLmDescriptions.end())
		{
			return {QString(), false};
		}

		return {it->second, true};
	}
}
