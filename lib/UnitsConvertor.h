#ifndef UNITSCONVERTOR_H
#define UNITSCONVERTOR_H

#include <assert.h>


#include "Types.h"
#include "UnitsConvertorTable.h"

// ==============================================================================================

// limits for input signals
//

const double RESISTOR_V_0_5 = 0.25;	 // 250 Ohm

const double V_0_5_LOW_LIMIT = 0;
const double V_0_5_HIGH_LIMIT = 5.1;

const double V_m10_p10_LOW_LIMIT = -11.5;
const double V_m10_p10_HIGH_LIMIT = 11.5;

// limits for otput signals
//

const double OUT_PH_LOW_LIMIT = 0;
const double OUT_PH_HIGH_LIMIT = 65535;

// limits for thermo couple
//

const double mV_K_TXA_LOW_LIMIT = -5.891;		// -200
const double mV_K_TXA_HIGH_LIMIT = 52.410;		// 1300

const double mV_L_TXK_LOW_LIMIT = -9.488;		// -200
const double mV_L_TXK_HIGH_LIMIT = 66.466;		// 800

const double mV_N_THH_LOW_LIMIT = -4.345;		// -270
const double mV_N_THH_HIGH_LIMIT = 47.513;		// 1300

const double mV_Type_B_LOW_LIMIT = 0.291;		// 250
const double mV_Type_B_HIGH_LIMIT = 13.820;		// 1820

const double mV_Type_E_LOW_LIMIT = -8.825;		// -200
const double mV_Type_E_HIGH_LIMIT = 76.373;		// 1000

const double mV_Type_J_LOW_LIMIT = -8.095;		// -210
const double mV_Type_J_HIGH_LIMIT = 69.553;		// 1200

const double mV_Type_K_LOW_LIMIT = -5.891;		// -200
const double mV_Type_K_HIGH_LIMIT = 54.886;		// 1372

const double mV_Type_N_LOW_LIMIT = -3.990;		// -200
const double mV_Type_N_HIGH_LIMIT = 47.513;		// 1300

const double mV_Type_R_LOW_LIMIT = -0.226;		// -50
const double mV_Type_R_HIGH_LIMIT = 21.101;		// 1768

const double mV_Type_S_LOW_LIMIT = -0.236;		// -50
const double mV_Type_S_HIGH_LIMIT = 18.693;		// 1768

const double mV_Type_T_LOW_LIMIT = -5.603;		// -200
const double mV_Type_T_HIGH_LIMIT = 20.872;		// 400

const double mV_Raw_Mul_8_LOW_LIMIT = -35.000;
const double mV_Raw_Mul_8_HIGH_LIMIT = 100.000;

const double mV_Raw_Mul_32_LOW_LIMIT = -8.800;
const double mV_Raw_Mul_32_HIGH_LIMIT = 19.000;

// limits for thermo resistor
//

const double Ohm_Pt_a_391_LOW_LIMIT = 17.2451;		// -200
const double Ohm_Pt_a_391_HIGH_LIMIT = 465.6806;	// 1100

const double Ohm_Pt_a_385_LOW_LIMIT = 18.5201;		// -200
const double Ohm_Pt_a_385_HIGH_LIMIT = 390.4811;	// 850

const double Ohm_Cu_a_428_LOW_LIMIT = 14.4500;		// -200
const double Ohm_Cu_a_428_HIGH_LIMIT = 185.5500;	// 200

const double Ohm_Cu_a_426_LOW_LIMIT = 78.6915;		// -50
const double Ohm_Cu_a_426_HIGH_LIMIT = 185.2340;	// 200

const double Ohm_Ni_a_617_LOW_LIMIT = 64.8300;		// -70
const double Ohm_Ni_a_617_HIGH_LIMIT = 223.2100;	// 180


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
	ElectricToEngeneering = 2,
	EngeneeringToElectric = 3,
};

// ==============================================================================================
// class UnitsConvert
//

class UnitsConvertor : public QObject
{
	Q_OBJECT

public:

	explicit UnitsConvertor(QObject *parent = nullptr);
	virtual ~UnitsConvertor();

public:

	Q_INVOKABLE UnitsConvertResult electricToPhysical_Input(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);						// for blocks of input signals - AIM, WAIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int outputMode);									// for blocks of output signals - AOM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);				// for blocks of thermocouple signals - TIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0);	// for blocks of thermoresistor signals - RIM

	double conversion(double val, const UnitsConvertType& conversionType, const E::ElectricUnit& unitID, const E::SensorType& sensorType, double r0 = 0);						// Only ThermoCouple and ThermoResistor.

};

// ==============================================================================================

#endif // UNITSCONVERTOR_H
