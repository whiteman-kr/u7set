#pragma once

#include "Context.h"

namespace Builder
{
	class ModulesReportGenerator
	{
	public:
		ModulesReportGenerator(const Context* context);
		bool run();

	private:
		bool fillModulesInfo();
		bool writeReport() const;

	private:
		struct ModuleInfo
		{
			QString equipmentID;
			QString moduleCaption;
			int moduleID = 0;		// value of uint16_t DeviceModule->m_type, high byte - Module Family, low byte - Module Version (implementation)
			QString presetName;
			int presetVersion = 0;
		};

	private:
		const Context* m_context;

		std::list<ModuleInfo> m_modulesInfo;
	};
}
