#ifndef GATEWAY_LIB_DOMAIN
#error Do not include this file in the project! Link GatewayLib instead.
#endif

#include <CommonLib/Types.h>
#include <AdsGatewayLib/AdsGwProtocol.hpp>

#include "AdsGateway.h"

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::ModbusTcpSlaveGateway implementation
	//
	// ---------------------------------------------------------------------------------

	AdsGateway::AdsGateway() :
		Gateway(E::GatewayType::AdsGateway)
	{
		appendRequiredSettings({E::Setting::ClientRequestIP1});
	}

	ParseResult AdsGateway::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		ParseResult pr = ParseResult::Ok;

		HostAddressPort addrPort;

		switch(sv.setting)
		{
		default:
			pr = Gateway::checkAndApplySetting(sv, log);
			break;

		case E::Setting::ClientRequestIP1:
			addrPort.setAddressPortStr(sv.value.toString(), AdsGatewayLib::ADSGW_PORT);
			m_clientRequestIP1 = addrPort;
			break;
		}

		return pr;
	}

	HostAddressPort AdsGateway::clientRequestIP1() const
	{
		return m_clientRequestIP1;
	}

	void AdsGateway::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		Q_UNUSED(hashes);
		// TEST_PTR_RETURN(hashes);

		// for(const auto& [addr16, mbSignal] : m_modbusSignals)
		// {
		// 	if (mbSignal.isConst == false)
		// 	{
		// 		hashes->emplace(calcHash(mbSignal.signalID));
		// 	}
		// }
	}

	void AdsGateway::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		Q_UNUSED(hashes);
		// TEST_PTR_RETURN(hashes);

		// for(const auto& [addr16, mbSignal] : m_modbusSignals)
		// {
		// 	if (mbSignal.isConst == false && mbSignal.format.isDiscrete())
		// 	{
		// 		hashes->emplace(calcHash(mbSignal.signalID));
		// 	}
		// }
	}

	void AdsGateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SETTINGS);

		xml.writeIPv4PortAttribute(XmlAttribute::CLIENT_REQUEST_IP1, m_clientRequestIP1);

		xml.writeEndElement();		//	</Settings>
	}

	bool AdsGateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SETTINGS);

		//

		m_clientRequestIP1.clear();

		result &= xml.readIPv4PortAttribute(XmlAttribute::CLIENT_REQUEST_IP1, &m_clientRequestIP1);

		return result;
	}

	void AdsGateway::writeSignalListsToXml(XmlWriteHelper& xml) const
	{
		Q_UNUSED(xml);
		return;
	}

	bool AdsGateway::readSignalListsFromXml(XmlReadHelper& xml)
	{
		Q_UNUSED(xml);
		return true;
	}

	bool AdsGateway::generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log)
	{
		Q_UNUSED(signalSet);
		Q_UNUSED(log);

		// m_files.clear();

		// bool result = true;

		// result = buildModbusSignalsList(log);

		// RETURN_IF_FALSE(result);

		// result = generateModbusSignalsFile();

		// return result;

		return true;
	}
}
