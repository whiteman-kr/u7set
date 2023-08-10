#pragma once

#include "../VFrame30/Schema.h"
#include "GlobalMessanger.h"

class EditSchemaTabPage;
class TagSelectorWidget;
class AppSignalSetProvider;


//
//
// SchemaListModel
//
//
class SchemaListModel : public QAbstractItemModel, protected HasDbController
{
	Q_OBJECT

public:
	SchemaListModel(DbController* dbc, QWidget* parentWidget);

public:
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
	std::pair<QModelIndex, bool> addFile(QModelIndex parentIndex, std::shared_ptr<DbFileInfo> file);
	bool deleteFilesUpdate(const QModelIndexList& selectedIndexes, const std::vector<std::shared_ptr<DbFileInfo>>& files);
	bool moveFilesUpdate(const QModelIndexList& selectedIndexes, int movedToParnetId, const std::vector<DbFileInfo>& movedFiles, std::vector<QModelIndex>* addedFilesIndexes);

	bool updateFiles(const QModelIndexList& selectedIndexes, const std::vector<DbFileInfo>& files);

	DbFileInfo file(int fileId) const;
	DbFileInfo file(const QModelIndex& modelIndex) const;
	std::shared_ptr<DbFileInfo> fileSharedPtr(const QModelIndex& modelIndex) const;

	bool isFolder(const QModelIndex& modelIndex) const;

	QModelIndexList searchFor(const QString searchText);
	void setFilter(QString filter);
	void setTagFilter(const QStringList& tags);
	const QStringList& tagFilter() const;

protected:
private:
	void applyFilter(DbFileTree* filesTree, const std::map<int, VFrame30::SchemaDetails>& detailsMap);
	void applyTagFilter(DbFileTree* filesTree, const std::map<int, VFrame30::SchemaDetails>& detailsMap);

	bool isSystemFile(int fileId) const;

	void updateTagsFromDetails();

signals:
	void tagsChanged();

public slots:
	void refresh();

private slots:
	void projectOpened(DbProject project);
	void projectClosed();

	// Properties
	//
public:
	QString usernameById(int userId) const noexcept;
	QString tagsColumnText(int fileId) const;
	QString detailsColumnText(int fileId) const;
	QString fileCaption(int fileId) const;
	bool excludedFromBuild(int fileId) const;

	const DbFileInfo& parentFile() const;

	int schemaFilterCount() const;

	const std::set<QString>& tags() const;

	// Data
	//
public:
	enum class Columns
	{
		FileNameColumn,
		CaptionColumn,
		FileStateColumn,
		FileActionColumn,
		ChangesetColumn,
		FileUserColumn,
		IssuesColumn,
		TagsColumn,
		DetailsColumn,

		// Add other column befor this line
		//
		ColumnCount
	};

	static const int SearchSchemaRole = Qt::UserRole + 1;
	static const int ExcludedSchemaRole = Qt::UserRole + 2;

private:
	DbFileInfo m_parentFile;

	DbFileTree m_files;
	mutable QString m_searchText;		// Set in match(), used in data for SearchSchemaRole()
	mutable QString m_filterText;

	std::map<int, QString> m_users;							// Key is UserID
	std::map<int, VFrame30::SchemaDetails> m_details; 		// Key is FileID
	std::set<QString> m_tags;
	QStringList m_tagFilter;						// If vector is empty, then all schemas must be shown

	std::set<int> m_systemFiles;	// Key is fileid

	int m_schemaFilterCount = 0;
};


class SchemaProxyListModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	SchemaProxyListModel(QObject* parent = nullptr);
	virtual ~SchemaProxyListModel();

	virtual void setSourceModel(QAbstractItemModel* sourceModel) override;

protected:
	virtual bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override;

public:
	DbFileInfo file(const QModelIndex& mi) const;
	std::vector<int> expandedFileIds(QTreeView* treeView);

private:
	SchemaListModel* m_sourceModel = nullptr;
};

//
//
// SchemaFileView
//
//
class SchemaFileView : public QTreeView, public HasDbController
{
	Q_OBJECT

public:
	SchemaFileView(DbController* dbc, QWidget* parent);
	virtual ~SchemaFileView();

	// Methods
	//
protected:
	void createActions();
	void createContextMenu();

	virtual void timerEvent(QTimerEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;

	// Methods
	//
public:
	std::vector<std::shared_ptr<DbFileInfo>> selectedFiles() const;

	void refreshFiles();

	void searchAndSelect(QString searchText);

	void setFilter(QString filter);
	void setTagFilter(const QStringList& tags);

signals:
	void showFileProperties(DbFileInfo files);
	void openFileSignal(DbFileInfo files);
	void viewFileSignal(DbFileInfo files);

	// Protected slots
	//
public slots:
	void projectOpened();
	void projectClosed();

	void slot_refreshFiles();
	void slot_doubleClicked(const QModelIndex& index);

public slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

	// Public properties
	//
public:
	SchemaListModel& filesModel();
	SchemaProxyListModel& proxyModel();

	//	const std::vector<std::shared_ptr<DbFileInfo>>& files() const;

	const DbFileInfo& parentFile() const;
	int parentFileId() const;

	// Protected properties
	//
protected:

	// Data
	//
private:
	SchemaListModel m_filesModel;
	SchemaProxyListModel m_proxyModel;

	int m_lastBuildIssueCount = -1;

	// Actions, public becaus they are used in control page toolbar
	//
public:
	// --
	QAction* m_newFileAction = nullptr;
	QAction* m_newFolderAction = nullptr;
	QAction* m_openAction = nullptr;
	QAction* m_viewAction = nullptr;
	QAction* m_cloneFileAction = nullptr;
	QAction* m_deleteAction = nullptr;
	QAction* m_moveFileAction = nullptr;

	// --
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_recursiveHistoryAction = nullptr;
	QAction* m_compareAction = nullptr;

	// --
	QAction* m_exportWorkingcopyAction = nullptr;
	QAction* m_importWorkingcopyAction = nullptr;

	QAction* m_exportToPdfAction = nullptr;
	QAction* m_exportToAlbumAction = nullptr;

	// --
	QAction* m_refreshFileAction = nullptr;
	QAction* m_behaviorAction = nullptr;
	QAction* m_propertiesAction = nullptr;

	// --
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
	SchemaControlTabPage(DbController* db, AppSignalSetProvider* signalSetProvider);
	virtual ~SchemaControlTabPage();

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void hideEvent(QHideEvent* event) override;

public:
	bool hasUnsavedSchemas() const;
	bool saveUnsavedSchemas();
	bool resetModified();

	void refresh();

private:
	void createToolBar();

	std::shared_ptr<VFrame30::Schema> createSchema(const DbFileInfo& parentFile) const;

	EditSchemaTabPage* findOpenedFile(const DbFileInfo& file, bool readOnly);

public slots:
	void removeFromOpenedList(EditSchemaTabPage* editTabPage);
	void detachOrAttachWindow(EditSchemaTabPage* editTabPage);

	void openFile(const DbFileInfo& file);
	void viewFile(const DbFileInfo& file);
	void viewFile(const DbFileInfo& file, int changesetId);


protected slots:
	void projectOpened();
	void projectClosed();

	int showSelectFolderDialog(int parentFileId, int currentSelectionFileId, bool showRootFile);

	void openSelectedFile();
	void viewSelectedFile();

	void addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile);
	void addFile();

	void addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, int parentFileId);
	void addSchemaFileToDb(std::shared_ptr<VFrame30::Schema> schema, QString fileExtension, QModelIndex parentIndex);

	void addFolder();

	void cloneFile();
	void deleteFiles();
	void moveFiles();

	void checkOutFiles();
	void checkInFiles();
	void undoChangesFiles();

	void showFileHistory();
	void showFileHistoryRecursive();
	void compareSelectedFile();
	void compareObject(DbChangesetObject object, CompareData compareData);

	void exportWorkcopy();
	void importWorkcopy();

	void exportToPdf();
	void exportToAlbum();

	void showFileProperties();
	void showBehaviorEditor();

private slots:
	void ctrlF();
	void search();
	void searchSchemaForLm(QString equipmentId);

	void filter();
	void resetFilter();

	void schemaTagsChanged();
	void tagSelectorHasChanges();

	// Properties
	//
public:
	[[nodiscard]] const DbFileInfo& parentFile() const;

	// Data
	//
private:
	SchemaFileView* m_filesView = nullptr;
	QToolBar* m_toolBar = nullptr;

	QAction* m_searchAction = nullptr;
	QLineEdit* m_searchEdit = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_searchCompleter = nullptr;
	QPushButton* m_searchButton = nullptr;
	QPushButton* m_filterButton = nullptr;
	QPushButton* m_resetFilterButton = nullptr;

	TagSelectorWidget* m_tagSelector = nullptr;

	std::list<EditSchemaTabPage*> m_openedFiles;		// Opened files (for edit and view)

	AppSignalSetProvider* m_signalSetProvider = nullptr;

	int m_lastSelectedNewSchemaForLmFileId = -1;
};

