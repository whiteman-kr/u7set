#include "Settings.h"
#include "MainWindow.h"
#include "TuningPage.h"
#include "DialogInputValue.h"
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

bool TuningItemSorter::sortFunction(const TuningObject& o1, const TuningObject& o2, int column, Qt::SortOrder order) const
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
			v1 = o1.valid();
			v2 = o2.valid();
		}
		break;
	case TuningItemModel::Columns::Underflow:
		{
			v1 = o1.underflow();
			v2 = o2.underflow();
		}
		break;
	case TuningItemModel::Columns::Overflow:
		{
			v1 = o1.overflow();
			v2 = o2.overflow();
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

void TuningItemModel::setObjectsIndexes(const std::vector<TuningObject>& allObjects, const std::vector<int>& objectsIndexes)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_objects.clear();

		endRemoveRows();
	}

	//
	if (objectsIndexes.size() > 0)
	{

		beginInsertRows(QModelIndex(), 0, (int)objectsIndexes.size() - 1);

		for (int index : objectsIndexes)
		{
			m_objects.push_back(allObjects[index]);
		}

		insertRows(0, (int)objectsIndexes.size());

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

	beginInsertColumns(QModelIndex(), 0, (int)columnsIndexes.size() - 1);

	m_columnsIndexes = columnsIndexes;

	insertColumns(0, (int)m_columnsIndexes.size());

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

void TuningItemModel::updateStates()
{
	static int counter = 0;

	if (counter++ >= 2)
	{
		counter = 0;
		m_blink = !m_blink;
	}

    if (m_objects.size() == 0)
	{
		return;
	}

    QMutexLocker l(&theObjectManager->m_mutex);

    int count = (int)m_objects.size();

    for (int i = 0; i < count; i++)
	{
        TuningObject& pageObject = m_objects[i];

        TuningObject* baseObject = theObjectManager->objectPtrByHash(pageObject.appSignalHash());

        if (baseObject == nullptr)
        {
            assert(false);
            continue;
        }

        pageObject.setReadLowLimit(baseObject->readLowLimit());
        pageObject.setReadHighLimit(baseObject->readHighLimit());
        pageObject.setValue(baseObject->value());
        pageObject.setValid(baseObject->valid());
        pageObject.setWriting(baseObject->writing());
	}

    l.unlock();

	return;
}

TuningObject* TuningItemModel::object(int index)
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
	return (int)m_columnsIndexes.size();

}

int TuningItemModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_objects.size();

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

        const TuningObject& o = m_objects[row];

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
            return o.analog() ? tr("Analog") : tr("Discrete");
		}

		//
		// State
		//

        if (displayIndex == static_cast<int>(Columns::Value))
		{
			if (o.valid() == true)
			{
				if (o.analog() == false)
				{
					QString valueString = o.value() == 0 ? tr("No") : tr("Yes");

                    if (o.userModified() == true)
                    {
                        QString editValueString = o.editValue() == 0 ? tr("No") : tr("Yes");
                        return tr("%1 => %2").arg(valueString).arg(editValueString);
                    }

                    if (o.writing() == true)
                    {
                        QString editValueString = o.editValue() == 0 ? tr("No") : tr("Yes");
                        return tr("Writing %1").arg(editValueString);
                    }

                    return valueString;
				}
				else
				{
					QString valueString = QString::number(o.value(), 'f', o.decimalPlaces());

                    if (o.userModified() == true)
                    {
                        QString editValueString = QString::number(o.editValue(), 'f', o.decimalPlaces());
                        return QString("%1 => %2").arg(valueString).arg(editValueString);
                    }

                    if (o.writing() == true)
                    {
                        QString editValueString = QString::number(o.editValue(), 'f', o.decimalPlaces());
                        return tr("Writing %1").arg(editValueString);
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
				return ((int)o.defaultValue() == 0 ? tr("No") : tr("Yes"));
			}
		}

        if (displayIndex == static_cast<int>(Columns::Valid))
		{
            return (o.valid() == true) ? tr("") : tr("VALID");
		}

        if (displayIndex == static_cast<int>(Columns::Underflow))
		{
            return (o.underflow() == true) ? tr("UNDRFLW") : tr("");
		}

        if (displayIndex == static_cast<int>(Columns::Overflow))
		{
            return (o.overflow() == true) ? tr("OVRFLW") : tr("");
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
// TuningItemModelMain
//


TuningItemModelMain::TuningItemModelMain(int tuningPageIndex, QWidget* parent)
	:TuningItemModel(parent)
{
	TuningPageSettings* pageSettings = theSettings.tuningPageSettings(tuningPageIndex);
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (pageSettings->m_columnCount == 0)
	{
        addColumn(Columns::CustomAppSignalID);
        addColumn(Columns::EquipmentID);
        addColumn(Columns::Caption);
        addColumn(Columns::Units);
        addColumn(Columns::Type);

        addColumn(Columns::Value);
        addColumn(Columns::LowLimit);
        addColumn(Columns::HighLimit);
        addColumn(Columns::Default);
        addColumn(Columns::Valid);
        addColumn(Columns::Underflow);
        addColumn(Columns::Overflow);
	}
	else
	{
		m_columnsIndexes  = pageSettings->m_columnsIndexes;
	}
}

void TuningItemModelMain::setValue(const std::vector<int>& selectedRows)
{
	bool first = true;
	bool analog = false;
    float value = 0.0;
	bool sameValue = true;
	int decimalPlaces = 0;
    float lowLimit = 0;
    float highLimit = 0;

	for (int i : selectedRows)
	{
		const TuningObject& o = m_objects[i];

        if (o.valid() == false)
        {
            return;
        }

		if (o.analog() == true)
		{
			if (o.decimalPlaces() > decimalPlaces)
			{
				decimalPlaces = o.decimalPlaces();
			}
		}

		if (first == true)
		{
			analog = o.analog();
            value = o.value();
            lowLimit = o.lowLimit();
            highLimit = o.highLimit();
            first = false;
		}
		else
		{
			if (analog != o.analog())
			{
				QMessageBox::warning(m_parent, tr("Set Value"), tr("Please select one type of objects: analog or discrete."));
				return;
			}

            if (analog == true)
            {
                if (lowLimit != o.lowLimit() || highLimit != o.highLimit())
                {
                    QMessageBox::warning(m_parent, tr("Set Value"), tr("Selected objects have different input range."));
                    return;
                }
            }

            if (value != o.value())
			{
				sameValue = false;
			}
		}
	}

    DialogInputValue d(analog, value, sameValue, lowLimit, highLimit, decimalPlaces, m_parent);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

    float newValue = d.value();

	for (int i : selectedRows)
	{
		TuningObject& o = m_objects[i];
        o.onEditValue(newValue);
	}
}

void TuningItemModelMain::invertValue(const std::vector<int>& selectedRows)
{
	for (int i : selectedRows)
	{
		TuningObject& o = m_objects[i];

        if (o.valid() == false)
        {
            return;
        }

        if (o.analog() == false)
		{
			if (o.editValue() == 0)
			{
                o.onEditValue(1);
			}
			else
			{
                o.onEditValue(0);
			}
		}
	}
}


QBrush TuningItemModelMain::backColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return QBrush();
	}

    if (displayIndex == static_cast<int>(Columns::Value))
	{
		const TuningObject& o = m_objects[row];

        if (m_blink == true && o.userModified() == true)
		{
			QColor color = QColor(Qt::yellow);
			return QBrush(color);
		}

		if (o.valid() == false)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

    if (displayIndex == static_cast<int>(Columns::Valid))
    {
        const TuningObject& o = m_objects[row];

        if (o.valid() == false)
        {
            QColor color = QColor(Qt::red);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::LowLimit) || displayIndex == static_cast<int>(Columns::HighLimit))
    {
        const TuningObject& o = m_objects[row];

        if (o.limitsUnbalance() == true)
        {
            QColor color = QColor(Qt::red);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::Underflow))
    {
        const TuningObject& o = m_objects[row];

        if (o.underflow() == true)
        {
            QColor color = QColor(Qt::red);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::Overflow))
    {
        const TuningObject& o = m_objects[row];

        if (o.overflow() == true)
        {
            QColor color = QColor(Qt::red);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::Default))
	{
		QColor color = QColor(Qt::gray);
		return QBrush(color);
	}

	return QBrush();
}

QBrush TuningItemModelMain::foregroundColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return QBrush();
	}

    if (displayIndex == static_cast<int>(Columns::Valid))
    {
        const TuningObject& o = m_objects[row];

        if (o.valid() == false)
        {
            QColor color = QColor(Qt::white);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::Value))
	{
		const TuningObject& o = m_objects[row];

        if (m_blink == true && o.userModified() == true)
		{
			QColor color = QColor(Qt::black);
			return QBrush(color);
		}

		if (o.valid() == false)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

    if (displayIndex == static_cast<int>(Columns::LowLimit) || displayIndex == static_cast<int>(Columns::HighLimit))
    {
        const TuningObject& o = m_objects[row];

        if (o.limitsUnbalance() == true)
        {
            QColor color = QColor(Qt::white);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::Underflow))
    {
        const TuningObject& o = m_objects[row];

        if (o.underflow() == true)
        {
            QColor color = QColor(Qt::white);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns:: Overflow))
    {
        const TuningObject& o = m_objects[row];

        if (o.overflow() == true)
        {
            QColor color = QColor(Qt::white);
            return QBrush(color);
        }
    }

    if (displayIndex == static_cast<int>(Columns::Default))
	{
		//int displayIndex = m_columnsIndexes[col];
		QColor color = QColor(Qt::white);
		return QBrush(color);
	}

	return QBrush();

}

Qt::ItemFlags TuningItemModelMain::flags(const QModelIndex &index) const
{
	Qt::ItemFlags f = TuningItemModel::flags(index);

	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return f;
	}


    if (displayIndex == static_cast<int>(Columns::Value))
	{
		const TuningObject& o = m_objects[row];


        if (o.valid() == true)
        {
            if (o.analog() == false)
            {
                f |= Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
            }
            else
            {
                f |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
            }
        }

	}

	return f;
}

QVariant TuningItemModelMain::data(const QModelIndex &index, int role) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return QVariant();
	}

	const TuningObject& o = m_objects[row];

    if (role == Qt::CheckStateRole && displayIndex == static_cast<int>(Columns::Value) && o.analog() == false && o.valid() == true)
	{
		return (o.editValue() == 0 ? Qt::Unchecked : Qt::Checked);
	}

	return TuningItemModel::data(index, role);
}

bool TuningItemModelMain::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
	{
		return false;
	}

	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return false;
	}

    if (role == Qt::EditRole && displayIndex == static_cast<int>(TuningItemModel::Columns::Value))
	{
		TuningObject& o = m_objects[row];

		bool ok = false;
        float v = value.toFloat(&ok);
		if (ok == false)
		{
			return false;
		}

        o.onEditValue(v);
		return true;
	}

    if (role == Qt::CheckStateRole && displayIndex == static_cast<int>(TuningItemModel::Columns::Value))
	{
		TuningObject& o = m_objects[row];

		if ((Qt::CheckState)value.toInt() == Qt::Checked)
		{
            o.onEditValue(1.0);
			return true;
		}
		else
		{
            o.onEditValue(0.0);
			return true;
		}
    }
	return false;
}

void TuningItemModelMain::slot_setDefaults()
{
	for (TuningObject& o : m_objects)
	{
        if (o.valid() == false)
        {
            continue;
        }

		if (o.analog() == true)
		{
            o.onEditValue(o.defaultValue());
		}
	}
}

void TuningItemModelMain::slot_setOn()
{
	for (TuningObject& o : m_objects)
	{
        if (o.valid() == false)
        {
            continue;
        }

        if (o.analog() == false)
		{
            o.onEditValue(1);
		}
	}
}

void TuningItemModelMain::slot_setOff()
{
	for (TuningObject& o : m_objects)
	{
        if (o.valid() == false)
        {
            continue;
        }

        if (o.analog() == false)
		{
            o.onEditValue(0);
		}
	}
}

void TuningItemModelMain::slot_undo()
{
	for (TuningObject& o : m_objects)
	{
        o.onEditValue(o.value());
	}
}

void TuningItemModelMain::slot_Apply()
{
	if (theUserManager.requestPassword() == false)
	{
		return;
	}

    QString str = tr("New values will be applied:") + QString("\r\n\r\n");
	QString strValue;

	bool modifiedFound = false;
	int modifiedCount = 0;

	for (TuningObject& o : m_objects)
	{
        if (o.userModified() == false)
		{
			continue;
		}

		modifiedFound = true;
		modifiedCount++;
	}

	if (modifiedFound == false)
	{
		return;
	}

	int listCount = 0;

	for (TuningObject& o : m_objects)
	{
        if (o.userModified() == false)
		{
			continue;
		}

		if (listCount >= 10)
		{
			str += tr("and %1 more values.").arg(modifiedCount - listCount);
			break;
		}

		if (o.analog() == true)
		{
			strValue = QString::number(o.editValue(), 'f', o.decimalPlaces());
		}
		else
		{
			strValue = o.editValue() == 0 ? tr("No") : tr("Yes");
		}

		str += tr("%1 (%2) = %3\r\n").arg(o.appSignalID()).arg(o.caption()).arg(strValue);

		listCount++;
	}

    str += QString("\r\n") + tr("Are you sure you want to continue?");

	if (QMessageBox::warning(m_parent, tr("Apply Changes"),
							 str,
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

    theObjectManager->writeModifiedTuningObjects(m_objects);
}


//
// TuningPage
//
FilterButton::FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filter = filter;
	m_caption = caption;

	setCheckable(true);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<TuningFilter> FilterButton::filter()
{
	return m_filter;
}

void FilterButton::slot_toggled(bool checked)
{
	if (checked == true)
	{
		emit filterButtonClicked(m_filter);
	}

}

//
// TuningPage
//

bool TuningTableView::editorActive()
{
    return m_editorActive;
}

bool TuningTableView::edit(const QModelIndex & index, EditTrigger trigger, QEvent * event)
{

    if (trigger == QAbstractItemView::EditKeyPressed)
    {
        TuningItemModel* m_model = dynamic_cast<TuningItemModel*>(model());
        if (m_model == nullptr)
        {
            assert(m_model);
            return false;
        }

        int row = index.row();

        if (row >= 0)
        {
            TuningObject* object = m_model->object(row);
            if (object == nullptr)
            {
                assert(object);
                return false;
            }

            if (object->analog() == true)
            {
                //qDebug()<<"edit "<<(int)trigger;
                m_editorActive = true;
            }
        }
    }

    return QTableView::edit(index, trigger, event);
}

void TuningTableView::closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{

    //qDebug()<<"closeEditor";

    m_editorActive = false;

    QTableView::closeEditor(editor, hint);
}

//
// TuningPage
//

TuningPage::TuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> tabFilter, QWidget *parent) :
	m_tuningPageIndex(tuningPageIndex),
	QWidget(parent),
	m_tabFilter(tabFilter)
{
	std::vector<FilterButton*> buttons;

	// Top buttons
	//
	int count = theFilters.m_root->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> f = theFilters.m_root->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isButton() == false)
		{
			continue;
		}

		FilterButton* button = new FilterButton(f, f->caption());
		buttons.push_back(button);

		connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);
	}

	// Child buttons
	//
	if (tabFilter != nullptr)
	{
		for (int i = 0; i < tabFilter->childFiltersCount(); i++)
		{
			std::shared_ptr<TuningFilter> f = tabFilter->childFilter(i);
			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			if (f->isButton() == false)
			{
				continue;
			}

			FilterButton* button = new FilterButton(f, f->caption());
			buttons.push_back(button);

			connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);

		}
	}

	if (buttons.empty() == false)
	{
		m_filterButtonGroup = new QButtonGroup(this);

		m_filterButtonGroup->setExclusive(true);

		m_buttonsLayout = new QHBoxLayout();

		for (auto b: buttons)
		{
			m_filterButtonGroup->addButton(b);
			m_buttonsLayout->addWidget(b);
		}

		m_buttonsLayout->addStretch();

		// Set the first button checked
		//
		buttons[0]->blockSignals(true);
		buttons[0]->setChecked(true);
		m_buttonFilter = buttons[0]->filter();
		buttons[0]->blockSignals(false);

	}

	// Object List
	//
    m_objectList = new TuningTableView();

    QFont f = m_objectList->font();

    // Models and data
    //
    m_model = new TuningItemModelMain(m_tuningPageIndex, this);
    m_model->setFont(f.family(), f.pointSize(), false);
    m_model->setImportantFont(f.family(), f.pointSize(), true);

    // Filter controls
	//
    m_filterTypeCombo = new QComboBox();
    m_filterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterType::All));
    m_filterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterType::AppSignalID));
    m_filterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterType::CustomAppSignalID));
    m_filterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterType::EquipmentID));
    m_filterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterType::Caption));

    connect(m_filterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterTypeIndexChanged(int)));

    m_filterTypeCombo->setCurrentIndex(0);

    m_filterEdit = new QLineEdit();
    connect(m_filterEdit, &QLineEdit::returnPressed, this, &TuningPage::slot_ApplyFilter);

    m_filterButton = new QPushButton(tr("Apply Filter"));
    connect(m_filterButton, &QPushButton::clicked, this, &TuningPage::slot_ApplyFilter);

    // Button controls
    //

    m_setValueButton = new QPushButton(tr("Set Value"));
	connect(m_setValueButton, &QPushButton::clicked, this, &TuningPage::slot_setValue);

    m_setOnButton = new QPushButton(tr("Set all to On"));
	connect(m_setOnButton, &QPushButton::clicked, m_model, &TuningItemModelMain::slot_setOn);

    m_setOffButton = new QPushButton(tr("Set all to Off"));
	connect(m_setOffButton, &QPushButton::clicked, m_model, &TuningItemModelMain::slot_setOff);

    m_setToDefaultButton = new QPushButton(tr("Set to Defaults"));
	connect(m_setToDefaultButton, &QPushButton::clicked, m_model, &TuningItemModelMain::slot_setDefaults);

    m_applyButton = new QPushButton(tr("Apply"));
	connect(m_applyButton, &QPushButton::clicked, m_model, &TuningItemModelMain::slot_Apply);

    m_undoButton = new QPushButton(tr("Undo"));
	connect(m_undoButton, &QPushButton::clicked, m_model, &TuningItemModelMain::slot_undo);

	m_bottomLayout = new QHBoxLayout();

    m_bottomLayout->addWidget(m_filterTypeCombo);
    m_bottomLayout->addWidget(m_filterEdit);
    m_bottomLayout->addWidget(m_filterButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_setValueButton);
	m_bottomLayout->addWidget(m_setOnButton);
	m_bottomLayout->addWidget(m_setOffButton);
	m_bottomLayout->addWidget(m_setToDefaultButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_applyButton);
	m_bottomLayout->addWidget(m_undoButton);

	m_mainLayout = new QVBoxLayout(this);

	if (m_buttonsLayout != nullptr)
	{
		m_mainLayout->addLayout(m_buttonsLayout);
	}

	m_mainLayout->addWidget(m_objectList);
	m_mainLayout->addLayout(m_bottomLayout);


	m_objectList->setModel(m_model);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_objectList->setSortingEnabled(true);
    m_objectList->setEditTriggers(QAbstractItemView::EditKeyPressed);

	connect(m_objectList->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &TuningPage::sortIndicatorChanged);

	connect(m_objectList, &QTableView::doubleClicked, this, &TuningPage::slot_tableDoubleClicked);

	m_objectList->installEventFilter(this);


	fillObjectsList();

	TuningPageSettings* pageSettings = theSettings.tuningPageSettings(tuningPageIndex);
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (pageSettings->m_columnCount == 0)
	{
		m_objectList->resizeColumnsToContents();
	}
	else
	{
		for (int i = 0; i < pageSettings->m_columnCount; i++)
		{
            int width = pageSettings->m_columnsWidth[i];
            if (width < 20)
            {
                width = 20;
            }

            m_objectList->setColumnWidth(i, width);
		}
	}

	m_updateStateTimerId = startTimer(250);

}

TuningPage::~TuningPage()
{
	TuningPageSettings* pageSettings = theSettings.tuningPageSettings(m_tuningPageIndex);
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	pageSettings->m_columnsIndexes = m_model->columnsIndexes();
	pageSettings->m_columnCount = (int)pageSettings->m_columnsIndexes.size();

	pageSettings->m_columnsWidth.resize(pageSettings->m_columnCount);
	for (int i = 0; i < pageSettings->m_columnCount; i++)
	{
		pageSettings->m_columnsWidth[i] = m_objectList->columnWidth(i);
	}
}

void TuningPage::fillObjectsList()
{
	std::vector<int> objectsIndexes;

    std::vector<TuningObject> objects = theObjectManager->objects();

    QString filter = m_filterEdit->text();

    FilterType filterType = FilterType::All;
    QVariant data = m_filterTypeCombo->currentData();
    if (data.isValid() == true)
    {
        filterType = static_cast<FilterType>(data.toInt());
    }

	for (int i = 0; i < objects.size(); i++)
	{
        TuningObject& o = objects[i];

		// Tree Filter
		//

        // If currently selected filter is root - all signals are displayed
        //

        if (m_treeFilter != nullptr && m_treeFilter->isRoot() == false)
		{
			bool result = true;

			TuningFilter* treeFilter = m_treeFilter.get();

            // If currently selected filter is empty - no signals are displayed (a "Folder" filter)

            if (treeFilter->isEmpty() == true)
            {
                continue;
            }

            // Otherwise, check parent filters

            while (treeFilter != nullptr)
            {
                if (treeFilter->match(o) == false)
                {
                    result = false;
                    break;
                }

                treeFilter = treeFilter->parentFilter();
            }
            if (result == false)
            {
                continue;
            }

            // Modify the default value from selected filter
            //

            TuningFilterValue filterValue;

            bool hasValue = m_treeFilter->value(o.appSignalHash(), filterValue);

            if (hasValue == true)
            {
                o.setDefaultValue(filterValue.value());
            }
		}

		// Tab Filter
		//

		if (m_tabFilter != nullptr)
		{
			if (m_tabFilter->match(o) == false)
			{
				continue;
			}
		}

		// Button Filter
		//

		if (m_buttonFilter != nullptr)
		{
			if (m_buttonFilter->match(o) == false)
			{
				continue;
			}
		}

        // Text filter
        //

        if (filter.length() != 0)
        {
            bool filterMatch = false;

            switch (filterType)
            {
            case FilterType::All:
                if (o.appSignalID().contains(filter, Qt::CaseInsensitive) == true
                        || o.customAppSignalID().contains(filter, Qt::CaseInsensitive) == true
                        || o.equipmentID().contains(filter, Qt::CaseInsensitive) == true
                        || o.caption().contains(filter, Qt::CaseInsensitive) == true)
                {
                    filterMatch = true;
                }
                break;
            case FilterType::AppSignalID:
                if (o.appSignalID().contains(filter, Qt::CaseInsensitive) == true)
                {
                    filterMatch = true;
                }
                break;
            case FilterType::CustomAppSignalID:
                if (o.customAppSignalID().contains(filter, Qt::CaseInsensitive) == true)
                {
                    filterMatch = true;
                }
                break;
            case FilterType::EquipmentID:
                if (o.equipmentID().contains(filter, Qt::CaseInsensitive) == true)
                {
                    filterMatch = true;
                }
                break;
            case FilterType::Caption:
                if (o.caption().contains(filter, Qt::CaseInsensitive) == true)
                {
                    filterMatch = true;
                }
                break;
            }

            if (filterMatch == false)
            {
                continue;
            }
        }



		objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(objects, objectsIndexes);
	m_objectList->sortByColumn(m_sortColumn, m_sortOrder);
}

void TuningPage::slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	m_buttonFilter = filter;

	fillObjectsList();

}

void TuningPage::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_model->sort(column, order);
}

void TuningPage::slot_setValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedIndexes();

	std::vector<int> selectedRows;

	for (const QModelIndex i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == false)
	{
		m_model->setValue(selectedRows);
	}
}

void TuningPage::invertValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == false)
	{
		m_model->invertValue(selectedRows);
	}
}

void TuningPage::slot_tableDoubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	slot_setValue();
}

void TuningPage::slot_FilterTypeIndexChanged(int index)
{
    Q_UNUSED(index);
    fillObjectsList();
}

void TuningPage::slot_ApplyFilter()
{
    fillObjectsList();
}

void TuningPage::slot_filterTreeChanged(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		//qDebug()<<"Filter tree removed.";
	}
	else
	{
		//qDebug()<<"Filter tree clicked: "<<filter->caption();
	}

	m_treeFilter = filter;

	fillObjectsList();
}

void TuningPage::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId && m_model->rowCount() > 0 && isVisible() == true)
	{
		// Update only visible dynamic items
		//
		int from = m_objectList->rowAt(0);
		int to = m_objectList->rowAt(m_objectList->height() - m_objectList->horizontalHeader()->height());

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
        m_model->updateStates();

        // Redraw visible table items
        //
        for (int row = from; row <= to; row++)
        {
            TuningObject* o = m_model->object(row);

            if (o == nullptr)
            {
                assert(o);
                continue;
            }

            if (o->redraw() == true || o->userModified() == true)
            {
                for (int col = 0; col < m_model->columnCount(); col++)
                {
                    int displayIndex = m_model->columnIndex(col);

                    if (displayIndex >= static_cast<int>(TuningItemModel::Columns::Value))
                    {
                        //QString str = QString("%1:%2").arg(row).arg(col);
                        //qDebug()<<str;

                        m_objectList->update(m_model->index(row, col));
                    }
                }
            }
        }
    }
}

bool TuningPage::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_objectList && event->type()==QEvent::KeyPress)
	{
		QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
		if(pKeyEvent->key() == Qt::Key_Return)
		{
            if (m_objectList->editorActive() == false)
            {
                m_model->slot_Apply();
                return true;
            }
            return true;
		}

		if(pKeyEvent->key() == Qt::Key_Space)
		{
			invertValue();
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}
