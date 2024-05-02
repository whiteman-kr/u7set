#include "../AppSignalLists/include/AppSignalLists/SignalListEditor.h"
#include "../../AppSignalLib/ISignalManager.h"
#include "../lib/PropertyEditorDialog.h"
#include "../UtilsLib/Ui/UiTools.h"

namespace AppSignalLists
{
	//
	// SignalsModel
	//
	SignalsModel::SignalsModel(ISignalManager& signalManager)
		:QAbstractTableModel(),
		m_signalManager(signalManager)
	{
	}

	SignalsModel::~SignalsModel()
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
				else
				{
					return "Discrete";
				}
			}

			if (columnType == static_cast<int>(Columns::Default))
			{
				if (asp.enableTuning() == true)
				{
					if (asp.isAnalog())
					{
						return defaultValue(asp).toString(E::AnalogFormat::g_9_or_9e, asp.precision());
					}
					else
					{
						return defaultValue(asp).toString();
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
			case Columns::CustomAppSignalID:	return col_CustomAppSignalId;
			case Columns::EquipmentID:			return col_EquipmentID;
			case Columns::AppSignalID:			return col_AppSignalId;
			case Columns::Caption:				return col_Caption;
			case Columns::Units:				return col_Units;
			case Columns::Type:					return col_Type;
			case Columns::LowLimit:				return col_LowLimit;
			case Columns::HighLimit:			return col_HighLimit;
			case Columns::Default:				return col_Default;
			default:
				Q_ASSERT(false);
			}
		}
		return QVariant();
	}

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
		{
			v1 = asp1.customSignalId();
			v2 = asp2.customSignalId();
		}
		break;

		case SignalsModel::Columns::EquipmentID:
		{
			v1 = asp1.lmEquipmentId();
			v2 = asp2.lmEquipmentId();
		}
		break;

		case SignalsModel::Columns::AppSignalID:
		{
			v1 = asp1.appSignalId();
			v2 = asp2.appSignalId();
		}
		break;

		case SignalsModel::Columns::Caption:
		{
			v1 = asp1.caption();
			v2 = asp2.caption();
		}
		break;

		case SignalsModel::Columns::Units:
		{
			v1 = asp1.unit();
			v2 = asp2.unit();
		}
		break;

		case SignalsModel::Columns::Type:
		{
			v1 = static_cast<int>(asp1.tuningType());
			v2 = static_cast<int>(asp2.tuningType());
		}
		break;

		case SignalsModel::Columns::Default:
		{
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
		}
		break;

		case SignalsModel::Columns::LowLimit:
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
		case SignalsModel::Columns::HighLimit:
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
				{
					result = v1.toBool() < v2.toBool();
					break;
				}
				case QMetaType::QString:
				{
					result = v1.toString() < v2.toString();
					break;
				}
				case QMetaType::Int:
				{
					result = v1.toInt() < v2.toInt();
					break;
				}
				case QMetaType::UInt:
				{
					result = v1.toUInt() < v2.toUInt();
					break;
				}
				case QMetaType::LongLong:
				{
					result = v1.toLongLong() < v2.toLongLong();
					break;
				}
				case QMetaType::ULongLong:
				{
					result = v1.toULongLong() < v2.toULongLong();
					break;
				}
				case QMetaType::Float:
				{
					result = v1.toFloat() < v2.toFloat();
					break;
				}
				case QMetaType::Double:
				{
					result = v1.toDouble() < v2.toDouble();
					break;
				}
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
	// AppSignalListModel
	//

	AppSignalListModel::AppSignalListModel(ISignalManager& signalManager)
		:QAbstractTableModel(),
		m_signalManager(signalManager)
	{
	}

	AppSignalListModel::~AppSignalListModel()
	{
	}

	const AppSignalList* AppSignalListModel::list() const
	{
		return m_appSignallist;
	}

	void AppSignalListModel::setList(AppSignalList* list)
	{
		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

			m_appSignallist = nullptr;
			m_allHashes.clear();

			endRemoveRows();
		}

		if (list != nullptr)
		{
			if (list->count() > 0)
			{
				beginInsertRows(QModelIndex(), 0, list->count() - 1);
			}

			m_appSignallist = list;
			
			int count = m_appSignallist->count();
			m_allHashes.reserve(count);
			
			for (int i = 0; i < count; i++)
			{
				m_allHashes.push_back((*m_appSignallist)[i].appSignalHash());
			}

			if (list->count() > 0)
			{
				endInsertRows();
			}
		}
	}

	bool AppSignalListModel::itemExists(Hash hash) const
	{
		if (m_appSignallist == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		return m_appSignallist->itemExists(hash);
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

		AppSignalListModelSorter sorter(m_signalManager, m_appSignallist, static_cast<Columns>(column), order);
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

			const AppSignalListItem& item = (*m_appSignallist)[hash];

			bool ok = false;

			const AppSignalParam asp = m_signalManager.signalParam(hash, &ok);

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
				else
				{
					return "Discrete";
				}			
			}

			if (columnType == static_cast<int>(Columns::Value))
			{
				if (asp.enableTuning() == true)
				{
					if (item.hasValue() == true)
					{
						if (asp.isAnalog() == true)
						{
							return item.value().toString(E::AnalogFormat::g_9_or_9e, asp.precision());
						}
						else
						{
							return item.value().toString();
						}
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
			case Columns::CustomAppSignalID:	return col_CustomAppSignalId;
			case Columns::EquipmentID:			return col_EquipmentID;
			case Columns::AppSignalID:			return col_AppSignalId;
			case Columns::Caption:				return col_Caption;
			case Columns::Units:				return col_Units;
			case Columns::Type:					return col_Type;
			case Columns::LowLimit:				return col_LowLimit;
			case Columns::HighLimit:			return col_HighLimit;
			case Columns::Value:				return col_Value;
			default:
				Q_ASSERT(false);
			}
		}

		return QVariant();
	}

	//
	// AppSignalListModelSorter
	//
	AppSignalListModelSorter::AppSignalListModelSorter(ISignalManager& signalManager, const AppSignalList* appSignallist, AppSignalListModel::Columns column, Qt::SortOrder order) :
		m_signalManager(signalManager),
		m_appSignallist(appSignallist),
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

		const AppSignalListItem& item1 = (*m_appSignallist)[hash1];
		const AppSignalListItem& item2 = (*m_appSignallist)[hash2];

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
				{
					result = v1.toBool() < v2.toBool();
					break;
				}
				case QMetaType::QString:
				{
					result = v1.toString() < v2.toString();
					break;
				}
				case QMetaType::Int:
				{
					result = v1.toInt() < v2.toInt();
					break;
				}
				case QMetaType::UInt:
				{
					result = v1.toUInt() < v2.toUInt();
					break;
				}
				case QMetaType::LongLong:
				{
					result = v1.toLongLong() < v2.toLongLong();
					break;
				}
				case QMetaType::ULongLong:
				{
					result = v1.toULongLong() < v2.toULongLong();
					break;
				}
				case QMetaType::Float:
				{
					result = v1.toFloat() < v2.toFloat();
					break;
				}
				case QMetaType::Double:
				{
					result = v1.toDouble() < v2.toDouble();
					break;
				}
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
	// DialogAppSignalListValue
	//

	DialogAppSignalListValue::DialogAppSignalListValue(TuningValue value, TuningValue defaultValue, bool sameValue, bool sameDefaultValue,
		TuningValue lowLimit, TuningValue highLimit, E::AnalogFormat analogFormat, int decimalPlaces, QWidget* parent) :
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

	DialogAppSignalListValue::~DialogAppSignalListValue()
	{
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

	//
	// AppSignalListWidget
	//

	AppSignalListWidget::AppSignalListWidget(ISignalManager& signalManager, bool requestValuesEnabled, QWidget* parent)
		:QWidget(parent),
		m_signalManager(signalManager),
		m_signalHashes(std::move(m_signalManager.signalHashes())),
		m_signalsModel(m_signalManager),
		m_itemsModel(m_signalManager)
	{
		// Left part
		//

		QHBoxLayout* mainLayout = new QHBoxLayout();
		QVBoxLayout* leftLayout = new QVBoxLayout();

		// Signals Model
		//

		// Signals table
		//
		m_signalsTable = new QTableView();
		m_signalsTable->verticalHeader()->hide();
		m_signalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
		m_signalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		m_signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_signalsTable->setSortingEnabled(true);
		m_signalsTable->horizontalHeader()->setHighlightSections(false);
		m_signalsTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		m_signalsTable->setModel(&m_signalsModel);
		
		connect(m_signalsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AppSignalListWidget::onSignalsTableSelectionChanged);
		connect(m_signalsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &AppSignalListWidget::onSignalsSortIndicatorChanged);
		connect(m_signalsTable, &QTableView::doubleClicked, this, &AppSignalListWidget::onSignalsTableDoubleClicked);
		connect(m_signalsTable->horizontalHeader(), &QWidget::customContextMenuRequested, this, &AppSignalListWidget::onSignalsHeaderColumnContextMenuRequested);
		leftLayout->addWidget(m_signalsTable);

		// Signals Filters
		//
		m_signalTypeCombo = new QComboBox();
		m_signalTypeCombo->blockSignals(true);
		m_signalTypeCombo->addItem(tr("All signals"), static_cast<int>(SignalType::All));
		m_signalTypeCombo->addItem(tr("Analog signals"), static_cast<int>(SignalType::Analog));
		m_signalTypeCombo->addItem(tr("Discrete signals"), static_cast<int>(SignalType::Discrete));
		m_signalTypeCombo->setCurrentIndex(0);
		m_signalTypeCombo->blockSignals(false);
		connect(m_signalTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AppSignalListWidget::onSignalsTypeComboCurrentIndexChanged);
		leftLayout->addWidget(m_signalTypeCombo);

		QHBoxLayout* leftFilterLayout = new QHBoxLayout();

		m_filterTextTypeCombo = new QComboBox();
		m_filterTextTypeCombo->blockSignals(true);
		m_filterTextTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterTextType::All));
		m_filterTextTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterTextType::AppSignalID));
		m_filterTextTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterTextType::CustomAppSignalID));
		m_filterTextTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterTextType::EquipmentID));
		m_filterTextTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterTextType::Caption));
		m_filterTextTypeCombo->addItem(tr("Tag"), static_cast<int>(FilterTextType::Tag));
		m_filterTextTypeCombo->setCurrentIndex(0);
		m_filterTextTypeCombo->blockSignals(false);
		connect(m_filterTextTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AppSignalListWidget::onSignalsFilterTypeComboCurrentIndexChanged);
		leftFilterLayout->addWidget(m_filterTextTypeCombo, 1);

		m_filterTextEdit = new QLineEdit();
		connect(m_filterTextEdit, &QLineEdit::returnPressed, this, &AppSignalListWidget::onSignalsFilterTextChanged);
		connect(m_filterTextEdit, &QLineEdit::textChanged, this, [this](const QString& str) {
			if (str.isEmpty() == true)
			{
				onSignalsFilterTextChanged();		// Process mask if text was cleared
			}
			});
		m_filterTextEdit->setClearButtonEnabled(true);
		leftFilterLayout->addWidget(m_filterTextEdit, 3);

		m_applyFilterButton = new QPushButton(tr("Apply Filter"));
		connect(m_applyFilterButton, &QPushButton::clicked, this, &AppSignalListWidget::onSignalsApplyFilterClicked);
		leftFilterLayout->addWidget(m_applyFilterButton, 1);

		// Value filter controls
		//
		if (requestValuesEnabled == true)
		{
			leftFilterLayout->addSpacing(20);

			QLabel* l = new QLabel(tr("Value:"));
			leftFilterLayout->addWidget(l);

			m_filterValueCombo = new QComboBox();
			m_filterValueCombo->addItem(tr("Any Value"), static_cast<int>(FilterValueType::All));
			m_filterValueCombo->addItem(tr("Discrete 0"), static_cast<int>(FilterValueType::Zero));
			m_filterValueCombo->addItem(tr("Discrete 1"), static_cast<int>(FilterValueType::One));
			leftFilterLayout->addWidget(m_filterValueCombo);
			connect(m_filterValueCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AppSignalListWidget::onSignalsFilterValueComboCurrentIndexChanged);
			m_filterValueCombo->setCurrentIndex(0);
		}

		leftLayout->addLayout(leftFilterLayout);
		mainLayout->addLayout(leftLayout);

		// Middle part
		//

		QVBoxLayout* midLayout = new QVBoxLayout();

		midLayout->addStretch();

		m_addValueButton = new QPushButton(tr("Add"));
		connect(m_addValueButton, &QPushButton::clicked, this, &AppSignalListWidget::onAddClicked);
		midLayout->addWidget(m_addValueButton);
		m_addValueButton->setEnabled(false);

		m_removeValueButton = new QPushButton(tr("Remove"));
		connect(m_removeValueButton, &QPushButton::clicked, this, &AppSignalListWidget::onRemoveClicked);
		midLayout->addWidget(m_removeValueButton);
		m_removeValueButton->setEnabled(false);

		midLayout->addStretch();

		mainLayout->addLayout(midLayout);

		// Right part
		//

		QVBoxLayout* rightLayout = new QVBoxLayout();

		m_itemsTable = new QTableView();
		m_itemsTable->verticalHeader()->hide();
		m_itemsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
		m_itemsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		m_itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_itemsTable->setSortingEnabled(true);
		m_itemsTable->horizontalHeader()->setHighlightSections(false);
		m_itemsTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		m_itemsTable->setModel(&m_itemsModel);

		connect(m_itemsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AppSignalListWidget::onItemsTreeSelectionChanged);
		connect(m_itemsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &AppSignalListWidget::onItemsSortIndicatorChanged);
		connect(m_itemsTable, &QTableView::doubleClicked, this, &AppSignalListWidget::onItemsTreeDoubleClicked);
		connect(m_itemsTable->horizontalHeader(), &QWidget::customContextMenuRequested, this, &AppSignalListWidget::onItemsHeaderColumnContextMenuRequested);

		rightLayout->addWidget(m_itemsTable);

		QHBoxLayout* rightGridLayout = new QHBoxLayout();

		rightGridLayout->addStretch();

		m_setValueButton = new QPushButton(tr("Set Value"));
		connect(m_setValueButton, &QPushButton::clicked, this, &AppSignalListWidget::onSetValueClicked);
		rightGridLayout->addWidget(m_setValueButton);
		m_setValueButton->setEnabled(false);

		if (requestValuesEnabled == true)
		{
			m_setCurrentButton = new QPushButton(tr("Set Current"));
			connect(m_setCurrentButton, &QPushButton::clicked, this, &AppSignalListWidget::onSetCurrentClicked);
			rightGridLayout->addWidget(m_setCurrentButton);
			m_setCurrentButton->setEnabled(false);
		}

		m_exportValuesButton = new QPushButton(tr("Export..."));
		connect(m_exportValuesButton, &QPushButton::clicked, this, &AppSignalListWidget::onExportValuesClicked);
		rightGridLayout->addWidget(m_exportValuesButton);

		m_importValuesButton = new QPushButton(tr("Import..."));
		connect(m_importValuesButton, &QPushButton::clicked, this, &AppSignalListWidget::onImportValuesClicked);
		rightGridLayout->addWidget(m_importValuesButton);
		m_importValuesButton->setEnabled(false);

		rightLayout->addLayout(rightGridLayout);

		mainLayout->addLayout(rightLayout);

		//

		setLayout(mainLayout);

		fillSignalsList();

		// Restore signals list settings
		{
			int count = QSettings().value("AppSignalListWidget/signalsTableHeaderCount").toInt();
			QByteArray ba = QSettings().value("AppSignalListWidget/signalsTableHeader").toByteArray();
			if (ba.isEmpty() == true || count != static_cast<int>(SignalsModel::Columns::Count))
			{
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::CustomAppSignalID));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::EquipmentID));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::Type));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::LowLimit));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::HighLimit));
				m_signalsTable->resizeColumnsToContents();
			}
			else
			{
				m_signalsTable->horizontalHeader()->restoreState(ba);
			}
		}

		// Restore items list settings
		{
			int count = QSettings().value("AppSignalListWidget/itemsTreeHeaderCount").toInt();
			QByteArray ba = QSettings().value("AppSignalListWidget/itemsTreeHeader").toByteArray();
			if (ba.isEmpty() == true || count != static_cast<int>(AppSignalListModel::Columns::Count))
			{
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::CustomAppSignalID));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::EquipmentID));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::Type));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::LowLimit));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::HighLimit));
				
				for (int i = 0; i < m_itemsTable->horizontalHeader()->count(); i++)
				{
					m_itemsTable->resizeColumnToContents(i);
				}
			}
			else
			{
				m_itemsTable->horizontalHeader()->restoreState(ba);
			}
		}
	}

	AppSignalListWidget::~AppSignalListWidget()
	{
		QSettings().setValue("AppSignalListWidget/signalsTableHeaderCount", static_cast<int>(SignalsModel::Columns::Count));
		QSettings().setValue("AppSignalListWidget/signalsTableHeader", m_signalsTable->horizontalHeader()->saveState());

		QSettings().setValue("AppSignalListWidget/itemsTreeHeaderCount", static_cast<int>(AppSignalListModel::Columns::Count));
		QSettings().setValue("AppSignalListWidget/itemsTreeHeader", m_itemsTable->horizontalHeader()->saveState());
	}

	bool AppSignalListWidget::readOnly() const
	{
		return m_readOnly;
	}

	void AppSignalListWidget::setReadOnly(bool value)
	{
		m_readOnly = value;
		enableSignalsListControls();
		enableItemsListControls();
		return;
	}

	AppSignalList* AppSignalListWidget::list() const
	{
		return m_appSignallist;
	}

	void AppSignalListWidget::setList(AppSignalList* list)
	{
		m_appSignallist = list;
		fillItemsList();

		enableSignalsListControls();
		enableItemsListControls();
	}

	void AppSignalListWidget::fillSignalsList()
	{
		SignalType signalType = SignalType::All;
		QVariant data = m_signalTypeCombo->currentData();
		if (data.isNull() == false && data.isValid() == true)
		{
			signalType = static_cast<SignalType>(data.toInt());
		}

		FilterTextType filterType = FilterTextType::AppSignalID;
		data = m_filterTextTypeCombo->currentData();
		if (data.isNull() == false && data.isValid() == true)
		{
			filterType = static_cast<FilterTextType>(data.toInt());
		}

		FilterValueType filterValue = FilterValueType::All;
		if (m_filterValueCombo != nullptr)
		{
			data = m_filterValueCombo->currentData();
			if (data.isValid() == true)
			{
				filterValue = static_cast<FilterValueType>(data.toInt());
			}
		}

		QString filterText = m_filterTextEdit->text().trimmed();

		std::vector<Hash> filteredHashes;
		filteredHashes.reserve(m_signalHashes.size());

		if (filterText.isEmpty() == true && signalType == SignalType::All)
		{
			// Filter is not set - skip all filtering, just copy hashes array
			//
			filteredHashes = m_signalHashes;
		}
		else
		{
			// Some filters are set
			//
			for (Hash hash : m_signalHashes)
			{
				bool ok = false;
				const AppSignalParam& asp = m_signalManager.signalParam(hash, &ok);
				Q_ASSERT(ok);

				if (signalType == SignalType::Analog && asp.isAnalog() == false)
				{
					continue;
				}

				if (signalType == SignalType::Discrete && asp.isAnalog() == true)
				{
					continue;
				}

				// Value filter
				//
				/*
				if (filterValue != ValueFilterType::All)
				{
					if (asp.isDiscrete() == false)
					{
						continue;
					}

					bool ok = false;

					const TuningSignalState state = m_signalManager.queuedState(hash, &ok);

					if (ok == true)
					{
						if (state.valid() == false)
						{
							continue;
						}
						if (filterValue == ValueFilterType::Zero && state.value().discreteValue() != 0)
						{
							continue;
						}
						if (filterValue == ValueFilterType::One && state.value().discreteValue() != 1)
						{
							continue;
						}
					}
				}*/

				// Text filter
				//

				if (filterText.isEmpty() == false)
				{
					bool filterResult = false;

					switch (filterType)
					{
					case FilterTextType::All:
					{
						if (asp.appSignalId().contains(filterText, Qt::CaseInsensitive) == true
							|| asp.customSignalId().contains(filterText, Qt::CaseInsensitive) == true
							|| asp.lmEquipmentId().contains(filterText, Qt::CaseInsensitive) == true
							|| asp.caption().contains(filterText, Qt::CaseInsensitive) == true
							|| asp.tags().contains(filterText) == true)
						{
							filterResult = true;
						}
					}
					break;
					case FilterTextType::AppSignalID:
					{
						if (asp.appSignalId().contains(filterText, Qt::CaseInsensitive) == true)
						{
							filterResult = true;
						}
					}
					break;
					case FilterTextType::CustomAppSignalID:
					{
						if (asp.customSignalId().contains(filterText, Qt::CaseInsensitive) == true)
						{
							filterResult = true;
						}
					}
					break;
					case FilterTextType::EquipmentID:
					{
						if (asp.lmEquipmentId().contains(filterText, Qt::CaseInsensitive) == true)
						{
							filterResult = true;
						}
					}
					break;
					case FilterTextType::Caption:
					{
						if (asp.caption().contains(filterText, Qt::CaseInsensitive) == true)
						{
							filterResult = true;
						}
					}
					break;
					case FilterTextType::Tag:
					{
						if (asp.tags().contains(filterText) == true)
						{
							filterResult = true;
						}
					}
					break;
					}

					if (filterResult == false)
					{
						continue;
					}
				}

				filteredHashes.push_back(hash);
			}
		}

		m_signalsModel.setHashes(filteredHashes);
		m_signalsTable->sortByColumn(m_signalsSortColumn, m_signalsSortOrder);
	}


	void AppSignalListWidget::fillItemsList()
	{
		m_itemsModel.setList(m_appSignallist);
		m_itemsTable->sortByColumn(m_itemsSortColumn, m_itemsSortOrder);
	}

	void AppSignalListWidget::enableSignalsListControls()
	{
		const QModelIndexList& selection = m_signalsTable->selectionModel()->selectedRows();
		
		m_addValueButton->setEnabled(readOnly() == false && selection.size() > 0 && m_appSignallist != nullptr);
	}

	void AppSignalListWidget::enableItemsListControls()
	{
		const QModelIndexList& selection = m_itemsTable->selectionModel()->selectedRows();
		
		m_removeValueButton->setEnabled(readOnly() == false && selection.size() > 0);

		// Check if only tunable signals are selected
		//
		bool tunableSelected = true;
		for (const QModelIndex& index: selection)
		{
			Hash hash = m_itemsModel.itemHash(index.row());

			const AppSignalListItem& item = (*m_appSignallist)[hash];

			bool ok = false;
			AppSignalParam asp = m_signalManager.signalParam(item.appSignalHash(), &ok);
			if (ok == false)
			{
				continue;
			}
			if (asp.enableTuning() == false)
			{
				tunableSelected = false;
			}
		}

		// Enable control buttons
		//
		m_setValueButton->setEnabled(readOnly() == false && selection.size() > 0 && tunableSelected == true);
		
		m_importValuesButton->setEnabled(readOnly() == false);
		
		if (m_setCurrentButton != nullptr)
		{
			m_setCurrentButton->setEnabled(readOnly() == false && selection.size() > 0);
		}
	}

	void AppSignalListWidget::onSignalsSortIndicatorChanged(int column, Qt::SortOrder order)
	{
		m_signalsSortColumn = column;
		m_signalsSortOrder = order;

		m_signalsModel.sort(column, order);
	}

	void AppSignalListWidget::onSignalsTableSelectionChanged(const QItemSelection&, const QItemSelection&)
	{
		enableSignalsListControls();
	}

	void AppSignalListWidget::onSignalsApplyFilterClicked()
	{
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsFilterTypeComboCurrentIndexChanged(int index)
	{
		Q_UNUSED(index);
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsFilterValueComboCurrentIndexChanged(int index)
	{
		Q_UNUSED(index);
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsFilterTextChanged()
	{
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsTypeComboCurrentIndexChanged(int index)
	{
		Q_UNUSED(index);
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsTableDoubleClicked(const QModelIndex& index)
	{
		Q_UNUSED(index);

		if (readOnly() == true)
		{
			return;
		}

		onAddClicked();
	}

	void AppSignalListWidget::onSignalsHeaderColumnContextMenuRequested(const QPoint& pos)
	{
		QMenu menu(this);

		QList<QAction*> actions;

		std::vector<std::pair<SignalsModel::Columns, QString>> actionsData;
		actionsData.reserve(static_cast<int>(SignalsModel::Columns::Count));

		for (int i = 0; i < static_cast<int>(SignalsModel::Columns::Count); i++)
		{
			actionsData.emplace_back(static_cast<SignalsModel::Columns>(i), m_signalsModel.columnText(i));
		}

		for (std::pair<SignalsModel::Columns, QString> ad : actionsData)
		{
			QAction* action = new QAction(ad.second, this);
			action->setData(QVariant::fromValue(ad.first));
			action->setCheckable(true);
			action->setChecked(!m_signalsTable->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

			if (m_signalsTable->horizontalHeader()->count() - m_signalsTable->horizontalHeader()->hiddenSectionCount() == 1 &&
				action->isChecked() == true)
			{
				action->setEnabled(false); // Impossible to uncheck the last column
			}

			connect(action, &QAction::toggled, this, &AppSignalListWidget::onSignalsHeaderColumnToggled);
			actions << action;
		}

		menu.exec(actions, mapToGlobal(pos), 0, this);
		return;
	}

	void AppSignalListWidget::onSignalsHeaderColumnToggled(bool checked)
	{
		QAction* action = dynamic_cast<QAction*>(sender());

		if (action == nullptr)
		{
			Q_ASSERT(action);
			return;
		}

		int column = action->data().value<int>();

		if (column >= static_cast<int>(SignalsModel::Columns::Count))
		{
			Q_ASSERT(column < static_cast<int>(SignalsModel::Columns::Count));
			return;
		}

		if (checked == true)
		{
			m_signalsTable->showColumn(column);
		}
		else
		{
			m_signalsTable->hideColumn(column);
		}

		return;
	}

	void AppSignalListWidget::onItemsSortIndicatorChanged(int column, Qt::SortOrder order)
	{
		m_itemsSortColumn = column;
		m_itemsSortOrder = order;

		m_itemsModel.sort(column, order);
	}

	void AppSignalListWidget::onItemsTreeSelectionChanged()
	{
		enableItemsListControls();
	}

	void AppSignalListWidget::onItemsTreeDoubleClicked(const QModelIndex& index)
	{
		Q_UNUSED(index);

		if (readOnly() == true)
		{
			return;
		}

		if (m_setValueButton->isEnabled() == true)
		{
			onSetValueClicked();
		}
		//onRemoveClicked();
	}

	void AppSignalListWidget::onItemsHeaderColumnContextMenuRequested(const QPoint& /*pos*/)
	{
		QMenu menu(this);

		QList<QAction*> actions;

		std::vector<std::pair<AppSignalListModel::Columns, QString>> actionsData;
		actionsData.reserve(static_cast<int>(AppSignalListModel::Columns::Count));

		for (int i = 0; i < static_cast<int>(AppSignalListModel::Columns::Count); i++)
		{
			actionsData.emplace_back(static_cast<AppSignalListModel::Columns>(i), m_itemsModel.columnText(i));
		}

		for (std::pair<AppSignalListModel::Columns, QString> ad : actionsData)
		{
			QAction* action = new QAction(ad.second, this);
			action->setData(QVariant::fromValue(ad.first));
			action->setCheckable(true);
			action->setChecked(!m_itemsTable->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

			if (m_itemsTable->horizontalHeader()->count() - m_itemsTable->horizontalHeader()->hiddenSectionCount() == 1 &&
				action->isChecked() == true)
			{
				action->setEnabled(false); // Impossible to uncheck the last column
			}

			connect(action, &QAction::toggled, this, &AppSignalListWidget::onItemsHeaderColumnToggled);
			actions << action;
		}

		menu.exec(actions, QCursor::pos(), 0, this);
		return;
	}

	void AppSignalListWidget::onItemsHeaderColumnToggled(bool checked)
	{
		QAction* action = dynamic_cast<QAction*>(sender());

		if (action == nullptr)
		{
			Q_ASSERT(action);
			return;
		}

		int column = action->data().value<int>();

		if (column >= static_cast<int>(AppSignalListModel::Columns::Count))
		{
			Q_ASSERT(column < static_cast<int>(AppSignalListModel::Columns::Count));
			return;
		}

		if (checked == true)
		{
			m_itemsTable->showColumn(column);
		}
		else
		{
			m_itemsTable->hideColumn(column);
		}

		return;
	}

	void AppSignalListWidget::onAddClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignallist == nullptr)
		{
			return;
		}

		for (const QModelIndex& index : m_signalsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_signalsModel.hash(index.row());

			if (m_itemsModel.itemExists(hash) == true)
			{
				continue;
			}

			bool ok = false;

			const AppSignalParam p = m_signalManager.signalParam(hash, &ok);
			if (ok == false)
			{
				Q_ASSERT(false);
				return;
			}
			/*
			const TuningSignalState s = m_signalManager.queuedState(hash, &ok);

			if (m_filter == nullptr)
			{
				Q_ASSERT(m_filter);
				return;
			}

			if (m_filter->filterSignalExists(p.hash()) == true)
			{
				continue;
			}

			// Create value
			*/
			AppSignalListItem item(p.appSignalId());
			/*if (s.valid() == true)
			{
				ofv.setValue(s.value());
			}*/

			m_appSignallist->add(item);

			if (m_itemsModel.add(item) == false)
			{
				Q_ASSERT(false);
				continue;
			}
		}

		emit signalsChanged();
	}

	void AppSignalListWidget::onRemoveClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignallist == nullptr)
		{
			return;
		}

		// Build list of selected items hashes
		//
		std::vector<Hash> hashesToDelete;
		auto selection = m_itemsTable->selectionModel()->selectedRows();
		for (const QModelIndex& index: selection)
		{
			Hash hash = m_itemsModel.itemHash(index.row());
			hashesToDelete.push_back(hash);
		}

		for (Hash hash : hashesToDelete)
		{
			// Delete item from model
			//
			if (m_itemsModel.remove(hash) == false)
			{
				Q_ASSERT(false);
			}

			// Delete item from list
			//
			if (m_appSignallist->remove(hash) == false)
			{
				Q_ASSERT(false);
				return;
			}
		}

		emit signalsChanged();
	}

	void AppSignalListWidget::onSetValueClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignallist == nullptr)
		{
			return;
		}

		bool first = true;
		TuningValue lowLimit;
		TuningValue highLimit;
		int precision = 0;
		TuningValue value;
		TuningValue defaultValue;

		bool sameValue = true;
		bool sameDefaultValue = true;

		for (const QModelIndex& index : m_itemsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_itemsModel.itemHash(index.row());
			
			if (m_signalManager.signalExists(hash) == false)
			{
				continue;
			}

			const AppSignalListItem& item = (*m_appSignallist)[hash];

			bool ok = false;
			AppSignalParam asp = m_signalManager.signalParam(item.appSignalHash(), &ok);
			if (ok == false)
			{
				Q_ASSERT(false);
				return;
			}

			if (first == true)
			{
				lowLimit = asp.tuningLowBound();
				highLimit = asp.tuningHighBound();
				precision = asp.precision();

				if (item.hasValue() == true)
				{
					value = item.value();
				}
				value.setType(asp.tuningType());

				defaultValue = asp.tuningDefaultValue();

				first = false;
			}
			else
			{
				if (asp.tuningType() != value.type())
				{
					QMessageBox::warning(this, qAppName(), tr("Please select signals of same type (analog or discrete)."));
					return;
				}

				if (asp.isAnalog() == true)
				{
					if (lowLimit != asp.tuningLowBound() || highLimit != asp.tuningHighBound())
					{
						QMessageBox::warning(this, qAppName(), tr("Selected signals have different input range."));
						return;
					}
				}

				if (item.hasValue() == true)
				{
					if (item.value() != value)
					{
						sameValue = false;
					}
				}
				if (defaultValue != asp.tuningDefaultValue())
				{
					sameDefaultValue = false;
				}
			}
		}

		DialogAppSignalListValue d(value, defaultValue, sameValue, sameDefaultValue, lowLimit, highLimit, E::AnalogFormat::g_9_or_9e, precision, this);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		for (const QModelIndex& index : m_itemsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_itemsModel.itemHash(index.row());

			if (m_signalManager.signalExists(hash) == false)
			{
				continue;
			}

			AppSignalListItem& item = (*m_appSignallist)[hash];

			item.setValue(d.value());

			m_itemsTable->update(index);
		}
		emit signalsChanged();
	}

	void AppSignalListWidget::onSetCurrentClicked()
	{
		/*
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignallist == nullptr)
		{
			return;
		}

		for (const QModelIndex& index : m_signalsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_itemsModel.itemHash(index.row());

			if (m_signalManager.signalExists(hash) == false)
			{
				continue;
			}

			AppSignalListItem& item = (*m_appSignallist)[hash];

			TuningValue currentValue;

			bool ok = false;
			emit getCurrentSignalValue(item.appSignalHash(), &currentValue, &ok);

			if (ok == true)
			{
				item.setValue(currentValue);
				m_itemsTable->update(index);
			}
			else
			{
				QMessageBox::warning(this, qAppName(), tr("Can't get current value of signal %1!").arg(item.appSignalId()));
			}
		}

		emit signalsChanged();*/
	}

	void AppSignalListWidget::onExportValuesClicked()
	{
		if (m_appSignallist == nullptr)
		{
			return;
		}

		int columnCount = m_itemsModel.columnCount();
		int rowCount = m_itemsModel.rowCount();

		static QString path{ "." };
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"),
			path + QDir::separator(),
			tr("CSV (*.csv)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		QFile file(fileName);
		if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Error writing file %1!").arg(fileName));
			return;
		}

		QTextStream out(&file);
		out.setEncoding(QStringConverter::Utf8);

		QString csvHeader;
		for (int c = 0; c < columnCount; c++)
		{
			csvHeader += m_itemsModel.columnText(c) + ';';
		}
		out << csvHeader << "\r\n";

		for (int r = 0; r < rowCount; r++)
		{
			QString csvRow;
			for (int c = 0; c < columnCount; c++)
			{
				csvRow += m_itemsModel.cellText(c, r) + ';';
			}
			out << csvRow << "\r\n";
		}

		QMessageBox::information(this, qAppName(), tr("Export complete."));
	}

	void AppSignalListWidget::onImportValuesClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignallist == nullptr)
		{
			return;
		}

		static QString path{ "." };
		QString fileName = QFileDialog::getOpenFileName(this, tr("Import from CSV"),
			path,
			tr("CSV (*.csv)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		QFile file(fileName);
		if (file.open(QFile::ReadOnly) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Error writing file %1!").arg(fileName));
			return;
		}

		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);

		bool headerString = true;

		int columnAppSignalIdIndex = -1;
		int columnValueIndex = -1;

		int signalsAdded = 0;
		int signalsUpdated = 0;
		QStringList notFoundSignals;

		while (!in.atEnd())
		{
			QString str = in.readLine().trimmed();
			if (str.isEmpty() == true)
			{
				break;
			}

			QStringList strings = str.split(';', Qt::KeepEmptyParts);

			// Load header string and find columns with appSignalId and value
			//
			if (headerString == true)
			{
				headerString = false;
				
				for (int i = 0; i < strings.size(); i++)
				{
					const QString& s = strings[i];
					if (s == AppSignalLists::col_AppSignalId)
					{
						columnAppSignalIdIndex = i;
						continue;
					}
					if (s == AppSignalLists::col_Value)
					{
						columnValueIndex = i;
						continue;
					}
				}
				if (columnValueIndex == -1 || columnAppSignalIdIndex == -1)
				{
					QMessageBox::critical(this, qAppName(), tr("Error: '%1' and '%2' columns are absent in CSV file. Import is impossible.")
						.arg(col_AppSignalId)
						.arg(col_Value));
					return;
				}
				
				continue;
			}

			// Load signal list item

			const QString& appSignalId = strings[columnAppSignalIdIndex];
			const QString& valueStr = strings[columnValueIndex];

			Hash hash = ::calcHash(appSignalId);

			// Get signal parameters from database

			bool ok = false;
			const AppSignalParam asp = m_signalManager.signalParam(hash, &ok);
			if (ok == false)
			{
				notFoundSignals.push_back(appSignalId);
				continue;
			}

			// Read value

			std::optional<TuningValue> tv;
			if (valueStr.isEmpty() == false && valueStr != "-")
			{
				TuningValue v(asp.tuningType());
				
				bool valueOk = false;
				v.fromString(valueStr, &valueOk);
				if (valueOk == true)
				{
					tv = v;
				}
			}

			// Add item to the list

			bool signalExists = m_appSignallist->itemExists(hash);
			if (signalExists == false)
			{
				AppSignalListItem item(asp.appSignalId());
				if (tv.has_value() == true)
				{
					item.setValue(tv.value());
				}
				m_appSignallist->add(item);
				signalsAdded++;
			}
			else
			{
				AppSignalListItem& item = (*m_appSignallist)[hash];
				if (tv.has_value() == true)
				{
					item.setValue(tv.value());
				}
				else
				{
					item.removeValue();
				}
				signalsUpdated++;
			}
		}

		fillItemsList();

		if (notFoundSignals.isEmpty() == true)
		{
			QMessageBox::information(this, qAppName(), tr("Import complete.\n\nAdded: %1 signals\nUpdated: %2 signals").arg(signalsAdded).arg(signalsUpdated));	
		}
		else
		{
			int notFoundCount = notFoundSignals.size();
			bool notFoundAbove10 = notFoundSignals.size() > 10;
			
			// Leave only first ten signals
			//
			while(notFoundSignals.size() > 10)
			{
				notFoundSignals.removeLast();
			}

			QString message = tr("Import complete.\n\nAdded: %1 signals\nUpdated: %2 signals\n\n%3 signals were not found:\n%4")
				.arg(signalsAdded)
				.arg(signalsUpdated)
				.arg(notFoundCount)
				.arg(notFoundSignals.join('\n'));
			if (notFoundAbove10 == true)
			{
				message += tr(" and more.");
			}

			QMessageBox::warning(this, qAppName(), message);	
		}

		emit signalsChanged();

		return;
	}
}