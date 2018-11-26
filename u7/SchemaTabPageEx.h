#pragma once

#include "MainTabPage.h"
//#include "SchemaListModel.h"
#include "../lib/DbController.h"
//#include "EditSchemaWidget.h"
#include "GlobalMessanger.h"
#include "../VFrame30/Schema.h"


//
//
// SchemaListModelEx
//
//
class SchemaListModelEx : public QAbstractItemModel
{
	Q_OBJECT

public:
	SchemaListModelEx(QObject* parent = nullptr);

public:
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	//virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	//void addFile(std::shared_ptr<DbFileInfo> file);

//	void setFiles(const std::vector<DbFileInfo>& files, const std::vector<DbUser>& users);
//	void clear();

//	std::shared_ptr<DbFileInfo> fileByRow(int row);
//	std::shared_ptr<DbFileInfo> fileByFileId(int fileId);

//	int getFileRow(int fileId) const;

	// Properties
	//
public:
	QString filter() const;
	void setFilter(const QString& value);		// "" -- no filter, "cdd" -- just cdd files

//	const std::vector<std::shared_ptr<DbFileInfo>>& files() const;

//	QString usernameById(int userId) const;

//	QString detailsColumnText(int fileId) const;
//	QString fileCaption(int fileId) const;
//	bool excludedFromBuild(int fileId) const;

	// Data
	//
public:
	enum Columns
	{
		FileNameColumn,
		FileCaptionColumn,
		FileStateColumn,
		FileUserColumn,
		FileActionColumn,
		FileIssuesColumn,
		FileDetailsColumn,

		// Add other column befor this line
		//
		ColumnCount
	};

private:
//	std::vector<std::shared_ptr<DbFileInfo>> m_files;
	QString m_filter;

	std::map<int, QString> m_users;
	std::map<int, VFrame30::SchemaDetails> m_details;		// Key is FileID
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
	SchemaFileViewEx(DbController* dbc, const QString& parentFileName);

	// Methods
	//
protected:
//	void CreateActions();

//	void timerEvent(QTimerEvent* event) override;

	// Methods
	//
public:
//	void setFiles(const std::vector<DbFileInfo>& files);
//	void clear();

//	void getSelectedFiles(std::vector<DbFileInfo>* out);

	void refreshFiles();

//signals:
//	void openFileSignal(std::vector<DbFileInfo> files);
//	void viewFileSignal(std::vector<DbFileInfo> files);
//	void cloneFileSignal(DbFileInfo file);
//	void addFileSignal();
//	void deleteFileSignal(std::vector<DbFileInfo> files);
//	void checkInSignal(std::vector<DbFileInfo> files);
//	void undoChangesSignal(std::vector<DbFileInfo> files);
//	void editSchemasProperties(std::vector<DbFileInfo> files);

	// Protected slots
	//
public slots:
//	void projectOpened();
//	void projectClosed();

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
//	void slot_doubleClicked(const QModelIndex& index);
//	void slot_properties();

//public slots:
//	void filesViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	// Public properties
	//
public:
	SchemaListModelEx& filesModel();

//	const std::vector<std::shared_ptr<DbFileInfo>>& files() const;

//	const DbFileInfo& parentFile() const;
//	int parentFileId() const;

//	// Protected properties
//	//
//protected:

	// Data
	//
private:
	SchemaListModelEx m_filesModel;

	QString m_parentFileName;
	DbFileInfo m_parentFile;

	int m_lastBuildIssueCount = -1;

//	//	Contexet Menu
//	//
//protected:
//	QAction* m_openFileAction = nullptr;
//	QAction* m_viewFileAction = nullptr;
//	// --
//	QAction* m_separatorAction0 = nullptr;
//	QAction* m_checkOutAction = nullptr;
//	QAction* m_checkInAction = nullptr;
//	QAction* m_undoChangesAction = nullptr;
//	QAction* m_historyAction = nullptr;
//	QAction* m_compareAction = nullptr;
//	QAction* m_allSchemasHistoryAction = nullptr;
//	// --
//	QAction* m_separatorAction1 = nullptr;
//	QAction* m_addFileAction = nullptr;
//	QAction* m_cloneFileAction = nullptr;
//	QAction* m_deleteFileAction = nullptr;
//	// --
//	QAction* m_separatorAction2 = nullptr;
//	QAction* m_exportWorkingcopyAction = nullptr;
//	QAction* m_importWorkingcopyAction = nullptr;
//	// --
//	QAction* m_separatorAction3 = nullptr;
//	QAction* m_refreshFileAction = nullptr;
//	QAction* m_propertiesAction = nullptr;
//	// End of ConextMenu
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
	SchemaControlTabPageEx(QString fileExt,
						   DbController* db,
						   QString parentFileName,
						   QString templateFileExtension,
						   std::function<VFrame30::Schema*()> createSchemaFunc);

	virtual ~SchemaControlTabPageEx();

public:
	VFrame30::Schema* createSchema() const;

private:
	void createActions();

//signals:

//public slots:
//	void refreshFiles();

protected slots:
	void projectOpened();
	void projectClosed();

//	void addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);
//	void addFile();
//	void addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, bool dontShowPropDialog);

//	void deleteFile(std::vector<DbFileInfo> files);

//	void checkIn(std::vector<DbFileInfo> files);
//	void undoChanges(std::vector<DbFileInfo> files);

//	void openFiles(std::vector<DbFileInfo> files);
//	void viewFiles(std::vector<DbFileInfo> files);
//	void cloneFile(DbFileInfo file);

//	void editSchemasProperties(std::vector<DbFileInfo> selectedFiles);


//private slots:
//	void ctrlF();
//	void search();
//	void searchSchemaForLm(QString equipmentId);

	// Properties
	//
public:
//	const DbFileInfo& parentFile() const;

	// Data
	//
private:
	std::function<VFrame30::Schema*()> m_createSchemaFunc;
	SchemaFileViewEx* m_filesView = nullptr;
	QString m_templateFileExtension;

	QToolBar* m_toolBar = nullptr;

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
	QAction* m_allSchemasHistoryAction = nullptr;

	// --
	QAction* m_exportWorkingcopyAction = nullptr;
	QAction* m_importWorkingcopyAction = nullptr;

	// --
	QAction* m_refreshFileAction = nullptr;
	QAction* m_propertiesAction = nullptr;

	QLineEdit* m_searchEdit = nullptr;
	QPushButton* m_searchButton = nullptr;
};


//
//
// SchemasTabPage
//
//
class SchemasTabPageEx : public MainTabPage
{
	Q_OBJECT

private:
	explicit SchemasTabPageEx(DbController* dbcontroller, QWidget* parent);

public:
	virtual ~SchemasTabPageEx();

public:
	template<typename SchemaType>
	static SchemasTabPageEx* create(DbController* dbc,
									QString fileExt,
									QString parentFileName,
									QString templFileExt,
									QString caption,
									QWidget* parent);

	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();

	void refreshControlTabPage();

	//std::vector<EditSchemaTabPageEx*> getOpenSchemas();

public slots:
	void projectOpened();
	void projectClosed();

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Data
	//
protected:
	QTabWidget* m_tabWidget;

	QString m_fileExtension;
	QString m_templFileExtension;
};


// Create MainTab!!!
//
template<typename SchemaType>
SchemasTabPageEx* SchemasTabPageEx::create(DbController* dbc,
										   QString fileExt,
										   QString parentFileName,
										   QString templFileExt,
										   QString caption,
										   QWidget* parent)
{
	static_assert(std::is_base_of<VFrame30::Schema, SchemaType>::value, "Base class must be VFrame30::Schema");
	assert(dbc != nullptr);

	SchemasTabPageEx* p = new SchemasTabPageEx(dbc, parent);

	p->m_fileExtension = fileExt;
	p->m_templFileExtension = templFileExt;

	// Create Schema function, will be stored in two places, SchemaTabPage and SchemaControlTabPage
	//
	std::function<VFrame30::Schema*()> createFunc(
		[]() -> VFrame30::Schema*
		{
			return new SchemaType();
		});

	// Add control page
	//
	SchemaControlTabPageEx* controlTabPage =
			new SchemaControlTabPageEx(fileExt, dbc, parentFileName, templFileExt, createFunc);

	p->m_tabWidget->addTab(controlTabPage, caption);

	return p;
}


//
//
// EditSchemaTabPage
//
//
//class EditSchemaTabPageEx : public QWidget, public HasDbController
//{
//	Q_OBJECT

//public:
//	EditSchemaTabPageEx() = delete;
//	EditSchemaTabPageEx(QTabWidget* tabWidget,
//					  std::shared_ptr<VFrame30::Schema> schema,
//					  const DbFileInfo& fileInfo,
//					  DbController* db,
//					  QObject* parent);

//	virtual ~EditSchemaTabPageEx();

//	// Public methods
//	//
//public:
//	void setPageTitle();

//	void updateAfbSchemaItems();
//	void updateUfbSchemaItems();
//	void updateBussesSchemaItems();

//protected:
//	void CreateActions();

//signals:
//	void vcsFileStateChanged();

//protected slots:
//	void closeTab();
//	void modifiedChanged(bool modified);

//	void checkInFile();
//	void checkOutFile();
//	void undoChangesFile();

//	void fileMenuTriggered();
//	void sizeAndPosMenuTriggered();
//	void itemsOrderTriggered();

//public:
//	bool saveWorkcopy();

//protected:
//	void getCurrentWorkcopy();				// Save current schema to a file
//	void setCurrentWorkcopy();				// Load a schema from a file

//	// Properties
//	//
//public:
//	std::shared_ptr<VFrame30::Schema> schema();

//	const DbFileInfo& fileInfo() const;
//	void setFileInfo(const DbFileInfo& fi);

//	bool readOnly() const;
//	void setReadOnly(bool value);

//	bool modified() const;
//	void resetModified();

//	bool compareWidget() const;
//	bool isCompareWidget() const;
//	void setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target);

//	void setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions);

//	// Data
//	//
//private:
//	EditSchemaWidget* m_schemaWidget = nullptr;
//	QToolBar* m_toolBar = nullptr;
//	QTabWidget* m_tabWidget = nullptr;
//};