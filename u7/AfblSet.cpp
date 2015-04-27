#include "AfblSet.h"

AfblSet::AfblSet()
{
}

bool AfblSet::loadFromDatabase(DbController* db, OutputLog* log, QWidget* parent)
{
	if (db == nullptr || log == nullptr)
	{
		assert(db);
		assert(log);

		return false;
	}

	std::vector<DbFileInfo> files;
	if (db->getFileList(&files, db->afblFileId(), "afb", parent) == false)
	{
		log->writeError("Could not get afb files list!", true, true);
		return false;
	}

	for (auto fi : files)
	{
		std::shared_ptr<DbFile> f;

		if (db->getLatestVersion(fi, &f, parent) == false)
		{
			log->writeError("Getting the latest version of the file " + fi.fileName() + " failed!", true, true);
			return false;
		}

		QByteArray data;
		f->swapData(data);

		AfbElement e;
		if (e.loadFromXml(data) == false)
		{
			log->writeError("Reading contents of the file " + fi.fileName() + " failed!", true, true);
			return false;
		}
		items.push_back(e);
	}

	return true;
}
