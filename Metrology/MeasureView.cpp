#include "MeasureView.h"
#include "Database.h"
#include "ProcessData.h"
#include "DialogObjectProperties.h"
#include "Options.h"

namespace Measure
{
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	Model::Model(QObject*)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	Model::~Model()
	{
		QMutexLocker l(&m_measureMutex);

		m_measureList.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool Model::columnIsVisible(int column)
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

	int Model::columnCount(const QModelIndex&) const
	{
		return m_header.count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	int Model::rowCount(const QModelIndex&) const
	{
		return TO_INT(m_measureCount);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role != Qt::DisplayRole)
		{
			return QVariant();
		}

		QVariant result = QVariant();

		if (orientation == Qt::Horizontal)
		{
			HeaderColumn* pColumn = m_header.column(section);
			if (pColumn != nullptr)
			{
				result = pColumn->title();
			}
		}

		if (orientation == Qt::Vertical)
		{
			result = QString("%1").arg(section + 1);
		}

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QVariant Model::data(const QModelIndex &index, int role) const
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

					if (columnIndex == MVC_CMN_L_CUSTOM_ID || columnIndex == MVC_CMN_L_ERROR_RESULT)
					{
						return theOptions.measureView().fontBold();
					}

					break;

				case Measure::Type::Comparators:

					if (columnIndex == MVC_CMN_C_CUSTOM_ID || columnIndex == MVC_CMN_C_ERROR_RESULT)
					{
						return theOptions.measureView().fontBold();
					}

					break;

				default:
					assert(0);
			}

			return theOptions.measureView().font();
		}

		if (role == Qt::ForegroundRole)
		{
			if (pMeasurement->foundInStatistics() == false)
			{
				return QColor(Qt::lightGray);
			}
			else
			{
				if (pMeasurement->hasWrongRange() == true)
				{
					return QColor(Qt::red);
				}
			}

			return QVariant();
		}

		if (role == Qt::BackgroundRole)
		{
			switch(m_measureType)
			{
				case Measure::Type::Linearity:

					if (columnIndex == MVC_CMN_L_MODULE_SN)
					{
						if (pMeasurement->location().moduleSerialNo() == 0)
						{
							return QColor(Qt::yellow);
						}
					}

					if (columnIndex == MVC_CMN_L_ERROR_RESULT)
					{
						return backgroundColor(pMeasurement);
					}

					break;

				case Measure::Type::Comparators:

					if (columnIndex == MVC_CMN_C_MODULE_SN)
					{
						if (pMeasurement->location().moduleSerialNo() == 0)
						{
							return QColor(Qt::yellow);
						}
					}

					if (columnIndex == MVC_CMN_C_ERROR_RESULT)
					{
						return backgroundColor(pMeasurement);
					}

					break;

				default:
					assert(0);
			}

			return QVariant();
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return text(rowIndex, columnIndex, pMeasurement);
		}

		return QVariant();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QColor Model::backgroundColor(Measure::Item* pMeasurement) const
	{
		if (ERR_MEASURE_TYPE(m_measureType) == true)
		{
			return theOptions.measureView().colorErrorLimit();
		}

		if (pMeasurement == nullptr)
		{
			return theOptions.measureView().colorErrorLimit();
		}

		if (theOptions.measureView().showNoValid() == false)
		{
			if (pMeasurement->isSignalValid() == false)
			{
				return theOptions.measureView().colorErrorLimit();
			}
		}

		if (pMeasurement->errorResult(m_measureType) != Measure::ErrorResult::Ok)
		{
			return theOptions.measureView().colorErrorLimit();
		}

		return theOptions.measureView().colorNotError();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Model::text(int row, int column, Measure::Item* pMeasurement) const
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

			default:
				result.clear();
		}

		return result;

	}

	// -------------------------------------------------------------------------------------------------------------------

	QString Model::textLinearity(int row, int column, Measure::Item* pMeasurement) const
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
			case OT::LinearityViewType::Simple:
			case OT::LinearityViewType::Extended:			limitType = m->limitTypeByRange(theOptions.linearity().calcErrorByRange());		break;
			case OT::LinearityViewType::Detail_Electric:		limitType = Measure::LimitType::Electric;										break;
			case OT::LinearityViewType::Detail_Engineering:	limitType = Measure::LimitType::Engineering;									break;

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
			case MVC_CMN_L_MODULE_TYPE:				result = m->location().moduleCaption(); break;
			case MVC_CMN_L_CONNECT_APP_ID:			result = m->connectionSignalID(); break;
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

			case MVC_CMN_L_VALUE_COUNT:				result = QString::number(m->measureInPoint()); break;
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
			case MVC_CMN_L_LOW_BORDER:				result = m->additionalParamStr(limitType, Measure::AdditionalParam::LowBorder); break;
			case MVC_CMN_L_HIGH_BORDER:				result = m->additionalParamStr(limitType, Measure::AdditionalParam::HighBorder); break;
			case MVC_CMN_L_UNCERTAINTY:				result = m->additionalParamStr(limitType, Measure::AdditionalParam::Uncertainty); break;

			case MVC_CMN_L_ERROR:					result = m->errorStr(m_measureType); break;
			case MVC_CMN_L_ERROR_LIMIT:				result = m->errorLimitStr(m_measureType); break;
			case MVC_CMN_L_ERROR_RESULT:			result = m->errorResultStr(m_measureType); break;

			case MVC_CMN_L_MEASUREMENT_TIME:		result = m->measureTimeStr(); break;
			case MVC_CMN_L_CALIBRATOR:				result = m->calibrator(); break;

			default:
				result.clear();
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

	QString Model::textComparator(int row, int column, Measure::Item* pMeasurement) const
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
			case MVC_CMN_C_MODULE_TYPE:				result = m->location().moduleCaption(); break;
			case MVC_CMN_C_CONNECT_APP_ID:			result = m->connectionSignalID(); break;
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

			case MVC_CMN_C_ERROR:					result = m->errorStr(m_measureType); break;
			case MVC_CMN_C_ERROR_LIMIT:				result = m->errorLimitStr(m_measureType); break;
			case MVC_CMN_C_ERROR_RESULT:			result = m->errorResultStr(m_measureType); break;

			case MVC_CMN_C_MEASUREMENT_TIME:		result = m->measureTimeStr(); break;
			case MVC_CMN_C_CALIBRATOR:				result = m->calibrator(); break;

			default:
				result.clear();
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

	bool Model::append(Measure::Item* pMeasurement)
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

	Measure::Item* Model::at(int index) const
	{
		QMutexLocker l(&m_measureMutex);

		if (index < 0 || index >= TO_INT(m_measureCount))
		{
			return nullptr;
		}

		return m_measureList[static_cast<quint64>(index)];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void Model::remove(const std::vector<int>& removeIndexList)
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

	void Model::set(const std::vector<Measure::Item*>& list_add)
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

	void Model::clear()
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
		m_model.header().init(measureType);
		m_model.setMeasureType(measureType);
		setModel(&m_model);

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
		if (m_headerContextMenu == nullptr)
		{
			return;
		}

		horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &View::onHeaderContextMenu);
		connect(horizontalHeader(), &QHeaderView::sectionResized, this, &View::onColumnResized);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::updateColumn()
	{
		if (m_headerContextMenu == nullptr)
		{
			return;
		}

		m_headerContextMenu->clear();

		m_model.header().updateColumnState();

		int count = m_model.header().count();
		for (int index = 0; index < count; index++)
		{
			HeaderColumn* pColumn = m_model.header().column(index);
			if (pColumn == nullptr)
			{
				continue;
			}

			setColumnWidth(index, pColumn->width());
			setColumnHidden(index, pColumn->enableVisible() == false);

			if (pColumn->enableVisible() == false)
			{
				continue;
			}

			QAction* pAction = m_headerContextMenu->addAction(pColumn->title());
			if (pAction == nullptr)
			{
				continue;
			}

			pAction->setCheckable(true);
			pAction->setChecked(true);
			pAction->setData(index);
		}

		disconnect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &View::onColumnAction);
		connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &View::onColumnAction);

		QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
		verticalHeader()->setDefaultSectionSize(cellSize.height());
	}

	// -------------------------------------------------------------------------------------------------------------------

	int View::firstVisibleColumn()
	{
		int visibleColumn = 0;

		int columnCount = model()->columnCount();
		for(int column = 0; column < columnCount; column++)
		{
			if (isColumnHidden(column) == true)
			{
				continue;
			}

			visibleColumn = column;

			break;
		}

		return visibleColumn;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::loadMeasurements(const Measure::Base& measureBase)
	{
		m_model.clear();

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

		m_model.set(measureList);
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
		if (m_model.append(pMeasurement) == false)
		{
			return;
		}

		setCurrentIndex(model()->index(m_model.count() - 1, firstVisibleColumn()));
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::removeMeasure()
	{
		int measureCount = m_model.count();
		if (measureCount == 0)
		{
			return;
		}

		std::vector<int> keyList;
		std::vector<int> removeIndexList;

		const QModelIndexList selectedList = selectionModel()->selectedRows();
		for(auto selectedIndex : selectedList)
		{
			int index = selectedIndex.row();
			if (index < 0 || index >= m_model.count())
			{
				continue;
			}

			Measure::Item* pMeasurement = m_model.at(index);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			if (pMeasurement->measureType() != m_measureType)
			{
				continue;
			}

			keyList.push_back(pMeasurement->measureID());

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

		std::sort(removeIndexList.begin(), removeIndexList.end());

		// remove from MeasureTable
		//
		m_model.remove(removeIndexList);

		// remove from Database and MesaureBase
		//
		emit removeFromBase(m_measureType, keyList);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onCopy()
	{
		CopyData copyData(this, false);
		copyData.exec();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onCopyCell()
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(model()->data(currentIndex()).toString());
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onProperty()
	{
		int measureIndex = currentIndex().row();
		if (measureIndex < 0 || measureIndex >= m_model.count())
		{
			return;
		}

		Measure::Item* pMeasurement = m_model.at(measureIndex);
		if (pMeasurement == nullptr)
		{
			return;
		}

		if (pMeasurement->measureType() != m_measureType)
		{
			return;
		}

		double errorLimit = pMeasurement->errorLimit(Measure::LimitType::Electric, MT::ErrorType::Reduce);

		DialogMeasureProperty dialog(pMeasurement, this);
		if (dialog.exec() != QDialog::Accepted)
		{
			return;
		}

		if (compareDouble(errorLimit, pMeasurement->errorLimit(Measure::LimitType::Electric, MT::ErrorType::Reduce)) == true)
		{
			return;
		}

		errorLimit = pMeasurement->errorLimit(Measure::LimitType::Electric, MT::ErrorType::Reduce);

		std::vector<Measure::Item*> measurementList;

		const QModelIndexList selectedList = selectionModel()->selectedRows();
		for(auto selectedIndex : selectedList)
		{
			int index = selectedIndex.row();
			if (index < 0 || index >= m_model.count())
			{
				continue;
			}

			pMeasurement = m_model.at(index);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			if (pMeasurement->measureType() != m_measureType)
			{
				continue;
			}

			pMeasurement->calcErrorLimit(errorLimit);

			measurementList.push_back(pMeasurement);
		}

		// update in Database
		//
		emit updateInBase(m_measureType, measurementList);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::showChart(ChartType chartType)
	{
		if (ERR_GRAPH_TYPE_TYPE(chartType) == true)
		{
			return;
		}

		int measureCount = m_model.count();
		if (measureCount == 0)
		{
			return;
		}

		int index = currentIndex().row();
		if (index < 0 || index >= measureCount)
		{
			return;
		}

		Measure::Item* pMeasurement = m_model.at(index);
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

		switch (chartType)
		{
			case ChartType::LinearityEl:
			case ChartType::Value20El:		limitType = Measure::LimitType::Electric;		break;
			case ChartType::LinearityEn:
			case ChartType::Value20En:		limitType = Measure::LimitType::Engineering;	break;

			default:
				assert(0);
		}

		if (limitType == Measure::LimitType::NoLimitType)
		{
			return;
		}

		// QChart
		//
		QChart* pChart = new QChart();
		if (pChart == nullptr)
		{
			return;
		}

		pChart->setTitle(pLinearityMeasurement->customAppSignalID() + " - " + pLinearityMeasurement->caption());
		pChart->setAnimationOptions(QChart::SeriesAnimations);

		// Add lines
		//
		int pointCount = 0;

		switch (chartType)
		{
			case ChartType::LinearityEl:
			case ChartType::LinearityEn:
				{
					QLineSeries* pNominalSeries = new QLineSeries();
					QLineSeries* pMeasureSeries = new QLineSeries();

					if (pNominalSeries == nullptr || pMeasureSeries == nullptr)
					{
						break;
					}

					pNominalSeries->setColor(Qt::green);
					pNominalSeries->setName(tr("Nominal"));

					pMeasureSeries->setColor(Qt::red);
					pMeasureSeries->setName(tr("Measure"));

					QLineSeries* pLowLimitlSeries = new QLineSeries();
					QLineSeries* pHighLimitSeries = new QLineSeries();

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
						Measure::Item* pMeasurementI = m_model.at(i);
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
							QPointF pLL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) + pLinearityMeasurementI->errorLimit(limitType, MT::ErrorType::Absolute));
							*pLowLimitlSeries << pLL;

							QPointF pHL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) - pLinearityMeasurementI->errorLimit(limitType, MT::ErrorType::Absolute));
							*pHighLimitSeries << pHL;
						}
						else
						{
							QPointF pLL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) - pLinearityMeasurementI->errorLimit(limitType, MT::ErrorType::Absolute));
							*pLowLimitlSeries << pLL;

							QPointF pHL(pointCount + 1, pLinearityMeasurementI->nominal(limitType) + pLinearityMeasurementI->errorLimit(limitType, MT::ErrorType::Absolute));
							*pHighLimitSeries << pHL;
						}

						pointCount ++;
					}

					pChart->addSeries(pNominalSeries);
					pChart->addSeries(pMeasureSeries);
					//pChart->addSeries(pHighLimitSeries);
					//pChart->addSeries(pLowLimitlSeries);
				}
				break;

			case ChartType::Value20El:
			case ChartType::Value20En:
				{
					QLineSeries* pMeasureSeries = new QLineSeries();
					if (pMeasureSeries == nullptr)
					{
						break;
					}

					pMeasureSeries->setColor(Qt::red);
					pMeasureSeries->setName(tr("Measure (Nominal = %1)").
											arg(QString::number(pLinearityMeasurement->nominal(limitType), 'f',
																pLinearityMeasurement->limitPrecision(limitType))));

					for (int i = 0; i < pLinearityMeasurement->measureInPoint(); i++)
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
		}

		if (pointCount == 0)
		{
			return;
		}

		// Asix
		//
		pChart->createDefaultAxes();

		QList<QAbstractAxis*> axisXList = pChart->axes(Qt::Horizontal);
		if (axisXList.isEmpty() == true)
		{
			return;
		}

		QValueAxis* pAxisX = dynamic_cast<QValueAxis*>(axisXList.at(0));
		if (pAxisX == nullptr)
		{
			return;
		}

		pAxisX->setRange(1, pointCount);
		pAxisX->setTickCount(pointCount);
		pAxisX->setLabelFormat("%.0f");

		QList<QAbstractAxis*> axisYList = pChart->axes(Qt::Vertical);
		if (axisYList.isEmpty() == true)
		{
			return;
		}

		QValueAxis* pAxisY = dynamic_cast<QValueAxis*>(axisYList.at(0));
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
		dialog.setWindowTitle(tr("Graph - %1").arg(pLinearityMeasurement->customAppSignalID()));

		QRect screen = parentWidget()->screen()->availableGeometry();
		dialog.resize(static_cast<int>(screen.width() * 0.7), static_cast<int>(screen.height() * 0.4));
		dialog.move(screen.center() - rect().center());

		dialog.grabGesture(Qt::PanGesture);
		dialog.grabGesture(Qt::PinchGesture);

		QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
		mainLayout->addWidget(pChartView);
		dialog.setLayout(mainLayout);

		dialog.exec();
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

	void View::onColumnAction(QAction* action)
	{
		if (action == nullptr)
		{
			return;
		}

		int index = action->data().toInt();
		if (index < 0 || index >= m_model.header().count())
		{
			return;
		}

		HeaderColumn* pColumn = m_model.header().column(index);
		if (pColumn == nullptr)
		{
			return;
		}

		setColumnHidden(index, action->isChecked() == false);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void View::onColumnResized(int index, int, int width)
	{
		if (index < 0 || index >= m_model.header().count())
		{
			return;
		}

		HeaderColumn* pColumn = m_model.header().column(index);
		if (pColumn == nullptr)
		{
			return;
		}

		if (pColumn->enableVisible() == false || width == 0)
		{
			return;
		}

		pColumn->setWidth(width);

		theOptions.measureView().saveColumnWidth(m_measureType, *pColumn);
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
}
