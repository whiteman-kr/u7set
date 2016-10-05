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

typedef std::pair<Hash, AppSignalState> SnapshotItem;

class SnapshotItemModel;

class SnapshotItemSorter
{
public:
	  SnapshotItemSorter(int column, Qt::SortOrder order, SnapshotItemModel* model);

	  bool operator()(const SnapshotItem& o1, const SnapshotItem& o2) const
	  {
		  return sortFunction(o1, o2, m_column, m_order);
	  }

	  bool sortFunction(const SnapshotItem& o1, const SnapshotItem& o2, int column, Qt::SortOrder order) const;

private:
	  int m_column = -1;

	  Qt::SortOrder m_order = Qt::AscendingOrder;

	  SnapshotItemModel* m_model = nullptr;
};


class SnapshotItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	SnapshotItemModel(QObject *parent);

public:

	std::vector<int> columnsIndexes() const;
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	int columnIndex(int index) const;
	QStringList columnsNames() const;

	Hash signalHash(int index) const;

	Signal signalParam(Hash hash, bool* found) const;

	void setSignals(std::vector<SnapshotItem>* signalsTable);

	void updateStates(int from, int to);

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	void sort(int column, Qt::SortOrder order) override;

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
		Valid,
		Underflow,
		Overflow,
	};

	enum class TypeFilter
	{
		All = 0,
		AnalogInput,
		AnalogOutput,
		DiscreteInput,
		DiscreteOutput
	};

protected:
	QModelIndex parent(const QModelIndex &index) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:

	std::vector<std::pair<Hash, AppSignalState>> m_signalsTable;

	QStringList m_columnsNames;
	std::vector<int> m_columnsIndexes;

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

	void fillSignals();

	void fillSchemas();

public:


	enum class MaskType
	{
		AppSignalId,
		CustomAppSignalId,
		EquipmentId
	};

private:

	Ui::DialogSignalSnapshot *ui;

	QCompleter* m_completer = nullptr;

	SnapshotItemModel *m_model = nullptr;

	MonitorConfigController* m_configController = nullptr;

	int m_updateStateTimerId = -1;

	// Dialog data

	std::vector<Hash> m_signalsHashes;

	// Filtering parameters

	SnapshotItemModel::TypeFilter m_signalType = SnapshotItemModel::TypeFilter::All;

	QStringList m_strIdMasks;
};

#endif // DIALOGSIGNALSNAPSHOT_H
