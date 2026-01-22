#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "GatewayParserLog.h"

class XmlWriteHelper;
class XmlReadHelper;

namespace Gateway
{
	//

	class E : public QObject
	{
		Q_OBJECT
	public:

		enum class GatewayType
		{
			Unknown,
			IVS_Impulse,
			ModbusSlave,
			AdsGateway
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
			Enable,
			UniqSignalsInAllLists,
			UniqSignalsInList,

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

			// ModbusSlave specific settings

			ModbusDeviceID,
			SignalsFormat,
			ModbusMode,

			// AdsGatewaay

			ClientRequestIP1
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

		enum class ModbusSignalFormat
		{
			Unknown,
			DiscreteBit,
			AnalogFloat16,
			AnalogFloat32,
			AnalogSInt16,
			AnalogSInt32,
		};
		Q_ENUM(ModbusSignalFormat)

		enum class ModbusByteOrder
		{
			Unknown,
										// Byte order in 16-bit registers, low addr to high addr
										//
										// 32-bit value: 0x44332211		|  16-bit value: 0x2211
										//								|  LE_ByteSwap equal to BE,
										//	  reg[0]	  reg[1]		|  BE_ByteSwap equal to LE
										//  LSB   MSB   LSB   MSB		|   LSB   MSB
			LE,							// [0x11 0x22] [0x33 0x44]		|  [0x11 0x22]
			LE_ByteSwap,				// [0x22 0x11] [0x44 0x33]		|  [0x22 0x11]
			BE,							// [0x44 0x33] [0x22 0x11]		|  [0x22 0x11]
			BE_ByteSwap,				// [0x33 0x44] [0x11 0x22]		|  [0x11 0x22]
		};
		Q_ENUM(ModbusByteOrder)

		enum class ModbusMode
		{
			Unknown,
			//ASCII,					// ASCII character mode, packets starts with ':', ends with CR+LF
			//RTU,						// binary mode RTU
			TCP,						// TCP (RTU with TCP header)
			UDP_ASCII,					// ASCII protocol over UDP
			UDP_ASCII_KZ_UIK			// ASCII protocol over UDP specific for UIK system on Kozloduy AES
		};
		Q_ENUM(ModbusMode)
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

	struct SettingValue
	{
		int lineNo = 0;
		E::Setting setting = E::Setting::Unknown;
		QVariant value;

			   //

		SettingValue() {}
		QString settingName() const { return ::E::valueToString<E::Setting>(setting); }
		bool isValid() const { return setting != E::Setting::Unknown; }
	};

	class SettingsSet
	{
	public:
		SettingsSet();
		virtual ~SettingsSet();

		void appendRequiredSetting(E::Setting reqSetting);
		void appendRequiredSettings(const std::vector<E::Setting>& reqSettings);

		void appendOptionalSetting(E::Setting optSetting);
		void appendOptionalSettings(const std::vector<E::Setting>& optSettings);

		bool isKnownSetting(E::Setting st) const;
		bool settingIsSet(E::Setting st) const;

		//

		const std::map<E::Setting, SettingValue>& settingsValues() const;

		ParseResult setSettingValue(int lineNo, E::Setting st, const QVariant& value, ParserLog* log);
		bool setSettingValue(E::Setting st, const QVariant& value);
		const SettingValue& getSettingValue(E::Setting st) const;

		QString settingName(E::Setting st) const;

		bool isSettingsChecked() const;

		ParseResult checkAndApplySettings(int lineNo, ParserLog& log);

	protected:
		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log);

	private:
		ParseResult checkRequiredSettings(int lineNo, ParserLog& log);

	private:
		std::set<E::Setting> m_requiredSettings;
		std::set<E::Setting> m_optionalSettings;

		std::map<E::Setting, SettingValue> m_settingsValues;

		bool m_settingsChecked = false;

		inline static const SettingValue m_invalidSettingValue;
	};

	class SignalList : public SettingsSet
	{
	public:
		SignalList();
		virtual ~SignalList() = default;

		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log) override;

		virtual ParseResult checkSignalTypeAndFormat(int lineNo, const AppSignal* appSignal, ParserLog& log);
		virtual ParseResult appendSignalID(int lineNo, const QString& appSignalID, ParserLog& log);
		virtual ParseResult parseAddressStr(int lineNo, const QString& addStr, Address16* addr, ParserLog& log);
		virtual ParseResult appendAddressSignalID(int lineNo, const Address16& addr16, const QString& appSignalID, ParserLog& log);
		virtual ParseResult appendAddressConstValue(int lineNo, const Address16& addr16, const QString& desc, double constValue, ParserLog& log);

		std::optional<::E::SignalType> signalType() const;
		void setSignalType(::E::SignalType st);

		const std::vector<QString>& signalIDs() const;
		int signalsCount() const;

		virtual void fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const;

		void writeToXml(XmlWriteHelper& xml) const;
		bool readFromXml(XmlReadHelper& xml);

	protected:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const;
		virtual bool readSettingsFromXml(XmlReadHelper& xml);

		virtual void writeSignalsToXml(XmlWriteHelper& xml) const;
		virtual bool readSignalsFromXml(XmlReadHelper& xml);

	protected:

		std::vector<QString> m_signalIDs;				// AppSignalIDs
		std::optional<::E::SignalType> m_signalType;

		bool m_uniqSignalsInList = false;
		std::set<Hash> m_existSignals;					// hashes of AppSignalIDs

		friend class Parser;
	};

	using SignalListShared = std::shared_ptr<SignalList>;
	using SignalLists = std::vector<SignalListShared>;

	class Gateway : public SettingsSet
	{
	public:
		Gateway();
		Gateway(E::GatewayType gwType);
		virtual ~Gateway() = default;

		static std::shared_ptr<Gateway> createTypedGateway(E::GatewayType gwType);

		E::GatewayType gatewayType() const;
		QString gatewayID() const;
		QString gatewayDescription() const;
		bool enable() const;
		bool uniqSignalsInAllLists() const;

		int signalListsCount() const;

		virtual ParseResult checkAndApplySetting(const SettingValue& sv, ParserLog& log) override;

		virtual void appendSignalList();

		const SignalLists& signalLists() const;

		int signalsCount() const;

		const std::vector<File>& files() const;

		void fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const;

		void writeToXml(XmlWriteHelper& xml) const;
		static std::shared_ptr<Gateway> readFromXml(XmlReadHelper& xml);	// returns typedGateway

	protected:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const;
		virtual bool readSettingsFromXml(XmlReadHelper& xml);

		virtual void writeSignalListsToXml(XmlWriteHelper& xml) const;
		virtual bool readSignalListsFromXml(XmlReadHelper& xml);

		virtual bool generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log);

	private:
		void initSettingsSet();

	protected:
		E::GatewayType m_gatewayType = E::GatewayType::Unknown;

		SignalLists m_signalLists;
		std::vector<File> m_files;

		friend class Parser;
	};

	using GatewayShared = std::shared_ptr<Gateway>;

	class Gateways
	{
	public:
		void append(GatewayShared gw);
		void replaceLast(GatewayShared gw);
		GatewayShared last();
		bool isUniqGatewayID(const QString& gwID, GatewayShared excludeGw) const;

		std::vector<GatewayShared>::iterator begin();
		std::vector<GatewayShared>::iterator end();

		std::vector<GatewayShared>::const_iterator begin() const;
		std::vector<GatewayShared>::const_iterator end() const;

		void clear();

		void fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const;

		virtual void writeToXml(XmlWriteHelper& xml) const;
		virtual bool readFromXml(XmlReadHelper& xml, bool skipDisabledGateways, QStringList* disabledGateways);

	private:
		std::vector<GatewayShared> m_gateways;
	};

	using GatewaysShared = std::shared_ptr<Gateways>;
}
