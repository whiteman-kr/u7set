#pragma once

#include "../../AppSignalLib/DiscretesLogRecord.h"
#include <SchemaClientLib/DragDropHelper.h>
#include <CommonLib/Hash.h>

namespace AppSignalLists
{
	class AppSignalListSet;
}

namespace ClientLib
{
	class SignalLog;
	class AppSignalManager;
} // namespace ClientLib

class IAppSignalManager;
class SignalLogModel;

enum class SignalLogColumns
{
	CustomAppSignalID = 0, // Signal Param Columns
	EquipmentID,
	LmEquipmentID,
	AppSignalID,
	Caption,
	Type,
	Tags,

	RecordTime,
	SystemTime, // Signal State Columns
	LocalTime,
	PlantTime,

	Value,
	Units,

	Valid,
	StateAvailable,
	Simulated,
	Blocked,
	Mismatch,
	OutOfLimits,

	Acknowledged,
	AckTime,
	AckSource,
	AckUser,

	ColumnCount
};

enum class SignalLogMaskType
{
	All = 0,
	AppSignalID,
	CustomAppSignalID,
	EquipmentID,
	LmEquipmentID,
	Count
};

struct RecordKey
{
	std::size_t operator()(const RecordKey& p) const { return ::calcHash(&p, sizeof(RecordKey)); }
	bool operator==(const RecordKey& p) const { return p.recordTime == recordTime && p.signalHash == signalHash; }

	RecordKey() :
		recordTime(0),
		signalHash(0)
	{
	}

	RecordKey(const DiscretesLogRecord& rec) :
		recordTime(rec.recordTime),
		signalHash(rec.signalHash)
	{
	}

	qint64 recordTime;
	Hash signalHash;
};

//
// SignalLogModel
//
class SignalLogModel : public QAbstractTableModel
{
public:
	SignalLogModel(const ClientLib::SignalLog& signalLog,
				   const IAppSignalManager* appSignalManager,
				   const AppSignalLists::AppSignalListSet* appSignalListSet,
				   QObject* parent);

public:
	// Properties

	QStringList columnsNames() const;

	qint64 updateCounter() const;

	qint64 maxInitialRecordTime() const;
	void resetMaxInitialRecordTime();

	// Overrides

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	// Operations

	void setMaskType(SignalLogMaskType type);
	void setMasks(const QStringList& masks);
	void setTags(const QStringList& tags);

	void setAppSignalList(const QString& listId);
	QString appSignalList() const;

	void setRecords(std::vector<DiscretesLogRecord>& records, qint64 updateCounter);	// Update the list when new records arrived or were removed
	void fillRecords(bool resetSelection);	// Refill the list when user changed filter settings

	int recordsCount() const;
	const DiscretesLogRecord& record(const RecordKey& key) const;
	const DiscretesLogRecord& filteredRecord(int index) const;

	void sort(int column, Qt::SortOrder order) override;

	AppSignalParam signalParam(int rowIndex, bool* found);

protected:
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	bool filterRecord(const DiscretesLogRecord& rec) const;

private:
	const ClientLib::SignalLog& m_signalLog;
	const IAppSignalManager* m_appSignalManager = nullptr;
	const AppSignalLists::AppSignalListSet* m_appSignalListSet = nullptr;

	QStringList m_columnsNames;
	
	// Model data

	std::unordered_map<RecordKey, DiscretesLogRecord, RecordKey> m_records;
	qint64 m_updateCounter = 0;

	bool m_initMaxInitialRecordTime = true;
	qint64 m_maxInitialRecordTime = -1; // This is the max record Time at the dialog showing. Further record Times are selected after adding

	std::vector<RecordKey> m_filteredRecords;

	// Filtering parameters

	SignalLogMaskType m_maskType = SignalLogMaskType::CustomAppSignalID;
	QStringList m_masks;
	QStringList m_tags;
	
	QString m_appSignallistID;
	std::set<Hash> m_appSignalListHashes;

};


struct SignalLogDialogSettings
{
	QByteArray horzHeader;
	int horzHeaderCount = 0; // Stores SignalLogColumns::ColumnCount constant to restore default settings if columns set changes

	QStringList maskList;
	QStringList tagsList;

	int sortColumn = 0;
	Qt::SortOrder sortOrder = Qt::AscendingOrder;

	void restore();
	void store();
};

//
// SignalLogTableView
//
class SignalLogTableView : public QTableView
{
protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	AppSignalParam m_appSignalParam;
	QPoint m_dragStartPosition;

	SchemaClientLib::DragDropHelper m_dragDropHelper;
};

//
// SignalLogWidget
//
class SignalLogWidget : public QWidget
{
	Q_OBJECT

public:
	SignalLogWidget(const ClientLib::SignalLog& signalLog,
					const IAppSignalManager* appSignalManager,
					const AppSignalLists::AppSignalListSet* appSignalListSet,
					const QString& projectName,
					const QString& equipmentId,
					const QString& signalLogTagCritical,
					const QString& signalLogTagWarning,
					QWidget* parent);

	virtual ~SignalLogWidget();

public:
	QString projectName() const;
	void setProjectName(const QString& projectName);

protected:
	void showEvent(QShowEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void timerEvent(QTimerEvent* event) override;

private slots:
	void headerColumnContextMenuRequested(const QPoint& pos);
	void headerColumnToggled(bool checked);

	void contextMenuRequested(const QPoint& pos);
	void tableViewDoubleClicked(const QModelIndex& index);
	void sortIndicatorChanged(int column, Qt::SortOrder order);
	void editMaskReturnPressed();
	void editTagsReturnPressed();
	void maskTypeComboCurrentIndexChanged(int index);
	void signalListComboIndexChanged(int index);
	void buttonExportClicked();
	void buttonPrintClicked();
	void buttonChooseTagsClicked();
	void buttonClearFilterClicked();

private:
	void createControls();
	void initFiltersView();
	void initRecordsView();

	void fillAppSignalLists();

	void updateRecords();
	void updateTableItems();

	void maskChanged(bool addToCompleter);
	void tagsChanged();

signals:
	void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

private:
	const ClientLib::SignalLog& m_signalLog;
	const IAppSignalManager* m_appSignalManager = nullptr;
	const AppSignalLists::AppSignalListSet* m_appSignalListSet = nullptr;
	SignalLogModel m_model;

	// Ui
	QComboBox* m_maskTypeCombo = nullptr;
	QComboBox* m_signalListCombo = nullptr;

	QLineEdit* m_editMask = nullptr;
	QLineEdit* m_editTags = nullptr;
	QToolButton* m_buttonChooseTags = nullptr;

	QPushButton* m_buttonFixate = nullptr;

	SignalLogTableView* m_tableView = nullptr;

	QCompleter* m_maskCompleter = nullptr;
	QCompleter* m_tagsCompleter = nullptr;

	QPushButton* m_clearFilterButton = nullptr;

	QMenu m_signalMenu;

	// Project Data
	QString m_projectName;
	QString m_equipmentId;

	int m_updateStateTimerId = -1;

	bool m_firstShow = true;

	QString m_maskHelp;
	QString m_tagsHelp;

	SignalLogDialogSettings m_settings;

	// Color tags

	QString m_signalLogTagCritical;
	QString m_signalLogTagWarning;
};

class SignalLogDialog : public QDialog
{
	Q_OBJECT

private:
	explicit SignalLogDialog(const ClientLib::SignalLog& signalLog,
							 ClientLib::AppSignalManager& appSignalManager,
							 const AppSignalLists::AppSignalListSet* appSignalListSet,
							 const QString& projectName,
							 const QString& equipmentId,
							 const QString& signalLogTagCritical,
							 const QString& signalLogTagWarning,
							 QWidget* parent = nullptr);

public:
	virtual ~SignalLogDialog();

	static SignalLogDialog* createDialog(const ClientLib::SignalLog& signalLog,
										 ClientLib::AppSignalManager& appSignalManager,
										 const AppSignalLists::AppSignalListSet* appSignalListSet,
										 const QString& projectName,
										 const QString& equipmentId,
										 const QString& signalLogTagCritical,
										 const QString& signalLogTagWarning,
										 QWidget* parent);

protected:
	void showEvent(QShowEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private slots:
	void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

private:
	const ClientLib::SignalLog& m_signalLog;
	ClientLib::AppSignalManager& m_appSignalManager;

	SignalLogWidget* m_logWidget = nullptr;
	static SignalLogDialog* s_instance;

	const QString& m_signalLogTagCritical;
	const QString& m_signalLogTagWarning;
};