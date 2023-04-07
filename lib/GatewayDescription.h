#pragma once

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
		IVS_Impulse,
	};
	Q_ENUM(GatewayType)

	enum class Section
	{
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
		Description,

		// IVS Impulse specific settings

		SystemID,
		GatewayIP1,
		GatewayIP2,
		ListsVersion,
		Period,
		SendEvents,
		ListNo,
		DataType,
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

private:
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
		LineType lineType = LineType::Unknown;

		Section section;
		Setting setting;

		QVariant value;

		//

		MsgType msgType = MsgType::Nothing;
		QString msg;

		void setError(const QString& err);
	};

	struct Gateway
	{
		GatewayType type;
		QString gatewayID;
		QString description;
	};

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

public:
	GatewayDescriptionParser();

	bool parse(const QString& desc,
			   std::vector<std::tuple<int, GatewayDescriptionParser::MsgType, QString>>* log);

private:
	bool parseLine(const QString& str, ParseLineResult* plr);
	bool parseSettingValue(Setting setting, const QString& valueStr, ParseLineResult* plr);

private:
	bool m_multilineCommentStarted = false;

	QStringList m_knownSections;
	QStringList m_knownSettings;

	ParseLineResult m_syntaxError;
};

