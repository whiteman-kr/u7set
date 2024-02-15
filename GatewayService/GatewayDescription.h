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
			IncludeAppSignalID
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
	};

	class ParserLog;

	struct SettingValue
	{
		int lineNo = 0;
		E::Setting setting = E::Setting::Unknown;
		QVariant value;
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

	// IVS_Impulse gateway structs

	class IvsImpulseSignalList : public SignalList
	{
	private:
		static const std::set<E::Setting> m_requiredSettings;

	public:
		IvsImpulseSignalList();

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log) override;

		int listNo() const;
		E::SignalListDataType dataType() const;
		char dataTypeLetter() const;
		bool sendEvents() const;
		bool includeAppSignalID() const;

	private:
		virtual void writeSettingsToXml(XmlWriteHelper& xml) const override;
		virtual bool readSettingsFromXml(XmlReadHelper& xml) override;

	private:
		int m_listNo;
		E::SignalListDataType m_dataType;
		bool m_sendEvents;
		bool m_includeAppSignalID;
	};

	using IvsImpulseSignalListShared = std::shared_ptr<IvsImpulseSignalList>;

	class IvsImpulseGateway : public Gateway
	{
	public:
		static const std::set<E::Setting> m_requiredSettings;
		static const std::set<E::Setting> m_optionalSettings;

	public:

		struct DataType_ListID
		{
			E::SignalListDataType dataType = E::SignalListDataType::Unknown;
			int listID = 0;
		};

	public:
		IvsImpulseGateway();
		IvsImpulseGateway(const QString& gwID, const QString& gwDesc);

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log) override;

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
		virtual bool generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log) override;

		bool checkSignalListsSettings(ParserLog& log);
		bool generateSignalListsFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log);

		bool generateSignalListFile(const IvsImpulseSignalList& signalList,
									File& file,
									const SignalSetAdapter& signalSetAdapter,
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
