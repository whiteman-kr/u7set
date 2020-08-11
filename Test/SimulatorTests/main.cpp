#include <QtTest>
#include <SimRamTests.h>
#include <SimCommandTest_LM5_LM6.h>

int main(int argc, char *argv[])
{
	int status = 0;

	{
		SimRamTests tc;
		status |= QTest::qExec(&tc, argc, argv);
	}

	{
		SimCommandTest_LM5_LM6 csm;
		status |= QTest::qExec(&csm, argc, argv);
	}

	return status;
}
