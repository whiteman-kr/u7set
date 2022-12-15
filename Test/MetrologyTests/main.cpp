#include <QCoreApplication>

#include "UnitsConvertorTests.h"
#include "MetrologyFormulaTests.h"

int main(int argc, char *argv[])
{
	int status = 0;

	{
		UnitsConvertorTests uc;
		status |= QTest::qExec(&uc, argc, argv);
	}

	{
		MetrologyFormulaTests mf;
		status |= QTest::qExec(&mf, argc, argv);
	}

	return status;
}
