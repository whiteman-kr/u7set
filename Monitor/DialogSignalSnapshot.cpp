#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include <QFileSystemModel>
#include "Stable.h"

SnapshotItemModel::SnapshotItemModel(QObject* parent)
	:QAbstractItemModel(parent)
{

}

void SnapshotItemModel::removeAll()
{
	m_signals.empty();
}

void SnapshotItemModel::setSignals(const std::vector<Signal>& signalList)
{
	m_signals = signalList;
}

QModelIndex SnapshotItemModel::index(int row, int column, const QModelIndex &parent) const
{
	return createIndex(row, column);
}

QModelIndex SnapshotItemModel::parent(const QModelIndex &index) const
{
	return QModelIndex();

}

int SnapshotItemModel::columnCount(const QModelIndex &parent) const
{
	return 10;

}

int SnapshotItemModel::rowCount(const QModelIndex &parent) const
{
	return m_signals.size();

}

QVariant SnapshotItemModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		int row = index.row();
		if (row >= m_signals.size())
		{
			assert(false);
			return QVariant();
		}

		int col = index.column();

		const Signal& s = m_signals[row];

		if (col == 0)
		{
			return s.customAppSignalID();

		}

		if (col == 1)
		{
			return s.caption();

		}


		return QVariant();
	}
	return QVariant();
}

QVariant SnapshotItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (section == 0)
		{
			return tr("SignalID");

		}

		if (section == 1)
		{
			return tr("Caption");

		}
	}


	return QVariant();
}




DialogSignalSnapshot::DialogSignalSnapshot(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalSnapshot)
{
	ui->setupUi(this);

	m_model = new SnapshotItemModel(this);

	ui->tableView->setModel(m_model);
	ui->tableView->verticalHeader()->hide();

	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);

	fillSignals();
}

DialogSignalSnapshot::~DialogSignalSnapshot()
{
	delete ui;
}

void DialogSignalSnapshot::fillSignals()
{
	m_model->removeAll();
	m_model->setSignals(theSignals.signalList());
}
