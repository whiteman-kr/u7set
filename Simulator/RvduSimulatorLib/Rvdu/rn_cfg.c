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

#include <stdbool.h>

#include "rn_cfg.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include "defs.h"

#include "fat_filelib.h"

// Private module data
//
static alt_u32 m_configurationSize = 0;

static struct VduConfigFile1* m_pVduConfigFile = NULL;
static struct VduConfigSchema1* m_pVduConfigSchema = NULL;
static char* m_pVduConfigGlobalScript = NULL;
//
// End of Private data
					
bool cacheConfigFile(const char* const cfgFile, alt_u8* const startAddr, const alt_u32 bufferSize, alt_u32* bytesRead)
{
	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "cacheConfigFile: loading from %s.", cfgFile);
	logMessage(msg);

	void* f = fl_fopen(cfgFile, "rb");
	if (f == NULL)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheConfigFile: file %s open error.", cfgFile);
		logError(OpenFileError, msg);
		return false;
	}

	bool readOk = true;

	// Check file size
	//
	alt_u32 size = fileSize(f);
	const alt_u32 minFileSize = sizeof(struct VduConfigFile1) + sizeof(alt_u64);

	if ((size == WRONG_FILE_SIZE) ||
		(size < minFileSize) ||
		(size > bufferSize))
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheConfigFile: file has wrong size (%d), expected %d .. %d bytes.",
			(int)size,
			(int)minFileSize,
			(int)bufferSize);
		logError(FileSizeError, msg);

		readOk = false;
	}

	// Read file data
	//
	if (readOk == true)
	{
		if (freadBigData(startAddr, size, f) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheConfigFile: file %s read error.", cfgFile);
			logError(ReadFileError, msg);

			readOk = false;
		}
	}

	(void)fl_fclose(f);

	if (readOk == true)
	{
		*bytesRead = size;

		// Setup pointers
		//
		m_pVduConfigFile = (struct VduConfigFile1*)startAddr;
		m_pVduConfigSchema = (struct VduConfigSchema1*)(startAddr + sizeof(struct VduConfigFile1));

		// Check file version
		//
		if (m_pVduConfigFile->version != FILE_CFG_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheConfigFile: file '%s' version (%d) is not supported, max supported version is (%u).",
				cfgFile,
				m_pVduConfigFile->version,
				FILE_CFG_VERSION);
			logError(UnsupportedVersion, msg);

			return false;
		}

		// Check crc64
		//
		alt_u64 crc = reverseBytes64(calcCrc64(startAddr, size - sizeof(alt_u64)));
		const alt_u64* pFileCrc64 = ((alt_u64*)(startAddr + size - sizeof(alt_u64)));	
		if (crc != *pFileCrc64)
		{
			logError(FileFormatError, "cacheConfigFile: file CRC64 is incorrect.");
			return false;
		}
		
		m_configurationSize = size - sizeof(alt_u64);

		return true;
	}

	return false;
}

bool cacheGlobalScript(const char* const luaFile, alt_u8* const startAddr, const alt_u32 bufferSize, alt_u32* bytesRead)
{
	if (bufferSize == 0u) 
	{
		logError(OpenFileError, "cacheGlobalScript: buffer is empty.");
		return false;
	}
	alt_u32 maxFileSize = bufferSize - 1u;

	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "cacheGlobalScript: loading from %s.", luaFile);
	logMessage(msg);

	void* f = fl_fopen(luaFile, "rb");
	if (f == NULL)
	{
		logError(OpenFileError, "cacheGlobalScript: Global Script file read error.");
		return false;
	}

	bool readOk = true;

	// Check file size, it shall be at least smaller by 1 byte than the buffer to include "\0"
	//
	alt_u32 size = fileSize(f);

	if ((size == WRONG_FILE_SIZE) ||
		(size > maxFileSize))
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheGlobalScript: file has wrong size (%d), max size is %d bytes.",
				(int)size,
				(int)maxFileSize);
		logError(FileSizeError, msg);

		readOk = false;
	}

	// Read file data
	//
	if (readOk == true)
	{
		if (freadBigData(startAddr, size, f) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheGlobalScript: file %s read error.", luaFile);
			logError(ReadFileError, msg);

			readOk = false;
		}
	}

	(void)fl_fclose(f);

	if (readOk == true)
	{
		alt_u32 readbytescount = size;

		// Setup pointers
		//
		if (readbytescount > 0u)
		{
			// Place terminating zero at the end
			//
			startAddr[readbytescount] = 0;
			readbytescount++;

			m_pVduConfigGlobalScript = (char*)startAddr;
		}
		else
		{
			logMessage("cacheGlobalScript: the global script is empty.");
		}

		*bytesRead = readbytescount;

		return true;
	}

	return false;
}

const struct VduConfigFile1* const vduConfig(void)
{
	return m_pVduConfigFile;
}

const char* const vduConfigGlobalScript(void)
{
	return m_pVduConfigGlobalScript;
}

void vduConfigPrintSchemasList(void)
{
	const struct VduConfigFile1* cfg = vduConfig();
	if (cfg == NULL)
	{
		RN_ASSERT(cfg != NULL);
		return;
	}

	for (alt_u16 i = 0; i < cfg->schemaCount; i++)
	{
		const struct VduConfigSchema1* const pSchema = vduConfigSchema(i);
		if (pSchema == NULL)
		{
			RN_ASSERT(pSchema != NULL);
			continue;
		}

		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "schema %u, version %x, size %x, id = %s, caption = %s",
			i, pSchema->version, pSchema->size, vduConfigString(pSchema->schemaId), vduConfigString(pSchema->caption));
		logMessage(msg);
	}
	return;
}

const struct VduConfigSchema1* const vduConfigSchema(const alt_u16 schemaIndex)
{
	if ((m_pVduConfigSchema == NULL) || (schemaIndex >= m_pVduConfigFile->schemaCount))
	{
		return NULL;
	}

	struct VduConfigSchema1* result = (struct VduConfigSchema1*)((char*)m_pVduConfigSchema + sizeof(struct VduConfigSchema1) * schemaIndex);
	return result;
}

const char* const vduConfigString(const alt_u32 offset)
{
	if ((m_pVduConfigFile == NULL) || (offset >= m_configurationSize))
	{
		return NULL;
	}

	char* str = (char*)m_pVduConfigFile + offset + sizeof(alt_u16) /*skip string size*/;
	return str;
}


