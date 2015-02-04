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
	FileTreeModelItem(const DbFileInfo& file);
	virtual ~FileTreeModelItem();

	FileTreeModelItem* parent();

	int childrenCount() const;

	FileTreeModelItem* child(int index) const;
	int childIndex(FileTreeModelItem* child) const;

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
	void CreateActions();

public slots:
	void projectOpened();
	void projectClosed();

private slots:

	// Data
	//
private:
	FileTreeView* m_fileView = nullptr;
	FileTreeModel* m_fileModel = nullptr;
};


