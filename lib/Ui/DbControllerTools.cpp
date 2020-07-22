#include <QObject>
#include "../lib/DbController.h"
#include "DbControllerTools.h"

std::pair<int, std::vector<int>> DbControllerTools::showSelectFolderDialog(DbController* db, int parentFileId, int currentSelectionFileId, bool showRootFile, QWidget* parentWidget)
{
	// Show dialog with file tree to select file, can be used as parent.
	// function returns selected file id or -1 if operation canceled
	//
	QDialog d(parentWidget, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	d.setWindowTitle(QObject::tr("Select parent"));

	// --
	//
	QLabel* textLabel = new QLabel(QObject::tr("Select parent for new file"));

	QTreeWidget* treeWidget = new QTreeWidget;
	treeWidget->setSortingEnabled(true);
	treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);
	treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	treeWidget->setHeaderLabel("File");

	DbFileTree files;

	if (bool ok = db->getFileListTree(&files, parentFileId, true, parentWidget);
		ok == false)
	{
		return {-1, {}};
	}

	files.removeIf([](const DbFileInfo& fi)
		{
			return fi.directoryAttribute() == false;
		});

	std::shared_ptr<DbFileInfo> schemaFile = files.rootFile();		// SchemaFile
	Q_ASSERT(schemaFile->directoryAttribute() == true);

	static QIcon staticFolderIcon(":/Images/Images/SchemaFolder.svg");
	const QIcon* const ptrToIcon = &staticFolderIcon;
	QTreeWidgetItem* treeItemToSelect = nullptr;

	std::function<void(std::shared_ptr<DbFileInfo>, QTreeWidgetItem*)> addChilderenFilesFunc =
		[&addChilderenFilesFunc, &files, treeWidget, currentSelectionFileId, &treeItemToSelect, ptrToIcon](std::shared_ptr<DbFileInfo> parent, QTreeWidgetItem* parentTreeItem)
		{
			Q_ASSERT(parent->isNull() == false);

			const auto& childeren = files.children(parent->fileId());

			for (auto file : childeren)
			{
				if (file->isNull() == true ||
					file->directoryAttribute() == false)
				{
					Q_ASSERT(file->isNull() == false);
					Q_ASSERT(file->directoryAttribute() == true);
					return;
				}

				QTreeWidgetItem* treeItem = nullptr;

				if (parentTreeItem == nullptr)
				{
					treeItem = new QTreeWidgetItem(treeWidget, {file->fileName()}, file->fileId()) ;
					treeWidget->addTopLevelItem(treeItem);
				}
				else
				{
					treeItem = new QTreeWidgetItem(parentTreeItem, {file->fileName()}, file->fileId()) ;
				}
				treeItem->setIcon(0, *ptrToIcon);

				addChilderenFilesFunc(file, treeItem);

				if (file->fileId() == currentSelectionFileId)
				{
					treeItem->setSelected(true);
					treeItemToSelect = treeItem;
				}
			}
		};

	QTreeWidgetItem* rootTreeItem = nullptr;
	if (showRootFile == true)
	{
		rootTreeItem = new QTreeWidgetItem(treeWidget, {schemaFile->fileName()}, schemaFile->fileId()) ;
		rootTreeItem->setIcon(0, staticFolderIcon);
		treeWidget->addTopLevelItem(rootTreeItem);

		if (schemaFile->fileId() == currentSelectionFileId)
		{
			rootTreeItem->setSelected(true);
			treeItemToSelect = rootTreeItem;
		}
	}

	addChilderenFilesFunc(schemaFile, rootTreeItem);

	if (treeItemToSelect != nullptr)
	{
		treeWidget->scrollToItem(treeItemToSelect);
	}

	treeWidget->expandRecursively(QModelIndex(), 1);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(textLabel);
	layout->addWidget(treeWidget);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	int result = d.exec();
	if (result == QDialog::Accepted)
	{
		QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();
		if (selected.size() != 1)
		{
			return {-1, {}};
		}

		std::vector<int> selectedParents;	// FileID of all parent items of selected item

		QTreeWidgetItem* selectedParent = selected.front()->parent();
		while (selectedParent != nullptr)
		{
			selectedParents.push_back(selectedParent->type());

			selectedParent = selectedParent->parent();
		}

		return {selected.front()->type(), selectedParents};
	}

	return {-1, {}};
}

