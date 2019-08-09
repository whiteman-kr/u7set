#include "DialogSignalSnapshot.h"
#include "ui_DialogSignalSnapshot.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "TcpSignalClient.h"

//
// MonitorExportPrint
//

SnapshotExportPrint::SnapshotExportPrint(ConfigSettings* configuration, QWidget* parent)
	:ExportPrint(parent),
	  m_configuration(configuration)
{

}

void SnapshotExportPrint::generateHeader(QTextCursor& cursor)
{
	if (m_configuration == nullptr)
	{
		Q_ASSERT(m_configuration);
		return;
	}

	QTextBlockFormat headerCenterFormat = cursor.blockFormat();
	headerCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat regularFormat = cursor.blockFormat();
	regularFormat.setAlignment(Qt::AlignLeft);

	QTextCharFormat headerCharFormat = cursor.charFormat();
	headerCharFormat.setFontWeight(static_cast<int>(QFont::Bold));
	headerCharFormat.setFontPointSize(12.0);

	QTextCharFormat regularCharFormat = cursor.charFormat();
	headerCharFormat.setFontPointSize(10.0);

	cursor.setBlockFormat(headerCenterFormat);
	cursor.setCharFormat(headerCharFormat);
	cursor.insertText(QObject::tr("Snapshot - %1\n").arg(m_configuration->project));
	cursor.insertText("\n");

	cursor.setBlockFormat(regularFormat);
	cursor.setCharFormat(regularCharFormat);
	cursor.insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));
	cursor.insertText(tr("Monitor: %1\n").arg(m_configuration->softwareEquipmentId));
	cursor.insertText("\n");

	cursor.insertText("\n");
}

//
// SignalSnapshotSorter
//

SignalSnapshotSorter::SignalSnapshotSorter(int column, SignalSnapshotModel *model):
	m_column(column),
	m_model(model)
{
}

bool SignalSnapshotSorter::sortFunction(int index1, int index2) const
{
	if (m_model == nullptr)
	{
		Q_ASSERT(m_model);
		return false;
	}

	if (index1 < 0
			|| index1 >= static_cast<int>(m_model->m_allSignals.size())
			|| index2 >= static_cast<int>(m_model->m_allSignals.size())
			|| index1 >= static_cast<int>(m_model->m_allStates.size())
			|| index2 >= static_cast<int>(m_model->m_allStates.size())
			)
	{
		Q_ASSERT(false);
		return index1 < index2;
	}

	const AppSignalParam& s1 = m_model->m_allSignals[index1];
	const AppSignalParam& s2 = m_model->m_allSignals[index2];

	const AppSignalState& st1 = m_model->m_allStates[index1];
	const AppSignalState& st2 = m_model->m_allStates[index2];

	QVariant v1;
	QVariant v2;

	switch (static_cast<SnapshotColumns>(m_column))
	{
	case SnapshotColumns::SignalID:
		{
			v1 = s1.customSignalId();
			v2 = s2.customSignalId();
		}
		break;
	case SnapshotColumns::EquipmentID:
		{
			v1 = s1.equipmentId();
			v2 = s2.equipmentId();
		}
		break;
	case SnapshotColumns::AppSignalID:
		{
			v1 = s1.appSignalId();
			v2 = s2.appSignalId();
		}
		break;
	case SnapshotColumns::Caption:
		{
			v1 = s1.caption();
			v2 = s2.caption();
		}
		break;
	case SnapshotColumns::Units:
		{
			v1 = s1.unit();
			v2 = s2.unit();
		}
		break;
	case SnapshotColumns::Type:
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

	case SnapshotColumns::SystemTime:
		{
			v1 = st1.m_time.system.timeStamp;
			v2 = st2.m_time.system.timeStamp;
		}
		break;
	case SnapshotColumns::LocalTime:
		{
			v1 = st1.m_time.local.timeStamp;
			v2 = st2.m_time.local.timeStamp;
		}
		break;
	case SnapshotColumns::PlantTime:
		{
			v1 = st1.m_time.plant.timeStamp;
			v2 = st2.m_time.plant.timeStamp;
		}
		break;
	case SnapshotColumns::Value:
		{
			if (st1.m_flags.valid != st2.m_flags.valid)
			{
				v1 = st1.m_flags.valid;
				v2 = st2.m_flags.valid;
			}
			else
			{
				if (st1.m_flags.stateAvailable != st2.m_flags.stateAvailable)
				{
					v1 = st1.m_flags.stateAvailable;
					v2 = st2.m_flags.stateAvailable;
				}
				else
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
			}
		}
		break;
	case SnapshotColumns::Valid:
		{
			v1 = st1.m_flags.valid;
			v2 = st2.m_flags.valid;
		}
		break;
	case SnapshotColumns::StateAvailable:
		{
			v1 = st1.m_flags.stateAvailable;
			v2 = st2.m_flags.stateAvailable;
		}
		break;
	case SnapshotColumns::Simulated:
		{
			v1 = st1.m_flags.simulated;
			v2 = st2.m_flags.simulated;
		}
		break;
	case SnapshotColumns::Blocked:
		{
			v1 = st1.m_flags.blocked;
			v2 = st2.m_flags.blocked;
		}
		break;
	case SnapshotColumns::Mismatch:
		{
			v1 = st1.m_flags.mismatch;
			v2 = st2.m_flags.mismatch;
		}
		break;
	case SnapshotColumns::OutOfLimits:
		{
			if (st1.m_flags.belowLowLimit == st2.m_flags.belowLowLimit)
			{
				v1 = st1.m_flags.aboveHighLimit;
				v2 = st2.m_flags.aboveHighLimit;
			}
			else
			{
				v1 = st1.m_flags.belowLowLimit;
				v2 = st2.m_flags.belowLowLimit;
			}

		}
		break;
	default:
		Q_ASSERT(false);
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
	m_columnsNames << tr("Type");

	m_columnsNames << tr("Server Time UTC%100").arg(QChar(0x00B1));
	m_columnsNames << tr("Server Time");
	m_columnsNames << tr("Plant Time");
	m_columnsNames << tr("Value");
	m_columnsNames << tr("Units");
	m_columnsNames << tr("Valid");
	m_columnsNames << tr("StateAvailable");
	m_columnsNames << tr("Simulated");
	m_columnsNames << tr("Blocked");
	m_columnsNames << tr("Mismatch");
	m_columnsNames << tr("OutOfLimits");

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
		Q_ASSERT(false);
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
			Q_ASSERT(false);
			return;
		}

		requestHashes.push_back(m_allSignals[index].hash());
	}

	int found = 0;

	theSignals.signalState(requestHashes, &requestStates, &found);

	if (requestHashes.size() != requestStates.size())
	{
		Q_ASSERT(false);
		return;
	}

	int state = 0;
	for (int i = from; i <= to; i++)
	{
		int index = m_filteredSignals[i];

		if (index < 0 || index >= static_cast<int>(m_allSignals.size()))
		{
			Q_ASSERT(false);
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
	return static_cast<int>(SnapshotColumns::ColumnCount);

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
		Q_ASSERT(found);
		return AppSignalParam();
	}

	if (rowIndex < 0 || rowIndex >= static_cast<int>(m_filteredSignals.size()))
	{
		Q_ASSERT(false);
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
		Q_ASSERT(found);
		return AppSignalState();
	}

	if (rowIndex < 0 || rowIndex >= static_cast<int>(m_filteredSignals.size()))
	{
		Q_ASSERT(false);
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

	updateStates(0, static_cast<int>(m_filteredSignals.size() - 1));

	int sortColumn = column;

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
	int col = index.column();
	if (col < 0 || col >= static_cast<int>(SnapshotColumns::ColumnCount))
	{
		Q_ASSERT(false);
		return QVariant();
	}

	int row = index.row();
	if (row >= m_filteredSignals.size())
	{
		Q_ASSERT(false);
		return QVariant();
	}

	SnapshotColumns columnIndex = static_cast<SnapshotColumns>(col);

	if (role == Qt::DisplayRole)
	{
		int signalIndex = m_filteredSignals[row];

		if (signalIndex >= m_allSignals.size() || signalIndex >= m_allStates.size())
		{
			Q_ASSERT(false);
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
		case SnapshotColumns::SystemTime:
			{
				QDateTime time = state.m_time.systemToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SnapshotColumns::LocalTime:
			{
				QDateTime time = state.m_time.localToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SnapshotColumns::PlantTime:
			{
				QDateTime time = state.m_time.plantToDateTime();
				return time.toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SnapshotColumns::Valid:
			{
				return (state.m_flags.valid == true) ? tr("") : tr("no");
			}
		case SnapshotColumns::StateAvailable:
			{
				return (state.m_flags.stateAvailable == true) ? tr("") : tr("no");
			}
		case SnapshotColumns::Simulated:
			{
				return (state.m_flags.simulated == true) ? tr("yes") : tr("");
			}
		case SnapshotColumns::Blocked:
			{
				return (state.m_flags.blocked == true) ? tr("yes") : tr("");
			}
		case SnapshotColumns::Mismatch:
			{
				return (state.m_flags.mismatch == true) ? tr("yes") : tr("");
			}
		case SnapshotColumns::OutOfLimits:
			{
				QString resultString;

				if (state.m_flags.belowLowLimit == true)
				{
					resultString += QStringLiteral("LOW ");
				}
				if (state.m_flags.aboveHighLimit == true)
				{
					resultString += QStringLiteral("HIGH ");
				}
				return resultString.trimmed();
			}
		}

		//
		// Get signal now
		//

		const AppSignalParam& s = m_allSignals[signalIndex];

		switch (columnIndex)
		{
		case SnapshotColumns::Value:
			{
				QString valueResult;

				switch (s.type())
				{
				case E::SignalType::Analog:
					valueResult = state.toString(state.m_value, E::ValueViewType::Dec, s.precision());
					break;
				case E::SignalType::Discrete:
					valueResult = static_cast<int>(state.m_value) == 0 ? "0" : "1";
					break;
				case E::SignalType::Bus:
					valueResult = tr("Bus Type");
					break;
				default:
					Q_ASSERT(false);
				}

				if (state.m_flags.valid == false)
				{
					if (state.m_flags.stateAvailable == true)
					{
						valueResult = QString("? (%1)").arg(valueResult);
					}
					else
					{
						valueResult = QStringLiteral("?");
					}
				}

				return valueResult;
			}

		case SnapshotColumns::SignalID:
			{
				return s.customSignalId();
			}

		case SnapshotColumns::EquipmentID:
			{
				return s.equipmentId();
			}

		case SnapshotColumns::AppSignalID:
			{
				return s.appSignalId();
			}

		case SnapshotColumns::Caption:
			{
				return s.caption();
			}

		case SnapshotColumns::Units:
			{
				return s.unit();
			}

		case SnapshotColumns::Type:
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

		// return QVariant();	Unreachable
	} // End of if (role == Qt::DisplayRole)

	if (role == Qt::TextAlignmentRole &&
		 (columnIndex ==  SnapshotColumns::Value ||
		 columnIndex ==  SnapshotColumns::Valid ||
		 columnIndex ==  SnapshotColumns::StateAvailable ||
		 columnIndex ==  SnapshotColumns::Simulated ||
		 columnIndex ==  SnapshotColumns::Blocked ||
		 columnIndex ==  SnapshotColumns::Mismatch))
	{
		return QVariant(Qt::AlignCenter);
	}

	return QVariant();
}

QVariant SignalSnapshotModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section < 0 || section >= static_cast<int>(SnapshotColumns::ColumnCount))
		{
			Q_ASSERT(false);
			return QVariant();
		}

		if (section < 0 || section >= static_cast<int>(m_columnsNames.size()))
        {
            return "???";
        }

		return m_columnsNames.at(section);
	}

	return QVariant();
}

//
//DialogSignalSnapshot
//

DialogSignalSnapshot::DialogSignalSnapshot(MonitorConfigController *configController, TcpSignalClient* tcpSignalClient, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalSnapshot),
	m_configController(configController),
	m_tcpSignalClient(tcpSignalClient),
	m_configuration(configController->configuration())
{
	ui->setupUi(this);

	if (m_configController == nullptr || m_tcpSignalClient == nullptr)
	{
		Q_ASSERT(m_configController);
		Q_ASSERT(m_tcpSignalClient);
		return;
	}

	setAttribute(Qt::WA_DeleteOnClose);

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
	ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->horizontalHeader()->setStretchLastSection(false);
	ui->tableView->setGridStyle(Qt::PenStyle::NoPen);
    ui->tableView->setSortingEnabled(true);
	ui->tableView->setWordWrap(false);

	int fontHeight = fontMetrics().height() + 4;

	QHeaderView *verticalHeader = ui->tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(fontHeight);

	connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogSignalSnapshot::sortIndicatorChanged);

	ui->tableView->horizontalHeader()->setHighlightSections(false);
	ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui->tableView->horizontalHeader(), &QWidget::customContextMenuRequested, this, &DialogSignalSnapshot::headerColumnContextMenuRequested);

	ui->tableView->horizontalHeader()->restoreState(theSettings.m_snapshotHorzHeader);

	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSnapshot::contextMenuRequested);

	if (theSettings.m_snapshotHorzHeader.isEmpty() == true || theSettings.m_snapshotHorzHeaderCount != static_cast<int>(SnapshotColumns::ColumnCount))
	{
		// First time? Set what is should be hidden by deafult
		//
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::EquipmentID));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::AppSignalID));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::Type));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::SystemTime));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::LocalTime));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::PlantTime));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::Valid));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::StateAvailable));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::Simulated));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::Blocked));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::Mismatch));
		ui->tableView->hideColumn(static_cast<int>(SnapshotColumns::OutOfLimits));
	}

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

	connect(m_tcpSignalClient, &TcpSignalClient::signalParamAndUnitsArrived, this, &DialogSignalSnapshot::tcpSignalClient_signalParamAndUnitsArrived);
	connect(m_tcpSignalClient, &TcpSignalClient::connectionReset, this, &DialogSignalSnapshot::tcpSignalClient_connectionReset);
	connect(m_configController, &MonitorConfigController::configurationArrived, this, &DialogSignalSnapshot::configController_configurationArrived);

	// Fill the data
	//

	fillSignals();

	ui->tableView->resizeColumnsToContents();

	m_updateStateTimerId = startTimer(500);
}

DialogSignalSnapshot::~DialogSignalSnapshot()
{
	theSettings.m_snapshotHorzHeader = ui->tableView->horizontalHeader()->saveState();
	theSettings.m_snapshotHorzHeaderCount = static_cast<int>(SnapshotColumns::ColumnCount);

	delete ui;
}

void DialogSignalSnapshot::headerColumnContextMenuRequested(const QPoint& pos)
{
	QMenu menu(this);

	QList<QAction*> actions;

	std::vector<std::pair<SnapshotColumns, QString>> actionsData;
	actionsData.reserve(static_cast<int>(SnapshotColumns::ColumnCount));

	SignalSnapshotModel* model = dynamic_cast<SignalSnapshotModel*>(ui->tableView->model());
	if (model == nullptr)
	{
		Q_ASSERT(model);
		return;
	}

	QStringList columns = model->columnsNames();

	for(int i = 0; i < columns.size(); i++)
	{
		actionsData.emplace_back(static_cast<SnapshotColumns>(i), columns[i]);
	}

	for (std::pair<SnapshotColumns, QString> ad : actionsData)
	{
		QAction* action = new QAction(ad.second, nullptr);
		action->setData(QVariant::fromValue(ad.first));
		action->setCheckable(true);
		action->setChecked(!ui->tableView->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

		if (ui->tableView->horizontalHeader()->count() - ui->tableView->horizontalHeader()->hiddenSectionCount() == 1 &&
			action->isChecked() == true)
		{
			action->setEnabled(false);			// Impossible to uncheck the last column
		}

		connect(action, &QAction::toggled, this, &DialogSignalSnapshot::headerColumnToggled);

		actions << action;
	}

	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}

void DialogSignalSnapshot::headerColumnToggled(bool checked)
{
	QAction* action = dynamic_cast<QAction*>(sender());

	if (action == nullptr)
	{
		Q_ASSERT(action);
		return ;
	}

	int column = action->data().value<int>();

	if (column >= static_cast<int>(SnapshotColumns::ColumnCount))
	{
		Q_ASSERT(column < static_cast<int>(SnapshotColumns::ColumnCount));
		return;
	}

	if (checked == true)
	{
		ui->tableView->showColumn(column);
	}
	else
	{
		ui->tableView->hideColumn(column);
	}

	return;
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

void DialogSignalSnapshot::on_DialogSignalSnapshot_finished(int result)
{
	Q_UNUSED(result);

	// Save window position
	//
	theSettings.m_signalSnapshotPos = pos();
	theSettings.m_signalSnapshotGeometry = saveGeometry();

}

void DialogSignalSnapshot::contextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	if (theMonitorMainWindow == nullptr)
	{
		Q_ASSERT(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		Q_ASSERT(cw);
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
	Q_ASSERT(event);

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
				 if (col >= static_cast<int>(SnapshotColumns::SystemTime))
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
	QString maskText = ui->editMask->text();
	QStringList masks;

	if (maskText.isEmpty() == false)
	{
		masks = maskText.split(';');

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
					Q_ASSERT(model);
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
		Q_ASSERT(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		Q_ASSERT(cw);
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

	QTimer::singleShot(10, [cw, s] {
		  cw->currentTab()->signalInfo(s.appSignalId());
	  });
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

void DialogSignalSnapshot::on_buttonExport_clicked()
{
	Q_ASSERT(m_model);
	if (m_model->rowCount() == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("Nothing to export."));
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													"untitled.pdf",
													tr("Portable Documnet Format (*.pdf);;CSV Files, semicolon separated (*.csv);;Plaintext (*.txt);;HTML (*.html)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}

	QFileInfo fileInfo(fileName);
	QString extension = fileInfo.completeSuffix();

	if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0/* ||
		extension.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0*/)
	{
		SnapshotExportPrint ep(&m_configuration, this);
		ep.exportTable(ui->tableView, fileName, extension);

		return;
	}

	QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
	return;
}

void DialogSignalSnapshot::on_buttonPrint_clicked()
{
	SnapshotExportPrint ep(&m_configuration, this);
	ep.printTable(ui->tableView);
}
