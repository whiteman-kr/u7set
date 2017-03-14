#include "MeasurementBase.h"

#include "Database.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

MeasurementBase theMeasurementBase;

// -------------------------------------------------------------------------------------------------------------------

MeasurementBase::MeasurementBase(QObject *parent) :
	QObject(parent)
{
	for (int t = 0; t < MEASURE_TYPE_COUNT; t++)
	{
		m_measurementCount[t] = 0;
	}
}

// -------------------------------------------------------------------------------------------------------------------

 MeasurementBase::~MeasurementBase()
 {
 }

// -------------------------------------------------------------------------------------------------------------------

int MeasurementBase::measurementCount() const
{
	int count = 0;

	m_measurmentListMutex.lock();

		count = m_measurementList.count();

	m_measurmentListMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int MeasurementBase::measurementCount(int measureType) const
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return 0;
	}

	int count = 0;

	m_measurmentListMutex.lock();

		count = m_measurementCount[measureType];

	m_measurmentListMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasurementBase::clear(bool removeData)
{
	m_measurmentListMutex.lock();

		if (removeData == true)
		{
			int count = m_measurementList.count();
			for(int i = count - 1; i >= 0; i--)
			{
				Measurement* pMeasurement = m_measurementList[i];
				if (pMeasurement == nullptr)
				{
					continue;
				}

				delete pMeasurement;
			}
		}

		m_measurementList.clear();

		for (int t = 0; t < MEASURE_TYPE_COUNT; t++)
		{
			m_measurementCount[t] = 0;
		}

	m_measurmentListMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------
// each measurement is located in several tables,
// firstly read data from the main table, and additional sub tables in memory
// later update the data in the main table from sub tables
//
int MeasurementBase::load(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return -1;
	}

	if (thePtrDB == nullptr)
	{
		return -1;
	}

	QTime responseTime;
	responseTime.start();

	m_measureType = measureType;

	struct rawTableData
	{
		int			 tableType;
		Measurement*	pMeasurement;
		int			 recordCount;
	};

	QVector<rawTableData> loadedTablesInMemory;

	// read all tables for current measureType in memory
	//
	for(int tableType = 0; tableType < SQL_TABLE_COUNT; tableType++)
	{
		if (SqlTableByMeasureType[tableType] == measureType)
		{
			SqlTable* table = thePtrDB->openTable(tableType);
			if (table != nullptr)
			{
				rawTableData data;

				// determine size data to allocate memory

				data.tableType = tableType;
				data.pMeasurement = nullptr;
				data.recordCount = table->recordCount();

				// allocate memory

				switch(measureType)
				{
					case MEASURE_TYPE_LINEARITY:			data.pMeasurement = new LinearityMeasurement[data.recordCount];		break;
					case MEASURE_TYPE_COMPARATOR:			data.pMeasurement = new ComparatorMeasurement[data.recordCount];	break;
					default:								assert(0);
				}

				// load data to memory

				if (data.pMeasurement != nullptr)
				{
					if (table->read(data.pMeasurement) == data.recordCount)
					{
						loadedTablesInMemory.append(data);
					}
				}

				table->close();
			}
		}
	}

	// if tables for current measureType is not exist, then exit
	//
	int tableInMemoryCount = loadedTablesInMemory.count();
	if (tableInMemoryCount == 0)
	{
		return 0;
	}

	// get main table, afterwards from sub tables update data in main table
	// append data-measurement in MeasurementBase
	//

	rawTableData mainTable = loadedTablesInMemory[SQL_TABLE_IS_MAIN];

	for(int mainIndex = 0; mainIndex < mainTable.recordCount; mainIndex++)
	{
		Measurement* pMainMeasure = mainTable.pMeasurement->at(mainIndex);
		if (pMainMeasure == nullptr)
		{
			continue;
		}

		for(int tableInMemory = SQL_TABLE_IS_SUB; tableInMemory < tableInMemoryCount; tableInMemory++)
		{
			rawTableData subTable = loadedTablesInMemory[tableInMemory];

			for(int subIndex = 0; subIndex < subTable.recordCount; subIndex++)
			{
				Measurement* pSubMeasure = subTable.pMeasurement->at(subIndex);
				if (pSubMeasure == nullptr)
				{
					continue;
				}

				// update main measurement from sub measurement
				//
				if (pMainMeasure->measureID() == pSubMeasure->measureID())
				{
					switch(subTable.tableType)
					{
						case SQL_TABLE_LINEARITY_20_EL:			static_cast<LinearityMeasurement*>(pMainMeasure)->updateMeasureArray(VALUE_TYPE_IN_ELECTRIC, pSubMeasure);	break;
						case SQL_TABLE_LINEARITY_20_PH:			static_cast<LinearityMeasurement*>(pMainMeasure)->updateMeasureArray(VALUE_TYPE_PHYSICAL, pSubMeasure);		break;
						case SQL_TABLE_LINEARITY_ADD_VAL:		static_cast<LinearityMeasurement*>(pMainMeasure)->updateAdditionalParam(pSubMeasure);						break;
						case SQL_TABLE_COMPARATOR_HYSTERESIS:	static_cast<ComparatorMeasurement*>(pMainMeasure)->updateHysteresis(pSubMeasure);							break;
					}

					break;
				}
			}
		}
	}

	// append measuremets to MeasureBase from updated main table
	//
	for(int index = 0; index < mainTable.recordCount; index++)
	{
		Measurement* pMeasureTable = mainTable.pMeasurement->at(index);
		if (pMeasureTable == nullptr)
		{
			continue;
		}

		Measurement* pMeasureAppend = nullptr;

		switch(measureType)
		{
			case MEASURE_TYPE_LINEARITY:	pMeasureAppend = new LinearityMeasurement;	break;
			case MEASURE_TYPE_COMPARATOR:	pMeasureAppend = new ComparatorMeasurement;	break;
			default:						assert(0);									break;
		}

		if (pMeasureAppend == nullptr)
		{
			continue;
		}

		*pMeasureAppend = *pMeasureTable;

		append(pMeasureAppend);
	}

	// if measurement is nonexistentin in main table, but exist in sub table,
	// need remove this measurement in sub table
	// remove nonexistent indexes-measurements-ID in sub tables
	//
	for(int tableInMemory = SQL_TABLE_IS_SUB; tableInMemory < tableInMemoryCount; tableInMemory++)
	{
		rawTableData subTable = loadedTablesInMemory[tableInMemory];

		QVector<int> removeKeyList;

		for(int subIndex = 0; subIndex < subTable.recordCount; subIndex++)
		{
			Measurement* pSubMeasure = subTable.pMeasurement->at(subIndex);
			if (pSubMeasure == nullptr)
			{
				continue;
			}

			bool foundMeasure = false;

			for(int mainIndex = 0; mainIndex < mainTable.recordCount; mainIndex++)
			{
				Measurement* pMainMeasure = mainTable.pMeasurement->at(mainIndex);
				if (pMainMeasure == nullptr)
				{
					continue;
				}

				if (pMainMeasure->measureID() == pSubMeasure->measureID())
				{
					foundMeasure = true;
					break;
				}
			}

			// if measurement is not found in main table then need remove it in sub table
			//
			if (foundMeasure == false)
			{
				removeKeyList.append(pSubMeasure->measureID());
			}
		}

		// remove unnecessary measurement from sub table
		//
		SqlTable* table = thePtrDB->openTable(subTable.tableType);
		if (table != nullptr)
		{
			table->remove(removeKeyList.data(), removeKeyList.count());
			table->close();
		}
	}

	// remove raw table data from memory
	//
	for(int tableInMemory = 0; tableInMemory < tableInMemoryCount; tableInMemory++)
	{
		rawTableData table = loadedTablesInMemory[tableInMemory];

		if (table.pMeasurement == nullptr)
		{
			continue;
		}

		switch(measureType)
		{
			case MEASURE_TYPE_LINEARITY:	delete [] static_cast<LinearityMeasurement*> (table.pMeasurement);	break;
			case MEASURE_TYPE_COMPARATOR:	delete [] static_cast<ComparatorMeasurement*> (table.pMeasurement);	break;
			default:						assert(0);
		}
	}

	qDebug() << __FUNCTION__ << ": MeasureType: " << measureType << ", Loaded MeasureItem: " << measurementCount() << ", Time for load: " << responseTime.elapsed() << " ms";

	return measurementCount();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasurementBase::append(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return -1;
	}

	int index = -1;

	m_measurmentListMutex.lock();

		m_measurementList.append(pMeasurement);
		index = m_measurementList.count() - 1;

		int measureType = pMeasurement->measureType();
		if (measureType >= 0 && measureType < MEASURE_TYPE_COUNT)
		{
			m_measurementCount[measureType] ++;
		}

	m_measurmentListMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasurementBase::remove(int index, bool removeData)
{
	if (index < 0 || index >= measurementCount())
	{
		return false;
	}

	m_measurmentListMutex.lock();

		Measurement* pMeasurement = m_measurementList[index];
		if (pMeasurement != nullptr)
		{
			int measureType = pMeasurement->measureType();
			if (measureType >= 0 && measureType < MEASURE_TYPE_COUNT)
			{
				m_measurementCount[measureType] --;
			}

			if (removeData == true)
			{
				delete pMeasurement;
			}
		}

		m_measurementList.remove(index);

	m_measurmentListMutex.unlock();

	return true;
}


// ----------------------------------------------------------------------------------------------

Measurement* MeasurementBase::measurement(int index) const
{
	if (index < 0 || index >= measurementCount())
	{
		return nullptr;
	}

	Measurement *pMeasurement = nullptr;

	m_measurmentListMutex.lock();

		pMeasurement = m_measurementList[index];

	m_measurmentListMutex.unlock();

	return pMeasurement;
}

// -------------------------------------------------------------------------------------------------------------------

StatisticItem MeasurementBase::statistic(const Hash& signalHash)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return StatisticItem();
	}

	StatisticItem si(signalHash);

	m_measurmentListMutex.lock();

		int measureCount = m_measurementList.size();
		for(int i = 0; i < measureCount; i ++)
		{
			Measurement* pMeasurement = m_measurementList[i];
			if (pMeasurement == 0)
			{
				continue;
			}

			if (pMeasurement->signalHash() != signalHash)
			{
				continue;
			}

			switch(pMeasurement->measureType())
			{
				case MEASURE_TYPE_LINEARITY:
					{
						LinearityMeasurement* pLinearityMeasurement = static_cast<LinearityMeasurement*>(pMeasurement);
						if (pLinearityMeasurement == nullptr)
						{
							break;
						}

						int errorType = theOptions.linearity().errorType();
						if (errorType < 0 || errorType >= MEASURE_ERROR_TYPE_COUNT)
						{
							break;
						}

						si.incrementMeasureCount();

						if (pLinearityMeasurement->error(VALUE_TYPE_PHYSICAL, errorType) > pLinearityMeasurement->errorLimit(VALUE_TYPE_PHYSICAL, errorType))
						{
							si.setState(STATISTIC_STATE_INVALID);
						}
					}
					break;

				case MEASURE_TYPE_COMPARATOR:
					static_cast<ComparatorMeasurement*> (pMeasurement);
					break;

				default:
					assert(0);
			}
		}

	m_measurmentListMutex.unlock();

	return si;
}

// -------------------------------------------------------------------------------------------------------------------
