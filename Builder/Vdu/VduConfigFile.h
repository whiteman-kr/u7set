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

	/// \brief Represents the display configuration in the VDU config file.
	struct VduConfigDisplay1
	{
		uint16_t version;       ///< Structure version, 1
		uint16_t size;          ///< Structure size, sizeof(VduConfigDisplay1)

		uint16_t width;         ///< Display width, in pixels
		uint16_t height;        ///< Display height, in pixels

		uint16_t mode;          ///< Color mode, always 0
		uint16_t reserve0;

		uint32_t reserve1;
		uint32_t reserve2;

		vdu_cstr startSchemaId; ///< The ID of the starting schema for the display.
		vdu_cstr displayName;   ///< The display name.

		uint32_t reserve3;
		uint32_t reserve4;
	};

	/// \brief Represents the header of the VDU config file.
	struct VduConfigFile1
	{
		char magic[8];              ///< The marker of structure "VDUCFG1\0"

		uint16_t version;           ///< Structure version, 1
		uint16_t size;              ///< Structure size, sizeof(VduConfigFile1)
		uint32_t reserve1;

		vdu_cstr project;           ///< The project name.
		uint32_t buildNo;           ///< The build number.
		uint32_t reserve2;

		vdu_cstr equipmentId;       ///< VDU equipment ID.
		vdu_cstr caption;           ///< VDU caption.
		uint32_t reserve3;
		uint32_t reserve4;

		uint16_t displayCount;      ///< The number of displays.
		uint16_t reserve5;
		VduConfigDisplay1 display0; ///< The first display configuration.
		VduConfigDisplay1 display1; ///< The second display configuration.
		VduConfigDisplay1 display2; ///< The third display configuration.
		VduConfigDisplay1 display3; ///< The fourth display configuration.

		uint16_t fontCount;         ///< The number of fonts.
		uint16_t reserve6;

		uint32_t reserve7;
		uint32_t reserve8;

		uint16_t schemaCount;       ///< The number of schemas.
		uint16_t reserve9;

		// Right after this struct follows the schema data
		// 1. VduConfigSchema1 schema[schemaCount];
		// 2. Strings
		// 3. crc64

		static const int MaxDisplayCount = 4;
	};

	/// \brief Represents the schema configuration in the VDU config file.
	struct VduConfigSchema1
	{
		uint16_t version;  ///< Structure version, 1
		uint16_t size;     ///< Structure size, sizeof(VduConfigSchema1)
		uint32_t reserve0;

		vdu_cstr schemaId; ///< The ID of the schema.
		vdu_cstr caption;  ///< The caption.

		uint32_t reserve1;
		uint32_t reserve2;

		uint64_t crc64;    ///< The CRC64 value of the schema file.
	};
#pragma pack(pop)

	class VduConfigFileWriter
	{
	public:
		/// \brief Generates the VDU config file.
		/// \param context The builder context.
		/// \return True if the generation is successful, false otherwise.
		static bool generate(Builder::Context& context);
	};

} // namespace Builder