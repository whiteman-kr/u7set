#pragma once

#include "MainTabPage.h"
#include "FileListView.h"
#include "../lib/DbController.h"
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

protected:
	void CreateActions();

signals:

protected slots:
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
	QString m_templateFileExtension;
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

	template<typename SchemaType1, typename SchemaType2>
	static SchemasTabPage* create(
			DbController* dbcontroller, QWidget* parent,
			QString fileExt1, QString parentFileName1, QString templFileExt1, QString caption1,
			QString fileExt2, QString parentFileName2, QString templFileExt2, QString caption2);

	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
protected:
	QTabWidget* m_tabWidget;
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

template<typename SchemaType1, typename SchemaType2>
SchemasTabPage* SchemasTabPage::create(DbController* dbcontroller, QWidget* parent,
		QString fileExt1, QString parentFileName1, QString templFileExt1, QString caption1,
		QString fileExt2, QString parentFileName2, QString templFileExt2, QString caption2)
{
	static_assert(std::is_base_of<VFrame30::Schema, SchemaType1>::value, "Base class must be VFrame30::Schema");
	static_assert(std::is_base_of<VFrame30::Schema, SchemaType2>::value, "Base class must be VFrame30::Schema");

	assert(dbcontroller != nullptr);

	SchemasTabPage* p = new SchemasTabPage(dbcontroller, parent);

	// Create Schema function, will be stored in two places, SchemaTabPage and SchemaControlTabPage
	//
	std::function<VFrame30::Schema*()> createFunc1(
		[]() -> VFrame30::Schema*
		{
			return new SchemaType1();
		});

	std::function<VFrame30::Schema*()> createFunc2(
		[]() -> VFrame30::Schema*
		{
			return new SchemaType2();
		});

	// Add control page
	//

	SchemaControlTabPage* controlTabPage1 =
			new SchemaControlTabPage(fileExt1, dbcontroller, parentFileName1, templFileExt1, createFunc1);

	p->m_tabWidget->addTab(controlTabPage1, caption1);

	SchemaControlTabPage* controlTabPage2 =
			new SchemaControlTabPage(fileExt2, dbcontroller, parentFileName2, templFileExt2, createFunc2);

	p->m_tabWidget->addTab(controlTabPage2, caption2);

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
	//
public:
	void setPageTitle();
	void updateAfbSchemaItems();

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

	// Data
	//
private:
	EditSchemaWidget* m_schemaWidget = nullptr;
	QToolBar* m_toolBar = nullptr;
};
