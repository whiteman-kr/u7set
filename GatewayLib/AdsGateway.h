#pragma once

#include "GatewayDescription.h"
#include <CommonLib/Types.h>
#include <CommonLib/HostAddressPort.h>

namespace Gateway
{
	class AdsGateway : public Gateway
	{
	public:
		AdsGateway();
		virtual ~AdsGateway() = default;

		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log) override;

		HostAddressPort clientRequestIP1() const;

		void getRequiredSignalsHashes(std::set<Hash>* hashes) const;
		void getEventSignalsHashes(std::set<Hash>* hashes) const;

		void setRequiredSignalHashes(std::set<Hash>& stateHashes, std::set<Hash>& eventHashes);

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

		virtual void writeSignalListsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSignalListsFromXml(XmlReadHelper& xml) override;

	private:
		virtual bool generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log) override;

	private:
		HostAddressPort m_clientRequestIP1;

		std::set<Hash> m_stateHashes;
		std::set<Hash> m_eventHashes;
	};

	using AdsGatewayShared = std::shared_ptr<AdsGateway>;
}
