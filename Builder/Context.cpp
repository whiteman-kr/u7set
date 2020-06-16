#include "Context.h"

namespace Builder
{
	Context::Context(IssueLogger* log, QString buildOutputPath, bool expertMode) :
		m_log(log),
		m_buildOutputPath(buildOutputPath),
		m_expertMode(expertMode)
	{
		assert(log);
	}

	bool Context::generateAppSignalsXml() const
	{
		return m_projectProperties.generateAppSignalsXml();
	}

	bool Context::generateExtraDebugInfo() const
	{
		return m_projectProperties.generateExtraDebugInfo();
	}
}
