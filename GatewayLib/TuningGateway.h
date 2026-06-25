#pragma once

#include "GatewayDescription.h"
#include <CommonLib/Types.h>
#include <CommonLib/HostAddressPort.h>

namespace Gateway
{
	class TuningGatewaySignalList : public SignalList
	{
	public:
		TuningGatewaySignalList();
		TuningGatewaySignalList(const QString& profile, const QStringList& signalList);
		virtual ~TuningGatewaySignalList();

		virtual bool isListForProfile(const QString& profile) const override;

		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

	private:
		QString m_profile;
	};

	class TuningGateway : public Gateway
	{
	public:
		TuningGateway();
		virtual ~TuningGateway() = default;

		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log) override;

		HostAddressPort clientRequestIP1() const;

		void getRequiredSignalsHashes(std::set<Hash>* hashes) const;
		void getEventSignalsHashes(std::set<Hash>* hashes) const;

		void setRequiredSignalHashes(std::set<Hash>& hashes);

		void appendSignalList(const QString& profile, const QStringList& signalList);

		virtual void appendSignalList() override;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

	private:
		virtual bool generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log) override;

	private:
		HostAddressPort m_clientRequestIP1;
		std::set<Hash> m_signalHashes;
	};

	using TuningGatewayShared = std::shared_ptr<TuningGateway>;
}
