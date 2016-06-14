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


	void removeAll();
	void setSignals(const std::vector<Signal>& signalList);
	void update();

public:
	QStringList m_columnsNames;
	std::vector<int> m_columnsIndexes;

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

protected:
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


private:
	std::vector<Signal> m_signals;

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

private:
	virtual void timerEvent(QTimerEvent* event) override;

	void fillSignals();

private:
	Ui::DialogSignalSnapshot *ui;

	SnapshotItemModel *m_model;

	int m_updateStateTimerId = -1;


};

#endif // DIALOGSIGNALSNAPSHOT_H
