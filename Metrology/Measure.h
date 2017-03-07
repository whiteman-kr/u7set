#pragma once

#include <assert.h>
#include <QDateTime>

#include "../lib/Hash.h"

#include "Calibrator.h"
#include "SignalBase.h"

// ==============================================================================================

const char* const MeasureType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of linearity"),
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of comparators"),
};

const int   MEASURE_TYPE_COUNT = sizeof(MeasureType)/sizeof(MeasureType[0]);

const int   MEASURE_TYPE_UNKNOWN            = -1,
            MEASURE_TYPE_LINEARITY          = 0,
            MEASURE_TYPE_COMPARATOR         = 1;

// ----------------------------------------------------------------------------------------------

#define		ERR_MEASURE_TYPE(type) (type < 0 || type >= MEASURE_TYPE_COUNT)
#define		TEST_MEASURE_TYPE(type)				if (ERR_MEASURE_TYPE(type)) { return; }
#define		TEST_MEASURE_TYPE1(type, retVal)	if (ERR_MEASURE_TYPE(type)) { return retVal; }

// ----------------------------------------------------------------------------------------------

const char* const MeasureFileName[MEASURE_TYPE_COUNT] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Linearity"),
            QT_TRANSLATE_NOOP("Measure.h", "Comparators"),
};

// ==============================================================================================

const int   MeasureTimeout[] =
{
            0, 1, 2, 3, 5, 10, 15, 20, 30, 45, 60,
};

const int   MeasureTimeoutCount = sizeof(MeasureTimeout)/sizeof(MeasureTimeout[0]);


// ==============================================================================================

const char* const	MeasureKind[] =
{
			QT_TRANSLATE_NOOP("Measure.h", " in one rack"),
			QT_TRANSLATE_NOOP("Measure.h", " in several racks"),
};

const int   MEASURE_KIND_COUNT = sizeof(MeasureKind)/sizeof(MeasureKind[0]);

const int	MEASURE_KIND_UNKNOWN			= -1,
            MEASURE_KIND_ONE				= 0,
            MEASURE_KIND_MULTI				= 1;

// ----------------------------------------------------------------------------------------------

#define     ERR_MEASURE_KIND(kind) (kind < 0 || kind >= MEASURE_KIND_COUNT)
#define		TEST_MEASURE_KIND(kind)				if (ERR_MEASURE_KIND(kind)) { return; }
#define     TEST_MEASURE_KIND1(kind, retVal)	if (ERR_MEASURE_KIND(kind)) { return retVal; }

// ==============================================================================================

const char* const ValueType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "InElectric"),
            QT_TRANSLATE_NOOP("Measure.h", "Physical"),
            QT_TRANSLATE_NOOP("Measure.h", "OutElectric"),
};

const int	VALUE_TYPE_COUNT		= sizeof(ValueType)/sizeof(ValueType[0]);

const int   VALUE_TYPE_UNKNOWN		= -1,
            VALUE_TYPE_IN_ELECTRIC  = 0,
            VALUE_TYPE_PHYSICAL     = 1,
            VALUE_TYPE_OUT_ELECTRIC = 2;

// ----------------------------------------------------------------------------------------------

#define     ERR_VALUE_TYPE(type) (type < 0 || type >= VALUE_TYPE_COUNT)
#define		TEST_VALUE_TYPE(type)			if (ERR_VALUE_TYPE(type)) { return; }
#define     TEST_VALUE_TYPE1(type, retVal)	if (ERR_VALUE_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const ErrorType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Absolute"),
            QT_TRANSLATE_NOOP("Measure.h", "Reduce"),
};

const int	MEASURE_ERROR_TYPE_COUNT		= sizeof(ErrorType)/sizeof(ErrorType[0]);

const int   MEASURE_ERROR_TYPE_UNKNOWN		= -1,
            MEASURE_ERROR_TYPE_ABSOLUTE		= 0,
            MEASURE_ERROR_TYPE_REDUCE		= 1;

// ----------------------------------------------------------------------------------------------

#define     ERR_MEASURE_ERROR_TYPE(type) (type < 0 || type >= MEASURE_ERROR_TYPE_COUNT)
#define		TEST_MEASURE_ERROR_TYPE(type)			if (ERR_MEASURE_ERROR_TYPE(type)) { return; }
#define     TEST_MEASURE_ERROR_TYPE1(type, retVal)	if (ERR_MEASURE_ERROR_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const MeasureAdditionalParam[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Measure value max"),
            QT_TRANSLATE_NOOP("Measure.h", "System error"),
            QT_TRANSLATE_NOOP("Measure.h", "MSE"),
            QT_TRANSLATE_NOOP("Measure.h", "Low High border"),
};

const int	MEASURE_ADDITIONAL_PARAM_COUNT              = sizeof(MeasureAdditionalParam)/sizeof(MeasureAdditionalParam[0]);

const int   MEASURE_ADDITIONAL_PARAM_UNKNOWN            = -1,
            MEASURE_ADDITIONAL_PARAM_MAX_VALUE          = 0,
            MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR       = 1,
            MEASURE_ADDITIONAL_PARAM_MSE                = 2,
            MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER    = 3;

            // maximum 16 items ( 0 .. 15)
            // now used 4 ( 0 .. 3 )

// ==============================================================================================

const int   MAX_MEASUREMENT_IN_POINT    = 20;
// ==============================================================================================

const int   INVALID_VALUE               = 0xFFFF;

// ==============================================================================================

#define     MEASURE_TIME_FORMAT         "dd-MM-yyyy hh:mm:ss"

// ==============================================================================================

class Measurement
{

public:

    explicit        Measurement(int measureType = MEASURE_TYPE_UNKNOWN);
                    ~Measurement();

private:

    int             m_measureType = MEASURE_TYPE_UNKNOWN;           // measure type
    Hash            m_signalHash = 0;                               // hash calced from AppSignalID by function calcHash()

    int             m_measureID = -1;                               // primary key of record in SQL table
    bool            m_filter = false;                               // filter for record, if "true" - hide record

    QDateTime       m_measureTime;                                  // measure time
    int             m_reportType = -1;                              // report type

public:

    void            virtual clear() {}

    int             measureType() const { return m_measureType; }
    void            setMeasureType(int type) { m_measureType = type; }

    Hash            signalHash() const { return m_signalHash; }
    void            setSignalHash(const Hash& hash) { m_signalHash = hash; }
    void            setSignalHash(const QString& id) { m_signalHash = calcHash(id); }

    int             measureID() const { return m_measureID; }
    void            setMeasureID(int id) { m_measureID = id; }

    bool            filter() const { return m_filter; }
    void            setFilter(bool filter) { m_filter = filter; }

    QDateTime       measureTime() const { return m_measureTime; }
    void            setMeasureTime(const QDateTime& time) { m_measureTime = time; }

    int             reportType() const { return m_reportType; }
    void            setReportType(int type) { m_reportType = type; }

    Measurement*    at(int index);

    Measurement&    operator=(Measurement& from);
};

// ==============================================================================================

class LinearityMeasurement : public Measurement
{

public:

                    LinearityMeasurement();
                    LinearityMeasurement(const MeasureParam& measureParam);
                    ~LinearityMeasurement();

private:

    QString         m_appSignalID;
    QString         m_customAppSignalID;
    QString         m_caption;

	Metrology::SignalLocation  m_location;

    double          m_percent = 0;

    double          m_nominal[VALUE_TYPE_COUNT];
    double          m_measure[VALUE_TYPE_COUNT];

    bool            m_hasLimit[VALUE_TYPE_COUNT];
    double          m_lowLimit[VALUE_TYPE_COUNT];
    double          m_highLimit[VALUE_TYPE_COUNT];
    QString         m_unit[VALUE_TYPE_COUNT];
    int             m_limitPrecision[VALUE_TYPE_COUNT];

    double          m_adjustment = 0;

    double          m_error[VALUE_TYPE_COUNT][MEASURE_ERROR_TYPE_COUNT];
    double          m_errorLimit[VALUE_TYPE_COUNT][MEASURE_ERROR_TYPE_COUNT];

    int             m_measureCount = 0;
    double          m_measureArray[VALUE_TYPE_COUNT][MAX_MEASUREMENT_IN_POINT];

    int             m_additionalParamCount = 0;
    double          m_additionalParam[MEASURE_ADDITIONAL_PARAM_COUNT];

public:

    void            virtual clear();

    void            set1(const MeasureParam& measureParam);
    void            set2(const MeasureParam& measureParam);
    void            set3(const MeasureParam& measureParam);

    void            calcError();
    void            calcAdditionalParam(int limitType);

    QString         appSignalID() const { return m_appSignalID; }
    void            setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID;  setSignalHash(m_appSignalID); }

    QString         customAppSignalID() const { return m_customAppSignalID; }
    void            setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

    QString         signalID(int type) const;

    QString         caption() const { return m_caption; }
    void            setCaption(const QString& caption) { m_caption = caption; }

	Metrology::SignalLocation& location() { return m_location; }
	void            setLocation(const Metrology::SignalLocation& location) { m_location = location; }

    double          percent() const { return m_percent; }
    void            setPercent(double percent) { m_percent = percent; }

    double          nominal(int limitType) const;
    QString         nominalStr(int limitType) const;
    void            setNominal(int limitType, double value);

    double          measure(int limitType) const;
    QString         measureStr(int limitType) const;
    void            setMeasure(int limitType, double value);

    bool            hasLimit(int limitType);
    void            setHasLimit(int limitType, bool hasLimit);

    double          lowLimit(int limitType) const;
    void            setLowLimit(int limitType, double lowLimit);

    double          highLimit(int limitType) const;
    void            setHighLimit(int limitType, double highLimit);

    QString         unit(int limitType) const;
    void            setUnit(int limitType, QString unit);

    int             limitPrecision(int limitType) const;
    void            setLimitPrecision(int limitType, int precision);

    QString         limitStr(int limitType) const;

    double          error(int limitType, int errotType) const;
    QString         errorStr(int limitType) const;
    void            setError(int limitType, int errotType, double value);

    double          errorLimit(int limitType, int errotType) const;
    QString         errorLimitStr(int limitType) const;
    void            setErrorLimit(int limitType, int errotType, double value);

    int             measureCount() const { return m_measureCount; }
    void            setMeasureCount(int count) { m_measureCount = count; }

    double          measureItemArray(int limitType, int index) const;
    QString         measureItemStr(int limitType, int index) const;
    void            setMeasureItemArray(int limitType, int index, double value);

    int             additionalParamCount() const { return m_additionalParamCount; }
    void            setAdditionalParamCount(int count) { m_additionalParamCount = count; }

    double          additionalParam(int paramType) const;
    void            setAdditionalParam(int paramType, double value);

    void            updateMeasureArray(int limitType, Measurement* pMeasurement);
    void            updateAdditionalParam(Measurement* pMeasurement);

    LinearityMeasurement& operator=(const LinearityMeasurement& from);
};

// ==============================================================================================

class ComparatorMeasurement : public Measurement
{

public:

                    ComparatorMeasurement();
    explicit        ComparatorMeasurement(Calibrator* pCalibrator);
                    ~ComparatorMeasurement();

private:

    QString         m_appSignalID;
    QString         m_customAppSignalID;
    QString         m_caption;

public:

    QString         appSignalID() const { return m_appSignalID; }
    void            setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

    QString         customAppSignalID() const { return m_customAppSignalID; }
    void            setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

    QString         caption() const { return m_caption; }
    void            setCaption(const QString& name) { m_caption = name; }

	void			updateHysteresis(Measurement* pMeasurement);
};

// ==============================================================================================

#pragma pack(push, 1)

// ==============================================================================================

// struct

// ==============================================================================================

#pragma pack(pop)

// ==============================================================================================



