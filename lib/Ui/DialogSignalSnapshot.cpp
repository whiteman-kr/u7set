#include "DialogSignalSnapshot.h"

//
// SnapshotExportPrint
//

SnapshotExportPrint::SnapshotExportPrint(QString projectName, QString softwareEquipmentId, QWidget* parent)
	:ExportPrint(parent),
	  m_projectName(projectName),
	  m_softwareEquipmentId(softwareEquipmentId)
{

}

void SnapshotExportPrint::generateHeader(QTextCursor& cursor)
{
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
	cursor.insertText(QObject::tr("Snapshot - %1\n").arg(m_projectName));
	cursor.insertText("\n");

	cursor.setBlockFormat(regularFormat);
	cursor.setCharFormat(regularCharFormat);
	cursor.insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));
	cursor.insertText(tr("%1: %2\n").arg(qAppName()).arg(m_softwareEquipmentId));
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

	case SnapshotColumns::Tags:
	{
		v1 = s1.tagStringList().join(' ');
		v2 = s2.tagStringList().join(' ');
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

SignalSnapshotModel::SignalSnapshotModel(AppSignalManager* appSignalManager, QObject* parent)
	: QAbstractItemModel(parent),
	  m_appSignalManager(appSignalManager)
{
	// Fill column names
	//
	m_columnsNames << tr("Signal ID");
	m_columnsNames << tr("Equipment ID");
	m_columnsNames << tr("App Signal ID");
	m_columnsNames << tr("Caption");
	m_columnsNames << tr("Type");
	m_columnsNames << tr("Tags");

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

	return;
}

void SignalSnapshotModel::setSignals(std::vector<AppSignalParam>& signalList)
{
	m_allSignals.swap(signalList);
	m_allStates.resize(m_allSignals.size());
}

QStringList SignalSnapshotModel::columnsNames() const
{
	return m_columnsNames;
}

QModelIndex SignalSnapshotModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
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

void SignalSnapshotModel::setSignalType(SignalType type)
{
	m_signalType = type;
}

void SignalSnapshotModel::setMasks(const QStringList& masks)
{
	m_masks = masks;
}

void SignalSnapshotModel::setTags(const QStringList& tags)
{
	m_tags = tags;
}

void SignalSnapshotModel::setSchemaAppSignals(std::set<QString> schemaAppSignals)
{
	m_schemaAppSignals = schemaAppSignals;
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

			switch (theDialogSignalSnapshotSettings.m_signalSnapshotMaskType)
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

		// Filter by tags

		if (m_tags.isEmpty() == false)
		{
			bool result = false;

			const auto& signalTags = s.tags();

			for (const QString& tag : m_tags)
			{
				if (std::find(signalTags.begin(), signalTags.end(), tag) != signalTags.end())
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

void SignalSnapshotModel::updateStates(int from, int to)
{
	if (m_appSignalManager == nullptr)
	{
		Q_ASSERT(m_appSignalManager);
		return;
	}

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

	m_appSignalManager->signalState(requestHashes, &requestStates, &found);

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

QModelIndex SignalSnapshotModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();
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

		case SnapshotColumns::Tags:
		{
			return s.tagStringList().join(' ');
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
//DialogSignalSnapshotSettings
//

void DialogSignalSnapshotSettings::restoreSettings(QSettings& s)
{
	m_signalSnapshotPos = s.value("DialogSignalSnapshot/pos", QPoint(-1, -1)).toPoint();
	m_signalSnapshotGeometry = s.value("DialogSignalSnapshot/geometry").toByteArray();
	m_snapshotHorzHeader = s.value("DialogSignalSnapshot/horzHeader").toByteArray();
	m_snapshotHorzHeaderCount = s.value("DialogSignalSnapshot/horzHeaderCount").toInt();
	m_signalSnapshotSignalType = static_cast<SignalSnapshotModel::SignalType>(s.value("DialogSignalSnapshot/type", static_cast<int>(m_signalSnapshotSignalType)).toInt());
	m_signalSnapshotMaskList = s.value("DialogSignalSnapshot/mask").toStringList();
	m_signalSnapshotMaskType = static_cast<SignalSnapshotModel::MaskType>(s.value("DialogSignalSnapshot/maskType", static_cast<int>(m_signalSnapshotMaskType)).toInt());
	m_signalSnapshotTagsList = s.value("DialogSignalSnapshot/tags").toStringList();
	m_signalSnapshotSortColumn = s.value("DialogSignalSnapshot/sortColumn", m_signalSnapshotSortColumn).toInt();
	m_signalSnapshotSortOrder = static_cast<Qt::SortOrder>(s.value("DialogSignalSnapshot/sortOrder", m_signalSnapshotSortOrder).toInt());
}

void DialogSignalSnapshotSettings::storeSettings(QSettings& s)
{
	s.setValue("DialogSignalSnapshot/pos", m_signalSnapshotPos);
	s.setValue("DialogSignalSnapshot/geometry", m_signalSnapshotGeometry);
	s.setValue("DialogSignalSnapshot/horzHeader", m_snapshotHorzHeader);
	s.setValue("DialogSignalSnapshot/horzHeaderCount", m_snapshotHorzHeaderCount);

	s.setValue("DialogSignalSnapshot/type", static_cast<int>(m_signalSnapshotSignalType));
	s.setValue("DialogSignalSnapshot/mask", m_signalSnapshotMaskList);
	s.setValue("DialogSignalSnapshot/tags", m_signalSnapshotTagsList);

	s.setValue("DialogSignalSnapshot/maskType", static_cast<int>(m_signalSnapshotMaskType));
	s.setValue("DialogSignalSnapshot/sortColumn", m_signalSnapshotSortColumn);
	s.setValue("DialogSignalSnapshot/sortOrder", static_cast<int>(m_signalSnapshotSortOrder));
}

//
//DialogSignalSnapshot
//

DialogSignalSnapshot::DialogSignalSnapshot(AppSignalManager* appSignalManager,
										   QString projectName,
										   QString softwareEquipmentId,
										   QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_appSignalManager(appSignalManager),
	m_projectName(projectName),
	m_softwareEquipmentId(softwareEquipmentId)
{
	setupUi();

	if (m_appSignalManager == nullptr)
	{
		Q_ASSERT(m_appSignalManager);
		return;
	}

	setAttribute(Qt::WA_DeleteOnClose);

	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	// Restore window pos
	//
	if (theDialogSignalSnapshotSettings.m_signalSnapshotPos.x() != -1 && theDialogSignalSnapshotSettings.m_signalSnapshotPos.y() != -1)
	{
		move(theDialogSignalSnapshotSettings.m_signalSnapshotPos);
		restoreGeometry(theDialogSignalSnapshotSettings.m_signalSnapshotGeometry);
	}

	// crete models
	//
	m_model = new SignalSnapshotModel(m_appSignalManager, this);

	std::vector<AppSignalParam> allSignals = m_appSignalManager->signalList();
	m_model->setSignals(allSignals);

	m_model->setSignalType(static_cast<SignalSnapshotModel::SignalType>(theDialogSignalSnapshotSettings.m_signalSnapshotSignalType));

	// Table view setup
	//

	m_tableView->setModel(m_model);
	m_tableView->verticalHeader()->hide();
	m_tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_tableView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_tableView->horizontalHeader()->setStretchLastSection(false);
	m_tableView->setGridStyle(Qt::PenStyle::NoPen);
	m_tableView->setSortingEnabled(true);
	m_tableView->setWordWrap(false);

	int fontHeight = fontMetrics().height() + 4;

	QHeaderView *verticalHeader = m_tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(fontHeight);

	connect(m_tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogSignalSnapshot::on_sortIndicatorChanged);

	m_tableView->horizontalHeader()->setHighlightSections(false);
	m_tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_tableView->horizontalHeader(), &QWidget::customContextMenuRequested, this, &DialogSignalSnapshot::headerColumnContextMenuRequested);

	m_tableView->horizontalHeader()->restoreState(theDialogSignalSnapshotSettings.m_snapshotHorzHeader);

	m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tableView, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSnapshot::on_contextMenuRequested);

	if (theDialogSignalSnapshotSettings.m_snapshotHorzHeader.isEmpty() == true || theDialogSignalSnapshotSettings.m_snapshotHorzHeaderCount != static_cast<int>(SnapshotColumns::ColumnCount))
	{
		// First time? Set what is should be hidden by deafult
		//
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::EquipmentID));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::AppSignalID));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Type));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Tags));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::SystemTime));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::LocalTime));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::PlantTime));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Valid));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::StateAvailable));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Simulated));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Blocked));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::Mismatch));
		m_tableView->hideColumn(static_cast<int>(SnapshotColumns::OutOfLimits));
	}

	// Type combo setup
	//

	m_typeCombo->blockSignals(true);
	m_typeCombo->addItem(tr("All signals"), static_cast<int>(SignalSnapshotModel::SignalType::All));
	m_typeCombo->addItem(tr("Analog Input signals"), static_cast<int>(SignalSnapshotModel::SignalType::AnalogInput));
	m_typeCombo->addItem(tr("Analog Output signals"), static_cast<int>(SignalSnapshotModel::SignalType::AnalogOutput));
	m_typeCombo->addItem(tr("Discrete Input signals"), static_cast<int>(SignalSnapshotModel::SignalType::DiscreteInput));
	m_typeCombo->addItem(tr("Discrete Output signals"), static_cast<int>(SignalSnapshotModel::SignalType::DiscreteOutput));

	if (theDialogSignalSnapshotSettings.m_signalSnapshotSignalType >= SignalSnapshotModel::SignalType::All
		&& theDialogSignalSnapshotSettings.m_signalSnapshotSignalType < SignalSnapshotModel::SignalType::DiscreteOutput)
	{
		m_typeCombo->setCurrentIndex(static_cast<int>(theDialogSignalSnapshotSettings.m_signalSnapshotSignalType));
	}
	else
	{
		m_typeCombo->setCurrentIndex(0);
	}

	m_typeCombo->blockSignals(false);

	// Masks setup
	//

	m_maskCompleter = new QCompleter(theDialogSignalSnapshotSettings.m_signalSnapshotMaskList, this);
	m_maskCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_tagsCompleter = new QCompleter(theDialogSignalSnapshotSettings.m_signalSnapshotTagsList, this);
	m_tagsCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_editMask->setCompleter(m_maskCompleter);

	m_editMask->setToolTip(m_maskHelp);

	m_editTags->setCompleter(m_tagsCompleter);
	m_editTags->setToolTip(m_tagsHelp);

	m_comboMaskType->blockSignals(true);
	m_comboMaskType->addItem("All");
	m_comboMaskType->addItem("AppSignalID");
	m_comboMaskType->addItem("CustomAppSignalID");
	m_comboMaskType->addItem("EquipmentID");
	if (theDialogSignalSnapshotSettings.m_signalSnapshotMaskType >= SignalSnapshotModel::MaskType::All && theDialogSignalSnapshotSettings.m_signalSnapshotMaskType <= SignalSnapshotModel::MaskType::EquipmentId)
	{
		m_comboMaskType->setCurrentIndex(static_cast<int>(theDialogSignalSnapshotSettings.m_signalSnapshotMaskType));
	}
	else
	{
		m_comboMaskType->setCurrentIndex(0);
	}
	m_comboMaskType->blockSignals(false);

	connect(m_editMask, &QLineEdit::textEdited, [this](){m_maskCompleter->complete();});
	connect(m_maskCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_editMask, &QLineEdit::setText);

	connect(m_editTags, &QLineEdit::textEdited, [this](){m_tagsCompleter->complete();});
	connect(m_tagsCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_editTags, &QLineEdit::setText);

	m_updateStateTimerId = startTimer(500);

	return;
}

DialogSignalSnapshot::~DialogSignalSnapshot()
{
	theDialogSignalSnapshotSettings.m_snapshotHorzHeader = m_tableView->horizontalHeader()->saveState();
	theDialogSignalSnapshotSettings.m_snapshotHorzHeaderCount = static_cast<int>(SnapshotColumns::ColumnCount);
}

void DialogSignalSnapshot::refreshSchemasList()
{
	// Refresh schemas combo
	//
	fillSchemas();

	// Refresh filtered signals list
	//
	on_schemaCombo_currentIndexChanged(0);

	return;
}

void DialogSignalSnapshot::reloadSignals()
{
	bool emptyModel = m_model->rowCount() == 0;

	// Set new signals to the model
	//
	std::vector<AppSignalParam> allSignals = m_appSignalManager->signalList();
	m_model->setSignals(allSignals);

	// Refresh signals list
	//

	fillSignals();

	if (emptyModel == true)
	{

		m_tableView->resizeColumnsToContents();
	}

	return;
}

void DialogSignalSnapshot::fillSchemas()
{
	m_schemaCombo->blockSignals(true);

	// Fill schemas
	//
	QString currentStrId;

	QVariant data = m_schemaCombo->currentData();
	if (data.isValid() == true)
	{
		currentStrId = data.toString();
	}

	m_schemaCombo->clear();

	m_schemaCombo->addItem("All Schemas", "");

	int selectedIndex = -1;

	std::vector<VFrame30::SchemaDetails> details = schemasDetails();

	for (const VFrame30::SchemaDetails& schema : details )
	{
		m_schemaCombo->addItem(schema.m_schemaId + " - " + schema.m_caption, schema.m_schemaId);

		if (currentStrId == schema.m_schemaId )
		{
			selectedIndex = m_schemaCombo->count() - 1;
		}
	}

	if (selectedIndex != -1)
	{
		m_schemaCombo->setCurrentIndex(selectedIndex);
	}

	m_schemaCombo->blockSignals(false);
}

void DialogSignalSnapshot::fillSignals()
{
	bool modelWasEmpty = m_model->rowCount() == 0;

	m_model->fillSignals();

	m_tableView->sortByColumn(theDialogSignalSnapshotSettings.m_signalSnapshotSortColumn, theDialogSignalSnapshotSettings.m_signalSnapshotSortOrder);

	if (modelWasEmpty == true)
	{
		m_tableView->resizeColumnsToContents();
	}
}

/*
std::vector<VFrame30::SchemaDetails> DialogSignalSnapshot::schemasDetails()
{
	Q_ASSERT(false);
	return std::vector<VFrame30::SchemaDetails>();
}

std::set<QString> DialogSignalSnapshot::schemaAppSignals(const QString& schemaStrId)
{
	Q_ASSERT(false);
	Q_UNUSED(schemaStrId);
	return std::set<QString>();
}*/

void DialogSignalSnapshot::headerColumnContextMenuRequested(const QPoint& pos)
{
	QMenu menu(this);

	QList<QAction*> actions;

	std::vector<std::pair<SnapshotColumns, QString>> actionsData;
	actionsData.reserve(static_cast<int>(SnapshotColumns::ColumnCount));

	SignalSnapshotModel* model = dynamic_cast<SignalSnapshotModel*>(m_tableView->model());
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
		QAction* action = new QAction(ad.second, this);
		action->setData(QVariant::fromValue(ad.first));
		action->setCheckable(true);
		action->setChecked(!m_tableView->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

		if (m_tableView->horizontalHeader()->count() - m_tableView->horizontalHeader()->hiddenSectionCount() == 1 &&
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
		m_tableView->showColumn(column);
	}
	else
	{
		m_tableView->hideColumn(column);
	}

	return;
}



void DialogSignalSnapshot::on_DialogSignalSnapshot_finished(int result)
{
	Q_UNUSED(result);

	// Save window position
	//
	theDialogSignalSnapshotSettings.m_signalSnapshotPos = pos();
	theDialogSignalSnapshotSettings.m_signalSnapshotGeometry = saveGeometry();

}

void DialogSignalSnapshot::on_contextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	int row = m_tableView->currentIndex().row();
	if (row == -1)
	{
		return;
	}

	int rowIndex = m_tableView->currentIndex().row();

	bool found = false;

	const AppSignalParam& s = m_model->signalParam(rowIndex, &found);

	if (found == false)
	{
		return;
	}

	emit signalContextMenu(s.appSignalId());
}

void DialogSignalSnapshot::setupUi()
{
	QGroupBox* groupBox = new QGroupBox(tr("Filter"));

	//typeLayout

	QGridLayout* typeLayout = new QGridLayout();
	typeLayout->setContentsMargins(1, 1, 1, 1);

	typeLayout->addWidget(new QLabel(tr("Signal Type")), 0, 0);

	m_typeCombo = new QComboBox();
	connect(m_typeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogSignalSnapshot::on_typeCombo_currentIndexChanged);
	typeLayout->addWidget(m_typeCombo, 0, 1);

	typeLayout->addWidget(new QLabel(tr("Schema")), 1, 0);

	m_schemaCombo = new QComboBox();
	connect(m_schemaCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogSignalSnapshot::on_schemaCombo_currentIndexChanged);
	typeLayout->addWidget(m_schemaCombo, 1, 1);

	//maskLayout

	QHBoxLayout* maskLayout = new QHBoxLayout();
	maskLayout->setContentsMargins(0, 0, 0, 0);

	maskLayout->addWidget(new QLabel(tr("Mask")));

	m_comboMaskType = new QComboBox();
	connect(m_comboMaskType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogSignalSnapshot::on_comboMaskType_currentIndexChanged);
	maskLayout->addWidget(m_comboMaskType);

	m_editMask = new QLineEdit();
	connect(m_editMask, &QLineEdit::returnPressed, this, &DialogSignalSnapshot::on_editMask_returnPressed);
	maskLayout->addWidget(m_editMask);

	//tagsLayout

	QHBoxLayout* tagsLayout = new QHBoxLayout();
	tagsLayout->setContentsMargins(0, 0, 0, 0);

	tagsLayout->addWidget(new QLabel(tr("Tags")));

	m_editTags = new QLineEdit();
	connect(m_editTags, &QLineEdit::returnPressed, this, &DialogSignalSnapshot::on_editTags_returnPressed);
	tagsLayout->addWidget(m_editTags);

	//maskTagsLayout

	QVBoxLayout* maskTagsLayout = new QVBoxLayout();
	maskTagsLayout->setContentsMargins(1, 1, 1, 1);
	maskTagsLayout->addLayout(maskLayout);
	maskTagsLayout->addLayout(tagsLayout);

	//groupLayout

	QHBoxLayout* groupLayout = new QHBoxLayout(groupBox);
	groupLayout->setContentsMargins(1, 1, 1, 1);
	groupLayout->addLayout(typeLayout);
	groupLayout->addLayout(maskTagsLayout);

	// Export/Print/Fixate

	QHBoxLayout* exPrintLayout = new QHBoxLayout();
	exPrintLayout->setContentsMargins(1, 1, 1, 1);

	QPushButton* b = new QPushButton(tr("Export..."));
	connect(b, &QPushButton::clicked, this, &DialogSignalSnapshot::on_buttonExport_clicked);
	exPrintLayout->addWidget(b);

	b = new QPushButton(tr("Print..."));
	connect(b, &QPushButton::clicked, this, &DialogSignalSnapshot::on_buttonPrint_clicked);
	exPrintLayout->addWidget(b);

	exPrintLayout->addStretch();

	m_buttonFixate = new QPushButton(tr("Fixate"));
	m_buttonFixate->setCheckable(true);
	exPrintLayout->addWidget(m_buttonFixate);

	// Table

	m_tableView = new QTableView();
	connect(m_tableView, &QTableView::doubleClicked, this, &DialogSignalSnapshot::on_tableView_doubleClicked);

	// Main layout

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(1, 1, 1, 1);

	mainLayout->addWidget(groupBox);
	mainLayout->addLayout(exPrintLayout);
	mainLayout->addWidget(m_tableView);

	setLayout(mainLayout);

}

void DialogSignalSnapshot::timerEvent(QTimerEvent* event)
{
	Q_ASSERT(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		if (m_buttonFixate->isChecked() == false && m_model->rowCount() > 0)
		{
			// Update only visible dynamic items
			//
			int from = m_tableView->rowAt(0);

			int to = m_tableView->rowAt(m_tableView->height() - m_tableView->horizontalHeader()->height());

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
						m_tableView->update(m_model->index(row, col));
					}
				}
			}
		}
	}
}

void DialogSignalSnapshot::maskChanged()
{
	QString maskText = m_editMask->text().trimmed();

	maskText.replace(' ', ';');

	QStringList masks;

	if (maskText.isEmpty() == false)
	{
		masks = maskText.split(';', QString::SkipEmptyParts);

		for (auto mask : masks)
		{
			// Save filter history
			//
			if (theDialogSignalSnapshotSettings.m_signalSnapshotMaskList.contains(mask) == false)
			{
				theDialogSignalSnapshotSettings.m_signalSnapshotMaskList.append(mask);

				QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_maskCompleter->model());
				if (completerModel == nullptr)
				{
					Q_ASSERT(completerModel);
					return;
				}

				completerModel->setStringList(theDialogSignalSnapshotSettings.m_signalSnapshotMaskList);
			}
		}
	}

	m_model->setMasks(masks);
}

void DialogSignalSnapshot::tagsChanged()
{
	QString tagsText = m_editTags->text().trimmed();
	tagsText.replace(' ', ';');

	QStringList tags;

	if (tagsText.isEmpty() == false)
	{
		tags = tagsText.split(';', QString::SkipEmptyParts);

		for (auto tag : tags)
		{
			// Save filter history
			//
			if (theDialogSignalSnapshotSettings.m_signalSnapshotTagsList.contains(tag) == false)
			{
				theDialogSignalSnapshotSettings.m_signalSnapshotTagsList.append(tag);

				QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_tagsCompleter->model());
				if (completerModel == nullptr)
				{
					Q_ASSERT(completerModel);
					return;
				}

				completerModel->setStringList(theDialogSignalSnapshotSettings.m_signalSnapshotTagsList);
			}
		}
	}

	m_model->setTags(tags);
}

void DialogSignalSnapshot::on_tableView_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	int row = m_tableView->currentIndex().row();
	if (row == -1)
	{
		return;
	}

	int rowIndex = m_tableView->currentIndex().row();

	bool found = false;

	const AppSignalParam& s = m_model->signalParam(rowIndex, &found);

	if (found == false)
	{
		return;
	}

	QTimer::singleShot(10, [this, s] {
		emit signalInfo(s.appSignalId());
	});
}

void DialogSignalSnapshot::on_sortIndicatorChanged(int column, Qt::SortOrder order)
{
	theDialogSignalSnapshotSettings.m_signalSnapshotSortColumn = column;
	theDialogSignalSnapshotSettings.m_signalSnapshotSortOrder = order;
}

void DialogSignalSnapshot::on_typeCombo_currentIndexChanged(int index)
{
	m_model->setSignalType(static_cast<SignalSnapshotModel::SignalType>(index));
	theDialogSignalSnapshotSettings.m_signalSnapshotSignalType = static_cast<SignalSnapshotModel::SignalType>(index);

	fillSignals();
}

void DialogSignalSnapshot::on_editMask_returnPressed()
{
	maskChanged();

	fillSignals();
}

void DialogSignalSnapshot::on_editTags_returnPressed()
{
	tagsChanged();

	fillSignals();
}

void DialogSignalSnapshot::on_schemaCombo_currentIndexChanged(int /*index)*/)
{
	// Get current schema's App Signals
	//

	QString currentSchemaStrId;
	QVariant data = m_schemaCombo->currentData();
	if (data.isValid() == true)
	{
		currentSchemaStrId = data.toString();
	}

	std::set<QString> appSignals = schemaAppSignals(currentSchemaStrId);

	m_model->setSchemaAppSignals(appSignals);

	fillSignals();
}

void DialogSignalSnapshot::on_comboMaskType_currentIndexChanged(int index)
{
	theDialogSignalSnapshotSettings.m_signalSnapshotMaskType = static_cast<SignalSnapshotModel::MaskType>(index);

	QString mask = m_editMask->text();
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
			extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
	{
		SnapshotExportPrint ep(m_projectName, m_softwareEquipmentId, this);
		ep.exportTable(m_tableView, fileName, extension);

		return;
	}

	QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
	return;
}

void DialogSignalSnapshot::on_buttonPrint_clicked()
{
	SnapshotExportPrint ep(m_projectName, m_softwareEquipmentId, this);
	ep.printTable(m_tableView);
}

DialogSignalSnapshotSettings theDialogSignalSnapshotSettings;

const QString DialogSignalSnapshot::m_maskHelp = QObject::tr("A mask contains '*' and '?' symbols.\n\
'*' symbol means any set of symbols on its place, '?' symbol means one symbol on its place.\n\
Several masks can be separated by semicolon or space.\n\n\
Examples:\n\n\
#SF001P014* (mask for AppSignalID),\n\
T?30T01? (mask for CustomAppSignalID),\n\
#SYSTEMID_RACK01_CH01_MD?? (mask for Equipment ID).\n\n\
To apply the filter, enter the mask and press Enter.");

const QString DialogSignalSnapshot::m_tagsHelp = QObject::tr("Tags for filtering signals.\n\n\
Several tags can be separated by semicolon or space: \"tag1; tag2\" or \"tag1 tag2\".\n\n\
To apply the filter, enter tags and press Enter.");
