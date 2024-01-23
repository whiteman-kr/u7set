#include <QTest>
#include "UnitsConverterTests.h"
#include "../Metrology/UnitsConverter.h"

// -------------------------------------------------------------------------------------------------------------------

UnitsConverterTests::UnitsConverterTests()
{
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::initTestCase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::cleanupTestCase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_ConversionLinearity()
{
	UnitsConverter uc;
	double val = 0;

	//
	//
	val = uc.conversionLinearity(12.5, UnitsConvertType::ElectricToPhysical, -10, 100, 4, 20);
	QCOMPARE(val, 48.4375);

	val = uc.conversionLinearity(50.5, UnitsConvertType::PhysicalToElectric, -10, 100, 4, 20);
	QCOMPARE(val, 12.8);
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_ConversionDegree_mV()
{
	UnitsConverter uc;
	double val = 0;

	//
	//
	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_K_TXA);
	QCOMPARE(val, -5.891);

	val = uc.conversionDegree(1300, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_K_TXA);
	QCOMPARE(val, 52.410);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_L_TXK);
	QCOMPARE(val, -9.488);

	val = uc.conversionDegree(800, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_L_TXK);
	QCOMPARE(val, 66.466);

	val = uc.conversionDegree(-270, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_N_THH);
	QCOMPARE(val, -4.345);

	val = uc.conversionDegree(1300, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_N_THH);
	QCOMPARE(val, 47.513);

	//
	//
	val = uc.conversionDegree(0, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_B);
	QCOMPARE(val, 0.000);

	val = uc.conversionDegree(1815, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_B);
	QCOMPARE(val, 13.763);

	val = uc.conversionDegree(-195, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_E);
	QCOMPARE(val, -8.696);

	val = uc.conversionDegree(995, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_E);
	QCOMPARE(val, 75.997);

	val = uc.conversionDegree(-205, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_J);
	QCOMPARE(val, -7.996);

	val = uc.conversionDegree(1195, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_J);
	QCOMPARE(val, 69.267);

	val = uc.conversionDegree(-195, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_K);
	QCOMPARE(val, -5.813);

	val = uc.conversionDegree(1367, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_K);
	QCOMPARE(val, 54.717);

	val = uc.conversionDegree(-195, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_N);
	QCOMPARE(val, -3.939);

	val = uc.conversionDegree(1295, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_N);
	QCOMPARE(val, 47.333);

	val = uc.conversionDegree(-50, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_R);
	QCOMPARE(val, -0.226);

	val = uc.conversionDegree(1763, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_R);
	QCOMPARE(val, 21.040);

	val = uc.conversionDegree(-50, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_S);
	QCOMPARE(val, -0.236);

	val = uc.conversionDegree(1763, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_S);
	QCOMPARE(val, 18.641);

	val = uc.conversionDegree(-195, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_T);
	QCOMPARE(val, -5.523);

	val = uc.conversionDegree(400, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_T);
	QCOMPARE(val, 20.872);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_L);
	QCOMPARE(val, -9.488);

	val = uc.conversionDegree(800, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_L);
	QCOMPARE(val, 66.466);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_M);
	QCOMPARE(val, -6.154);

	val = uc.conversionDegree(100, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::mV, E::SensorType::mV_Type_M);
	QCOMPARE(val, 4.722);
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_ConversionDegree_Ohm()
{
	UnitsConverter uc;
	double val = 0;

	//
	//
	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt50_W1391);
	QCOMPARE(val, 17.24/2);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt100_W1391);
	QCOMPARE(val, 17.24);

	val = uc.conversionDegree(850, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt50_W1391);
	QCOMPARE(val, 395.16/2);

	val = uc.conversionDegree(850, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt100_W1391);
	QCOMPARE(val, 395.16);

	//
	//
	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt50_W1385);
	QCOMPARE(val, 18.52/2);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt100_W1385);
	QCOMPARE(val, 18.52);

	val = uc.conversionDegree(850, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt50_W1385);
	QCOMPARE(val, 390.48/2);

	val = uc.conversionDegree(850, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt100_W1385);
	QCOMPARE(val, 390.48);

	//
	//
	val = uc.conversionDegree(-180, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu50_W1428);
	QCOMPARE(val, 20.53/2);

	val = uc.conversionDegree(-180, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu100_W1428);
	QCOMPARE(val, 20.53);

	val = uc.conversionDegree(200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu50_W1428);
	QCOMPARE(val, 185.60/2);

	val = uc.conversionDegree(200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu100_W1428);
	QCOMPARE(val, 185.60);

	//
	//
	val = uc.conversionDegree(-50, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu50_W1426);
	QCOMPARE(val, 78.70/2);

	val = uc.conversionDegree(-50, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu100_W1426);
	QCOMPARE(val, 78.70);

	val = uc.conversionDegree(200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu50_W1426);
	QCOMPARE(val, 185.20/2);

	val = uc.conversionDegree(200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu100_W1426);
	QCOMPARE(val, 185.20);

	//
	//
	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 100);
	QCOMPARE(val, 17.24);

	val = uc.conversionDegree(850, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 100);
	QCOMPARE(val, 395.16);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 100);
	QCOMPARE(val, 18.52);

	val = uc.conversionDegree(850, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 100);
	QCOMPARE(val, 390.48);

	val = uc.conversionDegree(-180, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 100);
	QCOMPARE(val, 20.53);

	val = uc.conversionDegree(200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 100);
	QCOMPARE(val, 185.60);

	val = uc.conversionDegree(-50, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 100);
	QCOMPARE(val, 78.70);

	val = uc.conversionDegree(200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 100);
	QCOMPARE(val, 185.20);

	val = uc.conversionDegree(-70, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 100);
	QCOMPARE(val, 64.83);

	val = uc.conversionDegree(180, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 100);
	QCOMPARE(val, 223.21);

	val = uc.conversionDegree(-200, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt21, 100);
	QCOMPARE(val, 17.28);

	val = uc.conversionDegree(500, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt21, 100);
	QCOMPARE(val, 283.80);

	val = uc.conversionDegree(-50, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu23, 100);
	QCOMPARE(val, 78.70);

	val = uc.conversionDegree(180, UnitsConvertType::PhysicalToElectric, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu23, 100);
	QCOMPARE(val, 176.68);
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_Celsius_Fahrenheit()
{
	UnitsConverter uc;
	double val = 0;

	//
	//
	val = uc.conversionDegree(100, UnitsConvertType::CelsiusToFahrenheit);
	QCOMPARE(val, 212);

	val = uc.conversionDegree(122, UnitsConvertType::FahrenheitToCelsius);
	QCOMPARE(val, 50);
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_electricToPhysical_Input()
{
	UnitsConverter uc;
	UnitsConvertResult result;

	result = uc.electricToPhysical_Input(2.5, 0, 5, E::ElectricUnit::mA, E::SensorType::V_0_5, 250);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 0.625);

	result = uc.electricToPhysical_Input(12, 4, 20, E::ElectricUnit::mA, E::SensorType::V_m10_p10, 250);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 3);

	result = uc.electricToPhysical_Input(2.5, 0, 5, E::ElectricUnit::V, E::SensorType::V_0_5, 0);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 2.5);

	result = uc.electricToPhysical_Input(2.5, -10, 10, E::ElectricUnit::V, E::SensorType::V_m10_p10, 0);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 2.5);

	result = uc.electricToPhysical_Input(10, -20, 20, E::ElectricUnit::uA, E::SensorType::uA_m20_p20, 0);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 10);

	result = uc.electricToPhysical_Input(100, 0.05, 50000, E::ElectricUnit::Hz, E::SensorType::Hz_005_50000, 0);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_electricToPhysical_ThermoCouple()
{
	UnitsConverter uc;
	UnitsConvertResult result;

	result = uc.electricToPhysical_ThermoCouple(4.0960, -5.891, 52.410, E::ElectricUnit::mV, E::SensorType::mV_K_TXA);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(6.8620, -9.488,	66.466, E::ElectricUnit::mV, E::SensorType::mV_L_TXK);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(2.7740, -4.345,	47.513, E::ElectricUnit::mV, E::SensorType::mV_N_THH);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);


	result = uc.electricToPhysical_ThermoCouple(0.0330, 0.000,	13.763, E::ElectricUnit::mV, E::SensorType::mV_Type_B);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(6.3190, -8.696,	75.997, E::ElectricUnit::mV, E::SensorType::mV_Type_E);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(5.2690, -7.996,	69.267, E::ElectricUnit::mV, E::SensorType::mV_Type_J);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(4.0960, -5.813,	54.717, E::ElectricUnit::mV, E::SensorType::mV_Type_K);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(2.7740, -3.939,	47.333, E::ElectricUnit::mV, E::SensorType::mV_Type_N);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(0.6470, -0.226,	21.040, E::ElectricUnit::mV, E::SensorType::mV_Type_R);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(0.6460, -0.236,	18.641, E::ElectricUnit::mV, E::SensorType::mV_Type_S);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(4.2790, -5.523,	20.872, E::ElectricUnit::mV, E::SensorType::mV_Type_T);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(6.8620, -9.488,	66.466, E::ElectricUnit::mV, E::SensorType::mV_Type_L);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoCouple(4.7220, -6.154,	 4.722, E::ElectricUnit::mV, E::SensorType::mV_Type_M);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);


	result = uc.electricToPhysical_ThermoCouple(50, -35.000,	100.00, E::ElectricUnit::mV, E::SensorType::mV_Raw_Mul_8);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 50);

	result = uc.electricToPhysical_ThermoCouple(10, -8.500,	19.000, E::ElectricUnit::mV, E::SensorType::mV_Raw_Mul_32);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 10);

	result = uc.electricToPhysical_ThermoCouple(100, -1200,	1200, E::ElectricUnit::mV, E::SensorType::mV_Raw_m1200_p1200);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);
}

// -------------------------------------------------------------------------------------------------------------------

void UnitsConverterTests::test_electricToPhysical_ThermoResistor()
{
	UnitsConverter uc;
	UnitsConvertResult result;

	result = uc.electricToPhysical_ThermoResistor(69.5550, 17.24/2,	395.16/2, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt50_W1391, 50);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(139.1100, 17.24,	395.16, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt100_W1391, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(69.2550, 18.52/2,	390.48/2, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt50_W1385, 50);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(138.5100, 18.52,	390.48, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt100_W1385, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(71.4000, 20.53/2,	185.60/2, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu50_W1428, 50);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(142.8000, 20.53,	185.60, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu100_W1428, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(71.3000, 78.70/2,	185.20/2, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu50_W1426, 50);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(142.6000, 78.70,	185.20, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu100_W1426, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);


	result = uc.electricToPhysical_ThermoResistor(139.1100, 17.24,	395.16, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_391, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(138.5100, 18.52,	390.48, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt_a_385, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(142.8000, 20.53,	185.60, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_428, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(142.6000, 78.70,	185.20, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu_a_426, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(161.7200, 64.83,	223.21, E::ElectricUnit::Ohm, E::SensorType::Ohm_Ni_a_617, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(139.1000, 17.28,	283.80, E::ElectricUnit::Ohm, E::SensorType::Ohm_Pt21, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);

	result = uc.electricToPhysical_ThermoResistor(142.6000, 78.70,	176.68, E::ElectricUnit::Ohm, E::SensorType::Ohm_Cu23, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);


	result = uc.electricToPhysical_ThermoResistor(100, 0.00,	 10000, E::ElectricUnit::Ohm, E::SensorType::Ohm_Raw, 100);
	QCOMPARE(result.ok(), true);
	QCOMPARE(result.errorCode(), 0);
	QCOMPARE(result.toDouble(), 100);
}

// -------------------------------------------------------------------------------------------------------------------
