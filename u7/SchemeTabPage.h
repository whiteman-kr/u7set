#pragma once

#include "MainTabPage.h"
#include "FileListView.h"
#include "../include/DbController.h"
#include "EditSchemeWidget.h"
#include "GlobalMessanger.h"

//
//
// SchemeFileView
//
//
class SchemeFileView : public FileListView
{
	Q_OBJECT
public:
	SchemeFileView(DbController* dbcontroller, const QString& parentFileName);
	virtual ~SchemeFileView();

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
// SchemesTabPage
//
//
class SchemesTabPage : public MainTabPage
{
	Q_OBJECT

private:
	SchemesTabPage(DbController* dbcontroller, QWidget* parent);

public:
	virtual ~SchemesTabPage();

	template<typename SchemeType>
	static SchemesTabPage* create(const QString& fileExt, DbController* dbcontroller, const QString& parentFileName, QWidget* parent);

	bool hasUnsavedSchemes() const;
	bool saveUnsavedSchemes();

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
protected:
	std::function<VFrame30::Scheme*()> m_createSchemeFunc;	// same as in SchemeControlTabPage
	QTabWidget* m_tabWidget;
};


//
//
// SchemeControlTabPage
//
//
class SchemeControlTabPage : public QWidget, public HasDbController
{
    Q_OBJECT
public:
	SchemeControlTabPage(
			const QString& fileExt,
			DbController* db,
			const QString& parentFileName,
			std::function<VFrame30::Scheme*()> createSchemeFunc) :
		HasDbController(db),
		m_createSchemeFunc(createSchemeFunc)
	{
		// Create actions
		//
		CreateActions();

		// Create controls
		//
		m_filesView = new SchemeFileView(db, parentFileName);
		m_filesView->filesModel().setFilter(fileExt);

		QHBoxLayout* pMainLayout = new QHBoxLayout();
		pMainLayout->addWidget(m_filesView);

		setLayout(pMainLayout);

		// --
		//
		//connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemeControlTabPage::projectOpened);
		//connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemeControlTabPage::projectClosed);

		connect(m_filesView, &SchemeFileView::openFileSignal, this, &SchemeControlTabPage::openFiles);
		connect(m_filesView, &SchemeFileView::viewFileSignal, this, &SchemeControlTabPage::viewFiles);
		connect(m_filesView, &SchemeFileView::addFileSignal, this, &SchemeControlTabPage::addFile);
		connect(m_filesView, &SchemeFileView::deleteFileSignal, this, &SchemeControlTabPage::deleteFile);
		connect(m_filesView, &SchemeFileView::checkInSignal, this, &SchemeControlTabPage::checkIn);
		connect(m_filesView, &SchemeFileView::undoChangesSignal, this, &SchemeControlTabPage::undoChanges);

		return;
	}

    virtual ~SchemeControlTabPage();

public:
	VFrame30::Scheme* createScheme() const;

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
	std::function<VFrame30::Scheme*()> m_createSchemeFunc;
    SchemeFileView* m_filesView;
};


// Create MainTab!!!
//
template<typename SchemeType>
SchemesTabPage* SchemesTabPage::create(const QString& fileExt, DbController* dbcontroller,  const QString& parentFileName, QWidget* parent)
{
	static_assert(std::is_base_of<VFrame30::Scheme, SchemeType>::value, "Base class must be VFrame30::Scheme");
	assert(dbcontroller != nullptr);

	SchemesTabPage* p = new SchemesTabPage(dbcontroller, parent);

	// Create Scheme function, will be stored in two places, SchemeTabPage and SchemeControlTabPage
	//
	std::function<VFrame30::Scheme*()> createFunc(
		[]() -> VFrame30::Scheme*
		{
			return new SchemeType();
		});

	p->m_createSchemeFunc = createFunc;

	// Add control page
	//
	SchemeControlTabPage* controlTabPage = new SchemeControlTabPage(fileExt, dbcontroller, parentFileName, createFunc);
	p->m_tabWidget->addTab(controlTabPage, tr("Control"));

	return p;
}


//
//
// EditSchemeTabPage
//
//
class EditSchemeTabPage : public QWidget, public HasDbController
{
	Q_OBJECT

public:
	EditSchemeTabPage() = delete;
	EditSchemeTabPage(std::shared_ptr<VFrame30::Scheme> scheme, const DbFileInfo& fileInfo, DbController* db);
	virtual ~EditSchemeTabPage();

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
	void getCurrentWorkcopy();				// Save current scheme to a file
	void setCurrentWorkcopy();				// Load a scheme from a file

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
	EditSchemeWidget* m_schemeWidget = nullptr;
	QToolBar* m_toolBar = nullptr;
};
