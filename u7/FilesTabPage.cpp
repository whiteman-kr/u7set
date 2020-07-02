#include "FilesTabPage.h"
#include "../lib/DbController.h"
#include "GlobalMessanger.h"
#include "Forms/ComparePropertyObjectDialog.h"

//
//
//	FilesTabPage
//
//
FilesTabPage::FilesTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

    m_editableExtensions << tr("afb");
	m_editableExtensions << tr("xml");
	m_editableExtensions << tr("xsd");
    m_editableExtensions << tr("descr");
	m_editableExtensions << tr("js");

	//
	// Controls
	//
	m_fileModel = new FileTreeModel(dbcontroller, DbFileInfo::fullPathToFileName(Db::File::RootFileName), this, this);

	m_proxyModel = new FileTreeProxyModel(this);
	m_proxyModel->setSourceModel(m_fileModel);

	m_fileView = new FileTreeView(dbcontroller);
	m_fileView->setModel(m_proxyModel);

	m_fileView->setSortingEnabled(true);
	connect(m_fileView->header(), &QHeaderView::sortIndicatorChanged, [this](int index, Qt::SortOrder order)
	{
		m_fileView->sortByColumn(index, order);
	});
	m_fileView->sortByColumn(0, Qt::AscendingOrder);

	// Create Actions
	//
	createActions();

	//
	// Set context menu to Equipment View
	//
	m_fileView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// -----------------
	m_fileView->addAction(m_addFileAction);
	m_fileView->addAction(m_viewFileAction);
    m_fileView->addAction(m_editFileAction);
	m_fileView->addAction(m_deleteFileAction);

	// -----------------
	m_fileView->addAction(m_SeparatorAction1);
	m_fileView->addAction(m_checkOutAction);
	m_fileView->addAction(m_checkInAction);
	m_fileView->addAction(m_undoChangesAction);
	m_fileView->addAction(m_historyAction);
	m_fileView->addAction(m_compareAction);
	// -----------------
	m_fileView->addAction(m_SeparatorAction2);
	m_fileView->addAction(m_getLatestVersionAction);
	m_fileView->addAction(m_getLatestTreeVersionAction);
	m_fileView->addAction(m_importWorkingcopyAction);
	// -----------------
	m_fileView->addAction(m_SeparatorAction3);
	m_fileView->addAction(m_refreshAction);
	// -----------------

	//
	// Layouts
	//

	// Left layout (project list)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();
	pLeftLayout->addWidget(m_fileView);

	// Right layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();

//	pRightLayout->addWidget(m_pNewProject);

	// Main Layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addLayout(pRightLayout);

	setLayout(pMainLayout);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &FilesTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &FilesTabPage::projectClosed);

	connect(m_fileView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FilesTabPage::selectionChanged);
	connect(m_fileModel, &FileTreeModel::dataChanged, this, &FilesTabPage::modelDataChanged);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &FilesTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

void FilesTabPage::createActions()
{

	m_addFileAction = new QAction(tr("Add file"), this);
	m_addFileAction->setStatusTip(tr("Add file..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, m_fileView, &FileTreeView::addFile);

	m_viewFileAction = new QAction(tr("View file"), this);
	m_viewFileAction->setStatusTip(tr("View file..."));
	m_viewFileAction->setEnabled(false);
	connect(m_viewFileAction, &QAction::triggered, m_fileView, &FileTreeView::viewFile);

	m_editFileAction = new QAction(tr("Edit file"), this);
    m_editFileAction->setStatusTip(tr("Edit file..."));
    m_editFileAction->setEnabled(false);
    connect(m_editFileAction, &QAction::triggered, m_fileView, &FileTreeView::editFile);

    m_deleteFileAction = new QAction(tr("Delete file"), this);
	m_deleteFileAction->setStatusTip(tr("Delete file..."));
	m_deleteFileAction->setEnabled(false);
	connect(m_deleteFileAction, &QAction::triggered, m_fileView, &FileTreeView::deleteFile);

	//----------------------------------
	m_SeparatorAction1 = new QAction(this);
	m_SeparatorAction1->setSeparator(true);

	m_checkOutAction = new QAction(tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, m_fileView, &FileTreeView::checkOutFile);

	m_checkInAction = new QAction(tr("CheckIn"), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, m_fileView, &FileTreeView::checkInFile);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, m_fileView, &FileTreeView::undoChangesFile);

	m_historyAction = new QAction(tr("History..."), this);
	m_historyAction->setStatusTip(tr("Show check in history"));
	m_historyAction->setEnabled(false);
	connect(m_historyAction, &QAction::triggered, m_fileView, &FileTreeView::showHistory);

	m_compareAction = new QAction(tr("Compare..."), this);
	m_compareAction->setStatusTip(tr("Compare file"));
	m_compareAction->setEnabled(false);
	connect(m_compareAction, &QAction::triggered, m_fileView, &FileTreeView::showCompare);

	//----------------------------------
	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);

	m_getLatestVersionAction = new QAction(tr("Get Latest Version"), this);
	m_getLatestVersionAction->setStatusTip(tr("Get the latest version (workcopy if cheked out)"));
	m_getLatestVersionAction->setEnabled(false);
	connect(m_getLatestVersionAction, &QAction::triggered, m_fileView, &FileTreeView::getLatestVersion);

	m_getLatestTreeVersionAction = new QAction(tr("Get Latest Tree Version"), this);
	m_getLatestTreeVersionAction->setStatusTip(tr("Get the latest tree version (workcopy if cheked out)"));
	m_getLatestTreeVersionAction->setEnabled(false);
	connect(m_getLatestTreeVersionAction, &QAction::triggered, m_fileView, &FileTreeView::getLatestTreeVersion);

	m_importWorkingcopyAction = new QAction(tr("Import Workingcopy..."), this);
	m_importWorkingcopyAction->setStatusTip(tr("Import workingcopy disk file to project file..."));
	m_importWorkingcopyAction->setEnabled(false);
	connect(m_importWorkingcopyAction, &QAction::triggered, m_fileView, &FileTreeView::setWorkcopy);


	//----------------------------------
	m_SeparatorAction3 = new QAction(this);
	m_SeparatorAction3->setSeparator(true);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh object list"));
	m_refreshAction->setEnabled(false);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, m_fileView, &FileTreeView::refreshFileTree);
	addAction(m_refreshAction);

	return;
}

void FilesTabPage::setActionState()
{
	// Disable all
	//
	m_addFileAction->setEnabled(false);
    m_editFileAction->setEnabled(false);
	m_deleteFileAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	m_checkInAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
	m_getLatestVersionAction->setEnabled(false);
	m_getLatestTreeVersionAction->setEnabled(false);
	m_importWorkingcopyAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	// Refresh
	//
	m_refreshAction->setEnabled(true);

	// --
	//
	QModelIndexList selectedIndexList = m_fileView->selectionModel()->selectedRows();

	// Add Action
	//
	m_addFileAction->setEnabled(selectedIndexList.size() == 1);

	// Delete Items action
	//
	m_deleteFileAction->setEnabled(false);
	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_fileModel->fileItem(mi);
		assert(file);

		if (file->state() == VcsState::CheckedIn /*&&
			file->action() != VcsItemAction::Deleted*/)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
			&& file->action() != VcsItemAction::Deleted)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}
	}

	// CheckIn, CheckOut, Undo, Get/set Workcopy
	//
	bool canAnyBeCheckedIn = false;
	bool canAnyBeCheckedOut = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_fileModel->fileItem(mi);
		assert(file);

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
		{
			canAnyBeCheckedIn = true;
		}

		if (file->state() == VcsState::CheckedIn)
		{
			canAnyBeCheckedOut = true;
		}

		// Don't need to go further
		//
		if (canAnyBeCheckedIn == true &&
			canAnyBeCheckedOut == true )
		{
			break;
		}
	}

	m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);
	m_historyAction->setEnabled(selectedIndexList.size() == 1);
	m_compareAction->setEnabled(selectedIndexList.size() == 1);

	m_getLatestVersionAction->setEnabled(selectedIndexList.isEmpty() == false);
	m_getLatestTreeVersionAction->setEnabled(selectedIndexList.isEmpty() == false);
	m_importWorkingcopyAction->setEnabled(canAnyBeCheckedIn && selectedIndexList.size() == 1);

    // Enable edit only files with several extensions!
    //
    bool editableExtension = false;
    for (const QModelIndex& mi : selectedIndexList)
    {
        const FileTreeModelItem* file = m_fileModel->fileItem(mi);
        assert(file);

        QString ext = QFileInfo(file->fileName()).suffix();
        if (m_editableExtensions.contains(ext))
        {
            editableExtension = true;
            break;
        }
    }

	m_viewFileAction->setEnabled(editableExtension && selectedIndexList.size() == 1);
    m_editFileAction->setEnabled(editableExtension && canAnyBeCheckedIn && selectedIndexList.size() == 1);

	return;
}

void FilesTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void FilesTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

void FilesTabPage::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	setActionState();

	return;
}

void FilesTabPage::modelDataChanged(const QModelIndex& topLeft,
									const QModelIndex& bottomRight, const QVector<int>& roles /*= QVector<int>()*/)
{
	Q_UNUSED(topLeft);
	Q_UNUSED(bottomRight);
	Q_UNUSED(roles);

	setActionState();

	return;
}

void FilesTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension
	//
	std::array<QString, 5> extensions = {".xml", ".xsd", ".descr", ".txt", ".afb"};

	bool extFound = false;
	QString fileName = object.name();

	for (const QString& ext : extensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			extFound = true;
			break;
		}
	}

	if (extFound == false)
	{
		return;
	}

	// Get vesrions from the project database
	//
	QString source;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceChangeset, &outFile, this);
			if (ok == true)
			{
				source = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceDate, &outFile, this);
			if (ok == true)
			{
				source = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				source = QString::fromUtf8(outFile->data());
			}
		}
		break;
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	QString target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetChangeset, &outFile, this);
			if (ok == true)
			{
				target = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetDate, &outFile, this);
			if (ok == true)
			{
				target = QString::fromUtf8(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				target = QString::fromUtf8(outFile->data());
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}
