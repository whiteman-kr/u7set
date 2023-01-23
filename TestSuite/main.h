#pragma once

#pragma pack(1)
struct TestSuiteSharedData
{
	int version = 1;
	bool showCommand = false;
};
#pragma pack()

extern QSharedMemory* theSharedMemorySingleApp;
