#pragma once

#include "MainTabPage.h"
#include "FileView.h"
#include "../include/DbController.h"
#include "EditSchemeWidget.h"

//
//
// VideoFrameFileView
//
//
class SchemeFileView : public FileView
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
	virtual void deleteFile(std::vector<DbFileInfo> files) override;

signals:
	void openFileSignal(std::vector<DbFileInfo> files);
	void viewFileSignal(std::vector<DbFileInfo> files);
	void addFileSignal();
	void deleteFileSignal(std::vector<DbFileInfo> files);

	// Data
	//
protected:
};

//
//
// EditVideoFrameTabPage
//
//
class SchemesTabPage : public MainTabPage
{
	Q_OBJECT

private:
	SchemesTabPage(DbController* dbcontroller, QWidget* parent);

public:
	virtual ~SchemesTabPage();

	template<typename VideoFrameType>
	static SchemesTabPage* create(const QString& fileExt, DbController* dbcontroller, const QString& parentFileName, QWidget* parent);

protected:

	// Events
	//
protected:

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
protected:
	std::function<VFrame30::Scheme*()> m_createVideoFrameFunc;	// same as in VideoFrameControlTabPage
	QTabWidget* m_tabWidget;
};


//
//
// VideoFrameControlTabPage
//
//
class SchemeControlTabPage : public QWidget, public HasDbController
{
    Q_OBJECT
public:
    SchemeControlTabPage(
        const QString& fileExt,
        DbController* dbcontroller,
        const QString& parentFileName,
        std::function<VFrame30::Scheme*()> createVideoFrameFunc);

    virtual ~SchemeControlTabPage();

public:
    VFrame30::Scheme* createVideoFrame() const;

protected:
    void CreateActions();

signals:

protected slots:
    //void projectOpened();
    //void projectClosed();

    void addFile();
    void deleteFile(std::vector<DbFileInfo> files);

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
    std::function<VFrame30::Scheme*()> m_createVideoFrameFunc;
    SchemeFileView* m_filesView;
};


// Create MainTab!!!
//
template<typename VideoFrameType>
SchemesTabPage* SchemesTabPage::create(const QString& fileExt, DbController* dbcontroller,  const QString& parentFileName, QWidget* parent)
{
	static_assert(std::is_base_of<VFrame30::Scheme, VideoFrameType>::value, "Base class must be VFrame30::CVideoFrame");
	assert(dbcontroller != nullptr);

	SchemesTabPage* p = new SchemesTabPage(dbcontroller, parent);

	// Create VideoFrame function, will be stored in two places, VideoFrameTabPage and VideoFrameControlTabPage
	//
	std::function<VFrame30::Scheme*()> createFunc(
		[]() -> VFrame30::Scheme*
		{
			return new VideoFrameType();
		});

	p->m_createVideoFrameFunc = createFunc;

	// Add control page
	//
	SchemeControlTabPage* controlTabPage = new SchemeControlTabPage(fileExt, dbcontroller, parentFileName, createFunc);
	p->m_tabWidget->addTab(controlTabPage, tr("Control"));

	return p;
}


//
//
// EditVideoFrameTabPage
//
//
class EditSchemeTabPage : public QWidget, public HasDbController
{
	Q_OBJECT
private:
	EditSchemeTabPage();		// Deleted
public:
	EditSchemeTabPage(std::shared_ptr<VFrame30::Scheme> videoFrame, const DbFileInfo& fileInfo, DbController* dbcontroller);
	virtual ~EditSchemeTabPage();

protected:
	void CreateActions();
	void setPageTitle();

signals:
	void vcsFileStateChanged();

protected slots:
	void closeTab();
	void modifiedChanged(bool modified);

	void checkInFile();
	void checkOutFile();
	void undoChangesFile();
	bool saveWorkcopy();
	void getCurrentWorkcopy();				// Save current videoframe to a file
	void setCurrentWorkcopy();				// Load a videoframe from a file

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
	EditSchemeWidget* m_videoFrameWidget;
};


