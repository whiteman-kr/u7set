#pragma once

#include "GatewayDescription.h"
#include "../CommonLib/Types.h"
#include "../CommonLib/HostAddressPort.h"

namespace Gateway
{
	struct ModbusFormat
	{
		E::ModbusSignalFormat signalFormat = E::ModbusSignalFormat::Unknown;
		E::ModbusByteOrder byteOrder = E::ModbusByteOrder::Unknown;

		bool isValid() const;
		bool isDiscrete() const;

		int registersCount() const;

		QString toString() const;
	};

	class ModbusSignalList : public SignalList
	{
	private:
		static const std::set<E::Setting> m_requiredSettings;

	public:
		ModbusSignalList();

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySetting(int lineNo, E::Setting st, const QVariant& value, ParserLog& log) override;
		virtual bool appendAddressSignalID(const QString& addressStr, const QString& signalID, QString* errMsg) override;

		ModbusFormat modbusFormat() const;
		Address16 getAddress(Hash hash) const;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

		bool checkAndApplySignalsFormat(int lineNo, QString formatStr, ParserLog& log);		// copy str Ok!

	private:
		ModbusFormat m_modbusFormat;

		std::map<Hash, Address16> m_signals;		// calcHash(AppSignalID) => Address16
	};

	class ModbusTcpSlaveGateway : public Gateway
	{
	public:
		static const std::set<E::Setting> m_requiredSettings;
		static const std::set<E::Setting> m_optionalSettings;

		inline static const int MODBUS_DEFAULT_PORT = 502;

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

		int modbusDeviceID() const;

		void getRequiredSignalsHashes(std::set<Hash>* hashes) const;
		void getEventSignalsHashes(std::set<Hash>* hashes) const;

		const std::map<Address16, std::pair<QString, ModbusFormat>>& modbusSignals() const;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

		virtual void writeSignalListsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSignalListsFromXml(XmlReadHelper& xml) override;

	private:
		virtual bool generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log) override;

		bool buildModbusSignalsList(ParserLog& log);
		bool generateModbusSignalsFile();

	private:
		HostAddressPort m_localGatewayIP1;
		HostAddressPort m_localGatewayIP2;

		int m_modbusDeviceID = 0;

		std::map<Address16, std::pair<QString, ModbusFormat>> m_modbusSignals;
	};

	using ModbusTcpSlaveGatewayShared = std::shared_ptr<ModbusTcpSlaveGateway>;
}
