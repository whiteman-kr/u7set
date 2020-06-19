#include "TestsTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"

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
	createUi();

	createActions();

	restoreSettings();

	std::vector<FileTreeModel::Columns> columns;
	columns.push_back(FileTreeModel::Columns::FileNameColumn);
	columns.push_back(FileTreeModel::Columns::FileStateColumn);
	columns.push_back(FileTreeModel::Columns::FileUserColumn);
	columns.push_back(FileTreeModel::Columns::CustomColumnIndex);
	m_testsTreeModel->setColumns(columns);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &TestsTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &TestsTabPage::projectClosed);

	connect(m_testsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TestsTabPage::selectionChanged);
	connect(m_testsTreeModel, &FileTreeModel::dataChanged, this, &TestsTabPage::modelDataChanged);

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

void TestsTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void TestsTabPage::projectClosed()
{
	closeAllDocuments();
	m_openFilesTreeWidget->clear();

	hideEditor();

	this->setEnabled(false);
	return;
}

void TestsTabPage::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	setActionState();

	return;
}

void TestsTabPage::modelDataChanged(const QModelIndex& topLeft,
									const QModelIndex& bottomRight, const QVector<int>& roles /*= QVector<int>()*/)
{
	Q_UNUSED(topLeft);
	Q_UNUSED(bottomRight);
	Q_UNUSED(roles);

	setActionState();

	return;
}

void TestsTabPage::testsTreeDoubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	if (m_openFileAction->isEnabled() == true)
	{
		openDocument();
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

	QString fileName = item->data(0, Qt::UserRole).toString();
	setCurrentDocument(fileName);

	return;
}

void TestsTabPage::openDocument()
{
	QModelIndexList indexes = m_testsTreeView->selectionModel()->selectedRows();

	if (indexes.size() != 1)
	{
		return;
	}

	QModelIndex& mi = indexes[0];

	FileTreeModelItem* f = m_testsTreeModel->fileItem(mi);
	if (f == nullptr)
	{
		Q_ASSERT(f);
		return;
	}

	QString fullFileName = f->fileName();

	FileTreeModelItem* parent = f->parent();
	while (parent != nullptr)
	{
		fullFileName = parent->fileName() + "/" + fullFileName;
		parent = parent->parent();
	}

	// Check if file is already open

	if (documentIsOpen(fullFileName) == true)
	{
		setCurrentDocument(fullFileName);
		return;
	}

	bool readOnly = f->state() != VcsState::CheckedOut ||
					  (db()->currentUser().isAdminstrator() == false
					   && db()->currentUser().userId() != f->userId());

	// Load file

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
	item->setToolTip(0, fullFileName);
	item->setData(0, Qt::UserRole, fullFileName);
	item->setSelected(true);

	m_openFilesTreeWidget->addTopLevelItem(item);
	m_openFilesTreeWidget->setCurrentItem(item);

	// Create document

	TestTabPageDocument& doc = m_openDocuments[fullFileName];

	doc.codeEditor = new IdeCodeEditor(CodeType::JavaScript, this);
	doc.codeEditor->setText(dbFile->data());
	doc.codeEditor->setReadOnly(readOnly);

	connect(doc.codeEditor, &IdeCodeEditor::textChanged, this, &TestsTabPage::textChanged);
	connect(doc.codeEditor, &IdeCodeEditor::cursorPositionChanged, this, &TestsTabPage::cursorPositionChanged);
	connect(doc.codeEditor, &IdeCodeEditor::closeKeyPressed, this, &TestsTabPage::onCloseKeyPressed);
	connect(doc.codeEditor, &IdeCodeEditor::saveKeyPressed, this, &TestsTabPage::onSaveKeyPressed);
	connect(doc.codeEditor, &IdeCodeEditor::ctrlTabKeyPressed, this, &TestsTabPage::onCtrlTabKeyPressed);
	m_editorLayout->addWidget(doc.codeEditor);


	doc.treeWidgetItem = item;
	doc.readOnly = readOnly;
	doc.dbFile = dbFile;

	// Open file in editor

	setCurrentDocument(fullFileName);

	return;
}


void TestsTabPage::textChanged()
{
	if (m_openDocuments.find(m_currentDocument) == m_openDocuments.end())
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& doc = m_openDocuments[m_currentDocument];

	if (doc.modified == false)
	{
		// Mark document as modified
		//
		doc.modified = true;

		if (doc.treeWidgetItem == nullptr)
		{
			Q_ASSERT(doc.treeWidgetItem);
		}
		else
		{
			doc.treeWidgetItem->setText(0, doc.treeWidgetItem->text(0) + "*");
		}
	}

	return;

}

void TestsTabPage::cursorPositionChanged(int line, int index)
{
	if (m_lineLabel == nullptr || m_columnLabel == nullptr)
	{
		Q_ASSERT(m_lineLabel);
		Q_ASSERT(m_columnLabel);
		return;
	}

	m_lineLabel->setText(tr(" Line: %1 ").arg(line));
	m_columnLabel->setText(tr(" Col: %1 ").arg(index));

	return;
}

void TestsTabPage::closeCurrentDocument()
{
	closeDocument(m_currentDocument);
	return;
}

void TestsTabPage::onSaveKeyPressed()
{
	if (documentIsOpen(m_currentDocument) == true)
	{
		const TestTabPageDocument& doc = m_openDocuments[m_currentDocument];

		if (doc.modified == true)
		{
			saveDocument(m_currentDocument);
		}
	}
}

void TestsTabPage::onCloseKeyPressed()
{
	if (documentIsOpen(m_currentDocument) == true)
	{
		closeDocument(m_currentDocument);
	}
}

void TestsTabPage::onCtrlTabKeyPressed()
{
	if (documentIsOpen(m_currentDocument) == true)
	{
		const TestTabPageDocument& doc = m_openDocuments[m_currentDocument];

		int currentIndex = m_openFilesTreeWidget->indexOfTopLevelItem(doc.treeWidgetItem);

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

		QString fileName = openItem->data(0, Qt::UserRole).toString();
		setCurrentDocument(fileName);
	}
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

	// Tests tree

	QWidget* testsWidget = new QWidget();

	QVBoxLayout* testsLayout = new QVBoxLayout(testsWidget);
	testsLayout->setContentsMargins(0, 0, 0, 0);

	testsLayout->addWidget(new QLabel(tr("Tests Files")));

	m_testsTreeModel = new TestsFileTreeModel(db(), DbFileInfo::fullPathToFileName(Db::File::TestsFileName), this, this);

	m_testsTreeView = new FileTreeView(db());
	m_testsTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_testsTreeView->setModel(m_testsTreeModel);

	testsLayout->addWidget(m_testsTreeView);

	// Open files

	QWidget* filesWidget = new QWidget();

	QVBoxLayout* filesLayout = new QVBoxLayout(filesWidget);
	filesLayout->setContentsMargins(0, 0, 0, 0);

	filesLayout->addWidget(new QLabel(tr("Open Files")));

	m_openFilesTreeWidget = new QTreeWidget();
	QStringList header;
	header << tr("File Name");
	m_openFilesTreeWidget->setHeaderLabels(header);
	m_openFilesTreeWidget->setColumnCount(header.size());
	filesLayout->addWidget(m_openFilesTreeWidget);

	// Editor layout

	QWidget* editorWidget = new QWidget();
	m_editorLayout = new QVBoxLayout(editorWidget);

	// Editor toolbar

	m_editorToolBar = new QToolBar();
	m_editorToolBar->setVisible(false);
	m_editorToolBar->setAutoFillBackground(true);
	m_editorToolBar->setStyleSheet("QToolBar{spacing:3px; background: gray; }");

	m_editorFileNameLabel = new QLabel("FileName");
	m_editorFileNameLabel->setStyleSheet("QLabel{color: white}");
	m_editorToolBar->addWidget(m_editorFileNameLabel);

	QPushButton* closeButton = new QPushButton("X");
	closeButton->setFlat(true);
	closeButton->setStyleSheet("QPushButton {\
	                                background-color: #808080;\
	                                color: white;\
                                }\
                                QPushButton:hover {\
	                                border-style: inset;\
									border-width: 1px;\
									border-color: #808080;\
									background-color: #909090;\
                                }\
                                QPushButton:pressed {\
	                                background-color: #707070;\
                                }");
    closeButton->setFixedSize(20, 20);
	connect(closeButton, &QPushButton::clicked, this, &TestsTabPage::closeCurrentDocument);

	m_editorToolBar->addWidget(closeButton);

	QWidget* empty = new QWidget();
	empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_editorToolBar->addWidget(empty);

	m_lineLabel = new QLabel("Line: 0");
	m_lineLabel->setStyleSheet("QLabel{color: white}");
	m_editorToolBar->addWidget(m_lineLabel);

	m_columnLabel = new QLabel("Col: 0");
	m_columnLabel->setStyleSheet("QLabel{color: white}");
	m_editorToolBar->addWidget(m_columnLabel);

	m_editorLayout->addWidget(m_editorToolBar);

	// Empty editor

	m_editorEmptyLabel = new QLabel(tr("No open documents"));
	m_editorEmptyLabel->setAlignment(Qt::AlignCenter);
	m_editorLayout->addWidget(m_editorEmptyLabel);

	m_lexerJavaScript.setDefaultFont(m_editorFont);

	// Left splitter

	m_leftSplitter = new QSplitter(Qt::Vertical);
	m_leftSplitter->addWidget(testsWidget);
	m_leftSplitter->addWidget(filesWidget);
	m_leftSplitter->setStretchFactor(0, 2);
	m_leftSplitter->setStretchFactor(1, 1);

	// Vertical splitter

	m_verticalSplitter = new QSplitter(Qt::Horizontal);
	m_verticalSplitter->addWidget(m_leftSplitter);
	m_verticalSplitter->addWidget(editorWidget);
	m_verticalSplitter->setStretchFactor(0, 1);
	m_verticalSplitter->setStretchFactor(1, 4);

	// Main layout

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

	m_addFileAction = new QAction(tr("Add file"), this);
	m_addFileAction->setStatusTip(tr("Add file..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::addFile);

	m_openFileAction = new QAction(tr("Open file"), this);
	m_openFileAction->setStatusTip(tr("Open file..."));
	m_openFileAction->setEnabled(false);
	connect(m_openFileAction, &QAction::triggered, this, &TestsTabPage::openDocument);

	m_deleteFileAction = new QAction(tr("Delete file"), this);
	m_deleteFileAction->setStatusTip(tr("Delete file..."));
	m_deleteFileAction->setEnabled(false);
	connect(m_deleteFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::deleteFile);

	//----------------------------------
	m_SeparatorAction1 = new QAction(this);
	m_SeparatorAction1->setSeparator(true);

	m_checkOutAction = new QAction(tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, m_testsTreeView, &FileTreeView::checkOutFile);

	m_checkInAction = new QAction(tr("CheckIn"), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, m_testsTreeView, &FileTreeView::checkInFile);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, m_testsTreeView, &FileTreeView::undoChangesFile);

	//----------------------------------
	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);

	//----------------------------------
	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh object list"));
	m_refreshAction->setEnabled(false);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, m_testsTreeView, &FileTreeView::refreshFileTree);
	addAction(m_refreshAction);

	m_testsTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// -----------------
	m_testsTreeView->addAction(m_addFileAction);
	m_testsTreeView->addAction(m_openFileAction);
	m_testsTreeView->addAction(m_deleteFileAction);

	// -----------------
	m_testsTreeView->addAction(m_SeparatorAction1);
	m_testsTreeView->addAction(m_checkOutAction);
	m_testsTreeView->addAction(m_checkInAction);
	m_testsTreeView->addAction(m_undoChangesAction);
	// -----------------
	m_testsTreeView->addAction(m_SeparatorAction2);
	// -----------------
	m_testsTreeView->addAction(m_SeparatorAction2);
	m_testsTreeView->addAction(m_refreshAction);
	// -----------------


	return;
}

void TestsTabPage::saveSettings()
{
	QSettings s;
	s.setValue("TestsTabPage/leftSplitterState", m_leftSplitter->saveState());
	s.setValue("TestsTabPage/verticalSplitterState", m_verticalSplitter->saveState());

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

	return;
}

void TestsTabPage::setActionState()
{
	// Disable all
	//
	m_addFileAction->setEnabled(false);
	m_openFileAction->setEnabled(false);
	m_deleteFileAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	m_checkInAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
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
	QModelIndexList selectedIndexList = m_testsTreeView->selectionModel()->selectedRows();

	// Add Action
	//
	m_addFileAction->setEnabled(selectedIndexList.size() == 1);

	// Delete Items action
	//
	m_deleteFileAction->setEnabled(false);
	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
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
		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
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

	// Enable edit only files with several extensions!
	//
	QStringList editableExtensions;
	editableExtensions << tr("js");

	bool editableExtension = false;
	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_testsTreeModel->fileItem(mi);
		assert(file);

		QString ext = QFileInfo(file->fileName()).suffix();
		if (editableExtensions.contains(ext))
		{
			editableExtension = true;
			break;
		}
	}

	m_openFileAction->setEnabled(editableExtension && selectedIndexList.size() == 1);

	return;
}

bool TestsTabPage::documentIsOpen(const QString& fileName)
{
	return m_openDocuments.find(fileName) != m_openDocuments.end();
}

void TestsTabPage::setCurrentDocument(const QString& fileName)
{
	if (m_currentDocument == fileName)
	{
		return;
	}

	if (documentIsOpen(fileName) == false)
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
		const QString& docFileName = it.first;
		TestTabPageDocument& doc = it.second;
		if (docFileName != fileName && doc.codeEditor->isVisible())
		{
			doc.codeEditor->setVisible(false);
		}
	}

	// Show new editor

	for (auto& it : m_openDocuments)
	{
		const QString& docFileName = it.first;
		TestTabPageDocument& doc = it.second;
		if (docFileName == fileName)
		{
			doc.codeEditor->setVisible(true);
			doc.codeEditor->activateEditor();

			if (m_lineLabel == nullptr || m_columnLabel == nullptr)
			{
				Q_ASSERT(m_lineLabel);
				Q_ASSERT(m_columnLabel);
				return;
			}

			int line = 0;
			int index = 0;
			doc.codeEditor->getCursorPosition(&line, &index);

			m_lineLabel->setText(tr(" Line: %1 ").arg(line));
			m_columnLabel->setText(tr(" Col: %1 ").arg(index));
		}
	}

	// Set new text
	//
	const TestTabPageDocument& newFile = m_openDocuments[fileName];

	if (newFile.treeWidgetItem == nullptr)
	{
		Q_ASSERT(newFile.treeWidgetItem);
		return;
	}

	newFile.treeWidgetItem->setSelected(true);
	m_openFilesTreeWidget->setCurrentItem(newFile.treeWidgetItem);

	//

	m_currentDocument = fileName;

	m_editorFileNameLabel->setText(tr(" %1 ").arg(fileName));
}

void TestsTabPage::saveDocument(const QString& fileName)
{
	if (documentIsOpen(fileName) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& doc = m_openDocuments[fileName];

	doc.dbFile->setData(doc.codeEditor->text().toUtf8());

	if (db()->setWorkcopy(doc.dbFile, this) == false)
	{
		QMessageBox::critical(this, "Error", "Set work copy error!");
		return;
	}

	doc.modified = false;
	doc.treeWidgetItem->setText(0, doc.treeWidgetItem->text(0).remove('*'));
}

void TestsTabPage::closeDocument(const QString& fileName)
{
	if (documentIsOpen(fileName) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[fileName];

	if (document.modified == true)
	{
		auto reply = QMessageBox::question(this,
										   qAppName(),
										   tr("Warning! File %1 is modified. Save it now?").arg(DbFile::fullPathToFileName(fileName)),
										   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (reply == QMessageBox::Cancel)
		{
			return;
		}
		if (reply == QMessageBox::Yes)
		{
			saveDocument(fileName);
		}
	}

	// Delete item from opened files list

	int closeIndex = m_openFilesTreeWidget->indexOfTopLevelItem(document.treeWidgetItem);

	// Delete item from opened files list

	m_openFilesTreeWidget->takeTopLevelItem(closeIndex);
	delete document.treeWidgetItem;

	// Delete item documents list

	document.codeEditor->deleteLater();
	m_openDocuments.erase(fileName);

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

		QString anotherFileName = anotherItem->data(0, Qt::UserRole).toString();
		setCurrentDocument(anotherFileName);
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

	return;
}

void TestsTabPage::hideEditor()
{
	m_editorToolBar->setVisible(false);
	m_editorEmptyLabel->setVisible(true);
	m_currentDocument.clear();
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
