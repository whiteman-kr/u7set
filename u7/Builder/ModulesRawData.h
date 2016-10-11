#pragma once

#include "../lib/DeviceObject.h"
#include "IssueLogger.h"

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

	bool init(const Hardware::DeviceModule *module, Builder::IssueLogger *log);

	const QVector<Item>& items() const { return m_items; }

	const int rawDataSize() const { return m_rawDataSize; }

private:
	bool parse(const QString& moduleEquipmentID, const QString& rawDataDescriptionStr, Builder::IssueLogger* log);
	bool calculateRawDataSize(const QString& moduleEquipmentID, Builder::IssueLogger* log);

private:
	static const char* RAW_DATA_SIZE;
	static const char* APP_DATA16;
	static const char* APP_DATA32;
	static const char* DIAG_DATA16;
	static const char* DIAG_DATA32;

	int m_rawDataSize = 0;

	QVector<Item> m_items;
};
