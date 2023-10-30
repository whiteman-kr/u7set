#include <QCoreApplication>
#include <QTest>

#include "UnitsConverterTests.h"
#include "MetrologyFormulaTests.h"

int main(int argc, char *argv[])
{
	int status = 0;

	{
		UnitsConverterTests uc;
		status |= QTest::qExec(&uc, argc, argv);
	}

	{
		MetrologyFormulaTests mf;
		status |= QTest::qExec(&mf, argc, argv);
	}

	return status;
}
