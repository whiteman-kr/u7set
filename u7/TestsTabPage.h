#pragma once

#include "MainTabPage.h"

#include <QToolBar>

#include "IdePropertyEditor.h"

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
	void testsTreeDoubleClicked(const QModelIndex &index);
	void openFilesDoubleClicked(const QModelIndex &index);

	void newDocument();
	void openDocument();
	void addFolder();
	void filterChanged();
	void textChanged();
	void cursorPositionChanged(int line, int index);
	void closeCurrentDocument();

	void onSaveKeyPressed();
	void onCloseKeyPressed();
	void onCtrlTabKeyPressed();

	void onGoToLine();


private:
	void createUi();
	void createActions();

	void saveSettings();
	void restoreSettings();

	void setActionState();

	bool documentIsOpen(const QString& fileName);
	void setCurrentDocument(const QString& fileName);
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

	// Widgets
	//
	FileTreeView* m_testsTreeView = nullptr;
	TestsFileTreeModel* m_testsTreeModel = nullptr;
	FileTreeProxyModel* m_testsTreeProxyModel = nullptr;

	QTreeWidget* m_openFilesTreeWidget = nullptr;

	QLineEdit* m_filterLineEdit = nullptr;
	QCompleter* m_filterCompleter = nullptr;

	QVBoxLayout* m_editorLayout = nullptr;

	QToolBar* m_editorToolBar = nullptr;
	QLabel* m_editorEmptyLabel = nullptr;

	QLabel* m_editorFileNameLabel = nullptr;
	QPushButton* m_lineButton = nullptr;
	QLabel* m_columnLabel = nullptr;

	QSplitter* m_leftSplitter = nullptr;
	QSplitter* m_verticalSplitter = nullptr;

	//Actions
	//
	QAction* m_newFileAction = nullptr;
	QAction* m_addFileAction = nullptr;
	QAction* m_addFolderAction = nullptr;
	QAction* m_openFileAction = nullptr;
	QAction* m_renameFileAction = nullptr;
	QAction* m_deleteFileAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction1 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction2 = nullptr;
	QAction* m_refreshAction = nullptr;

};


