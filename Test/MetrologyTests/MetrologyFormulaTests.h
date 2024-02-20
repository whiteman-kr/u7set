#ifndef METROLOGY_FORMULA_TESTS_H
#define METROLOGY_FORMULA_TESTS_H

#include <QTest>

// ==============================================================================================

class MetrologyFormulaTests : public QObject
{
	Q_OBJECT

public:

	MetrologyFormulaTests();

private slots:

	void initTestCase();
	void cleanupTestCase();

	void test_calcMetrologyErrors();
	void test_calcMetrologyCharacteristics();
	void test_calcMetrologyUcertainty();
};

// ==============================================================================================

#endif // METROLOGY_FORMULA_TESTS_H
