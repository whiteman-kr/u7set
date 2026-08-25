/*
Module title: Program main module

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Initializes the RVDU and runs the main loop

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Mazyr S.M.

-- Changes: code refactoring
-- Added GlobalScript support
-- Add startup stages for all modules
-- Added performance counters support to display FPS and scripts executing time
-- Add vary screen resolution support

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************
-- Version v1.02
-- Date:	25,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4095
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-4095, improve memory allocation control
-- Add memory allocation control
-- *************************************************************

-- Version v1.03
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

*/

#include <stdlib.h>
#include <string.h>

#include <system.h>

#include <altera_avalon_pio_regs.h>
#include <Altera_UP_SD_Card_Avalon_Interface.h>
#include <altera_avalon_performance_counter.h>

#include "fat_filelib.h"

#include <alt_stdio.h>

#include "defs.h"
#include "rn_ts.h"
#include "rn_kbd.h"
#include "rn_cfg.h"
#include "rn_diag.h"
#include "rn_fofp.h"
#include "rn_scfp.h"
#include "rn_lapi.h"
#include "rn_sifp.h"
#include "rn_desc.h"
#include "rn_vbuf.h"
#include "rn_schema.h"
#include "rn_prim.h"
#include "rn_opto.h"
#include "rn_utils.h"
#include "rn_trendproc.h"
#include "rn_trendview.h"


int main()
{
	setStartupStage(STAGE_MEM_INIT);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	IOWR_ALTERA_AVALON_PIO_DATA(FRAME_SW_BASE, 0u);
	IOWR_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE, 0u);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// Memory initialization
	// Memory starts with reserved area for the video buffer, 4 bytes per pixel
	// After this, the configuration data is written.
	//
	alt_u32 vBufSize = MAX_SCREEN_WIDTH * MAX_SCREEN_HEIGHT * 4u;

	alt_u32  MEM_IF_DDR3_EMIF_0_End = EMIF_0_ECC_CORE_BASE + vBufSize;

	alt_u32 memoryAvailable = EMIF_0_ECC_CORE_SPAN - vBufSize;

	// Initialize memory for log
	//
	{
		setStartupStage(STAGE_LOG_INIT);

		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);

		const alt_u16 recordCount = 1000u;

		alt_u32 bytesRead = 0;
		if (moduleRnDiagInitialize((alt_u8*)startAddr, recordCount, memoryAvailable, &bytesRead) == false)
		{
			setStartupStage(STAGE_MEM_ERR);
			return 0;
		}

		MEM_IF_DDR3_EMIF_0_End += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("moduleRnDiagInitialize", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);
	}

	// Initialize the input
	//
	{
		setStartupStage(STAGE_KBD_INIT);
		moduleRnKbdInitialize();
	}

	{
		setStartupStage(STAGE_TS_INIT);
		moduleRnTsInitialize();
	}

	// SD Card
	//
	{
		setStartupStage(STAGE_SD_INIT);

		alt_printf("Check SD card... ");
		const alt_up_sd_card_dev *sdhc = alt_up_sd_card_open_dev(ALTERA_UP_SD_CARD_AVALON_INTERFACE_0_NAME);
		if (sdhc != 0)
		{
			while(alt_up_sd_card_is_Present() == false)
			{
				static alt_u32 color = 1u;
				IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_RS485_DE_BASE, 1u);
				(void)writeVideoBuffer(color);

				IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_RS485_DE_BASE, 0u);
				(void)pushVideoBuffer();

				if (color < 0x800000u)
				{
					color = color <<1;
				}
				else
				{
					color = 1u;
				}
			}
		}
		else            
		{
			setStartupStage(STAGE_SD_ERR);
			return 0;
		}

		if (alt_up_sd_card_is_Present() == true)
		{
			if (alt_up_sd_card_is_FAT16() == true)
			{
				alt_printf("FAT16\n");
			}
			else
			{
				setStartupStage(STAGE_SD_FS_ERR);
				alt_printf("not FAT16\n");
				return 0;
			}
		}
	}

	// FAT library initialization
	//
	{
		setStartupStage(STAGE_FAT_INIT);
		fl_init();
		if (fl_attach_media(sd_readsector, sd_writesector) != FAT_INIT_OK)
		{
			setStartupStage(STAGE_FAT_ERR);
			alt_printf("ERROR: Failed to init file system\n");
			return 0;
		}
	}

	// Initialize working paths
	//
	char workFolder[STRING_MAX_LEN] = { 0 };

	// Initialize Equipment Id
	//
	{
		setStartupStage(STAGE_EQUIPMENT_INIT);

		static const alt_u32 OFFSET_OF_SUBSYSTEMKEY = 20495u * 2u;
		const alt_u16 equipmentKey = IORD_16DIRECT(TO_EXTERNAL_BUS_BRIDGE_1_BASE, OFFSET_OF_SUBSYSTEMKEY);

		char descFile[STRING_MAX_LEN] = { 0 };
		(void)snprintf(descFile, STRING_MAX_LEN, "/Description.json");

		char equipmentId[STRING_MAX_LEN] = { 0 };

		bool result = findEquipmentId(equipmentKey, descFile, equipmentId, STRING_MAX_LEN);
		if (result == true)
		{
			(void)snprintf(workFolder, STRING_MAX_LEN, "/%s", equipmentId);
		}
		else
		{
			setStartupStage(STAGE_EQUIPMENT_ERR);
			alt_printf("findEquipmentId failed.");
			return 0;
		}
	}

	// Load configuration file
	//
	{
		setStartupStage(STAGE_CFG_READ);

		char cfgFile[STRING_MAX_LEN] = { 0 };
		(void)snprintf(cfgFile, STRING_MAX_LEN, "%s/VduConfig.bin", workFolder);

		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);

		alt_u32 bytesRead = 0u;
		bool result = cacheConfigFile(cfgFile, (alt_u8* const)startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_CFG_ERR);
			alt_printf("cacheConfigFile failed.");
			return 0;
		}

		// Update the memory state constants
		//
		MEM_IF_DDR3_EMIF_0_End = startAddr + bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("cacheConfigFile", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);

		// Check the resolution
		//
		if ((vduConfig()->display0.width > MAX_SCREEN_WIDTH) ||
			(vduConfig()->display0.height > MAX_SCREEN_HEIGHT) ||
			(vduConfig()->display0.width != RVDU_SCREEN_WIDTH) ||
			(vduConfig()->display0.height != RVDU_SCREEN_HEIGHT))
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "cacheConfigFile: configured screen resolution (%u x %u) is not supported, allowed size is (%u x %u).\n",
				vduConfig()->display0.width,
				vduConfig()->display0.height,
				RVDU_SCREEN_WIDTH,
				RVDU_SCREEN_HEIGHT);
			alt_printf(msg);

			setStartupStage(STAGE_SCREEN_ERR);
			return 0;
		}

		vduConfigPrintSchemasList();
	}

	// Initialize video rendering modules
	//
	{
		moduleRnVbufInitialize((alt_u32*)EMIF_0_ECC_CORE_BASE, (alt_u32*)TO_EXTERNAL_BUS_BRIDGE_0_BASE,
			vduConfig()->display0.width, vduConfig()->display0.height);

		moduleRnPrimInitialize((alt_u32*)EMIF_0_ECC_CORE_BASE,
			vduConfig()->display0.width, vduConfig()->display0.height);

		moduleRnTrendProcInitialize();

		moduleRnTrendViewInitialize(vduConfig()->display0.width, vduConfig()->display0.height);
	}

	// Show the logo
	//
	{
		setStartupStage(STAGE_SHOWLOGO);

		char logoFile[STRING_MAX_LEN] = { 0 };
		(void)snprintf(logoFile, STRING_MAX_LEN, "%s/StartupLogo.bmp", workFolder);

		bool readOk = showLogo(logoFile);
		if (readOk == true)
		{
			(void)pushVideoBuffer();
		}

	}

	// Load globalScript file
	//
	{
		setStartupStage(STAGE_GLOBALSCRIPT_INIT);

		char luaFile[STRING_MAX_LEN] = { 0 };
		(void)snprintf(luaFile, STRING_MAX_LEN, "%s/GlobalScript.lua", workFolder);

		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);

		alt_u32 bytesRead = 0;
		bool result = cacheGlobalScript(luaFile, (alt_u8* const)startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_GLOBALSCRIPT_ERR);
			alt_printf("cacheGlobalScript failed.");
			return 0;
		}

		// Update the memory state constants
		//
		MEM_IF_DDR3_EMIF_0_End = startAddr + bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("cacheGlobalScript", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);
	}

	// Initialize LUA
	//
	setStartupStage(STAGE_LUA_INIT);
	lua_State* rvduLuaState = moduleRnLapiInitialize(vduConfig()->display0.width, vduConfig()->display0.height);
	if (rvduLuaState == NULL)
	{	
		setStartupStage(STAGE_LUA_INIT_ERR);
		alt_printf("scriptApiRegistration failed.");
		return 0;
	}

	// Compile GlobalScript
	//
	if (vduConfigGlobalScript() != NULL)
	{
		setStartupStage(STAGE_GLOBALSCRIPT_COMPILE);

		bool result = compileGlobalScript(rvduLuaState, vduConfigGlobalScript());
		if (result == false)
		{
			setStartupStage(STAGE_GLOBALSCRIPT_COMPILE_ERR);
			alt_printf("compileGlobalScript failed.");
			return 0;
		}
	}

	// Load signals file
	//
	{
		setStartupStage(STAGE_SIGNALS_INIT);

		char signalsFile[STRING_MAX_LEN] = { 0 };
		(void)snprintf(signalsFile, STRING_MAX_LEN, "%s/VduAppSignals.bin", workFolder);
		
		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);
		
		alt_u32 bytesRead = 0u;
		bool result = cacheSignalsFile(signalsFile, (alt_u8* const)startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_SIGNALS_ERR);
			alt_printf("cacheSignalsFile failed.");
			return 0;
		}

		MEM_IF_DDR3_EMIF_0_End = startAddr + bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("cacheSignalsFile", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);
	}
	
	// Write Comparison ID
	//
	{
		const alt_u8* id = vduConfigID();
		if (id == NULL)
		{
			RN_ASSERT(id);
			return 0;
		}
		writeComparsionID(id);
	}

	// Load font files
	//
	{
		setStartupStage(STAGE_FONTS_INIT);

		char fontsPath[STRING_MAX_LEN] = { 0 };
		(void)snprintf(fontsPath, STRING_MAX_LEN, "%s/Fonts", workFolder);

		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);

		alt_u32 bytesRead = 0u;
		bool result = cacheFonts(fontsPath, (alt_u8* const)startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_FONTS_ERR);
			alt_printf("cacheFonts failed.");
			return 0;
		}

		MEM_IF_DDR3_EMIF_0_End = startAddr + bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("cacheFonts", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);
	}

	// Load schema files
	//
	{
		setStartupStage(STAGE_SCHEMAS_INIT);

		char schemasPath[STRING_MAX_LEN];
		(void)snprintf(schemasPath, STRING_MAX_LEN, "%s/Schemas", workFolder);

		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);

		alt_u32 bytesRead = 0u;
		bool result = cacheSchemas(schemasPath, (alt_u8* const)startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_SCHEMAS_ERR);
			alt_printf("cacheSchemas failed.");
			return 0;
		}
		MEM_IF_DDR3_EMIF_0_End = startAddr + bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("cacheSchemas", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);
	}

	// Allocate memory for trends
	//
	{
		alt_u32 startAddr = alignAddressToDword(MEM_IF_DDR3_EMIF_0_End);

		alt_u32 bytesUsed = 0;
		bool result = allocateTrendsMemory((alt_u8* const)startAddr, memoryAvailable, &bytesUsed);
		if (result == false)
		{
			logError(InternalError, "allocateTrendsMemory: error allocating memory for trends data.");

			setStartupStage(STAGE_MEM_ERR);
			return 0;
		}

		MEM_IF_DDR3_EMIF_0_End = startAddr + bytesUsed;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesUsed);

		logMemoryUsage("allocateTrendsMemory", (const alt_u8*)EMIF_0_ECC_CORE_BASE, (const alt_u8*)MEM_IF_DDR3_EMIF_0_End, (alt_u32)EMIF_0_ECC_CORE_SPAN);
	}

	// Set default schema file
	//
	{
		setStartupStage(STAGE_SET_DEFAULT_SCHEMA);

		bool defaultSchemaFound = false;
		const char* const startSchemaId = vduConfigString(vduConfig()->display0.startSchemaId);
		if (startSchemaId != NULL)
		{
			defaultSchemaFound = vduSetCurrentSchema(startSchemaId);
		}

		if (defaultSchemaFound == false)
		{
			char msg[STRING_MAX_LEN];

			const char* schemaIdText = startSchemaId != NULL ? startSchemaId : "<null>";

			(void)snprintf(msg, STRING_MAX_LEN,
					"Specified default schema %s does not exist.",
					schemaIdText);
			logWarning(SchemaWasNotFound, msg);
			
			setStartupStage(STAGE_SET_DEFAULT_SCHEMA_ERR);
			return 0;
		}
	}

	setStartupStage(STAGE_RUNNING);

	// Main loop
	//
	while (true)
	{
		invertRs485SignalPin();			// Invert the output PIN that allows to control schema processing time (RS485-pin is used as free pin)

		resetBaseMeasuringTimer();

		startMeasuringTimer(TIMER_MAINLOOP);

		{
			externalBusBridgeReadLock();

			trendsProc();

			externalBusBridgeReadUnlock();
		}

		outputSignalsPreproc();

		bool schemaWasRepainted = false;
		schemaProc(rvduLuaState, &schemaWasRepainted);

		optoTxProc();

		stopMeasuringTimer(TIMER_MAINLOOP);

		if (schemaWasRepainted == true)
		{
			setTimerValue(TIMER_MAINLOOP, measuringTimerValue(TIMER_MAINLOOP));
			setTimerValue(TIMER_PREDRAW, measuringTimerValue(TIMER_PREDRAW));
		}
		
		writeExternalDiagPacket();
	}

	return 0;
}

