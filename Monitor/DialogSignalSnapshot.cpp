#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include <QFileSystemModel>
#include <QCompleter>
#include "Stable.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"

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

void SnapshotItemModel::setSignals(const std::vector<Signal*>& signalList)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_signals.clear();

		endRemoveRows();
	}

	//

	beginInsertRows(QModelIndex(), 0, (int)signalList.size());

	m_signals = signalList;

	insertRows(0, (int)signalList.size());

	endInsertRows();

}

std::vector<int> SnapshotItemModel::columnsIndexes()
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

QStringList SnapshotItemModel::columnsNames()
{
	return m_columnsNames;
}


void SnapshotItemModel::update()
{
	emit QAbstractItemModel::dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

const Signal* SnapshotItemModel::signal(int index)
{
	if (index < 0 || index >= m_signals.size())
	{
		assert(false);
		return nullptr;
	}
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

		Signal* s = m_signals[row];
		if (s == nullptr)
		{
			assert(s);
			return QVariant();
		}

		AppSignalState state;
		bool stateOk = false;

		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == SignalID)
		{
			return s->customAppSignalID();
		}

		if (displayIndex == EquipmentID)
		{
			return s->equipmentID();
		}

		if (displayIndex == AppSignalID)
		{
			return s->appSignalID();
		}

		if (displayIndex == Caption)
		{
			return s->caption();
		}

		if (displayIndex == Units)
		{
			return theSignals.units(s->unitID());
		}

		if (displayIndex == Type)
		{
			QString str = E::valueToString<E::SignalType>(s->type());
			if (s->isAnalog())
			{
				str = QString("%1 (%2)").arg(str).arg(E::valueToString<E::DataFormat>(s->dataFormat()));
			}
			str = QString("%1, %2").arg(str).arg(E::valueToString<E::SignalInOutType>(s->inOutTypeInt()));

			return str;
		}

		//
		// State
		//

		if (displayIndex >= SystemTime)
		{
			state = theSignals.signalState(s->appSignalID(), &stateOk);
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
					if (s->isDiscrete())
					{
						return ((int)state.value == s->normalState()) ? tr("No") : tr("Yes");
					}
					if (s->isAnalog())
					{
						QString str = QString::number(state.value, 'f', s->decimalPlaces());
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
//SnapshotItemModel
//
SnapshotItemProxyModel::SnapshotItemProxyModel(SnapshotItemModel *sourceModel, QObject *parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel)
{
	setSourceModel(sourceModel);
}

bool SnapshotItemProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
	const Signal* s = m_sourceModel->signal(source_row);
	if (s == nullptr)
	{
		assert(s);
		return false;
	}

	switch (m_signalType)
	{
	case SnapshotItemModel::AnalogInput:
		if (s->isAnalog() == false || s->isInput() == false)
		{
			return false;
		}
		break;
	case SnapshotItemModel::AnalogOutput:
		if (s->isAnalog() == false || s->isOutput() == false)
		{
			return false;
		}
		break;
	case SnapshotItemModel::DiscreteInput:
		if (s->isDiscrete() == false || s->isInput() == false)
		{
			return false;
		}
		break;
	case SnapshotItemModel::DiscreteOutput:
		if (s->isDiscrete() == false || s->isOutput() == false)
		{
			return false;
		}
		break;
	}

	if (m_strIdMasks.isEmpty() == false)
	{
		QString strId = s->customAppSignalID().trimmed();
		for (QString idMask : m_strIdMasks)
		{
			QRegExp rx(idMask.trimmed());
			rx.setPatternSyntax(QRegExp::Wildcard);
			if (rx.exactMatch(strId))
			{
				return true;
			}
		}
		return false;
	}

	return true;
}

bool SnapshotItemProxyModel::lessThan(const QModelIndex &left,
									   const QModelIndex &right) const
{
	 const Signal* s1 = m_sourceModel->signal(left.row());
	 const Signal* s2 = m_sourceModel->signal(right.row());

	 if (s1 == nullptr || s2 == nullptr)
	 {
		 assert(false);
		 return false;
	 }

	 std::vector<int> ci = m_sourceModel->columnsIndexes();

	 int sc = sortColumn();
	 if (sc < 0 || sc >= ci.size())
	 {
		assert(false);
		return false;
	 }

	 static AppSignalState st1;
	 static AppSignalState st2;

	 int c = ci[sc];
	 if (c >= SnapshotItemModel::SystemTime)
	 {
		 st1 = theSignals.signalState(s1->appSignalID());
		 st2 = theSignals.signalState(s2->appSignalID());
	 }

	 QVariant v1;
	 QVariant v2;


	 switch (c)
	 {
	 case SnapshotItemModel::SignalID:
	 {
		 v1 = s1->customAppSignalID();
		 v2 = s2->customAppSignalID();
	 }
		 break;
	 case SnapshotItemModel::EquipmentID:
	 {
		 v1 = s1->equipmentID();
		 v2 = s2->equipmentID();
	 }
		 break;
	 case SnapshotItemModel::AppSignalID:
	 {
		 v1 = s1->appSignalID();
		 v2 = s2->appSignalID();
	 }
		 break;
	 case SnapshotItemModel::Caption:
	 {
		 v1 = s1->caption();
		 v2 = s2->caption();
	 }
		 break;
	 case SnapshotItemModel::Units:
	 {
		 v1 = s1->unitID();
		 v2 = s2->unitID();
	 }
		 break;
	 case SnapshotItemModel::Type:
	 {
		 if (s1->type() == s2->type())
		 {
			 if (s1->dataFormat() == s2->dataFormat())
			 {
				 v1 = s1->inOutTypeInt();
				 v2 = s2->inOutTypeInt();
			 }
			 else
			 {
				 v1 = s1->dataFormatInt();
				 v2 = s2->dataFormatInt();
			 }
		 }
		 else
		 {
			 v1 = s1->typeInt();
			 v2 = s2->typeInt();
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
		 if (s1->isAnalog() == s2->isAnalog())
		 {
			v1 = st1.value;
			v2 = st2.value;
		 }
		 else
		 {
			 v1 = s1->isAnalog();
			 v2 = s2->isAnalog();
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
		 return s1->customAppSignalID() < s2->customAppSignalID();
	 }

	 return v1 < v2;

}

void SnapshotItemProxyModel::setSignalTypeFilter(int signalType)
{
	m_signalType = signalType;
}

void SnapshotItemProxyModel::setSignalIdFilter(QStringList strIds)
{
	m_strIdMasks = strIds;
}

void SnapshotItemProxyModel::refreshFilters()
{
	invalidateFilter();
}

const Signal* SnapshotItemProxyModel::signal(const QModelIndex& mi)
{
	return m_sourceModel->signal(mapToSource(mi).row());
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

	// get signals
	//
	m_signals = theSignals.signalList();

	std::vector<Signal*> filteredSignals;
	for (Signal& s : m_signals)
	{
		filteredSignals.push_back(&s);
	}

	// crete models
	//
	m_model = new SnapshotItemModel(this);
	m_model->setSignals(filteredSignals);

	m_proxyModel = new SnapshotItemProxyModel(m_model, this);

	ui->tableView->setModel(m_proxyModel);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->setSortingEnabled(true);

	ui->tableView->sortByColumn(0, Qt::AscendingOrder);
	//ui->tableView->horizontalHeader()->setStretchLastSection(true);

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
	int typeFilter = ui->typeCombo->currentData().toInt();
	m_proxyModel->setSignalTypeFilter(typeFilter);

	QString mask = ui->editMask->text();

	if (mask.isEmpty() == false)
	{
		// Set signal id filter editor text and save filter history
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

		QStringList maskList = ui->editMask->text().split(';');
		m_proxyModel->setSignalIdFilter(maskList);
	}
	else
	{
		m_proxyModel->setSignalIdFilter(QStringList());
	}

	m_proxyModel->refreshFilters();

}

void DialogSignalSnapshot::on_buttonColumns_clicked()
{
	DialogColumns dc(this, m_model->columnsNames(), m_model->columnsIndexes());
	if (dc.exec() == QDialog::Accepted)
	{
		m_model->setColumnsIndexes(dc.columnsIndexes());
		ui->tableView->sortByColumn(0, Qt::AscendingOrder);
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

	const Signal* s = (const Signal *)m_proxyModel->signal(ui->tableView->currentIndex());
	if (s == nullptr)
	{
		assert(false);
		return;
	}

	cw->currentTab()->signalContextMenu(QStringList()<<s->appSignalID());

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

	const Signal* s = (const Signal *)m_proxyModel->signal(ui->tableView->currentIndex());
	if (s == nullptr)
	{
		assert(false);
		return;
	}

	cw->currentTab()->signalInfo(s->appSignalID());
}

void DialogSignalSnapshot::on_typeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillSignals();

}

void DialogSignalSnapshot::on_buttonMaskApply_clicked()
{
	fillSignals();

}

void DialogSignalSnapshot::on_editMask_returnPressed()
{
	fillSignals();
}

void DialogSignalSnapshot::on_buttonMaskInfo_clicked()
{
	QMessageBox::information(this, tr("Signal Snapshot"), tr("A mask contains '*' and '?' symbols.\r\nSeveral masks separated by ';' can be entered."));
}
