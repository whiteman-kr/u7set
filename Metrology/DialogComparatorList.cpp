#include "DialogComparatorList.h"

#include "UnitsConvertor.h"

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant ComparatorListTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= count())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > m_columnCount)
	{
		return QVariant();
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = at(row);
	if (comparatorEx == nullptr)
	{
		return QVariant();
	}

	Metrology::Signal* pInSignal = theSignalBase.signalPtr(comparatorEx->input().appSignalID());
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case COMPARATOR_LIST_COLUMN_INPUT:			result = Qt::AlignLeft;		break;
			case COMPARATOR_LIST_COLUMN_SETPOINT:		result = Qt::AlignLeft;		break;
			case COMPARATOR_LIST_COLUMN_HYSTERESIS:		result = Qt::AlignLeft;		break;
			case COMPARATOR_LIST_COLUMN_TYPE:			result = Qt::AlignCenter;	break;
			case COMPARATOR_LIST_COLUMN_EL_RANGE:		result = Qt::AlignCenter;	break;
			case COMPARATOR_LIST_COLUMN_EL_SENSOR:		result = Qt::AlignCenter;	break;
			case COMPARATOR_LIST_COLUMN_EN_RANGE:		result = Qt::AlignCenter;	break;
			case COMPARATOR_LIST_COLUMN_OUTPUT:			result = Qt::AlignLeft;		break;
			case COMPARATOR_LIST_COLUMN_SCHEMA:			result = Qt::AlignLeft;		break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::ForegroundRole)
	{
		if (comparatorEx->signalsIsValid()  == false)
		{
			return QColor(Qt::red);
		}
		else
		{
			if (column == COMPARATOR_LIST_COLUMN_HYSTERESIS)
			{
				if (comparatorEx->deviation() != Metrology::ComparatorEx::DeviationType::Unused)
				{
					return QColor(Qt::lightGray);
				}
			}
		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (pInSignal->param().isInput() == true)
		{
			if (column == COMPARATOR_LIST_COLUMN_EL_RANGE)
			{
				if (pInSignal->param().electricRangeIsValid() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
			}

			if (column == COMPARATOR_LIST_COLUMN_EL_SENSOR)
			{
				if (pInSignal->param().electricSensorType() == E::SensorType::NoSensor)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pInSignal, comparatorEx);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorListTable::text(int row, int column, Metrology::Signal* pInSignal, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const
{
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	if (pInSignal == nullptr)
	{
		return QString();
	}

	const Metrology::SignalParam& param = pInSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	if (comparatorEx == nullptr)
	{
		return QString();
	}

	bool visible = true;

	if (row > 0)
	{
		std::shared_ptr<Metrology::ComparatorEx> prevComparatorEx = at(row - 1);
		if (prevComparatorEx != nullptr)
		{
			if (prevComparatorEx->input().appSignalID() == param.appSignalID())
			{
				visible = false;
			}
		}
	}

	//
	//
	QString strCompareValue;

	if (comparatorEx->compare().isConst() == true)
	{
		strCompareValue = comparatorEx->compareDefaultValueStr() + " " + pInSignal->param().unit();

		if (pInSignal->param().electricRangeIsValid() == true)
		{
			UnitsConvertor uc;
			double electric = uc.conversion(comparatorEx->compareConstValue(), UnitsConvertType::PhysicalToElectric, pInSignal->param());

			strCompareValue += "  [" + QString::number(electric, 'f', pInSignal->param().electricPrecision()) + " " + pInSignal->param().electricUnitStr() + "]";
		}
	}
	else
	{
		strCompareValue = comparatorEx->compareDefaultValueStr();
	}

	//
	//
	QString strHysteresisValue;

	if (comparatorEx->deviation() == Metrology::ComparatorEx::DeviationType::Unused)
	{
		switch (comparatorEx->cmpType())
		{
			case E::CmpType::Less:		strHysteresisValue = "+ " + comparatorEx->hysteresisDefaultValueStr(); break;
			case E::CmpType::Greate:	strHysteresisValue = "- " + comparatorEx->hysteresisDefaultValueStr(); break;
		}

		if (comparatorEx->hysteresis().isConst() == true)
		{
			strHysteresisValue += " " + pInSignal->param().unit();
		}
	}
	else
	{
		strHysteresisValue = comparatorEx->hysteresisDefaultValueStr();
	}

	//
	//
	QString result;

	switch (column)
	{
		case COMPARATOR_LIST_COLUMN_INPUT:				result = visible ? param.appSignalID() : QString();								break;
		case COMPARATOR_LIST_COLUMN_SETPOINT:			result = strCompareValue;														break;
		case COMPARATOR_LIST_COLUMN_HYSTERESIS:			result = qApp->translate("MetrologySignal", strHysteresisValue.toUtf8());		break;
		case COMPARATOR_LIST_COLUMN_TYPE:				result = qApp->translate("MetrologySignal", param.signalTypeStr().toUtf8());	break;
		case COMPARATOR_LIST_COLUMN_EL_RANGE:			result = param.electricRangeStr();												break;
		case COMPARATOR_LIST_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();											break;
		case COMPARATOR_LIST_COLUMN_EN_RANGE:			result = param.engineeringRangeStr();											break;
		case COMPARATOR_LIST_COLUMN_OUTPUT:				result = comparatorEx->output().appSignalID();									break;
		case COMPARATOR_LIST_COLUMN_SCHEMA:				result = comparatorEx->schemaID();												break;
		default:										assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogComparatorList::DialogComparatorList(QWidget* parent) :
	DialogList(0.8, 0.4, false, parent)
{
	createInterface();
	DialogComparatorList::updateList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogComparatorList::~DialogComparatorList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::createInterface()
{
	setWindowTitle(tr("Comparators"));

	// menu
	//
	m_pComparatorMenu = new QMenu(tr("&Comparator"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	// action
	//
	m_pComparatorMenu->addAction(m_pExportAction);

	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pPropertyAction);

	//
	//
	addMenu(m_pComparatorMenu);
	addMenu(m_pEditMenu);

	//
	//
	m_comparatorTable.setColumnCaption(DialogComparatorList::metaObject()->className(), COMPARATOR_LIST_COLUMN_COUNT, ComparatorListColumn);
	setModel(&m_comparatorTable);

	//
	//
	DialogList::createHeaderContexMenu(COMPARATOR_LIST_COLUMN_COUNT, ComparatorListColumn, ComparatorListColumnWidth);
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::createContextMenu()
{
	addContextAction(m_pCopyAction);
	addContextSeparator();
	addContextAction(m_pPropertyAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::updateVisibleColunm()
{
	for(int c = 0; c < COMPARATOR_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(COMPARATOR_LIST_COLUMN_EL_SENSOR, true);
	hideColumn(COMPARATOR_LIST_COLUMN_SCHEMA, true);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::updateList()
{
	DialogComparatorList::updateVisibleColunm();

	m_comparatorTable.clear();

	std::vector<std::shared_ptr<Metrology::ComparatorEx>> comparatorList;

	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.signalPtr(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.isAnalog() == false)
		{
			continue;
		}

		int comparatorCount = pSignal->param().comparatorCount();
		for (int c = 0; c < comparatorCount; c++)
		{
			std::shared_ptr<Metrology::ComparatorEx> comparatorEx = pSignal->param().comparator(c);
			if (comparatorEx == nullptr)
			{
				continue;
			}

			comparatorList.push_back(pSignal->param().comparator(c));
		}
	}

	m_comparatorTable.set(comparatorList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::onProperties()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	QModelIndex visibleIndex = pView->currentIndex();

	int index = visibleIndex .row();
	if (index < 0 || index >= m_comparatorTable.count())
	{
		return;
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = m_comparatorTable.at(index);
	if (comparatorEx == nullptr)
	{
		return;
	}

	DialogComparatorProperty dialog(*comparatorEx, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	*comparatorEx = dialog.comparator();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
