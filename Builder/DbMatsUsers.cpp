#include "DbMatsUsers.h"
#include <DbLib/DbController.h>
#include "../lib/ConstStrings.h"

namespace Builder
{
	bool DbMatsUserStorage::load(DbController* db, QString& errorCode)
	{
		const QString& fileName = File::MATSUSERS_XML;

		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		// Load the file from the database
		//
		std::vector<DbFileInfo> fileList;
		int fileId = db->systemFileId(DbDir::EtcDir);

		bool ok = db->getFileList(&fileList, fileId, fileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			// No such file, no users
			return true;
		}

		std::shared_ptr<DbFile> file = nullptr;
		ok = db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			return false;
		}

		QByteArray data;
		file->swapData(data);

		// Load MATS users from XML
		//
		return loadFromByteArray(data, errorCode);
	}

	bool DbMatsUserStorage::save(DbController* db, const QString& comment) const
	{
		const QString& fileName = File::MATSUSERS_XML;


		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		QByteArray data;

		bool ok = saveToByteArray(data);
		if (ok == false)
		{
			return false;
		}

		// save to db
		//
		std::shared_ptr<DbFile> file = nullptr;
		std::vector<DbFileInfo> fileList;
		int fileId = db->systemFileId(DbDir::EtcDir);

		ok = db->getFileList(&fileList, fileId, fileName, true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			// create a file, if it does not exists
			//
			std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
			pf->setFileName(fileName);

			if (db->addFile(pf, fileId, nullptr) == false)
			{
				return false;
			}

			ok = db->getFileList(&fileList, fileId, fileName, true, nullptr);
			if (ok == false || fileList.size() != 1)
			{
				return false;
			}
		}

		ok = db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			return false;
		}

		if (file->state() != E::VcsState::CheckedOut)
		{
			if (db->checkOut(fileList[0], nullptr) == false)
			{
				return false;
			}
		}

		file->swapData(data);

		if (db->setWorkcopy(file, nullptr) == false)
		{
			return false;
		}

		if (db->checkIn(fileList[0], comment, nullptr) == false)
		{
			return false;
		}

		return true;
	}
}
