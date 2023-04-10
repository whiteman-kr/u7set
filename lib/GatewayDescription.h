#pragma once

#include <QRegularExpression>
#include "../CommonLib/HostAddressPort.h"

class GwParserLog;

class GatewayDescriptionParser : public QObject
{
	Q_OBJECT

public:
	enum class MsgType
	{
		Nothing,
		Message,
		Warning,
		Error
	};

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

		Section section = Section::Unknown;
		Setting setting = Setting::Unknown;

		QVariant value;

		//

		MsgType msgType = MsgType::Nothing;
		QString msg;

		void setError(const QString& err);
		void setWarning(const QString& wrn);
		void setMessage(const QString& msg);
		void clear();
	};

	//

	class SignalList
	{
	public:
		std::map<Setting, QVariant> settingValue;
		std::vector<QString> signalIDs;

		void setSettingValue(const ParseLineResult& plr);
		bool settingIsSet(Setting st) const;

		virtual bool prepare(int lineNo, GwParserLog* log);
	};

	class Gateway
	{
	public:
		Gateway();
		Gateway(GatewayType gwType);
		~Gateway();

		static Gateway* createGateway(GatewayType gwType);

		bool setSettingValue(const ParseLineResult& plr);
		bool settingIsSet(Setting st) const;

		virtual bool isKnownGatewaySetting(Setting st) const;
		virtual bool isKnownSignalListSetting(Setting st) const;

		virtual void appendSignalList();

		virtual bool prepare(int lineNo, GwParserLog* log);

	public:
		GatewayType gatewayType = GatewayType::Unknown;
		QString gatewayID;
		QString gatewayDescription;

		std::map<Setting, QVariant> settingValue;

		std::vector<SignalList*> signalLists;
	};

	// IVS_Impulse gateway structs

	class IVS_Impulse_SignalList : public SignalList
	{
	public:
		static const std::set<Setting> requiredSettings;

	public:
		IVS_Impulse_SignalList();
		virtual bool prepare(int lineNo, GwParserLog* log) override;

	public:
		enum class DataType
		{
			Unknown,

			Analog_A,			// Analog parameters, format 'A'
			Discrete_B			// Discrete packed parameters, format 'B'
		};

		int listNo;
		DataType dataType;
		bool sendEvents;
		bool includeAppSignalID;
	};

	class IVS_Impulse_Gateway : public Gateway
	{
	public:
		static const std::set<Setting> requiredSettings;

	public:
		IVS_Impulse_Gateway();

		virtual bool isKnownGatewaySetting(Setting st) const override;
		virtual bool isKnownSignalListSetting(Setting st) const override;

		virtual void appendSignalList() override;

		virtual bool prepare(int lineNo, GwParserLog* log) override;

	public:
		int systemID = 0;
		HostAddressPort gatewayIP1;
		HostAddressPort gatewayIP2;
		int listsVersion = 0;
		int period = 1000;
	};


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

	static const std::map<Setting, SettingType> m_settingType;

	enum class ParseResult
	{
		Ok,
		Error,
		CriticalError
	};

public:
	GatewayDescriptionParser();
	~GatewayDescriptionParser();

	bool parse(const QString& desc, GwParserLog* log);

private:
	ParseResult parseUnknownSection(Section& parsingSection, const ParseLineResult& plr, GwParserLog* log);
	ParseResult parseGatewaySection(Section& parsingSection, const ParseLineResult& plr, GwParserLog* log);
	ParseResult parseSignalListSection(Section& parsingSection, const ParseLineResult& plr, GwParserLog* log);

	bool parseLine(const QString& str, ParseLineResult* plr);
	bool parseSettingValue(Setting setting, const QString& valueStr, ParseLineResult* plr);

	bool parseIntValueStr(const QString& valueStr, ParseLineResult* plr);
	bool parseBoolValueStr(const QString& valueStr, ParseLineResult* plr);
	bool parseIpPortValueStr(const QString& valueStr, ParseLineResult* plr);

	Gateway* createApropriateGateway(GatewayType gwType);

private:
	bool m_multilineCommentStarted = false;

	QStringList m_knownSections;
	QStringList m_knownSettings;

	static const QRegularExpression m_appSignalIdTemplate;
	static const QRegularExpression m_anyWhitespaceTemplate;

	std::vector<Gateway*> m_gateways;
};

class GwParserLog : public std::vector<std::tuple<int, GatewayDescriptionParser::MsgType, QString>>
{
public:
	void logResult(const GatewayDescriptionParser::ParseLineResult& plr);

	void logError(int lineNo, const QString& errMsg);
	void logError(const QString& errMsg);

	void logWarning(int lineNo, const QString& wrnMsg);
	void logWarning(const QString& wrnMsg);

private:
	void log(int lineNo, GatewayDescriptionParser::MsgType msgType, const QString& msg);
};

