#include "Context.h"
#include "../VFrame30/LogicSchema.h"
#include "../VFrame30/VduSchema.h"

namespace Builder
{
	Context::Context(IssueLogger* log, QString buildOutputPath, bool expertMode) :
		m_log(log),
		m_buildOutputPath(buildOutputPath),
		m_expertMode(expertMode)
	{
		assert(log);
	}

	Context::~Context() = default;

	bool Context::generateAppSignalsXml() const
	{
		return m_projectProperties.generateAppSignalsXml();
	}

	bool Context::generateAppSignalsExtXml() const
	{
		return m_projectProperties.generateAppSignalsExtXml();
	}

	bool Context::generateExtraDebugInfo() const
	{
		return m_projectProperties.generateExtraDebugInfo();
	}
}
