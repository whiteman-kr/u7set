#pragma once

#include "MainTabPage.h"
#include <vector>
#include <memory>
#include "../include/DbStruct.h"

class DbController;

class FileTreeModelItem : public DbFileInfo
{
public:
	FileTreeModelItem();
	explicit FileTreeModelItem(const DbFileInfo& file);
	virtual ~FileTreeModelItem();

	FileTreeModelItem* parent();

	int childrenCount() const;

	FileTreeModelItem* child(int index) const;
	int childIndex(FileTreeModelItem* child) const;

	FileTreeModelItem* childByFileId(int fileId) const;

	std::shared_ptr<FileTreeModelItem> childSharedPtr(int index);

	void addChild(std::shared_ptr<FileTreeModelItem> child);
	void deleteChild(FileTreeModelItem* child);
	void deleteAllChildren();

	void sortChildrenByFileName();

private:
	FileTreeModelItem* m_parent = nullptr;
	std::vector<std::shared_ptr<FileTreeModelItem>> m_children;
};


class FileTreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	FileTreeModel() = delete;
	FileTreeModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent);
	virtual ~FileTreeModel();

	QModelIndex index(int row, const QModelIndex& parentIndex) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual bool canFetchMore(const QModelIndex& parent) const override;
	virtual void fetchMore(const QModelIndex& parent) override;

	// Extensions
	//
public:
	FileTreeModelItem* fileItem(QModelIndex& index);
	const FileTreeModelItem* fileItem(const QModelIndex& index) const;

	std::shared_ptr<FileTreeModelItem> fileItemSharedPtr(QModelIndex& index);

	void addFile(QModelIndex& parentIndex, std::shared_ptr<FileTreeModelItem>& file);
	void removeFile(QModelIndex index);
	void updateFile(QModelIndex index, const DbFileInfo& file);

	void refresh();

public slots:
	void projectOpened();
	void projectClosed();

	// Properties
public:
	DbController* db();
	DbController* db() const;

	// Data
	//
private:
	DbController* m_dbc;
	QWidget* m_parentWidget;

	std::shared_ptr<FileTreeModelItem> m_root;

	enum Columns
	{
		FileNameColumn,
		FileSizeColumn,
		FileStateColumn,
		FileUserColumn,
		FileIdColumn,
		FileDetailsColumn,

		// Add other column befor this line
		//
		ColumnCount
	};
};

class FileTreeView : public QTreeView
{
	Q_OBJECT
public:
	FileTreeView() = delete;
	explicit FileTreeView(DbController* dbc);
	virtual ~FileTreeView();

	// public slots
	//
public slots:
	void addFile();
    void editFile();
	void deleteFile();
	void checkOutFile();
	void checkInFile();
	void undoChangesFile();
	void getLatestVersion();
	void getLatestTreeVersion();
	void setWorkcopy();
	void refreshFileTree();

private:
	bool getLatestFileVersionRecursive(const DbFileInfo& f, const QString &dir);


	// Protected props
	//
protected:
	FileTreeModel* fileTreeModel();
	FileTreeModel* fileTreeModel() const;
	DbController* db();

	// Data
	//
private:
	DbController* m_dbc;
};

class FilesTabPage : public MainTabPage
{
	Q_OBJECT
public:
	FilesTabPage(DbController* dbcontroller, QWidget* parent);

protected:
	void createActions();
	void setActionState();

public slots:
	void projectOpened();
	void projectClosed();

	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());

private slots:

	// Data
	//
private:
	FileTreeView* m_fileView = nullptr;
	FileTreeModel* m_fileModel = nullptr;

    QStringList m_editableExtensions;

	//
	QAction* m_addFileAction = nullptr;
    QAction* m_editFileAction = nullptr;
    QAction* m_deleteFileAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction1 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction2 = nullptr;
	QAction* m_getLatestVersionAction = nullptr;
	QAction* m_getLatestTreeVersionAction = nullptr;
	QAction* m_setWorkcopyAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction3 = nullptr;
	QAction* m_refreshAction = nullptr;
};


