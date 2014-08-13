#ifndef EDITVIDEOFRAMETABPAGE_H
#define EDITVIDEOFRAMETABPAGE_H

#include "MainTabPage.h"
#include "FileView.h"
#include "../include/DbStore.h"
#include "EditVideoFrameWidget.h"

//
//
// VideoFrameFileView
//
//
class VideoFrameFileView : public FileView
{
	Q_OBJECT
public:
	VideoFrameFileView(DbStore* dbstore);
	virtual ~VideoFrameFileView();

	// Methods
	//
public:
	virtual void openFile(std::vector<DbFileInfo> files) override;
	virtual void viewFile(std::vector<DbFileInfo> files) override;
	virtual void addFile() override;

signals:
	void openFileSignal(std::vector<DbFileInfo> files);
	void viewFileSignal(std::vector<DbFileInfo> files);
	void addFileSignal();

	// Data
	//
protected:
};

//
//
// EditVideoFrameTabPage
//
//
class VideoFrameTabPage : public MainTabPage
{
	Q_OBJECT

private:
	VideoFrameTabPage(DbStore* dbstore, QWidget* parent);

public:
	virtual ~VideoFrameTabPage();

	template<typename VideoFrameType>
	static VideoFrameTabPage* create(const QString& fileExt, DbStore* dbstore, QWidget* parent);

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
	std::function<VFrame30::CVideoFrame*()> m_createVideoFrameFunc;	// same as in VideoFrameControlTabPage
	QTabWidget* m_tabWidget;
};


// Create MainTab!!!
//
template<typename VideoFrameType>
VideoFrameTabPage* VideoFrameTabPage::create(const QString& fileExt, DbStore* dbstore, QWidget* parent)
{
	static_assert(std::is_base_of<VFrame30::CVideoFrame, VideoFrameType>::value, "Base class must be VFrame30::CVideoFrame");
	assert(dbstore != nullptr);

	VideoFrameTabPage* p = new VideoFrameTabPage(dbstore, parent);

	// Create VideoFrame function, will be stored in two places, VideoFrameTabPage and VideoFrameControlTabPage
	//
	std::function<VFrame30::CVideoFrame*()> createFunc(
		[]() -> VFrame30::CVideoFrame*
		{
			return new VideoFrameType();
		});

	p->m_createVideoFrameFunc = createFunc;

	// Add control page
	//
	VideoFrameControlTabPage* controlTabPage = new VideoFrameControlTabPage(fileExt, dbstore, createFunc);
	p->m_tabWidget->addTab(controlTabPage, tr("Control"));

	return p;
}


//
//
// VideoFrameControlTabPage
//
//
class VideoFrameControlTabPage : public QWidget, public HasDbStore
{
	Q_OBJECT
public:
	VideoFrameControlTabPage(const QString& fileExt, DbStore* dbstore, std::function<VFrame30::CVideoFrame*()> createVideoFrameFunc);
	virtual ~VideoFrameControlTabPage();

public:
	VFrame30::CVideoFrame* createVideoFrame() const;

protected:
	void CreateActions();

signals:

protected slots:
	void addFile();
	void openFiles(std::vector<DbFileInfo> files);
	void viewFiles(std::vector<DbFileInfo> files);

	void refreshFiles();

	// Data
	//
private:
	std::function<VFrame30::CVideoFrame*()> m_createVideoFrameFunc;
	VideoFrameFileView* m_filesView;
};

//
//
// EditVideoFrameTabPage
//
//
class EditVideoFrameTabPage : public QWidget, public HasDbStore
{
	Q_OBJECT
private:
	EditVideoFrameTabPage();		// Deleted
public:
	EditVideoFrameTabPage(std::shared_ptr<VFrame30::CVideoFrame> videoFrame, const DbFileInfo& fileInfo, DbStore* dbstore);
	virtual ~EditVideoFrameTabPage();

protected:
	void CreateActions();
	void setPageTitle();

signals:
	void vcsFileStateChanged();

protected slots:
	void closeTab();

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
	EditVideoFrameWidget* m_videoFrameWidget;
};


#endif // EDITVIDEOFRAMETABPAGE_H
