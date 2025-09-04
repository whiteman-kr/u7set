#pragma once

#include "../../AppSignalLib/DiscretesLogRecord.h"
#include <SchemaClientLib/DragDropHelper.h>

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
	SignalID = 0, // Signal Param Columns
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

// Q_DECLARE_METATYPE(SchemaClientLib::SignalLogColumns);

enum class SignalLogMaskType
{
	All = 0,
	AppSignalId,
	CustomAppSignalId,
	EquipmentId,
	LmEquipmentId,
	Count
};

//
// SignalLogSorter
//
class SignalLogSorter
{
public:
	SignalLogSorter(int column, const SignalLogModel* model, const IAppSignalManager* appSignalManager);

	bool operator()(int index1, int index2) const { return sortFunction(index1, index2); }

	bool sortFunction(int index1, int index2) const;

private:
	int m_column = -1;

	const SignalLogModel* m_model = nullptr;
	const IAppSignalManager* m_appSignalManager = nullptr;
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
				   const QString& signalLogTagCritical,
				   const QString& signalLogTagWarning,
				   QObject* parent);

public:
	// Properties

	QStringList columnsNames() const;

	qint64 updateCounter() const;

	qint64 maxInitialRecordID() const;
	void resetMaxInitialRecordID();

	// Overrides

	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	// Operations

	void setMaskType(SignalLogMaskType type);
	void setMasks(const QStringList& masks);

	void setTags(const QStringList& tags);

	void setAppSignalList(const QString& listId);
	QString appSignalList() const;

	void setRecords(std::vector<DiscretesLogRecord>&& records, qint64 updateCounter);
	void fillRecords(bool resetSelection);

	int recordsCount() const;
	const DiscretesLogRecord& record(int index) const;
	const DiscretesLogRecord& filteredRecord(int index) const;

	void sort(int column, Qt::SortOrder order) override;

	AppSignalParam signalParam(int rowIndex, bool* found);

	E::AnalogFormat analogFormat() const;
	void setAnalogFormat(E::AnalogFormat format);

	int analogPrecision() const;
	void setAnalogPrecision(int precision);

protected:
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	const ClientLib::SignalLog& m_signalLog;
	const IAppSignalManager* m_appSignalManager = nullptr;
	const AppSignalLists::AppSignalListSet* m_appSignalListSet = nullptr;

	QStringList m_columnsNames;

	// Model data

	std::vector<DiscretesLogRecord> m_records;
	qint64 m_updateCounter = 0;

	bool m_initMaxInitialRecordID = true;
	qint64 m_maxInitialRecordID = -1; // This is the max record ID at the dialog showing. Further record IDs are selected after adding

	std::vector<size_t> m_filteredRecords;

	QString m_signalLogTagCritical;
	QString m_signalLogTagWarning;

	// Filtering parameters

	SignalLogMaskType m_maskType = SignalLogMaskType::CustomAppSignalId;
	QStringList m_masks;
	QStringList m_tags;
	QString m_listId;

	// View params

	E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e;
	int m_analogPrecision = -1;
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
	void createMenus();
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

	QAction* m_formatAutoSelect = nullptr;
	QAction* m_formatDecimal = nullptr;
	QAction* m_formatExponential = nullptr;

	QAction* m_precisionDefault = nullptr;
	QList<QAction*> m_precisionActions;

	QCompleter* m_maskCompleter = nullptr;
	QCompleter* m_tagsCompleter = nullptr;

	QPushButton* m_clearFilterButton = nullptr;

	QMenu m_formatMenu;

	// Project Data
	QString m_projectName;
	QString m_equipmentId;

	int m_updateStateTimerId = -1;

	bool m_firstShow = true;

	QString m_maskHelp;
	QString m_tagsHelp;

	SignalLogDialogSettings m_settings;
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