#include "SignalListEditorPrivate.h"
#include "TextResource.h"

namespace
{
	using namespace AppSignalLists;

	class SignalModelSorter
	{
	public:
		SignalModelSorter(ISignalManager& tuningSignalManager, SignalsModel::Columns column, Qt::SortOrder order);
		bool sortFunction(Hash hash1, Hash hash2) const;
		bool operator()(Hash hash1, Hash hash2) const { return sortFunction(hash1, hash2); }

	private:
		ISignalManager& m_signalManager;
		SignalsModel::Columns m_column = SignalsModel::Columns::AppSignalID;
		Qt::SortOrder m_order = Qt::AscendingOrder;
	};

	class AppSignalListModelSorter
	{
	public:
		AppSignalListModelSorter(ISignalManager& tuningSignalManager,
								 const AppSignalList* appSignallist,
								 AppSignalListModel::Columns column,
								 Qt::SortOrder order);

		bool sortFunction(Hash hash1, Hash hash2) const;
		bool operator()(Hash hash1, Hash hash2) const { return sortFunction(hash1, hash2); }

	private:
		ISignalManager& m_signalManager;
		const AppSignalList* m_appSignalList = nullptr;

		AppSignalListModel::Columns m_column = AppSignalListModel::Columns::AppSignalID;
		Qt::SortOrder m_order = Qt::AscendingOrder;
	};

	//
	// SignalModelSorter
	//
	SignalModelSorter::SignalModelSorter(ISignalManager& signalManager, SignalsModel::Columns column, Qt::SortOrder order) :
		m_signalManager(signalManager),
		m_column(column),
		m_order(order)
	{
	}

	bool SignalModelSorter::sortFunction(Hash hash1, Hash hash2) const
	{
		QVariant v1;
		QVariant v2;

		bool ok1 = false;
		bool ok2 = false;

		AppSignalParam asp1 = m_signalManager.signalParam(hash1, &ok1);
		AppSignalParam asp2 = m_signalManager.signalParam(hash2, &ok2);

		//

		switch (m_column)
		{
		case SignalsModel::Columns::CustomAppSignalID:
			v1 = asp1.customSignalId();
			v2 = asp2.customSignalId();
			break;

		case SignalsModel::Columns::EquipmentID:
			v1 = asp1.lmEquipmentId();
			v2 = asp2.lmEquipmentId();
			break;

		case SignalsModel::Columns::AppSignalID:
			v1 = asp1.appSignalId();
			v2 = asp2.appSignalId();
			break;

		case SignalsModel::Columns::Caption:
			v1 = asp1.caption();
			v2 = asp2.caption();
			break;

		case SignalsModel::Columns::Units:
			v1 = asp1.unit();
			v2 = asp2.unit();
			break;

		case SignalsModel::Columns::Type:
			v1 = static_cast<int>(asp1.tuningType());
			v2 = static_cast<int>(asp2.tuningType());
			break;

		case SignalsModel::Columns::Default:
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				v1 = asp1.tuningDefaultValue().toDouble();
				v2 = asp2.tuningDefaultValue().toDouble();
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
			break;

		case SignalsModel::Columns::LowLimit:
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				v1 = asp1.tuningLowBound().toDouble();
				v2 = asp2.tuningLowBound().toDouble();
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
			break;
		case SignalsModel::Columns::HighLimit:
			if (asp1.isAnalog() == asp2.isAnalog())
			{
				v1 = asp1.tuningHighBound().toDouble();
				v2 = asp2.tuningHighBound().toDouble();
			}
			else
			{
				v1 = asp1.isAnalog();
				v2 = asp2.isAnalog();
			}
			break;
		default:
			assert(false);
			return false;
		}

		bool result = false;

		if (v1.userType() != v2.userType())
		{
			Q_ASSERT(false);
			result = asp1.customSignalId() < asp2.customSignalId();
		}
		else
		{
			if (v1 == v2)
			{
				result = asp1.customSignalId() < asp2.customSignalId();
			}
			else
			{
				switch (v1.userType())
				{
				case QMetaType::Bool:
					result = v1.toBool() < v2.toBool();
					break;
				case QMetaType::QString:
					result = v1.toString() < v2.toString();
					break;
				case QMetaType::Int:
					result = v1.toInt() < v2.toInt();
					break;
				case QMetaType::UInt:
					result = v1.toUInt() < v2.toUInt();
					break;
				case QMetaType::LongLong:
					result = v1.toLongLong() < v2.toLongLong();
					break;
				case QMetaType::ULongLong:
					result = v1.toULongLong() < v2.toULongLong();
					break;
				case QMetaType::Float:
					result = v1.toFloat() < v2.toFloat();
					break;
				case QMetaType::Double:
					result = v1.toDouble() < v2.toDouble();
					break;
				default:
					break;
				}
			}
		}

		if (m_order == Qt::DescendingOrder)
		{
			result = !result;
		}

		return result;
	}

	//
	// AppSignalListModelSorter
	//
	AppSignalListModelSorter::AppSignalListModelSorter(ISignalManager& signalManager,
													   const AppSignalList* appSignalList,
													   AppSignalListModel::Columns column,
													   Qt::SortOrder order) :
		m_signalManager(signalManager),
		m_appSignalList(appSignalList),
		m_column(column),
		m_order(order)
	{
	}

	bool AppSignalListModelSorter::sortFunction(Hash hash1, Hash hash2) const
	{
		QVariant v1;
		QVariant v2;

		bool ok1 = false;
		bool ok2 = false;

		AppSignalParam asp1 = m_signalManager.signalParam(hash1, &ok1);
		AppSignalParam asp2 = m_signalManager.signalParam(hash2, &ok2);

		const AppSignalListItem& item1 = m_appSignalList->itemByHash(hash1);
		const AppSignalListItem& item2 = m_appSignalList->itemByHash(hash2);

		//

		switch (m_column)
		{
		case AppSignalListModel::Columns::CustomAppSignalID:
			{
				v1 = asp1.customSignalId();
				v2 = asp2.customSignalId();
			}
			break;

		case AppSignalListModel::Columns::EquipmentID:
			{
				v1 = asp1.lmEquipmentId();
				v2 = asp2.lmEquipmentId();
			}
			break;

		case AppSignalListModel::Columns::AppSignalID:
			{
				v1 = asp1.appSignalId();
				v2 = asp2.appSignalId();
			}
			break;

		case AppSignalListModel::Columns::Caption:
			{
				v1 = asp1.caption();
				v2 = asp2.caption();
			}
			break;

		case AppSignalListModel::Columns::Units:
			{
				v1 = asp1.unit();
				v2 = asp2.unit();
			}
			break;

		case AppSignalListModel::Columns::Type:
			{
				v1 = static_cast<int>(asp1.tuningType());
				v2 = static_cast<int>(asp2.tuningType());
			}
			break;

		case AppSignalListModel::Columns::Value:
			{
				if (item1.hasValue() == item2.hasValue() && item1.hasValue() == true)
				{
					v1 = item1.value().toDouble();
					v2 = item2.value().toDouble();
				}
				else
				{
					v1 = item1.hasValue();
					v2 = item2.hasValue();
				}
			}
			break;

		case AppSignalListModel::Columns::LowLimit:
			{
				if (asp1.isAnalog() == asp2.isAnalog())
				{
					v1 = asp1.tuningLowBound().toDouble();
					v2 = asp2.tuningLowBound().toDouble();
				}
				else
				{
					v1 = asp1.isAnalog();
					v2 = asp2.isAnalog();
				}
			}
			break;
		case AppSignalListModel::Columns::HighLimit:
			{
				if (asp1.isAnalog() == asp2.isAnalog())
				{
					v1 = asp1.tuningHighBound().toDouble();
					v2 = asp2.tuningHighBound().toDouble();
				}
				else
				{
					v1 = asp1.isAnalog();
					v2 = asp2.isAnalog();
				}
			}
			break;
		default:
			assert(false);
			return false;
		}

		bool result = false;

		if (v1.userType() != v2.userType())
		{
			Q_ASSERT(false);
			result = asp1.customSignalId() < asp2.customSignalId();
		}
		else
		{
			if (v1 == v2)
			{
				result = asp1.customSignalId() < asp2.customSignalId();
			}
			else
			{
				switch (v1.userType())
				{
				case QMetaType::Bool:
					result = v1.toBool() < v2.toBool();
					break;
				case QMetaType::QString:
					result = v1.toString() < v2.toString();
					break;
				case QMetaType::Int:
					result = v1.toInt() < v2.toInt();
					break;
				case QMetaType::UInt:
					result = v1.toUInt() < v2.toUInt();
					break;
				case QMetaType::LongLong:
					result = v1.toLongLong() < v2.toLongLong();
					break;
				case QMetaType::ULongLong:
					result = v1.toULongLong() < v2.toULongLong();
					break;
				case QMetaType::Float:
					result = v1.toFloat() < v2.toFloat();
					break;
				case QMetaType::Double:
					result = v1.toDouble() < v2.toDouble();
					break;
				default:
					break;
				}
			}
		}

		if (m_order == Qt::DescendingOrder)
		{
			result = !result;
		}

		return result;
	}
} // namespace

namespace AppSignalLists
{
	//
	// SignalsModel
	//
	SignalsModel::SignalsModel(ISignalManager& signalManager) :
		QAbstractTableModel(),
		m_signalManager(signalManager)
	{
	}

	TuningValue SignalsModel::defaultValue(const AppSignalParam& asp) const
	{
		auto it = m_defaultValues.find(asp.hash());
		if (it != m_defaultValues.end())
		{
			return it->second;
		}

		return asp.tuningDefaultValue();
	}

	void SignalsModel::setDefaultValues(const std::vector<std::pair<Hash, TuningValue>>& values)
	{
		m_defaultValues.clear();

		for (const auto& value : values)
		{
			m_defaultValues[value.first] = value.second;
		}
	}

	std::vector<Hash> SignalsModel::allHashes() const
	{
		return m_allHashes;
	}

	void SignalsModel::setHashes(std::vector<Hash>& hashes)
	{
		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

			m_allHashes.clear();

			endRemoveRows();
		}

		if (hashes.empty() == false)
		{
			int count = static_cast<int>(hashes.size());

			beginInsertRows(QModelIndex(), 0, count - 1);

			m_allHashes.swap(hashes);

			endInsertRows();
		}
	}

	Hash SignalsModel::hash(int row) const
	{
		if (row < 0 || row >= m_allHashes.size())
		{
			Q_ASSERT(false);
			return UNDEFINED_HASH;
		}
		return m_allHashes[row];
	}

	QString SignalsModel::columnText(int index) const
	{
		if (index < 0 || index >= columnCount())
		{
			Q_ASSERT(false);
			return QString();
		}

		return headerData(index, Qt::Horizontal, Qt::DisplayRole).toString();
	}

	QString SignalsModel::cellText(int column, int row) const
	{
		if (column < 0 || column >= columnCount())
		{
			Q_ASSERT(false);
			return QString();
		}
		if (row < 0 || row >= rowCount())
		{
			Q_ASSERT(false);
			return QString();
		}

		return data(createIndex(row, column), Qt::DisplayRole).toString();
	}

	int SignalsModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_allHashes.size());
	}

	int SignalsModel::columnCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(Columns::Count);
	}

	void SignalsModel::sort(int column, Qt::SortOrder order)
	{
		if (rowCount() == 0)
		{
			return;
		}

		SignalModelSorter sorter(m_signalManager, static_cast<Columns>(column), order);
		std::sort(m_allHashes.begin(), m_allHashes.end(), sorter);

		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

		return;
	}

	QVariant SignalsModel::data(const QModelIndex& index, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			int col = index.column();
			if (col < 0 || col >= static_cast<int>(Columns::Count))
			{
				assert(false);
				return QVariant();
			}

			int row = index.row();
			if (row < 0 || row >= rowCount())
			{
				assert(false);
				return QVariant();
			}

			Hash aspHash = m_allHashes[row];

			bool ok = false;

			const AppSignalParam asp = m_signalManager.signalParam(aspHash, &ok);

			int columnType = col;

			if (columnType == static_cast<int>(Columns::CustomAppSignalID))
			{
				return asp.customSignalId();
			}

			if (columnType == static_cast<int>(Columns::EquipmentID))
			{
				return asp.equipmentId();
			}

			if (columnType == static_cast<int>(Columns::AppSignalID))
			{
				return asp.appSignalId();
			}

			if (columnType == static_cast<int>(Columns::Caption))
			{
				return asp.caption();
			}

			if (columnType == static_cast<int>(Columns::Units))
			{
				return asp.unit();
			}

			if (columnType == static_cast<int>(Columns::LowLimit))
			{
				if (asp.enableTuning() == true)
				{
					return asp.tuningLowBound().toString();
				}

				return QString::number(asp.lowEngineeringUnits(), 'g', asp.precision());
			}

			if (columnType == static_cast<int>(Columns::HighLimit))
			{
				if (asp.enableTuning() == true)
				{
					return asp.tuningHighBound().toString();
				}

				return QString::number(asp.highEngineeringUnits(), 'g', asp.precision());
			}

			if (columnType == static_cast<int>(Columns::Type))
			{
				if (asp.isAnalog() == true)
				{
					return "Analog";
				}

				if (asp.isDiscrete() == true)
				{
					return "Discrete";
				}

				if (asp.isBus() == true)
				{
					return "Bus";
				}

				Q_ASSERT(false);
				return "Other";
			}

			if (columnType == static_cast<int>(Columns::Default))
			{
				if (asp.enableTuning() == true)
				{
					if (asp.isAnalog())
					{
						return defaultValue(asp).toString(E::AnalogFormat::g_9_or_9e, asp.precision());
					}

					if (asp.isDiscrete() == true)
					{
						return defaultValue(asp).toString();
					}

					Q_ASSERT(false);
					return "";
				}
				else
				{
					return "-";
				}
			}
		}

		return QVariant();
	}

	QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		{
			if (section < 0 || section >= static_cast<int>(Columns::Count))
			{
				assert(false);
				return QVariant();
			}

			switch (static_cast<Columns>(section))
			{
			case Columns::CustomAppSignalID:
				return col_CustomAppSignalId;
			case Columns::EquipmentID:
				return col_EquipmentID;
			case Columns::AppSignalID:
				return col_AppSignalId;
			case Columns::Caption:
				return col_Caption;
			case Columns::Units:
				return col_Units;
			case Columns::Type:
				return col_Type;
			case Columns::LowLimit:
				return col_LowLimit;
			case Columns::HighLimit:
				return col_HighLimit;
			case Columns::Default:
				return col_Default;
			default:
				Q_ASSERT(false);
			}
		}
		return QVariant();
	}


	//
	// AppSignalListModel
	//
	AppSignalListModel::AppSignalListModel(ISignalManager& signalManager) :
		QAbstractTableModel(),
		m_signalManager(signalManager)
	{
	}

	const AppSignalList* AppSignalListModel::list() const
	{
		return m_appSignalList;
	}

	void AppSignalListModel::setList(AppSignalList* list)
	{
		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

			m_appSignalList = nullptr;
			m_allHashes.clear();

			endRemoveRows();
		}

		if (list != nullptr)
		{
			if (list->count() > 0)
			{
				beginInsertRows(QModelIndex(), 0, list->count() - 1);
			}

			m_appSignalList = list;

			auto itemsHashesSet = m_appSignalList->itemsHashes();
			for (Hash hash : itemsHashesSet) 
			{
				m_allHashes.push_back(hash);
			}

			if (list->count() > 0)
			{
				endInsertRows();
			}
		}
	}

	bool AppSignalListModel::itemExists(Hash hash) const
	{
		if (m_appSignalList == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		return m_appSignalList->itemExists(hash);
	}

	Hash AppSignalListModel::itemHash(int row) const
	{
		if (row < 0 || row >= m_allHashes.size())
		{
			Q_ASSERT(false);
			return UNDEFINED_HASH;
		}
		return m_allHashes[row];
	}

	bool AppSignalListModel::add(const AppSignalListItem& item)
	{
		auto it = std::find(m_allHashes.begin(), m_allHashes.end(), item.appSignalHash());
		if (it != m_allHashes.end())
		{
			Q_ASSERT(false);
			return false;
		}

		beginInsertRows(QModelIndex(), static_cast<int>(m_allHashes.size()), static_cast<int>(m_allHashes.size()));

		m_allHashes.push_back(item.appSignalHash());

		endInsertRows();

		return true;
	}

	bool AppSignalListModel::remove(Hash hash)
	{
		auto it = std::find(m_allHashes.begin(), m_allHashes.end(), hash);
		if (it == m_allHashes.end())
		{
			Q_ASSERT(false);
			return false;
		}

		int index = std::distance(m_allHashes.begin(), it);

		beginRemoveRows(QModelIndex(), index, index);

		m_allHashes.erase(it);

		endRemoveRows();

		return true;
	}

	QString AppSignalListModel::columnText(int index) const
	{
		if (index < 0 || index >= columnCount())
		{
			Q_ASSERT(false);
			return QString();
		}

		return headerData(index, Qt::Horizontal, Qt::DisplayRole).toString();
	}

	QString AppSignalListModel::cellText(int column, int row) const
	{
		if (column < 0 || column >= columnCount())
		{
			Q_ASSERT(false);
			return QString();
		}
		if (row < 0 || row >= rowCount())
		{
			Q_ASSERT(false);
			return QString();
		}

		return data(createIndex(row, column), Qt::DisplayRole).toString();
	}

	int AppSignalListModel::rowCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(m_allHashes.size());
	}

	int AppSignalListModel::columnCount(const QModelIndex& parent) const
	{
		Q_UNUSED(parent);
		return static_cast<int>(Columns::Count);
	}

	void AppSignalListModel::sort(int column, Qt::SortOrder order)
	{
		if (rowCount() == 0)
		{
			return;
		}

		AppSignalListModelSorter sorter(m_signalManager, m_appSignalList, static_cast<Columns>(column), order);
		std::sort(m_allHashes.begin(), m_allHashes.end(), sorter);

		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

		return;
	}

	QVariant AppSignalListModel::data(const QModelIndex& index, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			int col = index.column();
			if (col < 0 || col >= static_cast<int>(Columns::Count))
			{
				assert(false);
				return QVariant();
			}

			int row = index.row();
			if (row < 0 || row >= rowCount())
			{
				assert(false);
				return QVariant();
			}

			Hash hash = m_allHashes[row];

			bool found = false;
			const AppSignalParam asp = m_signalManager.signalParam(hash, &found);

			int columnType = col;

			if (columnType == static_cast<int>(Columns::AppSignalID))
			{
				if (found == false) 
				{
					const AppSignalListItem& item = m_appSignalList->itemByHash(hash);
					return item.appSignalId();
				}
			
				return asp.appSignalId();
			}

			if (found == false)
			{
				return "?";
			}

			if (columnType == static_cast<int>(Columns::CustomAppSignalID))
			{
				return asp.customSignalId();
			}

			if (columnType == static_cast<int>(Columns::EquipmentID))
			{
				return asp.equipmentId();
			}

			if (columnType == static_cast<int>(Columns::Caption))
			{
				return asp.caption();
			}

			if (columnType == static_cast<int>(Columns::Units))
			{
				return asp.unit();
			}

			if (columnType == static_cast<int>(Columns::LowLimit))
			{
				if (asp.enableTuning() == true)
				{
					return asp.tuningLowBound().toString();
				}

				return QString::number(asp.lowEngineeringUnits(), 'f', asp.precision());
			}

			if (columnType == static_cast<int>(Columns::HighLimit))
			{
				if (asp.enableTuning() == true)
				{
					return asp.tuningHighBound().toString();
				}

				return QString::number(asp.highEngineeringUnits(), 'f', asp.precision());
			}

			if (columnType == static_cast<int>(Columns::Type))
			{
				if (asp.isAnalog() == true)
				{
					return "Analog";
				}

				if (asp.isDiscrete() == true)
				{
					return "Discrete";
				}

				if (asp.isBus() == true)
				{
					return "Bus";
				}

				Q_ASSERT(false);
				return "Other";
			}

			if (columnType == static_cast<int>(Columns::Value))
			{
				if (asp.enableTuning() == true)
				{
					const AppSignalListItem& item = m_appSignalList->itemByHash(hash);
					if (item.hasValue() == true)
					{
						if (asp.isAnalog() == true)
						{
							return item.value().toString(E::AnalogFormat::g_9_or_9e, asp.precision());
						}

						if (asp.isDiscrete())
						{
							return item.value().toString();
						}

						Q_ASSERT(false);
					}
				}
				else
				{
					return "-";
				}
			}
		}
		return QVariant();
	}

	QVariant AppSignalListModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		{
			if (section < 0 || section >= static_cast<int>(Columns::Count))
			{
				assert(false);
				return QVariant();
			}

			switch (static_cast<Columns>(section))
			{
			case Columns::CustomAppSignalID:
				return col_CustomAppSignalId;
			case Columns::EquipmentID:
				return col_EquipmentID;
			case Columns::AppSignalID:
				return col_AppSignalId;
			case Columns::Caption:
				return col_Caption;
			case Columns::Units:
				return col_Units;
			case Columns::Type:
				return col_Type;
			case Columns::LowLimit:
				return col_LowLimit;
			case Columns::HighLimit:
				return col_HighLimit;
			case Columns::Value:
				return col_Value;
			default:
				Q_ASSERT(false);
			}
		}

		return QVariant();
	}

	//
	// DialogAppSignalListValue
	//
	DialogAppSignalListValue::DialogAppSignalListValue(TuningValue value,
													   TuningValue defaultValue,
													   bool sameValue,
													   bool sameDefaultValue,
													   TuningValue lowLimit,
													   TuningValue highLimit,
													   E::AnalogFormat analogFormat,
													   int decimalPlaces,
													   QWidget* parent) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
		m_value(value),
		m_defaultValue(defaultValue),
		m_lowLimit(lowLimit),
		m_highLimit(highLimit),
		m_decimalPlaces(decimalPlaces),
		m_analogFormat(analogFormat)
	{
		m_discreteCheck = new QCheckBox();
		connect(m_discreteCheck, &QCheckBox::stateChanged, this, &DialogAppSignalListValue::onValueCheckStateChanged);

		m_analogEdit = new QLineEdit();

		m_defaultButton = new QPushButton();
		connect(m_defaultButton, &QPushButton::clicked, this, &DialogAppSignalListValue::onValueDefaultClicked);

		m_okButton = new QPushButton(tr("OK"));
		m_okButton->setDefault(true);
		connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);

		m_cancelButton = new QPushButton(tr("Cancel"));
		connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

		QHBoxLayout* valueLayout = new QHBoxLayout();
		valueLayout->addWidget(m_discreteCheck);
		valueLayout->addWidget(m_analogEdit);

		QHBoxLayout* controlLayout = new QHBoxLayout();
		controlLayout->addWidget(m_defaultButton);
		controlLayout->addStretch();
		controlLayout->addWidget(m_okButton);
		controlLayout->addWidget(m_cancelButton);

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

			m_defaultButton->setText(tr("Default: ") + m_defaultValue.toString());
		}
		else
		{
			QString str = tr("Enter the value (%1 - %2):")
							  .arg(m_lowLimit.toString(analogFormat, decimalPlaces))
							  .arg(m_highLimit.toString(analogFormat, decimalPlaces));

			setWindowTitle(str);

			if (sameValue == true)
			{
				m_analogEdit->setText(value.toString(analogFormat, decimalPlaces));
				m_analogEdit->selectAll();
			}

			m_defaultButton->setText(tr("Default: ") + m_defaultValue.toString(analogFormat, m_decimalPlaces));
		}

		if (sameDefaultValue == false)
		{
			m_defaultButton->setText(tr("Default: ") + "...");
			m_defaultButton->setEnabled(false);
		}
	}

	void DialogAppSignalListValue::accept()
	{
		if (m_value.type() == TuningValueType::Discrete)
		{
			if (m_discreteCheck->checkState() == Qt::PartiallyChecked)
			{
				QMessageBox::critical(this, qAppName(), tr("Please select the value."));
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
				QMessageBox::critical(this, qAppName(), tr("Please enter the value."));
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
				QMessageBox::critical(this, qAppName(), tr("The value is incorrect."));
				return;
			}

			if (newValue < m_lowLimit || newValue > m_highLimit)
			{
				QMessageBox::critical(this, qAppName(), tr("The value is out of range."));
				return;
			}

			m_value = newValue;
		}

		QDialog::accept();
	}

	void DialogAppSignalListValue::onValueCheckStateChanged(int state)
	{
		m_discreteCheck->setText(state == Qt::Checked ? tr("1") : tr("0"));
	}

	void DialogAppSignalListValue::onValueDefaultClicked()
	{
		if (m_value.type() == TuningValueType::Discrete)
		{
			bool defaultState = m_defaultValue.discreteValue() == 0 ? false : true;

			m_discreteCheck->setChecked(defaultState);

			m_discreteCheck->setText(defaultState ? tr("1") : tr("0"));
		}
		else
		{
			m_analogEdit->setText(m_defaultValue.toString(m_analogFormat, m_decimalPlaces));
		}
	}
} // namespace AppSignalLists