#pragma once

#include <QRegularExpression>
#include "../CommonLib/HostAddressPort.h"
#include "../AppSignalLib/AppSignal.h"

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
			GatewayIP1,
			GatewayIP2,
			ListsVersion,
			Period,

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

	class SignalSetAdapter
	{
	public:
		SignalSetAdapter() = delete;

		SignalSetAdapter(const AppSignalSet* appSignalSet);
		SignalSetAdapter(const AppSignals& appSignals);

		const AppSignal* getAppSignal(const QString& appSignalID) const;

	private:
		const AppSignalSet* m_appSignalSet = nullptr;
		const AppSignals* m_appSignals = nullptr;
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

	protected:
		SettingsValues m_settingsValues;
		std::vector<QString> m_signalIDs;

		friend class Parser;
	};

	class Gateway
	{
	private:
		static const std::set<E::Setting> m_gatewayRequiredSettings;

	public:
		Gateway();
		Gateway(E::GatewayType gwType);
		~Gateway();

		static Gateway* createGateway(E::GatewayType gwType);

		bool setSettingValue(int lineNo, E::Setting st, const QVariant& value);
		bool settingIsSet(E::Setting st) const;

		virtual bool isKnownSetting(E::Setting st) const;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log);

		virtual void appendSignalList();

		const std::vector<File>& files() const;

		static bool checkRequiredSettings(const std::set<E::Setting> reqSettings,
										  const SettingsValues& settingsValues,
										  int lineNo, ParserLog& log);

	protected:
		virtual bool generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log);

	protected:
		E::GatewayType m_gatewayType = E::GatewayType::Unknown;
		QString m_gatewayID;
		QString m_gatewayDescription;

		SettingsValues m_settingsValues;

		std::vector<SignalList*> m_signalLists;
		std::vector<File> m_files;

		friend class Parser;
	};

	// IVS_Impulse gateway structs

	class IVS_Impulse_SignalList : public SignalList
	{
	public:
		enum class DataType
		{
			Unknown,

			Analog_A,			// Analog parameters, format 'A'
			Discrete_B			// Discrete packed parameters, format 'B'
		};

	private:
		static const std::set<E::Setting> m_requiredSettings;

	public:
		IVS_Impulse_SignalList();

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log) override;

		int listNo() const;
		DataType dataType() const;
		char dataTypeLetter() const;
		bool sendEvents() const;
		bool includeAppSignalID() const;

	private:
		int m_listNo;
		DataType m_dataType;
		bool m_sendEvents;
		bool m_includeAppSignalID;
	};

	class IVS_Impulse_Gateway : public Gateway
	{
	public:
		static const std::set<E::Setting> m_requiredSettings;

	public:
		IVS_Impulse_Gateway();

		virtual bool isKnownSetting(E::Setting st) const override;
		virtual bool checkAndApplySettings(int lineNo, ParserLog& log) override;

		virtual void appendSignalList() override;

	private:
		virtual bool generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log) override;

		bool checkSignalListsSettings(ParserLog& log);
		bool generateSignalListsFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log);

		bool generateSignalListFile(const IVS_Impulse_SignalList& signalList,
									File& file,
									const SignalSetAdapter& signalSetAdapter,
									ParserLog& log);

	private:
		int m_systemID = 0;
		HostAddressPort m_gatewayIP1;
		HostAddressPort m_gatewayIP2;
		int m_listsVersion = 0;
		int m_period = 1000;
	};
}
