#pragma once
#include <QtGlobal>

// ==============================================================================================

const char* const MeasureTypeStr[] =
{
    QT_TRANSLATE_NOOP("Measure.h", "Measurement linearity"),
    QT_TRANSLATE_NOOP("Measure.h", "Measurement comparators"),
//    QT_TRANSLATE_NOOP("Measure.h", "Measurement complex comparators"),
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

#pragma pack(push, 1)

// ==============================================================================================

// struct

// ==============================================================================================

#pragma pack(pop)



