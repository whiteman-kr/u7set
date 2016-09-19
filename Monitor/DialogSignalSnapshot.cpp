#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include <QFileSystemModel>
#include <QCompleter>
#include "Stable.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"

SnapshotItemSorter::SnapshotItemSorter(int column, Qt::SortOrder order):
	m_column(column),
	m_order(order)
{
}

bool SnapshotItemSorter::sortFunction(const SnapshotItem& o1, const SnapshotItem& o2, int column, Qt::SortOrder order) const
{
	bool found = false;

	const Signal& s1 = theSignals.signal(o1.first, &found);
	if (found == false)
	{
		return false;
	}

	const Signal& s2 = theSignals.signal(o2.first, &found);
	if (found == false)
	{
		return false;
	}

	const AppSignalState& st1 = o1.second;
	const AppSignalState& st2 = o2.second;

	static QVariant v1;
	static QVariant v2;

	switch (column)
	{
	case SnapshotItemModel::SignalID:
	{
		v1 = s1.customAppSignalID();
		v2 = s2.customAppSignalID();
	}
		break;
	case SnapshotItemModel::EquipmentID:
	{
		v1 = s1.equipmentID();
		v2 = s2.equipmentID();
	}
		break;
	case SnapshotItemModel::AppSignalID:
	{
		v1 = s1.appSignalID();
		v2 = s2.appSignalID();
	}
		break;
	case SnapshotItemModel::Caption:
	{
		v1 = s1.caption();
		v2 = s2.caption();
	}
		break;
	case SnapshotItemModel::Units:
	{
		v1 = s1.unitID();
		v2 = s2.unitID();
	}
		break;
	case SnapshotItemModel::Type:
	{
		if (s1.type() == s2.type())
		{
			if (s1.dataFormat() == s2.dataFormat())
			{
				v1 = s1.inOutTypeInt();
				v2 = s2.inOutTypeInt();
			}
			else
			{
				v1 = s1.dataFormatInt();
				v2 = s2.dataFormatInt();
			}
		}
		else
		{
			v1 = s1.typeInt();
			v2 = s2.typeInt();
		}
	}
		break;

	case SnapshotItemModel::SystemTime:
	{
		v1 = st1.time.system;
		v2 = st2.time.system;
	}
		break;
	case SnapshotItemModel::LocalTime:
	{
		v1 = st1.time.local;
		v2 = st2.time.local;
	}
		break;
	case SnapshotItemModel::PlantTime:
	{
		v1 = st1.time.plant;
		v2 = st2.time.plant;
	}
		break;
	case SnapshotItemModel::Value:
	{
		if (s1.isAnalog() == s2.isAnalog())
		{
		   v1 = st1.value;
		   v2 = st2.value;
		}
		else
		{
			v1 = s1.isAnalog();
			v2 = s2.isAnalog();
		}
	}
		break;
	case SnapshotItemModel::Valid:
	{
		v1 = st1.flags.valid;
		v2 = st2.flags.valid;
	}
		break;
	case SnapshotItemModel::Underflow:
	{
		v1 = st1.flags.underflow;
		v2 = st2.flags.underflow;
	}
		break;
	case SnapshotItemModel::Overflow:
	{
		v1 = st1.flags.overflow;
		v2 = st2.flags.overflow;
	}
		break;
	default:
		assert(false);
		return false;
	}

	if (v1 == v2)
	{
		return s1.customAppSignalID() < s2.customAppSignalID();
	}

	if (order == Qt::AscendingOrder)
		return v1 < v2;
	else
		return v1 > v2;
}

//
//SnapshotItemModel
//

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

void SnapshotItemModel::setSignals(std::vector<SnapshotItem>* signalsTable)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_signalsTable.clear();

		endRemoveRows();
	}

	if (signalsTable->empty() == true)
	{
		return;
	}

	//

	beginInsertRows(QModelIndex(), 0, static_cast<int>(signalsTable->size()) - 1);

	std::swap(m_signalsTable, *signalsTable);

	insertRows(0, static_cast<int>(m_signalsTable.size()));

	endInsertRows();
}

std::vector<int> SnapshotItemModel::columnsIndexes() const
{
	return m_columnsIndexes;
}

void SnapshotItemModel::setColumnsIndexes(std::vector<int> columnsIndexes)
{
	if (columnCount() > 0)
	{
		beginRemoveColumns(QModelIndex(), 0, columnCount() - 1);

		removeColumns(0, columnCount());

		m_columnsIndexes.clear();

		endRemoveColumns();
	}

	beginInsertColumns(QModelIndex(), 0, (int)columnsIndexes.size() - 1);

	m_columnsIndexes = columnsIndexes;

	insertColumns(0, (int)m_columnsIndexes.size());

	endInsertColumns();

}

int SnapshotItemModel::columnIndex(int index) const
{
	if (index <0 || index >= m_columnsIndexes.size())
	{
		assert(false);
		return -1;
	}

	return m_columnsIndexes[index];
}

QStringList SnapshotItemModel::columnsNames() const
{
	return m_columnsNames;
}

Hash SnapshotItemModel::signalHash(int index) const
{
	if (index < 0 || index >= m_signalsTable.size())
	{
		assert(false);
		return 0;
	}
	return m_signalsTable[index].first;

}

void SnapshotItemModel::updateStates(int from, int to)
{
	if (m_signalsTable.size() == 0)
	{
		return;
	}
	if (from >= m_signalsTable.size() || to >= m_signalsTable.size())
	{
		assert(false);
		return;
	}

	std::vector<Hash> requestHashes;
	requestHashes.reserve(to - from);

	std::vector<AppSignalState> requestStates;

	for (int i = from; i <= to; i++)
	{
		requestHashes.push_back(m_signalsTable[i].first);
	}

	int count = theSignals.signalState(requestHashes, &requestStates);

	if (count != requestHashes.size() || count != requestStates.size())
	{
		assert(false);
		return;
	}

	int state = 0;
	for (int i = from; i <= to; i++)
	{
		m_signalsTable[i].second = requestStates[state];
		state++;
	}
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
	return (int)m_signalsTable.size();

}

void SnapshotItemModel::sort(int column, Qt::SortOrder order)
{
	if (column < 0 || column >= m_columnsIndexes.size())
	{
		assert(false);
		return;
	}

	int sortColumnIndex = m_columnsIndexes[column];

	//
	// Fill the states map for sorting
	//

	std::sort(m_signalsTable.begin(), m_signalsTable.end(), SnapshotItemSorter(sortColumnIndex, order));

	if (rowCount() > 0)
	{
		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	}

	return;
}

QVariant SnapshotItemModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		int col = index.column();
		if (col < 0 || col >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int row = index.row();
		if (row >= m_signalsTable.size())
		{
			assert(false);
			return QVariant();
		}

		int displayIndex = m_columnsIndexes[col];

		//
		// State
		//

		const AppSignalState& state = m_signalsTable[row].second;

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

		//
		// Get signal now
		//

		bool found = false;
		Signal s = theSignals.signal(m_signalsTable[row].first, &found);

		if (found == false)
		{
			assert(false);
			return QVariant();
		}

		if (displayIndex == Value)
		{
			if (state.flags.valid == true)
			{
				if (s.isDiscrete() == true)
				{
					return static_cast<int>(state.value) == 0 ? "0" : "1";
				}

				if (s.isAnalog() == true)
				{
					QString str = QString::number(state.value, 'f', s.decimalPlaces());

					if (state.flags.underflow == true)
					{
						str += tr(" [Underflow]");
					}

					return str;
				}
			}
			else
			{
				return tr("?");
			}
		}

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

			if (s.isAnalog() == true)
			{
				str = QString("%1 (%2)").arg(str).arg(E::valueToString<E::DataFormat>(s.dataFormat()));
			}

			str = QString("%1, %2").arg(str).arg(E::valueToString<E::SignalInOutType>(s.inOutTypeInt()));

			return str;
		}

		return QVariant();
	}

	return QVariant();
}

QVariant SnapshotItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
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

//
//DialogSignalSnapshot
//

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


	// crete models
	//
	m_model = new SnapshotItemModel(this);

	ui->tableView->setModel(m_model);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->horizontalHeader()->setStretchLastSection(false);
	ui->tableView->setSortingEnabled(true);

	connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogSignalSnapshot::sortIndicatorChanged);

	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSnapshot::prepareContextMenu);

	ui->typeCombo->addItem(tr("All signals"), SnapshotItemModel::All);
	ui->typeCombo->addItem(tr("Analog Input signals"), SnapshotItemModel::AnalogInput);
	ui->typeCombo->addItem(tr("Analog Output signals"), SnapshotItemModel::AnalogOutput);
	ui->typeCombo->addItem(tr("Discrete Input signals"), SnapshotItemModel::DiscreteInput);
	ui->typeCombo->addItem(tr("Discrete Output signals"), SnapshotItemModel::DiscreteOutput);
	ui->typeCombo->blockSignals(true);
	ui->typeCombo->setCurrentIndex(theSettings.m_signalSnapshotSignalType);
	ui->typeCombo->blockSignals(false);

	m_completer = new QCompleter(theSettings.m_signalSnapshotMaskList, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);

	ui->editMask->setCompleter(m_completer);

	connect(ui->editMask, &QLineEdit::textEdited, [=](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), ui->editMask, &QLineEdit::setText);

	connect(theMonitorMainWindow, &MonitorMainWindow::signalParamAndUnitsArrived, this, &DialogSignalSnapshot::tcpSignalClient_signalParamAndUnitsArrived);
	connect(theMonitorMainWindow, &MonitorMainWindow::connectionReset, this, &DialogSignalSnapshot::tcpSignalClient_connectionReset);

	// get signals
	//
	m_signalsHashes = theSignals.signalHashes();

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
	std::vector<SnapshotItem> filteredTable;

	filteredTable.reserve(m_signalsHashes.size());

	for (const Hash& hash : m_signalsHashes)
	{
		bool found = false;
		const Signal& s = theSignals.signal(hash, &found);
		if (found == false)
		{
			continue;
		}

		switch (m_signalType)
		{
		case SnapshotItemModel::AnalogInput:
			if (s.isAnalog() == false || s.isInput() == false)
			{
				continue;
			}
			break;
		case SnapshotItemModel::AnalogOutput:
			if (s.isAnalog() == false || s.isOutput() == false)
			{
				continue;
			}
			break;
		case SnapshotItemModel::DiscreteInput:
			if (s.isDiscrete() == false || s.isInput() == false)
			{
				continue;
			}
			break;
		case SnapshotItemModel::DiscreteOutput:
			if (s.isDiscrete() == false || s.isOutput() == false)
			{
				continue;
			}
			break;
		}

		if (m_strIdMasks.isEmpty() == false)
		{
			bool result = false;
			QString strId = s.customAppSignalID().trimmed();
			for (QString idMask : m_strIdMasks)
			{
				QRegExp rx(idMask.trimmed());
				rx.setPatternSyntax(QRegExp::Wildcard);
				if (rx.exactMatch(strId))
				{
					result = true;
					break;
				}
			}
			if (result == false)
			{
				continue;
			}
		}

		filteredTable.push_back(std::make_pair(s.hash(), AppSignalState()));
	}

	m_model->setSignals(&filteredTable);
}

void DialogSignalSnapshot::on_buttonColumns_clicked()
{
	DialogColumns dc(this, m_model->columnsNames(), m_model->columnsIndexes());
	if (dc.exec() == QDialog::Accepted)
	{
		m_model->setColumnsIndexes(dc.columnsIndexes());
		ui->tableView->resizeColumnsToContents();
	}
}

void DialogSignalSnapshot::on_DialogSignalSnapshot_finished(int result)
{
	Q_UNUSED(result);

	std::vector<int> columnIndexes = m_model->columnsIndexes();
	theSettings.m_signalSnapshotColumnCount = (int)columnIndexes.size();

	theSettings.m_signalSnapshotColumns = QByteArray(reinterpret_cast<const char*>(columnIndexes.data()), (int)columnIndexes.size() * sizeof(int));

	theSettings.m_signalSnapshotSignalType = ui->typeCombo->currentIndex();

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

	Hash hash = m_model->signalHash(ui->tableView->currentIndex().row());
	if (hash == 0)
	{
		assert(false);
		return;
	}

	bool found = false;
	const Signal& s = theSignals.signal(hash, &found);

	if (found == false)
	{
		return;
	}

	cw->currentTab()->signalContextMenu(QStringList()<<s.appSignalID());
}

void DialogSignalSnapshot::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		if (ui->buttonFixate->isChecked() == false)
		{
			// Update only visible dynamic items
			//
			int from = ui->tableView->rowAt(0);
			int to = ui->tableView->rowAt(ui->tableView->height());

			// Update signal states
			//
			m_model->updateStates(from, to);

			// Redraw visible table items
			//
			for (int col = 0; col < m_model->columnCount(); col++)
			{
				 int displayIndex = m_model->columnIndex(col);

				 if (displayIndex >= SnapshotItemModel::SystemTime)
				 {
					 for (int row = from; row <= to; row++)
					 {
						ui->tableView->update(m_model->index(row, col));
					 }
				 }
			}
		}
	}
}

void DialogSignalSnapshot::maskChanged()
{
	// Get mask
	//
	QString mask = ui->editMask->text();

	if (mask.isEmpty() == false)
	{
		m_strIdMasks = ui->editMask->text().split(';');

		for (auto mask : m_strIdMasks)
		{
			// Save filter history
			//
			if (theSettings.m_signalSnapshotMaskList.contains(mask) == false)
			{
				theSettings.m_signalSnapshotMaskList.append(mask);

				QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
				if (model == nullptr)
				{
					assert(model);
					return;
				}
				model->setStringList(theSettings.m_signalSnapshotMaskList);
			}
		}
	}
	else
	{
		m_strIdMasks.clear();
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

	Hash hash = m_model->signalHash(ui->tableView->currentIndex().row());
	if (hash == 0)
	{
		assert(false);
		return;
	}

	bool found = false;
	const Signal& s = theSignals.signal(hash, &found);

	if (found == false)
	{
		return;
	}

	cw->currentTab()->signalInfo(s.appSignalID());
}

void DialogSignalSnapshot::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
	m_model->sort(logicalIndex, order);
}

void DialogSignalSnapshot::on_typeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);

	// Get signal type
	//
	m_signalType = ui->typeCombo->currentData().toInt();
	fillSignals();
}

void DialogSignalSnapshot::on_buttonMaskApply_clicked()
{
	maskChanged();
	fillSignals();
}

void DialogSignalSnapshot::on_editMask_returnPressed()
{
	maskChanged();
	fillSignals();
}

void DialogSignalSnapshot::on_buttonMaskInfo_clicked()
{
	QMessageBox::information(this, tr("Signal Snapshot"), tr("A mask contains '*' and '?' symbols.\r\nSeveral masks separated by ';' can be entered."));
}

void DialogSignalSnapshot::tcpSignalClient_signalParamAndUnitsArrived()
{
	m_signalsHashes = theSignals.signalHashes();
	fillSignals();
	ui->tableView->resizeColumnsToContents();
}

void DialogSignalSnapshot::tcpSignalClient_connectionReset()
{
	m_signalsHashes.clear();
	fillSignals();
}
