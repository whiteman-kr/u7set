#pragma once

#include "MainTabPage.h"
#include <QToolBar>
#include "IdePropertyEditor.h"
#include "GlobalMessanger.h"
#include "../lib/Ui/FilesTreeView.h"
#include "../Simulator/Simulator.h"

class TestsFileTreeModel : public FileTreeModel
{
public:
	TestsFileTreeModel() = delete;
	TestsFileTreeModel(DbController* dbcontroller, QString rootFilePath, QWidget* parentWidget, QObject* parent);
	virtual ~TestsFileTreeModel();

private:
	QString customColumnText(Columns column, const FileTreeModelItem* item) const override;
	QString customColumnName(Columns column) const override;

};

class TestTabPageDocument
{
private:
	TestTabPageDocument() = delete;

public:
	TestTabPageDocument(const QString& fileName, IdeCodeEditor* codeEditor, QTreeWidgetItem* openFilesTreeWidgetItem);

public:
	QString fileName() const;
	void setFileName(const QString& fileName);

	bool modified() const;
	void setModified(bool value);

	IdeCodeEditor* codeEditor() const;

	QTreeWidgetItem* openFilesTreeWidgetItem() const;

private:
	QString m_fileName;
	IdeCodeEditor* m_codeEditor = nullptr;
	bool m_modified = false;
	QTreeWidgetItem* m_openFilesTreeWidgetItem = nullptr;
};

class TestsWidget;

//
// OutputLogWidget
//

class OutputDockLog : public ILogFile
{
public:
	OutputDockLog();

	void swapData(QStringList* data, int* errorCount, int* warningCount);

private:
	// ILogFile implementation
	//
	bool writeAlert(const QString& text) override;
	bool writeError(const QString& text)  override;
	bool writeWarning(const QString& text) override;
	bool writeMessage(const QString& text)  override;
	bool writeText(const QString& text)  override;

	void write(QtMsgType type, const QString& msg);

private:
	QMutex m_mutex;
	QStringList m_data;

	int m_errorCount = 0;
	int m_warningCount = 0;
};

class OutputLogTextEdit : public QTextEdit
{
public:
	OutputLogTextEdit(QWidget* parent);

private:
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void keyPressEvent(QKeyEvent*) override;
};

//
// OutputDockWidget
//

class OutputDockWidgetTitleButton : public QAbstractButton
{
	Q_OBJECT

public:
	OutputDockWidgetTitleButton(QDockWidget *dockWidget, bool drawActualIconSizeOnWindows);

	QSize sizeHint() const override;
	QSize minimumSizeHint() const override
	{ return sizeHint(); }

	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

protected:
	bool event(QEvent *event) override;

private:
	QSize dockButtonIconSize() const;

	mutable int m_iconSize = -1;
	bool m_drawActualIconSizeOnWindows = false;
};

class OutputDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	OutputDockWidget(const QString& title, OutputDockLog* log, QWidget* parent);

	void clear();

	void setWidget(QWidget *widget);

private slots:
	void floatingChanged(bool floating);

	void prevIssue(const QLatin1String& prefix);
	void nextIssue(const QLatin1String& prefix);

private:
	virtual void paintEvent(QPaintEvent *event) override;
	virtual void timerEvent(QTimerEvent* event) override;

	void createToolbar();

	OutputLogTextEdit* m_logTextEdit = nullptr;

	QLabel* m_errorLabel = nullptr;
	QLabel* m_warningLabel = nullptr;

	QLineEdit* m_findEdit = nullptr;
	QPushButton* m_findButton = nullptr;

	OutputDockWidgetTitleButton* m_prevWarningButton = nullptr;
	OutputDockWidgetTitleButton* m_nextWarningButton = nullptr;

	OutputDockWidgetTitleButton* m_prevErrorButton = nullptr;
	OutputDockWidgetTitleButton* m_nextErrorButton = nullptr;

	OutputDockWidgetTitleButton* m_floatButton = nullptr;
	OutputDockWidgetTitleButton* m_closeButton = nullptr;

	OutputDockLog* m_log = nullptr;

	int m_errorCount = 0;
	int m_warningCount = 0;

	// Issue navigation
	//
	QTextCursor m_lastNavCursor;
	bool m_lastNavIsPrevIssue = false;
	bool m_lastNavIsNextIssue = false;

};

//
// TestsTabPage
//

class TestsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	explicit TestsTabPage(DbController* dbc, QWidget* parent);
	virtual ~TestsTabPage();

public:
	bool hasUnsavedTests() const;
	void saveUnsavedTests();
	void resetModified();

public slots:
	void projectOpened();
	void projectClosed();

private:
	TestsWidget* m_testsWidget = nullptr;
};

//
// TestsWidget
//

class TestsWidget : public QMainWindow, HasDbController
{
	Q_OBJECT

public:
	TestsWidget(DbController* db, QWidget* parent);
	virtual ~TestsWidget();

public:
	bool hasUnsavedTests() const;
	void saveUnsavedTests();
	void resetModified();

private slots:
	// Project operations
	//
	void projectOpened();
	void projectClosed();
	void buildStarted();

	// Tree controls operations
	//
	void testsTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void testsTreeModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
	void testsTreeModelReset();
	void testsTreeDoubleClicked(const QModelIndex &index);

	void openFilesClicked(const QModelIndex &index);

	// Tests file tree slots
	//
	void newFile();
	void openFile();
	void newFolder();
	void renameFile();
	void checkInSelectedFiles();
	void checkOutSelectedFiles();
	void undoChangesSelectedFiles();
	void deleteSelectedFiles();
	void moveSelectedFiles();
	void refreshFileTree();
	void runAllTestFiles();
	void runSelectedTestFiles();

	// Code Editor slots
	//
	void filterChanged();
	void textChanged();
	void cursorPositionChanged(int line, int index);
	void onGoToLine();

	void checkInCurrentFile();
	void checkOutCurrentFile();
	void undoChangesCurrentFile();
	void saveCurrentFile();
	void closeCurrentFile();
	void runTestCurrentFile();
	void stopTests();

	// Open Files slots
	//
	void openFilesMenuRequested(const QPoint &pos);

	void checkInOpenFile();
	void checkOutOpenFile();
	void undoChangesOpenFile();
	void saveOpenFile();
	void closeOpenFile();
	void runTestOpenFile();

	// Hotkeys
	//
	void onSaveKeyPressed();
	void onCloseKeyPressed();
	void onCtrlTabKeyPressed();

	// Open documents combo operations
	//
	void openDocumentsComboTextChanged(int index);

	// Other operations
	//
	void closeDocumentsForDeletedFiles();

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Build operations slots
	//
	void selectBuild();

	void runSimTests(const QString& buildPath, const std::vector<DbFileInfo>& files);
	void stopSimTests();

	void simStateChanged(Sim::SimControlState state);

private:
	void createToolbar();
	void createTestsDock();
	void createLogDock();
	void createEditorWidget();
	void createActions();

	void setTestsTreeActionsState();
	void setCodeEditorActionsState();

	void saveSettings();
	void restoreSettings();

	void hideEditor();

	// Documents operations

	bool documentIsOpen(int fileId) const;
	bool documentIsModified(int fileId) const;

	void checkInDocument(std::vector<int> fileIds);
	void checkOutDocument(std::vector<int> fileIds);
	void undoChangesDocument(std::vector<int> fileIds);

	void setCurrentDocument(int fileId);
	void setDocumentReadOnly(int fileId, bool readOnly);
	void saveDocument(int fileId);
	void saveAllDocuments();
	void closeDocument(int fileId, bool force);
	void closeAllDocuments();
	void updateOpenDocumentInfo(int fileId);
	void runTests(std::vector<int> fileIds);

	// Override functions
private:
	virtual void showEvent(QShowEvent* e) override;
	virtual void keyPressEvent(QKeyEvent* event) override;

	bool isEditableExtension(const QString& fileName);

private:
	// Data
	//
	std::map<int, TestTabPageDocument> m_openDocuments;
	int m_currentFileId = -1;
	QFont m_editorFont;

	QStringList m_editableExtensions;

	QString m_buildPath;

	// Widgets
	//
	QToolBar* m_testsToolbar = nullptr;

	FileTreeView* m_testsTreeView = nullptr;
	TestsFileTreeModel* m_testsTreeModel = nullptr;

	QPushButton* m_filterSetButton = nullptr;
	QPushButton* m_filterResetButton = nullptr;
	QLineEdit* m_filterLineEdit = nullptr;
	QCompleter* m_filterCompleter = nullptr;

	QTreeWidget* m_openFilesTreeWidget = nullptr;

	QVBoxLayout* m_editorLayout = nullptr;
	QToolBar* m_editorToolBar = nullptr;

	QComboBox* m_openDocumentsCombo = nullptr;
	QPushButton* m_cursorPosButton = nullptr;

	QLabel* m_buildLabel = nullptr;

	QSplitter* m_leftSplitter = nullptr;

	QDockWidget* m_testsDockWidget = nullptr;
	OutputDockWidget m_outputDockWidget;

	// Tests file tree actions
	//
	QAction* m_newFileAction = nullptr;
	QAction* m_addFileAction = nullptr;
	QAction* m_newFolderAction = nullptr;
	QAction* m_openFileAction = nullptr;
	QAction* m_renameFileAction = nullptr;
	QAction* m_deleteFileAction = nullptr;
	QAction* m_moveFileAction = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	QAction* m_runAllTestsAction = nullptr;
	QAction* m_runCurrentTestsAction = nullptr;
	QAction* m_stopTestsAction = nullptr;
	QAction* m_refreshAction = nullptr;

	QAction* m_runSelectedTestsAction = nullptr;

	// Editor context menu actions
	//
	QAction* m_checkInCurrentDocumentAction = nullptr;
	QAction* m_checkOutCurrentDocumentAction = nullptr;
	QAction* m_undoChangesCurrentDocumentAction = nullptr;
	QAction* m_documentSeparatorAction1 = nullptr;
	QAction* m_runTestCurrentDocumentAction = nullptr;
	QAction* m_documentSeparatorAction2 = nullptr;
	QAction* m_saveCurrentDocumentAction = nullptr;
	QAction* m_closeCurrentDocumentAction = nullptr;

	// Open documents list actions
	//
	QAction* m_checkInOpenDocumentAction = nullptr;
	QAction* m_checkOutOpenDocumentAction = nullptr;
	QAction* m_undoChangesOpenDocumentAction = nullptr;
	QAction* m_saveOpenDocumentAction = nullptr;
	QAction* m_closeOpenDocumentAction = nullptr;
	QAction* m_runTestOpenDocumentAction = nullptr;

	// Build toolbar actions
	//
	QAction* m_selectBuildAction = nullptr;

	// --
	//
	OutputDockLog m_log;

	Sim::Simulator m_simulator{&m_log, nullptr};	// log to OutputLog, no parent
};


