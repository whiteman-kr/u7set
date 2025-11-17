#pragma once

#include "../../AppSignalLib/DiscretesLogRecord.h"


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
class SignalLogTableView;

enum class SignalLogColumns
{
	CustomAppSignalID = 0, // Signal Param Columns
	EquipmentID,
	LmEquipmentID,
	AppSignalID,
	Caption,
	Tags,

	RecordTime,
	SystemTime, // Signal State Columns
	LocalTime,
	PlantTime,

	Value,

	Flags,
	Valid,
	StateAvailable,
	Simulated,
	Blocked,
	Mismatch,

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
	bool operator==(const RecordKey& p) const
	{
		return p.recordTime == recordTime && p.signalHash == signalHash && p.plantTime == plantTime;
	}

	RecordKey() :
		recordTime(0),
		plantTime(0),
		signalHash(0)
	{
	}

	explicit RecordKey(const DiscretesLogRecord& rec) :
		recordTime(rec.recordTime),
		plantTime(rec.plantTime),
		signalHash(rec.signalHash)
	{
	}

	qint64 recordTime;
	qint64 plantTime;
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

	// Overrides

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	// Operations

	void setMaskType(SignalLogMaskType type);
	void setMasks(const QStringList& masks);
	void setTags(const QStringList& tags);

	void setAppSignalList(const QString& listId);
	QString appSignalList() const;

	void setRecords(std::vector<DiscretesLogRecord>& records,
					qint64 updateCounter); // Update the list when new records arrived or were removed
	void fillRecords(bool clearBeforeFilling);  // Fill the list according to records
	void removeUpTo(qint64 plantTime);     // Remove records up to plantTime

	int recordsCount() const;
	const DiscretesLogRecord& record(const RecordKey& key) const;
	const DiscretesLogRecord& filteredRecord(int index) const;
	const std::vector<RecordKey>& filteredRecords() const;

	std::optional<AppSignalParam> signalParam(int rowIndex);

public:
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

	std::vector<DiscretesLogRecord> m_recordsVec;
	std::unordered_map<RecordKey, DiscretesLogRecord, RecordKey> m_recordsMap;

	qint64 m_updateCounter = 0;

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

	void restore();
	void store();
};


//
// SignalLogWidget
//
class SignalLogWidget : public QWidget
{
	Q_OBJECT

public:
	SignalLogWidget(ClientLib::SignalLog& signalLog,
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
	void editMaskReturnPressed();
	void editTagsReturnPressed();
	void maskTypeComboCurrentIndexChanged(int index);
	void signalListComboIndexChanged(int index);
	void buttonExportClicked();
	void buttonPrintClicked();
	void buttonChooseTagsClicked();
	void buttonClearFilterClicked();
	void buttonAckAllClicked();
	void turnOffAutoscroll();
	void copySelected();

private:
	void createControls();
	void initFiltersView();
	void initRecordsView();

	void fillAppSignalLists();

	void updateRecords();
	void updateTableItems();

	void maskChanged(bool addToCompleter);
	void tagsChanged();

	bool filterIsSet() const;
	bool warnAboutAckFiltered();

	void printData(bool printSelected);

signals:
	void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);
	void updateStatus(int totalRecords, int filteredRecords);

private:
	ClientLib::SignalLog& m_signalLog;
	const IAppSignalManager* m_appSignalManager = nullptr;
	const AppSignalLists::AppSignalListSet* m_appSignalListSet = nullptr;
	SignalLogModel m_model;

	// Ui
	QComboBox* m_maskTypeCombo = nullptr;
	QComboBox* m_signalListCombo = nullptr;

	QLineEdit* m_editMask = nullptr;
	QLineEdit* m_editTags = nullptr;
	QToolButton* m_buttonChooseTags = nullptr;

	QPushButton* m_buttonPause = nullptr;
	QToolButton* m_buttonAutoScroll = nullptr;

	SignalLogTableView* m_tableView = nullptr;

	QCompleter* m_maskCompleter = nullptr;
	QCompleter* m_tagsCompleter = nullptr;

	QPushButton* m_clearFilterButton = nullptr;
	QPushButton* m_ackButton = nullptr;

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
	explicit SignalLogDialog(ClientLib::SignalLog& signalLog,
							 ClientLib::AppSignalManager& appSignalManager,
							 const AppSignalLists::AppSignalListSet* appSignalListSet,
							 const QString& projectName,
							 const QString& equipmentId,
							 const QString& signalLogTagCritical,
							 const QString& signalLogTagWarning,
							 QWidget* parent = nullptr);

public:
	virtual ~SignalLogDialog();

	static SignalLogDialog* createDialog(ClientLib::SignalLog& signalLog,
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
	void reject() override;

private slots:
	void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

private:
	ClientLib::SignalLog& m_signalLog;
	ClientLib::AppSignalManager& m_appSignalManager;

	SignalLogWidget* m_logWidget = nullptr;
	static SignalLogDialog* s_instance;

	QStatusBar* m_statusBar = nullptr;
	QLabel* m_labelTotal = nullptr;
	QLabel* m_labelFiltered = nullptr;

	const QString& m_signalLogTagCritical;
	const QString& m_signalLogTagWarning;
};