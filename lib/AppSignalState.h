#pragma once

#include <QtGlobal>
#include "../Proto/serialization.pb.h"
#include "../lib/Hash.h"


struct Times
{
	qint64	system = 0;
	qint64	local = 0;
	qint64	plant = 0;
};


union AppSignalStateFlags
{
	struct
	{
		quint32	valid : 1;
		quint32	overflow : 1;
		quint32	underflow : 1;
	};

	quint32 all;
};


const quint32 VALID_STATE = 1;
const quint32 INVALID_STATE = 0;


struct AppSignalState
{
	Hash hash = 0;					// == calcHash(AppSignalID)

	Times time;

	AppSignalStateFlags flags;

	double value = 0;

	void setProtoAppSignalState(Proto::AppSignalState* protoState);
	Hash getProtoAppSignalState(const Proto::AppSignalState* protoState);
};

