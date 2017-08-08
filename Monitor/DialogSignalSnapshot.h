#ifndef DIALOGSIGNALSNAPSHOT_H
#define DIALOGSIGNALSNAPSHOT_H

#include <QDialog>
#include <QAbstractItemModel>
#include "../lib/AppSignalManager.h"
#include "DialogColumns.h"
#include "MonitorConfigController.h"

namespace Ui {
class DialogSignalSnapshot;
}

class SignalSnapshotModel;

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


class SignalSnapshotModel : public QAbstractItemModel
{
	Q_OBJECT

	friend class SignalSnapshotSorter;

public:

	enum class Columns
	{
		SignalID = 0,		// Signal Param Columns
		EquipmentID,
		AppSignalID,
		Caption,
		Units,
		Type,

		SystemTime,			// Signal State Columns
		LocalTime,
		PlantTime,
		Value,
		Valid
	};

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
		All,
		AppSignalId,
		CustomAppSignalId,
		EquipmentId
	};

public:
	SignalSnapshotModel(QObject *parent);

public:
	// Properties

	std::vector<int> columnsIndexes() const;
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	int columnIndex(int index) const;
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
	std::vector<int> m_columnsIndexes;

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
	explicit DialogSignalSnapshot(MonitorConfigController* configController, QWidget *parent = 0);
	~DialogSignalSnapshot();

private slots:
	void on_buttonColumns_clicked();

	void on_DialogSignalSnapshot_finished(int result);

	void prepareContextMenu(const QPoint& pos);

	void on_tableView_doubleClicked(const QModelIndex &index);

	void on_typeCombo_currentIndexChanged(int index);

	void on_buttonMaskApply_clicked();

	void on_editMask_returnPressed();

	void on_buttonMaskInfo_clicked();

	void sortIndicatorChanged(int column, Qt::SortOrder order);

	void tcpSignalClient_signalParamAndUnitsArrived();

	void tcpSignalClient_connectionReset();

	void configController_configurationArrived(ConfigSettings configuration);

	void on_schemaCombo_currentIndexChanged(const QString &arg1);

	void on_comboMaskType_currentIndexChanged(int index);

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void maskChanged();

	void fillSchemas();

	void fillSignals();

private:

	Ui::DialogSignalSnapshot *ui;

	QCompleter* m_completer = nullptr;

	SignalSnapshotModel *m_model = nullptr;

	MonitorConfigController* m_configController = nullptr;

	int m_updateStateTimerId = -1;
};

#endif // DIALOGSIGNALSNAPSHOT_H
