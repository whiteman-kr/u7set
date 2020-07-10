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

struct TestTabPageDocument
{
	IdeCodeEditor* codeEditor = nullptr;
	bool readOnly = false;
	bool modified = false;
	QTreeWidgetItem* treeWidgetItem = nullptr;
	std::shared_ptr<DbFile> dbFile;
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

private slots:
	void projectOpened();
	void projectClosed();

	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
	void modelReset();
	void testsTreeDoubleClicked(const QModelIndex &index);
	void openFilesDoubleClicked(const QModelIndex &index);

	void newFile();
	void openFile();
	void newFolder();
	void filterChanged();
	void textChanged();
	void cursorPositionChanged(int line, int index);
	void closeCurrentDocument();

	void onSaveKeyPressed();
	void onCloseKeyPressed();
	void onCtrlTabKeyPressed();

	void onGoToLine();
	void setCurrentDocument(const QString& fileName);

	void compareObject(DbChangesetObject object, CompareData compareData);

private:
	void createUi();
	void createActions();

	void saveSettings();
	void restoreSettings();

	void setActionState();

	bool documentIsOpen(const QString& fileName);
	void saveDocument(const QString& fileName);
	void closeDocument(const QString& fileName);
	void closeAllDocuments();
	void hideEditor();

	virtual void keyPressEvent(QKeyEvent* event) override;

private:
	// Data
	//
	std::map<QString, TestTabPageDocument> m_openDocuments;
	QString m_currentDocument;
	QFont m_editorFont;

	QStringList m_editableExtensions;

	// Widgets
	//

	QToolBar* m_testsToolbar = nullptr;

	FileTreeView* m_testsTreeView = nullptr;
	TestsFileTreeModel* m_testsTreeModel = nullptr;
	FileTreeProxyModel* m_testsTreeProxyModel = nullptr;


	QLineEdit* m_filterLineEdit = nullptr;
	QCompleter* m_filterCompleter = nullptr;

	QTreeWidget* m_openFilesTreeWidget = nullptr;

	QVBoxLayout* m_editorLayout = nullptr;

	QToolBar* m_editorToolBar = nullptr;
	QLabel* m_editorEmptyLabel = nullptr;

	QComboBox* m_openDocumentsCombo = nullptr;
	QPushButton* m_cursorPosButton = nullptr;

	QSplitter* m_leftSplitter = nullptr;
	QSplitter* m_verticalSplitter = nullptr;

	//Actions
	//
	QAction* m_newFileAction = nullptr;
	QAction* m_SeparatorAction1 = nullptr;

	QAction* m_addFileAction = nullptr;
	QAction* m_newFolderAction = nullptr;
	QAction* m_openFileAction = nullptr;
	QAction* m_renameFileAction = nullptr;
	QAction* m_deleteFileAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction2 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction3 = nullptr;
	QAction* m_refreshAction = nullptr;
};


