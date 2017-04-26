//#include "Settings.h"
//#include "MainWindow.h"
#include "TuningModel.h"
//#include "DialogInputValue.h"
#include <QKeyEvent>

using namespace std;

//
// TuningItemSorter
//

TuningModelRecordSorter::TuningModelRecordSorter(int column, Qt::SortOrder order):
	m_column(column),
	m_order(order)
{
}

bool TuningModelRecordSorter::sortFunction(const TuningModelRecord &o1, const TuningModelRecord &o2, int column, Qt::SortOrder order) const
{

	QVariant v1;
	QVariant v2;

	switch (static_cast<TuningModel::Columns>(column))
	{
	case TuningModel::Columns::CustomAppSignalID:
		{
			v1 = o1.param.customSignalId();
			v2 = o2.param.customSignalId();
		}
		break;
	case TuningModel::Columns::EquipmentID:
		{
			v1 = o1.param.equipmentId();
			v2 = o2.param.equipmentId();
		}
		break;
	case TuningModel::Columns::AppSignalID:
		{
			v1 = o1.param.appSignalId();
			v2 = o2.param.appSignalId();
		}
		break;
	case TuningModel::Columns::Caption:
		{
			v1 = o1.param.caption();
			v2 = o2.param.caption();
		}
		break;
	case TuningModel::Columns::Units:
		{
			v1 = o1.param.unitId();
			v2 = o2.param.unitId();
		}
		break;
	case TuningModel::Columns::Type:
		{
			v1 = o1.param.isAnalog();
			v2 = o2.param.isAnalog();
		}
		break;

	case TuningModel::Columns::Default:
		{
			if (o1.param.isAnalog() == o2.param.isAnalog())
			{
				v1 = o1.param.tuningDefaultValue();
				v2 = o2.param.tuningDefaultValue();
			}
			else
			{
				v1 = o1.param.isAnalog();
				v2 = o2.param.isAnalog();
			}
		}
		break;
	case TuningModel::Columns::Value:
		{
			if (o1.param.isAnalog() == o2.param.isAnalog())
			{
				v1 = o1.state.value();
				v2 = o2.state.value();
			}
			else
			{
				v1 = o1.param.isAnalog();
				v2 = o2.param.isAnalog();
			}
		}
		break;
    case TuningModel::Columns::LowLimit:
        {
			if (o1.param.isAnalog() == true && o2.param.isAnalog() == true)
            {
				v1 = o1.param.lowEngeneeringUnits();
				v2 = o2.param.lowEngeneeringUnits();
            }
            else
            {
				v1 = o1.param.isAnalog();
				v2 = o2.param.isAnalog();
            }
        }
        break;
    case TuningModel::Columns::HighLimit:
        {
			if (o1.param.isAnalog() == true && o2.param.isAnalog() == true)
            {
				v1 = o1.param.highEngeneeringUnits();
				v2 = o2.param.highEngeneeringUnits();
            }
            else
            {
				v1 = o1.param.isAnalog();
				v2 = o2.param.isAnalog();
            }
        }
        break;
	case TuningModel::Columns::Valid:
		{
			v1 = o1.state.valid();
			v2 = o2.state.valid();
		}
		break;
	case TuningModel::Columns::Underflow:
		{
			v1 = o1.state.underflow();
			v2 = o2.state.underflow();
		}
		break;
	case TuningModel::Columns::Overflow:
		{
			v1 = o1.state.overflow();
			v2 = o2.state.overflow();
		}
		break;
	default:
		assert(false);
		return false;
	}

	if (v1 == v2)
	{
		return o1.param.customSignalId() < o2.param.customSignalId();
	}

	if (order == Qt::AscendingOrder)
		return v1 < v2;
	else
		return v1 > v2;
}

//
// TuningItemModel
//

TuningModel::TuningModel(QWidget *parent)
	:QAbstractItemModel(parent),
	m_parent(parent)
{
	// Fill column names

	m_columnsNames<<tr("Custom AppSignal ID");
	m_columnsNames<<tr("Equipment ID");
	m_columnsNames<<tr("App Signal ID");
	m_columnsNames<<tr("Caption");
	m_columnsNames<<tr("Units");
	m_columnsNames<<tr("Type");

	m_columnsNames<<tr("Value");
    m_columnsNames<<tr("LowLimit");
    m_columnsNames<<tr("HighLimit");
    m_columnsNames<<tr("Default");
    m_columnsNames<<tr("Valid");
	m_columnsNames<<tr("Underflow");
	m_columnsNames<<tr("Overflow");
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

void TuningModel::setSignals(std::vector<TuningModelRecord>& signalsList)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_items.clear();

		endRemoveRows();
	}

	//
	if (signalsList.empty() == false)
	{
		int count = static_cast<int>(signalsList.size());

		beginInsertRows(QModelIndex(), 0, count - 1);

		m_items.swap(signalsList);

		insertRows(0, static_cast<int>(count));

		endInsertRows();
	}

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

int TuningModel::columnIndex(int index) const
{
	if (index <0 || index >= m_columnsIndexes.size())
	{
		assert(false);
		return -1;
	}

	return m_columnsIndexes[index];
}


AppSignalParam* TuningModel::param(int index)
{
	if (index < 0 || index >= m_items.size())
	{
		assert(false);
        return nullptr;
	}
	return &m_items[index].param;
}

TuningSignalState* TuningModel::state(int index)
{
	if (index < 0 || index >= m_items.size())
	{
		assert(false);
		return nullptr;
	}
	return &m_items[index].state;
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

QModelIndex TuningModel::index(int row, int column, const QModelIndex &parent) const
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

	int sortColumnIndex = m_columnsIndexes[column];

	std::sort(m_items.begin(), m_items.end(), TuningModelRecordSorter(sortColumnIndex, order));

	if (m_items.empty() == false)
	{
		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	}

	return;
}

QModelIndex TuningModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();

}

int TuningModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_columnsIndexes.size());

}

int TuningModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_items.size());

}

QVariant TuningModel::data(const QModelIndex &index, int role) const
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
            return *m_importantFont;
        }

        if (m_font != nullptr)
        {
            return *m_font;
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
		if (row >= m_items.size())
		{
			assert(false);
			return QVariant();
		}

        //QString str = QString("%1:%2").arg(row).arg(col);
        //qDebug()<<str;

		const TuningModelRecord& item = m_items[row];

		int displayIndex = m_columnsIndexes[col];

        if (displayIndex == static_cast<int>(Columns::CustomAppSignalID))
		{
			return item.param.customSignalId();
		}

        if (displayIndex == static_cast<int>(Columns::EquipmentID))
		{
			return item.param.equipmentId();
		}

        if (displayIndex == static_cast<int>(Columns::AppSignalID))
		{
			return item.param.appSignalId();
		}

        if (displayIndex == static_cast<int>(Columns::Caption))
		{
			return item.param.caption();
		}

        if (displayIndex == static_cast<int>(Columns::Units))
		{
			return item.param.unit();
		}

        if (displayIndex == static_cast<int>(Columns::Type))
		{
			return item.param.isAnalog() ? tr("A") : tr("D");
		}

		//
		// State
		//

        if (displayIndex == static_cast<int>(Columns::Value))
		{
			if (item.state.valid() == true)
			{
				if (item.param.isAnalog() == false)
				{
					QString valueString = item.state.value() == 0 ? tr("0") : tr("1");

					if (item.state.userModified() == true)
                    {
						QString editValueString = item.state.editValue() == 0 ? tr("0") : tr("1");
                        return tr("%1 => %2").arg(valueString).arg(editValueString);
                    }

					if (item.state.writing() == true)
                    {
						QString editValueString = item.state.editValue() == 0 ? tr("0") : tr("1");
                        return tr("Writing %1").arg(editValueString);
                    }

                    return valueString;
				}
				else
				{
					QString valueString = QString::number(item.state.value(), 'f', item.param.precision());

					if (item.state.userModified() == true)
                    {
						QString editValueString = QString::number(item.state.editValue(), 'f', item.param.precision());
                        return QString("%1 => %2").arg(valueString).arg(editValueString);
                    }

					if (item.state.writing() == true)
                    {
						QString editValueString = QString::number(item.state.editValue(), 'f', item.param.precision());
                        return tr("Writing %1").arg(editValueString);
                    }

					if (item.state.underflow() == true)
                    {
                        return tr("UNDRFLW");
                    }

					if (item.state.overflow() == true)
                    {
                        return tr("OVERFLW");
                    }

                    return valueString;
				}
			}
			else
			{
                return "?";
			}
		}

        if (displayIndex == static_cast<int>(Columns::LowLimit))
        {
			if (item.param.isAnalog())
            {
				if (item.limitsUnbalance())
                {
					QString str = tr("Base %1, read %2")
							.arg(QString::number(item.param.lowEngeneeringUnits(), 'f', item.param.precision()))
							.arg(QString::number(item.state.readLowLimit(), 'f', item.param.precision()));
					return str;
                }
                else
                {
					return QString::number(item.param.lowEngeneeringUnits(), 'f', item.param.precision());
                }
            }
            else
            {
                return "";
            }
        }

        if (displayIndex == static_cast<int>(Columns::HighLimit))
        {
			if (item.param.isAnalog())
            {
				if (item.limitsUnbalance())
				{
					QString str = tr("Base %1, read %2")
							.arg(QString::number(item.param.highEngeneeringUnits(), 'f', item.param.precision()))
							.arg(QString::number(item.state.readHighLimit(), 'f', item.param.precision()));
					return str;
                }
                else
                {
					return QString::number(item.param.highEngeneeringUnits(), 'f', item.param.precision());
                }
            }
            else
            {
                return "";
            }
        }

        if (displayIndex == static_cast<int>(Columns::Default))
		{
			if (item.param.isAnalog())
			{
				return QString::number(item.param.tuningDefaultValue(), 'f', item.param.precision());
			}
			else
			{
				return (static_cast<int>(item.param.tuningDefaultValue()) == 0 ? tr("0") : tr("1"));
			}
		}

        if (displayIndex == static_cast<int>(Columns::Valid))
		{
			return (item.state.valid() == true) ? tr("") : tr("VALID");
		}

        if (displayIndex == static_cast<int>(Columns::Underflow))
		{
			return (item.state.underflow() == true) ? tr("UNDRFLW") : tr("");
		}

        if (displayIndex == static_cast<int>(Columns::Overflow))
		{
			return (item.state.overflow() == true) ? tr("OVRFLW") : tr("");
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

DialogInputTuningValue::DialogInputTuningValue(bool analog, float value, float defaultValue, bool sameValue, float lowLimit, float highLimit, int decimalPlaces, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_defaultValue(defaultValue),
    m_analog(analog),
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

    m_discreteCheck->setVisible(analog == false);
    m_analogEdit->setVisible(analog == true);



    if (analog == true)
    {

        QString str = tr("Enter the value (%1 - %2):")
                .arg(QString::number(m_lowLimit, 'f', decimalPlaces))
                .arg(QString::number(m_highLimit, 'f', decimalPlaces));

        setWindowTitle(str);

        if (sameValue == true)
        {
            m_analogEdit->setText(QString::number(value, 'f', decimalPlaces));
            m_analogEdit->selectAll();
        }

        m_buttonDefault->setText(tr("Default: ") + QString::number(m_defaultValue, 'f', m_decimalPlaces));
    }
    else
    {
        setWindowTitle(tr("Enter the value:"));

        if (sameValue == true)
        {
            m_discreteCheck->setChecked(value != 0);
            m_discreteCheck->setText(value != 0 ? tr("1") : tr("0"));
        }
        else
        {
            m_discreteCheck->setTristate(true);
            m_discreteCheck->setCheckState(Qt::PartiallyChecked);
            m_discreteCheck->setText(tr("Unknown"));
        }

        m_buttonDefault->setText(tr("Default: ") + QString::number(m_defaultValue, 'f', 0));
    }
}

DialogInputTuningValue::~DialogInputTuningValue()
{
}

void DialogInputTuningValue::accept()
{
    if (m_analog == true)
    {
        QString text = m_analogEdit->text();
        if (text.isEmpty() == true)
        {
            QMessageBox::critical(this, tr("Error"), tr("Please enter the value."));
            return;
        }

        bool ok = false;
        m_value = text.toFloat(&ok);

        if (ok == false)
        {
            QMessageBox::critical(this, tr("Error"), tr("The value is incorrect."));
            return;
        }

        if (m_value < m_lowLimit || m_value > m_highLimit)
        {
            QMessageBox::critical(this, tr("Error"), tr("The value is out of range."));
            return;
        }
    }
    else
    {
        if (m_discreteCheck->checkState() == Qt::PartiallyChecked)
        {
            QMessageBox::critical(this, tr("Error"), tr("Please select the value."));
            return;
        }

        if (m_discreteCheck->checkState() == Qt::Checked)
        {
            m_value = 1;
        }
        else
        {
            m_value = 0;
        }
    }


    QDialog::accept();
}

void DialogInputTuningValue::on_m_checkBox_clicked(bool checked)
{
    m_discreteCheck->setText(checked ? tr("1") : tr("0"));
}

void DialogInputTuningValue::on_m_buttonDefault_clicked()
{
    if (m_analog == true)
    {
        m_analogEdit->setText(QString::number(m_defaultValue, 'f', m_decimalPlaces));
    }
    else
    {
        bool defaultState = m_defaultValue == 0.0 ? false : true;

        m_discreteCheck->setChecked(defaultState);

        m_discreteCheck->setText(defaultState ? tr("1") : tr("0"));
    }
}
