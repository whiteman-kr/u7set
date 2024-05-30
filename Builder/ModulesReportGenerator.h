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
		bool writeReport();
		bool writeModulesList(QStringList& file);
		bool writePresetsList(QStringList& file);

	private:
		struct ModuleInfo
		{
			QString equipmentID;
			QString moduleCaption;
			int moduleID = 0;		// value of uint16_t DeviceModule->m_type, high byte - Module Family, low byte - Module Version (implementation)
			QString presetName;
			int presetVersion = 0;
		};

		struct PresetInfo
		{
			QString name;
			int version = -1;

			bool operator < (const PresetInfo& b) const
			{
				if (name == b.name)
				{
					return version < b.version;
				}

				return name < b.name;
			}
		};

	private:
		const Context* m_context;

		std::list<ModuleInfo> m_modulesInfo;

		inline static const QString m_cnEquipmentID = "Module EquipmentID";
		inline static const QString m_cnCaption = "Caption";
		inline static const QString m_cnModuleID = "ModuleID";
		inline static const QString m_cnFamily = "Family";
		inline static const QString m_cnVersion = "Version";
		inline static const QString m_cnPresetName = "Preset Name";
		inline static const QString m_cnQuantity = "Quantity";

		int m_equipmentIdLen = 0;
		int m_captionLen = 0;
		int m_moduleIdLen = 0;
		int m_familyLen = 0;
		int m_moduleVersionLen = 0;
		int m_presetNameLen = 0;
		int m_presetVersionLen = 0;
		int m_quantityLen = 0;
	};
}
