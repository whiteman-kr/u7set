#include "ModulesRawData.h"

namespace Hardware
{
	const char* ModuleRawDataDescription::RAW_DATA_SIZE = "RAW_DATA_SIZE";
	const char* ModuleRawDataDescription::APP_DATA16 = "APP_DATA16";
	const char* ModuleRawDataDescription::APP_DATA32 = "APP_DATA32";
	const char* ModuleRawDataDescription::DIAG_DATA16 = "DIAG_DATA16";
	const char* ModuleRawDataDescription::DIAG_DATA32 = "DIAG_DATA32";


	ModuleRawDataDescription::ModuleRawDataDescription()
	{
	}

	bool ModuleRawDataDescription::init(DeviceModule* module, Builder::IssueLogger* log)
	{
		if (module == nullptr || log == nullptr)
		{
			assert(false);
			return false;
		}

		m_moduleEquipmentID = module->equipmentIdTemplate();

		bool result = true;

		result = parse(module->rawDataDescription(), log);

		if (result == false)
		{
			return false;
		}

		return calculateRawDataSize(log);
	}


	bool ModuleRawDataDescription::parse(const QString& rawDataDescriptionStr, Builder::IssueLogger* log)
	{
		m_items.clear();

		QString srcStr = rawDataDescriptionStr.toUpper().trimmed();

		if (srcStr.isEmpty())
		{
			m_rawDataSize = 0;
			return true;
		}

		bool result = true;

		// split string

		QStringList list = srcStr.split("\n", QString::SkipEmptyParts);

		bool rawDataSizeFound = false;

		QString msg;

		for(QString str : list)
		{
			Item item;

			str = str.trimmed();

			QString itemTypeStr = str.section("=", 0, 0).trimmed();

			QString valueStr = str.section("=", 1, 1).trimmed();

			bool convertResult = false;

			int value = valueStr.toInt(&convertResult);

			if (itemTypeStr == RAW_DATA_SIZE)
			{
				if (rawDataSizeFound == true)
				{
					msg = QString("Duplicate RAW_DATA_SIZE section in module '%1' settings.").arg(m_moduleEquipmentID);
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}

				rawDataSizeFound = true;

				if (valueStr != "AUTO" )
				{
					if (convertResult == false)
					{
						msg = QString("Invalid RAW_DATA_SIZE value in module '%1' settings.").arg(m_moduleEquipmentID);
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						continue;
					}

					item.rawDataSize = value;
					item.rawDataSizeIsAuto = false;
				}
				else
				{
					item.rawDataSizeIsAuto = true;
				}

				item.type = ItemType::RawDataSize;
				m_items.insert(RAW_DATA_SIZE_INDEX, item);		// RAW_DATA_SIZE always first item
				continue;
			}

			if (itemTypeStr == APP_DATA16)
			{
				item.type = ItemType::AppData16;
			}
			else
			{
				if (itemTypeStr == APP_DATA32)
				{
					item.type = ItemType::AppData32;
				}
				else
				{
					if (itemTypeStr == DIAG_DATA16)
					{
						item.type = ItemType::DiagData16;
					}
					else
					{
						if (itemTypeStr == DIAG_DATA32)
						{
							item.type = ItemType::DiagData32;
						}
						else
						{
							msg = QString("Unknown item %1 in module '%2' settings.").arg(itemTypeStr).arg(m_moduleEquipmentID);
							LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
							result = false;

							break;
						}
					}
				}

			}

			if (convertResult == false)
			{
				msg = QString("Invalid %1 value in module '%2' settings.").arg(itemTypeStr).arg(m_moduleEquipmentID);
				LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
				result = false;
				continue;
			}

			item.offset = value;
			m_items.append(item);
		}

		if (rawDataSizeFound == false)
		{
			msg = QString("RAW_DATA_SIZE value is not found in module '%1' settings.").arg(m_moduleEquipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			result = false;
		}

		return result;
	}


	bool ModuleRawDataDescription::calculateRawDataSize(Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = true;

		return result;
	}

}
