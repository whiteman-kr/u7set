#pragma once

#include "GatewayDescription.h"
#include <CommonLib/HostAddressPort.h>

namespace Gateway
{
	class IvsImpulseSignalList : public SignalList
	{
	public:
		IvsImpulseSignalList();

		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log) override;

		int listNo() const;
		E::SignalListDataType dataType() const;
		char dataTypeLetter() const;
		bool sendEvents() const;
		bool includeAppSignalID() const;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

	private:
		int m_listNo{};
		E::SignalListDataType m_dataType{E::SignalListDataType::Unknown};
		bool m_sendEvents{};
		bool m_includeAppSignalID{};
	};

	using IvsImpulseSignalListShared = std::shared_ptr<IvsImpulseSignalList>;

	class IvsImpulseGateway : public Gateway
	{
	public:
		struct DataType_ListID
		{
			E::SignalListDataType dataType = E::SignalListDataType::Unknown;
			int listID = 0;
		};

	public:
		IvsImpulseGateway();
		IvsImpulseGateway(const QString& gwID, const QString& gwDesc, bool enable);

		void initSettings();

		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log) override;

		virtual void appendSignalList() override;

			   //

		int systemID() const;
		HostAddressPort localGatewayIP1() const;
		HostAddressPort remoteGatewayIP1() const;
		HostAddressPort localGatewayIP2() const;
		HostAddressPort remoteGatewayIP2() const;
		int listsVersion() const;
		::E::TimeType timeType() const;
		int period() const;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

	private:
		virtual bool generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log) override;

		bool checkSignalListsSettings(ParserLog& log);
		bool generateSignalListsFiles(const AppSignalSet* signalSet, ParserLog& log);

		bool generateSignalListFile(const IvsImpulseSignalList& signalList,
									File& file,
									const AppSignalSet* signalSet,
									ParserLog& log);

	private:
		int m_systemID = 0;

		HostAddressPort m_localGatewayIP1;
		HostAddressPort m_remoteGatewayIP1;

		HostAddressPort m_localGatewayIP2;
		HostAddressPort m_remoteGatewayIP2;

		int m_listsVersion = 0;
		::E::TimeType m_timeType = ::E::TimeType::Plant;
		int m_period = 1000;
	};

	bool operator < (const IvsImpulseGateway::DataType_ListID& s1,
				   const IvsImpulseGateway::DataType_ListID& s2);

	using IvsImpulseGatewayShared = std::shared_ptr<IvsImpulseGateway>;
}
