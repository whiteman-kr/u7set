#include <QtTest>
#include <SimRamTests.h>

int main(int argc, char *argv[])
{
	int status = 0;

	{
		SimRamTests tc;
		status |= QTest::qExec(&tc, argc, argv);
	}

	return status;
}
