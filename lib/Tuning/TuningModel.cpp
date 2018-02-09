#include "TuningModel.h"
#include <QKeyEvent>

using namespace std;

//
// TuningItemSorter
//

TuningModelSorter::TuningModelSorter(int column, Qt::SortOrder order, TuningModel* model, TuningSignalManager* tuningSignalManager):
	m_column(column),
	m_order(order),
	m_tuningSignalManager(tuningSignalManager),
	m_model(model)
{
}

bool TuningModelSorter::sortFunction(Hash hash1, Hash hash2, int column, Qt::SortOrder order) const
{

	QVariant v1;
	QVariant v2;

	AppSignalParam asp1;
	AppSignalParam asp2;

	TuningSignalState tss1;
	TuningSignalState tss2;

	bool ok = false;

	asp1 = m_tuningSignalManager->signalParam(hash1, &ok);
	asp2 = m_tuningSignalManager->signalParam(hash2, &ok);

	tss1 = m_tuningSignalManager->state(hash1, &ok);
	tss2 = m_tuningSignalManager->state(hash2, &ok);

	switch (static_cast<TuningModel::Columns>(column))
	{
	case TuningModel::Columns::CustomAppSignalID:
	{
		v1 = asp1.customSignalId();
		v2 = asp2.customSignalId();
	}
		break;
	case TuningModel::Columns::EquipmentID:
	{
		v1 = asp1.equipmentId();
		v2 = asp2.equipmentId();
	}
		break;
	case TuningModel::Columns::AppSignalID:
	{
		v1 = asp1.appSignalId();
		v2 = asp2.appSignalId();
	}
		break;
	case TuningModel::Columns::Caption:
	{
		v1 = asp1.caption();
		v2 = asp2.caption();
	}
		break;
	case TuningModel::Columns::Units:
	{
		v1 = asp1.unit();
		v2 = asp2.unit();
	}
		break;
	case TuningModel::Columns::Type:
	{
		v1 = asp1.isAnalog();
		v2 = asp2.isAnalog();
	}
		break;

	case TuningModel::Columns::Default:
	{
		if (asp1.isAnalog() == asp2.isAnalog())
		{
			TuningValue tv1 = m_model->defaultValue(asp1);
			TuningValue tv2 = m_model->defaultValue(asp2);

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
	case TuningModel::Columns::Value:
	{
		if (asp1.isAnalog() == asp2.isAnalog())
		{
			TuningValue tv1 = tss1.value();
			TuningValue tv2 = tss2.value();

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
	case TuningModel::Columns::LowLimit:
	{
		if (asp1.isAnalog() == asp2.isAnalog())
		{
			TuningValue tv1 = asp1.tuningLowBound();
			TuningValue tv2 = asp2.tuningLowBound();

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
	case TuningModel::Columns::HighLimit:
	{
		if (asp1.isAnalog() == asp2.isAnalog())
		{
			TuningValue tv1 = asp1.tuningHighBound();
			TuningValue tv2 = asp2.tuningHighBound();

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
	case TuningModel::Columns::Valid:
	{
		v1 = tss1.valid();
		v2 = tss2.valid();
	}
		break;
	case TuningModel::Columns::OutOfRange:
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

TuningModel::TuningModel(TuningSignalManager* tuningSignalManager, QWidget* parent)
	:QAbstractItemModel(parent),
	  m_tuningSignalManager(tuningSignalManager)
{
	// Fill column names

	m_columnsNames << tr("Custom AppSignal ID");
	m_columnsNames << tr("Equipment ID");
	m_columnsNames << tr("App Signal ID");
	m_columnsNames << tr("Caption");
	m_columnsNames << tr("Units");
	m_columnsNames << tr("Type");

	m_columnsNames << tr("Value");
	m_columnsNames << tr("LowLimit");
	m_columnsNames << tr("HighLimit");
	m_columnsNames << tr("Default");
	m_columnsNames << tr("Valid");
	m_columnsNames << tr("Underflow");
	m_columnsNames << tr("Overflow");
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

std::vector<Hash> TuningModel::hashes() const
{
	return m_hashes;
}

void TuningModel::setHashes(std::vector<Hash>& hashes)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_hashes.clear();

		endRemoveRows();
	}

	//
	if (hashes.empty() == false)
	{
		int count = static_cast<int>(hashes.size());

		beginInsertRows(QModelIndex(), 0, count - 1);

		m_hashes.swap(hashes);

		insertRows(0, static_cast<int>(count));

		endInsertRows();
	}
}

Hash TuningModel::hashByIndex(int index) const
{
	if (index < 0 || index >= m_hashes.size())
	{
		assert(false);
		return UNDEFINED_HASH;
	}

	return m_hashes[index];
}

TuningSignalManager* TuningModel::tuningSignalManager()
{
	return m_tuningSignalManager;
}



void TuningModel::addColumn(Columns column)
{
	m_columnsIndexes.push_back(static_cast<int>(column));
}

void TuningModel::removeColumn(Columns column)
{
	for (auto it = m_columnsIndexes.begin(); it != m_columnsIndexes.end(); it++)
	{
		if (*it == static_cast<int>(column))
		{
			m_columnsIndexes.erase(it);
			break;
		}
	}
}

int TuningModel::columnIndex(int index) const
{
	if (index <0 || index >= m_columnsIndexes.size())
	{
		assert(false);
		return -1;
	}

	return m_columnsIndexes[index];
}

std::vector<int> TuningModel::columnsIndexes()
{
	return m_columnsIndexes;

}

void TuningModel::setColumnsIndexes(std::vector<int> columnsIndexes)
{
	if (columnCount() > 0)
	{
		beginRemoveColumns(QModelIndex(), 0, columnCount() - 1);

		removeColumns(0, columnCount());

		m_columnsIndexes.clear();

		endRemoveColumns();
	}

	beginInsertColumns(QModelIndex(), 0, static_cast<int>(columnsIndexes.size()) - 1);

	m_columnsIndexes = columnsIndexes;

	insertColumns(0, static_cast<int>(m_columnsIndexes.size()));

	endInsertColumns();

}

void TuningModel::setFont(const QString& fontName, int fontSize, bool fontBold)
{
	if (m_font != nullptr)
	{
		delete m_font;
	}
	m_font = new QFont(fontName, fontSize);
	m_font->setBold(fontBold);
}

void TuningModel::setImportantFont(const QString& fontName, int fontSize, bool fontBold)
{
	if (m_importantFont != nullptr)
	{
		delete m_importantFont;
	}
	m_importantFont = new QFont(fontName, fontSize);
	m_importantFont->setBold(fontBold);

}

int TuningModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_hashes.size());

}

int TuningModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_columnsIndexes.size());

}

QModelIndex TuningModel::index(int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

void TuningModel::sort(int column, Qt::SortOrder order)
{
	if (column < 0 || column >= m_columnsIndexes.size())
	{
		assert(false);
		return;
	}

	if (m_hashes.empty() == true)
	{
		return;
	}

	int sortColumnIndex = m_columnsIndexes[column];

	std::sort(m_hashes.begin(), m_hashes.end(), TuningModelSorter(sortColumnIndex, order, this, m_tuningSignalManager));

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

	return;
}

bool TuningModel::limitsUnbalance(const AppSignalParam& asp, const TuningSignalState& tss) const
{
	if (tss.valid() == true && asp.isAnalog() == true)
	{
		if (tss.lowBound() != asp.tuningLowBound() || tss.highBound() != asp.tuningHighBound())
		{
			return true;
		}
	}
	return false;

}

QModelIndex TuningModel::parent(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QModelIndex();

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
		int displayIndex = m_columnsIndexes[col];

		if (m_importantFont != nullptr && displayIndex == static_cast<int>(Columns::Value))
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
		int displayIndex = m_columnsIndexes[col];

		if (displayIndex >= static_cast<int>(Columns::Value))
		{
			return Qt::AlignCenter;
		}
		return Qt::AlignLeft + Qt::AlignVCenter;
	}

	if (role == Qt::DisplayRole)
	{

		int col = index.column();

		if (col < 0 || col >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int row = index.row();
		if (row >= m_hashes.size())
		{
			assert(false);
			return QVariant();
		}

		Hash hash = m_hashes[row];

		//QString str = QString("%1:%2").arg(row).arg(col);
		//qDebug() << str;

		bool ok = false;

		const AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == static_cast<int>(Columns::CustomAppSignalID))
		{
			return asp.customSignalId();
		}

		if (displayIndex == static_cast<int>(Columns::EquipmentID))
		{
			return asp.equipmentId();
		}

		if (displayIndex == static_cast<int>(Columns::AppSignalID))
		{
			return asp.appSignalId();
		}

		if (displayIndex == static_cast<int>(Columns::Caption))
		{
			return asp.caption();
		}

		if (displayIndex == static_cast<int>(Columns::Units))
		{
			return asp.unit();
		}

		if (displayIndex == static_cast<int>(Columns::Type))
		{
			return asp.tuningDefaultValue().tuningValueTypeString();
		}

		//
		// State
		//

		const TuningSignalState tss = m_tuningSignalManager->state(hash, &ok);

		if (displayIndex == static_cast<int>(Columns::Value))
		{

			if (tss.controlIsEnabled() == false)
			{
				return tr("Disabled");
			}
			else
			{
				if (tss.valid() == true)
				{
					if (asp.isAnalog() == false)
					{
						QString valueString = tss.value().toString();

						if (tss.userModified() == true)
						{
							QString editValueString = tss.newValue().toString();
							return tr("%1 => %2").arg(valueString).arg(editValueString);
						}

						if (tss.writeInProgress() == true)
						{
							QString editValueString = tss.newValue().toString();
							return tr("Writing %1").arg(editValueString);
						}

						return valueString;
					}
					else
					{
						QString valueString = tss.value().toString(asp.precision());

						if (tss.userModified() == true)
						{
							QString editValueString = tss.newValue().toString(asp.precision());
							return QString("%1 => %2").arg(valueString).arg(editValueString);
						}

						if (tss.writeInProgress() == true)
						{
							QString editValueString = tss.newValue().toString(asp.precision());
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

		if (displayIndex == static_cast<int>(Columns::LowLimit))
		{
			if (asp.isAnalog())
			{
				if (limitsUnbalance(asp, tss))
				{
					QString str = tr("Base %1, read %2")
							.arg(asp.tuningLowBound().toString(asp.precision()))
							.arg(tss.lowBound().toString(asp.precision()));
					return str;
				}
				else
				{
					return asp.tuningLowBound().toString(asp.precision());
				}
			}
			else
			{
				return QString();
			}
		}

		if (displayIndex == static_cast<int>(Columns::HighLimit))
		{
			if (asp.isAnalog())
			{
				if (limitsUnbalance(asp, tss))
				{
					QString str = tr("Base %1, read %2")
							.arg(asp.tuningHighBound().toString(asp.precision()))
							.arg(tss.highBound().toString(asp.precision()));
					return str;
				}
				else
				{
					return asp.tuningHighBound().toString(asp.precision());
				}
			}
			else
			{
				return QString();
			}
		}

		if (displayIndex == static_cast<int>(Columns::Default))
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

		if (displayIndex == static_cast<int>(Columns::Valid))
		{
			return (tss.valid() == true) ? tr("") : tr("VALID");
		}

		if (displayIndex == static_cast<int>(Columns::OutOfRange))
		{
			return (tss.outOfRange() == true) ? tr("RANGE") : tr("");
		}
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
	connect(m_discreteCheck, &QCheckBox::toggled, this, &DialogInputTuningValue::on_m_checkBox_clicked);

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

		if (sameValue == true)
		{
			m_discreteCheck->setChecked(value.discreteValue() != 0);
			m_discreteCheck->setText(value.toString());
		}
		else
		{
			m_discreteCheck->setTristate(true);
			m_discreteCheck->setCheckState(Qt::PartiallyChecked);
			m_discreteCheck->setText(tr("Unknown"));
		}

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

void DialogInputTuningValue::on_m_checkBox_clicked(bool checked)
{
	m_discreteCheck->setText(checked ? tr("1") : tr("0"));
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
