#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include <QFileSystemModel>
#include <QCompleter>
#include "Stable.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"

SnapshotItemSorter::SnapshotItemSorter(int column, Qt::SortOrder order, SnapshotItemModel *model):
	m_column(column),
	m_order(order),
	m_model(model)
{
}

bool SnapshotItemSorter::sortFunction(const SnapshotItem& o1, const SnapshotItem& o2, int column, Qt::SortOrder order) const
{
	if (m_model == nullptr)
	{
		assert(m_model);
		return false;
	}

	bool found = false;

	AppSignalParam s1 = m_model->signalParam(o1.first, &found);
	if (found == false)
	{
		return false;
	}

	AppSignalParam s2 = m_model->signalParam(o2.first, &found);
	if (found == false)
	{
		return false;
	}

	const AppSignalState& st1 = o1.second;
	const AppSignalState& st2 = o2.second;

	QVariant v1;
	QVariant v2;

	switch (static_cast<SnapshotItemModel::Columns>(column))
	{
	case SnapshotItemModel::Columns::SignalID:
		{
			v1 = s1.customSignalId();
			v2 = s2.customSignalId();
		}
		break;
	case SnapshotItemModel::Columns::EquipmentID:
		{
			v1 = s1.equipmentId();
			v2 = s2.equipmentId();
		}
		break;
	case SnapshotItemModel::Columns::AppSignalID:
		{
			v1 = s1.appSignalId();
			v2 = s2.appSignalId();
		}
		break;
	case SnapshotItemModel::Columns::Caption:
		{
			v1 = s1.caption();
			v2 = s2.caption();
		}
		break;
	case SnapshotItemModel::Columns::Units:
		{
			v1 = s1.unitId();
			v2 = s2.unitId();
		}
		break;
	case SnapshotItemModel::Columns::Type:
		{
			if (s1.isDiscrete() == true || s2.isDiscrete() == true)
			{
				v1 = static_cast<int>(s1.inOutType());
				v2 = static_cast<int>(s2.inOutType());
				break;
			}

			if (s1.type() == s2.type())
			{
				if (s1.analogSignalFormat() == s2.analogSignalFormat())
				{
					v1 = static_cast<int>(s1.inOutType());
					v2 = static_cast<int>(s2.inOutType());
				}
				else
				{
					v1 = static_cast<int>(s1.analogSignalFormat());
					v2 = static_cast<int>(s2.analogSignalFormat());
				}
			}
			else
			{
				v1 = static_cast<int>(s1.type());
				v2 = static_cast<int>(s2.type());
			}
		}
		break;

	case SnapshotItemModel::Columns::SystemTime:
		{
			v1 = st1.time.system;
			v2 = st2.time.system;
		}
		break;
	case SnapshotItemModel::Columns::LocalTime:
		{
			v1 = st1.time.local;
			v2 = st2.time.local;
		}
		break;
	case SnapshotItemModel::Columns::PlantTime:
		{
			v1 = st1.time.plant;
			v2 = st2.time.plant;
		}
		break;
	case SnapshotItemModel::Columns::Value:
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
	case SnapshotItemModel::Columns::Valid:
		{
			v1 = st1.flags.valid;
			v2 = st2.flags.valid;
		}
		break;
//	case SnapshotItemModel::Columns::Underflow:
//		{
//			v1 = st1.flags.underflow;
//			v2 = st2.flags.underflow;
//		}
//		break;
//	case SnapshotItemModel::Columns::Overflow:
//		{
//			v1 = st1.flags.overflow;
//			v2 = st2.flags.overflow;
//		}
//		break;
	default:
		assert(false);
		return false;
	}

	if (v1 == v2)
	{
		return s1.customSignalId() < s2.customSignalId();
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
	: QAbstractItemModel(parent)
{
	// Fill column names
	//
	m_columnsNames << tr("Signal ID");
	m_columnsNames << tr("Equipment ID");
	m_columnsNames << tr("App Signal ID");
	m_columnsNames << tr("Caption");
	m_columnsNames << tr("Units");
	m_columnsNames << tr("Type");

	m_columnsNames << tr("System Time");
	m_columnsNames << tr("Local Time");
	m_columnsNames << tr("Plant Time");
	m_columnsNames << tr("Value");
	m_columnsNames << tr("Valid");
	m_columnsNames << tr("Underflow");
	m_columnsNames << tr("Overflow");


	if (theSettings.m_signalSnapshotColumns.isEmpty() == true)
	{
		m_columnsIndexes.push_back(static_cast<int>(Columns::SignalID));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Caption));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Units));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Type));

		m_columnsIndexes.push_back(static_cast<int>(Columns::LocalTime));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Value));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Valid));
//		m_columnsIndexes.push_back(static_cast<int>(Columns::Underflow));
//		m_columnsIndexes.push_back(static_cast<int>(Columns::Overflow));
	}
	else
	{
		m_columnsIndexes.clear();
		m_columnsIndexes.reserve(theSettings.m_signalSnapshotColumns.size());

		m_columnsIndexes = theSettings.m_signalSnapshotColumns.toStdVector();
	}

	return;
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

	return;
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

	std::sort(m_signalsTable.begin(), m_signalsTable.end(), SnapshotItemSorter(sortColumnIndex, order, this));

	if (m_signalsTable.empty() == false)
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

		Columns displayIndex = static_cast<Columns>(m_columnsIndexes[col]);

		//
		// State
		//
		const AppSignalState& state = m_signalsTable[row].second;

		switch (displayIndex)
		{
		case Columns::SystemTime:
			{
				QDateTime time = state.time.systemToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case Columns::LocalTime:
			{
				QDateTime time = state.time.localToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case Columns::PlantTime:
			{
				QDateTime time = state.time.plantToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case Columns::Valid:
			{
				return (state.flags.valid == true) ? tr("") : tr("no");
			}
//		case Columns::Underflow:
//			{
//				return (state.flags.underflow == true) ? tr("yes") : tr("");
//			}
//		case Columns::Overflow:
//			{
//				return (state.flags.overflow == true) ? tr("yes") : tr("");
//			}
		}

		//
		// Get signal now
		//

		bool found = false;
		AppSignalParam s = signalParam(m_signalsTable[row].first, &found);

		if (found == false)
		{
			assert(false);
			return QVariant();
		}

		switch (displayIndex)
		{
		case Columns::Value:
			{
				if (state.flags.valid == true)
				{
					if (s.isDiscrete() == true)
					{
						return static_cast<int>(state.value) == 0 ? "0" : "1";
					}

					if (s.isAnalog() == true)
					{
						QString str = QString::number(state.value, 'f', s.precision());

//						if (state.flags.underflow == true)
//						{
//							str += tr(" [UF]");
//						}

//						if (state.flags.overflow == true)
//						{
//							str += tr(" [OF]");
//						}

						return str;
					}

					assert(false);
				}

				return tr("?");
			}

		case Columns::SignalID:
			{
				return s.customSignalId();
			}

		case Columns::EquipmentID:
			{
				return s.equipmentId();
			}

		case Columns::AppSignalID:
			{
				return s.appSignalId();
			}

		case Columns::Caption:
			{
				return s.caption();
			}

		case Columns::Units:
			{
				return s.unit();
			}

		case Columns::Type:
			{
				QString str = E::valueToString<E::SignalType>(s.type());

				if (s.isAnalog() == true)
				{
					str = QString("%1 (%2)").arg(str).arg(E::valueToString<E::AnalogAppSignalFormat>(static_cast<int>(s.analogSignalFormat())));
				}

				str = QString("%1, %2").arg(str).arg(E::valueToString<E::SignalInOutType>(s.inOutType()));

				return str;
			}

		default:
			assert(false);
		}

		return QVariant();
	} // End of if (role == Qt::DisplayRole)

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

AppSignalParam SnapshotItemModel::signalParam(Hash hash, bool* found) const
{
	if (found == nullptr)
	{
		assert(found);
		return AppSignalParam();
	}

	*found = false;

	AppSignalParam s1 = theSignals.signal(hash, found);
	if (*found == false)
	{
		return AppSignalParam();
	}

	return s1;
}

//
//DialogSignalSnapshot
//

DialogSignalSnapshot::DialogSignalSnapshot(MonitorConfigController *configController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_configController(configController),
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

	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	// crete models
	//
	m_model = new SnapshotItemModel(this);

	ui->tableView->setModel(m_model);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->horizontalHeader()->setStretchLastSection(false);
	ui->tableView->setGridStyle(Qt::PenStyle::NoPen);
	ui->tableView->setSortingEnabled(true);

	connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogSignalSnapshot::sortIndicatorChanged);

	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSnapshot::prepareContextMenu);

	ui->typeCombo->blockSignals(true);
	ui->typeCombo->addItem(tr("All signals"), static_cast<int>(SnapshotItemModel::TypeFilter::All));
	ui->typeCombo->addItem(tr("Analog Input signals"), static_cast<int>(SnapshotItemModel::TypeFilter::AnalogInput));
	ui->typeCombo->addItem(tr("Analog Output signals"), static_cast<int>(SnapshotItemModel::TypeFilter::AnalogOutput));
	ui->typeCombo->addItem(tr("Discrete Input signals"), static_cast<int>(SnapshotItemModel::TypeFilter::DiscreteInput));
	ui->typeCombo->addItem(tr("Discrete Output signals"), static_cast<int>(SnapshotItemModel::TypeFilter::DiscreteOutput));
	ui->typeCombo->setCurrentIndex(theSettings.m_signalSnapshotSignalType);
	ui->typeCombo->blockSignals(false);

	fillSchemas();

	m_completer = new QCompleter(theSettings.m_signalSnapshotMaskList, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);

	ui->editMask->setCompleter(m_completer);

	ui->comboMaskType->blockSignals(true);
	ui->comboMaskType->addItem("AppSignalId");
	ui->comboMaskType->addItem("CustomAppSignalId");
	ui->comboMaskType->addItem("EquipmentId");
	ui->comboMaskType->setCurrentIndex(static_cast<int>(theSettings.m_signalSnapshotMaskType));
	ui->comboMaskType->blockSignals(false);

	connect(ui->editMask, &QLineEdit::textEdited, [=](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), ui->editMask, &QLineEdit::setText);

	connect(theMonitorMainWindow, &MonitorMainWindow::signalParamAndUnitsArrived, this, &DialogSignalSnapshot::tcpSignalClient_signalParamAndUnitsArrived);
	connect(theMonitorMainWindow, &MonitorMainWindow::connectionReset, this, &DialogSignalSnapshot::tcpSignalClient_connectionReset);

	connect(m_configController, &MonitorConfigController::configurationArrived, this, &DialogSignalSnapshot::configController_configurationArrived);

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

	// Get current schema's App Signals
	//
	QString currentSchemaStrId;
	QVariant data = ui->schemaCombo->currentData();
	if (data.isValid() == true)
	{
		currentSchemaStrId = data.toString();
	}

	std::set<QString> schemaAppSignals;
	if (currentSchemaStrId.isEmpty() == false)
	{
		schemaAppSignals = m_configController->schemaAppSignals(currentSchemaStrId);
	}

	// Fill signals
	//
	for (const Hash& hash : m_signalsHashes)
	{
		bool found = false;
		const AppSignalParam& s = theSignals.signal(hash, &found);
		if (found == false)
		{
			continue;
		}

		switch (m_signalType)
		{
		case SnapshotItemModel::TypeFilter::AnalogInput:
			if (s.isAnalog() == false || s.isInput() == false)
			{
				continue;
			}
			break;
		case SnapshotItemModel::TypeFilter::AnalogOutput:
			if (s.isAnalog() == false || s.isOutput() == false)
			{
				continue;
			}
			break;
		case SnapshotItemModel::TypeFilter::DiscreteInput:
			if (s.isDiscrete() == false || s.isInput() == false)
			{
				continue;
			}
			break;
		case SnapshotItemModel::TypeFilter::DiscreteOutput:
			if (s.isDiscrete() == false || s.isOutput() == false)
			{
				continue;
			}
			break;
		}

		if (m_strIdMasks.isEmpty() == false)
		{
			bool result = false;
			QString strId;
			switch (theSettings.m_signalSnapshotMaskType)
			{
			case MaskType::AppSignalId:
				{
					strId = s.appSignalId().trimmed();
				}
				break;
			case MaskType::CustomAppSignalId:
				{
					strId = s.customSignalId().trimmed();
				}
				break;
			case MaskType::EquipmentId:
				{
					strId = s.equipmentId().trimmed();
				}
				break;
			}

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

		if (currentSchemaStrId.isEmpty() == false)
		{
			bool result = false;
			QString strId = s.appSignalId().trimmed();
			for (QString appSignal : schemaAppSignals)
			{
				if (appSignal == strId)
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

	ui->tableView->sortByColumn(theSettings.m_signalSnapshotSortColumn, theSettings.m_signalSnapshotSortOrder);
}

void DialogSignalSnapshot::fillSchemas()
{
	// Fill schemas
	//
	QString currentStrId;

	QVariant data = ui->schemaCombo->currentData();
	if (data.isValid() == true)
	{
		currentStrId = data.toString();
	}

	ui->schemaCombo->clear();
	ui->schemaCombo->addItem("All Schemas", "");

	int index = 0;
	std::vector<ConfigSchema> schemasParams = m_configController->schemasParams();
	for (auto schema : schemasParams)
	{
		ui->schemaCombo->addItem(schema.strId + " - " + schema.caption, schema.strId);

		if (currentStrId == schema.strId)
		{
			ui->schemaCombo->setCurrentIndex(index);
		}

		index++;
	}
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

	theSettings.m_signalSnapshotColumns = QVector<int>::fromStdVector(m_model->columnsIndexes());
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
	const AppSignalParam& s = m_model->signalParam(hash, &found);

	if (found == false)
	{
		return;
	}

	cw->currentTab()->signalContextMenu(QStringList() << s.appSignalId());
}

void DialogSignalSnapshot::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		if (ui->buttonFixate->isChecked() == false && m_model->rowCount() > 0)
		{
			// Update only visible dynamic items
			//
			int from = ui->tableView->rowAt(0);
			int to = ui->tableView->rowAt(ui->tableView->height() - ui->tableView->horizontalHeader()->height());

			if (from == -1)
			{
				from = 0;
			}

			if (to == -1)
			{
				to = m_model->rowCount() - 1;
			}

			// Update signal states
			//
			m_model->updateStates(from, to);

			// Redraw visible table items
			//
			for (int col = 0; col < m_model->columnCount(); col++)
			{
				 int displayIndex = m_model->columnIndex(col);

				 if (displayIndex >= static_cast<int>(SnapshotItemModel::Columns::SystemTime))
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
	const AppSignalParam& s = m_model->signalParam(hash, &found);

	if (found == false)
	{
		return;
	}

	cw->currentTab()->signalInfo(s.appSignalId());
}

void DialogSignalSnapshot::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_signalSnapshotSortColumn = column;
	theSettings.m_signalSnapshotSortOrder = order;

	m_model->sort(column, order);
}

void DialogSignalSnapshot::on_typeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);

	// Get signal type
	//
	m_signalType = static_cast<SnapshotItemModel::TypeFilter>(ui->typeCombo->currentData().toInt());
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
	QMessageBox::information(this, tr("Signal Snapshot"), tr("A mask contains '*' and '?' symbols. '*' symbol means any set of symbols on its place, "
															 "'?' symbol means one symbol on ist place. Several masks separated by ';' can be entered.\r\n\r\n"
															 "Examples:\r\n\r\n#SF001P014* (mask for AppSignalId),\r\n"
															 "T?30T01? (mask for CustomAppSignalId),\r\n"
															 "#SYSTEMID_RACK01_CH01_MD?? (mask for Equipment Id)."));
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

void DialogSignalSnapshot::configController_configurationArrived(ConfigSettings /*configuration*/)
{
	fillSchemas();
	fillSignals();
}

void DialogSignalSnapshot::on_schemaCombo_currentIndexChanged(const QString&/* arg1*/)
{
	fillSignals();
}

void DialogSignalSnapshot::on_comboMaskType_currentIndexChanged(int index)
{
	theSettings.m_signalSnapshotMaskType = static_cast<MaskType>(index);

	QString mask = ui->editMask->text();
	if (mask.isEmpty() == true)
	{
		return;
	}

	fillSignals();
}
