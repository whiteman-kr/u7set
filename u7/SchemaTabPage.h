#pragma once

#include "MainTabPage.h"
#include "SchemaListModel.h"
#include "../lib/DbController.h"
#include "EditSchemaWidget.h"
#include "GlobalMessanger.h"

//
//
// SchemaFileView
//
//
class SchemaFileView : public QTableView, public HasDbController
{
	Q_OBJECT

public:
	SchemaFileView(DbController* dbcontroller, const QString& parentFileName);

	// Methods
	//
protected:
	void CreateActions();

	// Methods
	//
public:
	void setFiles(const std::vector<DbFileInfo>& files);
	void clear();

	void getSelectedFiles(std::vector<DbFileInfo>* out);

	void refreshFiles();

signals:
	void openFileSignal(std::vector<DbFileInfo> files);
	void viewFileSignal(std::vector<DbFileInfo> files);
	void cloneFileSignal(DbFileInfo file);
	void addFileSignal();
	void deleteFileSignal(std::vector<DbFileInfo> files);
	void checkInSignal(std::vector<DbFileInfo> files);
	void undoChangesSignal(std::vector<DbFileInfo> files);

	// Protected slots
	//
protected slots:
	void projectOpened();
	void projectClosed();

	void slot_OpenFile();
	void slot_ViewFile();
	void slot_CheckOut();
	void slot_CheckIn();
	void slot_UndoChanges();
	void slot_showHistory();
	void slot_compare();
	void slot_showHistoryForAllSchemas();
	void slot_AddFile();
	void slot_cloneFile();
	void slot_DeleteFile();
	void slot_GetWorkcopy();
	void slot_SetWorkcopy();
	void slot_RefreshFiles();
	void slot_doubleClicked(const QModelIndex& index);

public slots:
	void filesViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	// Public properties
	//
public:
	SchemaListModel& filesModel();

	const std::vector<std::shared_ptr<DbFileInfo>>& files() const;

	const DbFileInfo& parentFile() const;
	int parentFileId() const;

	// Protected properties
	//
protected:

	// Data
	//
private:
	SchemaListModel m_filesModel;

	QString m_parentFileName;
	DbFileInfo m_parentFile;

	//	Contexet Menu
	//
protected:
	QAction* m_openFileAction = nullptr;
	QAction* m_viewFileAction = nullptr;
	// --
	QAction* m_separatorAction0 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	QAction* m_allSchemasHistoryAction = nullptr;
	// --
	QAction* m_separatorAction1 = nullptr;
	QAction* m_addFileAction = nullptr;
	QAction* m_cloneFileAction = nullptr;
	QAction* m_deleteFileAction = nullptr;
	// --
	QAction* m_separatorAction2 = nullptr;
	QAction* m_exportWorkingcopyAction = nullptr;
	QAction* m_importWorkingcopyAction = nullptr;
	// --
	QAction* m_separatorAction3 = nullptr;
	QAction* m_refreshFileAction = nullptr;
	// End of ConextMenu
};


//
//
// SchemaControlTabPage
//
//
class SchemaControlTabPage : public QWidget, public HasDbController
{
	Q_OBJECT

public:
	SchemaControlTabPage(QString fileExt,
						 DbController* db,
						 QString parentFileName,
						 QString templateFileExtension,
						 std::function<VFrame30::Schema*()> createSchemaFunc);

	virtual ~SchemaControlTabPage();

public:
	VFrame30::Schema* createSchema() const;

signals:

public slots:
	void refreshFiles();

protected slots:
	void addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);
	void addFile();
	void addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, bool dontShowPropDialog);

	void deleteFile(std::vector<DbFileInfo> files);

	void checkIn(std::vector<DbFileInfo> files);
	void undoChanges(std::vector<DbFileInfo> files);

	void openFiles(std::vector<DbFileInfo> files);
	void viewFiles(std::vector<DbFileInfo> files);
	void cloneFile(DbFileInfo file);


private slots:
	void ctrlF();
	void search();
	void searchSchemaForLm(QString equipmentId);

	// Properties
	//
public:
	const DbFileInfo& parentFile() const;

	// Data
	//
private:
	std::function<VFrame30::Schema*()> m_createSchemaFunc;
	SchemaFileView* m_filesView = nullptr;
	QString m_templateFileExtension;

	QLineEdit* m_searchEdit = nullptr;
	QPushButton* m_searchButton = nullptr;

	QAction* m_searchAction = nullptr;
	QAction* m_refreshAction = nullptr;
};



//
//
// SchemasTabPage
//
//
class SchemasTabPage : public MainTabPage
{
	Q_OBJECT

private:
	SchemasTabPage(DbController* dbcontroller, QWidget* parent);

public:
	virtual ~SchemasTabPage();

	template<typename SchemaType>
	static SchemasTabPage* create(
			DbController* dbcontroller,	QWidget* parent,
			QString fileExt, QString parentFileName, QString templFileExt, QString caption);

	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();

	void refreshControlTabPage();

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
SchemasTabPage* SchemasTabPage::create(
		DbController* dbcontroller,	QWidget* parent,
		QString fileExt, QString parentFileName, QString templFileExt, QString caption)
{
	static_assert(std::is_base_of<VFrame30::Schema, SchemaType>::value, "Base class must be VFrame30::Schema");
	assert(dbcontroller != nullptr);

	SchemasTabPage* p = new SchemasTabPage(dbcontroller, parent);

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
	SchemaControlTabPage* controlTabPage =
			new SchemaControlTabPage(fileExt, dbcontroller, parentFileName, templFileExt, createFunc);

	p->m_tabWidget->addTab(controlTabPage, caption);

	return p;
}


//
//
// EditSchemaTabPage
//
//
class EditSchemaTabPage : public QWidget, public HasDbController
{
	Q_OBJECT

public:
	EditSchemaTabPage() = delete;
	EditSchemaTabPage(QTabWidget* tabWidget, std::shared_ptr<VFrame30::Schema> schema, const DbFileInfo& fileInfo, DbController* db);
	virtual ~EditSchemaTabPage();

	// Public methods
	//
public:
	void setPageTitle();

	void updateAfbSchemaItems();
	void updateUfbSchemaItems();
	void updateBussesSchemaItems();

protected:
	void CreateActions();

signals:
	void vcsFileStateChanged();

protected slots:
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
