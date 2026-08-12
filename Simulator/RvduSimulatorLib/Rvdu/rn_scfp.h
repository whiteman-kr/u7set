/*
Module title: Schema Files Processing.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Loads and manages schema files.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_scfp initial update

-- *************************************************************
-- Version v1.01
-- Date:	30,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-3966, rn_scfp module refactoring
-- Add bitmaps caching
-- Add file version control
-- Add GlobalScript support
-- By default, schema's background is not cleared before redraw, this saves time. But if Lua functons that move items are called, this schema is redrawed fully.
-- Now, predrawScripts and clickScripts are compiled to bytecode on startup.

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************
-- Version v1.02
-- Date:	19,06,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, schema switching improvement: it is scheduled and actually changed at the next schemaProc loop to prevent scripts data corruption

-- *************************************************************
-- Version v1.03
-- Date:	25,06,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4095
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-4095, rn_scfp improve memory allocation control
-- Add memory allocation control
-- *************************************************************

-- Version v1.04
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.05
-- Date:	11,08,2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

RADIX-4112, OnShowScript and PreDrawScript on schemas are processed

-- *************************************************************
*/


#ifndef RN_SCFP_H
#define RN_SCFP_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM

#include <alt_types.h>
#include <stdio.h>

#else

#include "msvc_alt_types.h"

#endif
// Microsoft Visual Studio-specific

#include "types.h"
#include "rn_diag.h"

#include <stdbool.h>

#define MAX_SCHEMAS_COUNT 512u

#define TYPE_LINE 0x4e4cu				// Line
#define TYPE_RECT 0x4352u				// Rect
#define TYPE_VALUE 0x4c56u				// Value
#define TYPE_IMAGE 0x4D47u				// Image
#define TYPE_IMAGEVALUE 0x5649u			// ImageValue
#define TYPE_TREND 0x5254u				// Trend

#define SCHEMA_PROP_VERSION 1u			// VduSchemaFileProperties1
#define SCHEMA_ITEM_VERSION 2u			// VduSchemaFileSchemaItem1

#define TYPE_LINE_VERSION 1u			// VduSchemaFileSchemaItemLine1
#define TYPE_RECT_VERSION 2u			// VduSchemaFileSchemaItemRect1
#define TYPE_VALUE_VERSION 1u			// VduSchemaFileSchemaItemValue1
#define TYPE_IMAGE_VERSION 1u			// VduSchemaFileSchemaItemImage1
#define TYPE_IMAGEVALUE_VERSION 1u		// VduSchemaFileSchemaItemImageValue1
#define TYPE_TREND_VERSION 2u			// VduSchemaFileSchemaItemTrend1

#pragma pack(push, 1)

struct VduSchemaFileProperties1 {
	alt_u16 version;    			// 1
	alt_u16 headerSize; 			// This header size
	alt_u16 width;      			// In pixels
	alt_u16 height;     			// In pixels
	alt_u32 reserve0;
	alt_u32 backgroundColor;
	vdu_ref schemaId; 				// vdu_cstr
	vdu_ref caption; 				// vdu_cstr
	vdu_ref onShowScript;			// vdu_cstr
	vdu_ref onShowScriptBytecode;	// vdu_cstr
	vdu_ref preDrawScript;			// vdu_cstr
	vdu_ref preDrawScriptBytecode;	// vdu_cstr
	alt_u32 reserve1;
	alt_u32 reserve2;
};

struct VduSchemaFile
{
	// 1. Header and schema properties
	//
	char magic[4];				// "VDU\0"
	alt_u16 fileVersion;		// 1
	alt_u16 reserve1;

	struct VduSchemaFileProperties1 schemaProperties;
	alt_u32 reserve2[4];

	alt_u16 schemaItemCount;	// Number of schema items - each items is a VduSchemaFileSchemaItem1 + specific data
								// (VduSchemaFileSchemaItemLine1 | VduSchemaFileSchemaItemRect1 | ...).

	alt_u16 reserve3;

	// Next fields are present in a text description:

	// 2. SchemaItem Offset Table
	// Offsets to SchemaItem;
	// alt_u32 schemaItemOffset[schemaItemCount]

	// 3. SchemaItems[schemaItemCount]
	// SchemaItems: a list of schema items.
	// Schema item is a struct that starts with VduSchemaFileSchemaItem1 and is followed
	// by the data of the specific schema item like VduSchemaFileSchemaItemLine1, VduSchemaFileSchemaItemRect1, ...

	// 4. Strings
	// Strings: a list of strings.
	// see vdu_cstr

	// 5. crc64
	// alt_u64 crc64; // CRC64 of the file from the beginning to the end of the strings.
}; //*pSchemaFile;



typedef struct
{
	alt_u16 version;  // 1
	alt_u16 itemType; // VduFileSchemaItemLine, VduFileSchemaItemRect, ...
	alt_u32 reserve0;

	alt_u16 x1;
	alt_u16 y1;
	alt_u16 x2;
	alt_u16 y2;

	alt_u16 weight;
	alt_u16 reserve1;

	alt_u32 color;
	alt_u32 reserve2;
}  VduSchemaFileSchemaItemLine1;

typedef struct
{
	alt_u16 version;  // 2
	alt_u16 itemType; // VduFileSchemaItemRectId = 0x4352
	alt_u32 reserve0;

	alt_u16 left;
	alt_u16 top;
	alt_u16 width;
	alt_u16 height;

	alt_u16 weight;
	bool fill;
	bool drawRect;
	alt_u32 reserve1;

	alt_u32 lineColor;
	alt_u32 fillColor;
	alt_u32 textColor;
	alt_u32 reserve2;

	alt_u16 fontIndex; // Fonts are generated on build, each font is a folder with name as index, this folder contains font files.
	bool wordWrap;
	bool reserverBool1;
	
	char text[128];			// Text to display, in version 2, changed from vdu_cstr to char[]
	
	alt_u32 reserve4;

	alt_u32 align;      // HorzAlign | VertAlign
	alt_u32 reserve5;
} VduSchemaFileSchemaItemRect1;

typedef struct
{
	alt_u16 version;  // 1
	alt_u16 itemType; // VduFileSchemaItemValueId
	alt_u32 reserve0;

	alt_u16 left;
	alt_u16 top;
	alt_u16 width;
	alt_u16 height;

	alt_u16 weight;
	bool reserve1;
	bool drawRect;
	alt_u32 reserve2;

	alt_u32 lineColor;
	alt_u32 fillColor;
	alt_u32 textColor;

	alt_u32 reserve3[8];   // Reserved for different colors

	alt_u16 fontIndex;     // Fonts are generated on build, each font is a folder with name as index, this folder contains font files.
	alt_u16 reserve4;

	alt_u16 decimalPlaces; // Number of decimal places for floating point values.
	alt_u16 reserve5;

	alt_u32 align;          // HorzAlign | VertAlign
	alt_u32 reserve7;

	char text[128];			// Text to display

	alt_u32 appSignalCount;

	// Then follows appSignalCount * sizeof(alt_u32) appSignalIndexes
	//
	// alt_u32 appSignalIndexes[appSignalCount];
} VduSchemaFileSchemaItemValue1;

typedef struct 
{
	alt_u16 version;   // 1
	alt_u16 itemType;  // VduFileSchemaItemImageId = 0x4D47
	alt_u32 reserve0;

	alt_u16 left;
	alt_u16 top;
	alt_u16 width;
	alt_u16 height;

	alt_u32 reserve1;
	alt_u32 imageHash; // Image hash, used to generate image file name: [VduSubsystemId]/[VduEquipmentId]/Images/[ImageHash].bmp

	alt_u32 reserve3;
	alt_u32 reserve4;
} VduSchemaFileSchemaItemImage1;

struct ImageInternal
{
	alt_u16 version;   // 1
	alt_u16 reserve0;

	vdu_ref imageId;
	alt_u32 reserve1;

	alt_u32 imageHash; // Image hash, used to generate image file name: [VduSubsystemId]/[VduEquipmentId]/Images/[ImageHash].bmp
	alt_u32 reserve2;
};

typedef struct
{
	alt_u16 version;  // 1
	alt_u16 itemType; // VduFileSchemaItemImageValueId, 0x5649, IV
	alt_u32 reserve0;

	alt_u16 left;
	alt_u16 top;
	alt_u16 width;
	alt_u16 height;

	alt_u32 reserve4[4];

	alt_u16 reserve5;
	alt_u16 imageCount;    // 1 - 12

	struct ImageInternal images[12];

	alt_u16 currentImage;  // Placeholder for the currently selected image index in the 'images' array. Default value: 0.
	alt_u16 reserve6;

	alt_u32 appSignalCount;
	//
	// Then follows appSignalCount * sizeof(alt_u32) appSignalIndexes
	//
	// alt_u32 appSignalIndexes[appSignalCount];
} VduSchemaFileSchemaItemImageValue1;


enum Type
{
	i64 = 0,         // Not used yet, but reserved for future use.
	i32 = 1,
	f32 = 2,
	f64 = 3,         // Not used yet, but reserved for future use.
};

union Data
{
	alt_64 i64;     // For future use
	alt_32 i32;
	float f32;
	double f64;      // For future use
};

typedef struct
{
	alt_u16 type;       // 2 bytes
	alt_u16 reserve0;   // 2 bytes
	alt_u32 reserve1;   // 4 bytes
	union Data data;           // 8 bytes
} VduValue;

#define TREND_MAX_SIGNALS_COUNT 16u
#define TREND_MAX_DURATIONS_COUNT 16u

enum TrendViewMode
{
	Separate,
	Overlapped
};

enum TrendScaleType
{
	Linear,
	Logarithmic
};

enum DisplayValueFormat
{
	Auto,
	Decimal,
	Exponential
}; 

struct TrendSignal
{
	alt_u16 version; // 1
	alt_u16 reserve0;

	alt_u32 appSignalIndex;
	alt_u32 validityAppSignalIndex;
	union {
		//alt_u16 trendSignalFileRecordIndex; // Index of the record (struct TrendItemSignal) in file File::VDU_TREND_SIGNALS ("TrendSignals.dat")
		alt_u16 trendSignalDataRecordIndex; // In RVDU program, index of the record (struct TrendSignalData) in the memory, filled during initialization
	};

	alt_u16 reserve1;

	alt_16 decimalPlaces; // Number of decimal places for floating point values (-1, take precision from the signal).
	alt_u16 valueFormat;  // E::DisplayValueFormat, Auto = 0, Decimal = 1, Exponential = 2

	VduValue highViewLimit;
	VduValue lowViewLimit;

	alt_u32 color;
	alt_u16 lineWeight;

	union
	{
		alt_u16 reserve2;	// Stored in the file
		alt_u16 value;

		struct
		{
			bool visible : 1;
		} bits;

	} flags;

}; 

typedef struct
{
	alt_u16 version;  // 2
	alt_u16 itemType; // VduFileSchemaItemTrendId, 0x5254, TR
	alt_u32 reserve0;

	alt_u16 left;
	alt_u16 top;
	alt_u16 width;
	alt_u16 height;

	alt_16 indentLeft;
	alt_16 indentRight;
	alt_16 indentTop;
	alt_16 indentBottom;

	alt_u32 reserve4[4];

	alt_u32 durationsSecs[TREND_MAX_DURATIONS_COUNT];

	alt_u32 columnCount;

	alt_u16 viewMode;  // 0 - E::TrendViewMode::Separated,
	// 1 - E::TrendViewMode::Overlapped
	alt_u16 scaleType; // 0 - E::TrendScaleType::Linear
	// 1 - E::TrendScaleType::Log10
	// 2 - E::TrendScaleType::Period

	alt_u16 fontIndex;
	alt_u16 reserve5;

	alt_u32 lineColor;
	alt_u32 backColor;
	alt_u32 backColor1st;
	alt_u32 backColor2nd;

	bool showSignalIds;
	bool showSignalCaptions;
	bool showSignalScales;
	bool showTimeLabels;
	bool showDateLabels;
	bool use24hTimeFormat;
	bool reserve6;
	bool reserve7;
	bool reserve8;

	alt_u16 maxSignalCount;      // 16
	alt_u16 signalCount;         // Number of signals in the trend
	alt_u16 discreteSignalCount; // Number of discrete signals in the trend
	alt_u16 reserve9;
	
	alt_u32 reserve10;

	// Filled with signalCount TrendSignal structs, followed by (maxSignalCount - signalCount) empty TrendSignal structs.
	// The first discreteSignalCount TrendSignal structs are discrete signals.
	//

	struct TrendSignal trendSignals[TREND_MAX_SIGNALS_COUNT];
} VduSchemaFileSchemaItemTrend1;

// VduSchemaFileSchemaItem1 - Common schema item header, right after it follows specific schema item data.
//
struct VduSchemaFileSchemaItem1
{
	alt_u16 version;       // 2
	alt_u16 size;          // sizeof(VduSchemaFileSchemaItem1)
	alt_u16 itemType;      // VduFileSchemaItemLineId, VduFileSchemaItemRectId, ...
	alt_u16 reserve0;
	alt_u16 totalItemSize; // sizeof(VduSchemaFileSchemaItem1) + sizeof(VduSchemaFileSchemaItemLine1 | VduSchemaFileSchemaItemRect1 | ...)

	bool isStatic;          // If true, the item is static and can be cached.
	bool acceptClick;       // If true, the item can accept click events.

	vdu_ref objectName;
	vdu_ref label;

	vdu_ref clickScript;
	vdu_ref clickScriptBytecode;

	vdu_ref preDrawScript;
	vdu_ref preDrawScriptBytecode;

	alt_u32 reserve2;
	
	union
	{
		alt_u32 reserve3;
		alt_u32 scriptResult;
	};

	// This struct is followed by the data of the specific schema item like VduSchemaFileSchemaItemLine1, VduSchemaFileSchemaItemRect1,
	// VduSchemaFileSchemaItemValue1, ...
	union
	{
		VduSchemaFileSchemaItemLine1 line;
		VduSchemaFileSchemaItemRect1 rect;
		VduSchemaFileSchemaItemValue1 value;
		VduSchemaFileSchemaItemImage1 image;
		VduSchemaFileSchemaItemImageValue1 imageValue;
		VduSchemaFileSchemaItemTrend1 trend;
	};
};

#pragma pack(pop)

/*
Copies the contents of the all schemas files to memory at the address specified in the parameter.
Caches the bitmaps contained in schema items
Compiles the bytecode of scripts
Returns �true� if the copy is successful.
Returns �false� if an error.
Parameters:
    schemasPath - Path to fonts subfolders (e.g EQUIPMENTID_/Schemas).
    startAddr - 32-bit pointer to a buffer where to store the configuration.
    memoryAvailable - memory available for schemas data.
Return Value:
    bytesRead - receives number of actually read bytes. The size of each file is upward-aligned by 4 bytes (to dword).
    Returns "true" if successful, "false" on error.
*/
bool cacheSchemas(const char* const schemaPath, alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesRead);

// --------------- Schema access functions -------------------

// Returns a pointer to a cached schema by index or NULL if schema is not loaded or index is out of range.
//
struct VduSchemaFile* const vduSchemaByIndex(alt_u16 schemaIndex);

// Returns a pointer to a cached schema by schema ID or NULL if schema with the specified identifier not found.
//
struct VduSchemaFile* const vduSchemaByID(const char* schemaId);

// Returns a pointer to the schema element whose index is specified in the parameter.
//
struct VduSchemaFileSchemaItem1* const vduSchemaItem(struct VduSchemaFile* const schema, alt_u32 itemIndex);

// --------------- Schema strings access functions -------------------

// Returns a pointer to a schema string by its offset or the NULL if configuration is not loaded or the offset is out of range.
//
const char* const vduSchemaString(const struct VduSchemaFile* const schema, const vdu_ref offset);

// Returns a length of schema string by its offset or INVALID_LENGTH if configuration is not loaded or the offset is out of range.
//
const alt_u16 vduSchemaStringLength(const struct VduSchemaFile* const schema, const vdu_ref offset);

// Returns a pointer to a current schema string by its offset or the NULL if configuration is not loaded or the offset is out of range.
//
const char* const vduCurrentSchemaString(const vdu_ref offset);

// Returns a length of current schema string by its offset or INVALID_LENGTH if configuration is not loaded or the offset is out of range.
//
const alt_u16 vduCurrentSchemaStringLength(const vdu_ref offset);

// Returns a pointer to a current schema script by its offset or the NULL if configuration is not loaded or the offset is out of range.
//
const char* const vduCurrentSchemaScript(const vdu_ref offset);

// Returns a length of current schema script by its offset or INVALID_LENGTH if configuration is not loaded or the offset is out of range.
//
const alt_u32 vduCurrentSchemaScriptLength(const vdu_ref offset);

// --------------- Current schema control functions -------------------

// Returns a pointer to the currently displayed diagram.
//
const struct VduSchemaFile* const vduCurrentSchema(void);

// Immediately changes the block's displayed schema to the schema specified in the parameter.
//
bool vduSetCurrentSchema(const char* const shemaId);

// Returns a pointer to the schema element whose index is specified in the parameter.
//
struct VduSchemaFileSchemaItem1* const vduCurrentSchemaItem(alt_u32 itemIndex);

// Schedules the change of the schema at next loop.
//
bool vduScheduleSchemaChange(const char* const shemaId);

// Gets the pointer to a scheduled schema change and resets the schedule
//
const struct VduSchemaFile* const vduGetScheduledSchema(void);

// --------------- Schema items helper functions -------------------

// Returns a signal global index of a ShemaItemValue or INVALID_INDEX on wrong index
//
static inline alt_u16 schemaItemValueAppSignalIndex(const VduSchemaFileSchemaItemValue1* const schemaItem, alt_u16 index)
{
	if ((schemaItem == NULL) || (index >= schemaItem->appSignalCount))
	{
		return INVALID_INDEX;
	}
	alt_u32 result = ((const alt_u32*)((const alt_u8*)schemaItem + sizeof(VduSchemaFileSchemaItemValue1)))[index];
	return (alt_u16)result;
}

// Returns a signal global index of a ShemaItemImageValue or INVALID_INDEX on wrong index
//
static inline alt_u16 schemaItemImageValueAppSignalIndex(const VduSchemaFileSchemaItemImageValue1* const schemaItem, alt_u16 index)
{
	if ((schemaItem == NULL) || (index >= schemaItem->appSignalCount))
	{
		return INVALID_INDEX;
	}
	alt_u32 result = ((const alt_u32*)((const alt_u8*)schemaItem + sizeof(VduSchemaFileSchemaItemImageValue1)))[index];
	return (alt_u16)result;
}

// ------------------------------------ Bitmap functions -------------------------------------

// Returns a cached bitmap raw data by image hash or NULL for wrong hash
//
const alt_u8* const bitmapData(alt_u32 imageHash);

#endif
