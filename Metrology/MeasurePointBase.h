#ifndef MEASUREPOINTBASE_H
#define MEASUREPOINTBASE_H

#include <QObject>
#include <QMutex>

// ==============================================================================================

const double MeasurePointValue[] =
{
	5, 20, 40, 50, 60, 80, 95
};

const int MeasurePointValueCount = sizeof(MeasurePointValue)/sizeof(MeasurePointValue[0]);

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

const int PointSensorCount	= 9;

#define ERR_POINT_SENSOR(sensor) (TO_INT(sensor) < 0 || TO_INT(sensor) >= PointSensorCount)

QString PointSensorCaption(int sensor);

// ==============================================================================================

class MeasurePoint
{
public:

	MeasurePoint() { setPercent(0); }
	explicit MeasurePoint(double percent) { setPercent(percent); }
	virtual ~MeasurePoint() {}

public:

	int Index() const { return m_index; }
	void setIndex(int index) { m_index = index; }

	double percent() const {return m_sensorValue[PointSensor::Percent]; }
	void setPercent(double value);

	double sensorValue(int sensor);

private:

	int m_index = -1;

	double m_sensorValue[PointSensorCount];
};

// ==============================================================================================

class MeasurePointBase : public QObject
{
	Q_OBJECT

public:

	explicit MeasurePointBase(QObject* parent = nullptr);
	virtual ~MeasurePointBase() {}

public:

	void clear();
	int count();
	bool isEmpty() { return count() == 0; }

	void append(const MeasurePoint& point);
	void insert(int index, const MeasurePoint& point);
	bool remove(int index);
	void swap(int i, int j);

	MeasurePoint point(int index);
	void setPoint(int index, const MeasurePoint& point);

	QString text();

	void initEmpty();
	bool load();
	bool save();

	MeasurePointBase& operator=(const MeasurePointBase& from);

private:

	mutable QMutex m_mutex;
	QVector<MeasurePoint> m_pointList;
};

// ==============================================================================================

#endif // MEASUREPOINTBASE_H
