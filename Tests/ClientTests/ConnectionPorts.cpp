#include "ConnectionPorts.h"


Sim::Profile g_profile;
ConnectionPorts g_connectionPorts;


std::pair<bool, QString> ConnectionPorts::init(Sim::Profile profile)
{
	std::pair<bool, QString> result{true, {}};

	if (profile.isEmpty() == true)
	{
		result.first = false;
		result.second = QString{"Profile is empty."};
		return result;
	}

	auto getProperty = [](const Sim::ProfileProperties& props, const QString& property) -> int
		{
			auto it = props.properties.find(property);
			if (it == props.properties.end())
			{
				QString message = QString("Cannot find profile property %1.%2.")
									.arg(props.equipmentId)
									.arg(property);
				throw std::logic_error{message.toStdString()};
			}

			const QVariant& var = it->second;
			if (var.canConvert<int>() == false)
			{
				QString message = QString("Profile property %1.%2 cannot be converted to integer.")
								  .arg(props.equipmentId)
								  .arg(property);
				throw std::logic_error{message.toStdString()};
			}

			return var.toInt();
		};

	try
	{
		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS01_CFGS_RC1");
			cfgService1.clientRequestPort = getProperty(props, "ClientRequestPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS02_CFGS_RC1");
			cfgService2.clientRequestPort = getProperty(props, "ClientRequestPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS01_ADS");
			ads1.appDataReceivingPort = getProperty(props, "AppDataReceivingPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS01_ADS_RC1");
			ads1.clientRequestPort = getProperty(props, "ClientRequestPort");
			ads1.rtTrendsRequestPort = getProperty(props, "RtTrendsRequestPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS02_ADS");
			ads2.appDataReceivingPort = getProperty(props, "AppDataReceivingPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS02_ADS_RC1");
			ads2.clientRequestPort = getProperty(props, "ClientRequestPort");
			ads2.rtTrendsRequestPort = getProperty(props, "RtTrendsRequestPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS01_TUNS");
			tuningService1.clientRequestPort = getProperty(props, "ClientRequestPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS02_TUNS");
			tuningService2.clientRequestPort = getProperty(props, "ClientRequestPort");
		}

		{
			Sim::ProfileProperties props = profile.properties("SYSTEMID_CLIENTTEST_WS04_TUNS");
			tuningService3.clientRequestPort = getProperty(props, "ClientRequestPort");
		}
	}
	catch (std::exception& e)
	{
		result.first = false;
		result.second = QString::fromStdString(e.what());
	}

	return result;
}
