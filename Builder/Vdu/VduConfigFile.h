#pragma once
#include "VduTypes.h"

namespace Hardware
{
	class DeviceModule;
}

namespace Builder
{
	class Context;

	// VduConfig.bin file format
	// offset 0x0000: VduConfigFile1
	// offset 0x00D0: VduConfigSchema1 schema[schemaCount];
	// offset 0x00D0 + schemaCount * sizeof(VduConfigSchema1): Strings
	// last 8 bytes are crc64.

#pragma pack(push, 1)

	struct VduConfigDisplay1
	{
		uint16_t version; // 1
		uint16_t size;    // sizeof(VduConfigDisplay1)

		uint16_t width;   // In pixels
		uint16_t height;  // In pixels

		uint16_t mode;    // Color mode
		uint16_t reserve0;

		uint32_t reserve1;
		uint32_t reserve2;

		vdu_string_ref startSchemaId;
		vdu_string_ref displayName;

		uint32_t reserve3;
		uint32_t reserve4;
	};

	// The header of the VDU config file
	//
	struct VduConfigFile1
	{
		char magic[8];    // "VDUCFG1\0"

		uint16_t version; // 1
		uint16_t size;    // sizeof(VduConfigFile1)
		uint32_t reserve1;

		vdu_string_ref project;
		uint32_t buildNo;
		uint32_t reserve2;

		vdu_string_ref equipmentId;
		vdu_string_ref caption;
		uint32_t reserve3;
		uint32_t reserve4;

		uint16_t displayCount;
		uint16_t reserve5;
		VduConfigDisplay1 display0;
		VduConfigDisplay1 display1;
		VduConfigDisplay1 display2;
		VduConfigDisplay1 display3;

		uint16_t fontCount;
		uint16_t reserve6;

		uint32_t reserve7;
		uint32_t reserve8;

		uint16_t schemaCount;
		uint16_t reserve9;

		// Right after this struct follows the schema data
		// 1. VduConfigSchema1 schema[schemaCount];
		// 2. Strings
		// 3. crc64

		static const int MaxDisplayCount = 4;
	};

	struct VduConfigSchema1
	{
		uint16_t version; // 1
		uint16_t size;    // sizeof(VduConfigSchema1)
		uint32_t reserve0;

		vdu_string_ref schemaId;
		vdu_string_ref caption;

		uint32_t reserve1;
		uint32_t reserve2;

		uint64_t crc64; // Schema file crc64
	};
#pragma pack(pop)

	class VduConfigFileWriter
	{
	public:
		static bool generate(Builder::Context& context);

	private:
		static bool generate(const Hardware::DeviceModule& vdu, Builder::Context& context, QByteArray& out);

		static QString dump(QByteArray& data);
	};

} // namespace Builder