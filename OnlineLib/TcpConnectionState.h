#pragma once
#include "HostAddressPort.h"
#include "SoftwareInfo.h"

namespace Tcp
{
	enum class SetConnectionResult
	{
		Undefined,
		Ok,

		UnknownClientID,
		WrongClientHostname,

		WrongServerID,
	};

	struct ConnectionState
	{
		bool isSocketConnected = false;					// True - if TCP connection is etablished

		bool isConnected = false;						// True - if TCP connection is etablished and
		// requests/reply RQID_SECURITY_LEVEL and RQID_INTRODUCE_MYSELF
		// successfully processed

		E::SecurityLevel securityLevel = E::SecurityLevel::Basic;
		SetConnectionResult setConnectionResult = SetConnectionResult::Undefined;
		int connectionNo = 0;

		QString serverEquipmentID;						// Valid only for Tcp::Client, indicates service the client tries connect to.

		// nex data is valid if isConnected == true
		//
		HostAddressPort peerAddr;
		qint64 startTime = 0;					// milliseconds since epoch

		qint64 sentBytes = 0;
		qint64 receivedBytes = 0;

		qint64 requestCount = 0;
		qint64 replyCount = 0;

		SoftwareInfo connectedSoftwareInfo;
		SoftwareInfo localSoftwareInfo;

		bool isActual = false;

		void clear() { *this = ConnectionState(); }

		void dump();
	};
} // namespace Tcp