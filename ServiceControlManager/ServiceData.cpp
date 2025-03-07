#include "../UtilsLib/WUtils.h"
#include "ServiceData.h"

// --------------------------------------------------------------------------------------
//
//	ServiceData struct implementation
//
// --------------------------------------------------------------------------------------

ServiceData::ServiceData()
{
	protoServiceInfo.mutable_softwareinfo()->set_softwaretype(E::SoftwareType::BaseService);
	protoServiceInfo.set_servicestate(TO_INT(E::ServiceState::Undefined));
}

E::ServiceState ServiceData::serviceState() const
{
	return static_cast<E::ServiceState>(protoServiceInfo.servicestate());
}

bool ServiceData::parseProtoServiceInfo()
{
	buildInfo.loadFromProto(protoServiceInfo.buildinfo());
	swInfo.serializeFrom(protoServiceInfo.softwareinfo());
	sessionParams.loadFrom(protoServiceInfo.sessionparams());

	QString settingsXml = QString::fromStdString(protoServiceInfo.settingsxml());

	if (settings == nullptr)
	{
		settings = SoftwareSettingsSet::createAppropriateSettings(type);
	}

	SoftwareSettings* pSettings = settings.get();
	SoftwareSettingsSet::readSettingsFromXmlString(settingsXml, settings.get());

	clientRequestIPs.clear();

	switch (type)
	{
	case E::SoftwareType::ConfigurationService:
	{
		CfgServiceSettings* cfgServiceSettings = dynamic_cast<CfgServiceSettings*>(pSettings);

		TEST_PTR_RETURN_FALSE(cfgServiceSettings);

		fillClientRequestIPs(cfgServiceSettings->rcSettings);
	}
	break;

	case E::SoftwareType::AppDataService:
	{
		AppDataServiceSettings* adsSettings = dynamic_cast<AppDataServiceSettings*>(pSettings);

		TEST_PTR_RETURN_FALSE(adsSettings);

		fillClientRequestIPs(adsSettings->rcSettings);
	}
	break;

	case E::SoftwareType::TuningService:
	{
		TuningServiceSettings* tuningDataServiceSettings = dynamic_cast<TuningServiceSettings*>(pSettings);

		TEST_PTR_RETURN_FALSE(tuningDataServiceSettings);

		// TO DO 2ch tuning!
		//
		clientRequestIPs.emplace_back(tuningDataServiceSettings->clientRequestIP);
	}
	break;

	case E::SoftwareType::ArchiveService:
	{
		ArchivingServiceSettings* archivingServiceSettings = dynamic_cast<ArchivingServiceSettings*>(pSettings);

		TEST_PTR_RETURN_FALSE(archivingServiceSettings);

		clientRequestIPs.emplace_back(archivingServiceSettings->clientRequestIP);
	}
	break;

	case E::SoftwareType::DiagDataService:
	{
		DiagDataServiceSettings* diagDataServiceSettings = dynamic_cast<DiagDataServiceSettings*>(pSettings);

		TEST_PTR_RETURN_FALSE(diagDataServiceSettings);

		clientRequestIPs.emplace_back(diagDataServiceSettings->clientRequestIP);
	}
	break;

	case E::SoftwareType::GatewayService:
	{
		GatewayServiceSettings* gatewayServiceSettings = dynamic_cast<GatewayServiceSettings*>(pSettings);

		TEST_PTR_RETURN_FALSE(gatewayServiceSettings);

		clientRequestIPs.clear();
	}
	break;

	default:
		Q_ASSERT(false);
	}

	return true;
}

void ServiceData::fillClientRequestIPs(const std::vector<RqCtrlSettings>& rcSettings)
{
	clientRequestIPs.clear();

	for(const RqCtrlSettings& rcs : rcSettings)
	{
		clientRequestIPs.emplace_back(rcs.clientRequestIP());
	}
}

E::ServiceState serviceState(const Network::ServiceInfo& protoServiceInfo)
{
	return static_cast<E::ServiceState>(protoServiceInfo.servicestate());
}

