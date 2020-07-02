#pragma once

#include "MainTabPage.h"
#include "../lib/DbStruct.h"
#include "GlobalMessanger.h"
#include "../lib/Ui/FilesTreeView.h"




class FilesTabPage : public MainTabPage
{
	Q_OBJECT
public:
	FilesTabPage(DbController* dbcontroller, QWidget* parent);

protected:
	void createActions();
	void setActionState();

public slots:
	void projectOpened();
	void projectClosed();

	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());

private slots:
	void compareObject(DbChangesetObject object, CompareData compareData);

	// Data
	//
private:
	FileTreeView* m_fileView = nullptr;
	FileTreeModel* m_fileModel = nullptr;
	FileTreeProxyModel* m_proxyModel = nullptr;


    QStringList m_editableExtensions;

	//
	QAction* m_addFileAction = nullptr;
	QAction* m_viewFileAction = nullptr;
    QAction* m_editFileAction = nullptr;
    QAction* m_deleteFileAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction1 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction2 = nullptr;
	QAction* m_getLatestVersionAction = nullptr;
	QAction* m_getLatestTreeVersionAction = nullptr;
	QAction* m_importWorkingcopyAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction3 = nullptr;
	QAction* m_refreshAction = nullptr;
};


