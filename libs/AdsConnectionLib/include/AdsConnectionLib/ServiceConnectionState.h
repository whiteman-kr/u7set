#pragma once

#include "ServiceEndpoint.h"
#include <Network.pb.h>

// TODO: There is the same enum SetConnectionResult in TcpConnectoinState.h, how to avoid duplication?
//
enum class SetConnectionResult2
{
	Undefined,
	Ok,

	UnknownClientID,
	WrongClientHostname,

	WrongServerID,
};

struct ServiceConnectionState
{
	std::string name;

	bool isSocketConnected = false; // True - if TCP connection is etablished

	bool isConnected = false;       // True - if TCP connection is etablished and
	// requests/reply RQID_SECURITY_LEVEL and RQID_INTRODUCE_MYSELF
	// successfully processed

	// E::SecurityLevel securityLevel = E::SecurityLevel::Basic;
	SetConnectionResult2 setConnectionResult = SetConnectionResult2::Undefined;
	// int connectionNo = 0;

	std::string serverEquipmentID; // Valid only for Tcp::Client, indicates service the client tries connect to.

	// nex data is valid if isConnected == true
	//
	ServiceAddress peerAddr;
	int64_t startTime = 0; // milliseconds since epoch

	int64_t sentBytes = 0;
	int64_t receivedBytes = 0;

	int64_t requestCount = 0;
	int64_t replyCount = 0;

	::Network::SoftwareInfo connectedSoftwareInfo;
	::Network::SoftwareInfo localSoftwareInfo;

	// bool isActual = false;

	// void clear() { *this = ConnectionState(); }

	// void dump();
};
