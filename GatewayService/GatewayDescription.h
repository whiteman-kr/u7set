#pragma once

#include <QRegularExpression>
#include "../CommonLib/HostAddressPort.h"

class XmlWriteHelper;
class XmlReadHelper;

namespace Gateway
{
	class E : public QObject
	{
		Q_OBJECT
	public:

		enum class GatewayType
		{
			Unknown,
			IVS_Impulse,
			ModbusTcpSlave
		};
		Q_ENUM(GatewayType)

		enum class Section
		{
			Unknown,
			Gateway,
			SignalList
		};
		Q_ENUM(Section)

		enum class Setting
		{
			Unknown,

			// Common gateway settings

			GatewayType,
			GatewayID,
			GatewayDescription,

			// IVS Impulse specific settings

			SystemID,
			LocalGatewayIP1,
			RemoteGatewayIP1,
			LocalGatewayIP2,
			RemoteGatewayIP2,
			ListsVersion,
			Period,
			TimeType,

			ListNo,
			DataType,
			SendEvents,
			IncludeAppSignalID,

			// ModbusTcpSlave specific settings

			CodingMode,
			ModbusDeviceAddress,
			AnalogFormat,
			DiscreteFormat,
		};
		Q_ENUM(Setting)

		enum class SettingType
		{
			Unknown,
			Int,
			String,
			AlphaNumericUnderlineString,
			Bool,
			IpPort
		};
		Q_ENUM(SettingType)

		enum class SignalListDataType
		{
			Unknown,

			// IVS_Impulse data types
			//
			Analog_A,			// Analog parameters, format 'A'
			Discrete_B,			// Discrete packed parameters, format 'B'
			Discrete_D			// Discrete parameters, format 'D'
		};
		Q_ENUM(SignalListDataType)

		enum class ModbusCoding
		{
			Unknown,
			ASCII,
			RTU
		};
		Q_ENUM(ModbusCoding)

		enum class ModbusDataFormat
		{
			Unknown,
			DiscreteUint16,
			AnalogFloat16,
		};
		Q_ENUM(ModbusDataFormat)
	};

	class ParserLog;

	struct SettingValue
	{
		int lineNo = 0;
		E::Setting setting = E::Setting::Unknown;
		QVariant value;

		QString settingName() const { return ::E::valueToString<E::Setting>(setting); }
	};

	class SettingsValues
	{
	public:
		bool contains(E::Setting st) const;
		bool insert(int lineNo, E::Setting st, const QVariant& value);

		std::map<E::Setting, SettingValue>::const_iterator begin() const;
		std::map<E::Setting, SettingValue>::iterator begin();

		std::map<E::Setting, SettingValue>::const_iterator end() const;
		std::map<E::Setting, SettingValue>::iterator end();

		SettingValue getSettingVaue(E::Setting st) const;

	private:
		std::map<E::Setting, SettingValue> m_settingsValues;
	};

	class File
	{
	public:
		File(E::GatewayType gatewayType, const QString& gatewayID, const QString& fileName);

		const QByteArray& fileData() const;
		QByteArray& mutableFileData();

		QString gatewayID() const;
		QString fileName() const;

	private:
		E::GatewayType m_gatewayType = E::GatewayType::Unknown;
		QString m_gatewayID;
		QString m_fileName;

		QByteArray m_fileData;
	};

	class SignalList
	{
	public:
		bool setSettingValue(int lineNo, E::Setting st, const QVariant& value);
		bool settingIsSet(E::Setting st) const;

		virtual bool isKnownSetting(E::Setting st) const;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log);

		SettingValue getSettingValue(E::Setting st) const;

		const std::vector<QString>& signalIDs() const;
		int signalsCount() const;

		void fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const;

		void writeToXml(XmlWriteHelper& xml) const;
		bool readFromXml(XmlReadHelper& xml);

	protected:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const;
		virtual bool readSettingsFromXml(XmlReadHelper& xml);

		virtual void writeSignalsToXml(XmlWriteHelper& xml) const;
		virtual bool readSignalsFromXml(XmlReadHelper& xml);

	protected:
		SettingsValues m_settingsValues;
		std::vector<QString> m_signalIDs;

		friend class Parser;
	};

	using SignalListShared = std::shared_ptr<SignalList>;
	using SignalLists = std::vector<SignalListShared>;

	class SignalSetAdapter;

	class Gateway
	{
	private:
		static const std::set<E::Setting> m_gatewayRequiredSettings;

	public:
		Gateway();
		Gateway(E::GatewayType gwType);
		Gateway(E::GatewayType gwType, const QString& gwID, const QString& gwDesc);
		~Gateway();

		E::GatewayType gatewayType() const;
		QString gatewayID() const;
		QString gatewayDescription() const;
		int signalListsCount() const;

		bool setSettingValue(int lineNo, E::Setting st, const QVariant& value);
		bool settingIsSet(E::Setting st) const;

		virtual bool isKnownSetting(E::Setting st) const;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log);

		virtual void appendSignalList();

		const SignalLists& signalLists() const;

		int signalsCount() const;

		const std::vector<File>& files() const;

		static bool checkRequiredSettings(const std::set<E::Setting> reqSettings,
										  const SettingsValues& settingsValues,
										  int lineNo, ParserLog& log);

		void fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const;

		void writeToXml(XmlWriteHelper& xml) const;
		bool readFromXml(XmlReadHelper& xml);

	protected:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const;
		virtual bool readSettingsFromXml(XmlReadHelper& xml);

		void writeSignalListsToXml(XmlWriteHelper& xml) const;
		bool readSignalListsFromXml(XmlReadHelper& xml);

		virtual bool generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log);

	protected:
		E::GatewayType m_gatewayType = E::GatewayType::Unknown;
		QString m_gatewayID;
		QString m_gatewayDescription;

		SettingsValues m_settingsValues;

		SignalLists m_signalLists;
		std::vector<File> m_files;

		friend class Parser;
	};

	using GatewayShared = std::shared_ptr<Gateway>;

	class Gateways
	{
	public:
		void append(GatewayShared gw);
		void setLast(GatewayShared gw);
		GatewayShared last();

		std::vector<GatewayShared>::iterator begin();
		std::vector<GatewayShared>::iterator end();

		std::vector<GatewayShared>::const_iterator begin() const;
		std::vector<GatewayShared>::const_iterator end() const;

		void clear();

		GatewayShared createTypedGateway(E::GatewayType gwType,
										 const QString& gwID,
										 const QString& gwDesc);

		void fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const;

		virtual void writeToXml(XmlWriteHelper& xml) const;
		virtual bool readFromXml(XmlReadHelper& xml);

	private:
		std::vector<GatewayShared> m_gateways;
	};

	using GatewaysShared = std::shared_ptr<Gateways>;
}
