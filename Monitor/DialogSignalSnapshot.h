#ifndef DIALOGSIGNALSNAPSHOT_H
#define DIALOGSIGNALSNAPSHOT_H

#include <QDialog>
#include <QAbstractItemModel>
#include "../lib/AppSignalManager.h"

namespace Ui {
class DialogSignalSnapshot;
}

class SnapshotItemModel : public QAbstractItemModel
{
public:
	SnapshotItemModel(QObject *parent);
public:


	void removeAll();
	void setSignals(const std::vector<Signal>& signalList);

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

private:
	void fillSignals();

private:
	Ui::DialogSignalSnapshot *ui;

	SnapshotItemModel *m_model;
};

#endif // DIALOGSIGNALSNAPSHOT_H
