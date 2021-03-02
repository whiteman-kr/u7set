#include "MeasurePointBase.h"

#include "Database.h"

namespace Measure
{
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	void Point::setPercent(double value)
	{
		for(int sensor = 0; sensor < PointSensorCount; sensor++)
		{
			double sensorValue = 0;

			switch(sensor)
			{
				case PointSensor::Percent:		sensorValue = value;					break;

				case PointSensor::U_0_5_V:		sensorValue = value * 5 / 100;			break;
				case PointSensor::U_m10_10_V:	sensorValue = value * 20 / 100 + (-10);	break;

				case PointSensor::I_0_5_mA:		sensorValue = value * 5 / 100;			break;
				case PointSensor::I_4_20_mA:	sensorValue = value * 16 / 100 + 4;		break;

				case PointSensor::T_0_100_C:	sensorValue = value * 100 / 100;		break;
				case PointSensor::T_0_150_C:	sensorValue = value * 150 / 100;		break;
				case PointSensor::T_0_200_C:	sensorValue = value * 200 / 100;		break;
				case PointSensor::T_0_400_C:	sensorValue = value * 400 / 100;		break;

				default:						assert(0);
			}

			m_sensorValue[sensor] = sensorValue;
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	double Point::sensorValue(int sensor)
	{
		if (ERR_POINT_SENSOR(sensor) == true)
		{
			return 0;
		}

		return m_sensorValue[sensor];
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	PointBase::PointBase(QObject* parent) :
		QObject(parent)
	{
	}

	// -------------------------------------------------------------------------------------------------------------------

	void PointBase::clear()
	{
		QMutexLocker l(&m_mutex);

		m_pointList.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	int PointBase::count()
	{
		QMutexLocker l(&m_mutex);

		return  m_pointList.count();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void PointBase::append(const Point& point)
	{
		QMutexLocker l(&m_mutex);

		m_pointList.append(point);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void PointBase::insert(int index, const Point& point)
	{
		QMutexLocker l(&m_mutex);

		if (index < 0 || index > m_pointList.count())
		{
			return;
		}

		m_pointList.insert(index, point);
	}


	// ----------------------------------------------------------------------------------------------

	bool PointBase::remove(int index)
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

	void PointBase::swap(int i, int j)
	{
		QMutexLocker l(&m_mutex);

		if ((i < 0 || i >= m_pointList.count()) || (j < 0 || j >= m_pointList.count()))
		{
			return;
		}

		Point point	= m_pointList[j];
		m_pointList[j]	= m_pointList[i];
		m_pointList[i]	= point;
	}

	// -------------------------------------------------------------------------------------------------------------------

	Point PointBase::point(int index)
	{
		QMutexLocker l(&m_mutex);

		if (index < 0 || index > m_pointList.count())
		{
			return Point();
		}

		return  m_pointList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void PointBase::setPoint(int index, const Point& point)
	{
		QMutexLocker l(&m_mutex);

		if (index < 0 || index > m_pointList.count())
		{
			return;
		}

		m_pointList[index] = point;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString PointBase::text()
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

	void PointBase::initEmpty()
	{
		for(int index = 0; index < PointValueCount; index++)
		{
			Point point;

			point.setIndex(index);
			point.setPercent(PointValue[index]);

			m_pointList.append(point);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool PointBase::load()
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

	bool PointBase::save()
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

	PointBase& PointBase::operator=(const PointBase& from)
	{
		QMutexLocker l(&m_mutex);

		m_pointList = from.m_pointList;

		return *this;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	QString PointSensorCaption(int sensor)
	{
		QString caption;

		switch (sensor)
		{
			case PointSensor::Percent:		caption = QT_TRANSLATE_NOOP("MeasurePointBase", "%");			break;

			case PointSensor::U_0_5_V:		caption = QT_TRANSLATE_NOOP("MeasurePointBase", "0 .. 5 V");	break;
			case PointSensor::U_m10_10_V:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "-10 .. 10 V");	break;

			case PointSensor::I_0_5_mA:		caption = QT_TRANSLATE_NOOP("MeasurePointBase", "0 .. 5 mA");	break;
			case PointSensor::I_4_20_mA:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "4 .. 20 mA");	break;

			case PointSensor::T_0_100_C:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "0 .. 100 °C");	break;
			case PointSensor::T_0_150_C:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "0 .. 150 °C");	break;
			case PointSensor::T_0_200_C:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "0 .. 200 °C");	break;
			case PointSensor::T_0_400_C:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "0 .. 400 °C");	break;

			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasurePointBase", "Unknown");
		}

		return caption;
	};

	// -------------------------------------------------------------------------------------------------------------------

	QString LinearityDivisionCaption(int division)
	{
		QString caption;

		switch (division)
		{
			case LinearityDivision::Manual:		caption = QT_TRANSLATE_NOOP("MeasurePointBase", "Manual division of the measure range");	break;
			case LinearityDivision::Automatic:	caption = QT_TRANSLATE_NOOP("MeasurePointBase", "Automatic division of the measure range");	break;

			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MeasurePointBase", "Unknown");
		}

		return caption;
	};

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
}
