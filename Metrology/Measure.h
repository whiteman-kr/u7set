#pragma once

#include <assert.h>
#include <QDateTime>

#include "Calibrator.h"
#include "SignalBase.h"

#include "../lib/Hash.h"

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
            QT_TRANSLATE_NOOP("Measure.h", " in one channel"),
            QT_TRANSLATE_NOOP("Measure.h", " in all channels"),
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
            QT_TRANSLATE_NOOP("Measure.h", "Electric"),
            QT_TRANSLATE_NOOP("Measure.h", "Physical"),
            QT_TRANSLATE_NOOP("Measure.h", "Output"),
};

const int	VALUE_TYPE_COUNT		= sizeof(ValueType)/sizeof(ValueType[0]);

const int   VALUE_TYPE_UNKNOWN		= -1,
            VALUE_TYPE_ELECTRIC		= 0,
            VALUE_TYPE_PHYSICAL		= 1,
            VALUE_TYPE_OUTPUT		= 2;

// ----------------------------------------------------------------------------------------------

#define     ERR_VALUE_TYPE(type) (type < 0 || type >= VALUE_TYPE_COUNT)
#define		TEST_VALUE_TYPE(type)			if (ERR_VALUE_TYPE(type)) { return; }
#define     TEST_VALUE_TYPE1(type, retVal)	if (ERR_VALUE_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const ErrorType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Absolute"),
            QT_TRANSLATE_NOOP("Measure.h", "Reduce"),
            QT_TRANSLATE_NOOP("Measure.h", "Relative"),
};

const int	ERROR_TYPE_COUNT		= sizeof(ErrorType)/sizeof(ErrorType[0]);

const int   ERROR_TYPE_UNKNOWN		= -1,
            ERROR_TYPE_ABSOLUTE		= 0,
            ERROR_TYPE_REDUCE		= 1,
            ERROR_TYPE_RELATIVE		= 2;

// ----------------------------------------------------------------------------------------------

#define     ERR_ERROR_TYPE(type) (type < 0 || type >= ERROR_TYPE_COUNT)
#define		TEST_ERROR_TYPE(type)			if (ERR_ERROR_TYPE(type)) { return; }
#define     TEST_ERROR_TYPE1(type, retVal)	if (ERR_ERROR_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const AdditionalValue[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Measure value max"),
            QT_TRANSLATE_NOOP("Measure.h", "System error"),
            QT_TRANSLATE_NOOP("Measure.h", "MSE"),
            QT_TRANSLATE_NOOP("Measure.h", "Low High border"),
};

const int	ADDITIONAL_VALUE_COUNT              = sizeof(AdditionalValue)/sizeof(AdditionalValue[0]);

const int   ADDITIONAL_VALUE_UNKNOWN            = -1,
            ADDITIONAL_VALUE_MEASURE_MAX        = 0,
            ADDITIONAL_VALUE_SYSTEM_ERROR       = 1,
            ADDITIONAL_VALUE_MSE                = 2,
            ADDITIONAL_VALUE_LOW_HIGH_BORDER    = 3;

            // maximum 16 items ( 0 .. 15)
            // now used 4 ( 0 .. 3 )

// ==============================================================================================

const int   MAX_MEASUREMENT_IN_POINT    = 20;

// ==============================================================================================

const int   INVALID_VALUE           = 0xFFFF;

// ==============================================================================================

#define     MEASURE_TIME_FORMAT     "dd-MM-yyyy hh:mm:ss"

// ==============================================================================================

class Measurement
{

public:

    explicit        Measurement(const int measureType = MEASURE_TYPE_UNKNOWN);
                    ~Measurement();

private:

    int             m_measureType = MEASURE_TYPE_UNKNOWN;           // measure type
    Hash            m_signalHash = 0;                               // hash calced from AppSignalID by function calcHash()

    int             m_measureID = -1;                               // primary key of record in SQL table
    bool            m_filter = false;                               // filter for record, if "true" - hide record

    QDateTime       m_measureTime;                                  // measure time
    int             m_reportType = -1;                              // report type

public:

    int             measureType() const { return m_measureType; }
    void            setMeasureType(const int type) { m_measureType = type; }

    Hash            signalHash() const { return m_signalHash; }
    void            setSignalHash(const Hash& hash) { m_signalHash = hash; }
    void            setSignalHash(const QString& id) { m_signalHash = calcHash(id); }

    int             measureID() const { return m_measureID; }
    void            setMeasureID(const int id) { m_measureID = id; }

    bool            filter() const { return m_filter; }
    void            setFilter(const bool filter) { m_filter = filter; }

    QDateTime       measureTime() const { return m_measureTime; }
    void            setMeasureTime(const QDateTime& time) { m_measureTime = time; }

    int             reportType() const { return m_reportType; }
    void            setReportType(const int type) { m_reportType = type; }

    Measurement*    at(const int index);

    Measurement&    operator=(Measurement& from);
};

// ==============================================================================================

class LinearityMeasurement : public Measurement
{

public:

                    LinearityMeasurement();
                    LinearityMeasurement(const Calibrator* pCalibrator, const MeasureSignalParam& param);
                    ~LinearityMeasurement();

private:

    QString         m_appSignalID;
    QString         m_customAppSignalID;
    QString         m_caption;

    DevicePosition  m_position;

    double          m_nominal[VALUE_TYPE_COUNT];

    double          m_percent = 0;

    double          m_measure[VALUE_TYPE_COUNT];

    int             m_measureArrayCount = 0;
    double          m_measureArray[VALUE_TYPE_COUNT][MAX_MEASUREMENT_IN_POINT];

    double          m_lowLimit[VALUE_TYPE_COUNT];
    double          m_highLimit[VALUE_TYPE_COUNT];
    QString         m_unit[VALUE_TYPE_COUNT];

    int             m_valuePrecision[VALUE_TYPE_COUNT];

    bool            m_hasOutput = false;
    double          m_adjustment = 0;

    double          m_errorInput[ERROR_TYPE_COUNT];
    double          m_errorOutput[ERROR_TYPE_COUNT];
    double          m_errorLimit[ERROR_TYPE_COUNT];

    int             m_errorPrecision[ERROR_TYPE_COUNT];

    int             m_additionalValueCount = 0;
    double          m_additionalValue[ADDITIONAL_VALUE_COUNT];

public:

    QString         appSignalID() const { return m_appSignalID; }
    void            setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID;  setSignalHash(m_appSignalID); }

    QString         customAppSignalID() const { return m_customAppSignalID; }
    void            setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

    QString         name() const { return m_caption; }
    void            setCaption(const QString& caption) { m_caption = caption; }

    DevicePosition& position() { return m_position; }
    void            setPosition(const DevicePosition& pos) { m_position = pos; }

    double          nominal(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_nominal[type]; }
    void            setNominal(const int type, const double value) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_nominal[type] = value; }

    QString         nominalString(const int type) const;

    double          percent() const { return m_percent; }
    void            setPercent(const double percent) { m_percent = percent; }

    double          measure(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_measure[type]; }
    void            setMeasure(const int type, const double value) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_measure[type] = value; }

    QString         measureString(const int type) const;

    int             measureArrayCount() const { return m_measureArrayCount; }
    void            setMeasureArrayCount(const int count) { m_measureArrayCount = count; }

    double          measureItemArray(const int type, const int index) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT) { assert(0); return 0; } return m_measureArray[type][index]; }
    void            setMeasureItemArray(const int type, const int index, const double value) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } if (index < 0 || index >= MAX_MEASUREMENT_IN_POINT) { assert(0); return; } m_measureArray[type][index] = value; }

    QString         measureItemString(const int type, const int index) const;

    double          lowLimit(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_lowLimit[type]; }
    void            setLowLimit(const int type, const double lowLimit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_lowLimit[type] = lowLimit; }

    double          highLimit(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_highLimit[type]; }
    void            setHighLimit(const int type, const double highLimit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_highLimit[type] = highLimit; }

    QString         unit(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return QString(); } return m_unit[type]; }
    void            setUnit(const int type, QString unit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_unit[type] = unit; }

    QString         limitString(const int type) const;

    int             valuePrecision(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_valuePrecision[type]; }
    void            setValuePrecision(const int type, const int precision) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_valuePrecision[type] = precision; }

    bool            hasOutput() { return m_hasOutput; }
    void            setHasOutput(bool hasOutput) { m_hasOutput = hasOutput; }

    double          adjustment() const { return m_adjustment; }
    void            setAdjustment(const double adjustment) { m_adjustment = adjustment; }

    double          errorInput(const int type) const { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return 0; } return m_errorInput[type]; }
    void            setErrorInput(const int type, const double value) { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return; } m_errorInput[type] = value; }

    double          errorOutput(const int type) const { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return 0; } return m_errorOutput[type]; }
    void            setErrorOutput(const int type, const double value) { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return; } m_errorOutput[type] = value; }

    double          errorLimit(const int type) const { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return 0; } return m_errorLimit[type]; }
    void            setErrorLimit(const int type, const double value) { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return; } m_errorLimit[type] = value; }

    int             errorPrecision(const int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_errorPrecision[type]; }
    void            setErrorPrecision(const int type, const int precision) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_errorPrecision[type] = precision; }

    int             additionalValueCount() const { return m_additionalValueCount; }
    void            setAdditionalValueCount(const int count) { m_additionalValueCount = count; }

    double          additionalValue(const int type) const { if (type < 0 || type >= ADDITIONAL_VALUE_COUNT) { assert(0); return 0; } return m_additionalValue[type]; }
    void            setAdditionalValue(const int type, const double value) { if (type < 0 || type >= ADDITIONAL_VALUE_COUNT) { assert(0); return; } m_additionalValue[type] = value; }

    void            updateMeasureArray(const int type, Measurement* pMeasurement);
    void            updateAdditionalValue(Measurement* pMeasurement);

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

    DevicePosition  m_position;

public:

    QString         appSignalID() const { return m_appSignalID; }
    void            setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

    QString         customAppSignalID() const { return m_customAppSignalID; }
    void            setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

    QString         caption() const { return m_caption; }
    void            setCaption(const QString& name) { m_caption = name; }

    DevicePosition& position() { return m_position; }

    void updateHysteresis(Measurement* pMeasurement);
};

// ==============================================================================================

#pragma pack(push, 1)

// ==============================================================================================

// struct

// ==============================================================================================

#pragma pack(pop)

// ==============================================================================================



