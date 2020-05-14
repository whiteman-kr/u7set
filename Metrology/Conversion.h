﻿#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>
#include <assert.h>

#include "../lib/Types.h"
#include "../lib/UnitsConvertorTable.h"


#include "SignalBase.h"

// ==============================================================================================

const int	CT_PHYSICAL_TO_ELECTRIC	= 0,
			CT_ELECTRIC_TO_PHYSICAL	= 1,

			CT_ENGINEER_TO_ELECTRIC	= 2,
			CT_ELECTRIC_TO_ENGINEER	= 3;

const int	CT_COUNT                = 4;

// ==============================================================================================

double		conversion(double val, int conversionType, const Metrology::SignalParam& param);
double		conversion(double val, int conversionType, const E::ElectricUnit unitID, const E::SensorType sensorType, double r0 = 0);

// ==============================================================================================

const int	CT_DEGREE_C_TO_F		= 0,
			CT_DEGREE_F_TO_C		= 1;

const int	CT_DEGREE_COUNT         = 2;

// ==============================================================================================

double		conversionDegree(double val, int conversionType);

// ==============================================================================================

#endif // CONVERSION_H
