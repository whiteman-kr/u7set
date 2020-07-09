#ifndef FILESTREEMODEL_H
#define FILESTREEMODEL_H

#include "../lib/DbStruct.h"

class DbController;
class FileTreeView;

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

private:
	FileTreeModelItem* m_parent = nullptr;
	std::vector<std::shared_ptr<FileTreeModelItem>> m_children;
};

class FileTreeProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	FileTreeProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

};

class FileTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	FileTreeModel() = delete;
	FileTreeModel(DbController* dbcontroller, QString rootFilePath, QWidget* parentWidget, QObject* parent);
	virtual ~FileTreeModel();

	enum class Columns
	{
		FileNameColumn,
		FileSizeColumn,
		FileStateColumn,
		FileUserColumn,
		FileIdColumn,
		FileAttributesColumn,

		// Add other column befor this line
		//
		StandardColumnCount,
		CustomColumnIndex
	};

	void setColumns(std::vector<Columns> columns);
	Columns columnAtIndex(int index) const;

	QModelIndex childIndex(int row, int column, const QModelIndex& parentIndex) const;

protected:
	virtual QString customColumnText(Columns column, const FileTreeModelItem* item) const;
	virtual QString customColumnName(Columns column) const;
	virtual QVariant columnIcon(const QModelIndex& index, FileTreeModelItem* file) const;

private:

	QModelIndex index(int row, const QModelIndex& parentIndex) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

private:
	virtual bool canFetchMore(const QModelIndex& parent) const override;
public:
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
	DbController* m_dbc = nullptr;
	QWidget* m_parentWidget = nullptr;
	QString m_rootFilePath;
	int m_rootFileId = -1;

	std::shared_ptr<FileTreeModelItem> m_root;

	std::vector<Columns> m_columns;
};

class FileTreeView : public QTreeView
{
	Q_OBJECT
public:
	FileTreeView() = delete;
	explicit FileTreeView(DbController* dbc);
	virtual ~FileTreeView();

	QModelIndexList selectedSourceRows() const;	// Returns selected rows mapped to source model

	bool addNewFile(const QString& fileName);

	// public slots
	//
public slots:
	void addFile();
	void addFolder(const QString& folderName);
	void viewFile();
	void editFile();
	void renameFile();
	void deleteFile();
	void checkOutFile();
	void checkInFile();
	void undoChangesFile();
	void showHistory();
	void showCompare();
	void getLatestVersion();
	void getLatestTreeVersion();
	void setWorkcopy();
	void refreshFileTree();

private:
	bool createFiles(std::vector<std::shared_ptr<DbFile> > files);
	bool getLatestFileVersionRecursive(const DbFileInfo& f, const QString &dir);
	void runFileEditor(bool viewOnly);

	// Protected props
	//
protected:
	FileTreeModel* fileTreeModel();
	FileTreeModel* fileTreeModel() const;

	FileTreeProxyModel* fileTreeProxyModel();
	FileTreeProxyModel* fileTreeProxyModel() const;

	DbController* db();

	// Data
	//
private:
	DbController* m_dbc;
};

#endif // FILESTREEMODEL_H
