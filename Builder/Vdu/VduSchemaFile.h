#include "VduTypes.h"

// 29 Mar 2024 - Version 1.0 -  The first version of the file format.
// 10 Apr 2024 - Version 1.1 -  Added VduSchemaFileSchemaItem1::isStatic, VduSchemaFileSchemaItemValue1::decimalPlaces.
// 13 Aug 2024 - Version 1.2 -  Added VduSchemaFileSchemaItem1::preDrawScript, VduSchemaFileSchemaItem1::clickScript,
// VduSchemaFileSchemaItem1::objectName, VduSchemaFileSchemaItem1::clickScript, VduSchemaFileSchemaItem1::preDrawScript.


// SVDU schema file, extension *.vbs
// Data stored in little-endian format.
// The file is a binary file with the following high-level structure:
// 1. Header
// 2. SchemaItems
// 3. Strings
// 4. crc64
//

// Pack structs to 1 byte alignment
//
#pragma pack(push, 1)

struct VduSchemaFileProperties1
{
	uint16_t version;    // 1
	uint16_t headerSize; // This header size
	uint16_t width;      // In pixels
	uint16_t height;     // In pixels
	uint32_t reserve0;
	uint32_t backgroundColor;
	vdu_string_ref schemaId;
	vdu_string_ref caption;
	vdu_string_ref onShowScript;
	vdu_string_ref preDrawScript;
	uint32_t reserve1;
	uint32_t reserve2;
};

//
// Start point, this structure has to be at the beginning of the file.
//
struct VduSchemaFile
{
	// 1. Header and schema properties
	//
	char magic[4];            // "VDU\0"
	uint16_t fileVersion;     // 1
	uint16_t reserve1;

	struct VduSchemaFileProperties1 schemaProperties;
	uint32_t reserve2[4];

	uint16_t schemaItemCount; // Number of schema items - each items is a VduSchemaFileSchemaItem1 + specific data
							  // (VduSchemaFileSchemaItemLine1 | VduSchemaFileSchemaItemRect1 | ...).
	uint16_t reserve3;

	// Next fields are present in a text description:
	//

	// 2. SchemaItems[schemaItemCount]
	// SchemaItems: a list of schema items.
	// Schema item is a struct that starts with VduSchemaFileSchemaItem1 and is followed
	// by the data of the specific schema item like VduSchemaFileSchemaItemLine1, VduSchemaFileSchemaItemRect1, ...
	//

	// 3. Strings
	// Strings: a list of strings.
	// see vdu_string_ref

	// 4. crc64
	// uint64_t crc64; // CRC64 of the file from the beginning to the end of the strings.
};

// VduSchemaFileSchemaItem1::itemType
//
const uint16_t VduFileSchemaItemLineId = 0x4E4C;  // LN
const uint16_t VduFileSchemaItemRectId = 0x4352;  // RC
const uint16_t VduFileSchemaItemValueId = 0x4C56; // VL

struct VduSchemaFileSchemaItem1
{
	uint16_t version;                             // 1
	uint16_t size;                                // sizeof(VduSchemaFileSchemaItem1)
	uint16_t itemType;                            // VduFileSchemaItemLineId, VduFileSchemaItemRectId, ...
	uint16_t reserve0;
	uint16_t totalItemSize; // sizeof(VduSchemaFileSchemaItem1) + sizeof(VduSchemaFileSchemaItemLine1 | VduSchemaFileSchemaItemRect1 | ...)

	bool isStatic;          // If true, the item is static and can be cached.
	bool reserveBool0;

	vdu_string_ref objectName;
	vdu_string_ref clickScript;
	vdu_string_ref preDrawScript;

	uint32_t reserve2;
	uint32_t reserve3;
	// This struct is followed by the data of the specific schema item like VduSchemaFileSchemaItemLine1, VduSchemaFileSchemaItemRect1,
	// VduSchemaFileSchemaItemValue1, ...
};

struct VduSchemaFileSchemaItemLine1
{
	uint16_t version;  // 1
	uint16_t itemType; // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	uint32_t reserve0;

	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;

	uint16_t weight;
	uint16_t reserve1;

	uint32_t color;
	uint32_t reserve2;
};

struct VduSchemaFileSchemaItemRect1
{
	uint16_t version;  // 1
	uint16_t itemType; // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	uint32_t reserve0;

	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;

	uint16_t weight;
	bool fill;
	bool drawRect;
	uint32_t reserve1;

	uint32_t lineColor;
	uint32_t fillColor;
	uint32_t textColor;
	uint32_t reserve2;

	uint16_t fontIndex; // Fonts are generated on build, each font is a folder with name as index, this folder contains font files.
	uint16_t reserve3;
	vdu_string_ref text;
	uint32_t reserve4;

	int32_t align;      // HorzAlign | VertAlign
	uint32_t reserve5;
};

struct VduSchemaFileSchemaItemValue1
{
	uint16_t version;   // 1
	uint16_t itemType;  // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	uint32_t reserve0;

	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;

	uint16_t weight;
	bool reserve1;
	bool drawRect;
	uint32_t reserve2;

	uint32_t lineColor;
	uint32_t fillColor;
	uint32_t textColor;

	uint32_t reserve3[8];   // Reserved for different colors

	uint16_t fontIndex;     // Fonts are generated on build, each font is a folder with name as index, this folder contains font files.
	uint16_t reserve4;

	uint16_t decimalPlaces; // Number of decimal places for floating point values.
	uint16_t reserve5;

	uint32_t reserve6;
	uint32_t reserve7;

	uint32_t appSignalIndex;
};

#pragma pack(pop)
