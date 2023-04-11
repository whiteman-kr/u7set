#pragma once

#include <QRegularExpression>
#include "../CommonLib/HostAddressPort.h"

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
			Bool,
			IpPort
		};
		Q_ENUM(SettingType)
	};

	class Gateway;
	class SignalList;

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

	private:
		std::map<E::Setting, SettingValue> m_settingsValues;
	};

	class Parser
	{
		//

		static const QString SECTION_GATEWAY;
		static const QString SECTION_SIGNAL_LIST;

		static const QString START_LINE_COMMENT;
		static const QString START_MULTILINE_COMMENT;
		static const QString END_MULTILINE_COMMENT;

		static const QString START_SECTION;
		static const QString END_SECTION;

		static const QString EQUAL_SIGN;
		static const QString APP_SIGNAL_ID_START_SIGN;

		static const QString ERR_SYNTAX;

		static const std::map<E::Setting, E::SettingType> m_settingType;

		//

	public:
		enum class ParseResult
		{
			Ok,
			Error,
			CriticalError
		};

		enum class MsgType
		{
			Nothing,
			Message,
			Warning,
			Error
		};

		enum class LineType
		{
			Unknown,
			Section,
			Setting,
			Comment,
			AppSignalID,
			CustomAppSignalID,
		};

		struct ParseLineResult
		{
			int lineNo = 0;
			LineType lineType = LineType::Unknown;

			E::Section section = E::Section::Unknown;
			E::Setting setting = E::Setting::Unknown;

			QVariant value;

			//

			MsgType msgType = MsgType::Nothing;
			QString msg;

			//

			void setError(const QString& err);
			void setWarning(const QString& wrn);
			void setMessage(const QString& msg);
			void clear();
		};

		class Log : public std::vector<std::tuple<int, Parser::MsgType, QString>>
		{
		public:
			void logResult(const Parser::ParseLineResult& plr);

			void logError(int lineNo, const QString& errMsg);
			void logError(const QString& errMsg);

			void logWarning(int lineNo, const QString& wrnMsg);
			void logWarning(const QString& wrnMsg);

			void logRequirtedSettingIsNotSet(int lineNo, E::Setting st);

		private:
			void log(int lineNo, Parser::MsgType msgType, const QString& msg);
		};

	public:
		Parser();
		~Parser();

		bool parse(const QString& desc);

		const Log& log() const;

	private:
		ParseResult parseUnknownSection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseGatewaySection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseSignalListSection(E::Section& parsingSection, const ParseLineResult& plr);

		bool parseLine(const QString& str, ParseLineResult* plr);
		bool parseSettingValue(E::Setting setting, const QString& valueStr, ParseLineResult* plr);

		bool parseIntValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseBoolValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseIpPortValueStr(const QString& valueStr, ParseLineResult* plr);

		Gateway* createApropriateGateway(E::GatewayType gwType);

		void clear();

	private:
		Log m_log;
		std::vector<Gateway*> m_gateways;

		//

		bool m_multilineCommentStarted = false;

		QStringList m_knownSections;
		QStringList m_knownSettings;

		static const QRegularExpression m_appSignalIdTemplate;
		static const QRegularExpression m_anyWhitespaceTemplate;
	};

	class SignalList
	{
	public:
		bool setSettingValue(int lineNo, E::Setting st, const QVariant& value);
		bool settingIsSet(E::Setting st) const;

		virtual bool isKnownSetting(E::Setting st) const;
		virtual bool checkAndApplySettings(int lineNo, Parser::Log& log);

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
		virtual bool checkAndApplySettings(int lineNo, Parser::Log& log);

		virtual void appendSignalList();

		static bool checkRequiredSettings(const std::set<E::Setting> reqSettings,
										  const SettingsValues& settingsValues,
										  int lineNo, Parser::Log& log);
	protected:
		E::GatewayType m_gatewayType = E::GatewayType::Unknown;
		QString m_gatewayID;
		QString m_gatewayDescription;

		SettingsValues m_settingsValues;

		std::vector<SignalList*> m_signalLists;

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
		virtual bool checkAndApplySettings(int lineNo, Parser::Log& log) override;

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
		virtual bool checkAndApplySettings(int lineNo, Parser::Log& log) override;

		virtual void appendSignalList() override;

	private:
		int m_systemID = 0;
		HostAddressPort m_gatewayIP1;
		HostAddressPort m_gatewayIP2;
		int m_listsVersion = 0;
		int m_period = 1000;
	};
}
