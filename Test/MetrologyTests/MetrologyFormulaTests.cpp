#include "MetrologyFormulaTests.h"

#include "../Metrology/MetrologyFormula.h"

// -------------------------------------------------------------------------------------------------------------------

MetrologyFormulaTests::MetrologyFormulaTests()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyFormulaTests::initTestCase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyFormulaTests::cleanupTestCase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyFormulaTests::test_calcMetrologyErrors()
{
	double val = 0;

	//
	//
	val = calcMetrologyError(Measure::MT::ErrorType::Absolute, 12, 12.01, 4, 20);
	QCOMPARE(val, 0.01);

	val = calcMetrologyError(Measure::MT::ErrorType::Reduce, 12, 12.01, 4, 20);
	QCOMPARE(val, 0.0625);

	val = calcMetrologyError(Measure::MT::ErrorType::Relative, 12, 12.03, 4, 20);
	QCOMPARE(val, 0.25);
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyFormulaTests::test_calcMetrologyCharacteristics()
{
	double val = 0;

	const int measureCount = 20;

	std::vector<double> measureArray;
	measureArray.resize(measureCount);

	//
	//
	for (int i = 0; i < measureCount; i++)
	{
		measureArray[i] = 10;
	}

	measureArray[7] = 11;

	val = calcMaxDeviation(10, measureArray);
	QCOMPARE(val, 11);

	//
	//
	val = calcSystemDeviation(10, 11);
	QCOMPARE(val, -1);

	//
	//
	for (int i = 0; i < measureCount; i++)
	{
		measureArray[i] = i;
	}

	val = calcSCO(10, measureArray);
	val = round(val*100000)/100000;
	QCOMPARE(val, 5.93828);

	//
	//
	val = calcLowBorder(1, 0.2, measureCount);
	QCOMPARE(val, 0.5814);

	val = calcHighBorder(1, 0.2, measureCount);
	QCOMPARE(val, 1.4186);
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyFormulaTests::test_calcMetrologyUcertainty()
{
	double val = 0;

	//
	//
	val = calcUcertainty1(2, 3, 4, 5, 6);
	val = round(val*100000)/100000;
	QCOMPARE(val, 24.84619);

	val = calcUcertainty2(2, 3, 4, 5);
	val = round(val*100000)/100000;
	QCOMPARE(val, 9.5219);

	val = calcUcertainty3(2, 3, 4, 5);
	val = round(val*100000)/100000;
	QCOMPARE(val, 8.1035);

	val = calcUcertainty4(2, 3, 4, 5, 6, 7);
	val = round(val*100000)/100000;
	QCOMPARE(val, 25.17274);
}

// -------------------------------------------------------------------------------------------------------------------

