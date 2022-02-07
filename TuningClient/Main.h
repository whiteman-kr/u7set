#pragma once
#include "ScriptTuningClientApplication.h"

#pragma pack(1)
struct TuningClientSharedData
{
	int version = 1;
	bool showCommand = false;
};
#pragma pack()

extern ScriptTuningClientApplication theApp;
extern QSharedMemory* theSharedMemorySingleApp;


