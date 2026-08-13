/*
Module title: Description File Processing.

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Reads and interprets description files that define the list of 
available RVDUs in the subsystem, Equipment identifiers, Configuration numbers and Jumpers configuration.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.
-- Changes: RADIX-3966, rn_desc initial update

-- *************************************************************
-- Version v1.01
-- Date:	01,05,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_desc module refactoring
-- Change sprintf to snprintf, strcmp to strncmp for MISRA suggestion
-- Add version history to module descriptions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include "rn_desc.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include "defs.h"

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include <system.h>
#include "../fat_io_lib/fat_filelib.h"
#include "../rvdu_bsp/HAL/inc/sys/alt_errno.h"

#endif // End of Microsoft Visual Studio-specific

#include <string.h>
#include <stdlib.h>
#include <errno.h>

bool findEquipmentId(alt_u16 equipmentKey, const char* const descriptionFile, char* const equipmentId, alt_u16 bufferSize)
{
	if ((descriptionFile == NULL) ||
		(equipmentId == NULL) || 
		(bufferSize < MAX_KEY_LEN))
	{
		RN_ASSERT(false);
		return false;
	}

	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "findEquipmentId: searching %d key in '%s'.", equipmentKey, descriptionFile);
	logMessage(msg);

	void* f = fl_fopen(descriptionFile, "rb");
	if (f == NULL)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "findEquipmentId: failed to open '%s' file.", descriptionFile);
		logError(OpenFileError, msg);
		return false;
	}

	// JSON processing state
	//
	enum
	{
		NotProcessing,		// State when we are expecting to read the name of the key
		EquipmentIdFound,	// State when we are expecting to read the EquipmentID
		JumpersFound		// State when we are expecting to read the Jumpers code
	} lastKey = NotProcessing;

	bool fileMalformed = false;	// File format error flag, if is set - function stops processing
	bool keyFound = false;		// Key found flag, if is set - function stops processing and returns the result

	bool expectColon = false;
	bool haveEquipmentId = false;

	char keyValue[MAX_KEY_LEN] = { 0 };
	char equipmentIdValue[MAX_KEY_LEN] = { 0 };

	alt_32 dataRead = fl_fgetc(f);
	while ((dataRead != EOF) && (fileMalformed == false) && (keyFound == false))
	{
		char byte = (char)dataRead;

		if (expectColon == true)
		{
			if (byte == ':')
			{
				expectColon = false;
			}
			else
			{
				if ((byte != ' ') && (byte != '\n') && (byte != '\r') && (byte != '\t') && (byte != '\v'))
				{
					logError(FileFormatError, "findEquipmentId: wrong symbol in the file.");
					fileMalformed = true;
					break;
				}
			}
		}

		// Check if its start of key
		//
		if (byte == '\"')
		{
			// Start loading the key
			//
			alt_u16 keyPos = 0;
			(void)memset(keyValue, 0, sizeof(keyValue));

			for (int r = 0; r < 1048576; r++)	// Limit the file size to 1 Megabyte to prevent infinite loop
			{
				dataRead = fl_fgetc(f);
				if (dataRead == EOF)
				{
					logError(UnexpectedEOF, "findEquipmentId: unexpected end of file.");
					fileMalformed = true;
					break;
				}
				byte = (char)dataRead;

				// Check if key is too long
				//
				if ((keyPos >= MAX_KEY_LEN) || (keyPos >= bufferSize))
				{
					logError(StringTooLong, "findEquipmentId: a key in the file is too long.");
					fileMalformed = true;
					break;
				}

				// Write the next byte of the file to the buffer
				//
				keyValue[keyPos] = (char)byte;

				// Skip incorrect key
				//
				if ((keyValue[keyPos] == ':') || (keyValue[keyPos] == ','))
				{
					logError(FileFormatError, "findEquipmentId: a key in the file contains \':\' or \',\'.");
					fileMalformed = true;
					break;
				} // End of Skip incorrect key

				// Check if this the end of a key
				//
				if (keyValue[keyPos] == '\"')
				{
					// Key is read, place the termination zero
					//
					keyValue[keyPos] = '\0';

					if (lastKey == EquipmentIdFound)
					{
						// Key next to EquipmentId
						//
						(void)strncpy(equipmentIdValue, keyValue, MAX_KEY_LEN);
						haveEquipmentId = true;
					}
					else
					{
						if ((haveEquipmentId == true) && (lastKey == JumpersFound))
						{
							// Key next to Jumpers, compare it with the hardware-set key
							//
							char* lastChar = NULL;
							errno = 0;
							alt_u16 keyCode = (alt_u16)strtol(keyValue, &lastChar, 10);
							if ((strlen(keyValue) == 0u) || (errno != 0) || (*lastChar != '\0'))
							{
								// Invalid number format
								//
								(void)snprintf(msg, STRING_MAX_LEN, "findEquipmentId: a code in the file has invalid format: %s",
									keyValue);
								logError(FileFormatError, msg);

								fileMalformed = true;
								break;
							}

							if (keyCode == equipmentKey)
							{
								keyFound = true;
								break;
							}
							else
							{
								haveEquipmentId = false;
							}
						}
						else
						{
							// No special tag, processing the key name	
						}
					}

					// Set the corresponding JSON state
					//
					if (strncmp(keyValue, "EquipmentId", strlen("EquipmentId")) == 0)
					{
						lastKey = EquipmentIdFound;
						expectColon = true;
					}
					else if (strncmp(keyValue, "Jumpers", strlen("Jumpers")) == 0)
					{
						lastKey = JumpersFound;
						expectColon = true;
					}
					else
					{
						lastKey = NotProcessing;
					}

					break;
				} // End of the key

				keyPos++;

			} // key loading "for" loop
		} // 'end of"\"

		dataRead = fl_fgetc(f);
	}

	if (fileMalformed == false)
	{
		if (keyFound == true)
		{
			logMessage("findEquipmentId: EquipmentId found for the block jumper combination.");

			// Save the result
			//
			equipmentId[0] = '\0';
			(void)snprintf(equipmentId, bufferSize, "%s", equipmentIdValue);
		}
		else
		{
			logError(EquipmentKeyWasNotFound, "findEquipmentId: EquipmentId for the block jumper combination was not found.");
		}
	}

	(void)fl_fclose(f);

	if ((fileMalformed == true) || (keyFound == false))
	{
		equipmentId[0] = '\0';
		return false;
	}

	if (strlen(equipmentId) == 0u)
	{
		logError(EquipmentKeyWasNotFound, "findEquipmentId: Block jumper combination is found, but EquipmentId is empty.");
		return false;
	}

	return true;
}
