#pragma once

#include "SoftwareCfgGenerator.h"
#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"

namespace Builder
{
	class DASCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		static const int LM_ETHERNET_ADAPTER1 = 1;
		static const int LM_ETHERNET_ADAPTER2 = 2;
		static const int LM_ETHERNET_ADAPTER3 = 3;

		struct LmEthernetAdapterNetworkProperties
		{
			int adapterNo;		// LM_ETHERNET_ADAPTER* values
			QString adapterStrID;

			// only for adapterNo == LM_ETHERNET_ADAPTER1
			//
			bool tuningEnable = true;
			QString tuningIP;
			int tuningPort = 0;
			QString tuningServiceStrID;

			// only for adapterNo == LM_ETHERNET_ADAPTER2 or adapterNo == LM_ETHERNET_ADAPTER3
			//
			bool appDataEnable = true;
			QString appDataIP;
			int appDataPort = 0;
			QString appDataServiceStrID;

			bool diagDataEnable = true;
			QString diagDataIP;
			int diagDataPort = 0;
			QString diagDataServiceStrID;

			bool getLmEthernetAdapterNetworkProperties(Hardware::DeviceModule* lm, int adapterNo, OutputLog *log);
		};

		bool writeSettings();
		bool writeAppSignalsXml();
		bool writeEquipmentXml();
		bool writeDataSourcesXml();

	public:
		DASCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		~DASCfgGenerator();

		virtual bool generateConfiguration();
	};
}
