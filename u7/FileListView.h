#pragma once

#include <QTableView>
#include "../lib/DbStruct.h"

class DbController;

class FileListModel : public QAbstractTableModel
{
public:
	FileListModel(QObject* parent = nullptr);
	virtual ~FileListModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	void addFile(std::shared_ptr<DbFileInfo> file);

	void setFiles(const std::vector<DbFileInfo>& files);
	void clear();

	std::shared_ptr<DbFileInfo> fileByRow(int row);
	std::shared_ptr<DbFileInfo> fileByFileId(int fileId);

	int getFileRow(int fileId) const;

	// Properties
	//
public:
	QString filter() const;
	void setFilter(const QString& value);		// "" -- no filter, "cdd" -- just cdd files

	// Data
	//
private:
	std::vector<std::shared_ptr<DbFileInfo>> m_files;
	QString m_filter;

	enum Columns
	{
		FileNameColumn,
		FileSizeColumn,
		FileStateColumn,
		FileUserColumn,
		FileActionColumn,
		FileLastCheckInColumn,
		FileIdColumn,
		FileDetailsColumn,

		// Add other column befor this line
		//
		ColumnCount
	};
};


class FileListView : public QTableView
{
	Q_OBJECT
private:
	FileListView();
	FileListView(const FileListView&);

public:
	FileListView(DbController* dbstore, const QString& parentFileName);
	virtual ~FileListView();

protected:
	void CreateActions();

	// Methods
	//
public:
	void setFiles(const std::vector<DbFileInfo>& files);
	void clear();

	void getSelectedFiles(std::vector<DbFileInfo>* out);

	virtual void openFile(std::vector<DbFileInfo> files);
	virtual void viewFile(std::vector<DbFileInfo> files);
	virtual void checkOut(std::vector<DbFileInfo> files);
	virtual void checkIn(std::vector<DbFileInfo> files);
	virtual void undoChanges(std::vector<DbFileInfo> files);
	virtual void addFile();
	virtual void deleteFile(std::vector<DbFileInfo> files);
	virtual void getWorkcopy(std::vector<DbFileInfo> files);
	virtual void setWorkcopy(std::vector<DbFileInfo> files);
	virtual void refreshFiles();
	virtual void fileDoubleClicked(DbFileInfo file);

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
	void slot_AddFile();
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
	FileListModel& filesModel();

	const DbFileInfo& parentFile() const;
	int parentFileId() const;

	// Protected properties
	//
protected:
	DbController* dbController();

	// Data
	//
private:
	DbController* m_dbController;
	FileListModel m_filesModel;

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
	// --
	QAction* m_separatorAction1 = nullptr;
	QAction* m_addFileAction = nullptr;
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

