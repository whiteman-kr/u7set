#ifndef MAIN_H
#define MAIN_H

#pragma pack(1)
struct TuningClientSharedData
{
	int version = 1;
	bool showCommand = 0;
};
#pragma pack()

extern QSharedMemory* theSharedMemorySingleApp;

#endif // MAIN_H
