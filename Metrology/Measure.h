#pragma once

// ==============================================================================================

const int			MEASURE_TYPE_UNKNOWN            = -1,
                    MEASURE_TYPE_LINEARETY          = 0,
                    MEASURE_TYPE_COMPARATOR         = 1;
                    //MEASURE_TYPE_COMPLEX_COMPARATOR	= 2;

const int			MEASURE_TYPE_COUNT              = 2;

// ----------------------------------------------------------------------------------------------

#define				ERR_MEASURE_TYPE(type) (type < 0 || type >= MEASURE_TYPE_COUNT)
#define				TEST_MEASURE_TYPE(type)				if (ERR_MEASURE_TYPE(type)) { return; }
#define				TEST_MEASURE_TYPE1(type, retVal)	if (ERR_MEASURE_TYPE(type)) { return retVal; }

// ----------------------------------------------------------------------------------------------

extern const char*  MeasureTypeStr[MEASURE_TYPE_COUNT];


// ==============================================================================================

#pragma pack(push, 1)

// ==============================================================================================

// struct

// ==============================================================================================

#pragma pack(pop)



