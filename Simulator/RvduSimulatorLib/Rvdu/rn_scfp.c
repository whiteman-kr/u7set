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

#include <string.h>
#include <assert.h>
#include <stddef.h>

#include "rn_scfp.h"
#include "rn_cfg.h"
#include "rn_diag.h"
#include "rn_utils.h"
#include "rn_vbuf.h"
#include "rn_trendproc.h"

#include "defs.h"

#include "fat_filelib.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct CachedBitmap
{
	alt_u32 imageHash;
	alt_u32 size;
	alt_u16 width;
	alt_u16 height;
	alt_u8* data;
};

struct SchemaData
{
	alt_u8* data;
	bool transparent;
};

// Private module data
//
static struct SchemaData m_schemas[MAX_SCHEMAS_COUNT] = { 0 };	// Schema data in the block cache memory

static struct VduSchemaFile* m_currentSchema = NULL;
static char m_currentSchemaID[STRING_MAX_LEN];

static const struct VduSchemaFile* m_scheduledSchema = NULL;

static struct CachedBitmap* m_cachedBitmap = NULL;	// Cached bitmap structures are stored here
static int m_cachedBitmapCount = 0;	// Cached bitmap structures count
//
// End of Private data

static bool schemaItemIsValid(const struct VduSchemaFileSchemaItem1* const schemaItem)
{
	char msg[STRING_MAX_LEN];

	if (schemaItem == NULL)
	{
		RN_ASSERT(schemaItem != NULL);
		return false;
	}

	if (schemaItem->version != SCHEMA_ITEM_VERSION)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItem has wrong version (%d), supported version is %d.",
			(int)schemaItem->version,
			(int)SCHEMA_ITEM_VERSION);
		logError(UnsupportedVersion, msg);
		return false;
	}

	bool wrongItemType = false;
	bool wrongVersion = false;

	switch (schemaItem->itemType)
	{
	case TYPE_LINE:
		if (schemaItem->line.version != TYPE_LINE_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItemLine has wrong version (%d), supported version is %d.",
				(int)schemaItem->line.version,
				(int)TYPE_LINE_VERSION);
			logError(UnsupportedVersion, msg);
			wrongVersion = true;
		}
		break;

	case TYPE_RECT:
		if (schemaItem->rect.version != TYPE_RECT_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItemRect has wrong version (%d), supported version is %d.",
				(int)schemaItem->rect.version,
				(int)TYPE_RECT_VERSION);
			logError(UnsupportedVersion, msg);
			wrongVersion = true;
		}
		break;

	case TYPE_VALUE:
		if (schemaItem->value.version != TYPE_VALUE_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItemValue has wrong version (%d), supported version is %d.",
				(int)schemaItem->value.version,
				(int)TYPE_VALUE_VERSION);
			logError(UnsupportedVersion, msg);
			wrongVersion = true;
		}
		break;

	case TYPE_IMAGE:
		if (schemaItem->image.version != TYPE_IMAGE_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItemImage has wrong version (%d), supported version is %d.",
				(int)schemaItem->image.version,
				(int)TYPE_IMAGE_VERSION);
			logError(UnsupportedVersion, msg);
			wrongVersion = true;
		}
		break;

	case TYPE_IMAGEVALUE:
		if (schemaItem->imageValue.version != TYPE_IMAGEVALUE_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItemImageValue has wrong version (%d), supported version is %d.",
				(int)schemaItem->imageValue.version,
				(int)TYPE_IMAGEVALUE_VERSION);
			logError(UnsupportedVersion, msg);
			wrongVersion = true;
		}
		break;

	case TYPE_TREND:
		if (schemaItem->trend.version != TYPE_TREND_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: VduSchemaFileSchemaItemTrend has wrong version (%d), supported version is %d.",
				(int)schemaItem->trend.version,
				(int)TYPE_TREND_VERSION);
			logError(UnsupportedVersion, msg);
			wrongVersion = true;
		}
		break;

	default:
	{
		(void)snprintf(msg, STRING_MAX_LEN, "schemaItemIsValid: unknown schema item type: 0x%x.",
			(int)TYPE_TREND_VERSION);
		logError(UnsupportedVersion, msg);
		wrongItemType = true;
	}
	break;
	}

	return (wrongVersion == false) && (wrongItemType == false);
}


// Cache specified schema file
//
static bool cacheSchemaFile(const char* fileName, alt_u8* const schemaStartAddr, alt_u64 configCrc, alt_u32 maxSchemaSize, alt_u32* bytesRead)
{
	if ((fileName == NULL) || (schemaStartAddr == NULL) || (bytesRead == NULL))
	{
		RN_ASSERT(fileName != NULL);
		RN_ASSERT(schemaStartAddr != NULL);
		RN_ASSERT(bytesRead != NULL);
		return false;
	}

	char msg[STRING_MAX_LEN];

	void* f = fl_fopen(fileName, "rb");
	if (f == NULL)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: schema file %s, file open error.", fileName);
		logError(OpenFileError, msg);
		return false;
	}
	else
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: %s.", fileName);
		logMessage(msg);
	}

	bool readOk = true;

	// Check file size
	//
	alt_u32 size = fileSize(f);

	if ((size == WRONG_FILE_SIZE) ||
		(size > maxSchemaSize) ||
		(size < (sizeof(struct VduSchemaFile) + sizeof(alt_u64))))
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: file has wrong size (%d), max size is %d bytes.",
				(int)size,
				(int)maxSchemaSize);
		logError(FileSizeError, msg);

		readOk = false;
	}

	// Read file data
	//
	if (readOk == true)
	{
		if (freadBigData(schemaStartAddr, size, f) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: file %s read error.", fileName);
			logError(ReadFileError, msg);

			readOk = false;
		}
	}

	(void)fl_fclose(f);

	if (readOk == true)
	{
		*bytesRead = size;

		const struct VduSchemaFile* const pSchema = (struct VduSchemaFile*)schemaStartAddr;

		if (pSchema->fileVersion > FILE_SCHEMA_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: file '%s' version (%u) is not supported, max supported version is (%u).",
				fileName,
				pSchema->fileVersion,
				FILE_SCHEMA_VERSION);
			logError(UnsupportedVersion, msg);

			readOk = false;
		}
	}

	if (readOk == true)
	{
		// Check crc64
		//
		alt_u64 crc = reverseBytes64(calcCrc64(schemaStartAddr, (*bytesRead) - sizeof(alt_u64)));
		const alt_u64* pFileCrc64 = ((alt_u64*)(schemaStartAddr + (*bytesRead) - sizeof(alt_u64)));
		if ((crc != *pFileCrc64) || (configCrc != *pFileCrc64))
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: file '%s' CRC64 is incorrect.", fileName);
			logError(FileFormatError, msg);

			readOk = false;
		}
	}

	// Check versions of all schema items
	//
	if (readOk == true)
	{
		struct VduSchemaFile* const pSchema = (struct VduSchemaFile*)schemaStartAddr;

		if (pSchema->schemaProperties.version != SCHEMA_PROP_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSchemaFile: file '%s' properties version (%u) is not supported, max supported version is (%u).",
				fileName,
				pSchema->schemaProperties.version,
				SCHEMA_PROP_VERSION);
			logError(UnsupportedVersion, msg);
			return false;
		}



		for (alt_u32 i = 0; i < pSchema->schemaItemCount; i++)
		{
			const struct VduSchemaFileSchemaItem1* const item = vduSchemaItem(pSchema, i);
			if (schemaItemIsValid(item) == false)
			{
				readOk = false;
				break;
			}
		}
	}

	// Initialize trend signal run-time flags
	//
	if (readOk == true)
	{
		struct VduSchemaFile* const pSchema = (struct VduSchemaFile*)schemaStartAddr;

		for (alt_u32 i = 0; i < pSchema->schemaItemCount; i++)
		{
			struct VduSchemaFileSchemaItem1* const item = vduSchemaItem(pSchema, i);
			if (item->itemType == TYPE_TREND)
			{
				for (alt_u16 si = 0; si < item->trend.signalCount; si++)
				{
					item->trend.trendSignals[si].flags.bits.visible = true;
				}
			}
		}
	}

	return readOk;
}

// Checks if bitmap is already cached
//
static bool cacheBitmapExists(alt_u32 hash)
{
	for (int e = 0; e < m_cachedBitmapCount; e++)
	{
		if (m_cachedBitmap[e].imageHash == hash)
		{
			return true;
		}
	}
	return false;
}

// Cache all bitmaps
//
static bool cacheBitmaps(const char* const schemaPath, alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesRead)
{
	if ((schemaPath == NULL) || (startAddr == NULL) || (bytesRead == NULL))
	{
		RN_ASSERT(schemaPath != NULL);
		RN_ASSERT(startAddr != NULL);
		RN_ASSERT(bytesRead != NULL);
		return false;
	}

	*bytesRead = 0;

	// Collect all bitmaps from schema items
	//
	alt_u16 schemaCount = vduConfig()->schemaCount;
	if (schemaCount > MAX_SCHEMAS_COUNT)
	{
		return false;
	}

	m_cachedBitmap = (struct CachedBitmap*)startAddr;

	for (alt_u16 i = 0; i < schemaCount; i++)
	{
		struct VduSchemaFile* const schema = vduSchemaByIndex(i);
		if (schema == NULL)
		{
			return false;
		}

		for (alt_u16 j = 0; j < schema->schemaItemCount; j++)
		{
			struct VduSchemaFileSchemaItem1* const item = vduSchemaItem(schema, j);
			if (item == NULL)
			{
				RN_ASSERT(item != NULL);
				continue;
			}

			if (item->itemType == TYPE_IMAGE)
			{
				// Add hash if it does not exist
				//
				if (cacheBitmapExists(item->image.imageHash) == false)
				{
					m_cachedBitmap[m_cachedBitmapCount].imageHash = item->image.imageHash;
					m_cachedBitmap[m_cachedBitmapCount].width = item->image.width;
					m_cachedBitmap[m_cachedBitmapCount].height = item->image.height;
					m_cachedBitmap[m_cachedBitmapCount].data = NULL;
					m_cachedBitmap[m_cachedBitmapCount].size = 0;
					m_cachedBitmapCount++;

					if (memoryAvailable < sizeof(struct CachedBitmap))
					{
						logError(FileSizeError, "cacheBitmaps: not enough memory for CachedBitmap structures.");
						return false;
					}
					memoryAvailable = DEC_MEMORY(memoryAvailable, sizeof(struct CachedBitmap));
					(*bytesRead) += sizeof(struct CachedBitmap);
				}

			}
			else if (item->itemType == TYPE_IMAGEVALUE)
			{
				for (alt_u16 q = 0; q < item->imageValue.imageCount; q++)
				{
					// Add hash if it does not exist
					//
					if (cacheBitmapExists(item->imageValue.images[q].imageHash) == false)
					{
						m_cachedBitmap[m_cachedBitmapCount].imageHash = item->imageValue.images[q].imageHash;
						m_cachedBitmap[m_cachedBitmapCount].width = item->imageValue.width;
						m_cachedBitmap[m_cachedBitmapCount].height = item->imageValue.height;
						m_cachedBitmap[m_cachedBitmapCount].data = NULL;
						m_cachedBitmap[m_cachedBitmapCount].size = 0;
						m_cachedBitmapCount++;

						if (memoryAvailable < sizeof(struct CachedBitmap))
						{
							logError(FileSizeError, "cacheBitmaps: not enough memory for CachedBitmap structures.");
							return false;
						}
						memoryAvailable = DEC_MEMORY(memoryAvailable, sizeof(struct CachedBitmap));
						(*bytesRead) += sizeof(struct CachedBitmap);
					}
				}
			}
			else
			{
				continue;
			}
		}
	}

	char msg[STRING_MAX_LEN];

	// Now load all bitmaps to memory
	//

	alt_u8* bmpData = startAddr + (*bytesRead);	// Bitmap data starts here

	for (int i = 0; i < m_cachedBitmapCount; i++)
	{
		char fileName[FILE_NAME_MAX_LEN];
		(void)snprintf(fileName, sizeof(fileName), "%s/Images/%08X.bmp", schemaPath, (unsigned int)m_cachedBitmap[i].imageHash);

		(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: loading bitmap file %s.", fileName);
		logMessage(msg);


		void* readFile = fl_fopen(fileName, "rb");
		if (readFile == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: can't open bitmap file %s.", fileName);
			logError(OpenFileError, msg);
			return false;
		}

		bool readOk = true;

		alt_u32 size = fileSize(readFile);
		if (size == WRONG_FILE_SIZE)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: get file size error: %s.", fileName);
			logError(FileSizeError, msg);

			readOk = false;
		}
		if (alignAddressToDword(size) > memoryAvailable)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: not enough memory to read file: %s.", fileName);
			logError(FileSizeError, msg);

			readOk = false;
		}

		if (readOk == true)
		{
			if (freadBigData(bmpData, size, readFile) == false)
			{
				(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: file %s read error.", fileName);
				logError(ReadFileError, msg);

				readOk = false;
			}
		}

		if (readOk == true)
		{
			// Read the BMP header
			//
			const struct BMPFileHeader* bmpHeader = (struct BMPFileHeader*)bmpData;
			const struct BMPInfoHeader* bmpInfo = (struct BMPInfoHeader*)(bmpData + sizeof(struct BMPFileHeader));

			// Check the BMP header
			//
			if ((bmpHeader->bfType != 0x4d42u) || // BM
				(bmpInfo->biSize != 40u) ||
				(bmpInfo->biPlanes != 1u) ||
				(bmpInfo->biBitCount != 24u) ||
				(bmpInfo->biCompression != 0u))
			{
				(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: file %s header error.", fileName);
				logError(FileFormatError, msg);

				readOk = false;
			}

			if (readOk == true)
			{
				if (((alt_u16)bmpInfo->biWidth != m_cachedBitmap[i].width) ||
					((alt_u16)bmpInfo->biHeight != m_cachedBitmap[i].height))
				{
					(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: file %s image size error.", fileName);
					logError(FileFormatError, msg);

					readOk = false;
				}
			}
		}

		if (readOk == true)
		{
			m_cachedBitmap[i].data = bmpData;
			m_cachedBitmap[i].size = size;

			size = alignAddressToDword(size);

			bmpData += size;
			(*bytesRead) += size;

			memoryAvailable = DEC_MEMORY(memoryAvailable, size);
		}

		(void)fl_fclose(readFile);

		if (readOk == false)
		{
			return false;
		}
	}

	(void)snprintf(msg, STRING_MAX_LEN, "cacheBitmaps: total %d bitmaps cached, %u KBytes used.", m_cachedBitmapCount, (unsigned int)(*bytesRead) / 1024u);
	logMessage(msg);

	return true;
}

// Compile bytecode help stuff
//
struct BytecodeArray
{
	alt_u8* startAddr;
	alt_u8* currAddr;
	alt_u32 size;
};

static int bytecodeWriter(lua_State* L, const void* p, size_t sz, void* ud)
{
	(void)L;

	struct BytecodeArray* arr = (struct BytecodeArray*)ud;

	//static_assert(sizeof(size_t) == sizeof(alt_u32), "");

	(void)memcpy(arr->currAddr, p, sz);
	arr->currAddr += (alt_u32)sz;

	arr->size += (alt_u32)sz;

	return 0;
}

typedef struct
{
	size_t size;
} ByteCounter;

static int countWriter(lua_State* L, const void* p, size_t sz, void* ud)
{
	(void)L;
	(void)p;

	ByteCounter* counter = (ByteCounter*)ud;
	counter->size += sz;
	return 0;
}

static bool compileScript(lua_State* L, 
	const struct VduSchemaFile* const schema, 
	alt_u32 codeRef,
	enum WarningCode warningCode,
	alt_u8* const bytecodeAddr,
	alt_u32 memoryAvailable,
	alt_u32* bytesUsed)
{
	*bytesUsed = 0;

	alt_u16 len = vduSchemaStringLength(schema, codeRef);
	if ((len == INVALID_LENGTH) || (len == 0u))
	{
		RN_ASSERT(false);
		return false;
	}

	if (luaL_loadbuffer(L, vduSchemaString(schema, codeRef), len, "") != LUA_OK)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "compileScript, compile error: %s", lua_tostring(L, -1));
		logWarning(warningCode, msg);

		lua_pop(L, 1);
		return false;
	}

	// Get the dump size
	//
	ByteCounter counter = { 0 };

	if (lua_dump(L, &countWriter, &counter, 0) != LUA_OK)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "compileScript, dump error: %s", lua_tostring(L, -1));
		logWarning(warningCode, msg);

		lua_pop(L, 1);
		return false;
	}

	if ((sizeof(alt_u32) /*length*/ + counter.size)  > memoryAvailable)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "compileScript, not enough memory for script dump.");
		logWarning(warningCode, msg);

		lua_pop(L, 1);
		return false;
	}

	// Dump to bytecode. Target structure is alt_32 size, then the bytecode body.
	//
	struct BytecodeArray arr = { 
		.startAddr = bytecodeAddr + sizeof(alt_u32), 
		.currAddr = bytecodeAddr + sizeof(alt_u32),
		.size = 0};

	// Dump the body
	//
	if (lua_dump(L, &bytecodeWriter, &arr, 0) != LUA_OK)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "compileScript, dump error: %s", lua_tostring(L, -1));
		logWarning(warningCode, msg);

		lua_pop(L, 1);
		return false;
	}

	// Copy the size
	//
	*(alt_u32*)bytecodeAddr = arr.size;

	// Remove compiled chunk
	//
	lua_pop(L, 1);

	*bytesUsed = sizeof(alt_u32) + arr.size;

	return true;
}

// Compile all bytecode
//
static bool compileSchemasBytecode(alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesUsed)
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	*bytesUsed = 0;

	// Collect all bytecode from schema items
	//
	alt_u16 schemaCount = vduConfig()->schemaCount;
	if (schemaCount > MAX_SCHEMAS_COUNT)
	{
		return false;
	}

	alt_u8* bytecodeAddr = (alt_u8*)startAddr;
	int scriptsCompiled = 0;

	for (alt_u16 i = 0; i < schemaCount; i++)
	{
		struct VduSchemaFile* const schema = vduSchemaByIndex(i);
		if (schema == NULL)
		{
			return false;
		}

		// Compile OnShowScript
		{
			alt_u16 len = vduSchemaStringLength(schema, schema->schemaProperties.onShowScript);
			if (len == INVALID_LENGTH)
			{
				return false;
			}

			if (len > 0u)
			{
				alt_u32 bytesUsedByScript = 0;

				if (compileScript(L, schema, schema->schemaProperties.onShowScript, 
					LuaPreDrawScript, bytecodeAddr, memoryAvailable, &bytesUsedByScript) == true)
				{
					// Set item's pointer to the compiled [size + bytecode]
					//
					ptrdiff_t offset = (alt_u8*)bytecodeAddr - (alt_u8*)schema;
					if (offset < 0)
					{
						RN_ASSERT(false);
						return false;
					}
					schema->schemaProperties.onShowScriptBytecode = (alt_u32)offset;

					bytesUsedByScript = alignAddressToDword(bytesUsedByScript);

					// Move the memory pointer
					//
					bytecodeAddr += bytesUsedByScript;

					// Increase the size
					//
					*bytesUsed += bytesUsedByScript;

					// Decrease the available memory
					//
					memoryAvailable = DEC_MEMORY(memoryAvailable, bytesUsedByScript);

					scriptsCompiled++;
				}
			}
		}

		// Compile PreDrawScript
		{
			alt_u16 len = vduSchemaStringLength(schema, schema->schemaProperties.preDrawScript);
			if (len == INVALID_LENGTH)
			{
				return false;
			}

			if (len > 0u)
			{
				alt_u32 bytesUsedByScript = 0;

				if (compileScript(L, schema, schema->schemaProperties.preDrawScript,
					LuaPreDrawScript, bytecodeAddr, memoryAvailable, &bytesUsedByScript) == true)
				{
					// Set item's pointer to the compiled [size + bytecode]
					//
					ptrdiff_t offset = (alt_u8*)bytecodeAddr - (alt_u8*)schema;
					if (offset < 0)
					{
						RN_ASSERT(false);
						return false;
					}
					schema->schemaProperties.preDrawScriptBytecode = (alt_u32)offset;

					bytesUsedByScript = alignAddressToDword(bytesUsedByScript);

					// Move the memory pointer
					//
					bytecodeAddr += bytesUsedByScript;

					// Increase the size
					//
					*bytesUsed += bytesUsedByScript;

					// Decrease the available memory
					//
					memoryAvailable = DEC_MEMORY(memoryAvailable, bytesUsedByScript);

					scriptsCompiled++;
				}
			}
		}

		// Compile Items Scripts

		for (alt_u16 j = 0; j < schema->schemaItemCount; j++)
		{
			struct VduSchemaFileSchemaItem1* const item = vduSchemaItem(schema, j);
			if (item == NULL)
			{
				RN_ASSERT(item != NULL);
				continue;
			}

			// Compile ClickScript

			alt_u16 len = vduSchemaStringLength(schema, item->clickScript);
			if (len == INVALID_LENGTH)
			{
				return false;
			}
			
			if (len > 0u)
			{
				alt_u32 bytesUsedByScript = 0;

				if (compileScript(L, schema, item->clickScript, LuaClickScript, bytecodeAddr, memoryAvailable, &bytesUsedByScript) == true)
				{
					// Set item's pointer to the compiled [size + bytecode]
					//
					ptrdiff_t offset = (alt_u8*)bytecodeAddr - (alt_u8*)schema;
					if (offset < 0)
					{
						RN_ASSERT(false);
						return false;
					}
					item->clickScriptBytecode = (alt_u32)offset;

					bytesUsedByScript = alignAddressToDword(bytesUsedByScript);

					// Move the memory pointer
					//
					bytecodeAddr += bytesUsedByScript;

					// Increase the size
					//
					*bytesUsed += bytesUsedByScript;

					// Decrease the available memory
					//
					memoryAvailable = DEC_MEMORY(memoryAvailable, bytesUsedByScript);

					scriptsCompiled++;
				}
			}

			// Compile PreDraeScript

			len = vduSchemaStringLength(schema, item->preDrawScript);
			if (len == INVALID_LENGTH)
			{
				return false;
			}

			if (len > 0u)
			{
				alt_u32 bytesUsedByScript = 0;

				if (compileScript(L, schema, item->preDrawScript, LuaPreDrawScript, bytecodeAddr, memoryAvailable, &bytesUsedByScript) == true)
				{
					// Set item's pointer to the compiled [size + bytecode]
					//
					ptrdiff_t offset = (alt_u8*)bytecodeAddr - (alt_u8*)schema;
					if (offset < 0)
					{
						RN_ASSERT(false);
						return false;
					}
					item->preDrawScriptBytecode = (alt_u32)offset;

					bytesUsedByScript = alignAddressToDword(bytesUsedByScript);

					// Move the memory pointer
					//
					bytecodeAddr += bytesUsedByScript;

					// Increase the size
					//
					*bytesUsed += bytesUsedByScript;

					// Decrease the available memory
					//
					memoryAvailable = DEC_MEMORY(memoryAvailable, bytesUsedByScript);

					scriptsCompiled++;
				}
			}
		}
	}

	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "compileSchemasBytecode: total %d scripts compiled, %u KBytes used.", scriptsCompiled, (unsigned int)(*bytesUsed) / 1024u);
	logMessage(msg);

	lua_close(L);

	return true;
}

bool cacheSchemas(const char* const schemaPath, alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesRead)
{
	if ((schemaPath == NULL) || (startAddr == NULL) || (bytesRead == NULL))
	{
		RN_ASSERT(schemaPath != NULL);
		RN_ASSERT(startAddr != NULL);
		RN_ASSERT(bytesRead != NULL);
		return false;
	}

	alt_u16 schemaCount = vduConfig()->schemaCount;
	if (schemaCount > MAX_SCHEMAS_COUNT)
	{
		return false;
	}

	alt_u8* schemaStartAddr = startAddr;

	*bytesRead = 0;
	m_currentSchemaID[0] = '\0';

	for (alt_u16 i = 0u; i < schemaCount; i++)
	{
		char fileName[FILE_NAME_MAX_LEN];
		const struct VduConfigSchema1* const pSchema = vduConfigSchema(i);
		(void)snprintf(fileName, FILE_NAME_MAX_LEN, "%s/%s.vbs", schemaPath, vduConfigString(pSchema->schemaId));

		alt_u32 schemaBytesRead = 0;
		bool result = cacheSchemaFile(fileName, schemaStartAddr, pSchema->crc64, memoryAvailable, &schemaBytesRead);
		if (result == false)
		{
			return false;
		}

		m_schemas[i].data = schemaStartAddr;
		m_schemas[i].transparent = true;

		// Number of bytes read from the file is upward-rounded to dword-size,
		// because all fonts data shall be aligned by dword
		//
		schemaBytesRead = alignAddressToDword(schemaBytesRead);

		schemaStartAddr += schemaBytesRead;
		(*bytesRead) += schemaBytesRead;

		memoryAvailable = DEC_MEMORY(memoryAvailable, schemaBytesRead);
	}

	// Cache bitmaps
	//
	{
		alt_u8* bitmapStartAddr = startAddr + (*bytesRead);

		alt_u32 bitmapsBytesRead = 0;
		bool result = cacheBitmaps(schemaPath, bitmapStartAddr, memoryAvailable, &bitmapsBytesRead);
		if (result == false)
		{
			return false;
		}

		bitmapsBytesRead = alignAddressToDword(bitmapsBytesRead);
		(*bytesRead) += bitmapsBytesRead;

		memoryAvailable = DEC_MEMORY(memoryAvailable, bitmapsBytesRead);
	}

	// Compile bytecode
	//
	{
		alt_u8* bytecodeStartAddr = startAddr + (*bytesRead);

		alt_u32 bytecodeBytesUsed = 0;
		bool result = compileSchemasBytecode(bytecodeStartAddr, memoryAvailable, &bytecodeBytesUsed);
		if (result == false)
		{
			return false;
		}

		bytecodeBytesUsed = alignAddressToDword(bytecodeBytesUsed);
		(*bytesRead) += bytecodeBytesUsed;

		// Uncomment "memoryAvailable = DEC_MEMORY(memoryAvailable, bytecodeBytesUsed);" if more operations added
	}

	return true;
}

struct VduSchemaFile* const vduSchemaByIndex(alt_u16 schemaIndex)
{
	if ((vduConfig() == NULL) || (schemaIndex >= vduConfig()->schemaCount))
	{
		RN_ASSERT(false);
		return NULL;
	}

	struct VduSchemaFile* const result = (struct VduSchemaFile* const)m_schemas[schemaIndex].data;

	if (strncmp(result->magic, "VDU", strlen("VDU")) != 0)
	{
		return NULL;
	}
	
	return result;
}

struct VduSchemaFile* const vduSchemaByID(const char* schemaId)
{
	if (vduConfig() == NULL)
	{
		RN_ASSERT(false);
		return NULL;

	}

	if (schemaId == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "vduSchemaByID: schemaId = NULL.");
		logWarning(SchemaWasNotFound, msg);
		return NULL;
	}

	alt_u16 schemaCount = vduConfig()->schemaCount;
	for (alt_u16 i = 0u; i < schemaCount; i++)
	{
		struct VduSchemaFile* const result = (struct VduSchemaFile* const)m_schemas[i].data;
		const char* str = (const char* const)result + result->schemaProperties.schemaId + sizeof(alt_u16) /*skip string size*/;

		if (strcmp(schemaId, str) == 0)
		{
			return result;
		}
	}

	return NULL;
}

struct VduSchemaFileSchemaItem1* const vduSchemaItem(struct VduSchemaFile* const schema, alt_u32 itemIndex)
{
	if (schema == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "vduSchemaItem: schema = NULL.");
		logWarning(SchemaWasNotFound, msg);
		return NULL;
	}

	if (itemIndex >= schema->schemaItemCount)
	{
		RN_ASSERT(false);
		return NULL;
	}

	const alt_u32* itemOffsets = (const alt_u32*)((alt_u8*)schema + sizeof(struct VduSchemaFile));

	return (struct VduSchemaFileSchemaItem1* const)((alt_u8*)schema + itemOffsets[itemIndex]);
}

const char* const vduSchemaString(const struct VduSchemaFile* const schema, const alt_u32 offset)
{
	if (schema == NULL)
	{
		RN_ASSERT(false);
		return NULL;
	}

	const char* str = (const char*)schema + offset + sizeof(alt_u16) /*skip string size*/;

	return str;
}

const alt_u16 vduSchemaStringLength(const struct VduSchemaFile* const schema, const alt_u32 offset)
{
	if (schema == NULL)
	{
		RN_ASSERT(false);
		return INVALID_LENGTH;
	}

	alt_u16 result = *(const alt_u16*)((const alt_u8*)schema + offset);
	return result;
}

const char* const vduCurrentSchemaString(const alt_u32 offset)
{
	if (m_currentSchema == NULL)
	{
		RN_ASSERT(false);
		return NULL;
	}
	const char* str = (const char*)m_currentSchema + offset + sizeof(alt_u16) /*skip string size*/;
	return str;
}

const alt_u16 vduCurrentSchemaStringLength(const alt_u32 offset)
{
	if (m_currentSchema == NULL)
	{
		RN_ASSERT(false);
		return INVALID_LENGTH;
	}
	alt_u16 result = *(const alt_u16*)((const alt_u8*)m_currentSchema + offset);
	return result;
}

// Returns a pointer to a current schema script by its offset or the NULL if configuration is not loaded or the offset is out of range.
//
const char* const vduCurrentSchemaScript(const alt_u32 offset)
{
	if (m_currentSchema == NULL)
	{
		RN_ASSERT(false);
		return NULL;
	}
	const char* str = (const char*)m_currentSchema + offset + sizeof(alt_u32) /*skip data size*/;
	return str;
}

// Returns a length of current schema script by its offset or the NULL if configuration is not loaded or the offset is out of range.
//
const alt_u32 vduCurrentSchemaScriptLength(const alt_u32 offset)
{
	if (m_currentSchema == NULL)
	{
		RN_ASSERT(false);
		return INVALID_LENGTH;
	}
	alt_u32 result = *(const alt_u32*)((const alt_u8*)m_currentSchema + offset);
	return result;
}

const struct VduSchemaFile* const vduCurrentSchema(void)
{
	return m_currentSchema;
}

bool vduSetCurrentSchema(const char* const shemaId)
{
	char msg[STRING_MAX_LEN];
	if (shemaId == NULL)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "vduSetCurrentSchema: shemaId = NULL.");
		logWarning(SchemaWasNotFound, msg);
		return false;
	}

	if (strcmp((char*)m_currentSchemaID, shemaId) != 0)
	{
		struct VduSchemaFile* const newSchema = vduSchemaByID((const char*)shemaId);
		if (newSchema != NULL)
		{
			(void)strncpy((char*)m_currentSchemaID, (const char*)shemaId, STRING_MAX_LEN);
			m_currentSchema = newSchema;

			(void)snprintf(msg, STRING_MAX_LEN, "vduSetCurrentSchema: %s.", shemaId);
			logMessage(msg);

			return true;
		}
		else
		{
			(void)snprintf(msg, STRING_MAX_LEN, "vduSetCurrentSchema: schema not found.");
			logWarning(SchemaWasNotFound, msg);
		}
	}
	return false;
}

struct VduSchemaFileSchemaItem1* const vduCurrentSchemaItem(alt_u32 itemIndex)
{
	if ((m_currentSchema == NULL) || (itemIndex >= m_currentSchema->schemaItemCount))
	{
		RN_ASSERT(false);
		return NULL;
	}

	const alt_u32* itemOffsets = (const alt_u32*)((alt_u8*)m_currentSchema + sizeof(struct VduSchemaFile));
	return (struct VduSchemaFileSchemaItem1* const)((alt_u8*)m_currentSchema + itemOffsets[itemIndex]);
}

bool vduScheduleSchemaChange(const char* const shemaId)
{
	char msg[STRING_MAX_LEN];

	if (shemaId == NULL)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "vduScheduleSchemaChange: shemaId = NULL.");
		logWarning(SchemaWasNotFound, msg);
		return false;
	}

	if (strcmp((char*)m_currentSchemaID, shemaId) != 0)
	{
		const struct VduSchemaFile* const newSchema = vduSchemaByID((const char*)shemaId);
		if (newSchema != NULL)
		{
			m_scheduledSchema = newSchema;
			return true;
		}
		else
		{
			(void)snprintf(msg, STRING_MAX_LEN, "vduScheduleSchemaChange: schema not found.");
			logWarning(SchemaWasNotFound, msg);
		}
	}
	return false;
}

const struct VduSchemaFile* const vduGetScheduledSchema(void)
{
	const struct VduSchemaFile* const result = m_scheduledSchema;
	m_scheduledSchema = NULL;
	return result;
}

const alt_u8* const bitmapData(alt_u32 imageHash)
{
	for (int i = 0; i < m_cachedBitmapCount; i++)
	{
		if (m_cachedBitmap[i].imageHash == imageHash)
		{
			return m_cachedBitmap[i].data;
		}
	}
	return NULL;
}

