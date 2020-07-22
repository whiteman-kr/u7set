#ifndef DBCONTROLLERTOOLS_H
#define DBCONTROLLERTOOLS_H

class DbController;

class DbControllerTools
{
public:
	static std::pair<int, std::vector<int>> showSelectFolderDialog(DbController* db, int parentFileId, int currentSelectionFileId, bool showRootFile, QWidget* parentWidget);

};


#endif // DBCONTROLLERTOOLS_H
