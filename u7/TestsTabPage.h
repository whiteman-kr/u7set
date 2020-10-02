#pragma once

#include "MainTabPage.h"
#include <QToolBar>
#include "IdePropertyEditor.h"
#include "GlobalMessanger.h"
#include "../lib/Ui/FilesTreeView.h"

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

//
// TestsTabPage
//

class TestsTabPage : public MainTabPage
{
	Q_OBJECT
public:
	explicit TestsTabPage(DbController* dbc, QWidget* parent);
	virtual ~TestsTabPage();

	bool hasUnsavedTests() const;
	void saveUnsavedTests();
	void resetModified();

private slots:

	// Project operations

	void projectOpened();
	void projectClosed();
	void buildStarted();

	// Tree controls operations

	void testsTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void testsTreeModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
	void testsTreeModelReset();
	void testsTreeDoubleClicked(const QModelIndex &index);

	void openFilesClicked(const QModelIndex &index);

	// Tests file tree slots

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
	void runTestFiles();

	// Code Editor slots

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

	// Open Files slots

	void openFilesMenuRequested(const QPoint &pos);

	void checkInOpenFile();
	void checkOutOpenFile();
	void undoChangesOpenFile();
	void saveOpenFile();
	void closeOpenFile();
	void runTestOpenFile();

	// Hotkeys

	void onSaveKeyPressed();
	void onCloseKeyPressed();
	void onCtrlTabKeyPressed();

	// Open documents combo operations

	void openDocumentsComboTextChanged(int index);

	// Other operations

	void closeDocumentsForDeletedFiles();

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Build operations slots

	void selectBuild();

	void runSimTests(const QString& buildPath, const std::vector<std::shared_ptr<DbFile>>& files);

private:
	void createUi();
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
	virtual void keyPressEvent(QKeyEvent* event) override;

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

	QVBoxLayout* m_rightLayout = nullptr;

	QVBoxLayout* m_editorLayout = nullptr;

	QToolBar* m_editorToolBar = nullptr;
	QLabel* m_editorEmptyLabel = nullptr;

	QComboBox* m_openDocumentsCombo = nullptr;
	QPushButton* m_cursorPosButton = nullptr;

	QToolBar* m_buildToolBar = nullptr;
	QLabel* m_buildLabel = nullptr;

	QSplitter* m_leftSplitter = nullptr;
	QSplitter* m_verticalSplitter = nullptr;

	// Tests file tree actions

	QAction* m_newFileAction = nullptr;
	QAction* m_SeparatorAction1 = nullptr;
	QAction* m_addFileAction = nullptr;
	QAction* m_newFolderAction = nullptr;
	QAction* m_openFileAction = nullptr;
	QAction* m_renameFileAction = nullptr;
	QAction* m_deleteFileAction = nullptr;
	QAction* m_moveFileAction = nullptr;
	QAction* m_SeparatorAction2 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	QAction* m_SeparatorAction3 = nullptr;
	QAction* m_runTestsAction = nullptr;
	QAction* m_SeparatorAction4 = nullptr;
	QAction* m_refreshAction = nullptr;

	// Editor context menu actions

	QAction* m_checkInCurrentDocumentAction = nullptr;
	QAction* m_checkOutCurrentDocumentAction = nullptr;
	QAction* m_undoChangesCurrentDocumentAction = nullptr;
	QAction* m_saveCurrentDocumentAction = nullptr;
	QAction* m_closeCurrentDocumentAction = nullptr;
	QAction* m_runTestCurrentDocumentAction = nullptr;

	// Open documents list actions

	QAction* m_checkInOpenDocumentAction = nullptr;
	QAction* m_checkOutOpenDocumentAction = nullptr;
	QAction* m_undoChangesOpenDocumentAction = nullptr;
	QAction* m_saveOpenDocumentAction = nullptr;
	QAction* m_closeOpenDocumentAction = nullptr;
	QAction* m_runTestOpenDocumentAction = nullptr;

	// Build toolbar actions

	QAction* m_selectBuildAction = nullptr;
};


