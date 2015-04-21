#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>

// ==============================================================================================

const char* const InputUnitStr[] =
{ 
            QT_TRANSLATE_NOOP("Conversion.h", "Ohm"),
            QT_TRANSLATE_NOOP("Conversion.h", "mV"),
            QT_TRANSLATE_NOOP("Conversion.h", "mA"),
};

const int	INPUT_UNIT_COUNT	=	sizeof(InputUnitStr)/sizeof(const char* const);

const int	INPUT_UNIT_OHM	= 0,
            INPUT_UNIT_MV	= 1,
            INPUT_UNIT_MA	= 2;

// ==============================================================================================

const char* const InputSensorStr[] =
{
            QT_TRANSLATE_NOOP("Conversion.h", "Don't used"),

            QT_TRANSLATE_NOOP("Conversion.h", "Pt50 W=1.391"),
            QT_TRANSLATE_NOOP("Conversion.h", "Pt100 W=1.391"),
            QT_TRANSLATE_NOOP("Conversion.h", "Pt50 W=1.385"),
            QT_TRANSLATE_NOOP("Conversion.h", "Pt100 W=1.385"),

            QT_TRANSLATE_NOOP("Conversion.h", "Cu50 W=1.428"),
            QT_TRANSLATE_NOOP("Conversion.h", "Cu100 W=1.428"),
            QT_TRANSLATE_NOOP("Conversion.h", "Cu50 W=1.426"),
            QT_TRANSLATE_NOOP("Conversion.h", "Cu100 W=1.426"),

            QT_TRANSLATE_NOOP("Conversion.h", "Pt21"),
            QT_TRANSLATE_NOOP("Conversion.h", "Cu23"),
	
            QT_TRANSLATE_NOOP("Conversion.h", "K (TXA)"),
            QT_TRANSLATE_NOOP("Conversion.h", "L (TXK)"),
            QT_TRANSLATE_NOOP("Conversion.h", "N (THH)"),
};

const int	INPUT_SENSOR_COUNT = sizeof(InputSensorStr) / sizeof(const char*);

// ==============================================================================================

const int	NO_CONVERTER        = 0,

            //
			//
            OHM_PT_50_W_1391	= 1,
            OHM_PT_100_W_1391	= 2,
            OHM_PT_50_W_1385	= 3,
            OHM_PT_100_W_1385	= 4,

            OHM_CU_50_W_1428	= 5,
            OHM_CU_100_W_1428	= 6,
            OHM_CU_50_W_1426	= 7,
            OHM_CU_100_W_1426	= 8,
	
            OHM_PT_21			= 9,
            OHM_CU_23			= 10,

            //
			//
            MV_K_TXA			= 11,
            MV_L_TXK			= 12,
            MV_N_THH			= 13;

// ==============================================================================================

struct INPUT_UNIT_SENSOR
{
            int unit;
            int sensor;
};

// ==============================================================================================

const INPUT_UNIT_SENSOR InputUnitSensorStr[] = 
{
            { INPUT_UNIT_OHM, 	OHM_PT_50_W_1391 },
            { INPUT_UNIT_OHM, 	OHM_PT_100_W_1391 },
            { INPUT_UNIT_OHM, 	OHM_PT_50_W_1385 },
            { INPUT_UNIT_OHM, 	OHM_PT_100_W_1385 },

            { INPUT_UNIT_OHM, 	OHM_CU_50_W_1428 },
            { INPUT_UNIT_OHM, 	OHM_CU_100_W_1428 },
            { INPUT_UNIT_OHM, 	OHM_CU_50_W_1426 },
            { INPUT_UNIT_OHM, 	OHM_CU_100_W_1426 },
	
            { INPUT_UNIT_OHM, 	OHM_PT_21 },
            { INPUT_UNIT_OHM, 	OHM_CU_23 },

            { INPUT_UNIT_MV, 	MV_K_TXA },
            { INPUT_UNIT_MV, 	MV_L_TXK },
            { INPUT_UNIT_MV, 	MV_N_THH },
};

const int	INPUT_UNIT_SENSOR_COUNT = sizeof(InputUnitSensorStr) / sizeof(INPUT_UNIT_SENSOR);

// ==============================================================================================

const int	CT_PHYSICAL_TO_ELECTRIC	= 0,
            CT_ELECTRIC_TO_PHYSICAL	= 1;

const int	CT_COUNT                = 2;

// ==============================================================================================

//double      conversion(double val, int type, Signal* s);
double conversion(double val, int type, int unit, int sensor);

// ==============================================================================================

#endif // CONVERSION_H
