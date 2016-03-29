#pragma once

#include "MainTabPage.h"
#include "FileListView.h"
#include "../include/DbController.h"
#include "EditSchemaWidget.h"
#include "GlobalMessanger.h"

//
//
// SchemaFileView
//
//
class SchemaFileView : public FileListView
{
	Q_OBJECT
public:
	SchemaFileView(DbController* dbcontroller, const QString& parentFileName);
	virtual ~SchemaFileView();

	// Methods
	//
public:
	virtual void openFile(std::vector<DbFileInfo> files) override;
	virtual void viewFile(std::vector<DbFileInfo> files) override;
	virtual void addFile() override;
	virtual void checkIn(std::vector<DbFileInfo> files) override;
	virtual void undoChanges(std::vector<DbFileInfo> files) override;
	virtual void deleteFile(std::vector<DbFileInfo> files) override;
	virtual void fileDoubleClicked(DbFileInfo file) override;

signals:
	void openFileSignal(std::vector<DbFileInfo> files);
	void viewFileSignal(std::vector<DbFileInfo> files);
	void addFileSignal();
	void deleteFileSignal(std::vector<DbFileInfo> files);
	void checkInSignal(std::vector<DbFileInfo> files);
	void undoChangesSignal(std::vector<DbFileInfo> files);

	// Data
	//
protected:
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
	static SchemasTabPage* create(const QString& fileExt, DbController* dbcontroller, const QString& parentFileName, QWidget* parent);

	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
protected:
	std::function<VFrame30::Schema*()> m_createSchemaFunc;	// same as in SchemaControlTabPage
	QTabWidget* m_tabWidget;
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
	SchemaControlTabPage(
			const QString& fileExt,
			DbController* db,
			const QString& parentFileName,
			std::function<VFrame30::Schema*()> createSchemaFunc) :
		HasDbController(db),
		m_createSchemaFunc(createSchemaFunc)
	{
		// Create actions
		//
		CreateActions();

		// Create controls
		//
		m_filesView = new SchemaFileView(db, parentFileName);
		m_filesView->filesModel().setFilter(fileExt);

		QHBoxLayout* pMainLayout = new QHBoxLayout();
		pMainLayout->addWidget(m_filesView);

		setLayout(pMainLayout);

		// --
		//
		//connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaControlTabPage::projectOpened);
		//connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaControlTabPage::projectClosed);

		connect(m_filesView, &SchemaFileView::openFileSignal, this, &SchemaControlTabPage::openFiles);
		connect(m_filesView, &SchemaFileView::viewFileSignal, this, &SchemaControlTabPage::viewFiles);
		connect(m_filesView, &SchemaFileView::addFileSignal, this, &SchemaControlTabPage::addFile);
		connect(m_filesView, &SchemaFileView::deleteFileSignal, this, &SchemaControlTabPage::deleteFile);
		connect(m_filesView, &SchemaFileView::checkInSignal, this, &SchemaControlTabPage::checkIn);
		connect(m_filesView, &SchemaFileView::undoChangesSignal, this, &SchemaControlTabPage::undoChanges);

		return;
	}

	virtual ~SchemaControlTabPage();

public:
	VFrame30::Schema* createSchema() const;

protected:
    void CreateActions();

signals:

protected slots:
    //void projectOpened();
    //void projectClosed();

    void addFile();
    void deleteFile(std::vector<DbFileInfo> files);

	void checkIn(std::vector<DbFileInfo> files);
	void undoChanges(std::vector<DbFileInfo> files);

    void openFiles(std::vector<DbFileInfo> files);
    void viewFiles(std::vector<DbFileInfo> files);

    void refreshFiles();

    // Properties
    //
public:
    const DbFileInfo& parentFile() const;

    // Data
    //
private:
	std::function<VFrame30::Schema*()> m_createSchemaFunc;
	SchemaFileView* m_filesView;
};


// Create MainTab!!!
//
template<typename SchemaType>
SchemasTabPage* SchemasTabPage::create(const QString& fileExt, DbController* dbcontroller,  const QString& parentFileName, QWidget* parent)
{
	static_assert(std::is_base_of<VFrame30::Schema, SchemaType>::value, "Base class must be VFrame30::Schema");
	assert(dbcontroller != nullptr);

	SchemasTabPage* p = new SchemasTabPage(dbcontroller, parent);

	// Create Schema function, will be stored in two places, SchemaTabPage and SchemaControlTabPage
	//
	std::function<VFrame30::Schema*()> createFunc(
		[]() -> VFrame30::Schema*
		{
			return new SchemaType();
		});

	p->m_createSchemaFunc = createFunc;

	// Add control page
	//
	SchemaControlTabPage* controlTabPage = new SchemaControlTabPage(fileExt, dbcontroller, parentFileName, createFunc);
	p->m_tabWidget->addTab(controlTabPage, tr("Control"));

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
	EditSchemaTabPage(std::shared_ptr<VFrame30::Schema> schema, const DbFileInfo& fileInfo, DbController* db);
	virtual ~EditSchemaTabPage();

	// Public methods
public:
	void setPageTitle();

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

	// Data
	//
private:
	EditSchemaWidget* m_schemaWidget = nullptr;
	QToolBar* m_toolBar = nullptr;
};
