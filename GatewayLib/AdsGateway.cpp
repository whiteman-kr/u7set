#ifndef GATEWAY_LIB_DOMAIN
#error Do not include this file in the project! Link GatewayLib instead.
#endif

#include <CommonLib/Types.h>
#include <GatewayClientLib/AdsGwProtocol.hpp>

#include "AdsGateway.h"

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::AdsGatewaySignalList implementation
	//
	// ---------------------------------------------------------------------------------

	AdsGatewaySignalList::AdsGatewaySignalList()
	{
	}

	AdsGatewaySignalList::AdsGatewaySignalList(const QString& profile, const QStringList& signalList) :
		m_profile(profile)
	{
		m_signalIDs.reserve(signalList.size());

		for(const QString& signalID : signalList)
		{
			m_signalIDs.emplace_back(signalID);
		}
	}

	AdsGatewaySignalList::~AdsGatewaySignalList()
	{
	}

	bool AdsGatewaySignalList::isListForProfile(const QString& profile) const
	{
		return (m_profile == profile);
	}

	void AdsGatewaySignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		xml.writeStringAttribute(XmlAttribute::PROFILE, m_profile);

		xml.writeEndElement();		//	</SignalList>
	}

	bool AdsGatewaySignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LIST);
		result &= xml.readStringAttribute(XmlAttribute::PROFILE, &m_profile);

		return result;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::AdsGateway implementation
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
			addrPort.setAddressPortStr(sv.value.toString(), GatewayClientLib::ADSGW_PORT);
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
		TEST_PTR_RETURN(hashes);
		*hashes = m_signalHashes;
	}

	void AdsGateway::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);
		*hashes = m_signalHashes;
	}

	void AdsGateway::setRequiredSignalHashes(std::set<Hash>& hashes)
	{
		m_signalHashes.swap(hashes);
	}

	void AdsGateway::appendSignalList(const QString& profile, const QStringList& signalList)
	{
		SignalListShared signalListShared = std::make_shared<AdsGatewaySignalList>(profile, signalList);

		m_signalLists.push_back(signalListShared);
	}

	void AdsGateway::appendSignalList()
	{
		SignalListShared signalListShared = std::make_shared<AdsGatewaySignalList>();
		m_signalLists.push_back(signalListShared);
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

	// void AdsGateway::writeSignalListsToXml(XmlWriteHelper& xml) const
	// {
	// 	Q_UNUSED(xml);
	// 	return;
	// }

	// bool AdsGateway::readSignalListsFromXml(XmlReadHelper& xml)
	// {
	// 	Q_UNUSED(xml);
	// 	return true;
	// }

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
