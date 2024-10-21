#pragma once
#include <SimulatorLib/SimProfiles.h>

struct ConnectionPorts
{
	struct CfgServicePorts
	{
		int clientRequestPort{};
	};

	struct AdsPorts
	{
		int appDataReceivingPort{};
		int clientRequestPort{};
		int rtTrendsRequestPort{};
	};

	struct TuningServicePorts
	{
		int clientRequestPort{};
	};

	CfgServicePorts cfgService1 = {.clientRequestPort = 13312};
	CfgServicePorts cfgService2 = {.clientRequestPort = 13313};

	AdsPorts ads1 = {.appDataReceivingPort = 1501, .clientRequestPort = 13323, .rtTrendsRequestPort = 13324};
	AdsPorts ads2 = {.appDataReceivingPort = 1502, .clientRequestPort = 13326, .rtTrendsRequestPort = 13327};

	TuningServicePorts tuningService1 = {.clientRequestPort = 13333};
	TuningServicePorts tuningService2 = {.clientRequestPort = 13334};
	TuningServicePorts tuningService3 = {.clientRequestPort = 13335};

	std::pair<bool, QString> init(Sim::Profile profile);	// Returns result bool and error message if necessary.
};

extern Sim::Profile g_profile;
extern ConnectionPorts g_connectionPorts;
