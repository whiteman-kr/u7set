#ifndef UNITSCONVERTOR_H
#define UNITSCONVERTOR_H

#include <assert.h>

#include "Signal.h"
#include "Types.h"
#include "UnitsConvertorTable.h"

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
	{   0,		5.1,	E::ElectricUnit::V,		E::SensorType::V_0_5,			},						// module AIM
	{ -11,		11,		E::ElectricUnit::V,		E::SensorType::V_m10_p10,		},						// module WAIM

	// mA
	//
	{   0,		5.1,	E::ElectricUnit::mA,	E::SensorType::V_0_5,			},						// module AIM and Rload

	// micro A
	//
	{   -20,	20,		E::ElectricUnit::uA,	E::SensorType::uA_m20_p20,		},						// module MAIM

	// Ohm - types of thermistors
	//
	{ 17.24,	395.16,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1391,	},	// -200 .. 850		// module non ptaform
	{ 17.24,	395.16,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1391,	},	// -200 .. 850		// module non ptaform
	{ 18.52,	390.48,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1385,	},	// -200 .. 850		// module non ptaform
	{ 18.52,	390.48,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1385,	},	// -200 .. 850		// module non ptaform

	{ 20.53,	185.60,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu50_W1428,	},	// -180 .. 200		// module non ptaform
	{ 20.53,	185.60,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu100_W1428,	},	// -180 .. 200		// module non ptaform
	{ 78.70,	185.20,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu50_W1426,	},	//  -50 .. 200		// module non ptaform
	{ 78.70,	185.20,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu100_W1426,	},	//  -50 .. 200		// module non ptaform

	{  7.95,	153.30,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt21,		},	// -200 .. 650		// module non ptaform
	{ 41.71,	 93.64,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu23,		},	//  -50 .. 180		// module non ptaform

	{ 17.24,	395.16,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_391,	},	// -200 .. 850		// module RIM and R0
	{ 18.52,	390.48,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_385,	},	// -200 .. 850		// module RIM and R0
	{ 20.53,	185.60,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_428,	},	// -180 .. 200		// module RIM and R0
	{ 78.70,	185.20,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_426,	},	//  -50 .. 200		// module RIM and R0
	{ 64.83,	223.21,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni_a_617,	},	//  -70 .. 180		// module RIM and R0

	{  0.00,	1500,	E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Raw,			},						// module RIM

	// mV - types of thermocouple
	//
	{ -5.891,	52.410,	E::ElectricUnit::mV,	E::SensorType::mV_K_TXA,		},	// -200 .. 1300		// module non ptaform
	{ -9.488,	66.466,	E::ElectricUnit::mV,	E::SensorType::mV_L_TXK,		},	// -200 .. 800		// module non ptaform
	{ -4.345,	47.513,	E::ElectricUnit::mV,	E::SensorType::mV_N_THH,		},	// -270 .. 1300		// module non ptaform

	{  0.304,	13.763,	E::ElectricUnit::mV,	E::SensorType::mV_Type_B,		},	//  255 .. 1815		// module TIM
	{ -8.696,	75.997,	E::ElectricUnit::mV,	E::SensorType::mV_Type_E,		},	// -195 .. 995		// module TIM
	{ -7.996,	69.267,	E::ElectricUnit::mV,	E::SensorType::mV_Type_J,		},	// -205 .. 1195		// module TIM
	{ -5.813,	54.717,	E::ElectricUnit::mV,	E::SensorType::mV_Type_K,		},	// -195 .. 1367		// module TIM
	{ -3.939,	47.333,	E::ElectricUnit::mV,	E::SensorType::mV_Type_N,		},	// -195 .. 1295		// module TIM
	{ -0.208,	21.040,	E::ElectricUnit::mV,	E::SensorType::mV_Type_R,		},	//  -45 .. 1763		// module TIM
	{ -0.215,	18.641,	E::ElectricUnit::mV,	E::SensorType::mV_Type_S,		},	//  -45 .. 1763		// module TIM
	{ -5.523,	20.563,	E::ElectricUnit::mV,	E::SensorType::mV_Type_T,		},	// -195 .. 395		// module TIM

	{ -35.000,	100.00,	E::ElectricUnit::mV,	E::SensorType::mV_Raw_Mul_8,	},						// module TIM
	{ -8.500,	19.000,	E::ElectricUnit::mV,	E::SensorType::mV_Raw_Mul_32,	},						// module TIM

	// Hz
	//
	{   50,		50000,	E::ElectricUnit::Hz,	E::SensorType::Hz_50_50000,		},						// module FIM
};

const int SignalElectricLimitCount = sizeof(SignalElectricLimits) / sizeof(SignalElectricLimits[0]);

// ==============================================================================================

// limits for Rload_Ohm if AIM use units mA
//
const double RLOAD_OHM_LOW_LIMIT = 50;																	// module AIM and Rload
const double RLOAD_OHM_HIGH_LIMIT = 1000;																// module AIM and Rload

// limits for output signals of module AOM
//
const double OUT_PH_LOW_LIMIT = 0;																		// module AOM
const double OUT_PH_HIGH_LIMIT = 65535;																	// module AOM


// limits for output signals of module ROM
//
//const double OUT_OHM_LOW_LIMIT = 0;																	// module ROM
//const double OUT_OHM_HIGH_LIMIT = 2110; // Ohm														// module ROM

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

enum class UnitsConvertModule
{
	NonPlatform = 0,
	AIM = 1,
	WAIM = 2,
	MAIM = 3,
	TIM = 4,
	RIM = 5,
	FIM = 6,
};

Q_DECLARE_METATYPE(UnitsConvertModule)

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
	virtual ~UnitsConvertor();

public:

	double conversion(double val, const UnitsConvertType& conversionType, const Signal& signal);																				// universal conversion from electrical to physical and vice versa
	double conversionDegree(double val, const UnitsConvertType& conversionType, const E::ElectricUnit& unitID, const E::SensorType& sensorType, double r0 = 0);					// conversion only ThermoCouple and ThermoResistor
	double conversionDegree(double val, const UnitsConvertType& conversionType);																								// conversion only Celsius to Fahrenheit and vice versa

	double conversionByConnection(double val, int connectionType, const Signal& sourSignal, const Signal& destSignal, ConversionDirection directType );							// conversion for Metrology connections

	double r0_from_signal(const Signal& signal);																																// for signals of module RIM
	bool r0_is_use(int sensorType);																																				// for signals of module RIM

	SignalElectricLimit getElectricLimit(int unitID, int sensorType);																											// take limit by unit and sensorType
	UnitsConvertResult electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0 = 0);						// test electrical value - out of electrical range?

	UnitsConvertModule getModuleType(int unitID, int sensorType);																												// take module type by unit and sensorType

	Q_INVOKABLE UnitsConvertResult electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double rload);			// get physical value for blocks of input signals			- module AIM, WAIM, MAIM, FIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);				// get physical value for blocks of thermocouple signals	- module TIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0);	// get physical value for blocks of thermoresistor signals	- module RIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode);						// get physical value for blocks of output signals			- module AOM
};

// ==============================================================================================

#endif // UNITSCONVERTOR_H
