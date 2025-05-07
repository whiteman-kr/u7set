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

	bool ModulesReportGenerator::writeReport()
	{
		LOG_MESSAGE(m_context->m_log, "Report generation...");

		m_equipmentIdLen = m_cnEquipmentID.length();
		m_captionLen = m_cnCaption.length();
		m_moduleIdLen = m_cnModuleID.length();
		m_familyLen = m_cnFamily.length();
		m_moduleVersionLen = QStringLiteral("0x0000 (000)").length();
		m_presetNameLen = m_cnPresetName.length();
		m_presetVersionLen = m_cnVersion.length();
		m_quantityLen = m_cnQuantity.length();

		for(const ModuleInfo& mi : m_modulesInfo)
		{
			if (mi.equipmentID.length() > m_equipmentIdLen)
			{
				m_equipmentIdLen = mi.equipmentID.length();
			}

			if (mi.moduleCaption.length() > m_captionLen)
			{
				m_captionLen = mi.moduleCaption.length();
			}

			if (mi.presetName.length() > m_presetNameLen)
			{
				m_presetNameLen = mi.presetName.length();
			}
		}

		QStringList file;

		bool result = true;

		file.append(QString(" Project: %1").arg(m_context->m_buildResultWriter->buildInfo().project));
		file.append(QString(" BildNo:  %1").arg(m_context->m_buildResultWriter->buildInfo().buildNo));
		file.append(QString(" Date:    %1").arg(m_context->m_buildResultWriter->buildInfo().dateTimeStr()));
		file.append(QString(" User:    %1").arg(m_context->m_buildResultWriter->buildInfo().user));
		file.append(QString(" Host:    %1").arg(m_context->m_buildResultWriter->buildInfo().workstation));
		file.append(Separator::EMPTY_STR);

		result &= writeModulesList(file);
		result &= writePresetsList(file);

		BuildFile* f = m_context->m_buildResultWriter->addFile(Directory::REPORTS, "InstalledModules.txt", "", "", file);

		result &= (f != nullptr);

		return result;
	}

	bool ModulesReportGenerator::writeModulesList(QStringList& file)
	{
		QString line;

		line.fill('-', m_equipmentIdLen + m_captionLen + m_moduleIdLen + m_familyLen + m_moduleVersionLen +
						   m_presetNameLen + m_presetVersionLen + 20);

		QString row(" %1 | %2 | %3 | %4 | %5 | %6 | %7");

		file.append(" Installed Modules");
		file.append(Separator::EMPTY_STR);
		file.append(line);
		file.append(row.
					arg(m_cnEquipmentID, -m_equipmentIdLen).arg(m_cnCaption, -m_captionLen).
					arg(m_cnModuleID, -m_moduleIdLen).arg(m_cnFamily, -m_familyLen).
					arg(m_cnVersion, -m_moduleVersionLen).arg(m_cnPresetName, -m_presetNameLen).
					arg(m_cnVersion, m_presetVersionLen));
		file.append(line);

		for(const ModuleInfo& mi : m_modulesInfo)
		{
			QString moduleIdStr = "0x" + QString("%1").arg(mi.moduleID, 4, 16, QChar('0')).toUpper();
			QString familyStr = "0x" + QString("%1").arg(mi.moduleID & 0xFF00, 4, 16, QChar('0')).toUpper();
			QString versionStr = "0x" + QString("%1 (%2)").arg((mi.moduleID & 0x00FF), 4, 16, QChar('0')).
										arg(mi.moduleID & 0x00FF).toUpper();
			file.append(row.
						arg(mi.equipmentID, -m_equipmentIdLen).arg(mi.moduleCaption, -m_captionLen).
						arg(moduleIdStr, -m_moduleIdLen).
						arg(familyStr, -m_familyLen).
						arg(versionStr, -m_moduleVersionLen).
						arg(mi.presetName, -m_presetNameLen).
						arg(mi.presetVersion, -m_presetVersionLen ));
		}

		file.append(line);
		file.append(Separator::EMPTY_STR);

		return true;
	}

	bool ModulesReportGenerator::writePresetsList(QStringList& file)
	{
		std::map<PresetInfo, std::set<QString>> presetsInfo;	// PresetInfo -> set of modules EquipmentID

		for(const ModuleInfo& mi : m_modulesInfo)
		{
			PresetInfo pi { mi.presetName, mi.presetVersion };

			auto it = findOrInsertKey(presetsInfo, pi);

			it->second.emplace(mi.equipmentID);
		}

		file.append(" Used Presets");
		file.append(Separator::EMPTY_STR);

		QString line;

		line.fill('-',m_presetNameLen + m_presetVersionLen + m_equipmentIdLen + m_quantityLen + 11);

		QString row(" %1 | %2 | %3 | %4");

		file.append(line);
		file.append(row.arg(m_cnPresetName, -m_presetNameLen).arg(m_cnVersion, -m_presetVersionLen).
					arg(m_cnEquipmentID, -m_equipmentIdLen).arg(m_cnQuantity, -m_quantityLen));
		file.append(line);

		for(const auto& [pi, equipmentIDs] : presetsInfo)
		{
			QString presetName = pi.name;
			QString presetVersion = QString::number(pi.version);
			QString quantity = QString::number(equipmentIDs.size());

			for(const QString& equipmentID : equipmentIDs)
			{
				file.append(row.arg(presetName, -m_presetNameLen).
							arg(presetVersion, -m_presetVersionLen).
							arg(equipmentID, -m_equipmentIdLen).
							arg(quantity, -m_quantityLen));

				presetName.clear();
				presetVersion.clear();
				quantity.clear();
			}

			file.append(line);
		}

		file.append(Separator::EMPTY_STR);

		return true;
	}
}
