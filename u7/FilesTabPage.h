#ifndef FILESTABPAGE_H
#define FILESTABPAGE_H

#include "MainTabPage.h"
#include "../include/DbStruct.h"
#include "FileView.h"

class FilesTabPage : public MainTabPage
{

public:
	FilesTabPage(DbStore* dbstore, QWidget* parent);

protected:
	void CreateActions();

public slots:
	void projectOpened();
	void projectClosed();

private slots:

	// Data
	//
private:
	FileView* m_filesView;
};

#endif // FILESTABPAGE_H
