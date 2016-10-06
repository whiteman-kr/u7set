#pragma once

#include "../lib/DeviceObject.h"
#include "IssueLogger.h"

namespace Hardware
{

	class ModuleRawDataDescription
	{
	public:
		enum class ItemType
		{
			RawDataSize,

			AppData16,
			DiagData16,

			AppData32,
			DiagData32,
		};

		struct Item
		{
			ItemType type;

			bool rawDataSizeIsAuto = false;			// for type - RawDataSize
			int rawDataSize = 0;					//

			int offset;								// for AppData* and DiagData*
		};

		const int RAW_DATA_SIZE_INDEX = 0;

	public:
		ModuleRawDataDescription();

		bool init(DeviceModule* module, Builder::IssueLogger *log);

		const QVector<Item>& items() const { return m_items; }

	private:
		bool parse(const QString& rawDataDescriptionStr, Builder::IssueLogger* log);
		bool calculateRawDataSize(Builder::IssueLogger* log);

	private:
		static const char* RAW_DATA_SIZE;
		static const char* APP_DATA16;
		static const char* APP_DATA32;
		static const char* DIAG_DATA16;
		static const char* DIAG_DATA32;

		QString m_moduleEquipmentID;

		int m_rawDataSize = 0;

		QVector<Item> m_items;
	};

}
