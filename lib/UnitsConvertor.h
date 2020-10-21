#ifndef UNITSCONVERTOR_H
#define UNITSCONVERTOR_H

#include <assert.h>

#include "Signal.h"
#include "Types.h"
#include "UnitsConvertorTable.h"

// ==============================================================================================
//
struct UnitsConvertorLimit
{
	E::ElectricUnit unit = E::ElectricUnit::NoUnit;
	E::SensorType sensorType = E::SensorType::NoSensor;

	double lowLimit = 0;
	double highLimit = 0;
};

const UnitsConvertorLimit UnitsConvertorLimits[] =
{
	// V
	//
	{ E::ElectricUnit::V,		E::SensorType::V_0_5,				0,			5.1 },							// module AIM
	{ E::ElectricUnit::V,		E::SensorType::V_m10_p10,			-11,		11 },							// module WAIM

	// mA
	//
	{ E::ElectricUnit::mA,		E::SensorType::V_0_5,				0,			5.1 },							// module AIM and Rload

	// types of thermistors
	//
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1391,		17.24,		395.16 },	// -200 .. 850		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1391,		17.24,		395.16 },	// -200 .. 850		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1385,		18.52,		390.48 },	// -200 .. 850		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1385,		18.52,		390.48 },	// -200 .. 850		// module non ptaform

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu50_W1428,		20.53,		185.60 },	// -180 .. 200		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu100_W1428,		20.53,		185.60 },	// -180 .. 200		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu50_W1426,		78.70,		185.20 },	//  -50 .. 200		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu100_W1426,		78.70,		185.20 },	//  -50 .. 200		// module non ptaform

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt21,			 7.95,		153.30 },	// -200 .. 650		// module non ptaform
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu23,			41.71,		 93.64 },	//  -50 .. 180		// module non ptaform

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_391,		17.24,		395.16 },	// -200 .. 850		// module RIM
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Pt_a_385,		18.52,		390.48 },	// -200 .. 850		// module RIM
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_428,		20.53,		185.60 },	// -180 .. 200		// module RIM
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Cu_a_426,		78.70,		185.20 },	//  -50 .. 200		// module RIM
	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Ni_a_617,		64.83,		223.21 },	//  -70 .. 180		// module RIM

	{ E::ElectricUnit::Ohm, 	E::SensorType::Ohm_Raw,				 0.00,		1500 },							// module RIM

	// types of thermocouple
	//
	{ E::ElectricUnit::mV,		E::SensorType::mV_K_TXA,			-5.891,		52.410 },	// -200 .. 1300		// module non ptaform
	{ E::ElectricUnit::mV,		E::SensorType::mV_L_TXK,			-9.488,		66.466 },	// -200 .. 800		// module non ptaform
	{ E::ElectricUnit::mV,		E::SensorType::mV_N_THH,			-4.345,		47.513 },	// -270 .. 1300		// module non ptaform

    { E::ElectricUnit::mV,		E::SensorType::mV_Type_B,			 0.304,		13.763 },	//  255 .. 1815		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_E,			-8.696,		75.997 },	// -195 .. 995		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_J,			-7.996,		69.267 },	// -205 .. 1195		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_K,			-5.813,		54.717 },	// -195 .. 1367		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_N,			-3.939,		47.333 },	// -195 .. 1295		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_R,			-0.208,		21.040 },	//  -45 .. 1763		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_S,			-0.215,		18.641 },	//  -45 .. 1763		// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Type_T,			-5.523,		20.563 },	// -195 .. 395		// module TIM

	{ E::ElectricUnit::mV,		E::SensorType::mV_Raw_Mul_8,		-35.000,	100.00 },						// module TIM
	{ E::ElectricUnit::mV,		E::SensorType::mV_Raw_Mul_32,		-8.500,		19.000 },						// module TIM
};

const int UnitsConvertorLimitCount = sizeof(UnitsConvertorLimits) / sizeof(UnitsConvertorLimits[0]);

// ==============================================================================================

// limits for Rload_Ohm if AIM use units mA
//
const double RLOAD_LOW_LIMIT = 50;																				// module AIM and Rload
const double RLOAD_HIGH_LIMIT = 1000;																			// module AIM and Rload

// limits for otput signals of module AOM
//
const double OUT_PH_LOW_LIMIT = 0;																				// module AOM
const double OUT_PH_HIGH_LIMIT = 65535;																			// module AOM

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
	TIM = 3,
	RIM = 4,
};

// ==============================================================================================

enum class UnitsConvertType
{
	ElectricToPhysical = 0,
	PhysicalToElectric = 1,
};

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

	double r0_from_signal(const Signal& signal);																																// for signals of module RIM
	bool r0_is_use(int sensorType);																																				// for signals of module RIM

	bool getElectricLimit(int unitID, int sensorType, UnitsConvertorLimit& unitLimit);																							// take limit by unit and sensorType
	UnitsConvertResult electricLimitIsValid(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0 = 0);						// test electrical value - out of electrical range?

	UnitsConvertModule getModuleType(int unitID, int sensorType);																												// take module type by unit and sensorType

	Q_INVOKABLE UnitsConvertResult electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double rload);			// get physical value for blocks of input signals			- module AIM, WAIM (V - AIM and WAIM, mA - only AIM with Rload)
	Q_INVOKABLE UnitsConvertResult electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode);						// get physical value for blocks of output signals			- module AOM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);				// get physical value for blocks of thermocouple signals	- module TIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0);	// get physical value for blocks of thermoresistor signals	- module RIM
};

// ==============================================================================================

#endif // UNITSCONVERTOR_H
