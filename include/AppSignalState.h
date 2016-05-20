#pragma once

#include <QtGlobal>
#include "../Proto/serialization.pb.h"
#include "../include/Hash.h"


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


struct AppSignalState
{
	Times time;

	AppSignalStateFlags flags;

	double value = 0;

	void setProtoAppSignalState(Hash hash, Proto::AppSignalState* protoState);
	Hash getProtoAppSignalState(const Proto::AppSignalState* protoState);
};

