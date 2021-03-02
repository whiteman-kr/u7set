#ifndef MEASUREPOINTBASE_H
#define MEASUREPOINTBASE_H

#include <QObject>
#include <QMutex>

namespace Measure
{
	// ==============================================================================================

	const double PointValue[] =
	{
		5, 20, 40, 50, 60, 80, 95	// default point value in percent
	};

	const int PointValueCount = sizeof(PointValue)/sizeof(PointValue[0]);

	// ==============================================================================================

	enum PointSensor
	{
		NoPointSensor	= -1,

		Percent			= 0,

		U_0_5_V			= 1,
		U_m10_10_V		= 2,

		I_0_5_mA		= 3,
		I_4_20_mA		= 4,

		T_0_100_C		= 5,
		T_0_150_C		= 6,
		T_0_200_C		= 7,
		T_0_400_C		= 8,
	};

	const int PointSensorCount = 9;

	#define ERR_POINT_SENSOR(sensor) (TO_INT(sensor) < 0 || TO_INT(sensor) >= Measure::PointSensorCount)

	QString PointSensorCaption(int sensor);

	// ==============================================================================================

	enum LinearityDivision
	{
		NoLinearityDivision	= -1,

		Manual				= 0,
		Automatic			= 1,
	};

	const int LinearityDivisionCount = 2;

	#define ERR_LINEARITY_DIVISION(division) (TO_INT(division) < 0 || TO_INT(division) >= Measure::LinearityDivisionCount)

	QString LinearityDivisionCaption(int division);

	// ==============================================================================================

	const double LinearityRangeLow	= 5;	// %
	const double LinearityRangeHigh	= 95;	// %

	// ==============================================================================================

	class Point
	{
	public:

		Point() { setPercent(0); }
		explicit Point(double percent) { setPercent(percent); }
		virtual ~Point() {}

	public:

		int Index() const { return m_index; }
		void setIndex(int index) { m_index = index; }

		double percent() const {return m_sensorValue[Measure::PointSensor::Percent]; }
		void setPercent(double value);

		double sensorValue(int sensor);

	private:

		int m_index = -1;

		double m_sensorValue[PointSensorCount];
	};

	// ==============================================================================================

	class PointBase : public QObject
	{
		Q_OBJECT

	public:

		explicit PointBase(QObject* parent = nullptr);
		virtual ~PointBase() {}

	public:

		void clear();
		int count();
		bool isEmpty() { return count() == 0; }

		void append(const Point& point);
		void insert(int index, const Point& point);
		bool remove(int index);
		void swap(int i, int j);

		Point point(int index);
		void setPoint(int index, const Point& point);

		QString text();

		void initEmpty();
		bool load();
		bool save();

		PointBase& operator=(const PointBase& from);

	private:

		mutable QMutex m_mutex;
		QVector<Point> m_pointList;
	};

	// ==============================================================================================
}

#endif // MEASUREPOINTBASE_H
