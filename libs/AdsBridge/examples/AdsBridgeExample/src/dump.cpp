#include <AdsBridge/AdsBridge.h>

#include <iostream>
#include <span>
#include <string>


void dumpConnectionStatus(const AdsConnectionStatus& status)
{
	std::string setConnectionResultString;
	switch (status.setConnectionResult)
	{
	case ADS_SET_CONNECTION_RESULT_UNDEFINED:
		setConnectionResultString = "Undefined";
		break;
	case ADS_SET_CONNECTION_RESULT_OK:
		setConnectionResultString = "Ok";
		break;
	case ADS_SET_CONNECTION_RESULT_UNKNOWN_CLIENT_ID:
		setConnectionResultString = "UnknownClientId";
		break;
	case ADS_SET_CONNECTION_RESULT_WRONG_CLIENT_HOST_NAME:
		setConnectionResultString = "WrongClientHostName";
		break;
	case ADS_SET_CONNECTION_RESULT_WRONG_SERVER_ID:
		setConnectionResultString = "WrongServerId";
		break;
	default:
		setConnectionResultString = "Unknown";
		break;
	}

	std::cout << "Connection: " << status.connectionType << "\n";
	std::cout << "\tid: " << std::hex << status.id << std::dec << "\n";
	std::cout << "\tConnection status: " << (status.status ? "Ok" : "NoConnection") << "\n";
	std::cout << "\tSetConnectionResult: " << setConnectionResultString << "\n";
	std::cout << "\tPort: " << status.port << "\n";
	std::cout << "\tAddress: " << status.address << "\n";
	std::cout << "\tEquipment ID: " << status.adsEquipmentId << "\n";
	std::cout << "\tReceived: " << status.received << "\n";
	std::cout << "\tSent: " << status.sent << "\n";
	std::cout << "\tRequest count: " << status.requestCount << "\n";
	std::cout << "\tReply count: " << status.replyCount << "\n";
	return;
}

void dumpConnectionStatus()
{
	std::cout << "------< ConnectionStatus >------\n";

	const auto count = AdsGetTcpConnectionCount();

	auto stats = std::make_unique<AdsConnectionStatus[]>(count);
	bool getOk = AdsGetTcpConnectionStatuses(stats.get(), count);

	if (getOk == true)
	{
		for (const auto& status : std::span{stats.get(), count})
		{
			dumpConnectionStatus(status);
		}
	}

	return;
}

void dumpAppSignalParam(const MatsAppSignalParam& signalParam)
{
	std::cout << "AppSignalParam: " << signalParam.appSignalId << "\n";
	std::cout << "\thash: " << std::hex << signalParam.hash << std::dec << "\n";
	std::cout << "\tcustomSignalId: " << signalParam.customSignalId << "\n";
	std::cout << "\tcaption: " << signalParam.caption << "\n";
	std::cout << "\tequipmentId: " << signalParam.equipmentId << "\n";
	std::cout << "\tlmEquipmentId: " << signalParam.lmEquipmentId << "\n";

	std::cout << "\tunits: " << signalParam.units << "\n";
	std::cout << "\ttags: " << signalParam.tags << "\n";

	std::cout << "\tchannel: " << signalParam.channel << "\n";
	std::cout << "\tinOutType: " << signalParam.inOutType << "\n";
	std::cout << "\ttype: " << signalParam.type << "\n";
	std::cout << "\tdecimalPlaces: " << signalParam.decimalPlaces << "\n";

	std::cout << "\tlowValidRange: " << signalParam.lowValidRange << "\n";
	std::cout << "\thighValidRange: " << signalParam.highValidRange << "\n";

	std::cout << "\ttuning: " << signalParam.tuning << "\n";

	return;
}

void dumpAppSignalState(const MatsAppSignalState& state, std::string_view appSignalId)
{
	std::cout << "AppSignalState: " << state.value << (state.flags & MATS_FLAG_VALID ? " (VALID)" : " (NOT_VALID)");
	if (appSignalId.empty() == false)
	{
		std::cout << " - " << appSignalId;
	}
	std::cout << "\n";

	std::cout << "\thash: " << std::hex << state.hash << std::dec << "\n";

	std::cout << "\tflags: ";
	std::cout << (state.flags & MATS_FLAG_VALID ? "VALID" : "NOT_VALID ");
	std::cout << (state.flags & MATS_FLAG_STATE_AVAILABLE ? "" : "STATE_NOT_AVAILABLE ");
	std::cout << (state.flags & MATS_FLAG_SIMULATED ? "SIMULATED " : "");
	std::cout << (state.flags & MATS_FLAG_BLOCKED ? "BLOCKED " : "");
	std::cout << (state.flags & MATS_FLAG_MISMATCH ? "MISMATCH " : "");
	std::cout << (state.flags & MATS_FLAG_ABOVE_HIGH_LIMIT ? "ABOVE_HIGH_LIMIT " : "");
	std::cout << (state.flags & MATS_FLAG_BELOW_LOW_LIMIT ? "BELOW_LOW_LIMIT " : "");
	std::cout << (state.flags & MATS_FLAG_SW_SIMULATED ? "SW_SIMULATED " : "");
	std::cout << (state.flags & MATS_FLAG_TUNING_DEFAULT ? "TUNING_DEFAULT " : "");
	std::cout << "\n";

	std::cout << "\tplantTime: " << state.plantTime << "\n";
	std::cout << "\tserverTime: " << state.serverTime << "\n";

	return;
}
