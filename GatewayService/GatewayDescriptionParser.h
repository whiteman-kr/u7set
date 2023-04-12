#pragma once

#include "GatewayDescription.h"

namespace Gateway
{
	enum class LogMsgType
	{
		Nothing,
		Message,
		Warning,
		Error
	};

	struct LogRecord
	{
		int lineNo;
		LogMsgType msgType;
		QString msg;
	};

	class ParserLog : public std::vector<LogRecord>
	{
	public:
		void logResult(int lineNo, LogMsgType msgType, const QString& msg);

		void logError(int lineNo, const QString& errMsg);
		void logError(const QString& errMsg);

		void logWarning(int lineNo, const QString& wrnMsg);
		void logWarning(const QString& wrnMsg);

		void logRequirtedSettingIsNotSet(int lineNo, E::Setting st);

		int errorCount() const;
		int warningCount() const;

	private:
		QString message(int lineNo, const QString& msg);
		void log(int lineNo, LogMsgType msgType, const QString& msg);

	private:
		int m_errCount = 0;
		int m_wrnCount = 0;
	};

	class Parser
	{
	private:
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

	public:
		enum class ParseResult
		{
			Ok,
			Error,
			CriticalError
		};

		enum class LineType
		{
			Unknown,
			Section,
			Setting,
			Comment,
			SignalID,
		};

		struct ParseLineResult
		{
			int lineNo = 0;
			LineType lineType = LineType::Unknown;

			E::Section section = E::Section::Unknown;
			E::Setting setting = E::Setting::Unknown;

			QVariant value;

			//

			LogMsgType msgType = LogMsgType::Nothing;
			QString msg;

			//

			void setError(const QString& err);
			void setWarning(const QString& wrn);
			void setMessage(const QString& msg);
			void clear();
		};

	public:
		Parser();
		~Parser();

		bool parse(const QString& desc, const SignalSetAdapter& signalSetAdapter);

		const ParserLog& log() const;

		std::vector<const Gateway*> gateways() const;

	private:
		bool generateGatewaysRequiredFiles(SignalSetAdapter signalSetAdapter);

		ParseResult parseUnknownSection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseGatewaySection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseSignalListSection(E::Section& parsingSection, const ParseLineResult& plr);

		bool parseLine(const QString& str, ParseLineResult* plr);
		bool parseSettingValue(E::Setting setting, const QString& valueStr, ParseLineResult* plr);

		bool parseIntValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseAlphsNumericUnderlineStr(const QString& valueStr, ParseLineResult* plr);
		bool parseBoolValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseIpPortValueStr(const QString& valueStr, ParseLineResult* plr);

		Gateway* createApropriateGateway(E::GatewayType gwType);

		void clear();

	private:
		ParserLog m_log;
		std::vector<Gateway*> m_gateways;

		//

		bool m_multilineCommentStarted = false;

		QStringList m_knownSections;
		QStringList m_knownSettings;

		static const QRegularExpression m_anyWhitespaceSymbol;
		static const QRegularExpression m_notAlphaNumericUnderlineSymbols;
	};
}
