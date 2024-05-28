#include "ModulesReportGenerator.h"
#include "WUtils.h"
#include <HardwareLib/DeviceModule.h>

namespace Builder
{
	ModulesReportGenerator::ModulesReportGenerator(const Context* context) :
		m_context(context)
	{
	}

	bool ModulesReportGenerator::run()
	{
		TEST_PTR_RETURN_FALSE(m_context);

		IssueLogger* log = m_context->m_log;
		TEST_PTR_RETURN_FALSE(log);

		TEST_PTR_LOG_RETURN_FALSE(m_context->m_equipmentSet, log);
		TEST_PTR_LOG_RETURN_FALSE(m_context->m_buildResultWriter, log);

		bool result = true;

		result &= fillModulesInfo();

		RETURN_IF_FALSE(result);

		result &= writeReport();

		return result;
	}

	bool ModulesReportGenerator::fillModulesInfo()
	{
		LOG_MESSAGE(m_context->m_log, "Gathering modules information...");

		bool result = true;

		std::function<void (Hardware::DeviceObject*)> scanDevice = [this, &result](Hardware::DeviceObject* device) -> void
		{
			IssueLogger* log = m_context->m_log;

			if (device == nullptr)
			{
				LOG_INTERNAL_ERROR(log);
				result = false;
				return;
			}

			if (device->isModule() == false)
			{
				return;
			}

			const Hardware::DeviceModule* module = device->toModule().get();

			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(log);
				result = false;
				return;
			}

			int moduleID = module->moduleType();

			if ((moduleID & 0xFF00) == TO_INT(Hardware::DeviceModule::FamilyType::OTHER))
			{
				moduleID |= module->customModuleFamily();
			}

			m_modulesInfo.emplace_back(module->equipmentIdTemplate(),
									   module->caption(),
									   moduleID,
									   module->presetName(),
									   module->presetVersion());
		};

		Hardware::equipmentWalker(m_context->m_equipmentSet->root().get(), scanDevice);

		RETURN_IF_FALSE(result);

		m_modulesInfo.sort([](const ModuleInfo& a, const ModuleInfo& b)
							{
								return a.equipmentID < b.equipmentID;
							} );

		return result;
	}

	bool ModulesReportGenerator::writeReport() const
	{
		LOG_MESSAGE(m_context->m_log, "Report generation...");

		static const QString cnEquipmentID("Module EquipmentID");
		static const QString cnCaption("Caption");
		static const QString cnModuleID("ModuleID");
		static const QString cnFamily("Family");
		static const QString cnVersion("Version");
		static const QString cnPreset("Preset");
		static const QString row(" %1 | %2 | %3 | %4 | %5 | %6");

		int equipmentIdLen = cnEquipmentID.length();
		int captionLen = cnCaption.length();
		int moduleIdLen = cnModuleID.length();
		int familyLen = cnFamily.length();
		int versionLen = QStringLiteral("0x0000 (000)").length();
		int presetLen = cnPreset.length();

		for(const ModuleInfo& mi : m_modulesInfo)
		{
			if (mi.equipmentID.length() > equipmentIdLen)
			{
				equipmentIdLen = mi.equipmentID.length();
			}

			if (mi.moduleCaption.length() > captionLen)
			{
				captionLen = mi.moduleCaption.length();
			}

			if (mi.presetName.length() > presetLen)
			{
				presetLen = mi.presetName.length();
			}
		}

		presetLen += cnPreset.length() + QStringLiteral("v000").length();

		QString line;

		line.fill('-', equipmentIdLen + captionLen + moduleIdLen + familyLen + versionLen + presetLen + 10);

		QStringList file;

		file.append(" Installed Modules Report");
		file.append(Separator::EMPTY_STR);
		file.append(QString(" Project: %1").arg(m_context->m_buildResultWriter->buildInfo().project));
		file.append(QString(" BildNo:  %1").arg(m_context->m_buildResultWriter->buildInfo().id));
		file.append(QString(" Date:    %1").arg(m_context->m_buildResultWriter->buildInfo().dateStr()));
		file.append(QString(" User:    %1").arg(m_context->m_buildResultWriter->buildInfo().user));
		file.append(QString(" Host:    %1").arg(m_context->m_buildResultWriter->buildInfo().workstation));
		file.append(Separator::EMPTY_STR);
		file.append(line);
		file.append(row.
					arg(cnEquipmentID, -equipmentIdLen).arg(cnCaption, -captionLen).
					arg(cnModuleID, -moduleIdLen).arg(cnFamily, -familyLen).
					arg(cnVersion, -versionLen).arg(cnPreset, -presetLen));
		file.append(line);

		for(const ModuleInfo& mi : m_modulesInfo)
		{
			QString moduleIdStr = "0x" + QString("%1").arg(mi.moduleID, 4, 16, QChar('0')).toUpper();
			QString familyStr = "0x" + QString("%1").arg(mi.moduleID & 0xFF00, 4, 16, QChar('0')).toUpper();
			QString versionStr = "0x" + QString("%1 (%2)").arg((mi.moduleID & 0x00FF), 4, 16, QChar('0')).
										arg(mi.moduleID & 0x00FF).toUpper();

			QString presetStr = QString("%1 v%2").arg(mi.presetName).arg(mi.presetVersion);

			file.append(row.
						arg(mi.equipmentID, -equipmentIdLen).arg(mi.moduleCaption, -captionLen).
						arg(moduleIdStr, -moduleIdLen).
						arg(familyStr, -familyLen).
						arg(versionStr, -versionLen).
						arg(presetStr, -presetLen));
		}

		BuildFile* f = m_context->m_buildResultWriter->addFile(Directory::REPORTS, "InstalledModules.txt", "", "", file);

		return (f != nullptr);
	}
}
