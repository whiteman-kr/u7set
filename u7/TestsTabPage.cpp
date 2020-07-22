#include "TestsTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"
#include "Forms/ComparePropertyObjectDialog.h"

#ifdef _DEBUG
	#include <QAbstractItemModelTester>
#endif
//
// TestsFileTreeModel
//

TestsFileTreeModel::TestsFileTreeModel(DbController* dbcontroller, QString rootFilePath, QWidget* parentWidget, QObject* parent):
	FileTreeModel(dbcontroller, rootFilePath, parentWidget, parent)
{
}

TestsFileTreeModel::~TestsFileTreeModel()
{
}

QString TestsFileTreeModel::customColumnText(Columns column, const FileTreeModelItem* item) const
{
	// Demo function
	Q_UNUSED(column);
	Q_UNUSED(item);
	return QObject::tr("Custom %1").arg(item->fileName());
}

QString TestsFileTreeModel::customColumnName(Columns column) const
{
	// Demo function
	Q_UNUSED(column);
	return QObject::tr("Custom %1").arg(static_cast<int>(column));
}

//
// TestsTabPage
//

TestsTabPage::TestsTabPage(DbController* dbc, QWidget* parent) :
	MainTabPage(dbc, parent)
{
	m_editableExtensions << tr("js");

	createUi();

	createActions();

	restoreSettings();

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &TestsTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &TestsTabPage::projectClosed);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::buildStarted, this, &TestsTabPage::saveAllDocuments);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &TestsTabPage::compareObject);

	connect(m_testsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TestsTabPage::testsTreeSelectionChanged);
	connect(m_testsTreeModel, &FileTreeModel::dataChanged, this, &TestsTabPage::testsTreeModelDataChanged);
	connect(m_testsTreeModel, &FileTreeModel::modelReset, this, &TestsTabPage::testsTreeModelReset);

	connect(m_testsTreeView, &QTreeWidget::doubleClicked, this, &TestsTabPage::testsTreeDoubleClicked);
	connect(m_openFilesTreeWidget, &QTreeWidget::doubleClicked, this, &TestsTabPage::openFilesDoubleClicked);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);

	return;
}

TestsTabPage::~TestsTabPage()
{
	saveSettings();

	return;
}

bool TestsTabPage::hasUnsavedTests() const
{
	for (auto& it : m_openDocuments)
	{
		const TestTabPageDocument& doc = it.second;

		if (doc.modified == true)
		{
			return true;
		}
	}

	return false;
}

void TestsTabPage::saveUnsavedTests()
{
	saveAllDocuments();

	return;
}

void TestsTabPage::resetModified()
{
	for (auto& it : m_openDocuments)
	{
		TestTabPageDocument& doc = it.second;
		doc.modified = false;
	}

	return;
}

void TestsTabPage::projectOpened()
{
	this->setEnabled(true);

	m_testsTreeModel->fetch(QModelIndex());

	m_testsTreeView->expandRecursively(QModelIndex(), 1);

	return;
}


void TestsTabPage::projectClosed()
{
	closeAllDocuments();

	hideEditor();

	m_openFilesTreeWidget->clear();

	this->setEnabled(false);

	return;
}

void TestsTabPage::buildStarted()
{
	saveAllDocuments();

	return;
}

void TestsTabPage::testsTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	setActionState();

	return;
}

void TestsTabPage::testsTreeModelDataChanged(const QModelIndex& topLeft,
									const QModelIndex& bottomRight, const QVector<int>& roles /*= QVector<int>()*/)
{
	Q_UNUSED(topLeft);
	Q_UNUSED(bottomRight);
	Q_UNUSED(roles);

	setActionState();

	return;
}

void TestsTabPage::testsTreeModelReset()
{
	setActionState();

	return;
}

void TestsTabPage::testsTreeDoubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	if (m_openFileAction->isEnabled() == true)
	{
		openFile();
	}

	return;
}

void TestsTabPage::openFilesDoubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	QTreeWidgetItem* item = m_openFilesTreeWidget->currentItem();
	if (item == nullptr)
	{
		return;
	}

	bool ok = false;
	int fileId = item->data(0, Qt::UserRole).toInt(&ok);

	if (ok == true)
	{
		setCurrentDocument(fileId);
	}
	else
	{
		Q_ASSERT(ok);
	}

	return;
}

void TestsTabPage::newFile()
{
	QString fileName = QInputDialog::getText(this, "New File", tr("Enter the file name:"), QLineEdit::Normal, tr("NewFile_%1.js").arg(db()->nextCounterValue()));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	bool result = m_testsTreeView->newFile(fileName);
	if (result == true)
	{
		// addNewFile will select new document
		//
		openFile();
	}

	return;
}

void TestsTabPage::openFile()
{
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	if (selectedIndexList.size() != 1)
	{
		return;
	}

	QModelIndex& mi = selectedIndexList[0];

	FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
	if (f == nullptr)
	{
		Q_ASSERT(f);
		return;
	}

	int fileId = f->fileId();

	// Check if file is already open

	if (documentIsOpen(fileId) == true)
	{
		setCurrentDocument(fileId);
		return;
	}

	// Load file

	bool readOnly = f->state() != VcsState::CheckedOut ||
					  (db()->currentUser().isAdminstrator() == false
					   && db()->currentUser().userId() != f->userId());

	std::shared_ptr<DbFile> dbFile;
	if (db()->getLatestVersion(*f, &dbFile, this) == false)
	{
		QMessageBox::critical(this, "Error", "Get latest version error!");
		return;
	}

	// Add and select tree item

	QString itemName = f->fileName();
	if (readOnly == true)
	{
		itemName += QObject::tr(" [Read-only]");
	}

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << itemName);
	item->setToolTip(0, f->fileName());
	item->setData(0, Qt::UserRole, fileId);
	item->setSelected(true);

	m_openFilesTreeWidget->addTopLevelItem(item);
	m_openFilesTreeWidget->setCurrentItem(item);

	// Create document

	TestTabPageDocument& document = m_openDocuments[fileId];

	document.codeEditor = new IdeCodeEditor(CodeType::JavaScript, this);
	document.codeEditor->setText(dbFile->data());
	document.codeEditor->setReadOnly(readOnly);

	connect(document.codeEditor, &IdeCodeEditor::textChanged, this, &TestsTabPage::textChanged);
	connect(document.codeEditor, &IdeCodeEditor::cursorPositionChanged, this, &TestsTabPage::cursorPositionChanged);
	connect(document.codeEditor, &IdeCodeEditor::closeKeyPressed, this, &TestsTabPage::onCloseKeyPressed);
	connect(document.codeEditor, &IdeCodeEditor::saveKeyPressed, this, &TestsTabPage::onSaveKeyPressed);
	connect(document.codeEditor, &IdeCodeEditor::ctrlTabKeyPressed, this, &TestsTabPage::onCtrlTabKeyPressed);
	m_editorLayout->addWidget(document.codeEditor);

	document.openFilesTreeWidgetItem = item;
	document.dbFile = dbFile;

	m_openDocumentsCombo->blockSignals(true);
	m_openDocumentsCombo->addItem(itemName, fileId);
	m_openDocumentsCombo->model()->sort(0, Qt::AscendingOrder);
	m_openDocumentsCombo->blockSignals(false);

	// Open file in editor

	setCurrentDocument(fileId);

	return;
}


void TestsTabPage::newFolder()
{
	QString folderName = QInputDialog::getText(this, "New Folder", tr("Enter the folder name:"), QLineEdit::Normal, tr("FOLDER_%1").arg(db()->nextCounterValue()));
	if (folderName.isEmpty() == true)
	{
		return;
	}

	m_testsTreeView->addFolder(folderName);

	return;
}

void TestsTabPage::renameFile()
{
	// Save modified files
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	if (selectedIndexList.size() != 1)
	{
		return;
	}

	FileTreeModelItem* f = m_testsTreeModel->fileItem(selectedIndexList[0]);
	if (f == nullptr)
	{
		Q_ASSERT(f);
		return;
	}

	int fileId = -1;

	if (documentIsOpen(f->fileId()) == true)
	{
		fileId = f->fileId();

		if (documentIsModified(f->fileId()) == true)
		{
			saveDocument(f->fileId());
		}
	}
	// rename


	m_testsTreeView->renameFile();

	if (fileId != -1)
	{
		updateOpenDocumentInfo(fileId);
	}

	return;
}

void TestsTabPage::checkInFile()
{
	// Save modified files
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		if (documentIsOpen(f->fileId()) == true && documentIsModified(f->fileId()) == true)
		{
			saveDocument(f->fileId());
		}
	}

	m_testsTreeView->checkInFile();

	// Set editors to read-only
	//
	selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		if (documentIsOpen(f->fileId()) == true && f->state() == VcsState::CheckedIn)
		{
			setDocumentReadOnly(f->fileId(), true);
		}
	}

	return;
}

void TestsTabPage::checkOutFile()
{
	m_testsTreeView->checkOutFile();

	// Set editors to editable
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		if (documentIsOpen(f->fileId()) == true && f->state() == VcsState::CheckedOut)
		{
			setDocumentReadOnly(f->fileId(), false);
		}
	}
}

void TestsTabPage::undoChangesFile()
{
	// Remember the list of open documents to undo

	QList<int> openUndoDocuments;

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (f->state() == VcsState::CheckedOut)
		{
			if (documentIsOpen(f->fileId()) == true)
			{
				openUndoDocuments.push_back(f->fileId());
			}
		}
	}

	m_testsTreeView->undoChangesFile();

	// Close documents that are deleted and re-read other

	selectedIndexList = m_testsTreeView->selectedSourceRows();

	for (QModelIndex& mi : selectedIndexList)
	{
		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (openUndoDocuments.contains(f->fileId()) == true)
		{
			// Undo operation was performed, re-read information

			std::shared_ptr<DbFile> dbFile;

			if (db()->getLatestVersion(*f, &dbFile, this) == false)
			{
				QMessageBox::critical(this, "Error", "Get latest version error!");
				return;
			}

			TestTabPageDocument& document = m_openDocuments[f->fileId()];

			document.codeEditor->setText(dbFile->data());
			document.modified = false;

			setDocumentReadOnly(f->fileId(), true);

			openUndoDocuments.removeOne(f->fileId());
		}
	}

	// And now close documents that were removed

	for (int undoOpenDocument : openUndoDocuments)
	{
		closeDocument(undoOpenDocument, true/*force*/);
	}

	return;
}

void TestsTabPage::deleteFile()
{
	// Close documents before deleting

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (documentIsOpen(f->fileId()) == true)
		{
			closeDocument(f->fileId(), true/*force*/);
		}
	}

	m_testsTreeView->deleteFile();
}

void TestsTabPage::moveFile()
{
	// Close documents before deleting

	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();
	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
		assert(f);

		if (documentIsOpen(f->fileId()) == true)
		{
			QMessageBox::critical(this, qAppName(), tr("Can't move file %1, as it is opened for edit. Close schema and repeat operation.").arg(f->fileName()));
			return;

		}
	}

	m_testsTreeView->moveFile(db()->testsFileId());
}

void TestsTabPage::refreshFileTree()
{
	m_testsTreeView->refreshFileTree();

	setActionState();

	return;
}

void TestsTabPage::filterChanged()
{
	QString filterText = m_filterLineEdit->text().trimmed();

	m_filterResetButton->setEnabled(filterText.isEmpty() == false);

	if (filterText.isEmpty() == true)
	{
		m_filterSetButton->setStyleSheet("");
	}
	else
	{
		m_filterSetButton->setStyleSheet("font: bold;");
	}

	m_testsTreeView->setFileNameFilter(filterText);

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("TestsTabPage/FilterCompleter").toStringList();

	if (filterText.isEmpty() == false &&
		completerStringList.contains(filterText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(filterText);
		QSettings{}.setValue("TestsTabPage/FilterCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	return;
}

void TestsTabPage::textChanged()
{
	if (m_openDocuments.find(m_currentFileId) == m_openDocuments.end())
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[m_currentFileId];
	if (document.openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(document.openFilesTreeWidgetItem);
		return;
	}

	if (document.modified == false)
	{
		// Mark document as modified
		//
		document.modified = true;
		updateOpenDocumentInfo(m_currentFileId);
	}

	return;

}

void TestsTabPage::cursorPositionChanged(int line, int index)
{
	if (m_cursorPosButton == nullptr)
	{
		Q_ASSERT(m_cursorPosButton);
		return;
	}

	m_cursorPosButton->setText(tr(" Line: %1  Col: %2 ").arg(line + 1).arg(index + 1));

	return;
}

void TestsTabPage::closeCurrentDocument()
{
	closeDocument(m_currentFileId, false/*force*/);
	return;
}

void TestsTabPage::onGoToLine()
{
	if (documentIsOpen(m_currentFileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[m_currentFileId];

	if (document.codeEditor == nullptr)
	{
		Q_ASSERT(document.codeEditor);
		return;
	}

	int line = 0;
	int index = 0;
	document.codeEditor->getCursorPosition(&line, &index);

	bool ok = false;

	int newLine = QInputDialog::getInt(this, "Go to Line", tr("Enter line number:"), line + 1, 1, 2147483647, 1, &ok);
	if (ok == false)
	{
		return;
	}

	int maxLine = document.codeEditor->lines();
	if (newLine > maxLine)
	{
		newLine  = maxLine;
	}

	document.codeEditor->setCursorPosition(newLine - 1, 0);
	document.codeEditor->activateEditor();

	return;
}

void TestsTabPage::onSaveKeyPressed()
{
	if (documentIsOpen(m_currentFileId) == true)
	{
		const TestTabPageDocument& document = m_openDocuments[m_currentFileId];

		if (document.modified == true)
		{
			saveDocument(m_currentFileId);
		}
	}
}

void TestsTabPage::onCloseKeyPressed()
{
	if (documentIsOpen(m_currentFileId) == true)
	{
		closeDocument(m_currentFileId, false/*force*/);
	}
}

void TestsTabPage::onCtrlTabKeyPressed()
{
	if (documentIsOpen(m_currentFileId) == true)
	{
		const TestTabPageDocument& document = m_openDocuments[m_currentFileId];

		int currentIndex = m_openFilesTreeWidget->indexOfTopLevelItem(document.openFilesTreeWidgetItem);

		int openIndex = 0;

		if (currentIndex < m_openFilesTreeWidget->topLevelItemCount() - 1)
		{
			openIndex = currentIndex + 1;
		}

		QTreeWidgetItem* openItem = m_openFilesTreeWidget->topLevelItem(openIndex);
		if (openItem == nullptr)
		{
			Q_ASSERT(openItem);
			return;
		}

		bool ok = false;
		int fileId = openItem->data(0, Qt::UserRole).toInt(&ok);

		if (ok == true)
		{
			setCurrentDocument(fileId);
		}
		else
		{
			Q_ASSERT(ok);
		}
	}
}

bool TestsTabPage::documentIsModified(int fileId)
{
	if (documentIsOpen(fileId) == false)
	{
		return false;
	}

	TestTabPageDocument& document = m_openDocuments[fileId];
	if (document.codeEditor == nullptr)
	{
		Q_ASSERT(document.codeEditor);
		return false;
	}

	return document.modified;
}

void TestsTabPage::setCurrentDocument(int fileId)
{
	if (m_currentFileId == fileId)
	{
		return;
	}

	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	// Show Editor

	m_editorEmptyLabel->setVisible(false);
	m_editorToolBar->setVisible(true);

	// Hide current editor

	for (auto& it : m_openDocuments)
	{
		int docFileId = it.first;
		TestTabPageDocument& document = it.second;

		if (document.codeEditor == nullptr)
		{
			Q_ASSERT(document.codeEditor);
			return;
		}

		if (docFileId != fileId && document.codeEditor->isVisible())
		{
			document.codeEditor->setVisible(false);
		}
	}

	// Show new editor

	for (auto& it : m_openDocuments)
	{
		int docFileId = it.first;
		TestTabPageDocument& document = it.second;

		if (document.codeEditor == nullptr)
		{
			Q_ASSERT(document.codeEditor);
			return;
		}

		if (docFileId == fileId)
		{
			document.codeEditor->setVisible(true);
			document.codeEditor->activateEditor();

			if (m_cursorPosButton == nullptr)
			{
				Q_ASSERT(m_cursorPosButton);
				return;
			}

			int line = 0;
			int index = 0;
			document.codeEditor->getCursorPosition(&line, &index);

			m_cursorPosButton->setText(tr(" Line: %1  Col: %2").arg(line + 1).arg(index + 1));
		}
	}

	// Set current document to new

	m_currentFileId = fileId;

	// Select Open files tree widget item

	const TestTabPageDocument& newFile = m_openDocuments[fileId];

	if (newFile.openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(newFile.openFilesTreeWidgetItem);
		return;
	}

	newFile.openFilesTreeWidgetItem->setSelected(true);
	m_openFilesTreeWidget->setCurrentItem(newFile.openFilesTreeWidgetItem);


	// Select combo box item

	int comboIndex = m_openDocumentsCombo->findData(fileId, Qt::UserRole);
	if (comboIndex == -1)
	{
		Q_ASSERT(false);
	}
	else
	{
		m_openDocumentsCombo->blockSignals(true);
		m_openDocumentsCombo->setCurrentIndex(comboIndex);
		m_openDocumentsCombo->blockSignals(false);
	}

	return;
}

void TestsTabPage::setDocumentReadOnly(int fileId, bool readOnly)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[fileId];
	if (document.codeEditor == nullptr)
	{
		Q_ASSERT(document.codeEditor);
		return;
	}

	document.codeEditor->setReadOnly(readOnly);

	updateOpenDocumentInfo(fileId);

	return;
}

void TestsTabPage::openDocumentsComboTextChanged(int index)
{
	if (index < 0 || index >= m_openDocumentsCombo->count())
	{
		return;
	}

	bool ok = false;
	int fileId = m_openDocumentsCombo->itemData(index).toInt(&ok);

	if (ok == true)
	{
		setCurrentDocument(fileId);
	}
	else
	{
		Q_ASSERT(ok);
	}

	return;
}

void TestsTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension
	//
	bool extFound = false;
	QString fileName = object.name();

	for (const QString& ext : m_editableExtensions)
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

void TestsTabPage::createUi()
{
	// Set up default font
	//
#if defined(Q_OS_WIN)
	    m_editorFont = QFont("Consolas");
#elif defined(Q_OS_MAC)
	    m_editorFont = QFont("Courier");
#else
	    m_editorFont = QFont("Courier");
#endif

	// Tests tree and model
	//
	QWidget* testsWidget = new QWidget();

	QVBoxLayout* testsLayout = new QVBoxLayout(testsWidget);
	testsLayout->setContentsMargins(0, 0, 0, 0);

	m_testsToolbar = new QToolBar();
	m_testsToolbar->setStyleSheet("QToolButton { padding-top: 6px; padding-bottom: 6px; padding-left: 6px; padding-right: 6px;}");
	m_testsToolbar->setIconSize(m_testsToolbar->iconSize() * 0.9);

	testsLayout->addWidget(m_testsToolbar);

	m_testsTreeModel = new TestsFileTreeModel(db(), DbFileInfo::fullPathToFileName(Db::File::TestsFileName), this, this);

#ifdef _DEBUG
	[[maybe_unused]]QAbstractItemModelTester* modelTester = new QAbstractItemModelTester(m_testsTreeModel,
																	 QAbstractItemModelTester::FailureReportingMode::Fatal,
																		 this);
#endif

	std::vector<FileTreeModel::Columns> columns;
	columns.push_back(FileTreeModel::Columns::FileNameColumn);
	columns.push_back(FileTreeModel::Columns::FileStateColumn);
	columns.push_back(FileTreeModel::Columns::FileUserColumn);
	//columns.push_back(FileTreeModel::Columns::CustomColumnIndex);
	m_testsTreeModel->setColumns(columns);

	m_testsTreeView = new FileTreeView(db(), m_testsTreeModel);
	m_testsTreeView->setExpandsOnDoubleClick(false);
	m_testsTreeView->setSortingEnabled(true);
	connect(m_testsTreeView->header(), &QHeaderView::sortIndicatorChanged, [this](int index, Qt::SortOrder order)
	{
		m_testsTreeView->sortByColumn(index, order);
	});
	m_testsTreeView->sortByColumn(0, Qt::AscendingOrder);

	testsLayout->addWidget(m_testsTreeView);

	// Filter widgets
	//
	QHBoxLayout* filterLayout = new QHBoxLayout();

	m_filterLineEdit = new QLineEdit();
	m_filterLineEdit->setPlaceholderText(tr("Filter File Name"));
	m_filterLineEdit->setClearButtonEnabled(true);

	connect(m_filterLineEdit, &QLineEdit::returnPressed, this, &TestsTabPage::filterChanged);
	filterLayout->addWidget(m_filterLineEdit);

	m_filterSetButton = new QPushButton(tr("Filter"));
	connect(m_filterSetButton, &QPushButton::clicked, this, &TestsTabPage::filterChanged);
	filterLayout->addWidget(m_filterSetButton);

	m_filterResetButton = new QPushButton(tr("Reset Filter"));
	connect(m_filterResetButton, &QPushButton::clicked, [this](){
		m_filterLineEdit->clear();
		filterChanged();
	});
	m_filterResetButton->setEnabled(false);
	filterLayout->addWidget(m_filterResetButton);

	testsLayout->addLayout(filterLayout);

	// Filter completer
	QStringList completerStringList = QSettings{}.value("TestsTabPage/FilterCompleter").toStringList();
	m_filterCompleter = new QCompleter(completerStringList, this);
	m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_filterLineEdit->setCompleter(m_filterCompleter);

	// Opened files
	//
	QWidget* filesWidget = new QWidget();

	QVBoxLayout* filesLayout = new QVBoxLayout(filesWidget);
	filesLayout->setContentsMargins(0, 0, 0, 0);

	filesLayout->addWidget(new QLabel(tr("Open Files")));

	m_openFilesTreeWidget = new QTreeWidget();
	QStringList header;
	header << tr("File Name");
	m_openFilesTreeWidget->setHeaderLabels(header);
	m_openFilesTreeWidget->setColumnCount(header.size());
	m_openFilesTreeWidget->setHeaderHidden(true);
	m_openFilesTreeWidget->setSortingEnabled(true);
	m_openFilesTreeWidget->sortByColumn(0, Qt::AscendingOrder);
	filesLayout->addWidget(m_openFilesTreeWidget);

	// Editor layout
	//
	QWidget* editorWidget = new QWidget();
	m_editorLayout = new QVBoxLayout(editorWidget);

	// Editor toolbar
	//
	const int toolbarButtonSize = static_cast<int>(0.25 * logicalDpiY());

	m_editorToolBar = new QToolBar();
	m_editorToolBar->setVisible(false);
	m_editorToolBar->setAutoFillBackground(true);
	m_editorToolBar->setStyleSheet("QToolBar{spacing:3px; \
								   background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #808080, stop: 1 #606060); }");

	m_openDocumentsCombo = new QComboBox();
	m_openDocumentsCombo->setStyleSheet(tr("QComboBox{\
												border: 0px;\
												color: white;\
												background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #808080, stop: 1 #606060);\
											}\
											QComboBox:hover {\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #909090, stop: 1 #707070);\
											}\
											QComboBox:!editable:on, QComboBox::drop-down:editable:on {\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #707070, stop: 1 #505050);\
											}\
											QComboBox:on { \
												padding-top: 3px;\
												padding-left: 4px;\
											}\
											QComboBox::drop-down{\
												border: 1px;\
											}\
											QComboBox::down-arrow {\
													image: url(:/Images/Images/ComboDownArrow.svg);\
													width: %1px;\
													height: %1px;\
											}").arg(toolbarButtonSize/3));

	m_openDocumentsCombo->setFixedHeight(toolbarButtonSize);
	QString longFileName = QString().fill('0', 32);
	m_openDocumentsCombo->setMinimumWidth(QFontMetrics(m_openDocumentsCombo->font()).horizontalAdvance(longFileName));
	connect(m_openDocumentsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TestsTabPage::openDocumentsComboTextChanged);

	m_editorToolBar->addWidget(m_openDocumentsCombo);

	QString toolbarButtonStyle = "QPushButton {\
												background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #808080, stop: 1 #606060);\
												color: white;\
											}\
											QPushButton:hover {\
												border-style: none;\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #909090, stop: 1 #707070);\
											}\
											QPushButton:pressed {\
												background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #707070, stop: 1 #505050);\
											}";

	// Close Button
	//
	QPushButton* closeButton = new QPushButton(QIcon(":/Images/Images/CloseButtonWhite.svg"), QString());
	closeButton->setFlat(true);
	closeButton->setStyleSheet(toolbarButtonStyle);
	closeButton->setFixedSize(toolbarButtonSize, toolbarButtonSize);
	connect(closeButton, &QPushButton::clicked, this, &TestsTabPage::closeCurrentDocument);

	m_editorToolBar->addWidget(closeButton);

	// Spacer
	//
	QWidget* empty = new QWidget();
	empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_editorToolBar->addWidget(empty);

	// Cursor Pos Button
	//
	m_cursorPosButton = new QPushButton("Line: 1  Col: 1");
	m_cursorPosButton->setFlat(true);
	m_cursorPosButton->setStyleSheet(toolbarButtonStyle);

	QString longCursorPos = "Line: 9999 Col: 9999";
	m_cursorPosButton->setMinimumWidth(QFontMetrics(m_cursorPosButton->font()).horizontalAdvance(longCursorPos));
	m_cursorPosButton->setFixedHeight(toolbarButtonSize);
	connect(m_cursorPosButton, &QPushButton::clicked, this, &TestsTabPage::onGoToLine);
	m_editorToolBar->addWidget(m_cursorPosButton);

	m_editorLayout->addWidget(m_editorToolBar);

	// Empty editor
	//
	m_editorEmptyLabel = new QLabel(tr("No open documents"));
	m_editorEmptyLabel->setAlignment(Qt::AlignCenter);
	m_editorLayout->addWidget(m_editorEmptyLabel);

	// Left splitter
	//
	m_leftSplitter = new QSplitter(Qt::Vertical);
	m_leftSplitter->addWidget(testsWidget);
	m_leftSplitter->addWidget(filesWidget);
	m_leftSplitter->setStretchFactor(0, 2);
	m_leftSplitter->setStretchFactor(1, 1);
	m_leftSplitter->setCollapsible(0, false);
	m_leftSplitter->setCollapsible(1, false);

	// Vertical splitter
	//
	m_verticalSplitter = new QSplitter(Qt::Horizontal);
	m_verticalSplitter->addWidget(m_leftSplitter);
	m_verticalSplitter->addWidget(editorWidget);
	m_verticalSplitter->setStretchFactor(0, 1);
	m_verticalSplitter->setStretchFactor(1, 4);
	m_verticalSplitter->setCollapsible(0, false);
	m_verticalSplitter->setCollapsible(1, false);

	// Main layout
	//
	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(m_verticalSplitter);
	setLayout(mainLayout);

	return;
}

void TestsTabPage::createActions()
{
	if (m_testsTreeView == nullptr)
	{
		Q_ASSERT(m_testsTreeView);
		return;
	}

	QList<QAction*> toolbarActions;

	m_openFileAction = new QAction(QIcon(":/Images/Images/SchemaOpen.svg"), tr("Open File"), this);
	m_openFileAction->setStatusTip(tr("Open File..."));
	m_openFileAction->setEnabled(false);
	connect(m_openFileAction, &QAction::triggered, this, &TestsTabPage::openFile);
	toolbarActions.push_back(m_openFileAction);

	m_SeparatorAction1 = new QAction(this);
	m_SeparatorAction1->setSeparator(true);
	toolbarActions.push_back(m_SeparatorAction1);

	m_newFileAction = new QAction(QIcon(":/Images/Images/SchemaAddFile.svg"), tr("New File..."), this);
	m_newFileAction->setStatusTip(tr("New File..."));
	m_newFileAction->setEnabled(false);
	m_newFileAction->setShortcut(QKeySequence::StandardKey::New);
	connect(m_newFileAction, &QAction::triggered, this, &TestsTabPage::newFile);
	toolbarActions.push_back(m_newFileAction);

	m_newFolderAction = new QAction(QIcon(":/Images/Images/SchemaAddFolder2.svg"), tr("New Folder..."), this);
	m_newFolderAction->setStatusTip(tr("New Folder..."));
	m_newFolderAction->setEnabled(false);
	connect(m_newFolderAction, &QAction::triggered, this, &TestsTabPage::newFolder);
	toolbarActions.push_back(m_newFolderAction);

	m_addFileAction = new QAction(QIcon(":/Images/Images/SchemaAddFile.svg"), tr("Add file..."), this);
	m_addFileAction->setStatusTip(tr("Add file..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::addFileToFolder);
	//toolbarActions.push_back(m_addFileAction);

	m_renameFileAction = new QAction(tr("Rename..."), this);
	m_renameFileAction->setStatusTip(tr("Rename..."));
	m_renameFileAction->setEnabled(false);
	connect(m_renameFileAction, &QAction::triggered, this, &TestsTabPage::renameFile);

	m_deleteFileAction = new QAction(QIcon(":/Images/Images/SchemaDelete.svg"), tr("Delete"), this);
	m_deleteFileAction->setStatusTip(tr("Delete"));
	m_deleteFileAction->setEnabled(false);
	m_deleteFileAction->setShortcut(QKeySequence::StandardKey::Delete);
	connect(m_deleteFileAction, &QAction::triggered, this, &TestsTabPage::deleteFile);
	toolbarActions.push_back(m_deleteFileAction);

	m_moveFileAction = new QAction(tr("Move File..."), this);
	m_moveFileAction->setStatusTip(tr("Move"));
	m_moveFileAction->setEnabled(false);
	connect(m_moveFileAction, &QAction::triggered, this, &TestsTabPage::moveFile);

	//----------------------------------
	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);
	toolbarActions.push_back(m_SeparatorAction2);

	m_checkOutAction = new QAction(QIcon(":/Images/Images/SchemaCheckOut.svg"), tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, this, &TestsTabPage::checkOutFile);
	toolbarActions.push_back(m_checkOutAction);

	m_checkInAction = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("CheckIn"), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, this, &TestsTabPage::checkInFile);
	toolbarActions.push_back(m_checkInAction);

	m_undoChangesAction = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo Changes"), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, this, &TestsTabPage::undoChangesFile);
	toolbarActions.push_back(m_undoChangesAction);

	m_historyAction = new QAction(QIcon(":/Images/Images/SchemaHistory.svg"), tr("History"), this);
	m_historyAction->setStatusTip(tr("View History"));
	m_historyAction->setEnabled(false);
	connect(m_historyAction, &QAction::triggered, m_testsTreeView, &FileTreeView::showHistory);

	m_compareAction = new QAction(tr("Compare..."), this);
	m_compareAction->setStatusTip(tr("Compare"));
	m_compareAction->setEnabled(false);
	connect(m_compareAction, &QAction::triggered, m_testsTreeView, &FileTreeView::showCompare);

	//----------------------------------
	m_SeparatorAction3 = new QAction(this);
	m_SeparatorAction3->setSeparator(true);
	toolbarActions.push_back(m_SeparatorAction3);

	m_refreshAction = new QAction(QIcon(":/Images/Images/SchemaRefresh.svg"), tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh Objects List"));
	m_refreshAction->setEnabled(false);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, this, &TestsTabPage::refreshFileTree);
	toolbarActions.push_back(m_refreshAction);
	addAction(m_refreshAction);

	m_testsTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// -----------------

	m_testsTreeView->addAction(m_openFileAction);
	m_testsTreeView->addAction(m_SeparatorAction1);

	m_testsTreeView->addAction(m_newFileAction);
	m_testsTreeView->addAction(m_newFolderAction);
	m_testsTreeView->addAction(m_addFileAction);
	m_testsTreeView->addAction(m_renameFileAction);
	m_testsTreeView->addAction(m_deleteFileAction);
	m_testsTreeView->addAction(m_moveFileAction);
	m_testsTreeView->addAction(m_SeparatorAction2);

	m_testsTreeView->addAction(m_checkOutAction);
	m_testsTreeView->addAction(m_checkInAction);
	m_testsTreeView->addAction(m_undoChangesAction);
	m_testsTreeView->addAction(m_historyAction);
	m_testsTreeView->addAction(m_compareAction);
	m_testsTreeView->addAction(m_SeparatorAction3);

	m_testsTreeView->addAction(m_refreshAction);

	m_testsToolbar->addActions(toolbarActions);

	return;
}

void TestsTabPage::setActionState()
{
	// Disable all
	//
	m_openFileAction->setEnabled(false);
	m_newFileAction->setEnabled(false);
	m_newFolderAction->setEnabled(false);
	m_addFileAction->setEnabled(false);
	m_renameFileAction->setEnabled(false);
	m_deleteFileAction->setEnabled(false);
	m_moveFileAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	m_checkInAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
	m_historyAction->setEnabled(false);
	m_compareAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	// --
	//
	QModelIndexList selectedIndexList = m_testsTreeView->selectedSourceRows();

	bool folderSelected = true;

	// Folder is selected
	//
	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		if (file == nullptr)
		{
			assert(file);
			return;
		}

		if (file->isFolder() == false)
		{
			folderSelected = false;
			break;
		}
	}

	// Enable edit only files with several extensions!
	//
	bool editableExtension = false;
	for (const QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items processing
			//
			continue;
		}

		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		if (file == nullptr)
		{
			assert(file);
			return;
		}

		QString ext = QFileInfo(file->fileName()).suffix();
		if (m_editableExtensions.contains(ext))
		{
			editableExtension = true;
			break;
		}
	}

	// CheckIn, CheckOut, Undo, Get/set Workcopy
	//
	bool canAnyBeCheckedIn = false;
	bool canAnyBeCheckedOut = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items processing
			//
			continue;
		}

		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		if (file == nullptr)
		{
			assert(file);
			return;
		}

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

	// Enable Actions
	//
	m_openFileAction->setEnabled(selectedIndexList.size() == 1 && editableExtension == true);
	m_newFileAction->setEnabled(selectedIndexList.size() == 1);
	m_newFolderAction->setEnabled(selectedIndexList.size() == 1);
	m_addFileAction->setEnabled(selectedIndexList.size() == 1);
	m_renameFileAction->setEnabled(selectedIndexList.size() == 1 && canAnyBeCheckedIn);
	m_moveFileAction->setEnabled(canAnyBeCheckedIn);

	// Delete Items action
	//
	m_deleteFileAction->setEnabled(false);

	for (const QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		assert(file);

		if (file->state() == VcsState::CheckedIn/* &&
			/*file->action() != VcsItemAction::Deleted*/)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
			/*&& file->action() != VcsItemAction::Deleted*/)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}
	}

	m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);
	m_historyAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == false);
	m_compareAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == false);

	m_refreshAction->setEnabled(true);

	return;
}

void TestsTabPage::saveSettings()
{
	QSettings s;
	s.setValue("TestsTabPage/leftSplitterState", m_leftSplitter->saveState());
	s.setValue("TestsTabPage/verticalSplitterState", m_verticalSplitter->saveState());
	s.setValue("TestsTabPage/testsHeaderState", m_testsTreeView->header()->saveState());

	return;
}

void TestsTabPage::restoreSettings()
{
	// Restore settings

	QSettings s;

	QByteArray data = s.value("TestsTabPage/leftSplitterState").toByteArray();
	if (data.isEmpty() == false)
	{
		m_leftSplitter->restoreState(data);
	}

	data = s.value("TestsTabPage/verticalSplitterState").toByteArray();
	if (data.isEmpty() == false)
	{
		m_verticalSplitter->restoreState(data);
	}

	QByteArray headerState = s.value("TestsTabPage/testsHeaderState").toByteArray();
	if (headerState.isEmpty() == false)
	{
		m_testsTreeView->header()->restoreState(headerState);
	}

	return;
}

void TestsTabPage::hideEditor()
{
	m_editorToolBar->setVisible(false);
	m_editorEmptyLabel->setVisible(true);
	m_currentFileId = -1;
}

bool TestsTabPage::documentIsOpen(int fileId)
{
	return m_openDocuments.find(fileId) != m_openDocuments.end();
}

void TestsTabPage::saveDocument(int fileId)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[fileId];

	if (document.codeEditor == nullptr || document.openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(document.openFilesTreeWidgetItem);
		Q_ASSERT(document.codeEditor);
		return;
	}

	if (document.modified == false)
	{
		// Fils is not modified
		return;
	}

	document.dbFile->setData(document.codeEditor->text().toUtf8());

	if (db()->setWorkcopy(document.dbFile, this) == false)
	{
		QMessageBox::critical(this, "Error", "Set work copy error!");
		return;
	}

	document.modified = false;

	updateOpenDocumentInfo(fileId);

	return;

}

void TestsTabPage::saveAllDocuments()
{
	for (auto& it : m_openDocuments)
	{
		saveDocument(it.first);
	}
}

void TestsTabPage::closeDocument(int fileId, bool force)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[fileId];

	if (document.codeEditor == nullptr || document.openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(document.openFilesTreeWidgetItem);
		Q_ASSERT(document.codeEditor);
		return;
	}

	if (force == false)
	{
		if (document.modified == true)
		{
			if (document.dbFile == nullptr)
			{
				Q_ASSERT(document.dbFile);
				return;
			}

			QString fileName = document.dbFile->fileName();

			auto reply = QMessageBox::question(this,
											   qAppName(),
											   tr("Warning! File %1 is modified. Save it now?").arg(fileName),
											   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (reply == QMessageBox::Cancel)
			{
				return;
			}
			if (reply == QMessageBox::Yes)
			{
				saveDocument(fileId);
			}
		}
	}

	// Delete item from opened files list

	int closeIndex = m_openFilesTreeWidget->indexOfTopLevelItem(document.openFilesTreeWidgetItem);

	// Delete item from combo box

	int comboIndex = m_openDocumentsCombo->findData(fileId, Qt::UserRole);
	if (comboIndex == -1)
	{
		Q_ASSERT(false);
	}
	else
	{
		m_openDocumentsCombo->blockSignals(true);
		m_openDocumentsCombo->removeItem(comboIndex);
		m_openDocumentsCombo->blockSignals(false);
	}

	// Delete item from opened files list

	m_openFilesTreeWidget->takeTopLevelItem(closeIndex);
	delete document.openFilesTreeWidgetItem;

	// Delete item documents list

	document.codeEditor->deleteLater();
	m_openDocuments.erase(fileId);

	// Open another document

	if (m_openFilesTreeWidget->topLevelItemCount() == 0)
	{
		// No documents left
		Q_ASSERT(m_openDocuments.empty() == true);

		hideEditor();
	}
	else
	{
		if (closeIndex > 0)
		{
			closeIndex--;
		}

		QTreeWidgetItem* anotherItem = m_openFilesTreeWidget->topLevelItem(closeIndex);
		if (anotherItem == nullptr)
		{
			Q_ASSERT(anotherItem);
			return;
		}

		bool ok = false;
		int anotherFileId = anotherItem->data(0, Qt::UserRole).toInt(&ok);

		if (ok == true)
		{
			setCurrentDocument(anotherFileId);
		}
		else
		{
			Q_ASSERT(ok);
		}
	}

	return;
}

void TestsTabPage::closeAllDocuments()
{
	for (auto& it : m_openDocuments)
	{
		delete it.second.codeEditor;
	}

	m_openDocuments.clear();
	m_openDocumentsCombo->clear();

	return;
}

void TestsTabPage::updateOpenDocumentInfo(int fileId)
{
	if (documentIsOpen(fileId) == false)
	{
		Q_ASSERT(false);
		return;
	}

	const TestTabPageDocument& document = m_openDocuments[fileId];

	if (document.openFilesTreeWidgetItem == nullptr)
	{
		Q_ASSERT(document.openFilesTreeWidgetItem);
		return;
	}

	if (document.dbFile == nullptr)
	{
		Q_ASSERT(document.dbFile);
		return;
	}

	QString itemName = document.dbFile->fileName();

	if (document.codeEditor->readOnly() == true)
	{
		itemName += QObject::tr(" [Read-only]");
	}
	if (document.modified == true)
	{
		itemName += QObject::tr(" *");
	}

	// Update status on OpenFilesTreeWidget

	document.openFilesTreeWidgetItem->setText(0, itemName);

	// Update combo box item

	int comboRenameIndex = m_openDocumentsCombo->findData(fileId, Qt::UserRole);
	if (comboRenameIndex != -1)
	{
		int comboCurrentIndex = m_openDocumentsCombo->currentIndex();

		m_openDocumentsCombo->blockSignals(true);
		m_openDocumentsCombo->setItemText(comboCurrentIndex, itemName);
		m_openDocumentsCombo->model()->sort(0, Qt::AscendingOrder);
		m_openDocumentsCombo->blockSignals(false);
	}
	else
	{
		Q_ASSERT(comboRenameIndex != -1);
	}
}

void TestsTabPage::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers() & Qt::ControlModifier)
	{
		if (event->key() == Qt::Key_S)
		{
			onSaveKeyPressed();
		}

		if (event->key() == Qt::Key_W)
		{
			onCloseKeyPressed();
		}

		if (event->key() == Qt::Key_Tab)
		{
			onCtrlTabKeyPressed();
			return;
		}
	}
}
