#pragma once

class DbController;

class DbControllerTools
{
public:
	static std::pair<int, std::vector<int>> showSelectFolderDialog(DbController* db, int parentFileId, int currentSelectionFileId, bool showRootFile, QWidget* parentWidget);

};


