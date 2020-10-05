#include "MeasureView.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QClipboard>

#include "Database.h"
#include "Options.h"

#include "MainWindow.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureTable::MeasureTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureTable::~MeasureTable()
{
	QMutexLocker l(&m_measureMutex);

	m_measureList.clear();
	m_measureCount = 0;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureTable::columnIsVisible(int column)
{
	if (column < 0 || column >= m_header.count())
	{
		return false;
	}

	MeasureViewColumn* pColumn = m_header.column(column);
	if (pColumn == nullptr)
	{
		return false;
	}

	if (pColumn->title().isEmpty() == true)
	{
		return false;
	}

	return pColumn->enableVisible();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureTable::columnCount(const QModelIndex&) const
{
	return m_header.count();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureTable::rowCount(const QModelIndex&) const
{
	return m_measureCount;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant MeasureTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		MeasureViewColumn* column = m_header.column(section);
		if (column != nullptr)
		{
			result = column->title();
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant MeasureTable::data(const QModelIndex &index, int role) const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return QVariant();
	}

	if (index.isValid() == false)
	{
		return QVariant();
	}

	int rowIndex = index.row();
	if (rowIndex < 0 || rowIndex >= m_measureCount)
	{
		return QVariant();
	}

	Measurement* pMeasurement = m_measureList[rowIndex];
	if (pMeasurement == nullptr)
	{
		return QVariant();
	}

	int columnIndex = index.column();
	if (columnIndex < 0 || columnIndex > m_header.count())
	{
		return QVariant();
	}

	MeasureViewColumn* pColumn = m_header.column(columnIndex);
	if (pColumn == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return pColumn->alignment();
	}

	if (role == Qt::FontRole)
	{
		switch(m_measureType)
		{
			case MEASURE_TYPE_LINEARITY:

				if (columnIndex == MVC_CMN_L_APP_ID || columnIndex == MVC_CMN_L_ERROR_RESULT)
				{
					return theOptions.measureView().fontBold();
				}

				break;

			case MEASURE_TYPE_COMPARATOR:

				if (columnIndex == MVC_CMN_C_APP_ID || columnIndex == MVC_CMN_C_ERROR_RESULT)
				{
					return theOptions.measureView().fontBold();
				}

				break;
		}

		return theOptions.measureView().font();
	}

	if (role == Qt::ForegroundRole)
	{
		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		return backgroundColor(rowIndex, columnIndex, pMeasurement);
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(rowIndex, columnIndex, pMeasurement);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QColor MeasureTable::backgroundColor(int row, int column, Measurement* pMeasurement) const
{
	if (row < 0 || row >= m_measureCount)
	{
		return Qt::white;
	}

	if (column < 0 || column > m_header.count())
	{
		return Qt::white;
	}

	if (pMeasurement == nullptr)
	{
		return Qt::white;
	}

	QColor result = Qt::white;

	switch(m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:
			{
				if (column != MVC_CMN_L_ERROR_RESULT)
				{
					break;
				}

				LinearityMeasurement* pLinearityMeasurement = static_cast<LinearityMeasurement*> (pMeasurement);
				if (pLinearityMeasurement == nullptr)
				{
					break;
				}

				if (pLinearityMeasurement->isSignalValid() == false)
				{
					result = theOptions.measureView().colorErrorLimit();
					break;
				}

				if (pLinearityMeasurement->errorResult() != MEASURE_ERROR_RESULT_OK)
				{
					result = theOptions.measureView().colorErrorLimit();
					break;
				}

				result = theOptions.measureView().colorNotError();
			}
			break;

		case MEASURE_TYPE_COMPARATOR:
			{
				if (column != MVC_CMN_C_ERROR_RESULT)
				{
					break;
				}

				ComparatorMeasurement* pComparatorMeasurement = static_cast<ComparatorMeasurement*> (pMeasurement);
				if (pComparatorMeasurement == nullptr)
				{
					break;
				}

				if (pComparatorMeasurement->isSignalValid() == false)
				{
					result = theOptions.measureView().colorErrorLimit();
					break;
				}

				if (pComparatorMeasurement->errorResult() != MEASURE_ERROR_RESULT_OK)
				{
					result = theOptions.measureView().colorErrorLimit();
					break;
				}

				result = theOptions.measureView().colorNotError();
			}
			break;

		default:
			assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::text(int row, int column, Measurement* pMeasurement) const
{
	if (row < 0 || row >= m_measureCount)
	{
		return QString();
	}

	if (column < 0 || column > m_header.count())
	{
		return QString();
	}

	if (pMeasurement == nullptr)
	{
		return QString();
	}

	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return QString();
	}

	QString result;

	switch(m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:			result = textLinearity(row, column, pMeasurement);	break;
		case MEASURE_TYPE_COMPARATOR:			result = textComparator(row, column, pMeasurement);	break;
		default:								result.clear();
	}

	return result;

}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::textLinearity(int row, int column, Measurement* pMeasurement) const
{
	if (row < 0 || row >= m_measureCount)
	{
		return QString();
	}

	if (column < 0 || column > m_header.count())
	{
		return QString();
	}

	if (pMeasurement == nullptr)
	{
		return QString();
	}

	MeasureViewColumn* pColumn = m_header.column(column);
	if (pColumn == nullptr)
	{
		return QString();
	}

	LinearityMeasurement* m = static_cast<LinearityMeasurement*> (pMeasurement);
	if (m == nullptr)
	{
		return QString();
	}

	int detailLimitType = MEASURE_LIMIT_TYPE_ELECTRIC;

	if (theOptions.linearity().viewType() == LO_VIEW_TYPE_DETAIL_ENGINEERING)
	{
		detailLimitType = MEASURE_LIMIT_TYPE_ENGINEER;
	}

	QString result;

	switch(column)
	{
		case MVC_CMN_L_INDEX:					result = QString::number(m->measureID()); break;

		case MVC_CMN_L_MODULE_SN:				result = m->location().moduleSerialNoStr(); break;
		case MVC_CMN_L_APP_ID:					result = m->appSignalID(); break;
		case MVC_CMN_L_CUSTOM_ID:				result = m->customAppSignalID(); break;
		case MVC_CMN_L_EQUIPMENT_ID:			result = m->equipmentID(); break;
		case MVC_CMN_L_NAME:					result = m->caption(); break;

		case MVC_CMN_L_RACK:					result = m->location().rack().caption(); break;
		case MVC_CMN_L_CHASSIS:					result = m->location().chassisStr(); break;
		case MVC_CMN_L_MODULE:					result = m->location().moduleStr(); break;
		case MVC_CMN_L_PLACE:					result = m->location().placeStr(); break;

		case MVC_CMN_L_EL_NOMINAL:				result = m->nominalStr(MEASURE_LIMIT_TYPE_ELECTRIC); break;
	    case MVC_CMN_L_EN_NOMINAL:				result = m->nominalStr(MEASURE_LIMIT_TYPE_ENGINEER); break;

		case MVC_CMN_L_PERCENT:					result = QString::number(m->percent(), 'f', 2); break;

		case MVC_CMN_L_EL_MEASURE:				result = m->measureStr(MEASURE_LIMIT_TYPE_ELECTRIC); break;
	    case MVC_CMN_L_EN_MEASURE:				result = m->measureStr(MEASURE_LIMIT_TYPE_ENGINEER); break;

		case MVC_CMN_L_EL_RANGE:				result = m->limitStr(MEASURE_LIMIT_TYPE_ELECTRIC); break;
	    case MVC_CMN_L_EN_RANGE:				result = m->limitStr(MEASURE_LIMIT_TYPE_ENGINEER); break;

		case MVC_CMN_L_VALUE_COUNT:				result = QString::number(m->measureCount()); break;
		case MVC_CMN_L_VALUE_0:					result = m->measureItemStr(detailLimitType, 0); break;
		case MVC_CMN_L_VALUE_1:					result = m->measureItemStr(detailLimitType, 1); break;
		case MVC_CMN_L_VALUE_2:					result = m->measureItemStr(detailLimitType, 2); break;
		case MVC_CMN_L_VALUE_3:					result = m->measureItemStr(detailLimitType, 3); break;
		case MVC_CMN_L_VALUE_4:					result = m->measureItemStr(detailLimitType, 4); break;
		case MVC_CMN_L_VALUE_5:					result = m->measureItemStr(detailLimitType, 5); break;
		case MVC_CMN_L_VALUE_6:					result = m->measureItemStr(detailLimitType, 6); break;
		case MVC_CMN_L_VALUE_7:					result = m->measureItemStr(detailLimitType, 7); break;
		case MVC_CMN_L_VALUE_8:					result = m->measureItemStr(detailLimitType, 8); break;
		case MVC_CMN_L_VALUE_9:					result = m->measureItemStr(detailLimitType, 9); break;
		case MVC_CMN_L_VALUE_10:				result = m->measureItemStr(detailLimitType, 10); break;
		case MVC_CMN_L_VALUE_11:				result = m->measureItemStr(detailLimitType, 11); break;
		case MVC_CMN_L_VALUE_12:				result = m->measureItemStr(detailLimitType, 12); break;
		case MVC_CMN_L_VALUE_13:				result = m->measureItemStr(detailLimitType, 13); break;
		case MVC_CMN_L_VALUE_14:				result = m->measureItemStr(detailLimitType, 14); break;
		case MVC_CMN_L_VALUE_15:				result = m->measureItemStr(detailLimitType, 15); break;
		case MVC_CMN_L_VALUE_16:				result = m->measureItemStr(detailLimitType, 16); break;
		case MVC_CMN_L_VALUE_17:				result = m->measureItemStr(detailLimitType, 17); break;
		case MVC_CMN_L_VALUE_18:				result = m->measureItemStr(detailLimitType, 18); break;
		case MVC_CMN_L_VALUE_19:				result = m->measureItemStr(detailLimitType, 19); break;

		case MVC_CMN_L_SYSTEM_ERROR:			result = m->additionalParamStr(MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR); break;
		case MVC_CMN_L_SD:						result = m->additionalParamStr(MEASURE_ADDITIONAL_PARAM_SD); break;
		case MVC_CMN_L_BORDER:					result = tr("± ") + m->additionalParamStr(MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER); break;

		case MVC_CMN_L_ERROR:					result = m->errorStr(); break;
		case MVC_CMN_L_ERROR_LIMIT:				result = m->errorLimitStr(); break;
		case MVC_CMN_L_ERROR_RESULT:			result = m->errorResultStr(); break;

		case MVC_CMN_L_MEASUREMENT_TIME:		result = m->measureTimeStr(); break;

		default:								result.clear(); break;
	}

	if (row > 0)
	{
		Measurement* prev_m = m_measureList[row - 1];
		if (prev_m != nullptr)
		{
			if (prev_m->signalHash() == m->signalHash())
			{
				if (pColumn->enableDuplicate() == false)
				{
					result.clear();
				}
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::textComparator(int row, int column, Measurement* pMeasurement) const
{
	if (row < 0 || row >= m_measureCount)
	{
		return QString();
	}

	if (column < 0 || column > m_header.count())
	{
		return QString();
	}

	if (pMeasurement == nullptr)
	{
		return QString();
	}

	MeasureViewColumn* pColumn = m_header.column(column);
	if (pColumn == nullptr)
	{
		return QString();
	}

	ComparatorMeasurement* m = static_cast<ComparatorMeasurement*> (pMeasurement);
	if (m == nullptr)
	{
		return QString();
	}

	QString result;

	switch(column)
	{
		case MVC_CMN_C_INDEX:					result = QString::number(m->measureID()); break;

		case MVC_CMN_C_MODULE_SN:				result = m->location().moduleSerialNoStr(); break;
		case MVC_CMN_C_APP_ID:					result = m->appSignalID(); break;
		case MVC_CMN_C_CUSTOM_ID:				result = m->customAppSignalID(); break;
		case MVC_CMN_C_EQUIPMENT_ID:			result = m->equipmentID(); break;
		case MVC_CMN_C_NAME:					result = m->caption(); break;

		case MVC_CMN_C_RACK:					result = m->location().rack().caption(); break;
		case MVC_CMN_C_CHASSIS:					result = m->location().chassisStr(); break;
		case MVC_CMN_C_MODULE:					result = m->location().moduleStr(); break;
		case MVC_CMN_C_PLACE:					result = m->location().placeStr(); break;

		case MVC_CMN_C_SP_TYPE:					result = m->spTypeStr(); break;
		case MVC_CMN_C_CMP_TYPE:				result = m->cmpTypeStr(); break;

		case MVC_CMN_C_EL_NOMINAL:				result = m->nominalStr(MEASURE_LIMIT_TYPE_ELECTRIC); break;
	    case MVC_CMN_C_EN_NOMINAL:				result = m->nominalStr(MEASURE_LIMIT_TYPE_ENGINEER); break;

		case MVC_CMN_C_EL_MEASURE:				result = m->measureStr(MEASURE_LIMIT_TYPE_ELECTRIC); break;
	    case MVC_CMN_C_EN_MEASURE:				result = m->measureStr(MEASURE_LIMIT_TYPE_ENGINEER); break;

		case MVC_CMN_C_EL_RANGE:				result = m->limitStr(MEASURE_LIMIT_TYPE_ELECTRIC); break;
	    case MVC_CMN_C_EN_RANGE:				result = m->limitStr(MEASURE_LIMIT_TYPE_ENGINEER); break;

		case MVC_CMN_C_CMP_ID:					result = m->compareAppSignalID(); break;
		case MVC_CMN_C_OUT_ID:					result = m->outputAppSignalID(); break;

		case MVC_CMN_C_ERROR:					result = m->errorStr(); break;
		case MVC_CMN_C_ERROR_LIMIT:				result = m->errorLimitStr(); break;
		case MVC_CMN_C_ERROR_RESULT:			result = m->errorResultStr(); break;

		case MVC_CMN_C_MEASUREMENT_TIME:		result = m->measureTimeStr(); break;

		default:								result.clear(); break;
	}

	if (row > 0)
	{
		Measurement* prev_m = m_measureList[row - 1];
		if (prev_m != nullptr)
		{
			if (prev_m->signalHash() == m->signalHash())
			{
				if (pColumn->enableDuplicate() == false)
				{
					result.clear();
				}
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureTable::append(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return false;
	}

	if (pMeasurement->measureType() != m_measureType)
	{
		return false;
	}

	// append into MeasureTable
	//
	int indexTable = m_measureCount;

	beginInsertRows(QModelIndex(), indexTable, indexTable);

		m_measureMutex.lock();

			m_measureList.append(pMeasurement);
			m_measureCount = m_measureList.count();

		m_measureMutex.unlock();

	endInsertRows();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

Measurement* MeasureTable::at(int index)
{
	QMutexLocker l(&m_measureMutex);

	if (index < 0 || index >= m_measureCount)
	{
		return nullptr;
	}

	return m_measureList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureTable::remove(const QVector<int>& removeIndexList)
{
	// remove from MeasureTable
	//
	int count = removeIndexList.count();
	for(int index = count-1; index >= 0; index--)
	{
		int removeIndex = removeIndexList.at(index);

		Measurement* pMeasurement = at(removeIndex);
		if (pMeasurement == nullptr)
		{
			continue;
		}

		if (pMeasurement->measureType() != m_measureType)
		{
			continue;
		}

		beginRemoveRows(QModelIndex(), removeIndex, removeIndex);

			m_measureMutex.lock();

				m_measureList.remove(removeIndex);
				m_measureCount = m_measureList.count();

			m_measureMutex.unlock();

		endRemoveRows();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureTable::set(const QVector<Measurement*>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_measureMutex.lock();

			m_measureList = list_add;
			m_measureCount = m_measureList.count();

		m_measureMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureTable::clear()
{
	int count = m_measureCount;
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_measureMutex.lock();

			m_measureList.clear();
			m_measureCount = 0;

		m_measureMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureView::MeasureView(int measureType, QWidget *parent) :
	QTableView(parent),
	m_measureType(measureType)
{
	m_table.setMeasureType(measureType);
	m_table.header().init(measureType);
	setModel(&m_table);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setWordWrap(false);

	createContextMenu();

	updateColumn();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureView::~MeasureView()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::createContextMenu()
{
	// create header context menu
	//
	m_headerContextMenu = new QMenu(this);

	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MeasureView::onHeaderContextMenu);
	connect(horizontalHeader(), &QHeaderView::sectionResized, this, &MeasureView::onColumnResized);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::updateColumn()
{
	m_headerContextMenu->clear();

	m_table.header().updateColumnState();

	int count = m_table.header().count();
	for (int index = 0; index < count; index++)
	{
		MeasureViewColumn* pColumn = m_table.header().column(index);
		if (pColumn == nullptr)
		{
			continue;
		}

		setColumnWidth(index, pColumn->width());
		setColumnHidden(index, pColumn->enableVisible() == false);

		if (pColumn->enableVisible() == true)
		{
			QAction* pAction = m_headerContextMenu->addAction(pColumn->title());
			if (pAction != nullptr)
			{
				pAction->setCheckable(true);
				pAction->setChecked(true);
				pAction->setData(index);
			}
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &MeasureView::onHeaderContextAction);

	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	verticalHeader()->setDefaultSectionSize(cellSize.height());
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::loadMeasureList()
{
	m_table.clear();

	QVector<Measurement*> measureList;

	int measureCount = theMeasureBase.count();
	for (int i = 0; i < measureCount; i++)
	{
		Measurement* pMeasurement = theMeasureBase.measurement(i);
		if (pMeasurement == nullptr)
		{
			continue;
		}

		if (pMeasurement->measureType() != m_measureType)
		{
			continue;
		}

		measureList.append(pMeasurement);
	}

	m_table.set(measureList);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onHeaderContextAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	int index = action->data().toInt();
	if (index < 0 || index >= m_table.header().count())
	{
		return;
	}

	MeasureViewColumn* pColumn = m_table.header().column(index);
	if (pColumn == nullptr)
	{
		return;
	}

	setColumnHidden(index, action->isChecked() == false);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onColumnResized(int index, int, int width)
{
	if (index < 0 || index >= m_table.header().count())
	{
		return;
	}

	MeasureViewColumn* pColumn = m_table.header().column(index);
	if (pColumn == nullptr)
	{
		return;
	}

	if (pColumn->enableVisible() == false || width == 0)
	{
		return;
	}

	pColumn->setWidth(width);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::appendMeasure(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return;
	}

	if (pMeasurement->measureType() != m_measureType)
	{
		return;
	}

	// append into database
	//
	if (thePtrDB == nullptr)
	{
		return;
	}

	if (thePtrDB->appendMeasure(pMeasurement) == false)
	{
		QMessageBox::critical(this, tr("Append measurements"), tr("Error append measurements to database"));
		return;
	}

	// append into MeasureBase
	//
	if (theMeasureBase.append(pMeasurement) == -1)
	{
		return;
	}

	// append into MeasureTable
	//
	if (m_table.append(pMeasurement) == false)
	{
		return;
	}

	setCurrentIndex(model()->index(m_table.count() - 1, MVC_CMN_L_APP_ID));
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::removeMeasure()
{
	if (thePtrDB == nullptr)
	{
		return;
	}

	int measureCount = m_table.count();
	if (measureCount == 0)
	{
		return;
	}

	QVector<int> keyList;
	QVector<int> removeIndexList;

	for(int index = 0; index < measureCount; index++)
	{
		if (selectionModel()->isRowSelected(index, QModelIndex()) == false)
		{
			continue;
		}

		Measurement* pMeasuremet = m_table.at(index);
		if (pMeasuremet == nullptr)
		{
			continue;
		}

		if (pMeasuremet->measureType() != m_measureType)
		{
			continue;
		}

		keyList.append(pMeasuremet->measureID());

		removeIndexList.append(index);
	}

	if (removeIndexList.count() == 0)
	{
		return;
	}

	if (QMessageBox::question(this, windowTitle(), tr("Do you want delete %1 measurement(s)?").arg(removeIndexList.count())) == QMessageBox::No)
	{
		return;
	}

	// remove from database
	//
	if (thePtrDB->removeMeasure(m_measureType, keyList) == false)
	{
		QMessageBox::critical(this, tr("Delete measurements"), tr("Error remove measurements from database"));
		return;
	}

	// remove from MeasureTable
	//
	m_table.remove(removeIndexList);

	// remove from MesaureBase
	//
	theMeasureBase.remove(keyList);

	QMessageBox::information(this, tr("Delete"), tr("Deleted %1 measurement(s)").arg(removeIndexList.count()));
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::copy()
{
	QString textClipboard;

	int rowCount = model()->rowCount();
	int columnCount = model()->columnCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (isColumnHidden(column) == true)
			{
				continue;
			}

			textClipboard.append(model()->data(model()->index(row, column)).toString() + "\t");
		}

		textClipboard.replace(textClipboard.length() - 1, 1, "\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
