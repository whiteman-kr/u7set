#pragma once

#include "Context.h"

#include "../GatewayLib/GatewayDescription.h"
#include "../GatewayLib/GatewayParserLog.h"

namespace Gateway
{
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

		Parser(const Builder::Context* context, GatewaysShared gateways = nullptr);
		virtual ~Parser();

		bool parse(const QString& desc);

		GatewaysShared gateways();

		int errorCount() const;
		int warningCount() const;

		void flushParserLog();

	private:
		void clear();

		bool generateGatewaysRequiredFiles();

		ParseResult parseUnknownSection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseGatewaySection(E::Section& parsingSection, const ParseLineResult& plr);
		ParseResult parseSignalListSection(E::Section& parsingSection, const ParseLineResult& plr);

		ParseResult finalizeGatewaySection(const ParseLineResult& plr);

		ParseResult appendAddressSignalID(SignalListShared signalList, const ParseLineResult& plr, bool appendAddr);

		ParseResult parsePropValue(int lineNo, const QString& plrValue, bool* isPropValue, double* propValue);
		ParseResult findPropertyValue(int lineNo, const QString& itemLabel, const QString& propName, double* propValue);

		bool parseLine(const QString& str, ParseLineResult* plr);
		bool parseSettingValue(E::Setting setting, const QString& valueStr, ParseLineResult* plr);

		bool parseIntValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseAlphaNumericUnderlineStr(const QString& valueStr, ParseLineResult* plr);
		bool parseBoolValueStr(const QString& valueStr, ParseLineResult* plr);
		bool parseIpPortValueStr(const QString& valueStr, ParseLineResult* plr);

		QStringList knownGatewayTypes() const;
		E::GatewayType getGatewayType(const QString& gwTypeStr) const;

	private:
		const Builder::Context* m_context = nullptr;
		const AppSignalSet* m_appSignalSet = nullptr;
		Builder::IssueLogger* m_u7log = nullptr;

		GatewaysShared m_gateways;
		mutable ParserLog m_log;

		//

		std::set<Hash> m_mlFoundIn;
		std::set<Hash> m_mlNotFoundIn;

		//

		QStringList m_knownSections;
		QStringList m_knownSettings;

		static const QRegularExpression m_anyWhitespaceSymbol;
		static const QRegularExpression m_notAlphaNumericUnderlineSymbols;
	};
}
