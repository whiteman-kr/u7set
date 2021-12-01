#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "../CommonLib/Types.h"

// ==============================================================================================
//
struct SignalElectricLimit
{
	bool isValid();

	double lowLimit = 0;
	double highLimit = 0;

	E::ElectricUnit unit = E::ElectricUnit::NoUnit;
	E::SensorType sensorType = E::SensorType::NoSensor;
};

const SignalElectricLimit SignalElectricLimits[] =
{
	// V
	//
	{   0,		5.1,	E::ElectricUnit::V,		E::SensorType::V_0_5,				},						// module AIM
	{ -11,		11,		E::ElectricUnit::V,		E::SensorType::V_m10_p10,			},						// module MAI and WAIM

	// mA
	//
	{   0,		5.1,	E::ElectricUnit::mA,	E::SensorType::V_0_5,				},						// module AIM from Rload
	{ -12,		12,		E::ElectricUnit::mA,	E::SensorType::V_m10_p10,			},						// module MAI from Rload

	// micro A
	//
	{   -20,	20,		E::ElectricUnit::uA,	E::SensorType::uA_m20_p20,			},						// module MAIM

	// Ohm - types of thermistors																			// all ohm electric limits and tables for R0=100
	//
	{ 17.24,	395.16,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1391,		},	// -200 .. 850		// non ptaform module from R0=50
	{ 17.24,	395.16,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1391,		},	// -200 .. 850		// non ptaform module from R0=100
	{ 18.52,	390.48,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1385,		},	// -200 .. 850		// non ptaform module from R0=50
	{ 18.52,	390.48,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1385,		},	// -200 .. 850		// non ptaform module from R0=100

	{ 20.53,	185.60,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu50_W1428,		},	// -180 .. 200		// non ptaform module from R0=50
	{ 20.53,	185.60,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu100_W1428,		},	// -180 .. 200		// non ptaform module from R0=100
	{ 78.70,	185.20,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu50_W1426,		},	//  -50 .. 200		// non ptaform module from R0=50
	{ 78.70,	185.20,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu100_W1426,		},	//  -50 .. 200		// non ptaform module from R0=100

	{ 17.24,	395.16,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_391,		},	// -200 .. 850		// module MAI and RIM from R0
	{ 18.52,	390.48,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_385,		},	// -200 .. 850		// module MAI and RIM from R0
	{ 20.53,	185.60,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_428,		},	// -180 .. 200		// module MAI and RIM from R0
	{ 78.70,	185.20,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_426,		},	//  -50 .. 200		// module MAI and RIM from R0
	{ 64.83,	223.21,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni_a_617,		},	//  -70 .. 180		// module MAI and RIM from R0

	{ 17.28,	283.80,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt21,			},	// -200 .. 500		// module MAI and non ptaform module from R0
	{ 78.70,	176.68,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu23,			},	//  -50 .. 180		// module MAI and non ptaform module from R0

	{  0.00,	 10000,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Raw,				},						// module MAI and RIM

	// mV - types of thermocouple
	//
	{ -5.891,	52.410,	E::ElectricUnit::mV,	E::SensorType::mV_K_TXA,			},	// -200 .. 1300		// module non ptaform
	{ -9.488,	66.466,	E::ElectricUnit::mV,	E::SensorType::mV_L_TXK,			},	// -200 .. 800		// module non ptaform
	{ -4.345,	47.513,	E::ElectricUnit::mV,	E::SensorType::mV_N_THH,			},	// -270 .. 1300		// module non ptaform

	{  0.000,	13.763,	E::ElectricUnit::mV,	E::SensorType::mV_Type_B,			},	//    0 .. 1815		// module TIM (-+5 C) and MAI
	{ -8.696,	75.997,	E::ElectricUnit::mV,	E::SensorType::mV_Type_E,			},	// -195 .. 995		// module TIM (-+5 C) and MAI
	{ -7.996,	69.267,	E::ElectricUnit::mV,	E::SensorType::mV_Type_J,			},	// -205 .. 1195		// module TIM (-+5 C) and MAI
	{ -5.813,	54.717,	E::ElectricUnit::mV,	E::SensorType::mV_Type_K,			},	// -195 .. 1367		// module TIM (-+5 C) and MAI
	{ -3.939,	47.333,	E::ElectricUnit::mV,	E::SensorType::mV_Type_N,			},	// -195 .. 1295		// module TIM (-+5 C) and MAI
	{ -0.226,	21.040,	E::ElectricUnit::mV,	E::SensorType::mV_Type_R,			},	//  -50 .. 1763		// module TIM (-+5 C) and MAI
	{ -0.236,	18.641,	E::ElectricUnit::mV,	E::SensorType::mV_Type_S,			},	//  -50 .. 1763		// module TIM (-+5 C) and MAI
	{ -5.523,	20.872,	E::ElectricUnit::mV,	E::SensorType::mV_Type_T,			},	// -195 .. 400		// module TIM (-+5 C) and MAI
	{ -9.488,	66.466,	E::ElectricUnit::mV,	E::SensorType::mV_Type_L,			},	// -200 .. 800		// module MAI
	{ -6.154,	 4.722,	E::ElectricUnit::mV,	E::SensorType::mV_Type_M,			},	// -200 .. 100		// module MAI


	{ -35.000,	100.00,	E::ElectricUnit::mV,	E::SensorType::mV_Raw_Mul_8,		},						// module TIM
	{ -8.500,	19.000,	E::ElectricUnit::mV,	E::SensorType::mV_Raw_Mul_32,		},						// module TIM
	{ -1200,	1200,	E::ElectricUnit::mV,	E::SensorType::mV_Raw_m1200_p1200,	},						// module MAI

	// Hz
	//
	{  0.05,	50000,	E::ElectricUnit::Hz,	E::SensorType::Hz_005_50000,		},						// module FIM
};

const int SignalElectricLimitCount = sizeof(SignalElectricLimits) / sizeof(SignalElectricLimits[0]);

// ==============================================================================================

// limits for Rload_Ohm if AIM use units mA
//
const double RLOAD_OHM_LOW_LIMIT = 50;																		// module AIM and Rload
const double RLOAD_OHM_HIGH_LIMIT = 1000;																	// module AIM and Rload

// limits for output signals of module AOM
//
const double OUT_PH_LOW_LIMIT = 0;																			// module AOM
const double OUT_PH_HIGH_LIMIT = 65535;																		// module AOM

// ==============================================================================================
// class UnitsConvertResult
//

enum class UnitsConvertResultError
{
	NoError = 0,
	Generic = 1,
	LowLimitOutOfRange = 2,
	HighLimitOutOfRange = 3
};

class UnitsConvertResult
{
	Q_GADGET

public:
	UnitsConvertResult();

	explicit UnitsConvertResult(double result);																						// Good result constructor
	explicit UnitsConvertResult(UnitsConvertResultError errorCode, const QString& errorMessage);									// Generic Error constructor
	explicit UnitsConvertResult(UnitsConvertResultError errorCode, double expectedLowValidRange, double expectedHighValidRange);	// Range Error constructor

	bool ok() const;

	// Functions accessed if ok() is true

	bool isEqual(double value) const;
	double toDouble() const;

	// Functions accessed if ok() is false

	int errorCode() const;
	QString errorMessage() const;

	double expectedLowValidRange() const;
	double expectedHighValidRange() const;

	// Properties accessed from scripts

	Q_PROPERTY(bool ok READ ok)

	Q_PROPERTY(double toDouble READ toDouble)

	Q_PROPERTY(int errorCode READ errorCode)
	Q_PROPERTY(QString errorMessage READ errorMessage)
	Q_PROPERTY(double expectedLowValidRange READ expectedLowValidRange)
	Q_PROPERTY(double expectedHighValidRange READ expectedHighValidRange)

private:

	bool m_ok = false;

	double m_result = 0;

	UnitsConvertResultError m_errorCode = UnitsConvertResultError::NoError;

	QString m_errorMessage;

	double m_expectedLowValidRange = 0;
	double m_expectedHighValidRange = 0;
};

Q_DECLARE_METATYPE(UnitsConvertResult)

// ==============================================================================================

enum class UnitsConvertType
{
	ElectricToPhysical = 0,
	PhysicalToElectric = 1,
	CelsiusToFahrenheit = 2,
	FahrenheitToCelsius = 3,
};

Q_DECLARE_METATYPE(UnitsConvertType)

// ==============================================================================================

enum class ConversionDirection
{
	Normal = 0,
	Inversion = 1,
};

Q_DECLARE_METATYPE(ConversionDirection)

// ==============================================================================================
// class UnitsConvertor
//

class UnitsConvertor : public QObject
{
	Q_OBJECT

public:

	explicit UnitsConvertor(QObject *parent = nullptr);
	virtual ~UnitsConvertor() override;

public:

	double conversion(double val, UnitsConvertType conversionType, const AppSignal& signal);																					// universal conversion from electrical to physical and vice versa
	double conversionDegree(double val, UnitsConvertType conversionType, E::ElectricUnit unitID, E::SensorType sensorType, double r0 = 0);										// conversion only ThermoCouple and ThermoResistor
	double conversionDegree(double val, UnitsConvertType conversionType);																										// conversion only Celsius to Fahrenheit and vice versa

	Q_INVOKABLE double conversionByConnection(double val, int connectionType, const AppSignal& sourSignal, const AppSignal& destSignal, ConversionDirection directType);		// conversion for Metrology connections, return converted value

	double r0_from_signal(const AppSignal& signal);																																// for signals of module MAI and RIM
	bool r0_is_use(E::SensorType sensorType);																																	// for signals of module MAI and RIM
	double default_r0(E::SensorType sensorType);																																// for signals of module MAI and RIM

	SignalElectricLimit getElectricLimit(int unitID, int sensorType);																											// take limit by unit and sensorType
	UnitsConvertResult electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0 = 0);						// test electrical value - out of electrical range?

	Q_INVOKABLE UnitsConvertResult electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double rload);			// get physical value for blocks of input signals			- module AIM, WAIM, MAIM, FIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);				// get physical value for blocks of thermocouple signals	- module TIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0);	// get physical value for blocks of thermoresistor signals	- module MAI and RIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode);						// get physical value for blocks of output signals			- module AOM

	Q_INVOKABLE QString electricUnitName(int electricUnit) const;
	Q_INVOKABLE QString sensorTypeName(int sensorType) const;
};

// ==============================================================================================

