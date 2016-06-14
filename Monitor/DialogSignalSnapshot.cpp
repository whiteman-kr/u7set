#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include <QFileSystemModel>
#include "Stable.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"

SnapshotItemModel::SnapshotItemModel(QObject* parent)
	:QAbstractItemModel(parent)
{
	// Fill column names

	m_columnsNames<<tr("Signal ID");
	m_columnsNames<<tr("Equipment ID");
	m_columnsNames<<tr("App Signal ID");
	m_columnsNames<<tr("Caption");
	m_columnsNames<<tr("Units");
	m_columnsNames<<tr("Type");

	m_columnsNames<<tr("System Time");
	m_columnsNames<<tr("Local Time");
	m_columnsNames<<tr("Plant Time");
	m_columnsNames<<tr("Value");
	m_columnsNames<<tr("Valid");
	m_columnsNames<<tr("Underflow");
	m_columnsNames<<tr("Overflow");


	if (theSettings.m_signalSnapshotColumnCount == 0)
	{
		m_columnsIndexes.push_back(SignalID);
		m_columnsIndexes.push_back(Caption);
		m_columnsIndexes.push_back(Units);
		m_columnsIndexes.push_back(Type);

		m_columnsIndexes.push_back(LocalTime);
		m_columnsIndexes.push_back(Value);
		m_columnsIndexes.push_back(Valid);
		m_columnsIndexes.push_back(Underflow);
		m_columnsIndexes.push_back(Overflow);
	}
	else
	{
		const int* begin = reinterpret_cast<int*>(theSettings.m_signalSnapshotColumns.data());
		const int* end = begin + theSettings.m_signalSnapshotColumnCount;

		std::vector<int> buffer(begin, end);
		m_columnsIndexes = buffer;
	}
}

void SnapshotItemModel::removeAll()
{
	m_signals.empty();
}

void SnapshotItemModel::setSignals(const std::vector<Signal>& signalList)
{
	m_signals = signalList;

}

std::vector<int> SnapshotItemModel::сolumnsIndexes()
{
	return m_columnsIndexes;

}

void SnapshotItemModel::setColumnsIndexes(std::vector<int> columnsIndexes)
{
	beginResetModel();

	removeRows(0, rowCount());

	m_columnsIndexes = columnsIndexes;

	insertRows(0, (int)m_columnsIndexes.size());

	endResetModel();

}

QStringList SnapshotItemModel::columnsNames()
{
	return m_columnsNames;
}


void SnapshotItemModel::update()
{
	emit QAbstractItemModel::dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

const Signal& SnapshotItemModel::signal(int index)
{
	return m_signals[index];

}

QModelIndex SnapshotItemModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

QModelIndex SnapshotItemModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();

}

int SnapshotItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_columnsIndexes.size();

}

int SnapshotItemModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_signals.size();

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

		if (col < 0 || col >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		//QString str = QString("%1:%2").arg(row).arg(col);
		//qDebug()<<str;

		const Signal& s = m_signals[row];

		AppSignalState state;
		bool stateOk = false;

		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == SignalID)
		{
			return s.customAppSignalID();
		}

		if (displayIndex == EquipmentID)
		{
			return s.equipmentID();
		}

		if (displayIndex == AppSignalID)
		{
			return s.appSignalID();
		}

		if (displayIndex == Caption)
		{
			return s.caption();
		}

		if (displayIndex == Units)
		{
			return theSignals.units(s.unitID());
		}

		if (displayIndex == Type)
		{
			QString str = E::valueToString<E::SignalType>(s.type());
			if (s.isAnalog())
			{
				str = QString("%1 (%2)").arg(str).arg(E::valueToString<E::DataFormat>(s.dataFormat()));
			}
			str = QString("%1, %2").arg(str).arg(E::valueToString<E::SignalInOutType>(s.inOutTypeInt()));

			return str;
		}

		//
		// State
		//

		if (displayIndex >= SystemTime)
		{
			state = theSignals.signalState(s.appSignalID(), &stateOk);
		}

		if (stateOk == true)
		{
			if (displayIndex == SystemTime)
			{
				QDateTime time = QDateTime::fromMSecsSinceEpoch(state.time.system);
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}

			if (displayIndex == LocalTime)
			{
				QDateTime time = QDateTime::fromMSecsSinceEpoch(state.time.local);
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}

			if (displayIndex == PlantTime)
			{
				QDateTime time = QDateTime::fromMSecsSinceEpoch(state.time.plant);
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}

			if (displayIndex == Value)
			{
				if (state.flags.valid == true)
				{
					if (s.isDiscrete())
					{
						return ((int)state.value == s.normalState()) ? tr("No") : tr("Yes");
					}
					if (s.isAnalog())
					{
						QString str = QString::number(state.value, 'f', s.decimalPlaces());
						if (state.flags.underflow == true)
						{
							str += tr(" [Underflow");
						}

						return str;
					}
				}
				else
				{
					return tr("???");
				}
			}


			if (displayIndex == Valid)
			{
				return (state.flags.valid == true) ? tr("Yes") : tr("No");
			}

			if (displayIndex == Underflow)
			{
				return (state.flags.underflow == true) ? tr("Yes") : tr("No");
			}

			if (displayIndex == Overflow)
			{
				return (state.flags.overflow == true) ? tr("Yes") : tr("No");
			}
		}




		return QVariant();
	}
	return QVariant();
}

QVariant SnapshotItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);
	if (role == Qt::DisplayRole)
	{
		if (section < 0 || section >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int displayIndex = m_columnsIndexes[section];
		return m_columnsNames.at(displayIndex);
	}

	return QVariant();
}




DialogSignalSnapshot::DialogSignalSnapshot(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalSnapshot)
{
	ui->setupUi(this);

	// Restore window pos
	//
	if (theSettings.m_signalSnapshotPos.x() != -1 && theSettings.m_signalSnapshotPos.y() != -1)
	{
		move(theSettings.m_signalSnapshotPos);
		restoreGeometry(theSettings.m_signalSnapshotGeometry);
	}

	m_model = new SnapshotItemModel(this);

	ui->tableView->setModel(m_model);
	ui->tableView->verticalHeader()->hide();

	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	//ui->tableView->setSortingEnabled(true);
	//ui->tableView->horizontalHeader()->setStretchLastSection(true);

	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSnapshot::prepareContextMenu);




	fillSignals();


	ui->tableView->resizeColumnsToContents();

	m_updateStateTimerId = startTimer(500);
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

void DialogSignalSnapshot::on_buttonColumns_clicked()
{
	DialogColumns dc(this, m_model->columnsNames(), m_model->сolumnsIndexes());
	if (dc.exec() == QDialog::Accepted)
	{
		m_model->setColumnsIndexes(dc.columnsIndexes());
		ui->tableView->resizeColumnsToContents();


	}
}

void DialogSignalSnapshot::on_DialogSignalSnapshot_finished(int result)
{
	Q_UNUSED(result);

	/*// Save columns width
	//
	theSettings.m_signalSearchColumnWidth.clear();

	QDataStream stream(&theSettings.m_signalSearchColumnWidth, QIODevice::WriteOnly);

	for (int i = 0; i < ui->signalsTree->columnCount(); i++)
	{
		stream << (int)ui->signalsTree->columnWidth(i);
	}
	theSettings.m_signalSearchColumnCount = ui->signalsTree->columnCount();*/


	std::vector<int> columnIndexes = m_model->сolumnsIndexes();
	theSettings.m_signalSnapshotColumnCount = (int)columnIndexes.size();

	theSettings.m_signalSnapshotColumns = QByteArray(reinterpret_cast<const char*>(columnIndexes.data()), (int)columnIndexes.size() * sizeof(int));

	// Save window position
	//
	theSettings.m_signalSnapshotPos = pos();
	theSettings.m_signalSnapshotGeometry = saveGeometry();

}

void DialogSignalSnapshot::prepareContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		assert(cw);
		return;
	}

	int row = ui->tableView->currentIndex().row();
	if (row == -1)
	{
		return;
	}

	cw->currentTab()->signalContextMenu(QStringList()<<m_model->signal(row).appSignalID());

}

void DialogSignalSnapshot::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		if (ui->buttonFixate->isChecked() == false)
		{
			m_model->update();
		}
	}
}

void DialogSignalSnapshot::on_tableView_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		assert(cw);
		return;
	}

	int row = ui->tableView->currentIndex().row();
	if (row == -1)
	{
		return;
	}

	cw->currentTab()->signalInfo(m_model->signal(row).appSignalID());
}
