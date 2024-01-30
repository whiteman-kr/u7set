#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "../CommonLib/Types.h"

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

struct SignalElectricLimit
{
	bool isValid() const;

	E::ElectricUnit unit = E::ElectricUnit::NoUnit;
	E::SensorType sensorType = E::SensorType::NoSensor;

	double lowLimit = 0;
	double highLimit = 0;
};

#define ELECTRIC_LIMIT(electricUnit, sensorType, lowLimit, highLimit) \
					{ \
						{ electricUnit, sensorType },	\
						{ electricUnit, sensorType, lowLimit, highLimit } 	\
					}


// ==============================================================================================
// class UnitsConverter
//

class UnitsConverter : public QObject
{
	Q_OBJECT

	inline static const std::map<std::pair<E::ElectricUnit, E::SensorType>, SignalElectricLimit> m_electricLimits =
	{
		// V
		//
		ELECTRIC_LIMIT(E::ElectricUnit::V,		E::SensorType::V_0_5,				0,			5.1),		// module AIM
		ELECTRIC_LIMIT(E::ElectricUnit::V,		E::SensorType::V_m10_p10,			-11,		11),		// module MAI and WAIM

		// mA
		//
		ELECTRIC_LIMIT(E::ElectricUnit::mA,		E::SensorType::V_0_5,				0,			5.1),		// module AIM from Rload
		ELECTRIC_LIMIT(E::ElectricUnit::mA,		E::SensorType::V_m10_p10,			-12,		12),		// module MAI from Rload

		// micro A
		//
		ELECTRIC_LIMIT(E::ElectricUnit::uA,		E::SensorType::uA_m20_p20,			-20,		20),		// module MAIM

		// Ohm - types of thermistors									// all ohm electric limits and tables for R0=100
		//
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt50_W1391,		17.24,		395.16),	// -200 .. 850		// non platform module from R0=50
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt100_W1391,		17.24,		395.16),	// -200 .. 850		// non platform module from R0=100
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt50_W1385,		18.52,		390.48),	// -200 .. 850		// non platform module from R0=50
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt100_W1385,		18.52,		390.48),	// -200 .. 850		// non platform module from R0=100

		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu50_W1428,		20.53,		185.60),	// -180 .. 200		// non platform module from R0=50
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu100_W1428,		20.53,		185.60),	// -180 .. 200		// non platform module from R0=100
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu50_W1426,		78.70,		185.20),	//  -50 .. 200		// non platform module from R0=50
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu100_W1426,		78.70,		185.20),	//  -50 .. 200		// non platform module from R0=100

		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt_a_391,		17.24,		395.16),	// -200 .. 850		// module MAI and RIM from R0
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt_a_385,		18.52,		390.48),	// -200 .. 850		// module MAI and RIM from R0
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu_a_428,		20.53,		185.60),	// -180 .. 200		// module MAI and RIM from R0
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu_a_426,		78.70,		185.20),	//  -50 .. 200		// module MAI and RIM from R0
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Ni_a_617,		64.83,		223.21),	//  -70 .. 180		// module MAI and RIM from R0

		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Pt21,			17.28,		283.80),	// -200 .. 500		// module MAI and non platform module from R0=100
		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Cu23,			78.70,		176.68),	//  -50 .. 180		// module MAI and non platform module from R0=100

		ELECTRIC_LIMIT(E::ElectricUnit::Ohm,	E::SensorType::Ohm_Raw,				0.00,		10000),		// module MAI and RIM

		// mV - types of thermocouple
		//
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_K_TXA,			-5.891,		52.410),	// -200 .. 1300		// module non platform
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_L_TXK,			-9.488,		66.466),	// -200 .. 800		// module non platform
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_N_THH,			-4.345,		47.513),	// -270 .. 1300		// module non platform

		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_B,			0.000,		13.763),	//    0 .. 1815		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_E,			-8.696,		75.997),	// -195 .. 995		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_J,			-7.996,		69.267),	// -205 .. 1195		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_K,			-5.813,		54.717),	// -195 .. 1367		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_N,			-3.939,		47.333),	// -195 .. 1295		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_R,			-0.226,		21.040),	//  -50 .. 1763		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_S,			-0.236,		18.641),	//  -50 .. 1763		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_T,			-5.523,		20.872),	// -195 .. 400		// module TIM (-+5 C) and MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_L,			-9.488,		66.466),	// -200 .. 800		// module MAI
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Type_M,			-6.154,		4.722),		// -200 .. 100		// module MAI

		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Raw_Mul_8,		-35.000,	100.00),	// module TIM
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Raw_Mul_32,		-8.500,		19.000),	// module TIM
		ELECTRIC_LIMIT(E::ElectricUnit::mV,		E::SensorType::mV_Raw_m1200_p1200,	-1200,		1200),		// module MAI

		// Hz
		//
		ELECTRIC_LIMIT(E::ElectricUnit::Hz,		E::SensorType::Hz_005_50000,		0.05,	50000),			// module FIM
		ELECTRIC_LIMIT(E::ElectricUnit::Hz,		E::SensorType::Hz_0_60000,			0.0,	60000),			// module FIM
		ELECTRIC_LIMIT(E::ElectricUnit::Hz,		E::SensorType::Hz_0_50000,			0.0,	50000),			// module FIM-SR
	};

	inline static const std::map<E::ElectricUnit, std::set<E::SensorType>> electricUnitSensors =
	{
		{
			E::ElectricUnit::NoUnit,
			{
				E::SensorType::NoSensor
			}
		},

		{
			E::ElectricUnit::mA,
			{
				// no sensor for now
			}
		},

		{
			E::ElectricUnit::mV,
			{
				E::SensorType::mV_K_TXA,
				E::SensorType::mV_L_TXK,
				E::SensorType::mV_N_THH,

				E::SensorType::mV_Type_B,
				E::SensorType::mV_Type_E,
				E::SensorType::mV_Type_J,
				E::SensorType::mV_Type_K,
				E::SensorType::mV_Type_N,
				E::SensorType::mV_Type_R,
				E::SensorType::mV_Type_S,
				E::SensorType::mV_Type_T,

				E::SensorType::mV_Raw_Mul_8,
				E::SensorType::mV_Raw_Mul_32,

				E::SensorType::mV_Type_L,
				E::SensorType::mV_Type_M,
				E::SensorType::mV_Raw_m1200_p1200,
			}
		},

		{
			E::ElectricUnit::Ohm,
			{
				E::SensorType::Ohm_Pt50_W1391,
				E::SensorType::Ohm_Pt100_W1391,
				E::SensorType::Ohm_Pt50_W1385,
				E::SensorType::Ohm_Pt100_W1385,

				E::SensorType::Ohm_Cu50_W1428,
				E::SensorType::Ohm_Cu100_W1428,
				E::SensorType::Ohm_Cu50_W1426,
				E::SensorType::Ohm_Cu100_W1426,

				E::SensorType::Ohm_Pt21,
				E::SensorType::Ohm_Cu23,

				E::SensorType::Ohm_Ni50_W1617,
				E::SensorType::Ohm_Ni100_W1617,

				E::SensorType::Ohm_Pt_a_391,
				E::SensorType::Ohm_Pt_a_385,
				E::SensorType::Ohm_Cu_a_428,
				E::SensorType::Ohm_Cu_a_426,
				E::SensorType::Ohm_Ni_a_617,
				E::SensorType::Ohm_Raw,
			}
		},

		{
			E::ElectricUnit::V,
			{
				E::SensorType::V_0_5,
				E::SensorType::V_m10_p10,
			}
		},

		{
			E::ElectricUnit::uA,
			{
				E::SensorType::uA_m20_p20,
			}
		},

		{
			E::ElectricUnit::Hz,
			{
				E::SensorType::Hz_005_50000,
				E::SensorType::Hz_0_60000,
				E::SensorType::Hz_0_50000,
			}
		}
	};

	// limits for R0_Ohm of MAI
	//
	inline static const double R0_OHM_LOW_LIMIT = 1;
	inline static const double R0_OHM_HIGH_LIMIT = 2000;

	// limits for output signals of module AOM
	//
	inline static const double OUT_PH_LOW_LIMIT = 0;
	inline static const double OUT_PH_HIGH_LIMIT = 65535;

public:

	// limits for Rload_Ohm if AIM use units mA
	//
	inline static const double RLOAD_OHM_LOW_LIMIT = 10;
	inline static const double RLOAD_OHM_HIGH_LIMIT = 1000;

public:

	explicit UnitsConverter(QObject *parent = nullptr);
	virtual ~UnitsConverter() override;

public:

	double conversion(double val, UnitsConvertType conversionType, const AppSignal& signal);																					// universal conversion from electrical to physical and vice versa
	double conversionLinearity(double val, UnitsConvertType conversionType, double lowEn, double highEn, double lowEl, double highEl);											// simple linearity conversion
	double conversionDegree(double val, UnitsConvertType conversionType, E::ElectricUnit unitID, E::SensorType sensorType, double r0 = 0);										// conversion only ThermoCouple and ThermoResistor
	double conversionDegree(double val, UnitsConvertType conversionType);																										// conversion only Celsius to Fahrenheit and vice versa

	Q_INVOKABLE double conversionByConnection(double val, int connectionType, const AppSignal& sourSignal, const AppSignal& destSignal, ConversionDirection directType);		// conversion for Metrology connections, return converted value

	static double r0_from_signal(const AppSignal& signal);																																// for signals of module MAI and RIM
	static bool r0_is_use(E::SensorType sensorType);																																	// for signals of module MAI and RIM
	static double default_r0(E::SensorType sensorType);																																// for signals of module MAI and RIM

	static SignalElectricLimit getElectricLimit(E::ElectricUnit unit, E::SensorType sensorType);

	// test electrical value - out of electrical range?
	//
	static UnitsConvertResult electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit,
											E::ElectricUnit unitID, E::SensorType sensorType, double r0 = 0);
	static UnitsConvertResult electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit,
											int unitID, int sensorType, double r0 = 0);
	static bool rloadIsValid(double rload);
	static bool r0_OhmIsValid(double r0_Ohm);

	static bool isSensorValid(E::ElectricUnit electricUnit, E::SensorType sensorType);

	static QString electricUnitName(E::ElectricUnit unit);
	static QString sensorTypeName(E::SensorType sensorType);

	Q_INVOKABLE UnitsConvertResult electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double rload);			// get physical value for blocks of input signals			- module AIM, WAIM, MAIM, FIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);				// get physical value for blocks of thermocouple signals	- module TIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0);	// get physical value for blocks of thermoresistor signals	- module MAI and RIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode);						// get physical value for blocks of output signals			- module AOM

	Q_INVOKABLE QString electricUnitName(int electricUnit) const;
	Q_INVOKABLE QString sensorTypeName(int sensorType) const;
};

