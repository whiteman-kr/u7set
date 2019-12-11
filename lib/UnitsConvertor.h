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

const double V_m10_p10_LOW_LIMIT = -11;
const double V_m10_p10_HIGH_LIMIT = 11;

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

const double mV_Type_B_LOW_LIMIT = 0.3040;		// 255
const double mV_Type_B_HIGH_LIMIT = 13.763;		// 1815

const double mV_Type_E_LOW_LIMIT = -8.696;		// -195
const double mV_Type_E_HIGH_LIMIT = 75.997;		// 995

const double mV_Type_J_LOW_LIMIT = -7.996;		// -205
const double mV_Type_J_HIGH_LIMIT = 69.267;		// 1195

const double mV_Type_K_LOW_LIMIT = -5.813;		// -195
const double mV_Type_K_HIGH_LIMIT = 54.717;		// 1367

const double mV_Type_N_LOW_LIMIT = -3.939;		// -195
const double mV_Type_N_HIGH_LIMIT = 47.333;		// 1295

const double mV_Type_R_LOW_LIMIT = -0.208;		// -45
const double mV_Type_R_HIGH_LIMIT = 21.040;		// 1763

const double mV_Type_S_LOW_LIMIT = -0.215;		// -45
const double mV_Type_S_HIGH_LIMIT = 18.641;		// 1763

const double mV_Type_T_LOW_LIMIT = -5.523;		// -195
const double mV_Type_T_HIGH_LIMIT = 20.563;		// 395

const double mV_Raw_Mul_8_LOW_LIMIT = -35.000;
const double mV_Raw_Mul_8_HIGH_LIMIT = 100.000;

const double mV_Raw_Mul_32_LOW_LIMIT = -8.500;
const double mV_Raw_Mul_32_HIGH_LIMIT = 19.000;

// limits for thermo resistor
//

const double Ohm_Pt_a_391_LOW_LIMIT = 17.24;		// -200
const double Ohm_Pt_a_391_HIGH_LIMIT = 395.16;		// 850

const double Ohm_Pt_a_385_LOW_LIMIT = 18.52;		// -200
const double Ohm_Pt_a_385_HIGH_LIMIT = 390.48;		// 850

const double Ohm_Cu_a_428_LOW_LIMIT = 20.53;		// -180
const double Ohm_Cu_a_428_HIGH_LIMIT = 185.60;		// 200

const double Ohm_Cu_a_426_LOW_LIMIT = 78.70;		// -50
const double Ohm_Cu_a_426_HIGH_LIMIT = 185.20;		// 200

const double Ohm_Ni_a_617_LOW_LIMIT = 64.83;		// -70
const double Ohm_Ni_a_617_HIGH_LIMIT = 223.21;		// 180

const double Ohm_Raw_LOW_LIMIT = 0;
const double Ohm_Raw_HIGH_LIMIT = 1500;

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
	ElectricToEngineering = 2,
	EngineeringToElectric = 3,
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
	Q_INVOKABLE UnitsConvertResult electricToPhysical_Output(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int outputMode);						// for blocks of output signals - AOM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoCouple(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType);				// for blocks of thermocouple signals - TIM
	Q_INVOKABLE UnitsConvertResult electricToPhysical_ThermoResistor(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType, double r0);	// for blocks of thermoresistor signals - RIM

	double conversion(double val, const UnitsConvertType& conversionType, const E::ElectricUnit& unitID, const E::SensorType& sensorType, double r0 = 0);						// Only ThermoCouple and ThermoResistor.

};

// ==============================================================================================

#endif // UNITSCONVERTOR_H
