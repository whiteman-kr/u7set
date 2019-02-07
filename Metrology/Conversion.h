﻿#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>

#include "../lib/Types.h"

#include "SignalBase.h"

// ==============================================================================================

const int	CT_PHYSICAL_TO_ELECTRIC	= 0,
			CT_ELECTRIC_TO_PHYSICAL	= 1,

			CT_ENGENEER_TO_ELECTRIC	= 2,
			CT_ELECTRIC_TO_ENGENEER	= 3;


const int	CT_COUNT                = 4;

// ==============================================================================================

double		conversion(double val, int conversionType, const Metrology::SignalParam& param);
double		conversion(double val, int conversionType, const E::ElectricUnit unitID, const E::SensorType sensorType);

// ==============================================================================================

const int	CT_PROPABILITY_95    = 0;

const int	CT_PROPABILITY_COUNT = 1;

double		studentK(int measureCount, int p);

// ==============================================================================================


#endif // CONVERSION_H
