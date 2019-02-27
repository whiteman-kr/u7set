#include "Context.h"

namespace Builder
{
	Context::Context(IssueLogger* log, QString buildOutputPath, bool debug, bool expertMode) :
		m_buildOutputPath(buildOutputPath),
		m_debug(debug),
		m_expertMode(expertMode),
		m_log(log)
	{
		assert(log);
	}
}
