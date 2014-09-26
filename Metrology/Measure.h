#pragma once
#include <QtGlobal>

// ==============================================================================================

const char* const MeasureTypeStr[] =
{
    QT_TRANSLATE_NOOP("Measure.h", "Measurements of linearity"),
    QT_TRANSLATE_NOOP("Measure.h", "Measurements of comparators"),
//    QT_TRANSLATE_NOOP("Measure.h", "Measurements of complex comparators"),
};


const int   MEASURE_TYPE_COUNT = sizeof(MeasureTypeStr)/sizeof(char*);

const int   MEASURE_TYPE_UNKNOWN            = -1,
            MEASURE_TYPE_LINEARITY          = 0,
            MEASURE_TYPE_COMPARATOR         = 1,
            MEASURE_TYPE_COMPLEX_COMPARATOR	= 2;


#define		ERR_MEASURE_TYPE(type) (type < 0 || type >= MEASURE_TYPE_COUNT)
#define		TEST_MEASURE_TYPE(type)				if (ERR_MEASURE_TYPE(type)) { return; }
#define		TEST_MEASURE_TYPE1(type, retVal)	if (ERR_MEASURE_TYPE(type)) { return retVal; }

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



