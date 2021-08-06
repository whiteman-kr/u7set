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

	Metrology::Signal* pInSignal = comparatorEx->inputSignal();
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
			default:
				assert(0);
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
		return text(row, column, comparatorEx);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorListTable::text(int row, int column, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const
{
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	Metrology::Signal* pInSignal = comparatorEx->inputSignal();
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

	// InputSignalID
	//
	QString strInputSignalID;

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

	if (visible == true)
	{
		switch (m_typeID)
		{
			case SignalIDType::CustomID:	strInputSignalID = param.customAppSignalID();	break;
			case SignalIDType::AppSignalID:	strInputSignalID = param.appSignalID();		break;
			case SignalIDType::EquipmentID:	strInputSignalID = param.equipmentID();		break;
			default:
				assert(0);
		}
	}

	// CompareValue
	//
	QString strCompareValue;

	if (comparatorEx->compare().isConst() == true)
	{
		double compareValue = comparatorEx->compare().constValue();
		double hysteresisValue = comparatorEx->hysteresis().constValue()/2;

		switch (comparatorEx->deviation())
		{
			case Metrology::ComparatorEx::DeviationType::Down:	compareValue += -hysteresisValue;	break;
			case Metrology::ComparatorEx::DeviationType::Up:	compareValue += hysteresisValue;	break;
		}

		strCompareValue = QString::number(compareValue, 'f', comparatorEx->valuePrecision()) + " " + pInSignal->param().unit();

		if (pInSignal->param().electricRangeIsValid() == true)
		{
			UnitsConvertor uc;
			double electric = uc.conversion(compareValue, UnitsConvertType::PhysicalToElectric, pInSignal->param());

			strCompareValue += "  [" + QString::number(electric, 'f', pInSignal->param().electricPrecision()) + " " + pInSignal->param().electricUnitStr() + "]";
		}
	}
	else
	{
		Metrology::Signal* pCmpSignal = comparatorEx->compareSignal();
		if (pCmpSignal != nullptr && pCmpSignal->param().isValid() == true)
		{
			switch (m_typeID)
			{
				case SignalIDType::CustomID:	strCompareValue = pCmpSignal->param().customAppSignalID();	break;
				case SignalIDType::AppSignalID:	strCompareValue = pCmpSignal->param().appSignalID();		break;
				case SignalIDType::EquipmentID:	strCompareValue = pCmpSignal->param().equipmentID();		break;
				default:
					assert(0);
			}
		}
	}

	strCompareValue.insert(0, comparatorEx->cmpTypeStr() + " ");

	// HysteresisValue
	//
	QString strHysteresisValue;

	if (comparatorEx->hysteresis().isConst() == true)
	{
		double hysteresisValue = comparatorEx->hysteresis().constValue();

		strHysteresisValue = QString::number(hysteresisValue, 'f', comparatorEx->valuePrecision()) + " " + pInSignal->param().unit();

	}
	else
	{
		Metrology::Signal* pHysSignal = comparatorEx->hysteresisSignal();
		if (pHysSignal != nullptr && pHysSignal->param().isValid() == true)
		{
			switch (m_typeID)
			{
				case SignalIDType::CustomID:	strHysteresisValue = pHysSignal->param().customAppSignalID();	break;
				case SignalIDType::AppSignalID:	strHysteresisValue = pHysSignal->param().appSignalID();			break;
				case SignalIDType::EquipmentID:	strHysteresisValue = pHysSignal->param().equipmentID();			break;
				default:
					assert(0);
			}
		}
	}

	switch (comparatorEx->cmpType())
	{
		case E::CmpType::Less:		strHysteresisValue.insert(0, "+ "); break;
		case E::CmpType::Greate:	strHysteresisValue.insert(0, "- "); break;
	}

	if (comparatorEx->deviation() != Metrology::ComparatorEx::DeviationType::Unused)
	{
		strHysteresisValue = QT_TRANSLATE_NOOP("MetrologySignal", "Unused");
	}

	// OutputSignalID
	//
	QString strOutputSignalID;

	Metrology::Signal* pOutSignal = comparatorEx->outputSignal();
	if (pOutSignal != nullptr && pOutSignal->param().isValid() == true)
	{
		switch (m_typeID)
		{
			case SignalIDType::CustomID:	strOutputSignalID = pOutSignal->param().customAppSignalID();	break;
			case SignalIDType::AppSignalID:	strOutputSignalID = pOutSignal->param().appSignalID();			break;
			case SignalIDType::EquipmentID:	strOutputSignalID = pOutSignal->param().equipmentID();			break;
			default:
				assert(0);
		}
	}

	//
	//
	QString result;

	switch (column)
	{
		case COMPARATOR_LIST_COLUMN_INPUT:				result = strInputSignalID;														break;
		case COMPARATOR_LIST_COLUMN_SETPOINT:			result = strCompareValue;														break;
		case COMPARATOR_LIST_COLUMN_HYSTERESIS:			result = qApp->translate("MetrologySignal", strHysteresisValue.toUtf8());		break;
		case COMPARATOR_LIST_COLUMN_TYPE:				result = qApp->translate("MetrologySignal", param.signalTypeStr().toUtf8());	break;
		case COMPARATOR_LIST_COLUMN_EL_RANGE:			result = param.electricRangeStr();												break;
		case COMPARATOR_LIST_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();											break;
		case COMPARATOR_LIST_COLUMN_EN_RANGE:			result = param.engineeringRangeStr();											break;
		case COMPARATOR_LIST_COLUMN_OUTPUT:				result = strOutputSignalID;														break;
		case COMPARATOR_LIST_COLUMN_SCHEMA:				result = comparatorEx->schemaID();												break;
		default:
			assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalIDType DialogComparatorList::m_typeID = SignalIDType::CustomID;

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
	m_pViewMenu = new QMenu(tr("&View"), this);
	m_pViewTypeIDMenu = new QMenu(tr("Type SignalID"), this);

	// action
	//
	m_pComparatorMenu->addAction(m_pExportAction);

	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pPropertyAction);

	for(int typeID = 0; typeID < SignalIDTypeCount; typeID++)
	{
		m_pTypeIDActionList[typeID] = m_pViewTypeIDMenu->addAction(SignalIDTypeCaption(typeID));
		m_pTypeIDActionList[typeID]->setCheckable(true);
	}

	m_pViewMenu->addMenu(m_pViewTypeIDMenu);

	connect(m_pViewTypeIDMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &DialogComparatorList::showTypeID);

	//
	//
	addMenu(m_pComparatorMenu);
	addMenu(m_pEditMenu);
	addMenu(m_pViewMenu);

	//
	//
	m_comparatorTable.setColumnCaption(DialogComparatorList::metaObject()->className(), COMPARATOR_LIST_COLUMN_COUNT, ComparatorListColumn);
	setModel(&m_comparatorTable);

	//
	//
	DialogList::createHeaderContexMenu(COMPARATOR_LIST_COLUMN_COUNT, ComparatorListColumn, ComparatorListColumnWidth);
	createContextMenu();

	//
	//
	setTypeID(m_typeID);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::createContextMenu()
{
	addContextAction(m_pCopyAction);
	addContextAction(m_pCopyCellAction);
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

void DialogComparatorList::setTypeID(SignalIDType typeID)
{
	// clear all items of menu
	//
	for(int t = 0; t < SignalIDTypeCount; t++)
	{
		if (m_pTypeIDActionList[t] == nullptr)
		{
			continue;
		}

		m_pTypeIDActionList[t]->setChecked((bool) (t == typeID));
	}

	//
	//
	if (ERR_SIGNAL_ID_TYPE(typeID) == true)
	{
		return;
	}

	m_typeID = typeID;

	m_comparatorTable.setTypeID(m_typeID);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::showTypeID(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for (int typeID = 0; typeID < SignalIDTypeCount; typeID++)
	{
		if (m_pTypeIDActionList[typeID] != action)
		{
			continue;
		}

		setTypeID(static_cast<SignalIDType>(typeID));
	}
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
