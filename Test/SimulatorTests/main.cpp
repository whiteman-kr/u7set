#include <QtTest>
#include <SimRamTests.h>
#include <SimCommandTest_LM5_LM6.h>
#include <SimProfilesTest.h>
#include "../Protobuf/google/protobuf/message.h"

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

	{
		SimProfilesTest spt;
		status |= QTest::qExec(&spt, argc, argv);
	}

	google::protobuf::ShutdownProtobufLibrary();

	return status;
}
