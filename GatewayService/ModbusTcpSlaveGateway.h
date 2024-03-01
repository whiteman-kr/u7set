#pragma once

#include "GatewayDescription.h"
#include "../CommonLib/Types.h"

namespace Gateway
{
	class ModbusSignalList : public SignalList
	{
	private:
		static const std::set<E::Setting> m_requiredSettings;

		struct ModbusFormat
		{
			::E::ByteOrder byteOrder = ::E::ByteOrder::NoEndian;
			E::ModbusDataFormat dataFormat = E::ModbusDataFormat::Unknown;
		};

	public:
		ModbusSignalList();

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log) override;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

		bool checkAndApplyAnalogFormat(const SettingValue& sv, ParserLog& log);
		bool checkAndApplyDiscreteFormat(const SettingValue& sv, ParserLog& log);

	private:
		ModbusFormat m_commonAnalogFormat;
		ModbusFormat m_commonDiscreteFormat;
	};


	class ModbusTcpSlaveGateway : public Gateway
	{
	public:
		static const std::set<E::Setting> m_requiredSettings;
		static const std::set<E::Setting> m_optionalSettings;

	public:
		ModbusTcpSlaveGateway();
		ModbusTcpSlaveGateway(const QString& gwID, const QString& gwDesc);

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log) override;

		virtual void appendSignalList() override;

		HostAddressPort localGatewayIP1() const;
		HostAddressPort remoteGatewayIP1() const;

		HostAddressPort localGatewayIP2() const;
		HostAddressPort remoteGatewayIP2() const;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

	private:
		virtual bool generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log) override;

		bool checkSignalListsSettings(ParserLog& log);
		bool generateSignalListsFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log);

		bool generateSignalListFile(const ModbusSignalList& signalList,
									File& file,
									const SignalSetAdapter& signalSetAdapter,
									ParserLog& log);

	private:
		HostAddressPort m_localGatewayIP1;
		HostAddressPort m_remoteGatewayIP1;

		HostAddressPort m_localGatewayIP2;
		HostAddressPort m_remoteGatewayIP2;

		E::ModbusCoding m_coding = E::ModbusCoding::ASCII;
	};

	using ModbusTcpSlaveGatewayShared = std::shared_ptr<ModbusTcpSlaveGateway>;
}
