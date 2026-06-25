#ifndef GATEWAY_LIB_DOMAIN
#error Do not include this file in the project! Link GatewayLib instead.
#endif

#include <CommonLib/Types.h>
#include <GatewayClientLib/TuningGwProtocol.hpp>

#include "TuningGateway.h"

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::TuningGatewaySignalList implementation
	//
	// ---------------------------------------------------------------------------------

	TuningGatewaySignalList::TuningGatewaySignalList()
	{
	}

	TuningGatewaySignalList::TuningGatewaySignalList(const QString& profile, const QStringList& signalList) :
		m_profile(profile)
	{
		m_signalIDs.reserve(signalList.size());

		for(const QString& signalID : signalList)
		{
			m_signalIDs.emplace_back(signalID);
		}
	}

	TuningGatewaySignalList::~TuningGatewaySignalList()
	{
	}

	bool TuningGatewaySignalList::isListForProfile(const QString& profile) const
	{
		return (m_profile == profile);
	}

	void TuningGatewaySignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		xml.writeStringAttribute(XmlAttribute::PROFILE, m_profile);

		xml.writeEndElement();		//	</SignalList>
	}

	bool TuningGatewaySignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LIST);
		result &= xml.readStringAttribute(XmlAttribute::PROFILE, &m_profile);

		return result;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::TuningGateway implementation
	//
	// ---------------------------------------------------------------------------------

	TuningGateway::TuningGateway() :
		Gateway(E::GatewayType::TuningGateway)
	{
		appendRequiredSettings({E::Setting::ClientRequestIP1});
	}

	ParseResult TuningGateway::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		ParseResult pr = ParseResult::Ok;

		HostAddressPort addrPort;

		switch(sv.setting)
		{
		default:
			pr = Gateway::checkAndApplySetting(sv, log);
			break;

		case E::Setting::ClientRequestIP1:
			addrPort.setAddressPortStr(sv.value.toString(), GatewayClientLib::TUNING_GW_PORT);
			m_clientRequestIP1 = addrPort;
			break;
		}

		return pr;
	}

	HostAddressPort TuningGateway::clientRequestIP1() const
	{
		return m_clientRequestIP1;
	}

	void TuningGateway::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);
		*hashes = m_signalHashes;
	}

	void TuningGateway::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);
		*hashes = m_signalHashes;
	}

	void TuningGateway::setRequiredSignalHashes(std::set<Hash>& hashes)
	{
		m_signalHashes.swap(hashes);
	}

	void TuningGateway::appendSignalList(const QString& profile, const QStringList& signalList)
	{
		SignalListShared signalListShared = std::make_shared<TuningGatewaySignalList>(profile, signalList);

		m_signalLists.push_back(signalListShared);
	}

	void TuningGateway::appendSignalList()
	{
		SignalListShared signalListShared = std::make_shared<TuningGatewaySignalList>();
		m_signalLists.push_back(signalListShared);
	}

	void TuningGateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SETTINGS);

		xml.writeIPv4PortAttribute(XmlAttribute::CLIENT_REQUEST_IP1, m_clientRequestIP1);

		xml.writeEndElement();		//	</Settings>
	}

	bool TuningGateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SETTINGS);

		//

		m_clientRequestIP1.clear();

		result &= xml.readIPv4PortAttribute(XmlAttribute::CLIENT_REQUEST_IP1, &m_clientRequestIP1);

		return result;
	}

	// void TuningGateway::writeSignalListsToXml(XmlWriteHelper& xml) const
	// {
	// 	Q_UNUSED(xml);
	// 	return;
	// }

	// bool TuningGateway::readSignalListsFromXml(XmlReadHelper& xml)
	// {
	// 	Q_UNUSED(xml);
	// 	return true;
	// }

	bool TuningGateway::generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log)
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
