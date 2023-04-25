#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::IvsImpulseHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	IvsImpulseHandler::IvsImpulseHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 IvsImpulseGatewayShared gateway,
										 const AppSignals& appSignals) :
		Handler(swInfo, settings),
		m_gateway(gateway),
		m_appSignals(appSignals)
	{
		m_appDataServiceClient =
				new AppDataServiceClient(swInfo,
										 settings.appDataService1.address,
										 settings.appDataService2.address,
										 QString("GatewayService %1").arg(swInfo.equipmentID()));
	}

	bool IvsImpulseHandler::init()
	{
		return true;
	}

	void IvsImpulseHandler::run()
	{

	}

	void IvsImpulseHandler::shutdown()
	{

	}
}
