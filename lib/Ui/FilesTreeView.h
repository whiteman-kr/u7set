#ifndef FILESTREEMODEL_H
#define FILESTREEMODEL_H

#include "../DbLib/DbStruct.h"

class DbController;
class FileTreeView;
class FileTreeModel;

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

	void expandChildFilesToArray(std::vector<FileTreeModelItem*>& result);

	void addChild(std::shared_ptr<FileTreeModelItem> child);
	void deleteChild(FileTreeModelItem* child);
	void deleteAllChildren();

	bool fetched() const;
	void setFetched();

private:
	FileTreeModelItem* m_parent = nullptr;
	std::vector<std::shared_ptr<FileTreeModelItem>> m_children;

	bool m_fetched = false;
};

class FileTreeProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	FileTreeProxyModel(FileTreeModel* sourceModel, QObject *parent = 0);

	QModelIndexList getPersistentIndexList() const;

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
	FileTreeModel* m_sourceModel = nullptr;

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

	int childCount(const QModelIndex& parentIndex) const;
	QModelIndex childIndex(int row, int column, const QModelIndex& parentIndex) const;

	QModelIndex childParent(const QModelIndex& childIndex) const;

protected:
	virtual QString customColumnText(Columns column, const FileTreeModelItem* item) const;
	virtual QString customColumnName(Columns column) const;
	virtual QVariant columnIcon(const QModelIndex& index, FileTreeModelItem* file) const;

private:
	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;
	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

private:
	virtual bool canFetchMore(const QModelIndex& parentIndex) const override;
	virtual void fetchMore(const QModelIndex& parent) override;

public:
	void fetch(const QModelIndex& parent);
	void fetchRecursively(QModelIndex parentIndex, const std::vector<int>& fileIdSet);

	// Extensions
	//
public:
	FileTreeModelItem* fileItem(QModelIndex& index);
	const FileTreeModelItem* fileItem(const QModelIndex& index) const;

	std::shared_ptr<FileTreeModelItem> fileItemSharedPtr(QModelIndex& index);

	void addFile(QModelIndex& parentIndex, std::shared_ptr<FileTreeModelItem>& file);
	void removeFile(QModelIndex index);
	void updateFile(QModelIndex index, const DbFileInfo& file);
	bool moveFiles(const QModelIndexList& selectedIndexes, int movedToParnetId, const std::vector<DbFileInfo>& movedFiles, std::vector<QModelIndex>* addedFilesIndexes);

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

	bool m_addFileInProgress = false;

	std::shared_ptr<FileTreeModelItem> m_root;

	std::vector<Columns> m_columns;
};

class FileTreeView : public QTreeView
{
	Q_OBJECT
public:
	FileTreeView() = delete;
	explicit FileTreeView(DbController* dbc, FileTreeModel* model);
	virtual ~FileTreeView();

	QString defaultExtension() const;
	void setDefaultExtension(const QString& extension);	// Sets default file extension (with point!), for example, ".js"

	QModelIndexList selectedSourceRows() const;	// Returns selected rows mapped to source model

	bool newFile(const QString& fileName, const QByteArray& data);
	void moveFile(int parentFileId);
	void checkOutFilesById(std::vector<int> fileIds);
	void checkInFilesById(std::vector<int> fileIds, std::vector<int>* deletedFileIds);
	bool undoChangesFilesById(std::vector<int> fileIds, std::vector<int>* deletedFileIds);

	void setFileNameFilter(const QString& filterText);

	// public slots
	//
public slots:
	void addFile();
	void addFileToFolder();
	void addFolder(const QString& folderName);
	void viewFile();
	void editFile();
	void renameFile();
	void deleteFile();
	void checkOutSelectedFiles();
	void checkInSelectedFiles();
	bool undoChangesSelectedFiles();
	void showHistory();
	void showCompare();
	void getLatestVersion();
	void getLatestTreeVersion();
	void setWorkcopy();
	void refreshFileTree();

private:
	bool expandAndSelect(const QModelIndex& mi, std::vector<int> expandedFileIds, std::vector<int> selectedFilesIds);

	bool createFiles(std::vector<std::shared_ptr<DbFile> > files, bool createInParentFolder);
	void checkOutFiles(QModelIndexList indexList);
	void checkInFiles(QModelIndexList indexList);
	bool undoChangesFiles(QModelIndexList indexList);
	bool getLatestFileVersionRecursive(const DbFileInfo& f, const QString &dir);
	void runFileEditor(bool viewOnly);

	// Protected props
	//
protected:
	DbController* db();

	// Data
	//
private:
	DbController* m_dbc = nullptr;

	FileTreeModel* m_model = nullptr;
	FileTreeProxyModel* m_proxyModel = nullptr;

	QString m_defaultExtension;
};

#endif // FILESTREEMODEL_H
