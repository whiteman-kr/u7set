#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include <QFileSystemModel>
#include <QCompleter>
#include "Stable.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"

SignalSnapshotSorter::SignalSnapshotSorter(int column, SignalSnapshotModel *model):
	m_column(column),
	m_model(model)
{
}

bool SignalSnapshotSorter::sortFunction(int index1, int index2) const
{
	if (m_model == nullptr)
	{
		assert(m_model);
		return false;
	}

	if (index1 < 0
			|| index1 >= static_cast<int>(m_model->m_allSignals.size())
			|| index2 >= static_cast<int>(m_model->m_allSignals.size())
			|| index1 >= static_cast<int>(m_model->m_allStates.size())
			|| index2 >= static_cast<int>(m_model->m_allStates.size())
			)
	{
		assert(false);
		return index1 < index2;
	}

	const AppSignalParam& s1 = m_model->m_allSignals[index1];
	const AppSignalParam& s2 = m_model->m_allSignals[index2];

	const AppSignalState& st1 = m_model->m_allStates[index1];
	const AppSignalState& st2 = m_model->m_allStates[index2];

	QVariant v1;
	QVariant v2;

	switch (static_cast<SignalSnapshotModel::Columns>(m_column))
	{
	case SignalSnapshotModel::Columns::SignalID:
		{
			v1 = s1.customSignalId();
			v2 = s2.customSignalId();
		}
		break;
	case SignalSnapshotModel::Columns::EquipmentID:
		{
			v1 = s1.equipmentId();
			v2 = s2.equipmentId();
		}
		break;
	case SignalSnapshotModel::Columns::AppSignalID:
		{
			v1 = s1.appSignalId();
			v2 = s2.appSignalId();
		}
		break;
	case SignalSnapshotModel::Columns::Caption:
		{
			v1 = s1.caption();
			v2 = s2.caption();
		}
		break;
	case SignalSnapshotModel::Columns::Units:
		{
			v1 = s1.unit();
			v2 = s2.unit();
		}
		break;
	case SignalSnapshotModel::Columns::Type:
		{
			if (s1.isDiscrete() == true && s2.isDiscrete() == true)
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

	case SignalSnapshotModel::Columns::SystemTime:
		{
			v1 = st1.m_time.system.timeStamp;
			v2 = st2.m_time.system.timeStamp;
		}
		break;
	case SignalSnapshotModel::Columns::LocalTime:
		{
			v1 = st1.m_time.local.timeStamp;
			v2 = st2.m_time.local.timeStamp;
		}
		break;
	case SignalSnapshotModel::Columns::PlantTime:
		{
			v1 = st1.m_time.plant.timeStamp;
			v2 = st2.m_time.plant.timeStamp;
		}
		break;
	case SignalSnapshotModel::Columns::Value:
		{
			if (s1.isAnalog() == s2.isAnalog())
			{
				v1 = st1.m_value;
				v2 = st2.m_value;
			}
			else
			{
				v1 = s1.isAnalog();
				v2 = s2.isAnalog();
			}
		}
		break;
	case SignalSnapshotModel::Columns::Valid:
		{
			v1 = st1.m_flags.valid;
			v2 = st2.m_flags.valid;
		}
		break;
	default:
		assert(false);
		return index1 < index2;
	}

	return v1 < v2;
}

//
//SnapshotItemModel
//

SignalSnapshotModel::SignalSnapshotModel(QObject* parent)
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


	if (theSettings.m_signalSnapshotColumns.isEmpty() == true)
	{
		m_columnsIndexes.push_back(static_cast<int>(Columns::SignalID));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Caption));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Units));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Type));

		m_columnsIndexes.push_back(static_cast<int>(Columns::LocalTime));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Value));
		m_columnsIndexes.push_back(static_cast<int>(Columns::Valid));
	}
	else
	{
        std::vector<int> columnsIndexes;

        columnsIndexes.clear();
        columnsIndexes.reserve(theSettings.m_signalSnapshotColumns.size());
        columnsIndexes = theSettings.m_signalSnapshotColumns.toStdVector();

        m_columnsIndexes.clear();
        m_columnsIndexes.reserve(theSettings.m_signalSnapshotColumns.size());

        for (int c : columnsIndexes)
        {
            if (c < static_cast<int>(m_columnsNames.size()))
            {
                m_columnsIndexes.push_back(c);
            }
        }
	}

	// Copy signals to model

	m_allSignals = theSignals.signalList();
	m_allStates.resize(m_allSignals.size());

	return;
}

void SignalSnapshotModel::fillSignals()
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_filteredSignals.clear();

		endRemoveRows();
	}

	std::vector<int> filteredSignals;

	filteredSignals.reserve(m_allSignals.size());

	// Fill signals
	//

	int count = static_cast<int>(m_allSignals.size());

	for (int i = 0; i < count; i++)
	{
		const AppSignalParam& s = m_allSignals[i];

		// Filter by Signal Type

		switch (m_signalType)
		{
		case SignalSnapshotModel::SignalType::AnalogInput:
			if (s.isAnalog() == false || s.isInput() == false)
			{
				continue;
			}
			break;
		case SignalSnapshotModel::SignalType::AnalogOutput:
			if (s.isAnalog() == false || s.isOutput() == false)
			{
				continue;
			}
			break;
		case SignalSnapshotModel::SignalType::DiscreteInput:
			if (s.isDiscrete() == false || s.isInput() == false)
			{
				continue;
			}
			break;
		case SignalSnapshotModel::SignalType::DiscreteOutput:
			if (s.isDiscrete() == false || s.isOutput() == false)
			{
				continue;
			}
			break;
		}

		// Filter by Mask

		if (m_masks.isEmpty() == false)
		{
			bool result = false;

			QStringList strIdList;

			switch (theSettings.m_signalSnapshotMaskType)
			{
			case MaskType::All:
				{
					strIdList << s.appSignalId().trimmed();
					strIdList << s.customSignalId().trimmed();
					strIdList << s.equipmentId().trimmed();
				}
				break;
			case MaskType::AppSignalId:
				{
					strIdList << s.appSignalId().trimmed();
				}
				break;
			case MaskType::CustomAppSignalId:
				{
					strIdList << s.customSignalId().trimmed();
				}
				break;
			case MaskType::EquipmentId:
				{
					strIdList << s.equipmentId().trimmed();
				}
				break;
			}

			for (const QString& mask : m_masks)
			{
				QRegExp rx(mask.trimmed());
				rx.setPatternSyntax(QRegExp::Wildcard);

				for (const QString& strId : strIdList)
				{
					if (rx.exactMatch(strId))
					{
						result = true;
						break;
					}
				}

				if (result == true)
				{
					break;
				}
			}

			if (result == false)
			{
				continue;
			}
		}

		// Filter by Schema

		if (m_schemaAppSignals.empty() == false)
		{
			bool result = false;

			QString strId = s.appSignalId().trimmed();

			for (const QString& appSignal : m_schemaAppSignals)
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

		filteredSignals.push_back(i);
	}

	if (filteredSignals.empty() == false)
	{
		beginInsertRows(QModelIndex(), 0, static_cast<int>(filteredSignals.size()) - 1);

		std::swap(m_filteredSignals, filteredSignals);

		insertRows(0, static_cast<int>(m_filteredSignals.size()));

		endInsertRows();
	}

	//
}

std::vector<int> SignalSnapshotModel::columnsIndexes() const
{
	return m_columnsIndexes;
}

void SignalSnapshotModel::setColumnsIndexes(std::vector<int> columnsIndexes)
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

int SignalSnapshotModel::columnIndex(int index) const
{
	if (index <0 || index >= m_columnsIndexes.size())
	{
		assert(false);
		return -1;
	}

	return m_columnsIndexes[index];
}

QStringList SignalSnapshotModel::columnsNames() const
{
	return m_columnsNames;
}

void SignalSnapshotModel::setSignalType(SignalType type)
{
	m_signalType = type;
}

void SignalSnapshotModel::setMasks(const QStringList& masks)
{
	m_masks = masks;
}

void SignalSnapshotModel::setSchemaAppSignals(std::set<QString> schemaAppSignals)
{
	m_schemaAppSignals = schemaAppSignals;
}

void SignalSnapshotModel::updateStates(int from, int to)
{
	if (m_filteredSignals.size() == 0)
	{
		return;
	}
	if (from >= m_filteredSignals.size() || to >= m_filteredSignals.size())
	{
		assert(false);
		return;
	}

	std::vector<Hash> requestHashes;
	requestHashes.reserve(to - from);

	std::vector<AppSignalState> requestStates;
	requestStates.reserve(to - from);

	for (int i = from; i <= to; i++)
	{
		int index = m_filteredSignals[i];

		if (index < 0 || index >= static_cast<int>(m_allSignals.size()))
		{
			assert(false);
			return;
		}

		requestHashes.push_back(m_allSignals[index].hash());
	}

	int found = 0;

	theSignals.signalState(requestHashes, &requestStates, &found);

	if (requestHashes.size() != requestStates.size())
	{
		assert(false);
		return;
	}

	int state = 0;
	for (int i = from; i <= to; i++)
	{
		int index = m_filteredSignals[i];

		if (index < 0 || index >= static_cast<int>(m_allSignals.size()))
		{
			assert(false);
			return;
		}

		m_allStates[index] = requestStates[state];

		state++;
	}

	return;
}

QModelIndex SignalSnapshotModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

QModelIndex SignalSnapshotModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();

}

int SignalSnapshotModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_columnsIndexes.size());

}

int SignalSnapshotModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_filteredSignals.size());

}

AppSignalParam SignalSnapshotModel::signalParam(int rowIndex, bool* found)
{
	if (found == nullptr)
	{
		assert(found);
		return AppSignalParam();
	}

	if (rowIndex < 0 || rowIndex >= static_cast<int>(m_filteredSignals.size()))
	{
		assert(false);
		*found = false;
		return AppSignalParam();
	}

	*found = true;

	int si = m_filteredSignals[rowIndex];

	return m_allSignals[si];
}

AppSignalState SignalSnapshotModel::signalState(int rowIndex, bool* found)
{
	if (found == nullptr)
	{
		assert(found);
		return AppSignalState();
	}

	if (rowIndex < 0 || rowIndex >= static_cast<int>(m_filteredSignals.size()))
	{
		assert(false);
		*found = false;
		return AppSignalState();
	}

	*found = true;

	int si = m_filteredSignals[rowIndex];

	return m_allStates[si];
}

void SignalSnapshotModel::sort(int column, Qt::SortOrder sortOrder)
{
	if (m_filteredSignals.empty() == true)
	{
		return;
	}

	if (column < 0 || column >= m_columnsIndexes.size())
	{
		assert(false);
		return;
	}

	int sortColumn = m_columnsIndexes[column];

	std::sort(m_filteredSignals.begin(), m_filteredSignals.end(), SignalSnapshotSorter(sortColumn, this));

	if (sortOrder == Qt::DescendingOrder)
	{
		std::reverse(std::begin(m_filteredSignals), std::end(m_filteredSignals));
	}

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

	return;
}

QVariant SignalSnapshotModel::data(const QModelIndex &index, int role) const
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
		if (row >= m_filteredSignals.size())
		{
			assert(false);
			return QVariant();
		}

		Columns columnIndex = static_cast<Columns>(m_columnsIndexes[col]);

		int signalIndex = m_filteredSignals[row];

		if (signalIndex >= m_allSignals.size() || signalIndex >= m_allStates.size())
		{
			assert(false);
			return QVariant();
		}

		//QString str = QString("Col: %1, row: %2").arg(col).arg(row);
		//qDebug() << str;

		//
		// State
		//
		const AppSignalState& state = m_allStates[signalIndex];

		switch (columnIndex)
		{
		case Columns::SystemTime:
			{
				QDateTime time = state.m_time.systemToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case Columns::LocalTime:
			{
				QDateTime time = state.m_time.localToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case Columns::PlantTime:
			{
				QDateTime time = state.m_time.plantToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case Columns::Valid:
			{
				return (state.m_flags.valid == true) ? tr("") : tr("no");
			}
		}

		//
		// Get signal now
		//

		const AppSignalParam& s = m_allSignals[signalIndex];

		switch (columnIndex)
		{
		case Columns::Value:
			{
				if (state.m_flags.valid == true)
				{
					if (s.isDiscrete() == true)
					{
						return static_cast<int>(state.m_value) == 0 ? "0" : "1";
					}

					if (s.isAnalog() == true)
					{
						QString str = QString::number(state.m_value, 'f', s.precision());

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
            return QString();
		}

		return QVariant();
	} // End of if (role == Qt::DisplayRole)

	return QVariant();
}

QVariant SignalSnapshotModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section < 0 || section >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int displayIndex = m_columnsIndexes[section];

        if (displayIndex < 0 || displayIndex >= static_cast<int>(m_columnsNames.size()))
        {
            return "???";
        }

		return m_columnsNames.at(displayIndex);
	}

	return QVariant();
}

//
//DialogSignalSnapshot
//

DialogSignalSnapshot::DialogSignalSnapshot(MonitorConfigController *configController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalSnapshot),
	m_configController(configController)
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
	m_model = new SignalSnapshotModel(this);

	m_model->setSignalType(static_cast<SignalSnapshotModel::SignalType>(theSettings.m_signalSnapshotSignalType));

	// Table view setup
	//

    ui->tableView->setModel(m_model);
    ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->horizontalHeader()->setStretchLastSection(false);
	ui->tableView->setGridStyle(Qt::PenStyle::NoPen);
    ui->tableView->setSortingEnabled(true);

	int fontHeight = fontMetrics().height() + 4;

	QHeaderView *verticalHeader = ui->tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(fontHeight);

	connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogSignalSnapshot::sortIndicatorChanged);

	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSnapshot::prepareContextMenu);

	// Type combo setup
	//

	ui->typeCombo->blockSignals(true);
	ui->typeCombo->addItem(tr("All signals"), static_cast<int>(SignalSnapshotModel::SignalType::All));
	ui->typeCombo->addItem(tr("Analog Input signals"), static_cast<int>(SignalSnapshotModel::SignalType::AnalogInput));
	ui->typeCombo->addItem(tr("Analog Output signals"), static_cast<int>(SignalSnapshotModel::SignalType::AnalogOutput));
	ui->typeCombo->addItem(tr("Discrete Input signals"), static_cast<int>(SignalSnapshotModel::SignalType::DiscreteInput));
	ui->typeCombo->addItem(tr("Discrete Output signals"), static_cast<int>(SignalSnapshotModel::SignalType::DiscreteOutput));

    if (theSettings.m_signalSnapshotSignalType >= SignalSnapshotModel::SignalType::All && theSettings.m_signalSnapshotSignalType < SignalSnapshotModel::SignalType::DiscreteOutput)
    {
        ui->typeCombo->setCurrentIndex(static_cast<int>(theSettings.m_signalSnapshotSignalType));
    }
    else
    {
        ui->typeCombo->setCurrentIndex(0);
    }

	ui->typeCombo->blockSignals(false);

	// Schemas setup
	//

	fillSchemas();

	// Masks setup
	//

	m_completer = new QCompleter(theSettings.m_signalSnapshotMaskList, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);

	ui->editMask->setCompleter(m_completer);

	ui->comboMaskType->blockSignals(true);
	ui->comboMaskType->addItem("All");
	ui->comboMaskType->addItem("AppSignalID");
	ui->comboMaskType->addItem("CustomAppSignalID");
	ui->comboMaskType->addItem("EquipmentID");
    if (theSettings.m_signalSnapshotMaskType >= SignalSnapshotModel::MaskType::All && theSettings.m_signalSnapshotMaskType <= SignalSnapshotModel::MaskType::EquipmentId)
    {
        ui->comboMaskType->setCurrentIndex(static_cast<int>(theSettings.m_signalSnapshotMaskType));
    }
    else
    {
        ui->comboMaskType->setCurrentIndex(0);
    }
	ui->comboMaskType->blockSignals(false);

	connect(ui->editMask, &QLineEdit::textEdited, [this](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), ui->editMask, &QLineEdit::setText);

	//

	connect(theMonitorMainWindow, &MonitorMainWindow::signalParamAndUnitsArrived, this, &DialogSignalSnapshot::tcpSignalClient_signalParamAndUnitsArrived);
	connect(theMonitorMainWindow, &MonitorMainWindow::connectionReset, this, &DialogSignalSnapshot::tcpSignalClient_connectionReset);
	connect(m_configController, &MonitorConfigController::configurationArrived, this, &DialogSignalSnapshot::configController_configurationArrived);

	// Fill the data
	//

	fillSignals();

	ui->tableView->resizeColumnsToContents();

	m_updateStateTimerId = startTimer(500);
}

DialogSignalSnapshot::~DialogSignalSnapshot()
{
	delete ui;
}

void DialogSignalSnapshot::fillSchemas()
{
	ui->schemaCombo->blockSignals(true);

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

	std::vector<VFrame30::SchemaDetails> schemasDetails = m_configController->schemasDetails();

	for (const VFrame30::SchemaDetails& schema : schemasDetails )
	{
		ui->schemaCombo->addItem(schema.m_schemaId + " - " + schema.m_caption, schema.m_schemaId);

		if (currentStrId == schema.m_schemaId )
		{
			ui->schemaCombo->setCurrentIndex(index);
		}

		index++;
	}

	ui->schemaCombo->blockSignals(false);
}

void DialogSignalSnapshot::fillSignals()
{
	m_model->fillSignals();

	ui->tableView->sortByColumn(theSettings.m_signalSnapshotSortColumn, theSettings.m_signalSnapshotSortOrder);
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

	int rowIndex = ui->tableView->currentIndex().row();

	bool found = false;

	const AppSignalParam& s = m_model->signalParam(rowIndex, &found);

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

				 if (displayIndex >= static_cast<int>(SignalSnapshotModel::Columns::SystemTime))
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
	QString mask = ui->editMask->text();

	QStringList masks;

	if (mask.isEmpty() == false)
	{
		masks = ui->editMask->text().split(';');

		for (auto mask : masks)
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

	m_model->setMasks(masks);
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

	int rowIndex = ui->tableView->currentIndex().row();

	bool found = false;

	const AppSignalParam& s = m_model->signalParam(rowIndex, &found);

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
}

void DialogSignalSnapshot::on_typeCombo_currentIndexChanged(int index)
{
    m_model->setSignalType(static_cast<SignalSnapshotModel::SignalType>(index));
    theSettings.m_signalSnapshotSignalType = static_cast<SignalSnapshotModel::SignalType>(index);

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
//	m_signalsHashes = theSignals.signalHashes();
//	m_model->fillSignals();
//	ui->tableView->resizeColumnsToContents();
}

void DialogSignalSnapshot::tcpSignalClient_connectionReset()
{
//	m_signalsHashes.clear();
//	m_model->fillSignals();
}

void DialogSignalSnapshot::configController_configurationArrived(ConfigSettings /*configuration*/)
{
//	fillSchemas();
//	m_model->fillSignals();
}

void DialogSignalSnapshot::on_schemaCombo_currentIndexChanged(const QString&/* arg1*/)
{
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

	m_model->setSchemaAppSignals(schemaAppSignals);

	fillSignals();
}

void DialogSignalSnapshot::on_comboMaskType_currentIndexChanged(int index)
{
	theSettings.m_signalSnapshotMaskType = static_cast<SignalSnapshotModel::MaskType>(index);

	QString mask = ui->editMask->text();
	if (mask.isEmpty() == true)
	{
		return;
	}

	fillSignals();

}

