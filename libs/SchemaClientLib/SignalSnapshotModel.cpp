#include "SignalSnapshotModel.h"
#include "../AppSignalLib/IAppSignalManager.h"
#include <AppSignalLists/SignalList.h>
#include <ClientLib/ISignalDataServer.h>

//
// SignalSnapshotSorter
//
namespace SchemaClientLib
{
	SignalSnapshotSorter::SignalSnapshotSorter(int column, SignalSnapshotModel* model) :
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

		if (index1 < 0 || index1 >= static_cast<int>(m_model->m_allSignals.size()) ||
			index2 >= static_cast<int>(m_model->m_allSignals.size()) || index1 >= static_cast<int>(m_model->m_allStates.size()) ||
			index2 >= static_cast<int>(m_model->m_allStates.size()))
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
			v1 = s1.customSignalId();
			v2 = s2.customSignalId();
			break;
		case SnapshotColumns::EquipmentID:
			v1 = s1.equipmentId();
			v2 = s2.equipmentId();
			break;
		case SnapshotColumns::LmEquipmentID:
			v1 = s1.lmEquipmentId();
			v2 = s2.lmEquipmentId();
			break;
		case SnapshotColumns::AppSignalID:
			v1 = s1.appSignalId();
			v2 = s2.appSignalId();
			break;
		case SnapshotColumns::Caption:
			v1 = s1.caption();
			v2 = s2.caption();
			break;
		case SnapshotColumns::Units:
			v1 = s1.unit();
			v2 = s2.unit();
			break;
		case SnapshotColumns::Type:
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
			break;
		case SnapshotColumns::Tags:
			v1 = s1.tagStringList().join(' ');
			v2 = s2.tagStringList().join(' ');
			break;
		case SnapshotColumns::SystemTime:
			v1 = st1.m_time.system.timeStamp;
			v2 = st2.m_time.system.timeStamp;
			break;
		case SnapshotColumns::LocalTime:
			v1 = st1.m_time.local.timeStamp;
			v2 = st2.m_time.local.timeStamp;
			break;
		case SnapshotColumns::PlantTime:
			v1 = st1.m_time.plant.timeStamp;
			v2 = st2.m_time.plant.timeStamp;
			break;
		case SnapshotColumns::Value:
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
			break;
		case SnapshotColumns::Valid:
			v1 = st1.m_flags.valid;
			v2 = st2.m_flags.valid;
			break;
		case SnapshotColumns::StateAvailable:
			v1 = st1.m_flags.stateAvailable;
			v2 = st2.m_flags.stateAvailable;
			break;
		case SnapshotColumns::Simulated:
			v1 = st1.m_flags.simulated;
			v2 = st2.m_flags.simulated;
			break;
		case SnapshotColumns::Blocked:
			v1 = st1.m_flags.blocked;
			v2 = st2.m_flags.blocked;
			break;
		case SnapshotColumns::Mismatch:
			v1 = st1.m_flags.mismatch;
			v2 = st2.m_flags.mismatch;
			break;
		case SnapshotColumns::OutOfLimits:
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
			break;
		default:
			Q_ASSERT(false);
			return index1 < index2;
		}

		if (v1.userType() != v2.userType())
		{
			Q_ASSERT(false);
			return index1 < index2;
		}

		switch (v1.userType())
		{
		case QMetaType::Bool:
			return v1.toBool() < v2.toBool();
		case QMetaType::QString:
			return v1.toString() < v2.toString();
		case QMetaType::Int:
			return v1.toInt() < v2.toInt();
		case QMetaType::UInt:
			return v1.toUInt() < v2.toUInt();
		case QMetaType::LongLong:
			return v1.toLongLong() < v2.toLongLong();
		case QMetaType::ULongLong:
			return v1.toULongLong() < v2.toULongLong();
		case QMetaType::Float:
			return v1.toFloat() < v2.toFloat();
		case QMetaType::Double:
			return v1.toDouble() < v2.toDouble();
		default:
			break;
		}

		Q_ASSERT(false);
		return index1 < index2;
	}
} // namespace SchemaClientLib

//
// SignalSnapshotModel
//
namespace SchemaClientLib
{
	SignalSnapshotModel::SignalSnapshotModel(IAppSignalManager* appSignalManager, ClientLib::ISignalDataServer* signalDataServer, AppSignalLists::AppSignalListSet* appSignalListSet, QObject* parent) :
		QAbstractItemModel(parent),
		m_appSignalManager(appSignalManager),
		m_signalDataServer(signalDataServer),
		m_appSignalListSet(appSignalListSet)
	{
		// Fill column names
		//
		m_columnsNames << QObject::tr("Signal ID");
		m_columnsNames << QObject::tr("Equipment ID");
		m_columnsNames << QObject::tr("Lm Equipment ID");
		m_columnsNames << QObject::tr("App Signal ID");
		m_columnsNames << QObject::tr("Caption");
		m_columnsNames << QObject::tr("Type");
		m_columnsNames << QObject::tr("Tags");

		m_columnsNames << QObject::tr("Server Time UTC%100").arg(QChar(0x00B1));
		m_columnsNames << QObject::tr("Server Time");
		m_columnsNames << QObject::tr("Plant Time");
		m_columnsNames << QObject::tr("Value");
		m_columnsNames << QObject::tr("Units");
		m_columnsNames << QObject::tr("Valid");
		m_columnsNames << QObject::tr("StateAvailable");
		m_columnsNames << QObject::tr("Simulated");
		m_columnsNames << QObject::tr("Blocked");
		m_columnsNames << QObject::tr("Mismatch");
		m_columnsNames << QObject::tr("OutOfLimits");

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

	QModelIndex SignalSnapshotModel::index(int row, int column, const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return createIndex(row, column);
	}

	int SignalSnapshotModel::columnCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(SnapshotColumns::ColumnCount);
	}

	int SignalSnapshotModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_filteredSignals.size());
	}

	void SignalSnapshotModel::setSignalType(SnapshotSignalType type)
	{
		m_signalType = type;
	}

	void SignalSnapshotModel::setSignalRole(SnapshotSignalRole role)
	{
		m_signalRole = role;
	}

	void SignalSnapshotModel::setMaskType(SnapshotMaskType type)
	{
		m_maskType = type;
	}

	void SignalSnapshotModel::setMasks(const QStringList& masks)
	{
		m_masks = masks;
	}

	void SignalSnapshotModel::setTags(const QStringList& tags)
	{
		m_tags = tags;
	}

	void SignalSnapshotModel::setDataServiceId(const QString& dataServiceId)
	{
		m_dataServiceId = dataServiceId;
	}

	void SignalSnapshotModel::setSchemaAppSignals(std::set<QString> schemaAppSignals)
	{
		m_schemaAppSignals = schemaAppSignals;
	}

	void SignalSnapshotModel::setAppSignalList(const QString& listId) 
	{
		m_listId = listId;
	}

	QString SignalSnapshotModel::appSignalList() const 
	{
		return m_listId;
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

		// Filter by dataServiceId
		//
		bool filterByDataServiceId = m_signalDataServer != nullptr && m_dataServiceId.isEmpty() == false;

		std::vector<Hash> appDataServiceHashes;
		if (filterByDataServiceId == true)
		{
			appDataServiceHashes = m_signalDataServer->dataServiceSignals(m_dataServiceId);
			std::sort(appDataServiceHashes.begin(), appDataServiceHashes.end());
		}

		// Get hashes list filtered by signal list
		//
		bool filterByAppSignalList = m_appSignalListSet != nullptr && m_listId.isEmpty() == false;

		std::set<Hash> appSignalListHashes;
		if (filterByAppSignalList == true)
		{
			std::shared_ptr<AppSignalLists::AppSignalList> list =  m_appSignalListSet->get(m_listId);
			if (list != nullptr) 
			{
				appSignalListHashes = list->appListHashesCache();
			}
		}

		// Fill signals
		//
		int count = static_cast<int>(m_allSignals.size());

		for (int signalIndex = 0; signalIndex < count; signalIndex++)
		{
			const AppSignalParam& s = m_allSignals[signalIndex];

			// Filter by signal list
			//
			if (filterByAppSignalList == true && appSignalListHashes.contains(s.hash()) == false)
			{
				continue;
			}

			// Filter by appDataServiceHashes
			//
			if (filterByDataServiceId == true)
			{
				if (std::binary_search(appDataServiceHashes.begin(), appDataServiceHashes.end(), s.hash()) == false)
				{
					continue;
				}
			}

			// Filter by Signal Type
			//
			if (m_signalType == SnapshotSignalType::Analog && s.isAnalog() == false)
			{
				continue;
			}

			if (m_signalType == SnapshotSignalType::Discrete && s.isDiscrete() == false)
			{
				continue;
			}

			// Filter by Signal Role
			//
			if (m_signalRole == SnapshotSignalRole::Input && s.isInput() == false)
			{
				continue;
			}

			if (m_signalRole == SnapshotSignalRole::Output && s.isOutput() == false)
			{
				continue;
			}

			if (m_signalRole == SnapshotSignalRole::Internal && s.isInternal() == false)
			{
				continue;
			}
			
			if (m_signalRole == SnapshotSignalRole::Tunable && s.enableTuning() == false)
			{
				continue;
			}

			// Filter by Mask
			//
			if (m_masks.isEmpty() == false)
			{
				bool result = false;
				QStringList strIdList;

				// Select what to analyze
				//
				switch (m_maskType)
				{
				case SnapshotMaskType::All:
					strIdList << s.appSignalId().trimmed();
					strIdList << s.customSignalId().trimmed();
					strIdList << s.equipmentId().trimmed();
					strIdList << s.lmEquipmentId().trimmed();
					break;

				case SnapshotMaskType::AppSignalId:
					strIdList << s.appSignalId().trimmed();
					break;

				case SnapshotMaskType::CustomAppSignalId:
					strIdList << s.customSignalId().trimmed();
					break;

				case SnapshotMaskType::EquipmentId:
					strIdList << s.equipmentId().trimmed();
					break;

				case SnapshotMaskType::LmEquipmentId:
					strIdList << s.lmEquipmentId().trimmed();
					break;
				}

				for (const QString& mask : m_masks)
				{
					if (mask.contains('*') == true || mask.contains('?') == true)
					{
						// Process wildcard
						//
						QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(mask.trimmed()));

						for (const QString& strId : strIdList)
						{
							if (rx.match(strId).hasMatch())
							{
								result = true;
								break;
							}
						}
					}
					else
					{
						// Process substring
						//
						for (const QString& strId : strIdList)
						{
							if (strId.contains(mask, Qt::CaseInsensitive) == true)
							{
								result = true;
								break;
							}
						}
					}

					if (result == true)
					{
						// Mask matches
						//
						break;
					}
				}

				if (result == false)
				{
					// Mask does not match
					//
					continue;
				}
			}

			// Filter by tags
			//
			if (m_tags.isEmpty() == false)
			{
				bool result = false;
				const auto& signalTags = s.tags();

				for (const QString& tag : m_tags)
				{
					if (signalTags.contains(tag) == true)
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
			//
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

			filteredSignals.push_back(signalIndex);
		}

		if (filteredSignals.empty() == false)
		{
			beginInsertRows(QModelIndex(), 0, static_cast<int>(filteredSignals.size()) - 1);

			m_filteredSignals = std::move(filteredSignals);

			insertRows(0, static_cast<int>(m_filteredSignals.size()));
			endInsertRows();
		}

		return;
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

		if (m_dataServiceId.isEmpty() == true)
		{
			m_appSignalManager->signalState(requestHashes, &requestStates, &found);
		}
		else
		{
			m_appSignalManager->signalState(requestHashes, ::calcHash(m_dataServiceId), &requestStates, &found);
		}

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

	E::AnalogFormat SignalSnapshotModel::analogFormat() const
	{
		return m_analogFormat;
	}

	void SignalSnapshotModel::setAnalogFormat(E::AnalogFormat format)
	{
		m_analogFormat = format;
	}

	int SignalSnapshotModel::analogPrecision() const
	{
		return m_analogPrecision;
	}

	void SignalSnapshotModel::setAnalogPrecision(int precision)
	{
		m_analogPrecision = precision;
	}

	QModelIndex SignalSnapshotModel::parent(const QModelIndex& index) const
	{
		Q_UNUSED(index);
		return QModelIndex();
	}

	QVariant SignalSnapshotModel::data(const QModelIndex& index, int role) const
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

			// QString str = QString("Col: %1, row: %2").arg(col).arg(row);
			// qDebug() << str;

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
					return (state.m_flags.valid == true) ? QObject::tr("") : QObject::tr("no");
				}
			case SnapshotColumns::StateAvailable:
				{
					return (state.m_flags.stateAvailable == true) ? QObject::tr("") : QObject::tr("no");
				}
			case SnapshotColumns::Simulated:
				{
					return (state.m_flags.simulated == true) ? QObject::tr("yes") : QObject::tr("");
				}
			case SnapshotColumns::Blocked:
				{
					return (state.m_flags.blocked == true) ? QObject::tr("yes") : QObject::tr("");
				}
			case SnapshotColumns::Mismatch:
				{
					return (state.m_flags.mismatch == true) ? QObject::tr("yes") : QObject::tr("");
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
						valueResult = state.toString(state.m_value,
													 E::ValueViewType::Dec,
													 m_analogFormat,
													 s.analogSignalFormat(),
													 m_analogPrecision == -1 ? s.precision() : m_analogPrecision);
						break;
					case E::SignalType::Discrete:
						valueResult = static_cast<int>(state.m_value) == 0 ? "0" : "1";
						break;
					case E::SignalType::Bus:
						valueResult = QObject::tr("Bus Type");
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

			case SnapshotColumns::LmEquipmentID:
				{
					return s.lmEquipmentId();
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
					// An array for translation
					QString signalProperties[] = {QObject::tr("Analog"), // E::SignalType
												  QObject::tr("Discrete"),
												  QObject::tr("Bus"),
												  QObject::tr("Input"),  // E::SignalInOutType
												  QObject::tr("Output"),
												  QObject::tr("Internal")};
					Q_UNUSED(signalProperties);

					QString str = QObject::tr(E::valueToString<E::SignalType>(s.type()).toUtf8());
					if (s.isAnalog())
					{
						str = QString("%1 (%2)").arg(str).arg(
							E::valueToString<E::AnalogAppSignalFormat>(static_cast<int>(s.analogSignalFormat())));
					}

					str = QString("%1, %2").arg(str).arg(QObject::tr(E::valueToString<E::SignalInOutType>(s.inOutType()).toUtf8()));

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

		if (role == Qt::TextAlignmentRole && (columnIndex == SnapshotColumns::Value || columnIndex == SnapshotColumns::Valid ||
											  columnIndex == SnapshotColumns::StateAvailable || columnIndex == SnapshotColumns::Simulated ||
											  columnIndex == SnapshotColumns::Blocked || columnIndex == SnapshotColumns::Mismatch))
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
} // namespace SchemaClientLib