#include "DialogComparatorList.h"

#include "SignalBase.h"
#include "DialogObjectProperties.h"
#include "UnitsConverter.h"

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
			case COMPARATOR_LIST_COLUMN_RACK:			result = Qt::AlignLeft;		break;
			case COMPARATOR_LIST_COLUMN_INPUT:			result = Qt::AlignLeft;		break;
			case COMPARATOR_LIST_COLUMN_CMP_NO:			result = Qt::AlignCenter;	break;
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
			if (comparatorEx->enableMeasure() == false)
			{
				return QColor(Qt::lightGray);
			}
			else
			{
				if (column == COMPARATOR_LIST_COLUMN_HYSTERESIS)
				{
					if (comparatorEx->deviation() != Metrology::ComparatorEx::DeviationType::Unused)
					{
						return QColor(Qt::blue);
					}
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

		if (pInSignal->param().isOutput() == true)
		{
			if (column == COMPARATOR_LIST_COLUMN_TYPE)
			{
				return QColor(0xFF, 0xA0, 0xA0);
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

	//

	QString result;

	switch (column)
	{
		case COMPARATOR_LIST_COLUMN_RACK:				result = param.location().rackCaption();					break;
		case COMPARATOR_LIST_COLUMN_INPUT:				result = comparatorEx->inputSignalID(m_idType);				break;
		case COMPARATOR_LIST_COLUMN_CMP_NO:				result = comparatorEx->indexStr();							break;
		case COMPARATOR_LIST_COLUMN_SETPOINT:			result = comparatorEx->compareDefaultValueStr(m_idType);	break;
		case COMPARATOR_LIST_COLUMN_HYSTERESIS:			result = comparatorEx->hysteresisDefaultValueStr(m_idType);	break;
		case COMPARATOR_LIST_COLUMN_TYPE:				result = param.signalTypeStr();								break;
		case COMPARATOR_LIST_COLUMN_EL_RANGE:			result = param.electricRangeStr();							break;
		case COMPARATOR_LIST_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();						break;
		case COMPARATOR_LIST_COLUMN_EN_RANGE:			result = param.engineeringRangeStr();						break;
		case COMPARATOR_LIST_COLUMN_OUTPUT:				result = comparatorEx->outputSignalID(m_idType);			break;
		case COMPARATOR_LIST_COLUMN_SCHEMA:				result = comparatorEx->schemaID();							break;

		default:
			assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalIDType DialogComparatorList::m_idType = Metrology::SignalIDType::CustomID;

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

	m_pEnableMeasureAction = m_pEditMenu->addAction(tr("Enable measure"));
	m_pDisableMeasureAction = m_pEditMenu->addAction(tr("Disable measure"));
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pPropertyAction);

	for(int typeID = 0; typeID < Metrology::SignalIDTypeCount; typeID++)
	{
		m_pTypeIDActionList[typeID] = m_pViewTypeIDMenu->addAction(Metrology::SignalIDTypeCaption(typeID));
		m_pTypeIDActionList[typeID]->setCheckable(true);
	}

	m_pViewMenu->addMenu(m_pViewTypeIDMenu);

	connect(m_pEnableMeasureAction, &QAction::triggered, this, &DialogComparatorList::onEnableMeasure);
	connect(m_pDisableMeasureAction, &QAction::triggered, this, &DialogComparatorList::onDisableMeasure);
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
	setTypeID(m_idType);
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

	hideColumn(COMPARATOR_LIST_COLUMN_RACK, true);
	hideColumn(COMPARATOR_LIST_COLUMN_CMP_NO, true);
	hideColumn(COMPARATOR_LIST_COLUMN_EL_SENSOR, true);
	hideColumn(COMPARATOR_LIST_COLUMN_SCHEMA, true);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::updateList()
{
	DialogComparatorList::updateVisibleColunm();

	m_comparatorTable.clear();

	std::vector<Metrology::Signal*> signalList;
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

		if (param.hasComparators() == false)
		{
			continue;
		}

		signalList.push_back(pSignal);
	}

	std::sort(signalList.begin(), signalList.end(),
				[](Metrology::Signal* s1, Metrology::Signal* s2)
				{ return s1->param().location().positionID() < s2->param().location().positionID(); });

	for(auto pSignal : signalList)
	{
		if (pSignal == nullptr || pSignal->param().isValid() == false)
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

			comparatorList.push_back(comparatorEx);
		}
	}

	m_comparatorTable.set(comparatorList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::setTypeID(Metrology::SignalIDType idType)
{
	// clear all items of menu
	//
	for(int t = 0; t < Metrology::SignalIDTypeCount; t++)
	{
		if (m_pTypeIDActionList[t] == nullptr)
		{
			continue;
		}

		m_pTypeIDActionList[t]->setChecked((bool) (t == idType));
	}

	//
	//
	if (ERR_SIGNAL_ID_TYPE(idType) == true)
	{
		return;
	}

	m_idType = idType;

	m_comparatorTable.setTypeID(m_idType);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::showTypeID(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for (int typeID = 0; typeID < Metrology::SignalIDTypeCount; typeID++)
	{
		if (m_pTypeIDActionList[typeID] != action)
		{
			continue;
		}

		setTypeID(static_cast<Metrology::SignalIDType>(typeID));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::onEnableMeasure()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	const QModelIndexList selectedList = pView->selectionModel()->selectedRows();
	for(auto selectedIndex : selectedList)
	{
		int indexRow = selectedIndex.row();
		if (indexRow < 0 || indexRow >= m_comparatorTable.count())
		{
			return;
		}

		std::shared_ptr<Metrology::ComparatorEx> comparatorEx = m_comparatorTable.at(indexRow);
		if (comparatorEx == nullptr)
		{
			return;
		}

		comparatorEx->setEnableMeasure(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorList::onDisableMeasure()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	const QModelIndexList selectedList = pView->selectionModel()->selectedRows();
	for(auto selectedIndex : selectedList)
	{
		int indexRow = selectedIndex.row();
		if (indexRow < 0 || indexRow >= m_comparatorTable.count())
		{
			return;
		}

		std::shared_ptr<Metrology::ComparatorEx> comparatorEx = m_comparatorTable.at(indexRow);
		if (comparatorEx == nullptr)
		{
			return;
		}

		comparatorEx->setEnableMeasure(false);
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
