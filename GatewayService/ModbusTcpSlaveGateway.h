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
			E::ModbusSignalFormat signalsFormat = E::ModbusSignalFormat::Unknown;
			E::ModbusByteOrder byteOrder = E::ModbusByteOrder::Unknown;

			bool isValid() const
			{
				return signalsFormat != E::ModbusSignalFormat::Unknown &&
					   byteOrder != E::ModbusByteOrder::Unknown;
			}

			bool isDiscretes() const
			{
				return signalsFormat == E::ModbusSignalFormat::DiscreteUint16;
			}
		};

	public:
		ModbusSignalList();

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySetting(int lineNo, E::Setting st, const QVariant& value, ParserLog& log) override;
		virtual bool appendAddressSignalID(const QString& addressStr, const QString& signalID, QString* errMsg) override;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

		virtual void writeSignalsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSignalsFromXml(XmlReadHelper& xml) override;

		bool checkAndApplySignalsFormat(int lineNo, QString formatStr, ParserLog& log);		// copy str Ok!

	private:
		ModbusFormat m_modbusFormat;

		std::set<Hash> m_existsSignals;
		std::map<Address16, QString> m_signals;		// Address16 => AppSignalID (or CustomAppSignalID)
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
