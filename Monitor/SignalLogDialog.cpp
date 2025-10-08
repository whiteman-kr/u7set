#include "SignalLogDialog.h"
#include "../../AppSignalLib/IAppSignalManager.h"
#include "../libs/UiLib/include/UiLib/StandardColors.h"
#include <AppSignalLists/SignalList.h>
#include <ClientLib/SignalLog.h>
#include <ReportLib/ReportObject.h>
#include <ReportLib/TableViewReportGenerator.h>
#include <UiLib/ChooseItemsWidget.h>
#include "../UtilsLib/Ui/UiTools.h"

#include "Globals.h"
#include "MonitorMainWindow.h"
#include "MonitorSignalInfo.h"

//
// SignalLogReportGenerator
//
namespace
{
	class SignalLogReportGenerator : public ReportLib::ITableViewReportInfo
	{
	public:
		SignalLogReportGenerator(const QString& projectName, const QString& softwareEquipmentId);

	protected:
		virtual void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const override;

	private:
		QString m_projectName;
		QString m_equipmentId;
	};

	//
	// SignalLogReportGenerator
	//
	SignalLogReportGenerator::SignalLogReportGenerator(const QString& projectName, const QString& softwareEquipmentId) :
		m_projectName(projectName),
		m_equipmentId(softwareEquipmentId)
	{
	}

	void SignalLogReportGenerator::generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const
	{
		ReportLib::ReportFont marginFont{"Arial", 10};

		report.addMarginItem({QObject::tr("Generated: %1").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss")),
							  -1,
							  -1,
							  {marginFont, Qt::AlignLeft | Qt::AlignTop}});

		report.addMarginItem({QObject::tr("Signals Log"), -1, -1, {marginFont, Qt::AlignCenter | Qt::AlignTop}});

		report.addMarginItem({QObject::tr("Project: %1").arg(m_projectName), -1, -1, {marginFont, Qt::AlignRight | Qt::AlignTop}});

		report.addMarginItem(
			{QObject::tr("%1: %2").arg(qAppName()).arg(m_equipmentId), -1, -1, {marginFont, Qt::AlignLeft | Qt::AlignBottom}});

		report.addMarginItem({"%PAGE%", -1, -1, {marginFont, Qt::AlignRight | Qt::AlignBottom}});

		ReportLib::ReportFont textFont{"Arial", 12};
		mainSection.addText(" \n", {textFont, Qt::AlignLeft});
	}

	//
	// SignalLogSorter
	//
	class SignalLogSorter
	{
	public:
		SignalLogSorter(int column, const SignalLogModel* model, const IAppSignalManager* appSignalManager);

		bool operator()(const RecordKey& key1, const RecordKey& key2) const { return sortFunction(key1, key2); }

		bool sortFunction(const RecordKey& key1, const RecordKey& key2) const;

	private:
		int m_column = -1;

		const SignalLogModel* m_model = nullptr;
		const IAppSignalManager* m_appSignalManager = nullptr;
	};

	//
	// SignalLogSorter
	//
	SignalLogSorter::SignalLogSorter(int column, const SignalLogModel* model, const IAppSignalManager* appSignalManager) :
		m_column(column),
		m_model(model),
		m_appSignalManager(appSignalManager)
	{
	}

	bool SignalLogSorter::sortFunction(const RecordKey& key1, const RecordKey& key2) const
	{
		if (m_model == nullptr)
		{
			Q_ASSERT(m_model);
			return false;
		}

		const DiscretesLogRecord& rec1 = m_model->record(key1);
		const DiscretesLogRecord& rec2 = m_model->record(key2);

		QVariant v1;
		QVariant v2;

		AppSignalStateFlags flags1 = {.all = rec1.flags};
		AppSignalStateFlags flags2 = {.all = rec2.flags};

		bool compareSignalParams = false;

		switch (static_cast<SignalLogColumns>(m_column))
		{
		case SignalLogColumns::RecordTime:
			v1 = rec1.recordTime;
			v2 = rec2.recordTime;
			break;
		case SignalLogColumns::SystemTime:
			v1 = rec1.systemTime;
			v2 = rec2.systemTime;
			break;
		case SignalLogColumns::LocalTime:
			v1 = rec1.localTime;
			v2 = rec2.localTime;
			break;
		case SignalLogColumns::PlantTime:
			v1 = rec1.plantTime;
			v2 = rec2.plantTime;
			break;
		case SignalLogColumns::Valid:
			v1 = flags1.valid;
			v2 = flags2.valid;
			break;
		case SignalLogColumns::StateAvailable:
			v1 = flags1.stateAvailable;
			v2 = flags2.stateAvailable;
			break;
		case SignalLogColumns::Simulated:
			v1 = flags1.simulated;
			v2 = flags2.simulated;
			break;
		case SignalLogColumns::Blocked:
			v1 = flags1.blocked;
			v2 = flags2.blocked;
			break;
		case SignalLogColumns::Mismatch:
			v1 = flags1.mismatch;
			v2 = flags2.mismatch;
			break;
		case SignalLogColumns::OutOfLimits:
			if (flags1.belowLowLimit == flags2.belowLowLimit)
			{
				v1 = flags1.aboveHighLimit;
				v2 = flags2.aboveHighLimit;
			}
			else
			{
				v1 = flags1.belowLowLimit;
				v2 = flags2.belowLowLimit;
			}
			break;

		case SignalLogColumns::AckSource:
			v1 = rec1.ackSource;
			v2 = rec2.ackSource;
			break;
		case SignalLogColumns::AckTime:
			v1 = rec1.ackTime;
			v2 = rec2.ackTime;
			break;
		case SignalLogColumns::AckUser:
			v1 = rec1.ackUser;
			v2 = rec2.ackUser;
			break;

		default:
			compareSignalParams = true;
		}

		if (compareSignalParams == true)
		{
			bool found = false;
			const AppSignalParam& s1 = m_appSignalManager->signalParam(rec1.signalHash, &found);
			const AppSignalParam& s2 = m_appSignalManager->signalParam(rec2.signalHash, &found);

			switch (static_cast<SignalLogColumns>(m_column))
			{
			case SignalLogColumns::CustomAppSignalID:
				v1 = s1.customSignalId();
				v2 = s2.customSignalId();
				break;
			case SignalLogColumns::EquipmentID:
				v1 = s1.equipmentId();
				v2 = s2.equipmentId();
				break;
			case SignalLogColumns::LmEquipmentID:
				v1 = s1.lmEquipmentId();
				v2 = s2.lmEquipmentId();
				break;
			case SignalLogColumns::AppSignalID:
				v1 = s1.appSignalId();
				v2 = s2.appSignalId();
				break;
			case SignalLogColumns::Caption:
				v1 = s1.caption();
				v2 = s2.caption();
				break;
			case SignalLogColumns::Type:
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
			case SignalLogColumns::Tags:
				v1 = s1.tagStringList().join(' ');
				v2 = s2.tagStringList().join(' ');
				break;
			case SignalLogColumns::Value:
				if (flags1.valid != flags2.valid)
				{
					v1 = flags1.valid;
					v2 = flags2.valid;
				}
				else
				{
					if (flags1.stateAvailable != flags2.stateAvailable)
					{
						v1 = flags1.stateAvailable;
						v2 = flags2.stateAvailable;
					}
					else
					{
						if (s1.isAnalog() == s2.isAnalog())
						{
							v1 = rec1.value;
							v2 = rec2.value;
						}
						else
						{
							v1 = s1.isAnalog();
							v2 = s2.isAnalog();
						}
					}
				}
				break;

			default:
				Q_ASSERT(false);
				return rec1.recordTime < rec2.recordTime;
			}
		}

		Q_ASSERT(v1.userType() != v2.userType());

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
		return rec1.recordTime < rec2.recordTime;
	}

	class LogSelectionControlDelegate : public QStyledItemDelegate
	{
	public:
		LogSelectionControlDelegate(QObject* parent,
									SignalLogModel* model,
									const QString& signalLogTagCritical,
									const QString& signalLogTagWarning);
		void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

	private:
		SignalLogModel* m_model = nullptr;
		
		QString m_signalLogTagCritical;
		QString m_signalLogTagWarning;
	};

	//
	// LogSelectionControlDelegate
	//
	LogSelectionControlDelegate::LogSelectionControlDelegate(QObject* parent,
															 SignalLogModel* model,
															 const QString& signalLogTagCritical,
															 const QString& signalLogTagWarning) :
		QStyledItemDelegate(parent),
		m_model(model),
		m_signalLogTagCritical(signalLogTagCritical),
		m_signalLogTagWarning(signalLogTagWarning)
	{
	}

	void LogSelectionControlDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
	{
		QStyledItemDelegate::initStyleOption(option, index);

		bool active = option->state & QStyle::State_Active;
		bool selected = option->state & QStyle::State_Selected;

		bool found = false;
		const auto& asp = m_model->signalParam(index.row(), &found);

		QBrush br;

		if (asp.tagStringList().contains(m_signalLogTagCritical) == true)
		{
			if (selected == true)
			{
				br = QBrush{StandardColors::LogErrorForeground};
			}
			else
			{
				br = QBrush{StandardColors::LogErrorForegroundDark};
			}
		}
		else if (asp.tagStringList().contains(m_signalLogTagWarning) == true)
		{
			if (selected == true)
			{
				br = QBrush{StandardColors::LogWarningForegroundDark};
			}
			else
			{
				br = QBrush{StandardColors::LogWarningForeground};
			}
		}
		
		option->palette.setColor(QPalette::Text, br.color());

		// Set color for selected item (by default it is displayed by white)
		//
		if (selected == true)
		{
			if (br.style() == Qt::NoBrush && active == true)
			{
				// Use white color on selected items if control is active
				//
				option->palette.setColor(QPalette::HighlightedText, Qt::white);
			}
			else
			{
				option->palette.setColor(QPalette::HighlightedText, br.color());
			}
			if (active == true)
			{
				option->palette.setColor(QPalette::Highlight, qRgb(0x90, 0xc8, 0xf6));
			}
			else
			{
				option->palette.setColor(QPalette::Highlight, qRgb(0xe0, 0xe0, 0xe0));
			}
		}
		else
		{
			option->palette.setColor(QPalette::Base, Qt::white);
		}
	}

} // namespace

//
// SignalLogModel
//
SignalLogModel::SignalLogModel(const ClientLib::SignalLog& signalLog,
							   const IAppSignalManager* appSignalManager,
							   const AppSignalLists::AppSignalListSet* appSignalListSet,
							   QObject* parent) :
	m_signalLog(signalLog),
	m_appSignalManager(appSignalManager),
	m_appSignalListSet(appSignalListSet)
{
	Q_UNUSED(parent);

	// Fill column names
	//
	m_columnsNames << QObject::tr("Signal ID");
	m_columnsNames << QObject::tr("Equipment ID");
	m_columnsNames << QObject::tr("Lm Equipment ID");
	m_columnsNames << QObject::tr("App Signal ID");
	m_columnsNames << QObject::tr("Caption");
	m_columnsNames << QObject::tr("Type");
	m_columnsNames << QObject::tr("Tags");

	m_columnsNames << QObject::tr("Record Time");
	m_columnsNames << QObject::tr("Server Time UTC%100").arg(QChar(0x00B1));
	m_columnsNames << QObject::tr("Server Time");
	m_columnsNames << QObject::tr("Plant Time");

	m_columnsNames << QObject::tr("Value");

	m_columnsNames << QObject::tr("Valid");
	m_columnsNames << QObject::tr("StateAvailable");
	m_columnsNames << QObject::tr("Simulated");
	m_columnsNames << QObject::tr("Blocked");
	m_columnsNames << QObject::tr("Mismatch");
	m_columnsNames << QObject::tr("OutOfLimits");

	m_columnsNames << QObject::tr("AckTime");
	m_columnsNames << QObject::tr("AckSource");
	m_columnsNames << QObject::tr("AckUser");

	Q_ASSERT(m_columnsNames.size() == static_cast<qsizetype>(SignalLogColumns::ColumnCount));

	return;
}

QStringList SignalLogModel::columnsNames() const
{
	return m_columnsNames;
}

qint64 SignalLogModel::updateCounter() const
{
	return m_updateCounter;
}

qint64 SignalLogModel::maxInitialRecordTime() const
{
	return m_maxInitialRecordTime;
}

void SignalLogModel::resetMaxInitialRecordTime()
{
	m_maxInitialRecordTime = -1;

	for (const auto& [key, rec] : m_records)
	{
		if (rec.recordTime > m_maxInitialRecordTime)
		{
			m_maxInitialRecordTime = rec.recordTime; // Update the max record Time on first records filling
		}
	}
}

int SignalLogModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(SignalLogColumns::ColumnCount);
}

int SignalLogModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_filteredRecords.size());
}

void SignalLogModel::setMaskType(SignalLogMaskType type)
{
	m_maskType = type;
}

void SignalLogModel::setMasks(const QStringList& masks)
{
	m_masks = masks;
}

void SignalLogModel::setTags(const QStringList& tags)
{
	m_tags = tags;
}

void SignalLogModel::setAppSignalList(const QString& listId)
{
	m_appSignallistID = listId;

	if (m_appSignalListSet != nullptr)
	{
		if (m_appSignallistID.isEmpty() == true)
		{
			m_appSignalListHashes.clear();
		}
		else
		{
			std::shared_ptr<AppSignalLists::AppSignalList> list = m_appSignalListSet->get(m_appSignallistID);
			if (list != nullptr)
			{
				m_appSignalListHashes = list->appListHashesCache();
			}
		}
	}
}

QString SignalLogModel::appSignalList() const
{
	return m_appSignallistID;
}

void SignalLogModel::setRecords(std::vector<DiscretesLogRecord>& records, qint64 updateCounter)
{
	m_updateCounter = updateCounter;

	std::unordered_set<RecordKey, RecordKey> newKeys;
	newKeys.reserve(records.size());

	{
	// Add records that do not exist in the model
	//
		qsizetype prevRecordsCount = m_filteredRecords.size();

		for (const auto& rec : records)
		{
			RecordKey key{rec};

			newKeys.insert(key);

			if (m_records.contains(key) == true)
			{
				continue;
			}

			if (m_initMaxInitialRecordTime == true && rec.recordTime > m_maxInitialRecordTime)
			{
				m_maxInitialRecordTime = rec.recordTime; // Update the max record time on first records filling
			}

			m_records.insert({key, rec});

			if (filterRecord(rec) == true)
			{
				m_filteredRecords.push_back(key);
			}
		}

		qsizetype addedRecordsCount = m_filteredRecords.size() - prevRecordsCount;
		if (addedRecordsCount > 0)
		{
			beginInsertRows(QModelIndex(), prevRecordsCount, prevRecordsCount + addedRecordsCount - 1);
			insertRows(prevRecordsCount, prevRecordsCount + addedRecordsCount - 1);
			endInsertRows();
		}

		m_initMaxInitialRecordTime = false;
	}

	// Remove records that do not exist in new data
	{
		// Build a map with records needed to be removed
		//
		std::unordered_set<RecordKey, RecordKey> recordsToDelete;
		for (const auto& [key, rec] : m_records)
		{
			if (newKeys.contains(key) == false)
			{
				recordsToDelete.insert(key);
			}
		}

		// Remove records from the main map
		//
		for (const auto& key : recordsToDelete)
		{
			m_records.erase(key);
		}

		// Remove rows from filteded array and from the model
		//
		qsizetype count = m_filteredRecords.size();
		for (qsizetype i = count - 1; i >= 0; i--)
		{
			if (recordsToDelete.contains(m_filteredRecords[i]) == true)
			{
				beginRemoveRows(QModelIndex(), i, i);
				removeRows(i, 1);
				m_filteredRecords.erase(m_filteredRecords.begin() + i);
				endRemoveRows();
			}
		}
	}
}

void SignalLogModel::fillRecords(bool resetSelection)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
		removeRows(0, rowCount());

		m_filteredRecords.clear();

		endRemoveRows();
	}

	if (resetSelection == true)
	{
		m_initMaxInitialRecordTime = true;
		m_maxInitialRecordTime = -1;
	}

	std::vector<RecordKey> filteredRecords;
	filteredRecords.reserve(m_records.size());

	// Fill records
	//
	for (const auto& [key, rec]: m_records)
	{
		if (m_initMaxInitialRecordTime == true && rec.recordTime > m_maxInitialRecordTime)
		{
			m_maxInitialRecordTime = rec.recordTime; // Update the max record time on first records filling
		}

		if (filterRecord(rec) == false)
		{
			continue;
		}

		filteredRecords.push_back(key);
	}

	if (filteredRecords.empty() == false)
	{
		beginInsertRows(QModelIndex(), 0, static_cast<int>(filteredRecords.size()) - 1);

		m_filteredRecords = std::move(filteredRecords);

		insertRows(0, static_cast<int>(m_filteredRecords.size()));
		endInsertRows();
	}

	m_initMaxInitialRecordTime = false;

	return;
}

int SignalLogModel::recordsCount() const
{
	return static_cast<int>(m_records.size());
}

const DiscretesLogRecord& SignalLogModel::record(const RecordKey& key) const
{
	auto it = m_records.find(key);
	if (it == m_records.end())
	{
		Q_ASSERT(false);
		static DiscretesLogRecord err;
		return err;
	}

	return it->second;
}

const DiscretesLogRecord& SignalLogModel::filteredRecord(int index) const
{
	if (index < 0 || index >= m_filteredRecords.size())
	{
		Q_ASSERT(false);
		static DiscretesLogRecord err;
		return err;
	}

	const auto& recordKey = m_filteredRecords[index];

	auto it = m_records.find(recordKey);
	if (it == m_records.end())
	{
		Q_ASSERT(false);
		static DiscretesLogRecord err;
		return err;
	}

	return it->second;
}

void SignalLogModel::sort(int column, Qt::SortOrder sortOrder)
{
	if (m_filteredRecords.empty() == true)
	{
		return;
	}

	int sortColumn = column;

	std::stable_sort(m_filteredRecords.begin(), m_filteredRecords.end(), SignalLogSorter(sortColumn, this, m_appSignalManager));

	if (sortOrder == Qt::DescendingOrder)
	{
		std::reverse(std::begin(m_filteredRecords), std::end(m_filteredRecords));
	}

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

	return;
}

AppSignalParam SignalLogModel::signalParam(int rowIndex, bool* found)
{
	if (found == nullptr)
	{
		Q_ASSERT(found);
		return AppSignalParam();
	}

	if (rowIndex < 0 || rowIndex >= static_cast<int>(m_filteredRecords.size()))
	{
		Q_ASSERT(false);
		*found = false;
		return AppSignalParam();
	}

	*found = true;

	const auto& recordKey = m_filteredRecords[rowIndex];

	auto it = m_records.find(recordKey);
	if (it == m_records.end())
	{
		Q_ASSERT(false);
		return AppSignalParam();
	}

	const DiscretesLogRecord& rec = it->second;
	return m_appSignalManager->signalParam(rec.signalHash, found);
}

QVariant SignalLogModel::data(const QModelIndex& index, int role) const
{
	int col = index.column();
	if (col < 0 || col >= static_cast<int>(SignalLogColumns::ColumnCount))
	{
		Q_ASSERT(false);
		return QVariant();
	}

	int row = index.row();
	if (row >= m_filteredRecords.size())
	{
		Q_ASSERT(false);
		return QVariant();
	}

	SignalLogColumns columnIndex = static_cast<SignalLogColumns>(col);

	if (role == Qt::DisplayRole)
	{
		const auto& recordKey = m_filteredRecords[row];

		auto it = m_records.find(recordKey);
		if (it == m_records.end())
		{
			Q_ASSERT(false);
			return QVariant();
		}

		const DiscretesLogRecord& rec = it->second;

		AppSignalStateFlags flags{.all = rec.flags};

		switch (columnIndex)
		{
		case SignalLogColumns::RecordTime:
			{
				return QDateTime::fromMSecsSinceEpoch(rec.recordTime).toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SignalLogColumns::SystemTime:
			{
				return QDateTime::fromMSecsSinceEpoch(rec.systemTime, QTimeZone::UTC).toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SignalLogColumns::LocalTime:
			{
				return QDateTime::fromMSecsSinceEpoch(rec.localTime, QTimeZone::UTC).toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SignalLogColumns::PlantTime:
			{
				return QDateTime::fromMSecsSinceEpoch(rec.plantTime, QTimeZone::UTC).toString("dd.MM.yyyy hh:mm:ss.zzz");
			}
		case SignalLogColumns::Valid:
			{
				return (flags.valid == true) ? QObject::tr("yes") : QObject::tr("no");
			}
		case SignalLogColumns::StateAvailable:
			{
				return (flags.stateAvailable == true) ? QObject::tr("yes") : QObject::tr("no");
			}
		case SignalLogColumns::Simulated:
			{
				return (flags.simulated == true) ? QObject::tr("yes") : QObject::tr("no");
			}
		case SignalLogColumns::Blocked:
			{
				return (flags.blocked == true) ? QObject::tr("yes") : QObject::tr("no");
			}
		case SignalLogColumns::Mismatch:
			{
				return (flags.mismatch == true) ? QObject::tr("yes") : QObject::tr("no");
			}
		case SignalLogColumns::OutOfLimits:
			{
				QString resultString;

				if (flags.belowLowLimit == true)
				{
					resultString += QStringLiteral("LOW ");
				}
				if (flags.aboveHighLimit == true)
				{
					resultString += QStringLiteral("HIGH ");
				}
				return resultString.trimmed();
			}
		}

		//
		// Get signal now
		//
		bool found = false;
		const AppSignalParam& s = m_appSignalManager->signalParam(rec.signalHash, &found);
		if (found == false)
		{
			if (columnIndex == SignalLogColumns::AppSignalID)
			{
				return {"?"};
			}
			return {};
		}

		switch (columnIndex)
		{
		case SignalLogColumns::Value:
			{
				QString valueResult;

				switch (s.type())
				{
				case E::SignalType::Analog:
					valueResult = AppSignalState::toString(rec.value,
														   E::ValueViewType::Dec,
														   E::AnalogFormat::g_9_or_9e,
														   s.analogSignalFormat(),
														   s.precision());
					break;
				case E::SignalType::Discrete:
					valueResult = static_cast<int>(rec.value) == 0 ? "0" : "1";
					break;
				case E::SignalType::Bus:
					valueResult = QObject::tr("Bus Type");
					break;
				default:
					Q_ASSERT(false);
				}

				if (flags.valid == false)
				{
					if (flags.stateAvailable == true)
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

		case SignalLogColumns::CustomAppSignalID:
			{
				return s.customSignalId();
			}

		case SignalLogColumns::EquipmentID:
			{
				return s.equipmentId();
			}

		case SignalLogColumns::LmEquipmentID:
			{
				return s.lmEquipmentId();
			}

		case SignalLogColumns::AppSignalID:
			{
				return s.appSignalId();
			}

		case SignalLogColumns::Caption:
			{
				return s.caption();
			}

		case SignalLogColumns::Type:
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
				if (s.isAnalog() == true)
				{
					str = QString("%1 (%2)").arg(str).arg(
						E::valueToString<E::AnalogAppSignalFormat>(static_cast<int>(s.analogSignalFormat())));
				}

				str = QString("%1, %2").arg(str).arg(QObject::tr(E::valueToString<E::SignalInOutType>(s.inOutType()).toUtf8()));

				return str;
			}

		case SignalLogColumns::Tags:
			{
				return s.tagStringList().join(' ');
			}

		default:
			{
				return QString();
			}
		}
	}

	if (role == Qt::TextAlignmentRole && (columnIndex == SignalLogColumns::Value || columnIndex == SignalLogColumns::Valid ||
										  columnIndex == SignalLogColumns::StateAvailable || columnIndex == SignalLogColumns::Simulated ||
										  columnIndex == SignalLogColumns::Blocked || columnIndex == SignalLogColumns::Mismatch))
	{
		return QVariant(Qt::AlignCenter);
	}


	return QVariant();
}

QVariant SignalLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section < 0 || section >= static_cast<int>(SignalLogColumns::ColumnCount))
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

bool SignalLogModel::filterRecord(const DiscretesLogRecord& rec) const
{ 
	// Filter by signal list
	//
	if (m_appSignalListSet != nullptr && m_appSignallistID.isEmpty() == false && m_appSignalListHashes.contains(rec.signalHash) == false)
	{
		return false;
	}

	// Filter by Mask
	//
	if (m_masks.isEmpty() == false)
	{
		bool found = false;
		const AppSignalParam& asp = m_appSignalManager->signalParam(rec.signalHash, &found);
		if (found == false)
		{
			return false;
		}

		bool result = false;
		QStringList strIdList;

		// Select what to analyze
		//
		switch (m_maskType)
		{
		case SignalLogMaskType::All:
			strIdList << asp.appSignalId().trimmed();
			strIdList << asp.customSignalId().trimmed();
			strIdList << asp.equipmentId().trimmed();
			strIdList << asp.lmEquipmentId().trimmed();
			break;

		case SignalLogMaskType::AppSignalID:
			strIdList << asp.appSignalId().trimmed();
			break;

		case SignalLogMaskType::CustomAppSignalID:
			strIdList << asp.customSignalId().trimmed();
			break;

		case SignalLogMaskType::EquipmentID:
			strIdList << asp.equipmentId().trimmed();
			break;

		case SignalLogMaskType::LmEquipmentID:
			strIdList << asp.lmEquipmentId().trimmed();
			break;
		}

		for (const QString& mask : m_masks)
		{
			if (mask.contains('*') == true || mask.contains('?') == true)
			{
				// Process wildcard
				//
				static QRegularExpression rx;
				if (rx.pattern() != QRegularExpression::wildcardToRegularExpression(mask.trimmed()))
				{
					rx = QRegularExpression{QRegularExpression::wildcardToRegularExpression(mask.trimmed())};
				}

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
			return false;
		}
	}

	// Filter by tags
	//
	if (m_tags.isEmpty() == false)
	{
		bool found = false;
		const AppSignalParam& asp = m_appSignalManager->signalParam(rec.signalHash, &found);
		if (found == false)
		{
			return false;
		}

		bool result = false;
		const auto& signalTags = asp.tags();

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
			return false;
		}
	}

	return true;
}

//
// SignalLogDialogSettings
//
void SignalLogDialogSettings::restore()
{
	QSettings s;

	horzHeader = s.value("SignalLogWidget/horzHeader").toByteArray();
	horzHeaderCount = s.value("SignalLogWidget/horzHeaderCount").toInt();

	sortColumn = s.value("SignalLogWidget/sortColumn", sortColumn).toInt();
	sortOrder = static_cast<Qt::SortOrder>(s.value("SignalLogWidget/sortOrder", sortOrder).toInt());

	maskList = s.value("SignalLogWidget/maskList").toStringList();
	tagsList = s.value("SignalLogWidget/tagsList").toStringList();
}

void SignalLogDialogSettings::store()
{
	QSettings s;

	s.setValue("SignalLogWidget/horzHeader", horzHeader);
	s.setValue("SignalLogWidget/horzHeaderCount", horzHeaderCount);

	s.setValue("SignalLogWidget/sortColumn", sortColumn);
	s.setValue("SignalLogWidget/sortOrder", static_cast<int>(sortOrder));

	s.setValue("SignalLogWidget/maskList", maskList);
	s.setValue("SignalLogWidget/tagsList", tagsList);
}

// SignalLogTableView
//
void SignalLogTableView::mousePressEvent(QMouseEvent* event)
{
	QTableView::mousePressEvent(event);

	SignalLogModel* logModel = dynamic_cast<SignalLogModel*>(model());
	if (logModel == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	QList<AppSignalParam> appSignalParams;

	QModelIndexList rows = selectionModel()->selectedRows();

	for (QModelIndex& index : rows)
	{
		bool found = false;

		AppSignalParam appSignalParam = logModel->signalParam(index.row(), &found);

		if (found == true)
		{
			appSignalParams.push_back(appSignalParam);
		}
	}

	m_dragDropHelper.onMousePress(event, appSignalParams);

	return;
}

void SignalLogTableView::mouseMoveEvent(QMouseEvent* event)
{
	m_dragDropHelper.onMouseMove(event, this);

	return;
}

//
// SignalLogWidget
//
SignalLogWidget::SignalLogWidget(ClientLib::SignalLog& signalLog,
								 const IAppSignalManager* appSignalManager,
								 const AppSignalLists::AppSignalListSet* appSignalListSet,
								 const QString& projectName,
								 const QString& equipmentId,
								 const QString& signalLogTagCritical,
								 const QString& signalLogTagWarning,
								 QWidget* parent) :
	QWidget(parent),
	m_signalLog(signalLog),
	m_appSignalManager(appSignalManager),
	m_appSignalListSet(appSignalListSet),
	m_projectName(projectName),
	m_equipmentId(equipmentId),
	m_model(m_signalLog, m_appSignalManager, m_appSignalListSet, this),
	m_signalLogTagCritical(signalLogTagCritical),
	m_signalLogTagWarning(signalLogTagWarning)
	
{
	if (m_appSignalManager == nullptr)
	{
		Q_ASSERT(m_appSignalManager);
		return;
	}

	m_maskHelp = tr("A mask contains '*' and '?' symbols.\n\
	'*' symbol means any set of symbols on its place, '?' symbol means one symbol on its place.\n\
	Several masks can be separated by semicolon or space.\n\n\
	Examples:\n\n\
	#SF001P014* (mask for AppSignalID),\n\
	T?30T01? (mask for CustomAppSignalID),\n\
	#SYSTEMID_RACK01_CH01_MD?? (mask for Equipment ID).\n\n\
	To apply the filter, enter the mask and press Enter.");
	m_maskHelp.remove('\t');

	m_tagsHelp = tr("Tags for filtering signals.\n\n\
	Several tags can be separated by semicolon or space: \"tag1; tag2\" or \"tag1 tag2\".\n\n\
	To apply the filter, enter tags and press Enter.");
	m_tagsHelp.remove('\t');

	m_settings.restore();

	createControls();

	initRecordsView();
	initFiltersView();

	if (m_appSignalListSet != nullptr)
	{
		connect(m_appSignalListSet, &AppSignalLists::AppSignalListSet::updatePerformed, this, &SignalLogWidget::fillAppSignalLists);
	}

	m_updateStateTimerId = startTimer(500);

	return;
}

SignalLogWidget::~SignalLogWidget() // save type and mask data ??? need to add saved Type and Mask???
{
	killTimer(m_updateStateTimerId);

	// Save state
	//
	m_settings.horzHeader = m_tableView->horizontalHeader()->saveState();
	m_settings.horzHeaderCount = static_cast<int>(SignalLogColumns::ColumnCount);

	m_settings.store();

	return;
}

QString SignalLogWidget::projectName() const
{
	return m_projectName;
}

void SignalLogWidget::setProjectName(const QString& projectName)
{
	m_projectName = projectName;
}

void SignalLogWidget::showEvent([[maybe_unused]] QShowEvent* event)
{
	if (m_firstShow == false)
	{
		return;
	}

	m_firstShow = false;

	updateRecords();

	return;
}

void SignalLogWidget::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();

	if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape)
	{
		event->ignore();
	}
	else
	{
		QWidget::keyPressEvent(event);
	}

	return;
}

void SignalLogWidget::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == m_updateStateTimerId)
	{
		updateRecords();
	}
}

void SignalLogWidget::headerColumnContextMenuRequested(const QPoint& pos)
{
	QMenu menu(this);

	QList<QAction*> actions;

	std::vector<std::pair<SignalLogColumns, QString>> actionsData;
	actionsData.reserve(static_cast<int>(SignalLogColumns::ColumnCount));

	SignalLogModel* model = dynamic_cast<SignalLogModel*>(m_tableView->model());
	if (model == nullptr)
	{
		Q_ASSERT(model);
		return;
	}

	QStringList columns = model->columnsNames();

	for (int i = 0; i < columns.size(); i++)
	{
		actionsData.emplace_back(static_cast<SignalLogColumns>(i), columns[i]);
	}

	for (std::pair<SignalLogColumns, QString> ad : actionsData)
	{
		QAction* action = new QAction(ad.second, this);
		action->setData(QVariant::fromValue(ad.first));
		action->setCheckable(true);
		action->setChecked(!m_tableView->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

		if (m_tableView->horizontalHeader()->count() - m_tableView->horizontalHeader()->hiddenSectionCount() == 1 &&
			action->isChecked() == true)
		{
			action->setEnabled(false); // Impossible to uncheck the last column
		}

		connect(action, &QAction::toggled, this, &SignalLogWidget::headerColumnToggled);

		actions << action;
	}

	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}

void SignalLogWidget::headerColumnToggled(bool checked)
{
	QAction* action = dynamic_cast<QAction*>(sender());

	if (action == nullptr)
	{
		Q_ASSERT(action);
		return;
	}

	int column = action->data().value<int>();

	if (column >= static_cast<int>(SignalLogColumns::ColumnCount))
	{
		Q_ASSERT(column < static_cast<int>(SignalLogColumns::ColumnCount));
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

void SignalLogWidget::contextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	QStringList list;

	TimeStamp maxPlantTime = 0;

	m_signalMenu.clear();

	QModelIndexList rows = m_tableView->selectionModel()->selectedRows();

	for (QModelIndex& index : rows)
	if (index.isValid() == true)
	{
		bool found = false;

		AppSignalParam appSignalParam = m_model.signalParam(index.row(), &found);
		if (found == false) 
		{
			continue;
		}

		const auto& rec = m_model.filteredRecord(index.row());
		if (rec.acknowledged == false && rec.plantTime > maxPlantTime) 
		{
			maxPlantTime = rec.plantTime;
		}

		const auto& appSignalID = appSignalParam.appSignalId();
		if (list.contains(appSignalID) == false)
		{
			list << appSignalID;
		}
	}

	if (maxPlantTime != 0) 
	{
		QAction* action = new QAction(
			"Acknowledge up to " + QDateTime::fromMSecsSinceEpoch(maxPlantTime.timeStamp, QTimeZone::UTC).toString("dd.MM.yyyy hh:mm:ss.zzz"),
			&m_signalMenu);
		connect(action,
				&QAction::triggered,
				[maxPlantTime, this]()
				{
					if (warnAboutAckFiltered() == false) 
					{
						return;
					}

					m_signalLog.sendAckUpTo(maxPlantTime);
				});
		m_signalMenu.addAction(action);
	}

	if (list.isEmpty() == true) 
	{
		return;
	}

	emit signalContextMenu(list, QList<QMenu*>() << &m_signalMenu);
}

void SignalLogWidget::tableViewDoubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);

	int row = m_tableView->currentIndex().row();
	if (row == -1)
	{
		return;
	}

	int rowIndex = m_tableView->currentIndex().row();

	bool found = false;

	const AppSignalParam& s = m_model.signalParam(rowIndex, &found);

	if (found == false)
	{
		return;
	}

	QTimer::singleShot(10,
					   [this, s]
					   {
						   emit signalInfo(s.appSignalId());
					   });
}

void SignalLogWidget::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_settings.sortColumn = column;
	m_settings.sortOrder = order;
}

void SignalLogWidget::editMaskReturnPressed()
{
	maskChanged(true /*addToCompleter*/);

	m_model.fillRecords(true /*resetSelection*/);
}

void SignalLogWidget::editTagsReturnPressed()
{
	tagsChanged();

	m_model.fillRecords(true /*resetSelection*/);
}

void SignalLogWidget::maskTypeComboCurrentIndexChanged(int index)
{
	m_model.setMaskType(static_cast<SignalLogMaskType>(index));

	QString mask = m_editMask->text();
	if (mask.isEmpty() == true)
	{
		return;
	}

	m_model.fillRecords(true /*resetSelection*/);
}

void SignalLogWidget::signalListComboIndexChanged(int /*index*/)
{
	m_model.setAppSignalList(m_signalListCombo->currentData().toString());
	m_model.fillRecords(true /*resetSelection*/);
}

void SignalLogWidget::buttonExportClicked()
{
	if (m_model.rowCount() == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("Nothing to export."));
		return;
	}

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(
		this,
		tr("Save File"),
		path + QDir::separator() + "untitled.pdf",
		tr("Portable Document Format (*.pdf);;CSV Files, semicolon separated (*.csv);;Plaintext (*.txt);;HTML (*.html)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	QFileInfo fileInfo(fileName);
	QString extension = fileInfo.completeSuffix();

	if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
	{
		QPageLayout pageLayout(QPageSize(QPageSize::A4),
							   QPageLayout::Orientation::Portrait,
							   QMarginsF(25, 20, 15, 20),
							   QPageLayout::Unit::Millimeter);

		pageLayout = ReportLib::TableViewReportGenerator::loadPageLayoutFromSettings("SignalLogExportPageLayout", pageLayout);

		SignalLogReportGenerator ri(m_projectName, m_equipmentId);

		ReportLib::TableViewReportGenerator generator(this, *m_tableView, ri, pageLayout);
		generator.exportTable(fileName);

		pageLayout = generator.pageLayout();
		ReportLib::TableViewReportGenerator::savePageLayoutToSettings(pageLayout, "SignalLogExportPageLayout");

		return;
	}

	QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));
	return;
}

void SignalLogWidget::buttonPrintClicked()
{
	QPageLayout pageLayout(QPageSize(QPageSize::A4),
						   QPageLayout::Orientation::Portrait,
						   QMarginsF(10, 10, 10, 10),
						   QPageLayout::Unit::Millimeter);

	pageLayout = ReportLib::TableViewReportGenerator::loadPageLayoutFromSettings("SignalLogPrintPageLayout", pageLayout);

	SignalLogReportGenerator ri(m_projectName, m_equipmentId);

	ReportLib::TableViewReportGenerator generator(this, *m_tableView, ri, pageLayout);
	generator.printTable();

	pageLayout = generator.pageLayout();
	ReportLib::TableViewReportGenerator::savePageLayoutToSettings(pageLayout, "SignalLogPrintPageLayout");
}

void SignalLogWidget::buttonChooseTagsClicked()
{
	QDialog tagsSelectorDialog{this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint};

	int width = QSettings().value("SignalLogWidget/tagsSelectorDialog/width", 340).toInt();
	int height = QSettings().value("SignalLogWidget/tagsSelectorDialog/height", 400).toInt();
	tagsSelectorDialog.resize(width, height);

	UiLib::ChooseItemsWidget te{m_appSignalManager->tags(), this};
	te.setText(m_editTags->text());

	connect(&te, &UiLib::ChooseItemsWidget::okPressed, &tagsSelectorDialog, &QDialog::accept);
	connect(&te, &UiLib::ChooseItemsWidget::cancelPressed, &tagsSelectorDialog, &QDialog::reject);

	QHBoxLayout l;
	l.addWidget(&te);
	tagsSelectorDialog.setLayout(&l);

	if (tagsSelectorDialog.exec() == QDialog::Accepted)
	{
		m_editTags->setText(te.text());

		tagsChanged();

		m_model.fillRecords(true /*resetSelection*/);
	}

	QSettings().setValue("SignalLogWidget/tagsSelectorDialog/width", tagsSelectorDialog.width());
	QSettings().setValue("SignalLogWidget/tagsSelectorDialog/height", tagsSelectorDialog.height());
}

void SignalLogWidget::buttonClearFilterClicked()
{
	// Mask
	//
	m_editMask->blockSignals(true);
	m_editMask->clear();
	m_editMask->blockSignals(false);

	m_maskTypeCombo->blockSignals(true); // Block to prevent signals from updating automatically
	m_maskTypeCombo->setCurrentIndex(static_cast<int>(SignalLogMaskType::All));
	m_maskTypeCombo->blockSignals(false);

	m_model.setMasks({});

	// List
	//
	m_signalListCombo->blockSignals(true);
	m_signalListCombo->setCurrentIndex(0);
	m_signalListCombo->blockSignals(false);
	m_model.setAppSignalList({});

	// Tags
	//
	m_editTags->blockSignals(true);
	m_editTags->clear();
	m_editTags->blockSignals(false);

	m_model.setTags({});

	//
	m_model.fillRecords(true /*resetSelection*/);
}

void SignalLogWidget::buttonAckAllClicked()
{
	if (filterIsSet() == true) 
	{
		if (warnAboutAckFiltered() == false)
		{
			return;
		}
	}
	else
	{
		if (QMessageBox::warning(this,
								 qAppName(),
								 tr("Are you sure you want to acknowledge all signal events?"),
								 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
	}

	auto [records, index] = m_signalLog.getRecords();
	
	int count = static_cast<int>(records.size());
	for (int i = count - 1; i >= 0; i--) 
	{
		if (records[i].acknowledged == false) 
		{
			m_signalLog.sendAckUpTo(records[i].plantTime);
			break;
		}
	}

	return;
}

void SignalLogWidget::createControls()
{
	// Filter layout

	QGridLayout* filterLayout = new QGridLayout();

	int row = 0;
	int col = 0;

	// Mask field and type combo
	{
		QHBoxLayout* maskLayout = new QHBoxLayout();
		maskLayout->setContentsMargins(0, 0, 0, 0);

		maskLayout->addWidget(new QLabel(tr("Mask")));

		m_editMask = new QLineEdit();
		m_editMask->setPlaceholderText("Enter mask (\"*,?\") here");
		connect(m_editMask, &QLineEdit::returnPressed, this, &SignalLogWidget::editMaskReturnPressed);
		maskLayout->addWidget(m_editMask);
		m_editMask->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

		m_maskTypeCombo = new QComboBox();
		connect(m_maskTypeCombo,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this,
				&SignalLogWidget::maskTypeComboCurrentIndexChanged);
		maskLayout->addWidget(m_maskTypeCombo);

		filterLayout->addLayout(maskLayout, row, col++);
	}

	// Signal List
	//
	filterLayout->addWidget(new QLabel(tr("List")), row, col++);

	// Signal List Combo
	//
	m_signalListCombo = new QComboBox();
	connect(m_signalListCombo,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&SignalLogWidget::signalListComboIndexChanged);
	filterLayout->addWidget(m_signalListCombo, row, col++);
	m_signalListCombo->setMinimumContentsLength(30);

	// Tags field and button
	//
	{
		QHBoxLayout* tagsLayout = new QHBoxLayout();
		tagsLayout->setSpacing(4);
		tagsLayout->setContentsMargins(0, 0, 0, 0);

		tagsLayout->addWidget(new QLabel(tr("Tags")));

		m_editTags = new QLineEdit();
		m_editTags->setPlaceholderText("Signal tags space separated");
		connect(m_editTags, &QLineEdit::returnPressed, this, &SignalLogWidget::editTagsReturnPressed);
		tagsLayout->addWidget(m_editTags);
		m_editTags->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

		m_buttonChooseTags = new QToolButton();
		connect(m_buttonChooseTags, &QToolButton::clicked, this, &SignalLogWidget::buttonChooseTagsClicked);
		m_buttonChooseTags->setText("...");
		tagsLayout->addWidget(m_buttonChooseTags);

		filterLayout->addLayout(tagsLayout, row, col++);
	}

	filterLayout->addWidget(new QWidget(this), row, col++);

	m_clearFilterButton = new QPushButton(tr("Reset Filter"));
	m_clearFilterButton->setAutoDefault(false);
	filterLayout->addWidget(m_clearFilterButton, row, col++);
	connect(m_clearFilterButton, &QToolButton::clicked, this, &SignalLogWidget::buttonClearFilterClicked);

	filterLayout->setSpacing(4);

	filterLayout->setColumnStretch(0, 1);
	filterLayout->setColumnStretch(1, 0);
	filterLayout->setColumnStretch(2, 0);
	filterLayout->setColumnStretch(3, 1);
	filterLayout->setColumnStretch(4, 1);
	filterLayout->setColumnStretch(5, 0);

	// Export/Print/Fixate

	QHBoxLayout* exPrintLayout = new QHBoxLayout();

	QPushButton* b = new QPushButton(tr("Export..."));
	b->setAutoDefault(false);
	connect(b, &QPushButton::clicked, this, &SignalLogWidget::buttonExportClicked);
	exPrintLayout->addWidget(b);

	b = new QPushButton(tr("Print..."));
	b->setAutoDefault(false);
	connect(b, &QPushButton::clicked, this, &SignalLogWidget::buttonPrintClicked);
	exPrintLayout->addWidget(b);

	exPrintLayout->addStretch();

	m_ackButton = new QPushButton(tr("Ack All"));
	m_ackButton->setAutoDefault(false);
	connect(m_ackButton, &QToolButton::clicked, this, &SignalLogWidget::buttonAckAllClicked);
	exPrintLayout->addWidget(m_ackButton);

	m_buttonFixate = new QPushButton(tr("Fixate"));
	m_buttonFixate->setAutoDefault(false);
	m_buttonFixate->setCheckable(true);
	exPrintLayout->addWidget(m_buttonFixate);

	// Table

	m_tableView = new SignalLogTableView();
	connect(m_tableView, &QTableView::doubleClicked, this, &SignalLogWidget::tableViewDoubleClicked);
	m_tableView->setItemDelegate(new LogSelectionControlDelegate(this, &m_model, m_signalLogTagCritical, m_signalLogTagWarning));
	
	// Main layout

	QVBoxLayout* mainLayout = new QVBoxLayout();

	mainLayout->addLayout(filterLayout);
	mainLayout->addLayout(exPrintLayout);
	mainLayout->addWidget(m_tableView);

	setLayout(mainLayout);

	return;
}

void SignalLogWidget::initFiltersView()
{
	// Masks setup
	//
	m_maskCompleter = new QCompleter(m_settings.maskList, this);
	m_maskCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_editMask->setCompleter(m_maskCompleter);
	m_editMask->setToolTip(m_maskHelp);

	m_maskTypeCombo->blockSignals(true);
	m_maskTypeCombo->addItem(tr("All"));
	m_maskTypeCombo->addItem(tr("AppSignalID"));
	m_maskTypeCombo->addItem(tr("CustomAppSignalID"));
	m_maskTypeCombo->addItem(tr("EquipmentID"));
	m_maskTypeCombo->addItem(tr("LmEquipmentID"));
	m_maskTypeCombo->setCurrentIndex(0);
	m_maskTypeCombo->blockSignals(false);

	m_model.setMaskType(SignalLogMaskType::All);

	connect(m_editMask,
			&QLineEdit::textEdited,
			[this]()
			{
				m_maskCompleter->complete();
			});
	connect(m_maskCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_editMask, &QLineEdit::setText);

	// Signal Lists setup
	//
	fillAppSignalLists();

	// Tags setup
	//
	m_tagsCompleter = new QCompleter(m_settings.tagsList, this);
	m_tagsCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_editTags->setCompleter(m_tagsCompleter);
	m_editTags->setToolTip(m_tagsHelp);

	connect(m_editTags,
			&QLineEdit::textEdited,
			[this]()
			{
				m_tagsCompleter->complete();
			});
	connect(m_tagsCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_editTags, &QLineEdit::setText);
}

void SignalLogWidget::initRecordsView()
{
	// Table view setup
	//

	m_tableView->setModel(&m_model);
	m_tableView->verticalHeader()->hide();
	m_tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_tableView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_tableView->horizontalHeader()->setStretchLastSection(false);
	m_tableView->setGridStyle(Qt::PenStyle::NoPen);
	m_tableView->setSortingEnabled(true);
	m_tableView->setWordWrap(false);

	int fontHeight = fontMetrics().height() + 4;

	QHeaderView* verticalHeader = m_tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(fontHeight);

	connect(m_tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &SignalLogWidget::sortIndicatorChanged);

	m_tableView->horizontalHeader()->setHighlightSections(false);
	m_tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_tableView->horizontalHeader(),
			&QWidget::customContextMenuRequested,
			this,
			&SignalLogWidget::headerColumnContextMenuRequested);

	m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tableView, &QTreeWidget::customContextMenuRequested, this, &SignalLogWidget::contextMenuRequested);

	connect(m_tableView->selectionModel(),
			&QItemSelectionModel::selectionChanged,
			this,
			[this](const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
			{
				m_model.resetMaxInitialRecordTime();
			});

	if (m_settings.horzHeader.isEmpty() == true || m_settings.horzHeaderCount != static_cast<int>(SignalLogColumns::ColumnCount))
	{
		// First time? Set what is should be hidden by default
		//
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::EquipmentID));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::LmEquipmentID));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::Type));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::Tags));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::SystemTime));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::LocalTime));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::Valid));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::StateAvailable));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::Simulated));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::Blocked));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::Mismatch));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::OutOfLimits));

		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::AckTime));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::AckSource));
		m_tableView->hideColumn(static_cast<int>(SignalLogColumns::AckUser));
	}
	else
	{
		m_tableView->horizontalHeader()->restoreState(m_settings.horzHeader);
	}
}

void SignalLogWidget::fillAppSignalLists()
{
	// Remove appSignalList
	//
	if (m_model.appSignalList().isEmpty() == false)
	{
		m_model.setAppSignalList({});
		m_model.fillRecords(true /*resetSelection*/);
	}

	// Refresh AppSignalLists combo
	//
	m_signalListCombo->blockSignals(true);

	m_signalListCombo->clear();
	m_signalListCombo->addItem(tr("Not selected"), QString());

	if (m_appSignalListSet != nullptr)
	{
		const auto lists = m_appSignalListSet->lists();

		for (const auto& list : lists)
		{
			m_signalListCombo->addItem(tr("[%1] %2").arg(list->id()).arg(list->caption()), list->id());
		}
		if (lists.empty() == true)
		{
			m_signalListCombo->setEnabled(false);
		}
	}

	m_signalListCombo->blockSignals(false);
}

void SignalLogWidget::updateRecords()
{
	if (m_signalLog.updateCounter() == m_model.updateCounter())
	{
		return;
	}

	bool modelWasEmpty = m_model.rowCount() == 0;

	// Place new data to the model
	//
	auto [rec, counter] = m_signalLog.getRecords();
	m_model.setRecords(rec, counter);

	// Scroll to bottom
	//
	if (m_buttonFixate->isChecked() == false)
	{
		m_tableView->scrollTo(m_model.index(m_model.rowCount() - 1, 0), QAbstractItemView::EnsureVisible);
	}

	// Resize columns to fit text
	//
	if (modelWasEmpty == true && m_model.rowCount() > 0)
	{
		m_tableView->resizeColumnsToContents();
	}
}

void SignalLogWidget::updateTableItems()
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
		to = m_model.rowCount() - 1;
	}

	// Redraw visible table items
	//
	for (int col = 0; col < m_model.columnCount(); col++)
	{
		if (col >= static_cast<int>(SignalLogColumns::SystemTime))
		{
			for (int row = from; row <= to; row++)
			{
				m_tableView->update(m_model.index(row, col));
			}
		}
	}

	return;
}

void SignalLogWidget::maskChanged(bool addToCompleter)
{
	QString maskText = m_editMask->text().trimmed();

	maskText.replace(' ', ';');

	QStringList masks;

	if (maskText.isEmpty() == false)
	{
		masks = maskText.split(';', Qt::SkipEmptyParts);

		if (addToCompleter == true)
		{
			for (const QString& mask : masks)
			{
				// Save filter history
				//
				if (m_settings.maskList.contains(mask) == false)
				{
					m_settings.maskList.append(mask);

					QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_maskCompleter->model());
					if (completerModel == nullptr)
					{
						Q_ASSERT(completerModel);
						return;
					}

					completerModel->setStringList(m_settings.maskList);
				}
			}
		}
	}

	m_model.setMasks(masks);
}

void SignalLogWidget::tagsChanged()
{
	QString tagsText = m_editTags->text().trimmed();
	tagsText.replace(' ', ';');

	QStringList tags;

	if (tagsText.isEmpty() == false)
	{
		tags = tagsText.split(';', Qt::SkipEmptyParts);

		for (const QString& tag : tags)
		{
			// Save filter history
			//
			if (m_settings.tagsList.contains(tag) == false)
			{
				m_settings.tagsList.append(tag);

				QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_tagsCompleter->model());
				if (completerModel == nullptr)
				{
					Q_ASSERT(completerModel);
					return;
				}

				completerModel->setStringList(m_settings.tagsList);
			}
		}
	}

	m_model.setTags(tags);
}

bool SignalLogWidget::filterIsSet() const 
{
	return m_editMask->text().isEmpty() == false || m_editTags->text().isEmpty() == false || m_signalListCombo->currentIndex() > 0;
}

bool SignalLogWidget::warnAboutAckFiltered()
{
	if (filterIsSet() == true)
	{
		if (QMessageBox::warning(this,
								 qAppName(),
								 tr("Warning!\n\nAn event filter is set. Events that are not displayed will also be "
									"acknowledged.\n\nAre you sure you want to continue?"),
								 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		{
			return false;
		}
	}

	return true;
}


//
// SignalLogDialog
//
SignalLogDialog* SignalLogDialog::s_instance = nullptr;

SignalLogDialog::SignalLogDialog(ClientLib::SignalLog& signalLog,
								 ClientLib::AppSignalManager& appSignalManager,
								 const AppSignalLists::AppSignalListSet* appSignalListSet,
								 const QString& projectName,
								 const QString& equipmentId,
								 const QString& signalLogTagCritical,
								 const QString& signalLogTagWarning,
								 QWidget* parent) :
	QDialog{parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint},
	m_signalLog{signalLog},
	m_appSignalManager{appSignalManager},
	m_signalLogTagCritical{signalLogTagCritical},
	m_signalLogTagWarning{signalLogTagWarning}
{
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowTitle(tr("Signals Log"));

	setMinimumSize(400, 200);

	m_logWidget = new SignalLogWidget(m_signalLog,
									  &appSignalManager,
									  appSignalListSet,
									  projectName,
									  equipmentId,
									  m_signalLogTagCritical,
									  m_signalLogTagWarning,
									  parent);
	connect(m_logWidget, &SignalLogWidget::signalContextMenu, this, &SignalLogDialog::signalContextMenu);
	connect(m_logWidget, &SignalLogWidget::signalInfo, this, &SignalLogDialog::signalInfo);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(m_logWidget);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);
}

SignalLogDialog::~SignalLogDialog()
{
	s_instance = nullptr;
}

SignalLogDialog* SignalLogDialog::createDialog(ClientLib::SignalLog& signalLog,
											   ClientLib::AppSignalManager& appSignalManager,
											   const AppSignalLists::AppSignalListSet* appSignalListSet,
											   const QString& projectName,
											   const QString& equipmentId,
											   const QString& signalLogTagCritical,
											   const QString& signalLogTagWarning,
											   QWidget* parent)
{
	if (s_instance == nullptr)
	{
		s_instance = new SignalLogDialog{signalLog,
										 appSignalManager,
										 appSignalListSet,
										 projectName,
										 equipmentId,
										 signalLogTagCritical,
										 signalLogTagWarning,
										 parent};
		s_instance->show();
		return s_instance;
	}
	else
	{
		s_instance->raise(); // Bring to front
		UiTools::ensureVisible(s_instance);
	}

	return s_instance;
}

void SignalLogDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);

	auto mainWindowPos = QSettings().value("SignalLogDialog/pos", QPoint(200, 200)).toPoint();
	auto mainWindowGeometry = QSettings().value("SignalLogDialog/geometry").toByteArray();

	move(mainWindowPos);
	restoreGeometry(mainWindowGeometry);
}

void SignalLogDialog::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);

	QSettings().setValue("SignalLogDialog/pos", pos());
	QSettings().setValue("SignalLogDialog/geometry", saveGeometry());
}

void SignalLogDialog::signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu)
{
	// Compose menu
	//
	QMenu menu(this);

	for (const QString& s : signalList)
	{
		bool ok = false;
		AppSignalParam signal = m_appSignalManager.signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		auto f = [this, s]() -> void
		{
			signalInfo(s);
		};

		menu.addAction(signalId, f);
	}

	if (customMenu.empty() == false)
	{
		menu.addSeparator();

		for (auto cm : customMenu)
		{
			menu.addActions(cm->actions());
		}
	}

	menu.exec(QCursor::pos());
}

void SignalLogDialog::signalInfo(QString appSignalId)
{
	MonitorSignalInfo::showDialog(appSignalId,
								  m_appSignalManager,
								  theApp.mainWindow()->tuningSignalManager(),
								  theApp.mainWindow()->tuningConnection(),
								  theApp.mainWindow()->tuningAuthorization(),
								  &theApp.mainWindow()->configController(),
								  &theApp.mainWindow()->monitorCentralWidget());
	return;
}
