#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	IvsImpulseHandler::IvsImpulseHandler()
	{

	}

	bool IvsImpulseHandler::init(IvsImpulseGatewayShared gateway, const AppSignals& appSignals)
	{
		TEST_PTR_RETURN_FALSE(gateway);

		m_gateway = gateway;

		//

		return true;
	}

	void IvsImpulseHandler::run()
	{

	}

	void IvsImpulseHandler::shutdown()
	{

	}
}
