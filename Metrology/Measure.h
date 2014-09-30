#pragma once
#include <QtGlobal>

// ==============================================================================================

const char* const MeasureType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of linearity"),
            QT_TRANSLATE_NOOP("Measure.h", "Measurements of comparators"),
        //    QT_TRANSLATE_NOOP("Measure.h", "Measurements of complex comparators"),
};


const int   MEASURE_TYPE_COUNT = sizeof(MeasureType)/sizeof(char*);

const int   MEASURE_TYPE_UNKNOWN            = -1,
            MEASURE_TYPE_LINEARITY          = 0,
            MEASURE_TYPE_COMPARATOR         = 1,
            MEASURE_TYPE_COMPLEX_COMPARATOR	= 2;


#define		ERR_MEASURE_TYPE(type) (type < 0 || type >= MEASURE_TYPE_COUNT)
#define		TEST_MEASURE_TYPE(type)				if (ERR_MEASURE_TYPE(type)) { return; }
#define		TEST_MEASURE_TYPE1(type, retVal)	if (ERR_MEASURE_TYPE(type)) { return retVal; }

// ==============================================================================================

const int MeasureTimeout[] =
{
            0, 1, 2, 3, 5, 10, 15, 20, 30, 45, 60,
};

const int MeasureTimeoutCount = sizeof(MeasureTimeout)/sizeof(int);


// ==============================================================================================

const char* const	MeasureKind[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "In the one channel"),
            QT_TRANSLATE_NOOP("Measure.h", "In the all channels"),
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
            QT_TRANSLATE_NOOP("Measure.h", "Коррекция"),
            QT_TRANSLATE_NOOP("Measure.h", "Сигнал МПС"),
            QT_TRANSLATE_NOOP("Measure.h", "P → Tнас"),
            QT_TRANSLATE_NOOP("Measure.h", "Tнас → P"),
};

const int   OUTPUT_SIGNAL_TYPE_COUNT = sizeof(OutputSignalType)/sizeof(char*);

const int   OUTPUT_SIGNAL_TYPE_UNKNOWN			= -1,
            OUTPUT_SIGNAL_TYPE_CORRECTION		= 0,
            OUTPUT_SIGNAL_TYPE_MPS				= 1,
            OUTPUT_SIGNAL_TYPE_PT				= 2,
            OUTPUT_SIGNAL_TYPE_TP				= 3;

// ----------------------------------------------------------------------------------------------

#define     ERR_OUTPUT_SIGNAL_TYPE(type) (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
#define		TEST_OUTPUT_SIGNAL_TYPE(type)			if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return; }
#define     TEST_OUTPUT_SIGNAL_TYPE1(type, retVal)	if (ERR_OUTPUT_SIGNAL_TYPE(type)) { return retVal; }


// ==============================================================================================

const char* const CorrectSignalType[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Добавление (+)"),
            QT_TRANSLATE_NOOP("Measure.h", "Вычитание (-)"),
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

const char* const	MeasureErrorTypeStr[] =
{
            QT_TRANSLATE_NOOP("Measure.h", "Absolute"),
            QT_TRANSLATE_NOOP("Measure.h", "Reduce"),
};

const int	MEASURE_ERROR_TYPE_COUNT		= sizeof(MeasureErrorTypeStr)/sizeof(char*);

const int   MEASURE_ERROR_TYPE_UNKNOWN		= -1,
            MEASURE_ERROR_TYPE_ABSOLUTE		= 0,
            MEASURE_ERROR_TYPE_REDUCE		= 1;

// ==============================================================================================

#pragma pack(push, 1)

// ==============================================================================================

// struct

// ==============================================================================================

#pragma pack(pop)



