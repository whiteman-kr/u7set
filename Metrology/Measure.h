#pragma once

#include <assert.h>
#include <QtGlobal>
#include <QDateTime>

// ==============================================================================================

const char* const MeasureType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of linearity"),
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of comparators"),
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of complex comparators"),
};


const int   MEASURE_TYPE_COUNT = sizeof(MeasureType)/sizeof(char*);

const int   MEASURE_TYPE_UNKNOWN            = -1,
            MEASURE_TYPE_LINEARITY          = 0,
            MEASURE_TYPE_COMPARATOR         = 1,
            MEASURE_TYPE_COMPLEX_COMPARATOR	= 2;

// ----------------------------------------------------------------------------------------------

#define		ERR_MEASURE_TYPE(type) (type < 0 || type >= MEASURE_TYPE_COUNT)
#define		TEST_MEASURE_TYPE(type)				if (ERR_MEASURE_TYPE(type)) { return; }
#define		TEST_MEASURE_TYPE1(type, retVal)	if (ERR_MEASURE_TYPE(type)) { return retVal; }

// ----------------------------------------------------------------------------------------------

const char* const MeasureFileName[MEASURE_TYPE_COUNT] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Linearity"),
            QT_TRANSLATE_NOOP("Measure.h", "Comparators"),
            QT_TRANSLATE_NOOP("Measure.h", "ComplexComparators"),
};

// ==============================================================================================

const int   MeasureTimeout[] =
{
            0, 1, 2, 3, 5, 10, 15, 20, 30, 45, 60,
};

const int   MeasureTimeoutCount = sizeof(MeasureTimeout)/sizeof(int);


// ==============================================================================================

const char* const	MeasureKind[] =
{
            QT_TRANSLATE_NOOP("Measure.h", " in one channel"),
            QT_TRANSLATE_NOOP("Measure.h", " in all channels"),
};

const int   MEASURE_KIND_COUNT = sizeof(MeasureKind)/sizeof(char*);

const int	MEASURE_KIND_UNKNOWN			= -1,
            MEASURE_KIND_ONE				= 0,
            MEASURE_KIND_MULTI				= 1;

// ----------------------------------------------------------------------------------------------

#define     ERR_MEASURE_KIND(kind) (kind < 0 || kind >= MEASURE_KIND_COUNT)
#define		TEST_MEASURE_KIND(kind)				if (ERR_MEASURE_KIND(kind)) { return; }
#define     TEST_MEASURE_KIND1(kind, retVal)	if (ERR_MEASURE_KIND(kind)) { return retVal; }

// ==============================================================================================

const char* const OutputSignalKind[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Input"),
            QT_TRANSLATE_NOOP("Measure.h", "Output"),
            QT_TRANSLATE_NOOP("Measure.h", "Correct"),
};

const int   OUTPUT_SIGNAL_KIND_COUNT = sizeof(OutputSignalKind)/sizeof(char*);

const int   OUTPUT_SIGNAL_KIND_UNKNOWN			= -1,
            OUTPUT_SIGNAL_KIND_INPUT			= 0,
            OUTPUT_SIGNAL_KIND_OUTPUT			= 1,
            OUTPUT_SIGNAL_KIND_CORRECT			= 2;

// ----------------------------------------------------------------------------------------------

#define     ERR_OUTPUT_SIGNAL_KIND(kind) (kind < 0 || kind >= OUTPUT_SIGNAL_KIND_COUNT)
#define     TEST_OUTPUT_SIGNAL_KIND(kind)			if (ERR_OUTPUT_SIGNAL_KIND(kind)) { return; }
#define     TEST_OUTPUT_SIGNAL_KIND1(kind, retVal)	if (ERR_OUTPUT_SIGNAL_KIND(kind)) { return retVal; }

// ==============================================================================================

const char* const OutputSignalType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Don't used"),
            QT_TRANSLATE_NOOP("Measure.h", "Correction"),
            QT_TRANSLATE_NOOP("Measure.h", "Signal MPS"),
            QT_TRANSLATE_NOOP("Measure.h", "P → T"),
            QT_TRANSLATE_NOOP("Measure.h", "T → P"),
};

const int   OUTPUT_SIGNAL_TYPE_COUNT = sizeof(OutputSignalType)/sizeof(char*);

const int   OUTPUT_SIGNAL_TYPE_UNKNOWN			= -1,
            OUTPUT_SIGNAL_TYPE_DONT_USED		= 0,
            OUTPUT_SIGNAL_TYPE_CORRECTION		= 1,
            OUTPUT_SIGNAL_TYPE_MPS				= 2,
            OUTPUT_SIGNAL_TYPE_PT				= 3,
            OUTPUT_SIGNAL_TYPE_TP				= 4;

// ----------------------------------------------------------------------------------------------

#define     ERR_OUTPUT_SIGNAL_TYPE(type) (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
#define		TEST_OUTPUT_SIGNAL_TYPE(type)			if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return; }
#define     TEST_OUTPUT_SIGNAL_TYPE1(type, retVal)	if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return retVal; }


// ==============================================================================================

const char* const CorrectSignalType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Addition (+)"),
            QT_TRANSLATE_NOOP("Measure.h", "Subtraction (-)"),
};

const int   CORRECT_SIGNAL_TYPE_COUNT = sizeof(CorrectSignalType)/sizeof(char*);

const int   CORRECT_SIGNAL_TYPE_UNKNOWN		= -1,
            CORRECT_SIGNAL_TYPE_ADD			= 0,
            CORRECT_SIGNAL_TYPE_SUB			= 1;

// ----------------------------------------------------------------------------------------------

#define     ERR_CORRECT_SIGNAL_TYPE(type) (type < 0 || type >= CORRECT_SIGNAL_TYPE_COUNT)
#define		TEST_CORRECT_SIGNAL_TYPE(type)			if (ERR_CORRECT_SIGNAL_TYPE(type)) { return; }
#define     TEST_CORRECT_SIGNAL_TYPE1(type, retVal)	if (ERR_CORRECT_SIGNAL_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const ValueType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Electric"),
            QT_TRANSLATE_NOOP("Measure.h", "Physical"),
            QT_TRANSLATE_NOOP("Measure.h", "Output"),
};

const int	VALUE_TYPE_COUNT		= sizeof(ValueType)/sizeof(char*);

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
};

const int	ERROR_TYPE_COUNT		= sizeof(ErrorType)/sizeof(char*);

const int   ERROR_TYPE_UNKNOWN		= -1,
            ERROR_TYPE_ABSOLUTE		= 0,
            ERROR_TYPE_REDUCE		= 1;

// ----------------------------------------------------------------------------------------------

#define     ERR_ERROR_TYPE(type) (type < 0 || type >= ERROR_TYPE_COUNT)
#define		TEST_ERROR_TYPE(type)			if (ERR_ERROR_TYPE(type)) { return; }
#define     TEST_ERROR_TYPE1(type, retVal)	if (ERR_ERROR_TYPE(type)) { return retVal; }

// ==============================================================================================

const char* const AdditionalError[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "System error"),
            QT_TRANSLATE_NOOP("Measure.h", "MSE"),
            QT_TRANSLATE_NOOP("Measure.h", "Low border"),
            QT_TRANSLATE_NOOP("Measure.h", "High border"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
            QT_TRANSLATE_NOOP("Measure.h", "reserve"),
};

const int	ADDITIONAL_ERROR_COUNT          = sizeof(AdditionalError)/sizeof(char*);

const int   ADDITIONAL_ERROR_UNKNOWN        = -1,
            ADDITIONAL_ERROR_SYSTEM         = 0,
            ADDITIONAL_ERROR_MSE            = 1,
            ADDITIONAL_ERROR_LOW_BORDER     = 2,
            ADDITIONAL_ERROR_HIGH_BORDER    = 3;

// ==============================================================================================

const int   MEASUREMENT_IN_POINT    = 20;

// ==============================================================================================

const int   INVALID_VALUE           = 0xFFFF;

// ==============================================================================================

class DevicePosition
{
private:

    QString m_deviceStrID;

    int m_caseNo = -1;
    QString	m_caseType;

    int m_channel = -1;
    int m_subblock = -1;
    int m_block = -1;
    int m_entry = -1;

public:

    QString deviceStrID() const { return m_deviceStrID; }
    void setDeviceStrID(const QString& deviceStrID) { m_deviceStrID = deviceStrID; }

    int caseNo() const { return m_caseNo; }
    void setCaseNo(int caseNo) { m_caseNo = caseNo; }

    QString caseType() const { return m_caseType; }
    void setCaseType(const QString& caseType) { m_caseType = caseType; }

    QString caseString() const;

    int channel() const { return m_channel; }
    void setChannel(int channel) { m_channel = channel; }

    QString channelString() const;

    int subblock() const { return m_subblock; }
    void setSubblock(int subblock) { m_subblock = subblock; }

    QString subblockString() const;

    int block() const { return m_block; }
    void setBlock(int block) { m_block = block; }

    QString blockString() const;

    int entry() const { return m_entry; }
    void setEntry(int entry) { m_entry = entry; }

    QString entryString() const;

};

// ==============================================================================================

class MeasureItem
{

public:

    explicit MeasureItem(int type = MEASURE_TYPE_UNKNOWN);

private:

    int m_baseIndex = -1;                               // index of record in array MeasureBase
    int m_tableIndex = -1;                              // index of record in table SQL

    int m_recordID = -1;                                // primary key value of record
    bool m_filter = false;                              // filter for record, if "true" - hide record
    int m_measureType = MEASURE_TYPE_UNKNOWN;           // measure tyupe

    QDateTime m_measureTime;                            // measure time
    int m_reportType = -1;                              // report type

public:

    QString strID(bool external);

    int baseIndex() const { return m_baseIndex; }
    void setBaseIndex(int index) { m_baseIndex = index; }

    int tableIndex() const { return m_tableIndex; }
    void setTableIndex(int index) { m_tableIndex = index; }

    int recordID() const { return m_recordID; }
    void setRecordID(int record) { m_recordID = record; }

    bool filter() const { return m_filter; }
    void setFilter(bool filter) { m_filter = filter; }

    int measureType() const { return m_measureType; }
    void setMeasureType(int type) { m_measureType = type; }

    QDateTime measureTime() const { return m_measureTime; }
    void setMeasureTime(QDateTime time) { m_measureTime = time; }

    QString measureTimeString() const;

    int reportType() const { return m_reportType; }
    void setReportType(int type) { m_reportType = type; }

};

// ==============================================================================================

class LinearetyMeasureItem : public MeasureItem
{

public:

    explicit LinearetyMeasureItem();

private:

    QString m_strID;
    QString m_extStrID;
    QString m_name;

    DevicePosition m_position;

    bool m_hasOutput = false;

    double m_lowLimit[VALUE_TYPE_COUNT];
    double m_highLimit[VALUE_TYPE_COUNT];

    QString m_unit[VALUE_TYPE_COUNT];

    double m_adjustment = 0;

    double m_nominal[VALUE_TYPE_COUNT];

    double m_percent = 0;

    double m_measure[VALUE_TYPE_COUNT];

    int m_measureArrayCount = 0;
    double m_measureArray[VALUE_TYPE_COUNT][MEASUREMENT_IN_POINT];

    int m_valuePrecision[VALUE_TYPE_COUNT];

    double m_errorInput[ERROR_TYPE_COUNT];
    double m_errorOutput[ERROR_TYPE_COUNT];
    double m_errorLimit[ERROR_TYPE_COUNT];

    int m_errorPrecision[ERROR_TYPE_COUNT];

    double m_errorAddional[ADDITIONAL_ERROR_COUNT];

public:

    QString strID() const { return m_strID; }
    void setStrID(const QString& strID) { m_strID = strID; }

    QString extStrID() const { return m_extStrID; }
    void setExtStrID(const QString& extStrID) { m_extStrID = extStrID; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    DevicePosition& position() { return m_position; }

    bool hasOutput() { return m_hasOutput; }
    void setHasOutput(bool hasOutput) { m_hasOutput = hasOutput; }

    double lowLimit(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_lowLimit[type]; }
    void setLowLimit(int type, double lowLimit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_lowLimit[type] = lowLimit; }

    double highLimit(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_highLimit[type]; }
    void setHighLimit(int type, double highLimit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_highLimit[type] = highLimit; }

    QString limitString(int type) const;

    QString unit(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_unit[type]; }
    void setUnit(int type, QString unit) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_unit[type] = unit; }

    double adjustment() const { return m_adjustment; }
    void setAdjustment(double adjustment) { m_adjustment = adjustment; }

    double nominal(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_nominal[type]; }
    void setNominal(int type, double value) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_nominal[type] = value; }

    QString nominalString(int type) const;

    double percent() const { return m_percent; }
    void setPercent(double percent) { m_percent = percent; }

    double measure(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_measure[type]; }
    void setMeasure(int type, double value) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_measure[type] = value; }

    QString measureString(int type) const;

    int measureArrayCount() const { return m_measureArrayCount; }
    void setMeasureArrayCount(int count) { m_measureArrayCount = count; }

    double measureItemArray(int type, int index) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } if (index < 0 || index >= MEASUREMENT_IN_POINT) { assert(0); return 0; } return m_measureArray[type][index]; }
    void setMeasureItemArray(int type, int index, double value) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } if (index < 0 || index >= MEASUREMENT_IN_POINT) { assert(0); return; } m_measureArray[type][index] = value; }

    QString measureItemString(int type, int index) const;

    int valuePrecision(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_valuePrecision[type]; }
    void setValuePrecision(int type, int precision) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_valuePrecision[type] = precision; }

    double errorInput(int type) const { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return 0; } return m_errorInput[type]; }
    void setErrorInput(int type, double value) { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return; } m_errorInput[type] = value; }

    double errorOutput(int type) const { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return 0; } return m_errorOutput[type]; }
    void setErrorOutput(int type, double value) { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return; } m_errorOutput[type] = value; }

    double errorLimit(int type) const { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return 0; } return m_errorLimit[type]; }
    void setErrorLimit(int type, double value) { if (type < 0 || type >= ERROR_TYPE_COUNT) { assert(0); return; } m_errorLimit[type] = value; }

    int errorPrecision(int type) const { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return 0; } return m_errorPrecision[type]; }
    void setErrorPrecision(int type, int precision) { if (type < 0 || type >= VALUE_TYPE_COUNT) { assert(0); return; } m_errorPrecision[type] = precision; }

    double errorAddional(int type) const { if (type < 0 || type >= ADDITIONAL_ERROR_COUNT) { assert(0); return 0; } return m_errorAddional[type]; }
    void setErrorAddional(int type, double value) { if (type < 0 || type >= ADDITIONAL_ERROR_COUNT) { assert(0); return; } m_errorAddional[type] = value; }
};

// ==============================================================================================

#pragma pack(push, 1)

// ==============================================================================================

// struct

// ==============================================================================================

#pragma pack(pop)

// ==============================================================================================



