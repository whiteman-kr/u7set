#pragma once

#include "GatewayDescription.h"

namespace Gateway
{
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

	enum class ParseResult
	{
		Ok,
		Error,
		CriticalError
	};

	class Parser
	{
	private:
		static const QString SECTION_GATEWAY;
		static const QString SECTION_SIGNAL_LIST;

		static const QString START_COMMENT;

		static const QString START_SECTION;
		static const QString END_SECTION;

		static const QString EQUAL_SIGN;
		static const QString LEFT_POINTER_SIGN;
		static const QString APP_SIGNAL_ID_START_SIGN;

		static const QString ERR_SYNTAX;

		static const std::map<E::Setting, E::SettingType> m_settingType;

	public:
		enum class LineType
		{
			Unknown,
			Section,
			Setting,
			Comment,
			SignalID,
			AddressSignalID,
		};

		struct ParseLineResult
		{
			int lineNo = 0;
			LineType lineType = LineType::Unknown;

			E::Section section = E::Section::Unknown;
			E::Setting setting = E::Setting::Unknown;

			QVariant value;			// SectionName, Setting value or SignalID
			QString addressStr;

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
		Parser() = delete;
		Parser(const Parser&) = delete;

		Parser(const AppSignalSet* appSignalSet, GatewaysShared gateways = nullptr);
		Parser(const AppSignals& appSignals, GatewaysShared gateways = nullptr);
		~Parser();

		bool parse(const QString& desc);

		const ParserLog& log() const;

		GatewaysShared gateways();

	private:
		void commonInitialization();
		void clear();

		bool generateGatewaysRequiredFiles();

		ParseResult parseUnknownSection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseGatewaySection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseSignalListSection(E::Section& parsingSection, const ParseLineResult& plr);

		ParseResult appendAddressSignalID(SignalListShared signalList, const ParseLineResult& plr, bool appendAddr);

		ParseResult parsePropValue(const ParseLineResult& plr, QString* labelPropName);

		bool parseLine(const QString& str, ParseLineResult* plr);
		bool parseSettingValue(E::Setting setting, const QString& valueStr, ParseLineResult* plr);

		bool parseIntValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseAlphsNumericUnderlineStr(const QString& valueStr, ParseLineResult* plr);
		bool parseBoolValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseIpPortValueStr(const QString& valueStr, ParseLineResult* plr);

		GatewayShared createTypedGateway(E::GatewayType gwType);


	private:
		const SignalSetAdapter m_signalSetAdapter;
		GatewaysShared m_gateways;

		ParserLog m_log;

		//

		QStringList m_knownSections;
		QStringList m_knownSettings;

		static const QRegularExpression m_anyWhitespaceSymbol;
		static const QRegularExpression m_notAlphaNumericUnderlineSymbols;
	};
}
