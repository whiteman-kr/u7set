#pragma once

#include <CommonLib/Hash.h>
#include <CommonLib/Times.h>
#include "../UtilsLib/Queue.h"
#include "DiagSignalStateFlags.h"

struct SimpleDiagSignalState
{
	// light version of AppSignalState to use in queues and other AppDataService data structs
	//
	Hash hash = 0;							// == calcHash(DiagSignalID)
	Times time;
	DiagSignalStateFlags flags{};
	double value = 0;

	int attentions = 0;
	int warnings = 0;
	int errors = 0;
	int faults = 0;

	bool isValid() const { return flags.valid == 1; }

//	void save(Proto::AppSignalState* protoState) const;
//	Hash load(const Proto::AppSignalState& protoState);

//	void print() const;

	qint64 plantTime() const { return time.plant.timeStamp; }
	qint64 systemTime() const { return time.system.timeStamp; }
	qint64 localTime() const { return time.local.timeStamp; }
};
