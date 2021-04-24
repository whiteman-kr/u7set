#include "MeasureView.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>

#include "Database.h"
#include "ProcessData.h"
#include "Options.h"
#include "../lib/CUtils.h"

namespace Measure
{
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	Table::Table(QObject*)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	Table::~Table()
	{
		QMutexLocker l(&m_measureMutex);

		m_measureList.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Table::columnIsVisible(int column)
	{
		if (column < 0 || column >= m_header.count())
		{
			return false;
		}

		HeaderColumn* pColumn = m_header.column(column);
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

	int Table::columnCount(const QModelIndex&) const
	{
		return m_header.count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	int Table::rowCount(const QModelIndex&) const
	{
		return TO_INT(m_measureCount);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QVariant Table::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role != Qt::DisplayRole)
		{
			return QVariant();
		}

		QVariant result = QVariant();

		if (orientation == Qt::Horizontal)
		{
			HeaderColumn* column = m_header.column(section);
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

	QVariant Table::data(const QModelIndex &index, int role) const
	{
		if (ERR_MEASURE_TYPE(m_measureType) == true)
		{
			return QVariant();
		}

		if (index.isValid() == false)
		{
			return QVariant();
		}

		int rowIndex = index.row();
		if (rowIndex < 0 || rowIndex >= TO_INT(m_measureCount))
		{
			return QVariant();
		}

		Measure::Item* pMeasurement = at(rowIndex);
		if (pMeasurement == nullptr)
		{
			return QVariant();
		}

		int columnIndex = index.column();
		if (columnIndex < 0 || columnIndex > m_header.count())
		{
			return QVariant();
		}

		HeaderColumn* pColumn = m_header.column(columnIndex);
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
				case Measure::Type::Linearity:

					if (columnIndex == MVC_CMN_L_APP_ID || columnIndex == MVC_CMN_L_ERROR_RESULT)
					{
						return theOptions.measureView().fontBold();
					}

					break;

				case Measure::Type::Comparators:

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
			if (pMeasurement->foundInStatistics() == false)
			{
				return QColor(Qt::lightGray);
			}

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

	QColor Table::backgroundColor(int row, int column, Measure::Item* pMeasurement) const
	{
		if (row < 0 || row >= TO_INT(m_measureCount))
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
			case Measure::Type::Linearity:
				{
					if (column != MVC_CMN_L_ERROR_RESULT)
					{
						break;
					}

					Measure::LinearityItem* pLinearityMeasurement = static_cast<Measure::LinearityItem*> (pMeasurement);
					if (pLinearityMeasurement == nullptr)
					{
						break;
					}

					if (theOptions.measureView().showNoValid() == false)
					{
						if (pLinearityMeasurement->isSignalValid() == false)
						{
							result = theOptions.measureView().colorErrorLimit();
							break;
						}
					}

					if (pLinearityMeasurement->errorResult() != Measure::ErrorResult::Ok)
					{
						result = theOptions.measureView().colorErrorLimit();
						break;
					}

					result = theOptions.measureView().colorNotError();
				}
				break;

			case Measure::Type::Comparators:
				{
					if (column != MVC_CMN_C_ERROR_RESULT)
					{
						break;
					}

					Measure::ComparatorItem* pComparatorMeasurement = static_cast<Measure::ComparatorItem*> (pMeasurement);
					if (pComparatorMeasurement == nullptr)
					{
						break;
					}

					if (theOptions.measureView().showNoValid() == false)
					{
						if (pComparatorMeasurement->isSignalValid() == false)
						{
							result = theOptions.measureView().colorErrorLimit();
							break;
						}
					}

					if (pComparatorMeasurement->errorResult() != Measure::ErrorResult::Ok)
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

	QString Table::text(int row, int column, Measure::Item* pMeasurement) const
	{
		if (row < 0 || row >= TO_INT(m_measureCount))
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

		if (ERR_MEASURE_TYPE(m_measureType) == true)
		{
			return QString();
		}

		QString result;

		switch(m_measureType)
		{
			case Measure::Type::Linearity:		result = textLinearity(row, column, pMeasurement);	break;
			case Measure::Type::Comparators:	result = textComparator(row, column, pMeasurement);	break;
			default:							result.clear();
		}

		return result;

	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Table::textLinearity(int row, int column, Measure::Item* pMeasurement) const
	{
		if (row < 0 || row >= TO_INT(m_measureCount))
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

		HeaderColumn* pColumn = m_header.column(column);
		if (pColumn == nullptr)
		{
			return QString();
		}

		Measure::LinearityItem* m = static_cast<Measure::LinearityItem*> (pMeasurement);
		if (m == nullptr)
		{
			return QString();
		}

		Measure::LimitType limitType = Measure::LimitType::NoLimitType;

		switch (theOptions.linearity().viewType())
		{
			case LinearityViewType::Simple:
			case LinearityViewType::Extended:			limitType = static_cast<Measure::LimitType>(theOptions.linearity().limitType());	break;
			case LinearityViewType::DetailElectric:		limitType = Measure::LimitType::Electric;											break;
			case LinearityViewType::DetailEngineering:	limitType = Measure::LimitType::Engineering;										break;
			default:
				assert(0);
				return QString();
		}

		if (limitType == Measure::LimitType::NoLimitType)
		{
			return QString();
		}

		QString result;

		switch(column)
		{
			case MVC_CMN_L_INDEX:					result = QString::number(m->measureID()); break;

			case MVC_CMN_L_MODULE_SN:				result = m->location().moduleSerialNoStr(); break;
			case MVC_CMN_L_CONNECT_APP_ID:			result = m->connectionAppSignalID(); break;
			case MVC_CMN_L_CONNECT_TYPE:			result = m->connectionTypeStr(); break;
			case MVC_CMN_L_APP_ID:					result = m->appSignalID(); break;
			case MVC_CMN_L_CUSTOM_ID:				result = m->customAppSignalID(); break;
			case MVC_CMN_L_EQUIPMENT_ID:			result = m->equipmentID(); break;
			case MVC_CMN_L_NAME:					result = m->caption(); break;

			case MVC_CMN_L_RACK:					result = m->location().rack().caption(); break;
			case MVC_CMN_L_CHASSIS:					result = m->location().chassisStr(); break;
			case MVC_CMN_L_MODULE:					result = m->location().moduleStr(); break;
			case MVC_CMN_L_PLACE:					result = m->location().placeStr(); break;

			case MVC_CMN_L_EL_NOMINAL:				result = m->nominalStr(Measure::LimitType::Electric); break;
			case MVC_CMN_L_EN_NOMINAL:				result = m->nominalStr(Measure::LimitType::Engineering); break;

			case MVC_CMN_L_PERCENT:					result = QString::number(m->percent(), 'f', 2); break;

			case MVC_CMN_L_EL_MEASURE:				result = m->measureStr(Measure::LimitType::Electric); break;
			case MVC_CMN_L_EN_MEASURE:				result = m->measureStr(Measure::LimitType::Engineering); break;

			case MVC_CMN_L_EL_RANGE:				result = m->limitStr(Measure::LimitType::Electric); break;
			case MVC_CMN_L_EN_RANGE:				result = m->limitStr(Measure::LimitType::Engineering); break;

			case MVC_CMN_L_VALUE_COUNT:				result = QString::number(m->measureCount()); break;
			case MVC_CMN_L_VALUE_0:					result = m->measureItemStr(limitType, 0); break;
			case MVC_CMN_L_VALUE_1:					result = m->measureItemStr(limitType, 1); break;
			case MVC_CMN_L_VALUE_2:					result = m->measureItemStr(limitType, 2); break;
			case MVC_CMN_L_VALUE_3:					result = m->measureItemStr(limitType, 3); break;
			case MVC_CMN_L_VALUE_4:					result = m->measureItemStr(limitType, 4); break;
			case MVC_CMN_L_VALUE_5:					result = m->measureItemStr(limitType, 5); break;
			case MVC_CMN_L_VALUE_6:					result = m->measureItemStr(limitType, 6); break;
			case MVC_CMN_L_VALUE_7:					result = m->measureItemStr(limitType, 7); break;
			case MVC_CMN_L_VALUE_8:					result = m->measureItemStr(limitType, 8); break;
			case MVC_CMN_L_VALUE_9:					result = m->measureItemStr(limitType, 9); break;
			case MVC_CMN_L_VALUE_10:				result = m->measureItemStr(limitType, 10); break;
			case MVC_CMN_L_VALUE_11:				result = m->measureItemStr(limitType, 11); break;
			case MVC_CMN_L_VALUE_12:				result = m->measureItemStr(limitType, 12); break;
			case MVC_CMN_L_VALUE_13:				result = m->measureItemStr(limitType, 13); break;
			case MVC_CMN_L_VALUE_14:				result = m->measureItemStr(limitType, 14); break;
			case MVC_CMN_L_VALUE_15:				result = m->measureItemStr(limitType, 15); break;
			case MVC_CMN_L_VALUE_16:				result = m->measureItemStr(limitType, 16); break;
			case MVC_CMN_L_VALUE_17:				result = m->measureItemStr(limitType, 17); break;
			case MVC_CMN_L_VALUE_18:				result = m->measureItemStr(limitType, 18); break;
			case MVC_CMN_L_VALUE_19:				result = m->measureItemStr(limitType, 19); break;

			case MVC_CMN_L_SYSTEM_DEVIATION:		result = m->additionalParamStr(limitType, Measure::AdditionalParam::SystemDeviation); break;
			case MVC_CMN_L_SD:						result = m->additionalParamStr(limitType, Measure::AdditionalParam::StandardDeviation); break;
			case MVC_CMN_L_BORDER:					result = m->additionalParamStr(limitType, Measure::AdditionalParam::LowHighBorder); break;
			case MVC_CMN_L_UNCERTAINTY:				result = m->additionalParamStr(limitType, Measure::AdditionalParam::Uncertainty); break;

			case MVC_CMN_L_ERROR:					result = m->errorStr(); break;
			case MVC_CMN_L_ERROR_LIMIT:				result = m->errorLimitStr(); break;
			case MVC_CMN_L_ERROR_RESULT:			result = m->errorResultStr(); break;

			case MVC_CMN_L_MEASUREMENT_TIME:		result = m->measureTimeStr(); break;
			case MVC_CMN_L_CALIBRATOR:				result = m->calibrator(); break;

			default:								result.clear(); break;
		}

		if (row > 0)
		{
			Measure::Item* prev_m = m_measureList[static_cast<quint64>(row - 1)];
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

	QString Table::textComparator(int row, int column, Measure::Item* pMeasurement) const
	{
		if (row < 0 || row >= TO_INT(m_measureCount))
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

		HeaderColumn* pColumn = m_header.column(column);
		if (pColumn == nullptr)
		{
			return QString();
		}

		Measure::ComparatorItem* m = static_cast<Measure::ComparatorItem*> (pMeasurement);
		if (m == nullptr)
		{
			return QString();
		}

		QString result;

		switch(column)
		{
			case MVC_CMN_C_INDEX:					result = QString::number(m->measureID()); break;

			case MVC_CMN_C_MODULE_SN:				result = m->location().moduleSerialNoStr(); break;
			case MVC_CMN_C_CONNECT_APP_ID:			result = m->connectionAppSignalID(); break;
			case MVC_CMN_C_CONNECT_TYPE:			result = m->connectionTypeStr(); break;
			case MVC_CMN_C_APP_ID:					result = m->appSignalID(); break;
			case MVC_CMN_C_CUSTOM_ID:				result = m->customAppSignalID(); break;
			case MVC_CMN_C_EQUIPMENT_ID:			result = m->equipmentID(); break;
			case MVC_CMN_C_NAME:					result = m->caption(); break;

			case MVC_CMN_C_RACK:					result = m->location().rack().caption(); break;
			case MVC_CMN_C_CHASSIS:					result = m->location().chassisStr(); break;
			case MVC_CMN_C_MODULE:					result = m->location().moduleStr(); break;
			case MVC_CMN_C_PLACE:					result = m->location().placeStr(); break;

			case MVC_CMN_C_SP_TYPE:					result = m->cmpValueTypeStr(); break;
			case MVC_CMN_C_CMP_TYPE:				result = m->cmpTypeStr(); break;

			case MVC_CMN_C_EL_NOMINAL:				result = m->nominalStr(Measure::LimitType::Electric); break;
			case MVC_CMN_C_EN_NOMINAL:				result = m->nominalStr(Measure::LimitType::Engineering); break;

			case MVC_CMN_C_EL_MEASURE:				result = m->measureStr(Measure::LimitType::Electric); break;
			case MVC_CMN_C_EN_MEASURE:				result = m->measureStr(Measure::LimitType::Engineering); break;

			case MVC_CMN_C_EL_RANGE:				result = m->limitStr(Measure::LimitType::Electric); break;
			case MVC_CMN_C_EN_RANGE:				result = m->limitStr(Measure::LimitType::Engineering); break;

			case MVC_CMN_C_CMP_ID:					result = m->compareAppSignalID(); break;
			case MVC_CMN_C_OUT_ID:					result = m->outputAppSignalID(); break;

			case MVC_CMN_C_ERROR:					result = m->errorStr(); break;
			case MVC_CMN_C_ERROR_LIMIT:				result = m->errorLimitStr(); break;
			case MVC_CMN_C_ERROR_RESULT:			result = m->errorResultStr(); break;

			case MVC_CMN_C_MEASUREMENT_TIME:		result = m->measureTimeStr(); break;
			case MVC_CMN_C_CALIBRATOR:				result = m->calibrator(); break;

			default:								result.clear(); break;
		}

		if (row > 0)
		{
			Measure::Item* prev_m = m_measureList[static_cast<quint64>(row - 1)];
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

	bool Table::append(Measure::Item* pMeasurement)
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
		int indexTable = TO_INT(m_measureCount);

		beginInsertRows(QModelIndex(), indexTable, indexTable);

			m_measureMutex.lock();

				m_measureList.push_back(pMeasurement);
				m_measureCount = m_measureList.size();

			m_measureMutex.unlock();

		endInsertRows();

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	Measure::Item* Table::at(int index) const
	{
		QMutexLocker l(&m_measureMutex);

		if (index < 0 || index >= TO_INT(m_measureCount))
		{
			return nullptr;
		}

		return m_measureList[static_cast<quint64>(index)];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Table::remove(const std::vector<int>& removeIndexList)
	{
		// remove from MeasureTable
		//
		int count = TO_INT(removeIndexList.size());
		for(int index = count-1; index >= 0; index--)
		{
			int removeIndex = removeIndexList.at(static_cast<quint64>(index));

			Measure::Item* pMeasurement = at(removeIndex);
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

					m_measureList.erase(m_measureList.begin() + removeIndex);
					m_measureCount = m_measureList.size();

				m_measureMutex.unlock();

			endRemoveRows();
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Table::set(const std::vector<Measure::Item*>& list_add)
	{
		quint64 count = list_add.size();
		if (count == 0)
		{
			return;
		}

		beginInsertRows(QModelIndex(), 0, TO_INT(count - 1));

			m_measureMutex.lock();

				m_measureList = list_add;
				m_measureCount = m_measureList.size();

			m_measureMutex.unlock();

		endInsertRows();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Table::clear()
	{
		quint64 count = m_measureCount;
		if (count == 0)
		{
			return;
		}

		beginRemoveRows(QModelIndex(), 0,TO_INT(count - 1));

			m_measureMutex.lock();

				m_measureList.clear();
				m_measureCount = m_measureList.size();

			m_measureMutex.unlock();

		endRemoveRows();
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	View::View(Measure::Type measureType, QWidget* parent) :
		QTableView(parent),
		m_measureType(measureType)
	{
		m_table.header().init(measureType);
		m_table.setMeasureType(measureType);
		setModel(&m_table);

		setSelectionBehavior(QAbstractItemView::SelectRows);
		setWordWrap(false);

		createContextMenu();

		updateColumn();
	}

	// -------------------------------------------------------------------------------------------------------------------

	View::~View()
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::createContextMenu()
	{
		// create header context menu
		//
		m_headerContextMenu = new QMenu(this);

		horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &View::onHeaderContextMenu);
		connect(horizontalHeader(), &QHeaderView::sectionResized, this, &View::onColumnResized);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::updateColumn()
	{
		m_headerContextMenu->clear();

		m_table.header().updateColumnState();

		int count = m_table.header().count();
		for (int index = 0; index < count; index++)
		{
			HeaderColumn* pColumn = m_table.header().column(index);
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

		connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),
				this, &View::onHeaderContextAction);

		QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
		verticalHeader()->setDefaultSectionSize(cellSize.height());
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::loadMeasurements(const Measure::Base& measureBase)
	{
		m_table.clear();

		std::vector<Measure::Item*> measureList;

		int measureCount = measureBase.count();
		for (int i = 0; i < measureCount; i++)
		{
			Measure::Item* pMeasurement = measureBase.measurement(i);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			if (pMeasurement->measureType() != m_measureType)
			{
				continue;
			}

			measureList.push_back(pMeasurement);
		}

		m_table.set(measureList);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onHeaderContextMenu(QPoint)
	{
		if (m_headerContextMenu == nullptr)
		{
			return;
		}

		m_headerContextMenu->exec(QCursor::pos());
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onHeaderContextAction(QAction* action)
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

		HeaderColumn* pColumn = m_table.header().column(index);
		if (pColumn == nullptr)
		{
			return;
		}

		setColumnHidden(index, action->isChecked() == false);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onColumnResized(int index, int, int width)
	{
		if (index < 0 || index >= m_table.header().count())
		{
			return;
		}

		HeaderColumn* pColumn = m_table.header().column(index);
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

	void View::appendMeasure(Measure::Item* pMeasurement)
	{
		if (pMeasurement == nullptr)
		{
			return;
		}

		if (pMeasurement->measureType() != m_measureType)
		{
			return;
		}

		// append into Database and MeasureBase from MainWindow
		//
		// append into MeasureTable
		//
		if (m_table.append(pMeasurement) == false)
		{
			return;
		}

		setCurrentIndex(model()->index(m_table.count() - 1, MVC_CMN_L_APP_ID));
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::removeMeasure()
	{
		int measureCount = m_table.count();
		if (measureCount == 0)
		{
			return;
		}

		std::vector<int> keyList;
		std::vector<int> removeIndexList;

		for(int index = 0; index < measureCount; index++)
		{
			if (selectionModel()->isRowSelected(index, QModelIndex()) == false)
			{
				continue;
			}

			Measure::Item* pMeasuremet = m_table.at(index);
			if (pMeasuremet == nullptr)
			{
				continue;
			}

			if (pMeasuremet->measureType() != m_measureType)
			{
				continue;
			}

			keyList.push_back(pMeasuremet->measureID());

			removeIndexList.push_back(index);
		}

		if (removeIndexList.size() == 0)
		{
			return;
		}

		if (QMessageBox::question(this,
								  windowTitle(),
								  tr("Do you want delete %1 measurement(s)?").
								  arg(removeIndexList.size())) == QMessageBox::No)
		{
			return;
		}

		// remove from MeasureTable
		//
		m_table.remove(removeIndexList);

		// remove from Database and MesaureBase
		//
		emit removeFromBase(m_measureType, keyList);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::copy()
	{
		CopyData copyData(this, false);
		copyData.exec();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::showGraph(int graphType)
	{
		if (graphType < 0 || graphType >= MVG_TYPE_COUNT)
		{
			return;
		}

		int measureCount = m_table.count();
		if (measureCount == 0)
		{
			return;
		}

		int index = currentIndex().row();
		if (index < 0 || index >= measureCount)
		{
			return;
		}

		Measure::Item* pMeasurement = m_table.at(index);
		if (pMeasurement == nullptr)
		{
			return;
		}

		Measure::LinearityItem* pLinearityMeasurement = dynamic_cast<Measure::LinearityItem*>(pMeasurement);
		if (pLinearityMeasurement == nullptr)
		{
			return;
		}

		// select limit type
		//
		Measure::LimitType limitType = Measure::LimitType::NoLimitType;

		switch (graphType)
		{
			case MVG_TYPE_LIN_EL:
			case MVG_TYPE_20VAL_EL:	limitType = Measure::LimitType::Electric;		break;
			case MVG_TYPE_LIN_EN:
			case MVG_TYPE_20VAL_EN:	limitType = Measure::LimitType::Engineering;	break;

			default:
				assert(0);
		}

		if (limitType == Measure::LimitType::NoLimitType)
		{
			return;
		}

		// QChart
		//
		QtCharts::QChart* pChart = new QtCharts::QChart();
		if (pChart == nullptr)
		{
			return;
		}

		pChart->setTitle(pLinearityMeasurement->appSignalID() + " - " + pLinearityMeasurement->caption());
		pChart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

		// Add lines
		//
		int pointCount = 0;

		switch (graphType)
		{
			case MVG_TYPE_LIN_EL:
			case MVG_TYPE_LIN_EN:
				{
					QtCharts::QLineSeries* pNominalSeries = new QtCharts::QLineSeries();
					QtCharts::QLineSeries* pMeasureSeries = new QtCharts::QLineSeries();

					if (pNominalSeries == nullptr || pMeasureSeries == nullptr)
					{
						break;
					}

					pNominalSeries->setColor(Qt::green);
					pNominalSeries->setName(tr("Nominal"));

					pMeasureSeries->setColor(Qt::red);
					pMeasureSeries->setName(tr("Measure"));

					QtCharts::QLineSeries* pLowLimitlSeries = new QtCharts::QLineSeries();
					QtCharts::QLineSeries* pHighLimitSeries = new QtCharts::QLineSeries();

					if (pLowLimitlSeries == nullptr || pHighLimitSeries == nullptr)
					{
						break;
					}

					pLowLimitlSeries->setPen(QPen(Qt::black, 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
					pLowLimitlSeries->setName(tr("Low limit"));

					pHighLimitSeries->setPen(QPen(Qt::black, 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
					pHighLimitSeries->setName(tr("High limit"));

					for (int i = 0; i < measureCount; i++)
					{
						Measure::Item* pMeasurementI = m_table.at(i);
						if (pMeasurementI == nullptr)
						{
							continue;
						}

						if (pMeasurementI->signalHash() != pMeasurement->signalHash())
						{
							continue;
						}

						Measure::LinearityItem* pLinearityMeasurementI = dynamic_cast<Measure::LinearityItem*>(pMeasurementI);
						if (pLinearityMeasurementI == nullptr)
						{
							continue;
						}

						QPointF pN(pointCount + 1, pLinearityMeasurementI->nominal(limitType));
						*pNominalSeries << pN;

						QPointF pM(pointCount + 1, pLinearityMeasurementI->measure(limitType));
						*pMeasureSeries << pM;

						if (pLinearityMeasurement->nominal(limitType) >= 0)
						{
							QPointF pLL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) + pLinearityMeasurementI->errorLimit(limitType, Measure::ErrorType::Absolute));
							*pLowLimitlSeries << pLL;

							QPointF pHL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) - pLinearityMeasurementI->errorLimit(limitType, Measure::ErrorType::Absolute));
							*pHighLimitSeries << pHL;
						}
						else
						{
							QPointF pLL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) - pLinearityMeasurementI->errorLimit(limitType, Measure::ErrorType::Absolute));
							*pLowLimitlSeries << pLL;

							QPointF pHL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) + pLinearityMeasurementI->errorLimit(limitType, Measure::ErrorType::Absolute));
							*pHighLimitSeries << pHL;
						}

						pointCount ++;
					}

					pChart->addSeries(pNominalSeries);
					pChart->addSeries(pMeasureSeries);
					pChart->addSeries(pHighLimitSeries);
					pChart->addSeries(pLowLimitlSeries);
				}
				break;

			case MVG_TYPE_20VAL_EL:
			case MVG_TYPE_20VAL_EN:
				{
					QtCharts::QLineSeries* pMeasureSeries = new QtCharts::QLineSeries();
					if (pMeasureSeries == nullptr)
					{
						break;
					}

					pMeasureSeries->setColor(Qt::red);
					pMeasureSeries->setName(tr("Measure (Nominal = %1)").
											arg(QString::number(pLinearityMeasurement->nominal(limitType), 'f',
																pLinearityMeasurement->limitPrecision(limitType))));

					for (int i = 0; i < pLinearityMeasurement->measureCount(); i++)
					{
						QPointF pM(pointCount + 1, pLinearityMeasurement->measureItemArray(limitType ,i));
						*pMeasureSeries << pM;

						pointCount ++;
					}

					pChart->addSeries(pMeasureSeries);
				}
				break;

			default:
				assert(0);
				break;
		}

		if (pointCount == 0)
		{
			return;
		}

		// Asix
		//
		pChart->createDefaultAxes();

		QList<QtCharts::QAbstractAxis*> axisXList = pChart->axes(Qt::Horizontal);
		if (axisXList.isEmpty() == true)
		{
			return;
		}

		QtCharts::QValueAxis* pAxisX = dynamic_cast<QtCharts::QValueAxis*>(axisXList.at(0));
		if (pAxisX == nullptr)
		{
			return;
		}

		pAxisX->setRange(1, pointCount);
		pAxisX->setTickCount(pointCount);
		pAxisX->setLabelFormat("%.0f");

		QList<QtCharts::QAbstractAxis*> axisYList = pChart->axes(Qt::Vertical);
		if (axisYList.isEmpty() == true)
		{
			return;
		}

		QtCharts::QValueAxis* pAxisY = dynamic_cast<QtCharts::QValueAxis*>(axisYList.at(0));
		if (pAxisY == nullptr)
		{
			return;
		}

		pAxisY->setLabelFormat(QString("%.%1f").arg(pLinearityMeasurement->limitPrecision(limitType)));

		// QChartView
		//
		ChartView* pChartView = new ChartView(pChart);
		if (pChartView == nullptr)
		{
			return;
		}

		pChartView->setRenderHint(QPainter::Antialiasing);

		// QDialog
		//
		QDialog dialog(this, Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Graph - %1").arg(pLinearityMeasurement->appSignalID()));

		QRect screen = QDesktopWidget().availableGeometry(this);
		dialog.resize(static_cast<int>(screen.width() * 0.7), static_cast<int>(screen.height() * 0.4));
		dialog.move(screen.center() - rect().center());

		dialog.grabGesture(Qt::PanGesture);
		dialog.grabGesture(Qt::PinchGesture);

		QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
		mainLayout->addWidget(pChartView);
		dialog.setLayout(mainLayout);

		dialog.exec();
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ChartView::ChartView(QtCharts::QChart* chart, QWidget* parent) :
	QChartView(chart, parent),
	m_isTouching(false)
{
	setRubberBand(QChartView::RectangleRubberBand);
}

// -------------------------------------------------------------------------------------------------------------------

bool ChartView::viewportEvent(QEvent* event)
{
	if (event->type() == QEvent::TouchBegin)
	{
		m_isTouching = true;

		chart()->setAnimationOptions(QtCharts::QChart::NoAnimation);
	}

	return QChartView::viewportEvent(event);
}

// -------------------------------------------------------------------------------------------------------------------

void ChartView::mousePressEvent(QMouseEvent* event)
{
	if (m_isTouching == true)
	{
		return;
	}

	QChartView::mousePressEvent(event);
}

// -------------------------------------------------------------------------------------------------------------------

void ChartView::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isTouching == true)
	{
		return;
	}

	QChartView::mouseMoveEvent(event);
}

// -------------------------------------------------------------------------------------------------------------------

void ChartView::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_isTouching == true)
	{
		m_isTouching = false;
	}

	chart()->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

	QChartView::mouseReleaseEvent(event);
}

// -------------------------------------------------------------------------------------------------------------------
void ChartView::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
		case Qt::Key_Plus:	chart()->zoomIn();						break;
		case Qt::Key_Minus:	chart()->zoomOut();						break;
		case Qt::Key_Left:	chart()->scroll(-10, 0);				break;
		case Qt::Key_Right:	chart()->scroll(10, 0);					break;
		case Qt::Key_Up:	chart()->scroll(0, 10);					break;
		case Qt::Key_Down:	chart()->scroll(0, -10);				break;
		default:			QGraphicsView::keyPressEvent(event);	break;
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
