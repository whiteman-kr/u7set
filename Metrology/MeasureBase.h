#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include "../lib/Hash.h"
#include "../lib/MetrologySignal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const MeasureType[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Measurements of linearity"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Measurements of comparators"),
};

const int	MEASURE_TYPE_COUNT = sizeof(MeasureType)/sizeof(MeasureType[0]);

const int	MEASURE_TYPE_UNDEFINED	= -1,
			MEASURE_TYPE_LINEARITY	= 0,
			MEASURE_TYPE_COMPARATOR	= 1;

// ----------------------------------------------------------------------------------------------

const char* const MeasureFileName[MEASURE_TYPE_COUNT] =
{
			"Linearity",
			"Comparators",
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
			QT_TRANSLATE_NOOP("MeasureBase.h", " Single channel"),
			QT_TRANSLATE_NOOP("MeasureBase.h", " Single module"),
			QT_TRANSLATE_NOOP("MeasureBase.h", " Multi channel"),
};

const int	MEASURE_KIND_COUNT		= sizeof(MeasureKind)/sizeof(MeasureKind[0]);

const int	MEASURE_KIND_UNDEFINED	= -1,
			MEASURE_KIND_ONE_RACK	= 0,
			MEASURE_KIND_ONE_MODULE	= 1,
			MEASURE_KIND_MULTI_RACK	= 2;

// ==============================================================================================

const char* const MeasureLimitType[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Electric"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Engineering"),
};

const int	MEASURE_LIMIT_TYPE_COUNT		= sizeof(MeasureLimitType)/sizeof(MeasureLimitType[0]);

const int	MEASURE_LIMIT_TYPE_UNDEFINED	= -1,
			MEASURE_LIMIT_TYPE_ELECTRIC		= 0,
			MEASURE_LIMIT_TYPE_ENGINEER		= 1;

// ==============================================================================================

const char* const MeasureErrorType[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Absolute"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Reduced"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Relative"),
};

const int	MEASURE_ERROR_TYPE_COUNT	= sizeof(MeasureErrorType)/sizeof(MeasureErrorType[0]);

const int	MEASURE_ERROR_TYPE_UNDEFINED	= -1,
			MEASURE_ERROR_TYPE_ABSOLUTE		= 0,
			MEASURE_ERROR_TYPE_REDUCE		= 1,
			MEASURE_ERROR_TYPE_RELATIVE		= 2;

// ==============================================================================================

const char* const ErrorResult[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Ok"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Failed"),
};

const int	MEASURE_ERROR_RESULT_COUNT	= sizeof(ErrorResult)/sizeof(ErrorResult[0]);

const int	MEASURE_ERROR_RESULT_UNDEFINED		= -1,
			MEASURE_ERROR_RESULT_OK				= 0,
			MEASURE_ERROR_RESULT_FAILED			= 1;

// ==============================================================================================

const char* const MeasureAdditionalParam[] =
{
			QT_TRANSLATE_NOOP("MeasureBase.h", "Measure value max"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "System deviation"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Standard deviation"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Low High border"),
			QT_TRANSLATE_NOOP("MeasureBase.h", "Uncertainty"),
};

const int	MEASURE_ADDITIONAL_PARAM_COUNT				= sizeof(MeasureAdditionalParam)/sizeof(MeasureAdditionalParam[0]);

const int	MEASURE_ADDITIONAL_PARAM_UNDEFINED			= -1,
			MEASURE_ADDITIONAL_PARAM_MAX_VALUE			= 0,
			MEASURE_ADDITIONAL_PARAM_SYSTEM_DEVIATION	= 1,
			MEASURE_ADDITIONAL_PARAM_SD					= 2,
			MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER	= 3,
			MEASURE_ADDITIONAL_PARAM_UNCERTAINTY		= 4;

			// maximum 16 items (0 .. 15)
			// now used 5 (1 .. 5)

// ==============================================================================================

const int	MAX_MEASUREMENT_IN_POINT	= 20;

// ==============================================================================================

#define	 MEASURE_TIME_FORMAT			"dd-MM-yyyy hh:mm:ss"

// ==============================================================================================

class Measurement
{

public:

	explicit Measurement(int measureType = MEASURE_TYPE_UNDEFINED);
	virtual ~Measurement();

public:

	void			virtual clear();

	int				measureType() const { return m_measureType; }
	void			setMeasureType(int type) { m_measureType = type; }

	Hash			signalHash() const { return m_signalHash; }
	void			setSignalHash(const Hash& hash) { m_signalHash = hash; }
	void			setSignalHash(const QString& id) { m_signalHash = calcHash(id); }

	int				measureID() const { return m_measureID; }
	void			setMeasureID(int id) { m_measureID = id; }

	bool			filter() const { return m_filter; }
	void			setFilter(bool filter) { m_filter = filter; }

	bool			isSignalValid() const { return m_signalValid; }
	void			setSignalValid(bool valid) { m_signalValid = valid; }

	QString			connectionAppSignalID() const;
	void			setConnectionAppSignalID(const QString& appSignalID) { m_connectionAppSignalID = appSignalID; }

	Metrology::ConnectionType connectionType() const { return m_connectionType; }
	int				connectionTypeInt() const { return TO_INT(m_connectionType); }
	QString			connectionTypeStr() const;
	void			setConnectionType(Metrology::ConnectionType type) { m_connectionType = type; }
	void			setConnectionType(int type) { m_connectionType =  static_cast<Metrology::ConnectionType>(type); }

	QString			appSignalID() const { return m_appSignalID; }
	void			setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; setSignalHash(m_appSignalID); }

	QString			customAppSignalID() const { return m_customAppSignalID; }
	void			setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

	QString			equipmentID() const { return m_equipmentID; }
	void			setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString			caption() const { return m_caption; }
	void			setCaption(const QString& caption) { m_caption = caption; }

	Metrology::SignalLocation& location() { return m_location; }
	void			setLocation(const Metrology::SignalLocation& location) { m_location = location; }

	int				calibratorPrecision() const { return m_calibratorPrecision; }
	void			setCalibratorPrecision(int precision) { m_calibratorPrecision = precision; }

	double			nominal(int limitType) const;
	QString			nominalStr(int limitType) const;
	void			setNominal(int limitType, double value);

	double			measure(int limitType) const;
	QString			measureStr(int limitType) const;
	void			setMeasure(int limitType, double value);

	void			setLimits(const IoSignalParam &ioParam);

	double			lowLimit(int limitType) const;
	void			setLowLimit(int limitType, double lowLimit);

	double			highLimit(int limitType) const;
	void			setHighLimit(int limitType, double highLimit);

	QString			unit(int limitType) const;
	void			setUnit(int limitType, QString unit);

	int				limitPrecision(int limitType) const;
	void			setLimitPrecision(int limitType, int precision);

	QString			limitStr(int limitType) const;

	void			calcError();

	double			error(int limitType, int errotType) const;
	QString			errorStr() const;
	void			setError(int limitType, int errotType, double value);

	double			errorLimit(int limitType, int errotType) const;
	QString			errorLimitStr() const;
	void			setErrorLimit(int limitType, int errotType, double value);

	int				errorResult() const;
	QString			errorResultStr() const;

	QDateTime		measureTime() const { return m_measureTime; }
	QString			measureTimeStr() const;
	void			setMeasureTime(const QDateTime& time) { m_measureTime = time; }

	QString			calibrator() const { return m_calibrator; }
	void			setCalibrator(const QString& calibrator) { m_calibrator = calibrator; }

	void			setCalibratorData(const IoSignalParam &ioParam);

	int				reportType() const { return m_reportType; }
	void			setReportType(int type) { m_reportType = type; }

	bool			foundInStatistics() const { return m_foundInStatistics; }
	void			setFoundInStatistics(int signalIsfound) { m_foundInStatistics = signalIsfound; }

	Measurement*	at(int index);

	Measurement&	operator=(Measurement& from);

private:

	int				m_measureType = MEASURE_TYPE_UNDEFINED;					// measure type
	Hash			m_signalHash = UNDEFINED_HASH;							// hash calced from AppSignalID by function calcHash()

	int				m_measureID = -1;										// primary key of record in SQL table
	bool			m_filter = false;										// filter for record, if "true" - hide record

	bool			m_signalValid = true;									// signal is valid during the measurement

	QString			m_connectionAppSignalID;
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

	QString			m_appSignalID;
	QString			m_customAppSignalID;
	QString			m_equipmentID;
	QString			m_caption;

	Metrology::SignalLocation m_location;

	int				m_calibratorPrecision = DEFAULT_ECLECTRIC_UNIT_PRECESION;	// precision of electric range of calibrator

	double			m_nominal[MEASURE_LIMIT_TYPE_COUNT];
	double			m_measure[MEASURE_LIMIT_TYPE_COUNT];

	double			m_lowLimit[MEASURE_LIMIT_TYPE_COUNT];
	double			m_highLimit[MEASURE_LIMIT_TYPE_COUNT];
	QString			m_unit[MEASURE_LIMIT_TYPE_COUNT];
	int				m_limitPrecision[MEASURE_LIMIT_TYPE_COUNT];

	double			m_adjustment = 0;

	double			m_error[MEASURE_LIMIT_TYPE_COUNT][MEASURE_ERROR_TYPE_COUNT];
	double			m_errorLimit[MEASURE_LIMIT_TYPE_COUNT][MEASURE_ERROR_TYPE_COUNT];

	QDateTime		m_measureTime;											// measure time
	QString			m_calibrator;											// calibrator name and calibrator SN
	int				m_reportType = -1;										// report type

	bool			m_foundInStatistics = true;								// after loading find signal in the statistics list
};

// ==============================================================================================

class LinearityMeasurement : public Measurement
{

public:

	LinearityMeasurement();
	LinearityMeasurement(const IoSignalParam& ioParam);
	virtual ~LinearityMeasurement();

public:

	void			virtual clear();

	void			fill_measure_input(const IoSignalParam& ioParam);
	void			fill_measure_internal(const IoSignalParam& ioParam);
	void			fill_measure_output(const IoSignalParam& ioParam);

	void			calcAdditionalParam(const IoSignalParam &ioParam);
	double			calcUcertainty(const IoSignalParam &ioParam, int limitType) const;

	double			percent() const { return m_percent; }
	void			setPercent(double percent) { m_percent = percent; }

	int				measureCount() const { return m_measureCount; }
	void			setMeasureCount(int count) { m_measureCount = count; }

	double			measureItemArray(int limitType, int index) const;
	QString			measureItemStr(int limitType, int index) const;
	void			setMeasureItemArray(int limitType, int index, double value);

	int				additionalParamCount() const { return m_additionalParamCount; }
	void			setAdditionalParamCount(int count) { m_additionalParamCount = count; }

	double			additionalParam(int limitType, int paramType) const;
	QString			additionalParamStr(int limitType, int paramType) const;
	void			setAdditionalParam(int limitType, int paramType, double value);

	void			updateMeasureArray(int limitType, Measurement* pMeasurement);
	void			updateAdditionalParam(int limitType, Measurement* pMeasurement);

	LinearityMeasurement& operator=(const LinearityMeasurement& from);

private:

	double			m_percent = 0;

	int				m_measureCount = 0;
	double			m_measureArray[MEASURE_LIMIT_TYPE_COUNT][MAX_MEASUREMENT_IN_POINT];

	int				m_additionalParamCount = 0;
	double			m_additionalParam[MEASURE_LIMIT_TYPE_COUNT][MEASURE_ADDITIONAL_PARAM_COUNT];
};

// ==============================================================================================

class ComparatorMeasurement : public Measurement
{

public:

	ComparatorMeasurement();
	explicit ComparatorMeasurement(const IoSignalParam& ioParam);
	virtual ~ComparatorMeasurement();

public:

	void			virtual clear();

	void			fill_measure_input(const IoSignalParam& ioParam);
	void			fill_measure_internal(const IoSignalParam &ioParam);

	QString			compareAppSignalID() const { return m_compareAppSignalID; }
	void			setCompareAppSignalID(const QString& appSignalID) { m_compareAppSignalID = appSignalID; }

	QString			outputAppSignalID() const { return m_outputAppSignalID; }
	void			setOutputAppSignalID(const QString& appSignalID) { m_outputAppSignalID = appSignalID; }

	Metrology::CmpValueType cmpValueType() const { return m_cmpValueType; }
	int				cmpValueTypeInt() const { return TO_INT(m_cmpValueType); }
	QString			cmpValueTypeStr() const;
	void			setCmpValueType(Metrology::CmpValueType type) { m_cmpValueType = type; }
	void			setCmpValueType(int type) { m_cmpValueType = static_cast<Metrology::CmpValueType>(type); }

	E::CmpType		cmpType() const { return m_cmpType; }
	int				cmpTypeInt() const { return TO_INT(m_cmpType); }
	QString			cmpTypeStr() const;
	void			setCmpType(Metrology::CmpValueType cmpValueType, E::CmpType cmpType);
	void			setCmpType(int cmpType) { m_cmpType = static_cast<E::CmpType>(cmpType); }

	ComparatorMeasurement& operator=(const ComparatorMeasurement& from);

private:

	QString			m_compareAppSignalID;
	QString			m_outputAppSignalID;

	Metrology::CmpValueType m_cmpValueType = Metrology::CmpValueType::NoCmpValueType;
	E::CmpType		m_cmpType = E::CmpType::Greate;
};

// ==============================================================================================

class MeasureBase : public QObject
{
	Q_OBJECT

public:

	explicit MeasureBase(QObject* parent = nullptr);
	virtual ~MeasureBase();

public:

	int						count() const;
	void					clear(bool removeData = true);

	int						load(int measureType);

	int						append(Measurement* pMeasurement);
	Measurement*			measurement(int index) const;
	bool					remove(int index, bool removeData = true);
	bool					remove(int measureType, const QVector<int>& keyList);

	void					updateStatisticsItem(int measureType, StatisticsItem& si);
	void					updateStatisticsBase(int measureType);
	void					updateStatisticsBase(int measureType, Hash signalHash);
	static void				markNotExistMeasuremetsFromStatistics(MeasureBase* pThis);

private:

	int						m_measureType = MEASURE_TYPE_UNDEFINED;

	mutable QMutex			m_measureMutex;
	QVector<Measurement*>	m_measureList;

signals:

	void					updatedMeasureBase(Hash signalHash);
	void					updateMeasureView();

public slots:

	void					signalBaseLoaded();

	void					appendToBase(Measurement* pMeasurement);
	void					removeFromBase(int measureType, const QVector<int>& keyList);
};

// ==============================================================================================

#endif // MEASUREBASE_H
