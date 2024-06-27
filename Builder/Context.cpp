#include "Context.h"

namespace Builder
{
	Context::Context(IssueLogger* log, QString buildOutputPath, bool expertMode, BuildOptions buildOptions) :
		m_log(log),
		m_buildOutputPath(buildOutputPath),
		m_expertMode(expertMode),
		m_buildOptions(buildOptions)
	{
		assert(log);
	}

	Context::~Context() = default;

	bool Context::generateAppSignalsXml() const
	{
		bool result = BuildOptions::makeDecision(m_projectProperties.generateAppSignalsXml(), m_buildOptions.generateAppSignalsXml);
		return result;
	}

	bool Context::generateAppSignalsExtXml() const
	{
		bool result = BuildOptions::makeDecision(m_projectProperties.generateAppSignalsExtXml(), m_buildOptions.generateAppSignalsExtXml);
		return result;
	}

	bool Context::generateExtraDebugInfo() const
	{
		bool result = BuildOptions::makeDecision(m_projectProperties.generateExtraDebugInfo(), m_buildOptions.generateExtraDebugInfo);
		return result;
	}
} // namespace Builder