#pragma once
#include <variant>
#include <QDialog>
#include <QTreeView>
#include <QPlainTextEdit>
#include <QPushButton>
#include <../../lib/DbController.h>
#include <../../lib/AppSignal.h>

#ifdef _DEBUG
	#include <QAbstractItemModelTester>
#endif


using PendingChangesObject = std::variant<DbFileInfo, Signal>;


class PendingChangesModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	PendingChangesModel();

public:
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
	void setData(std::vector<PendingChangesObject>&& objects, const std::vector<DbUser>& users);
	void resetData();

	std::vector<PendingChangesObject> objectsByModelIndex(QModelIndexList& indexes);

	// Data
	//
private:
	enum class Columns
	{
		Index,				//
		Type,				// F or S
		Id,					// Table ID from tables File or Signals
		NameOrAppSignalId,	// FileName for files or AppSignalID for Signals
		Caption,			// For files - caption from details, for Signals - Caption from signal
		State,
		Action,
		User,

		// Add other column befor this line
		//
		Count
	};

	std::vector<PendingChangesObject> m_objects;
	std::map<int, DbUser> m_users;
};


class PendingChangesDialog : public QDialog, protected HasDbController
{
	Q_OBJECT

private:
	explicit PendingChangesDialog(DbController* db, QWidget* parent);
	virtual ~PendingChangesDialog();

protected slots:
	void checkIn();
	void undoChanges();
	void updateData();

	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

public:
	static void show(DbController* db, QWidget* parent);

private:
	inline static PendingChangesDialog* m_instance = nullptr;

	QTreeView* m_treeView = nullptr;
	PendingChangesModel m_model;

	QPlainTextEdit* m_commentEdit = nullptr;
	QSplitter* m_splitter = nullptr;
	QPushButton* m_checkInButton = nullptr;
	QPushButton* m_undoButton = nullptr;
	QPushButton* m_refershButton = nullptr;
};

