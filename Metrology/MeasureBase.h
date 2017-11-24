#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include "../lib/Hash.h"
#include "SignalBase.h"

// ==============================================================================================

const char* const MeasureType[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Measurements of linearity"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Measurements of comparators"),
};

const int	MEASURE_TYPE_COUNT = sizeof(MeasureType)/sizeof(MeasureType[0]);

const int	MEASURE_TYPE_UNKNOWN	= -1,
			MEASURE_TYPE_LINEARITY	= 0,
			MEASURE_TYPE_COMPARATOR	= 1;

// ----------------------------------------------------------------------------------------------

const char* const MeasureFileName[MEASURE_TYPE_COUNT] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Linearity"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Comparators"),
};

// ==============================================================================================

const int	MeasureTimeout[] =
{
			0, 1, 2, 3, 5, 10, 15, 20, 30, 45, 60,
};

const int	MeasureTimeoutCount = sizeof(MeasureTimeout)/sizeof(MeasureTimeout[0]);

// ==============================================================================================

const char* const	MeasureKind[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", " in one rack"),
			QT_TRANSLATE_NOOP("MeasureBase.h", " in several racks"),
};

const int	MEASURE_KIND_COUNT		= sizeof(MeasureKind)/sizeof(MeasureKind[0]);

const int	MEASURE_KIND_UNKNOWN	= -1,
			MEASURE_KIND_ONE		= 0,
			MEASURE_KIND_MULTI		= 1;

// ==============================================================================================

const char* const MeasureLimitType[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Electric"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Physical"),
};

const int	MEASURE_LIMIT_TYPE_COUNT		= sizeof(MeasureLimitType)/sizeof(MeasureLimitType[0]);

const int	MEASURE_LIMIT_TYPE_UNDEFINED	= -1,
			MEASURE_LIMIT_TYPE_ELECTRIC		= 0,
			MEASURE_LIMIT_TYPE_PHYSICAL		= 1;

// ==============================================================================================

const char* const ErrorType[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Absolute"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Reduced"),
};

const int	MEASURE_ERROR_TYPE_COUNT	= sizeof(ErrorType)/sizeof(ErrorType[0]);

const int	MEASURE_ERROR_TYPE_UNKNOWN	= -1,
			MEASURE_ERROR_TYPE_ABSOLUTE	= 0,
			MEASURE_ERROR_TYPE_REDUCE	= 1;

// ==============================================================================================

const char* const MeasureAdditionalParam[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Measure value max"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "System error"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Standard deviation"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Low High border"),
};

const int	MEASURE_ADDITIONAL_PARAM_COUNT				= sizeof(MeasureAdditionalParam)/sizeof(MeasureAdditionalParam[0]);

const int	MEASURE_ADDITIONAL_PARAM_UNKNOWN			= -1,
			MEASURE_ADDITIONAL_PARAM_MAX_VALUE			= 0,
			MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR		= 1,
			MEASURE_ADDITIONAL_PARAM_SD					= 2,
			MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER	= 3;

			// maximum 16 items (0 .. 15)
			// now used 4 (0 .. 3)

// ==============================================================================================

const int	MAX_MEASUREMENT_IN_POINT	= 20;

// ==============================================================================================

#define	 MEASURE_TIME_FORMAT			"dd-MM-yyyy hh:mm:ss"

// ==============================================================================================

class Measurement
{

public:

	explicit Measurement(int measureType = MEASURE_TYPE_UNKNOWN);
	virtual ~Measurement();

private:

	int				m_measureType = MEASURE_TYPE_UNKNOWN;			// measure type
	Hash			m_signalHash = 0;								// hash calced from AppSignalID by function calcHash()

	int				m_measureID = -1;								// primary key of record in SQL table
	bool			m_filter = false;								// filter for record, if "true" - hide record

	QDateTime		m_measureTime;									// measure time
	int				m_reportType = -1;								// report type

public:

	void			virtual clear() {}

	int				measureType() const { return m_measureType; }
	void			setMeasureType(int type) { m_measureType = type; }

	Hash			signalHash() const { return m_signalHash; }
	void			setSignalHash(const Hash& hash) { m_signalHash = hash; }
	void			setSignalHash(const QString& id) { m_signalHash = calcHash(id); }

	int				measureID() const { return m_measureID; }
	void			setMeasureID(int id) { m_measureID = id; }

	bool			filter() const { return m_filter; }
	void			setFilter(bool filter) { m_filter = filter; }

	QDateTime		measureTime() const { return m_measureTime; }
	void			setMeasureTime(const QDateTime& time) { m_measureTime = time; }

	int				reportType() const { return m_reportType; }
	void			setReportType(int type) { m_reportType = type; }

	Measurement*	at(int index);

	Measurement&	operator=(Measurement& from);
};

// ==============================================================================================

class LinearityMeasurement : public Measurement
{

public:

	LinearityMeasurement();
	LinearityMeasurement(const MeasureMultiParam& measureParam);
	virtual ~LinearityMeasurement();

private:

	QString			m_appSignalID;
	QString			m_customAppSignalID;
	QString			m_caption;

	Metrology::SignalLocation m_location;

	double			m_percent = 0;

	double			m_nominal[MEASURE_LIMIT_TYPE_COUNT];
	double			m_measure[MEASURE_LIMIT_TYPE_COUNT];

	double			m_lowLimit[MEASURE_LIMIT_TYPE_COUNT];
	double			m_highLimit[MEASURE_LIMIT_TYPE_COUNT];
	QString			m_unit[MEASURE_LIMIT_TYPE_COUNT];
	int				m_limitPrecision[MEASURE_LIMIT_TYPE_COUNT];

	double			m_adjustment = 0;

	double			m_error[MEASURE_LIMIT_TYPE_COUNT][MEASURE_ERROR_TYPE_COUNT];
	double			m_errorLimit[MEASURE_LIMIT_TYPE_COUNT][MEASURE_ERROR_TYPE_COUNT];

	int				m_measureCount = 0;
	double			m_measureArray[MEASURE_LIMIT_TYPE_COUNT][MAX_MEASUREMENT_IN_POINT];

	int				m_additionalParamCount = 0;
	double			m_additionalParam[MEASURE_ADDITIONAL_PARAM_COUNT];

public:

	void			virtual clear();

	void			fill_measure_aim(const MeasureMultiParam& measureParam);
	void			fill_measure_aom(const MeasureMultiParam& measureParam);

	void			setLimits(const Metrology::SignalParam& param);
	void			calcError();
	void			calcAdditionalParam(int limitType);

	QString			appSignalID() const { return m_appSignalID; }
	void			setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; setSignalHash(m_appSignalID); }

	QString			customAppSignalID() const { return m_customAppSignalID; }
	void			setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

	QString			signalID(int type) const;

	QString			caption() const { return m_caption; }
	void			setCaption(const QString& caption) { m_caption = caption; }

	Metrology::SignalLocation& location() { return m_location; }
	void			setLocation(const Metrology::SignalLocation& location) { m_location = location; }

	double			percent() const { return m_percent; }
	void			setPercent(double percent) { m_percent = percent; }

	double			nominal(int limitType) const;
	QString			nominalStr(int limitType) const;
	void			setNominal(int limitType, double value);

	double			measure(int limitType) const;
	QString			measureStr(int limitType) const;
	void			setMeasure(int limitType, double value);

	double			lowLimit(int limitType) const;
	void			setLowLimit(int limitType, double lowLimit);

	double			highLimit(int limitType) const;
	void			setHighLimit(int limitType, double highLimit);

	QString			unit(int limitType) const;
	void			setUnit(int limitType, QString unit);

	int				limitPrecision(int limitType) const;
	void			setLimitPrecision(int limitType, int precision);

	QString			limitStr(int limitType) const;

	double			error(int limitType, int errotType) const;
	QString			errorStr(int limitType) const;
	void			setError(int limitType, int errotType, double value);

	double			errorLimit(int limitType, int errotType) const;
	QString			errorLimitStr(int limitType) const;
	void			setErrorLimit(int limitType, int errotType, double value);

	int				measureCount() const { return m_measureCount; }
	void			setMeasureCount(int count) { m_measureCount = count; }

	double			measureItemArray(int limitType, int index) const;
	QString			measureItemStr(int limitType, int index) const;
	void			setMeasureItemArray(int limitType, int index, double value);

	int				additionalParamCount() const { return m_additionalParamCount; }
	void			setAdditionalParamCount(int count) { m_additionalParamCount = count; }

	double			additionalParam(int paramType) const;
	void			setAdditionalParam(int paramType, double value);

	void			updateMeasureArray(int limitType, Measurement* pMeasurement);
	void			updateAdditionalParam(Measurement* pMeasurement);

	LinearityMeasurement& operator=(const LinearityMeasurement& from);
};

// ==============================================================================================

class ComparatorMeasurement : public Measurement
{

public:

	ComparatorMeasurement();
	explicit ComparatorMeasurement(Calibrator* pCalibrator);
	virtual ~ComparatorMeasurement();

private:

	QString			m_appSignalID;
	QString			m_customAppSignalID;
	QString			m_caption;

public:

	QString			appSignalID() const { return m_appSignalID; }
	void			setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

	QString			customAppSignalID() const { return m_customAppSignalID; }
	void			setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

	QString			caption() const { return m_caption; }
	void			setCaption(const QString& name) { m_caption = name; }

	void			updateHysteresis(Measurement* pMeasurement);
};

// ==============================================================================================

class MeasureBase : public QObject
{
	Q_OBJECT

public:

	explicit MeasureBase(QObject *parent = 0);
	virtual ~MeasureBase();

private:

	int							m_measureType = MEASURE_TYPE_UNKNOWN;

	mutable QMutex				m_measureMutex;
	QVector<Measurement*>		m_measureList;

public:

	int							count() const;
	void						clear(bool removeData = true);

	int							load(int measureType);

	int							append(Measurement* pMeasurement);
	Measurement*				measurement(int index) const;
	bool						remove(int index, bool removeData = true);

	Metrology::SignalStatistic	statistic(const Hash& signalHash);
};

// ==============================================================================================

extern MeasureBase theMeasureBase;

// ==============================================================================================


#endif // MEASUREBASE_H
