//#include "Settings.h"
//#include "MainWindow.h"
#include "TuningModel.h"
//#include "DialogInputValue.h"
#include <QKeyEvent>

using namespace std;

//
// TuningItemSorter
//

TuningItemSorter::TuningItemSorter(int column, Qt::SortOrder order):
	m_column(column),
	m_order(order)
{
}

bool TuningItemSorter::sortFunction(const TuningSignal& o1, const TuningSignal& o2, int column, Qt::SortOrder order) const
{

	QVariant v1;
	QVariant v2;

	switch (static_cast<TuningItemModel::Columns>(column))
	{
	case TuningItemModel::Columns::CustomAppSignalID:
		{
			v1 = o1.customAppSignalID();
			v2 = o2.customAppSignalID();
		}
		break;
	case TuningItemModel::Columns::EquipmentID:
		{
			v1 = o1.equipmentID();
			v2 = o2.equipmentID();
		}
		break;
	case TuningItemModel::Columns::AppSignalID:
		{
			v1 = o1.appSignalID();
			v2 = o2.appSignalID();
		}
		break;
	case TuningItemModel::Columns::Caption:
		{
			v1 = o1.caption();
			v2 = o2.caption();
		}
		break;
	case TuningItemModel::Columns::Units:
		{
			v1 = o1.units();
			v2 = o2.units();
		}
		break;
	case TuningItemModel::Columns::Type:
		{
			v1 = o1.analog();
			v2 = o2.analog();
		}
		break;

	case TuningItemModel::Columns::Default:
		{
			if (o1.analog() == o2.analog())
			{
				v1 = o1.defaultValue();
				v2 = o2.defaultValue();
			}
			else
			{
				v1 = o1.analog();
				v2 = o2.analog();
			}
		}
		break;
	case TuningItemModel::Columns::Value:
		{
			if (o1.analog() == o2.analog())
			{
				v1 = o1.value();
				v2 = o2.value();
			}
			else
			{
				v1 = o1.analog();
				v2 = o2.analog();
			}
		}
		break;
    case TuningItemModel::Columns::LowLimit:
        {
            if (o1.analog() == true && o2.analog() == true)
            {
                v1 = o1.lowLimit();
                v2 = o2.lowLimit();
            }
            else
            {
                v1 = o1.analog();
                v2 = o2.analog();
            }
        }
        break;
    case TuningItemModel::Columns::HighLimit:
        {
            if (o1.analog() == true && o2.analog() == true)
            {
                v1 = o1.highLimit();
                v2 = o2.highLimit();
            }
            else
            {
                v1 = o1.analog();
                v2 = o2.analog();
            }
        }
        break;
    case TuningItemModel::Columns::Valid:
		{
			v1 = o1.state.valid();
			v2 = o2.state.valid();
		}
		break;
	case TuningItemModel::Columns::Underflow:
		{
			v1 = o1.state.underflow();
			v2 = o2.state.underflow();
		}
		break;
	case TuningItemModel::Columns::Overflow:
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
		return o1.customAppSignalID() < o2.customAppSignalID();
	}

	if (order == Qt::AscendingOrder)
		return v1 < v2;
	else
		return v1 > v2;
}

//
// TuningItemModel
//

TuningItemModel::TuningItemModel(QWidget *parent)
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

TuningItemModel::~TuningItemModel()
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

void TuningItemModel::setObjects(std::vector<TuningSignal>& objects)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_objects.clear();

		endRemoveRows();
	}

	//
    if (objects.empty() == false)
	{
        beginInsertRows(QModelIndex(), 0, static_cast<int>(objects.size()) - 1);

        m_objects.swap(objects);

        insertRows(0, static_cast<int>(objects.size()));

		endInsertRows();
	}

}

std::vector<int> TuningItemModel::columnsIndexes()
{
	return m_columnsIndexes;

}

void TuningItemModel::setColumnsIndexes(std::vector<int> columnsIndexes)
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

int TuningItemModel::columnIndex(int index) const
{
	if (index <0 || index >= m_columnsIndexes.size())
	{
		assert(false);
		return -1;
	}

	return m_columnsIndexes[index];
}


TuningSignal* TuningItemModel::object(int index)
{
	if (index < 0 || index >= m_objects.size())
	{
		assert(false);
        return nullptr;
	}
    return &m_objects[index];
}

void TuningItemModel::setFont(const QString& fontName, int fontSize, bool fontBold)
{
	if (m_font != nullptr)
	{
		delete m_font;
	}
    m_font = new QFont(fontName, fontSize);
    m_font->setBold(fontBold);
}

void TuningItemModel::setImportantFont(const QString& fontName, int fontSize, bool fontBold)
{
    if (m_importantFont != nullptr)
    {
        delete m_importantFont;
    }
    m_importantFont = new QFont(fontName, fontSize);
    m_importantFont->setBold(fontBold);

}

void TuningItemModel::addColumn(Columns column)
{
    m_columnsIndexes.push_back(static_cast<int>(column));
}

void TuningItemModel::removeColumn(Columns column)
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

QModelIndex TuningItemModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

void TuningItemModel::sort(int column, Qt::SortOrder order)
{
	if (column < 0 || column >= m_columnsIndexes.size())
	{
		assert(false);
		return;
	}

	int sortColumnIndex = m_columnsIndexes[column];

	std::sort(m_objects.begin(), m_objects.end(), TuningItemSorter(sortColumnIndex, order));

	if (m_objects.empty() == false)
	{
		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	}

	return;
}

QModelIndex TuningItemModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();

}

int TuningItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_columnsIndexes.size());

}

int TuningItemModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_objects.size());

}

QVariant TuningItemModel::data(const QModelIndex &index, int role) const
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
		if (row >= m_objects.size())
		{
			assert(false);
			return QVariant();
		}

        //QString str = QString("%1:%2").arg(row).arg(col);
        //qDebug()<<str;

        const TuningSignal& o = m_objects[row];

		int displayIndex = m_columnsIndexes[col];

        if (displayIndex == static_cast<int>(Columns::CustomAppSignalID))
		{
			return o.customAppSignalID();
		}

        if (displayIndex == static_cast<int>(Columns::EquipmentID))
		{
			return o.equipmentID();
		}

        if (displayIndex == static_cast<int>(Columns::AppSignalID))
		{
			return o.appSignalID();
		}

        if (displayIndex == static_cast<int>(Columns::Caption))
		{
			return o.caption();
		}

        if (displayIndex == static_cast<int>(Columns::Units))
		{
			return o.units();
		}

        if (displayIndex == static_cast<int>(Columns::Type))
		{
            return o.analog() ? tr("A") : tr("D");
		}

		//
		// State
		//

        if (displayIndex == static_cast<int>(Columns::Value))
		{
			if (o.state.valid() == true)
			{
				if (o.analog() == false)
				{
                    QString valueString = o.value() == 0 ? tr("0") : tr("1");

					if (o.state.userModified() == true)
                    {
                        QString editValueString = o.editValue() == 0 ? tr("0") : tr("1");
                        return tr("%1 => %2").arg(valueString).arg(editValueString);
                    }

					if (o.state.writing() == true)
                    {
                        QString editValueString = o.editValue() == 0 ? tr("0") : tr("1");
                        return tr("Writing %1").arg(editValueString);
                    }

                    return valueString;
				}
				else
				{
					QString valueString = QString::number(o.value(), 'f', o.decimalPlaces());

					if (o.state.userModified() == true)
                    {
                        QString editValueString = QString::number(o.editValue(), 'f', o.decimalPlaces());
                        return QString("%1 => %2").arg(valueString).arg(editValueString);
                    }

					if (o.state.writing() == true)
                    {
                        QString editValueString = QString::number(o.editValue(), 'f', o.decimalPlaces());
                        return tr("Writing %1").arg(editValueString);
                    }

					if (o.state.underflow() == true)
                    {
                        return tr("UNDRFLW");
                    }

					if (o.state.overflow() == true)
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
            if (o.analog())
            {
                if (o.limitsUnbalance())
                {
                    QString s = tr("Base %1, read %2")
                            .arg(QString::number(o.lowLimit(), 'f', o.decimalPlaces()))
                            .arg(QString::number(o.readLowLimit(), 'f', o.decimalPlaces()));
                    return s;
                }
                else
                {
                    return QString::number(o.lowLimit(), 'f', o.decimalPlaces());
                }
            }
            else
            {
                return "";
            }
        }

        if (displayIndex == static_cast<int>(Columns::HighLimit))
        {
            if (o.analog())
            {
                if (o.limitsUnbalance())
                {
                    QString s = tr("Base %1, read %2")
                            .arg(QString::number(o.highLimit(), 'f', o.decimalPlaces()))
                            .arg(QString::number(o.readHighLimit(), 'f', o.decimalPlaces()));
                    return s;
                }
                else
                {
                    return QString::number(o.highLimit(), 'f', o.decimalPlaces());
                }
            }
            else
            {
                return "";
            }
        }

        if (displayIndex == static_cast<int>(Columns::Default))
		{
			if (o.analog())
			{
				return QString::number(o.defaultValue(), 'f', o.decimalPlaces());
			}
			else
			{
                return (static_cast<int>(o.defaultValue()) == 0 ? tr("0") : tr("1"));
			}
		}

        if (displayIndex == static_cast<int>(Columns::Valid))
		{
			return (o.state.valid() == true) ? tr("") : tr("VALID");
		}

        if (displayIndex == static_cast<int>(Columns::Underflow))
		{
			return (o.state.underflow() == true) ? tr("UNDRFLW") : tr("");
		}

        if (displayIndex == static_cast<int>(Columns::Overflow))
		{
			return (o.state.overflow() == true) ? tr("OVRFLW") : tr("");
		}
	}
	return QVariant();
}

QBrush TuningItemModel::backColor(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QBrush();
}

QBrush TuningItemModel::foregroundColor(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QBrush();
}


QVariant TuningItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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
