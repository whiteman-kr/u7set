#ifndef DIALOGSIGNALSNAPSHOT_H
#define DIALOGSIGNALSNAPSHOT_H

#include <QDialog>
#include <QAbstractItemModel>
#include "../lib/AppSignalManager.h"
#include "DialogColumns.h"

namespace Ui {
class DialogSignalSnapshot;
}

class SnapshotItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	SnapshotItemModel(QObject *parent);
public:


	void setSignals(const std::vector<Signal*>& signalList);

	std::vector<int> сolumnsIndexes();
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	QStringList columnsNames();

	void update();

	const Signal* signal(int index);

public:

	enum DialogSignalSnapshotColumns
	{
		SignalID = 0,
		EquipmentID,
		AppSignalID,
		Caption,
		Units,
		Type,

		SystemTime,
		LocalTime,
		PlantTime,
		Value,
		Valid,
		Underflow,
		Overflow,
	};

	enum TypeFilter
	{
		All = 0,
		AnalogInput,
		AnalogOutput,
		DiscreteInput,
		DiscreteOutput

	};


protected:
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


private:
	std::vector<Signal*> m_signals;
	QStringList m_columnsNames;
	std::vector<int> m_columnsIndexes;

};

class SnapshotItemProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SnapshotItemProxyModel(SnapshotItemModel* sourceModel, QObject* parent = 0);

	bool filterAcceptsRow(int source_row, const QModelIndex&) const override;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

	void setSignalTypeFilter(int signalType);
	void setSignalIdFilter(QStringList strIds);
	void refreshFilters();

	const Signal* signal(const QModelIndex &mi);

private:
	SnapshotItemModel* m_sourceModel = nullptr;
	int m_signalType = SnapshotItemModel::All;
	QStringList m_strIdMasks;
};

class DialogSignalSnapshot : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSignalSnapshot(QWidget *parent = 0);
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

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void fillSignals();

private:
	Ui::DialogSignalSnapshot *ui;

	QCompleter* m_completer = nullptr;

	SnapshotItemModel *m_model = nullptr;
	SnapshotItemProxyModel *m_proxyModel = nullptr;

	int m_updateStateTimerId = -1;

	std::vector<Signal> m_signals;


};

#endif // DIALOGSIGNALSNAPSHOT_H
