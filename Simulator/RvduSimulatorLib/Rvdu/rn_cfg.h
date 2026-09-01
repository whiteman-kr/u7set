/*
Module title: Configuration File Processing.

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Loads and stores RVDU configuration settings.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_cfg initial update

-- *************************************************************
-- Version v1.01
-- Date:	01,05,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, code refactoring
-- Add function descriptions
-- Memory initialization review for all caching modules
-- Add check display resolution and error if resolution not supported
-- Add file version control
-- Add checking file's crc64
-- Add version history to module descriptions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- Changes: RADIX-3971, add GlobalScript
-- Added GlobalScript support
-- RADIX-3974, add 64-bit architecture support

-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#ifndef RN_CFG_H
#define RN_CFG_H

#include <alt_types.h>

#include <stdbool.h>

struct VduConfigDisplay1
{
	alt_u16 version; 		// 1
	alt_u16 size;    		// sizeof(VduConfigDisplay1)

	alt_u16 width;   		// In pixels
	alt_u16 height;  		// In pixels

	alt_u16 mode;    		// Color mode
	alt_u16 reserve0;

	alt_u32 reserve1;
	alt_u32 reserve2;

	alt_u32 startSchemaId;	// vdu_string_ref
	alt_u32 displayName;	// vdu_string_ref

	alt_u32 reserve3;
	alt_u32 reserve4;
};

struct VduConfigFile1
{
	char magic[8];    		// "VDUCFG1\0"

	alt_u16 version; 		// 1
	alt_u16 size;    		// sizeof(VduConfigFile1)
	alt_u32 reserve1;

	alt_u32 project; 		// vdu_string_ref
	alt_u32 buildNo;
	alt_u32 reserve2;

	alt_u32 equipmentId; 	// vdu_string_ref
	alt_u32 caption; 		// vdu_string_ref
	alt_u32 reserve3;
	alt_u32 reserve4;

	alt_u16 displayCount;
	alt_u16 reserve5;
	struct VduConfigDisplay1 display0;
	struct VduConfigDisplay1 display1;
	struct VduConfigDisplay1 display2;
	struct VduConfigDisplay1 display3;

	alt_u16 fontCount;
	alt_u16 reserve6;

	alt_u32 reserve7;
	alt_u32 reserve8;

	alt_u16 schemaCount;
	alt_u16 reserve9;

	// Right after this struct follows the schema data
	// 1. VduConfigSchema1 schema[schemaCount];
	// 2. Strings
	// 3. crc64
};

struct VduConfigSchema1
{
	alt_u16 version; 	// 1
	alt_u16 size;    	// sizeof(VduConfigSchema1)
	alt_u32 reserve0;

	alt_u32 schemaId; 	//vdu_string_ref
	alt_u32 caption; 	//vdu_string_ref

	alt_u32 reserve1;
	alt_u32 reserve2;

	alt_u64 crc64; 		// Schema file crc64
};

// Configuration loading functions

/*
Copies the contents of the VduConfig.bin file to memory at the address specified in the parameter.
Returns "true" if the copy is successful.
Returns "false" if an error occurs while opening or reading the configuration file or on the cache overflow.
Parameters:
    cfgFile - configuration file name.
    startAddr - 32-bit pointer to a buffer where to store the configuration.
    bufferSize - size, in bytes, of the buffer.
Return Value:
	bytesRead - receives number of actually read bytes.
	Returns "true" if successful, "false" on error.
*/
bool cacheConfigFile(const char* const cfgFile, alt_u8* const startAddr, const alt_u32 bufferSize, alt_u32* bytesRead);

/*
Copies the contents of the GlobalScript.lua file to memory at the address specified in the parameter.
Returns "true" if the copy is successful.
Returns "false" if an error occurs while opening or reading the configuration file or on the cache overflow.
Parameters:
    luaFile - global script file name.
    startAddr - 32-bit pointer to a buffer where to store the script.
    bufferSize - size, in bytes, of the buffer.
Return Value:
	bytesRead - receives number of actually read bytes plus 1 byte for terminating zero.
	Returns "true" if successful, "false" on error.

If the global script is absent, sets the pointer returned by vduConfigGlobalScript() to NULL.
*/
bool cacheGlobalScript(const char* const luaFile, alt_u8* const startAddr, const alt_u32 bufferSize, alt_u32* bytesRead);

// Configuration access functions

// Returns a pointer to a cached configuration or NULL if configuration is not loaded
//
const struct VduConfigFile1* const vduConfig(void);

// Returns a pointer to a cached globalScript or NULL if it is not loaded
//
const char* const vduConfigGlobalScript(void);

/*
Display information about all the schemas contained in file VduConfig.bin in this format:
schema <>, version <>, size <>, id = <>, caption <>
*/
void vduConfigPrintSchemasList(void);

// Returns a pointer to a cached schema configuration or NULL if configuration is not loaded or index is out of range
//
const struct VduConfigSchema1* const vduConfigSchema(const alt_u16 schemaIndex);

// Returns a pointer to a configuration string by its offset or the NULL if configuration is not loaded or the offset is out of range
//
const char* const vduConfigString(const alt_u32 offset);

#endif
