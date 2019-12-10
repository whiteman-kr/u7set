#ifndef DIALOGSIGNALSNAPSHOT_H
#define DIALOGSIGNALSNAPSHOT_H

#include "../lib/AppSignalManager.h"
#include "DialogColumns.h"
#include "MonitorConfigController.h"
#include "../lib/ExportPrint.h"


namespace Ui {
class DialogSignalSnapshot;
}

class MonitorConfigController;
class TcpSignalClient;
class SignalSnapshotModel;
struct ConfigSettings;

class SnapshotExportPrint : public ExportPrint
{
public:

	SnapshotExportPrint(ConfigSettings* configuration, QWidget* parent);

private:
	virtual void generateHeader(QTextCursor& cursor) override;

	ConfigSettings* m_configuration = nullptr;
};

class SignalSnapshotSorter
{
public:
	  SignalSnapshotSorter(int column, SignalSnapshotModel* model);

	  bool operator()(int index1, int index2) const
	  {
		  return sortFunction(index1, index2);
	  }

	  bool sortFunction(int index1, int index2) const;

private:
	  int m_column = -1;

	  SignalSnapshotModel* m_model = nullptr;
};


enum class SnapshotColumns
{
	SignalID = 0,		// Signal Param Columns
	EquipmentID,
	AppSignalID,
	Caption,
	Type,

	SystemTime,			// Signal State Columns
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

	ColumnCount
};

Q_DECLARE_METATYPE(SnapshotColumns);


class SignalSnapshotModel : public QAbstractItemModel
{
	Q_OBJECT

	friend class SignalSnapshotSorter;

public:

	enum class SignalType
	{
		All = 0,
		AnalogInput,
		AnalogOutput,
		DiscreteInput,
		DiscreteOutput
	};

	enum class MaskType
	{
        All = 0,
		AppSignalId,
		CustomAppSignalId,
		EquipmentId
	};

public:
	SignalSnapshotModel(QObject *parent);

	void setSignals(std::vector<AppSignalParam>& signalList);

public:
	// Properties

	QStringList columnsNames() const;

	// Overrides

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	// Operations

	void setSignalType(SignalType type);

	void setMasks(const QStringList& masks);

	void setSchemaAppSignals(std::set<QString> schemaAppSignals);

	void fillSignals();

	void updateStates(int from, int to);

	void sort(int column, Qt::SortOrder order) override;

	AppSignalParam signalParam(int rowIndex, bool* found);

	AppSignalState signalState(int rowIndex, bool* found);

protected:
	QModelIndex parent(const QModelIndex &index) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:

	QStringList m_columnsNames;

	// Model data

	std::vector<AppSignalParam> m_allSignals;

	std::vector<AppSignalState> m_allStates;

	std::vector<int> m_filteredSignals;

	// Filtering parameters

	SignalType m_signalType = SignalType::All;

	QStringList m_masks;

	std::set<QString> m_schemaAppSignals;
};

class DialogSignalSnapshot : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSignalSnapshot(MonitorConfigController* configController, TcpSignalClient* tcpSignalClient, QWidget *parent = 0);
	~DialogSignalSnapshot();

protected slots:
	void headerColumnContextMenuRequested(const QPoint& pos);

	void headerColumnToggled(bool checked);

private slots:
	void on_DialogSignalSnapshot_finished(int result);

	void contextMenuRequested(const QPoint& pos);

	void on_tableView_doubleClicked(const QModelIndex &index);

	void sortIndicatorChanged(int column, Qt::SortOrder order);

	void on_typeCombo_currentIndexChanged(int index);

	void on_buttonMaskApply_clicked();

	void on_editMask_returnPressed();

	void on_buttonMaskInfo_clicked();

	void tcpSignalClient_signalParamAndUnitsArrived();

	void configController_configurationArrived(ConfigSettings configuration);

	void on_schemaCombo_currentIndexChanged(const QString &arg1);

	void on_comboMaskType_currentIndexChanged(int index);

	void on_buttonExport_clicked();

	void on_buttonPrint_clicked();

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void maskChanged();

	void fillSchemas();

	void fillSignals();

private:
	ConfigSettings m_configuration;

	Ui::DialogSignalSnapshot *ui;

	QCompleter* m_completer = nullptr;

	SignalSnapshotModel *m_model = nullptr;

	MonitorConfigController* m_configController = nullptr;

	TcpSignalClient* m_tcpSignalClient = nullptr;

	int m_updateStateTimerId = -1;
};

#endif // DIALOGSIGNALSNAPSHOT_H
