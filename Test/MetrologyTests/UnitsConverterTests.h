#pragma once

#include <QTest>

class UnitsConverterTests : public QObject
{
	Q_OBJECT

public:

	UnitsConverterTests();

private slots:

	void initTestCase();
	void cleanupTestCase();

	void test_ConversionLinearity();
	void test_ConversionDegree_mV();
	void test_ConversionDegree_Ohm();
	void test_Celsius_Fahrenheit();

	void test_electricToPhysical_Input();
	void test_electricToPhysical_ThermoCouple();
	void test_electricToPhysical_ThermoResistor();
};
