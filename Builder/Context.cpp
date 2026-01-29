#include "Context.h"
#include "ModuleLogicCompiler.h"
#include "SoftwareCfgGenerator.h"

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

	void Context::appendModuleLogicCompiler(std::shared_ptr<ModuleLogicCompiler> mc)
	{
		Hash h = calcHash(mc->lmEquipmentID());

		Q_ASSERT(m_moduleLogicCompilers.find(h) == m_moduleLogicCompilers.end());

		m_moduleLogicCompilers.emplace(h, mc);
	}

	std::shared_ptr<ModuleLogicCompiler> Context::getModuleLogicCompiler(const QString& lmEquipmentID) const
	{
		auto it = m_moduleLogicCompilers.find(calcHash(lmEquipmentID));

		if (it != m_moduleLogicCompilers.end())
		{
			return it->second;
		}

		Q_ASSERT(false);

		return nullptr;
	}

} // namespace Builder
