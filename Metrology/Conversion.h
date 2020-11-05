#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>
#include <assert.h>

#include "../lib/Types.h"

#include "SignalBase.h"

// ==============================================================================================

enum class ConversionType
{
	PhysicalToElectric = 0,
	ElectricToPhysical = 1,
	EnginnerToElectric = 2,
	ElectricToEnginner = 3,
};

Q_DECLARE_METATYPE(ConversionType)

// ==============================================================================================

double conversion(double val, ConversionType conversionType, const Metrology::SignalParam& param);
double conversion(double val, ConversionType conversionType, const E::ElectricUnit unitID, const E::SensorType sensorType, double r0 = 0);

// ==============================================================================================

enum class ConversionCalcType
{
	Normal = 0,
	Inversion = 1,
};

Q_DECLARE_METATYPE(ConversionCalcType)

// ==============================================================================================

double conversionCalcVal(double val, ConversionCalcType calcType, int connectionType, const IoSignalParam& ioParam);

// ==============================================================================================

#endif // CONVERSION_H
