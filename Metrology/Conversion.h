#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>
#include <assert.h>

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

enum class ConversionDirection
{
	Normal = 0,
	Inversion = 1,
};

Q_DECLARE_METATYPE(ConversionDirection)

// ==============================================================================================

double conversionConnection(double val, ConversionDirection directType, Metrology::ConnectionType connectionType, const IoSignalParam& ioParam);
double conversionConnection(double val, ConversionDirection directType, Metrology::ConnectionType connectionType, const Signal& sourSignal, const Signal& destSignal);

// ==============================================================================================

#endif // CONVERSION_H
