#include "TuningModel.h"
#include <QKeyEvent>


Hash TuningModelHashSet::firstHash() const
{
	for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
	{
		if (hash[c] != UNDEFINED_HASH)
		{
			return hash[c];
		}
	}

	return UNDEFINED_HASH;
}

int TuningModelHashSet::hashCount() const
{
	int result = 0;

	for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
	{
		if (hash[c] != UNDEFINED_HASH)
		{
			result++;
		}
	}

	return result;
}

//
// TuningItemSorter
//

TuningModelSorter::TuningModelSorter(TuningModelColumns column, Qt::SortOrder order, const TuningModel* model, const TuningSignalManager* tuningSignalManager):
	m_column(column),
	m_order(order),
	m_tuningSignalManager(tuningSignalManager),
	m_model(model)
{
}

bool TuningModelSorter::sortFunction(const TuningModelHashSet& set1, const TuningModelHashSet& set2, TuningModelColumns column, Qt::SortOrder order) const
{
	QVariant v1;
	QVariant v2;

	static AppSignalParam asp1;
	static AppSignalParam asp2;

	static TuningSignalState tss1;
	static TuningSignalState tss2;

	int valueColumnIndex = 0;

	int columnIndex = static_cast<int>(column);

	if (columnIndex >= static_cast<int>(TuningModelColumns::ValueFirst) && columnIndex <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		valueColumnIndex = columnIndex - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumnIndex < 0 || valueColumnIndex >= MAX_VALUES_COLUMN_COUNT)
		{
			assert(false);
			return false;
		}
	}

	bool ok1 = false;
	bool ok2 = false;

	asp1 = m_tuningSignalManager->signalParam(set1.firstHash(), &ok1);
	asp2 = m_tuningSignalManager->signalParam(set2.firstHash(), &ok2);

	if (ok1 == true && set1.hash[valueColumnIndex] != UNDEFINED_HASH)
	{
		tss1 = m_tuningSignalManager->state(set1.hash[valueColumnIndex], &ok1);
	}

	if (ok2 == true && set2.hash[valueColumnIndex] != UNDEFINED_HASH)
	{
		tss2 = m_tuningSignalManager->state(set2.hash[valueColumnIndex], &ok2);
	}

	//

	TuningModelColumns sortColumn = column;

	if (sortColumn >= TuningModelColumns::ValueFirst && sortColumn <= TuningModelColumns::ValueLast)
	{
		sortColumn = TuningModelColumns::ValueFirst;
	}

	switch (sortColumn)
	{
		case TuningModelColumns::CustomAppSignalID:
		{
			v1 = asp1.customSignalId();
			v2 = asp2.customSignalId();
		}
		break;
		case TuningModelColumns::EquipmentID:
		{
			v1 = asp1.equipmentId();
			v2 = asp2.equipmentId();
		}
		break;
	case TuningModelColumns::AppSignalID:
		{
			v1 = asp1.appSignalId();
			v2 = asp2.appSignalId();
		}
		break;
	case TuningModelColumns::Caption:
		{
			v1 = asp1.caption();
			v2 = asp2.caption();
		}
		break;
	case TuningModelColumns::Units:
		{
			v1 = asp1.unit();
			v2 = asp2.unit();
		}
		break;
	case TuningModelColumns::Type:
		{
			v1 = static_cast<int>(asp1.toTuningType());
			v2 = static_cast<int>(asp2.toTuningType());
		}
		break;

	case TuningModelColumns::Default:
		{
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				double tv1 = m_model->defaultValue(asp1).toDouble();
				double tv2 = m_model->defaultValue(asp2).toDouble();

				if (tv1 == tv2)
				{
					return asp1.customSignalId() < asp2.customSignalId();
				}

				if (order == Qt::AscendingOrder)
					return tv1 < tv2;
				else
					return tv1 > tv2;
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
		}
		break;
	case TuningModelColumns::ValueFirst:
		{
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				double tv1 = tss1.value().toDouble();
				double tv2 = tss2.value().toDouble();

				if (tv1 == tv2)
				{
					return asp1.customSignalId() < asp2.customSignalId();
				}

				if (order == Qt::AscendingOrder)
					return tv1 < tv2;
				else
					return tv1 > tv2;
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
		}
		break;
	case TuningModelColumns::LowLimit:
		{
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				double tv1 = asp1.tuningLowBound().toDouble();
				double tv2 = asp2.tuningLowBound().toDouble();

				if (tv1 == tv2)
				{
					return asp1.customSignalId() < asp2.customSignalId();
				}

				if (order == Qt::AscendingOrder)
					return tv1 < tv2;
				else
					return tv1 > tv2;
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
		}
		break;
	case TuningModelColumns::HighLimit:
		{
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				double tv1 = asp1.tuningHighBound().toDouble();
				double tv2 = asp2.tuningHighBound().toDouble();

				if (tv1 == tv2)
				{
					return asp1.customSignalId() < asp2.customSignalId();
				}

				if (order == Qt::AscendingOrder)
					return tv1 < tv2;
				else
					return tv1 > tv2;
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
		}
		break;
	case TuningModelColumns::Valid:
		{
			v1 = tss1.valid();
			v2 = tss2.valid();
		}
		break;
	case TuningModelColumns::OutOfRange:
		{
			v1 = tss1.outOfRange();
			v2 = tss2.outOfRange();
		}
		break;
	default:
		assert(false);
		return false;
	}

	if (v1 == v2)
	{
		return asp1.customSignalId() < asp2.customSignalId();
	}

	if (order == Qt::AscendingOrder)
		return v1 < v2;
	else
		return v1 > v2;
}

//
// TuningItemModel
//

TuningModel::TuningModel(TuningSignalManager* tuningSignalManager, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent)
	:QAbstractTableModel(parent),
	  m_tuningSignalManager(tuningSignalManager)
{
	// Fill column names

	m_columnsNamesMap[TuningModelColumns::CustomAppSignalID] = tr("CustomAppSignalID");
	m_columnsNamesMap[TuningModelColumns::EquipmentID] = tr("EquipmentID");
	m_columnsNamesMap[TuningModelColumns::AppSignalID] = tr("AppSignalID");
	m_columnsNamesMap[TuningModelColumns::Caption] = tr("Caption");
	m_columnsNamesMap[TuningModelColumns::Units] = tr("Units");
	m_columnsNamesMap[TuningModelColumns::Type] = tr("Type");

	for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
	{
		int valueColumn = static_cast<int>(TuningModelColumns::ValueFirst) + c;

		if (c == 0 && valueColumnsAppSignalIdSuffixes.empty() == true)
		{
			m_columnsNamesMap[static_cast<TuningModelColumns>(valueColumn)] = tr("Value");
		}
		else
		{
			m_columnsNamesMap[static_cast<TuningModelColumns>(valueColumn)] = tr("Value %1").arg(c);
		}
	}

	m_columnsNamesMap[TuningModelColumns::Type] = tr("Type");
	m_columnsNamesMap[TuningModelColumns::LowLimit] = tr("LowLimit");
	m_columnsNamesMap[TuningModelColumns::HighLimit] = tr("HighLimit");
	m_columnsNamesMap[TuningModelColumns::Default] = tr("Default");
	m_columnsNamesMap[TuningModelColumns::Valid] = tr("Valid");
	m_columnsNamesMap[TuningModelColumns::OutOfRange] = tr("OutOfRange");

	// Convert suffixes strings to string lists

	for (const QString& s : valueColumnsAppSignalIdSuffixes)
	{
		m_valueColumnAppSignalIdSuffixes.push_back(s.split(';', QString::SkipEmptyParts));
	}
}

TuningModel::~TuningModel()
{
	if (m_font != nullptr)
	{
		delete m_font;
		m_font = nullptr;
	}

	if (m_importantFont != nullptr)
	{
		delete m_importantFont;
		m_importantFont = nullptr;
	}
}

TuningValue TuningModel::defaultValue(const AppSignalParam& asp) const
{
	auto it = m_defaultValues.find(asp.hash());
	if (it != m_defaultValues.end())
	{
		return it->second;
	}

	return asp.tuningDefaultValue();
}

void TuningModel::setDefaultValues(const std::vector<std::pair<Hash, TuningValue>>& values)
{
	m_defaultValues.clear();

	for (auto value : values)
	{
		m_defaultValues[value.first] = value.second;
	}
}

std::vector<Hash> TuningModel::allHashes() const
{
	return m_allHashes;
}

void TuningModel::setHashes(std::vector<Hash>& hashes)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		m_hashSets.clear();

		endRemoveRows();
	}

	m_allHashes.swap(hashes);

	// Build m_hashSets storage

	std::vector<TuningModelHashSet> hashSets;

	int channelCount = static_cast<int>(m_valueColumnAppSignalIdSuffixes.size());

	if (channelCount == 0)
	{
		// Single channel mode - all hashes are copied to own set

		hashSets.resize(m_allHashes.size());

		int h = 0;

		for (const Hash& hash : m_allHashes)
		{
			hashSets[h++].hash[0] = hash;
		}

	}
	else
	{
		// Multi channel mode - hashes are distributed to sets by channels

		m_generalHashToHashSetMap.clear();

		for (const Hash& hash : m_allHashes)
		{
			// Determine channel

			int hashChannel = -1;

			auto hashChannelIt = m_hashToChannelMap.find(hash);
			if (hashChannelIt == m_hashToChannelMap.end())
			{
				// Channel is unknown, try to determine it

				bool ok = false;

				AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

				QString appSignalId = asp.appSignalId();

				for (int c = 0; c < channelCount; c++)
				{
					const QStringList& channelSuffxes = m_valueColumnAppSignalIdSuffixes[c];

					for (const QString& suffix : channelSuffxes)
					{
						// Get separate parts of suffix set, separated by '+'. Example TZB5_GENERAL_1SF will give TZB5+_1SF

						QStringList suffixSet = suffix.split('+', QString::SkipEmptyParts);

						if (suffixSet.isEmpty() == true)
						{
							continue;
						}

						bool containsSuffix = true;

						for (const QString& suffixPart : suffixSet)
						{
							if (appSignalId.contains(suffixPart) == false)
							{
								containsSuffix = false;
								break;
							}
						}

						if (containsSuffix == false)
						{
							continue;
						}

						hashChannel = c;

						// Calculate general hash (without suffix) and put in into mapped cache

						auto hashGeneralHashIt = m_hashToGeneralHashMap.find(hash);

						if (hashGeneralHashIt == m_hashToGeneralHashMap.end())
						{
							QString generalAppSignalId = appSignalId;

							for (const QString& suffixPart : suffixSet)
							{
								generalAppSignalId.remove(suffixPart);
							}

							m_hashToGeneralHashMap[hash] = ::calcHash(generalAppSignalId);

						}

						break;
					}
				}

				m_hashToChannelMap[hash] = hashChannel;
			}
			else
			{
				hashChannel = hashChannelIt->second;		// Channel is already cached
			}

			if (hashChannel == -1)
			{
				continue;	// No prefixes exist for this signal
			}

			if (hashChannel < 0 || hashChannel >= channelCount)
			{
				assert(false);
				return;
			}

			// Get generalHash

			Hash generalHash = UNDEFINED_HASH;

			auto hashGeneralHashIt = m_hashToGeneralHashMap.find(hash);

			if (hashGeneralHashIt == m_hashToGeneralHashMap.end())
			{
				assert(false);		// General hash must be calculated earlier
				continue;
			}
			else
			{
				generalHash = hashGeneralHashIt->second;		// General hash is already cached
			}

			// Find a set that contains other channels of this signal (key is general hash), or create a new one if not found

			auto hashSetIt = m_generalHashToHashSetMap.find(generalHash);
			if (hashSetIt == m_generalHashToHashSetMap.end())
			{
				TuningModelHashSet set;

				for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
				{
					set.hash[c] = UNDEFINED_HASH;
				}
				set.hash[hashChannel] = hash;

				m_generalHashToHashSetMap[generalHash] = set;
			}
			else
			{
				TuningModelHashSet& set = hashSetIt->second;

				set.hash[hashChannel] = hash;
			}
		}

		// Copy map data to hashSets

		hashSets.reserve(m_generalHashToHashSetMap.size());
		for (auto setIt : m_generalHashToHashSetMap)
		{
			hashSets.push_back(setIt.second);
		}
	}

	//

	if (hashSets.empty() == false)
	{
		int count = static_cast<int>(hashSets.size());

		beginInsertRows(QModelIndex(), 0, count - 1);

		m_hashSets.swap(hashSets);

		endInsertRows();
	}
}

Hash TuningModel::hashByIndex(int row, int valueColumn) const
{
	if (row < 0 || row >= rowCount())
	{
		assert(false);
		return UNDEFINED_HASH;
	}

	if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
	{
		assert(false);
		return UNDEFINED_HASH;
	}

	return m_hashSets[row].hash[valueColumn];
}

const TuningModelHashSet& TuningModel::hashSetByIndex(int row) const
{
	if (row < 0 || row >= rowCount())
	{
		assert(false);
		static TuningModelHashSet err;
		return err;
	}

	return m_hashSets[row];
}

TuningSignalManager* TuningModel::tuningSignalManager()
{
	return m_tuningSignalManager;
}

int TuningModel::valueColumnsCount() const
{
	if (m_valueColumnAppSignalIdSuffixes.empty() == true)
	{
		return 1;
	}

	return static_cast<int>(m_valueColumnAppSignalIdSuffixes.size());
}

void TuningModel::addColumn(TuningModelColumns column)
{
	m_columnsTypes.push_back(column);
}

void TuningModel::removeColumn(TuningModelColumns column)
{
	for (auto it = m_columnsTypes.begin(); it != m_columnsTypes.end(); it++)
	{
		if (*it == column)
		{
			m_columnsTypes.erase(it);
			break;
		}
	}
}

TuningModelColumns TuningModel::columnType(int index) const
{
	if (index < 0 || index >= m_columnsTypes.size())
	{
		assert(false);
		return TuningModelColumns::EquipmentID;
	}

	return m_columnsTypes[index];
}

std::vector<TuningModelColumns> TuningModel::columnTypes()
{
	return m_columnsTypes;

}

void TuningModel::setColumnTypes(std::vector<TuningModelColumns> columnsIndexes)
{
	if (columnCount() > 0)
	{
		beginRemoveColumns(QModelIndex(), 0, columnCount() - 1);

		m_columnsTypes.clear();

		endRemoveColumns();
	}

	beginInsertColumns(QModelIndex(), 0, static_cast<int>(columnsIndexes.size()) - 1);

	m_columnsTypes = columnsIndexes;

	endInsertColumns();

}

void TuningModel::setFont(const QFont& font)
{
	if (m_font != nullptr)
	{
		delete m_font;
	}
	m_font = new QFont(font);
}

void TuningModel::setImportantFont(const QFont& font)
{
	if (m_importantFont != nullptr)
	{
		delete m_importantFont;
	}
	m_importantFont = new QFont(font);
}

int TuningModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_hashSets.size());

}

int TuningModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_columnsTypes.size());

}

void TuningModel::sort(int column, Qt::SortOrder order)
{
	if (column < 0 || column >= m_columnsTypes.size())
	{
		assert(false);
		return;
	}

	if (rowCount() == 0)
	{
		return;
	}

	TuningModelColumns sortColumn = m_columnsTypes[column];

	std::sort(m_hashSets.begin(), m_hashSets.end(), TuningModelSorter(sortColumn, order, this, m_tuningSignalManager));

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

	return;
}

QVariant TuningModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::BackgroundRole)
	{
		return backColor(index);
	}

	if (role == Qt::ForegroundRole)
	{
		return foregroundColor(index);
	}

	if (role == Qt::FontRole)
	{
		int col = index.column();
		if (col < 0 || col >= m_columnsTypes.size())
		{
			assert(false);
			return QVariant();
		}

		int columnType = static_cast<int>(m_columnsTypes[col]);

		if (m_importantFont != nullptr && (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast)))
		{
			return* m_importantFont;
		}

		if (m_font != nullptr)
		{
			return* m_font;
		}
	}

	if (role == Qt::TextAlignmentRole)
	{

		int col = index.column();
		if (col < 0 || col >= m_columnsTypes.size())
		{
			assert(false);
			return QVariant();
		}

		int columnType = static_cast<int>(m_columnsTypes[col]);

		if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst))
		{
			return Qt::AlignCenter;
		}
		return Qt::AlignLeft + Qt::AlignVCenter;
	}

	if (role == Qt::DisplayRole)
	{

		int col = index.column();
		if (col < 0 || col >= m_columnsTypes.size())
		{
			assert(false);
			return QVariant();
		}

		int row = index.row();
		if (row >= rowCount())
		{
			assert(false);
			return QVariant();
		}

		const TuningModelHashSet& hashSet = m_hashSets[row];

		//QString str = QString("%1:%2").arg(row).arg(col);
		//qDebug() << str;

		Hash aspHash = hashSet.firstHash();

		int hashCount = hashSet.hashCount();

		bool ok = false;

		const AppSignalParam asp = m_tuningSignalManager->signalParam(aspHash, &ok);

		int columnType = static_cast<int>(m_columnsTypes[col]);

		if (columnType == static_cast<int>(TuningModelColumns::CustomAppSignalID))
		{
			if (hashCount > 1)
			{
				return tr("%1 [+%2]").arg(asp.customSignalId()).arg(hashCount - 1);
			}

			return asp.customSignalId();
		}

		if (columnType == static_cast<int>(TuningModelColumns::EquipmentID))
		{
			if (hashCount > 1)
			{
				return tr("%1 [+%2]").arg(asp.equipmentId()).arg(hashCount - 1);
			}

			return asp.equipmentId();
		}

		if (columnType == static_cast<int>(TuningModelColumns::AppSignalID))
		{
			if (hashCount > 1)
			{
				return tr("%1 [+%2]").arg(asp.appSignalId()).arg(hashCount - 1);
			}

			return asp.appSignalId();
		}

		if (columnType == static_cast<int>(TuningModelColumns::Caption))
		{
			return asp.caption();
		}

		if (columnType == static_cast<int>(TuningModelColumns::Units))
		{
			return asp.unit();
		}

		if (columnType == static_cast<int>(TuningModelColumns::Type))
		{
			return asp.tuningDefaultValue().tuningValueTypeString();
		}

		if (columnType == static_cast<int>(TuningModelColumns::Default))
		{
			if (asp.isAnalog())
			{
				return defaultValue(asp).toString(asp.precision());
			}
			else
			{
				return defaultValue(asp).toString();
			}
		}

		//
		// State
		//

		if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
		{
			int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
			if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
			{
				assert(false);
				return QVariant();
			}

			Hash tssHash = hashSet.hash[valueColumn];

			if (tssHash == UNDEFINED_HASH)
			{
				return QVariant();
			}

			const TuningSignalState tss = m_tuningSignalManager->state(tssHash, &ok);

			if (tss.controlIsEnabled() == false)
			{
				return tr("-");
			}
			else
			{
				if (tss.valid() == true)
				{
					TuningValue newValue = m_tuningSignalManager->newValue(tssHash);

					if (asp.isAnalog() == false)
					{
						QString valueString = tss.value().toString();

						if (m_tuningSignalManager->newValueIsUnapplied(tssHash) == true)
						{
							QString editValueString = newValue.toString();
							return tr("%1 => %2").arg(valueString).arg(editValueString);
						}

						if (tss.writeInProgress() == true)
						{
							QString editValueString = newValue.toString();
							return tr("Writing %1").arg(editValueString);
						}

						return valueString;
					}
					else
					{
						QString valueString = tss.value().toString(asp.precision());

						if (m_tuningSignalManager->newValueIsUnapplied(tssHash) == true)
						{
							QString editValueString = newValue.toString(asp.precision());
							return QString("%1 => %2").arg(valueString).arg(editValueString);
						}

						if (tss.writeInProgress() == true)
						{
							QString editValueString = newValue.toString(asp.precision());
							return tr("Writing %1").arg(editValueString);
						}

						if (tss.outOfRange() == true)
						{
							return tr("RANGE");
						}

						return valueString;
					}
				}
				else
				{
					return "?";
				}
			}
		}

		const TuningModelHashSet& hashes = hashSetByIndex(row);

		QString result;

		for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
		{
			const Hash tssHash = hashes.hash[c];

			if (tssHash == UNDEFINED_HASH)
			{
				continue;
			}

			const TuningSignalState tss = m_tuningSignalManager->state(tssHash, &ok);

			if (columnType == static_cast<int>(TuningModelColumns::LowLimit))
			{
				if (asp.isAnalog())
				{
					if (tss.limitsUnbalance(asp) == true)
					{
						result = tr("Base %1, read %2")
								.arg(asp.tuningLowBound().toString(asp.precision()))
								.arg(tss.lowBound().toString(asp.precision()));
						break;
					}
					else
					{
						if (c == 0)
						{
							result = asp.tuningLowBound().toString(asp.precision());
						}
					}
				}
			}

			if (columnType == static_cast<int>(TuningModelColumns::HighLimit))
			{
				if (asp.isAnalog())
				{
					if (tss.limitsUnbalance(asp) == true)
					{
						result = tr("Base %1, read %2")
								.arg(asp.tuningHighBound().toString(asp.precision()))
								.arg(tss.highBound().toString(asp.precision()));
						break;
					}
					else
					{
						if (c == 0)
						{
							result = asp.tuningHighBound().toString(asp.precision());
						}
					}
				}
			}

			if (columnType == static_cast<int>(TuningModelColumns::Valid))
			{
				if (tss.valid() == false)
				{
					result = tr("NO");
					break;
				}
			}

			if (columnType == static_cast<int>(TuningModelColumns::OutOfRange))
			{
				if (tss.outOfRange() == true)
				{
					result = tr("RANGE");
					break;
				}
			}
		}	// c

		return result;
	}
	return QVariant();
}

QBrush TuningModel::backColor(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QBrush();
}

QBrush TuningModel::foregroundColor(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QBrush();
}


QVariant TuningModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section < 0 || section >= m_columnsTypes.size())
		{
			assert(false);
			return QVariant();
		}

		TuningModelColumns displayColumn = static_cast<TuningModelColumns>(m_columnsTypes[section]);

		if (m_columnsNamesMap.find(displayColumn) == m_columnsNamesMap.end())
		{
			assert(false);
			return QString();
		}

		return m_columnsNamesMap.at(displayColumn);
	}

	return QVariant();
}

//
// DialogInputTuningValue
//

DialogInputTuningValue::DialogInputTuningValue(TuningValue value, TuningValue defaultValue, bool sameValue, TuningValue lowLimit, TuningValue highLimit, int decimalPlaces, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_value(value),
	m_defaultValue(defaultValue),
	m_lowLimit(lowLimit),
	m_highLimit(highLimit),
	m_decimalPlaces(decimalPlaces)
{

	m_discreteCheck = new QCheckBox();
	connect(m_discreteCheck, &QCheckBox::stateChanged, this, &DialogInputTuningValue::on_m_checkBox_stateChanged);

	m_analogEdit = new QLineEdit();

	m_buttonDefault = new QPushButton();
	connect(m_buttonDefault, &QPushButton::clicked, this, &DialogInputTuningValue::on_m_buttonDefault_clicked);

	m_buttonOK = new QPushButton(tr("OK"));
	m_buttonOK->setDefault(true);
	connect(m_buttonOK, &QPushButton::clicked, this, &QDialog::accept);

	m_buttonCancel = new QPushButton(tr("Cancel"));
	connect(m_buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

	QHBoxLayout* valueLayout = new QHBoxLayout();
	valueLayout->addWidget(m_discreteCheck);
	valueLayout->addWidget(m_analogEdit);

	QHBoxLayout* controlLayout = new QHBoxLayout();
	controlLayout->addWidget(m_buttonDefault);
	controlLayout->addStretch();
	controlLayout->addWidget(m_buttonOK);
	controlLayout->addWidget(m_buttonCancel);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(valueLayout);
	mainLayout->addLayout(controlLayout);

	setLayout(mainLayout);

	setMinimumWidth(400);

	//

	m_discreteCheck->setVisible(value.type() == TuningValueType::Discrete);
	m_analogEdit->setVisible(value.type() != TuningValueType::Discrete);

	if (value.type() == TuningValueType::Discrete)
	{
		setWindowTitle(tr("Enter the value:"));

		m_discreteCheck->blockSignals(true);

		if (sameValue == true)
		{
			m_discreteCheck->setChecked(value.discreteValue() != 0);
			m_discreteCheck->setText(value.toString());
		}
		else
		{
			m_discreteCheck->setTristate(true);
			m_discreteCheck->setCheckState(Qt::PartiallyChecked);
			m_discreteCheck->setText(tr("Different values"));
		}

		m_discreteCheck->blockSignals(false);

		m_buttonDefault->setText(tr("Default: ") + m_defaultValue.toString());
	}
	else
	{
		QString str = tr("Enter the value (%1 - %2):")
				.arg(m_lowLimit.toString(decimalPlaces))
				.arg(m_highLimit.toString(decimalPlaces));

		setWindowTitle(str);

		if (sameValue == true)
		{
			m_analogEdit->setText(value.toString(decimalPlaces));
			m_analogEdit->selectAll();
		}

		m_buttonDefault->setText(tr("Default: ") + m_defaultValue.toString(m_decimalPlaces));
	}
}

DialogInputTuningValue::~DialogInputTuningValue()
{
}

void DialogInputTuningValue::accept()
{
	if (m_value.type() == TuningValueType::Discrete)
	{
		if (m_discreteCheck->checkState() == Qt::PartiallyChecked)
		{
			QMessageBox::critical(this, tr("Error"), tr("Please select the value."));
			return;
		}

		if (m_discreteCheck->checkState() == Qt::Checked)
		{
			m_value.setDiscreteValue(1);
		}
		else
		{
			m_value.setDiscreteValue(0);
		}
	}
	else
	{
		QString text = m_analogEdit->text();
		if (text.isEmpty() == true)
		{
			QMessageBox::critical(this, tr("Error"), tr("Please enter the value."));
			return;
		}

		bool ok = false;

		TuningValue newValue;
		newValue.setType(m_value.type());

		switch (m_value.type())
		{
		case TuningValueType::SignedInt32:
			newValue.setInt32Value(text.toInt(&ok));
			break;
		case TuningValueType::SignedInt64:
			newValue.setInt64Value(text.toInt(&ok));
			break;
		case TuningValueType::Float:
			newValue.setFloatValue(text.toFloat(&ok));
			break;
		case TuningValueType::Double:
			newValue.setDoubleValue(text.toDouble(&ok));
			break;
		default:
			assert(false);
			return;

		}

		if (ok == false)
		{
			QMessageBox::critical(this, tr("Error"), tr("The value is incorrect."));
			return;
		}

		if (newValue < m_lowLimit || newValue > m_highLimit)
		{
			QMessageBox::critical(this, tr("Error"), tr("The value is out of range."));
			return;
		}

		m_value = newValue;

	}

	QDialog::accept();
}

void DialogInputTuningValue::on_m_checkBox_stateChanged(int state)
{
	m_discreteCheck->setText(state == Qt::Checked ? tr("1") : tr("0"));
}

void DialogInputTuningValue::on_m_buttonDefault_clicked()
{
	if (m_value.type() == TuningValueType::Discrete)
	{
		bool defaultState = m_defaultValue.discreteValue() == 0 ? false : true;

		m_discreteCheck->setChecked(defaultState);

		m_discreteCheck->setText(defaultState ? tr("1") : tr("0"));
	}
	else
	{
		m_analogEdit->setText(m_defaultValue.toString(m_decimalPlaces));
	}
}
