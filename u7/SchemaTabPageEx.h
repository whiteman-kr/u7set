#pragma once

#include <QTableView>
#ifdef _DEBUG
	#include <QAbstractItemModelTester>
#endif
#include "MainTabPage.h"
//#include "SchemaListModel.h"
#include "../lib/DbController.h"
#include "GlobalMessanger.h"
#include "EditSchemaWidget.h"
#include "../VFrame30/LogicSchema.h"

class EditSchemaTabPageEx;

//
//
// SchemaListModelEx
//
//
class SchemaListModelEx : public QAbstractItemModel, protected HasDbController
{
	Q_OBJECT

public:
	SchemaListModelEx(DbController* dbc, QWidget* parentWidget);

public:
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
	std::pair<QModelIndex, bool> addFile(QModelIndex parentIndex, std::shared_ptr<DbFileInfo> file);
	bool deleteFiles(const QModelIndexList& selectedIndexes, const std::vector<std::shared_ptr<DbFileInfo>>& files);

	bool updateFiles(const QModelIndexList& selectedIndexes, const std::vector<DbFileInfo>& files);

	//	void setFiles(const std::vector<DbFileInfo>& files, const std::vector<DbUser>& users);
	//	void clear();

	//	std::shared_ptr<DbFileInfo> fileByRow(int row);
	//	std::shared_ptr<DbFileInfo> fileByFileId(int fileId);

	DbFileInfo file(const QModelIndex& modelIndex) const;

public slots:
	void refresh();

private slots:
	void projectOpened(DbProject project);
	void projectClosed();

	// Properties
	//
public:
//	QString filter() const;
//	void setFilter(const QString& value);		// "" -- no filter, "cdd" -- just cdd files

	QString usernameById(int userId) const noexcept;

	QString detailsColumnText(int fileId) const;
	QString fileCaption(int fileId) const;
	bool excludedFromBuild(int fileId) const;

	const DbFileInfo& parentFile() const;

	// Data
	//
public:
	enum class Columns
	{
		FileNameColumn,
		CaptionColumn,
		FileStateColumn,
		FileActionColumn,
		ChangesetColumn,
		FileUserColumn,
		IssuesColumn,
		DetailsColumn,

		// Add other column befor this line
		//
		ColumnCount
	};

private:
	DbFileInfo m_parentFile;

	QWidget* m_parentWidget = nullptr;	// Inside this model DbController is used, and it requires parent widget for
										// displaying progress and error messages
	DbFileTree m_files;
	//QString m_filter;

	std::map<int, QString> m_users;							// Key is UserID
	std::map<int, VFrame30::SchemaDetails> m_details; 		// Key is FileID
};


class SchemaProxyListModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	SchemaProxyListModel(QObject* parent = nullptr);
	virtual ~SchemaProxyListModel();

	virtual void setSourceModel(QAbstractItemModel* sourceModel) override;

public:
	DbFileInfo file(const QModelIndex& mi) const;
	std::vector<int> expandedFileIds();

private:
	SchemaListModelEx* m_sourceModel = nullptr;
};

//
//
// SchemaFileViewEx
//
//
class SchemaFileViewEx : public QTreeView, public HasDbController
{
	Q_OBJECT

public:
	SchemaFileViewEx(DbController* dbc, QWidget* parent);
	virtual ~SchemaFileViewEx();

	// Methods
	//
protected:
	void createActions();
	void createContextMenu();

	//	void timerEvent(QTimerEvent* event) override;

	// Methods
	//
public:
	//	void setFiles(const std::vector<DbFileInfo>& files);
	//	void clear();

	std::vector<DbFileInfo> selectedFiles() const;

	void refreshFiles();

signals:
	void openFileSignal(DbFileInfo files);
	void viewFileSignal(DbFileInfo files);
	//	void cloneFileSignal(DbFileInfo file);
	//	void addFileSignal();
	//	void deleteFileSignal(std::vector<DbFileInfo> files);
	//	void checkInSignal(std::vector<DbFileInfo> files);
	//	void undoChangesSignal(std::vector<DbFileInfo> files);
	//	void editSchemasProperties(std::vector<DbFileInfo> files);

	// Protected slots
	//
public slots:
	void projectOpened();
	void projectClosed();

	//	void slot_OpenFile();
	//	void slot_ViewFile();
	//	void slot_CheckOut();
	//	void slot_CheckIn();
	//	void slot_UndoChanges();
	//	void slot_showHistory();
	//	void slot_compare();
	//	void slot_showHistoryForAllSchemas();
	//	void slot_AddFile();
	//	void slot_cloneFile();
	//	void slot_DeleteFile();
	//	void slot_GetWorkcopy();
	//	void slot_SetWorkcopy();
		void slot_refreshFiles();
		void slot_doubleClicked(const QModelIndex& index);
	//	void slot_properties();

public slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	//	void filesViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	// Public properties
	//
public:
	SchemaListModelEx& filesModel();
	SchemaProxyListModel& proxyModel();

	//	const std::vector<std::shared_ptr<DbFileInfo>>& files() const;

	const DbFileInfo& parentFile() const;
	int parentFileId() const;

	// Protected properties
	//
protected:

	// Data
	//
private:
	SchemaListModelEx m_filesModel;
	SchemaProxyListModel m_proxyModel;

	int m_lastBuildIssueCount = -1;

	// Actions, public becaus they are used in control page toolbar
	//
public:
	// --
	QAction* m_newFileAction = nullptr;
	QAction* m_openAction = nullptr;
	QAction* m_viewAction = nullptr;
	QAction* m_cloneFileAction = nullptr;
	QAction* m_deleteAction = nullptr;

	// --
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	QAction* m_treeSchemasHistoryAction = nullptr;

	// --
	QAction* m_exportWorkingcopyAction = nullptr;
	QAction* m_importWorkingcopyAction = nullptr;

	// --
	QAction* m_refreshFileAction = nullptr;
	QAction* m_propertiesAction = nullptr;
};


//
//
// SchemaControlTabPage
//
//
class SchemaControlTabPageEx : public QWidget, public HasDbController
{
	Q_OBJECT

public:
	SchemaControlTabPageEx(DbController* db);
	virtual ~SchemaControlTabPageEx();

public:
	VFrame30::Schema* createSchema() const;		// To delete????

	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();
	bool resetModified();

private:
	void createToolBar();

	std::shared_ptr<VFrame30::Schema> createSchema(const DbFileInfo& parentFile) const;

public slots:
	void closeFile(EditSchemaTabPageEx* editTabPage);
	void detachOrAttachWindow(EditSchemaTabPageEx* editTabPage);

protected slots:
	void projectOpened();
	void projectClosed();

	int showSelectFileDialog(int currentSelectionFileId);

	void openSelectedFile();
	void viewSelectedFile();

	void openFile(const DbFileInfo& file);
	void viewFile(const DbFileInfo& file);

	void addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);
	void addFile();

	void addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, bool dontShowPropDialog);
	void addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, int parentFileId);
	void addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, QModelIndex parentIndex);

	void cloneFile();
	void deleteFiles();

	void checkOutFiles();
	void checkInFiles();

	//	void checkIn(std::vector<DbFileInfo> files);
	//	void undoChanges(std::vector<DbFileInfo> files);

	//	void openFiles(std::vector<DbFileInfo> files);
	//	void viewFiles(std::vector<DbFileInfo> files);

	//	void editSchemasProperties(std::vector<DbFileInfo> selectedFiles);


	//private slots:
	//	void ctrlF();
	//	void search();
	//	void searchSchemaForLm(QString equipmentId);

	// Properties
	//
public:
	const DbFileInfo& parentFile() const;

	// Data
	//
private:
	SchemaFileViewEx* m_filesView = nullptr;
	QToolBar* m_toolBar = nullptr;
	QLineEdit* m_searchEdit = nullptr;
	QPushButton* m_searchButton = nullptr;

	std::map<DbFileInfo, EditSchemaTabPageEx*> m_openedFiles;	// Opened files (for edit and view)
};


//
//
// SchemasTabPageEx - the main tab page added to IDE
//
//
class SchemasTabPageEx : public MainTabPage
{
	Q_OBJECT

public:
	explicit SchemasTabPageEx(DbController* dbc, QWidget* parent);
	virtual ~SchemasTabPageEx();

public:
	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();
	bool resetModified();

	void refreshControlTabPage();

	//std::vector<EditSchemaTabPageEx*> getOpenSchemas();

public slots:
	void projectOpened();
	void projectClosed();

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Data
	//
protected:
	QTabWidget* m_tabWidget = nullptr;
	SchemaControlTabPageEx* m_controlTabPage = nullptr;

	QString m_fileExtension;
	QString m_templFileExtension;
};


//
//
// EditSchemaTabPage
//
//
class EditSchemaTabPageEx : public QMainWindow, public HasDbController
{
	Q_OBJECT

public:
	EditSchemaTabPageEx() = delete;
	EditSchemaTabPageEx(QTabWidget* tabWidget,
					  std::shared_ptr<VFrame30::Schema> schema,
					  const DbFileInfo& fileInfo,
					  DbController* db);
	virtual ~EditSchemaTabPageEx();

protected:
	virtual void closeEvent(QCloseEvent* event) override;

	// Public methods
	//
public:
	void ensureVisible();
	void setPageTitle();
	void updateZoomAndScrolls(bool repaint);

	void updateAfbSchemaItems();
	void updateUfbSchemaItems();
	void updateBussesSchemaItems();

signals:
	void vcsFileStateChanged();
	void aboutToClose(EditSchemaTabPageEx*);
	void pleaseDetachOrAttachWindow(EditSchemaTabPageEx*);

public slots:
	void detachOrAttachWindow();

protected slots:
	void projectClosed();

	void closeTab();
	void modifiedChanged(bool modified);

	void checkInFile();
	void checkOutFile();
	void undoChangesFile();

	void fileMenuTriggered();
	void sizeAndPosMenuTriggered();
	void itemsOrderTriggered();

public:
	bool saveWorkcopy();

protected:
	void getCurrentWorkcopy();				// Save current schema to a file
	void setCurrentWorkcopy();				// Load a schema from a file

	// Properties
	//
public:
	std::shared_ptr<VFrame30::Schema> schema();

	const DbFileInfo& fileInfo() const;
	void setFileInfo(const DbFileInfo& fi);

	bool readOnly() const;
	void setReadOnly(bool value);

	bool modified() const;
	void resetModified();

	bool compareWidget() const;
	bool isCompareWidget() const;
	void setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target);

	void setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions);

	// Data
	//
private:
	EditSchemaWidget* m_schemaWidget = nullptr;
	QToolBar* m_toolBar = nullptr;
	QTabWidget* m_tabWidget = nullptr;
};
