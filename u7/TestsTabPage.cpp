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

	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &TestsTabPage::compareObject);

	connect(m_testsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TestsTabPage::selectionChanged);
	connect(m_testsTreeModel, &FileTreeModel::dataChanged, this, &TestsTabPage::modelDataChanged);
	connect(m_testsTreeModel, &FileTreeModel::modelReset, this, &TestsTabPage::modelReset);

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

void TestsTabPage::modelReset()
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

	QString fileName = item->data(0, Qt::UserRole).toString();

	setCurrentDocument(fileName);

	return;
}

void TestsTabPage::newFile()
{
	QString fileName = QInputDialog::getText(this, "New File", tr("Enter the file name:"), QLineEdit::Normal, tr("NewFile_%1.js").arg(db()->nextCounterValue()));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	bool result = m_testsTreeView->addNewFile(fileName);
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

	QString fileName = f->fileName();

	FileTreeModelItem* parent = f->parent();
	while (parent != nullptr)
	{
		fileName = parent->fileName() + "/" + fileName;
		parent = parent->parent();
	}

	// Check if file is already open

	if (documentIsOpen(fileName) == true)
	{
		setCurrentDocument(fileName);
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
	item->setToolTip(0, fileName);
	item->setData(0, Qt::UserRole, fileName);
	item->setSelected(true);

	m_openFilesTreeWidget->addTopLevelItem(item);
	m_openFilesTreeWidget->setCurrentItem(item);

	// Create document

	TestTabPageDocument& document = m_openDocuments[fileName];

	document.codeEditor = new IdeCodeEditor(CodeType::JavaScript, this);
	document.codeEditor->setText(dbFile->data());
	document.codeEditor->setReadOnly(readOnly);

	connect(document.codeEditor, &IdeCodeEditor::textChanged, this, &TestsTabPage::textChanged);
	connect(document.codeEditor, &IdeCodeEditor::cursorPositionChanged, this, &TestsTabPage::cursorPositionChanged);
	connect(document.codeEditor, &IdeCodeEditor::closeKeyPressed, this, &TestsTabPage::onCloseKeyPressed);
	connect(document.codeEditor, &IdeCodeEditor::saveKeyPressed, this, &TestsTabPage::onSaveKeyPressed);
	connect(document.codeEditor, &IdeCodeEditor::ctrlTabKeyPressed, this, &TestsTabPage::onCtrlTabKeyPressed);
	m_editorLayout->addWidget(document.codeEditor);

	document.treeWidgetItem = item;
	document.readOnly = readOnly;
	document.dbFile = dbFile;

	m_openDocumentsCombo->blockSignals(true);
	m_openDocumentsCombo->addItem(DbFileInfo::fullPathToFileName(fileName), fileName);

	m_openDocumentsCombo->blockSignals(false);

	// Open file in editor

	setCurrentDocument(fileName);

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

void TestsTabPage::filterChanged()
{
	QString filterText = m_filterLineEdit->text();

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
	if (m_openDocuments.find(m_currentDocument) == m_openDocuments.end())
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[m_currentDocument];
	if (document.treeWidgetItem == nullptr)
	{
		Q_ASSERT(document.treeWidgetItem);
		return;
	}

	if (document.modified == false)
	{
		// Mark document as modified
		//
		document.modified = true;
		document.treeWidgetItem->setText(0, document.treeWidgetItem->text(0) + "*");
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
	closeDocument(m_currentDocument);
	return;
}

void TestsTabPage::onSaveKeyPressed()
{
	if (documentIsOpen(m_currentDocument) == true)
	{
		const TestTabPageDocument& document = m_openDocuments[m_currentDocument];

		if (document.modified == true)
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
		const TestTabPageDocument& document = m_openDocuments[m_currentDocument];

		int currentIndex = m_openFilesTreeWidget->indexOfTopLevelItem(document.treeWidgetItem);

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

void TestsTabPage::onGoToLine()
{
	if (documentIsOpen(m_currentDocument) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[m_currentDocument];

	if (document.codeEditor == nullptr)
	{
		Q_ASSERT(document.codeEditor);
		return;
	}

	int line = 0;
	int index = 0;
	document.codeEditor->getCursorPosition(&line, &index);

	int maxLine = document.codeEditor->lines();

	bool ok = false;

	int newLine = QInputDialog::getInt(this, "Go to Line", tr("Enter line number:"), line + 1, 1, maxLine + 1, 1, &ok);
	if (ok == false)
	{
		return;
	}

	document.codeEditor->setCursorPosition(newLine - 1, 0);
	document.codeEditor->activateEditor();

	return;
}

void TestsTabPage::setCurrentDocument(const QString& fileName)
{
	if (m_currentDocument == fileName)
	{
		return;
	}

	if (documentIsOpen(fileName) == false)
	{
		qDebug() << fileName;
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
		TestTabPageDocument& document = it.second;

		if (document.codeEditor == nullptr)
		{
			Q_ASSERT(document.codeEditor);
			return;
		}

		if (docFileName != fileName && document.codeEditor->isVisible())
		{
			document.codeEditor->setVisible(false);
		}
	}

	// Show new editor

	for (auto& it : m_openDocuments)
	{
		const QString& docFileName = it.first;
		TestTabPageDocument& document = it.second;

		if (document.codeEditor == nullptr)
		{
			Q_ASSERT(document.codeEditor);
			return;
		}

		if (docFileName == fileName)
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

	//

	int comboIndex = m_openDocumentsCombo->findData(fileName, Qt::UserRole);
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

void TestsTabPage::openDocumentsComboTextChanged(int index)
{
	if (index < 0 || index >= m_openDocumentsCombo->count())
	{
		return;
	}


	QString fileName = m_openDocumentsCombo->itemData(index).toString();

	setCurrentDocument(fileName);
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

	testsLayout->addWidget(new QLabel(tr("Tests Files")));

	m_testsToolbar = new QToolBar();
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

	QPushButton* b = new QPushButton(tr("Filter"));
	connect(b, &QPushButton::clicked, this, &TestsTabPage::filterChanged);
	filterLayout->addWidget(b);

	b = new QPushButton(tr("Reset Filter"));
	connect(b, &QPushButton::clicked, [this](){
		m_filterLineEdit->clear();
		filterChanged();
	});
	filterLayout->addWidget(b);

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

	m_newFileAction = new QAction(QIcon(":/Images/Images/SchemaNewFile.svg"), tr("New File..."), this);
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
	connect(m_addFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::addFile);
	toolbarActions.push_back(m_addFileAction);

	m_renameFileAction = new QAction(tr("Rename..."), this);
	m_renameFileAction->setStatusTip(tr("Rename..."));
	m_renameFileAction->setEnabled(false);
	connect(m_renameFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::renameFile);

	m_deleteFileAction = new QAction(QIcon(":/Images/Images/SchemaDelete.svg"), tr("Delete"), this);
	m_deleteFileAction->setStatusTip(tr("Delete"));
	m_deleteFileAction->setEnabled(false);
	m_deleteFileAction->setShortcut(QKeySequence::StandardKey::Delete);
	connect(m_deleteFileAction, &QAction::triggered, m_testsTreeView, &FileTreeView::deleteFile);
	toolbarActions.push_back(m_deleteFileAction);

	//----------------------------------
	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);
	toolbarActions.push_back(m_SeparatorAction2);

	m_checkOutAction = new QAction(QIcon(":/Images/Images/SchemaCheckOut.svg"), tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, m_testsTreeView, &FileTreeView::checkOutFile);
	toolbarActions.push_back(m_checkOutAction);

	m_checkInAction = new QAction(QIcon(":/Images/Images/SchemaCheckIn.svg"), tr("CheckIn"), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, m_testsTreeView, &FileTreeView::checkInFile);
	toolbarActions.push_back(m_checkInAction);

	m_undoChangesAction = new QAction(QIcon(":/Images/Images/SchemaUndo.svg"), tr("Undo Changes"), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, m_testsTreeView, &FileTreeView::undoChangesFile);
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
	connect(m_refreshAction, &QAction::triggered, m_testsTreeView, &FileTreeView::refreshFileTree);
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
	m_newFileAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == true);
	m_newFolderAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == true);
	m_addFileAction->setEnabled(selectedIndexList.size() == 1);
	m_renameFileAction->setEnabled(selectedIndexList.size() == 1 && canAnyBeCheckedIn);

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

	m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);
	m_historyAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == false);
	m_compareAction->setEnabled(selectedIndexList.size() == 1 && folderSelected == false);

	m_refreshAction->setEnabled(true);

	return;
}

bool TestsTabPage::documentIsOpen(const QString& fileName)
{
	return m_openDocuments.find(fileName) != m_openDocuments.end();
}

void TestsTabPage::saveDocument(const QString& fileName)
{
	if (documentIsOpen(fileName) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[fileName];

	if (document.codeEditor == nullptr || document.treeWidgetItem == nullptr)
	{
		Q_ASSERT(document.treeWidgetItem);
		Q_ASSERT(document.codeEditor);
		return;
	}

	document.dbFile->setData(document.codeEditor->text().toUtf8());

	if (db()->setWorkcopy(document.dbFile, this) == false)
	{
		QMessageBox::critical(this, "Error", "Set work copy error!");
		return;
	}

	document.modified = false;
	document.treeWidgetItem->setText(0, document.treeWidgetItem->text(0).remove('*'));
}

void TestsTabPage::closeDocument(const QString& fileName)
{
	if (documentIsOpen(fileName) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TestTabPageDocument& document = m_openDocuments[fileName];

	if (document.codeEditor == nullptr || document.treeWidgetItem == nullptr)
	{
		Q_ASSERT(document.treeWidgetItem);
		Q_ASSERT(document.codeEditor);
		return;
	}

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

	// Delete item from combo box

	int comboIndex = m_openDocumentsCombo->findData(fileName, Qt::UserRole);
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
	m_openDocumentsCombo->clear();

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
