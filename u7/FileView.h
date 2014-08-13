#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QTableView>
#include "../include/DbStruct.h"

class DbStore;

class FilesModel : public QAbstractTableModel
{
public:
	FilesModel(QObject* parent = nullptr);
	virtual ~FilesModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

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
		FileLastCheckInColumn,

		// Add other column befor this line
		//
		ColumnCount
	};
};


class FileView : public QTableView
{
	Q_OBJECT
private:
	FileView();
	FileView(const FileView&);

public:
	FileView(DbStore* dbstore);
	virtual ~FileView();

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
	virtual void getWorkcopy(std::vector<DbFileInfo> files);
	virtual void setWorkcopy(std::vector<DbFileInfo> files);
	virtual void refreshFiles();

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
	void slot_GetWorkcopy();
	void slot_SetWorkcopy();
	void slot_RefreshFiles();

public slots:
	void filesViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	// Public properties
	//
public:
	FilesModel& filesModel();

	// Protected properties
	//
protected:
	DbStore* dbStore();

	// Data
	//
private:
	DbStore* m_pDbStore;
	FilesModel m_filesModel;

	//	Contexet Menu
	//
protected:
	QAction* m_openFileAction;
	QAction* m_viewFileAction;
	// --
	QAction* m_separatorAction0;
	QAction* m_checkOutAction;
	QAction* m_checkInAction;
	QAction* m_undoChangesAction;
	// --
	QAction* m_separatorAction1;
	QAction* m_addFileAction;
	// --
	QAction* m_separatorAction2;
	QAction* m_getWorkcopyAction;
	QAction* m_setWorkcopyAction;
	// --
	QAction* m_separatorAction3;
	QAction* m_refreshFileAction;
	// End of ConextMenu
};

#endif // FILEVIEW_H
