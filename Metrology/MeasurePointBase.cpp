#include "MeasurePointBase.h"

#include "Database.h"


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

void MeasurePoint::setPercent(double value)
{
	m_percentValue = value;

	for(int s = 0; s < POINT_SENSOR_COUNT; s++)
	{
		switch(s)
		{
			case POINT_SENSOR_PERCENT:		m_sensorValue[s] = value;						break;
			case POINT_SENSOR_U_0_5_V:		m_sensorValue[s] = value * 5 / 100;				break;
			case POINT_SENSOR_U_m10_10_V:	m_sensorValue[s] = value * 20 / 100 + (-10);	break;
			case POINT_SENSOR_I_0_5_MA:		m_sensorValue[s] = value * 5 / 100;				break;
			case POINT_SENSOR_I_4_20_MA:	m_sensorValue[s] = value * 16 / 100 + 4;		break;
			case POINT_SENSOR_T_0_100_C:	m_sensorValue[s] = value * 100 / 100;			break;
			case POINT_SENSOR_T_0_150_C:	m_sensorValue[s] = value * 150 / 100;			break;
			case POINT_SENSOR_T_0_200_C:	m_sensorValue[s] = value * 200 / 100;			break;
			case POINT_SENSOR_T_0_400_C:	m_sensorValue[s] = value * 400 / 100;			break;
			default:						assert(0);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

double MeasurePoint::sensorValue(int sensor)
{
	if (sensor < 0 || sensor >= POINT_SENSOR_COUNT)
	{
		return 0;
	}

	return m_sensorValue[sensor];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasurePointBase::MeasurePointBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasurePointBase::clear()
{
	QMutexLocker l(&m_mutex);

	m_pointList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasurePointBase::count()
{
	QMutexLocker l(&m_mutex);

	return  m_pointList.count();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasurePointBase::append(const MeasurePoint& point)
{
	QMutexLocker l(&m_mutex);

	m_pointList.append(point);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasurePointBase::insert(int index, const MeasurePoint& point)
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index > m_pointList.count())
	{
		return;
	}

	m_pointList.insert(index, point);
}


// ----------------------------------------------------------------------------------------------

bool MeasurePointBase::remove(int index)
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index > m_pointList.count())
	{
		return false;
	}

	m_pointList.remove(index);

	return true;
}

// ----------------------------------------------------------------------------------------------

void MeasurePointBase::swap(int i, int j)
{
	QMutexLocker l(&m_mutex);

	if ((i < 0 || i >= m_pointList.count()) || (j < 0 || j >= m_pointList.count()))
	{
		return;
	}

	MeasurePoint point	= m_pointList[j];
	m_pointList[j]	= m_pointList[i];
	m_pointList[i]	= point;
}

// -------------------------------------------------------------------------------------------------------------------

MeasurePoint MeasurePointBase::point(int index)
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index > m_pointList.count())
	{
		return MeasurePoint();
	}

	return  m_pointList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void MeasurePointBase::setPoint(int index, const MeasurePoint& point)
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index > m_pointList.count())
	{
		return;
	}

	m_pointList[index] = point;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasurePointBase::text()
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_pointList.isEmpty() == true)
	{
		result = QT_TRANSLATE_NOOP("MeasurePointBase.cpp", "The measurement points are not set");
	}
	else
	{
		int pointCount = m_pointList.count();
		for(int index = 0; index < pointCount; index++)
		{
			result.append(QString("%1%").arg(QString::number(m_pointList[index].percent(), 'f', 1)));

			if (index != pointCount - 1)
			{
				result.append(QString(", "));
			}
		}
	}

	return result;
}


// -------------------------------------------------------------------------------------------------------------------

void MeasurePointBase::initEmpty()
{
	const int valueCount = 7;
	double value[valueCount] = {5, 20, 40, 50, 60, 80, 95};

	for(int index = 0; index < valueCount; index++)
	{
		MeasurePoint point;

		point.setIndex(index);
		point.setPercent(value[index]);

		m_pointList.append(point);
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasurePointBase::load()
{
	if (theDatabase.isOpen() == false)
	{
		return false;
	}

	SqlTable* pTable = theDatabase.openTable(SQL_TABLE_LINEARITY_POINT);
	if (pTable == nullptr)
	{
		return false;
	}

	m_mutex.lock();

		int recordCount = pTable->recordCount();
		if (recordCount == 0)
		{
			// if table is empty then initialize data
			//
			initEmpty();

			// update table
			//
			pTable->write(m_pointList.data(), m_pointList.count());
		}
		else
		{
			// if table is not empty then read data from table
			//
			m_pointList.resize(recordCount);
			pTable->read(m_pointList.data());
		}

	m_mutex.unlock();

	pTable->close();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasurePointBase::save()
{
	if (theDatabase.isOpen() == false)
	{
		return false;
	}

	SqlTable* pTable = theDatabase.openTable(SQL_TABLE_LINEARITY_POINT);
	if (pTable == nullptr)
	{
		return false;
	}

	m_mutex.lock();

		if (pTable->clear() == true)
		{
			pTable->write(m_pointList.data(), m_pointList.count());
		}

	m_mutex.unlock();

	pTable->close();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

MeasurePointBase& MeasurePointBase::operator=(const MeasurePointBase& from)
{
	QMutexLocker l(&m_mutex);

	m_pointList = from.m_pointList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
