#include <stdint.h>
#include <stdbool.h>

// string_ref is an offset in a file to a string.
// The string is a null terminated QChar string 
// (In Qt, Unicode characters are 16-bit entities without any markup or structure).
typedef uint32_t vdu_string_ref;

// schema_item_ref is an offset in a file to a schema item.
typedef uint32_t vdu_schema_item_ref;

// Pack structs to 1 byte alignment
//
#pragma pack(push, 1)

struct VduSchemaFileProperties1
{
	uint16_t version; // 1
	uint16_t width;
	uint16_t height;
	uint16_t reserve0;
	uint32_t backgroundColor;
	vdu_string_ref schemaId;
	vdu_string_ref caption;
	uint32_t reserve1;
	uint32_t reserve2;
};

//
// Start point, this structure has to be at the beginning of the file.
//
struct VduSchemaFile
{
	char magic[4];        // "VDU\0"
	uint16_t fileVersion; // 1
	uint16_t reserve1;

	struct VduSchemaFileProperties1 schemaProperties;
	uint32_t reserve2[4];
	
	uint16_t count;
	uint16_t reserve3;

	// items is an array of schema_item_ref.
	// The size of the array is count.
	// schema_item_ref is an offset in a file to a schema item.
	// Schema item is a struct that starts with VduSchemaFileSchemaItem1 and is followed
	// by the data of the specific schema item like VduSchemaFileSchemaItemLine1, VduSchemaFileSchemaItemRect1, ...
	//
	// schema_item_ref items[count];
};

// VduSchemaFileSchemaItem1::itemType
const uint16_t VduFileSchemaItemLineId = 1;
const uint16_t VduFileSchemaItemRectId = 2;
const uint16_t VduFileSchemaItemValueId = 3;

struct VduSchemaFileSchemaItem1
{
	uint16_t version;  // 1
	uint16_t itemType; // VduFileSchemaItemLineId, VduFileSchemaItemRectId, ...
	uint32_t size;
	uint32_t reserve0;
	uint32_t reserve1;
	// This struct is followed by the data of the specific schema item like VduSchemaFileSchemaItemLine1, VduSchemaFileSchemaItemRect1, ...
	//
};

struct VduSchemaFileSchemaItemLine1
{
	uint16_t version;  // 1
	uint16_t itemType; // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	uint32_t size;
	uint32_t reserve0;
	
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
	uint32_t reserve1;

	uint32_t color;
	uint32_t reserve2;
};

struct VduSchemaFileSchemaItemRect1
{
	uint16_t version;  // 1
	uint16_t itemType; // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	uint32_t size;
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

	vdu_string_ref fontName;
	vdu_string_ref text;
	uint32_t reserve3;

	int16_t horzAlign;
	int16_t vertAlign;
	uint32_t reserve4;
};

struct VduSchemaFileSchemaItemValue1
{
	uint16_t version;  // 1
	uint16_t itemType; // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	uint32_t size;
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
	uint32_t reserve3;

	vdu_string_ref fontName;
	vdu_string_ref reserve4;
	uint32_t reserve5;
	uint32_t reserve6;

	uint64_t appSignalHash;
};


#pragma pack(pop)