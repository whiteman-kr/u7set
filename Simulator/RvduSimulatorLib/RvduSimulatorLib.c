#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER 
#include < crtdbg.h>
#endif

#include <assert.h>

#include "RvduSimulatorLib.h"

#include "defs.h"
#include "rn_cfg.h"
#include "rn_desc.h"
#include "rn_diag.h"
#include "rn_fofp.h"
#include "rn_kbd.h"
#include "rn_lapi.h"
#include "rn_logview.h"
#include "rn_opto.h"
#include "rn_prim.h"
#include "rn_scfp.h"
#include "rn_schema.h"
#include "rn_sifp.h"
#include "rn_signalsview.h"
#include "rn_trendproc.h"
#include "rn_trendview.h"
#include "rn_ts.h"
#include "rn_utils.h"
#include "rn_vbuf.h"

#include "rn_kbd_sim.h"
#include "rn_ts_sim.h"
#include "rn_utils_sim.h"
#include "rn_vbuf_sim.h"

#include <threads.h>

// Memory pointers
//
static alt_u8* m_memory = NULL;                 // RVDU Memory area 32 MBytes
static const int m_memorySize = 1048576 * 32;

static alt_u8* m_externalBusBridgeRead = NULL;  // Input memory area
static alt_u8* m_externalBusBridgeWrite = NULL; // Output memory area

#define TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE 65536
alt_u16* TO_EXTERNAL_BUS_BRIDGE_1_BASE_READ = NULL;
alt_u16* TO_EXTERNAL_BUS_BRIDGE_1_BASE_WRITE = NULL;

// Configuaraion parameters
//
static const char* m_buildFolder = NULL;
static alt_u16 m_subsystemKey = 0;

static lua_State* m_rvduLuaState = NULL; // LUA state object

static alt_u16 readWriteEBB16(alt_u16 wOffset)
{
	volatile alt_u16* p16 = (alt_u16*)TO_EXTERNAL_BUS_BRIDGE_1_BASE_WRITE + wOffset;
	return *p16;
}

static void writeReadEBB16(alt_u16 wOffset, alt_u16 data)
{
	volatile alt_u16* p16 = (alt_u16*)TO_EXTERNAL_BUS_BRIDGE_1_BASE_READ + wOffset;
	*p16 = data;
	return;
}

bool rnLibInit()
{
#ifdef _MSC_VER
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif

	m_memory = (alt_u8*)malloc(m_memorySize);
	if (m_memory == NULL)
	{
		return false;
	}
	memset(m_memory, 0, m_memorySize);

	// External bus read buffer
	//
	m_externalBusBridgeRead = (alt_u8*)malloc(TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE);
	if (m_externalBusBridgeRead == NULL)
	{
		return false;
	}
	memset(m_externalBusBridgeRead, 0, TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE);

	TO_EXTERNAL_BUS_BRIDGE_1_BASE_READ = (alt_u16*)m_externalBusBridgeRead;

	// External bus write buffer
	//
	m_externalBusBridgeWrite = (alt_u8*)malloc(TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE);
	if (m_externalBusBridgeWrite == NULL)
	{
		return false;
	}
	memset(m_externalBusBridgeWrite, 0, TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE);

	TO_EXTERNAL_BUS_BRIDGE_1_BASE_WRITE = (alt_u16*)m_externalBusBridgeWrite;

	// Simulate PIN code
	{
		alt_u32 pin123456 = calcHash32((const alt_u8*)("123456"), (alt_u8)strlen("123456"));

		writeReadEBB16(ADDR_PIN_HI, (alt_u16)(pin123456 >> 16));
		writeReadEBB16(ADDR_PIN_LO, (alt_u16)(pin123456 & 0xffff));
	}

	return true;
}

void rnLibShutdown()
{
	if (m_memory != NULL)
	{
		free(m_memory);
		m_memory = NULL;
	}

	if (m_externalBusBridgeRead != NULL)
	{
		free(m_externalBusBridgeRead);
		m_externalBusBridgeRead = NULL;
	}

	if (m_externalBusBridgeWrite != NULL)
	{
		free(m_externalBusBridgeWrite);
		m_externalBusBridgeWrite = NULL;
	}
}

bool rnLibStartSim(const char* buildFolder,
				   unsigned int subsystemKey,
				   char* pVideoBuffer,
				   char* pHdmiBuffer,
				   int screenWidth,
				   int screenHeight)
{
	m_buildFolder = buildFolder;
	m_subsystemKey = (alt_u16)subsystemKey;

	// Memory initialization
	//
	if (m_memory == NULL || m_externalBusBridgeRead == NULL || m_externalBusBridgeWrite == NULL)
	{
		return false;
	}
	alt_u8* memoryEnd = m_memory;
	alt_u32 memoryAvailable = m_memorySize;

	setStartupStage(STAGE_MEM_INIT);

	// Initialize memory for log
	//
	{
		setStartupStage(STAGE_LOG_INIT);

		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesRead = 0;
		if (moduleRnDiagInitialize(startAddr, 1000, memoryAvailable, &bytesRead) == false)
		{
			setStartupStage(STAGE_MEM_ERR);
			return false;
		}

		memoryEnd += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);

		logMemoryUsage("moduleRnDiagInitialize", m_memory, memoryEnd, m_memorySize);
	}

	moduleRnUtilsInitialize();

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

	// Initialize working paths
	//
	char workFolder[STRING_MAX_LEN] = {0};
	(void)strcpy(workFolder, m_buildFolder);

	// Initialize Equipment Id
	//
	{
		setStartupStage(STAGE_EQUIPMENT_INIT);

		char descFile[STRING_MAX_LEN] = {0};
		(void)strcpy(descFile, m_buildFolder);
		(void)strcat(descFile, "\\Description.json");

		char equipmentId[STRING_MAX_LEN] = {0};
		bool result = findEquipmentId(m_subsystemKey, descFile, equipmentId, STRING_MAX_LEN);
		if (result == true)
		{
			(void)strcat(workFolder, "\\");
			(void)strcat(workFolder, equipmentId);
		}
		else
		{
			setStartupStage(STAGE_EQUIPMENT_ERR);
			return false;
		}
	}

	// Load configuration file
	//
	{
		setStartupStage(STAGE_CFG_READ);

		char cfgFile[STRING_MAX_LEN] = {0};
		(void)strcpy(cfgFile, workFolder);
		(void)strcat(cfgFile, "/VduConfig.bin");

		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesRead = 0;
		bool result = cacheConfigFile(cfgFile, startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_CFG_ERR);
			return false;
		}

		// Check the resolution
		//
		if (vduConfig()->display0.width > MAX_SCREEN_WIDTH || vduConfig()->display0.height > MAX_SCREEN_HEIGHT ||
			vduConfig()->display0.width != screenWidth || vduConfig()->display0.height != screenHeight)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg,
						   STRING_MAX_LEN,
						   "cacheConfigFile: configured screen resolution (%d x %d) is not supported, allowed size is (%d x %d).",
						   vduConfig()->display0.width,
						   vduConfig()->display0.height,
						   screenWidth,
						   screenHeight);
			logError(InternalError, msg);

			setStartupStage(STAGE_SCREEN_ERR);
			return false;
		}

		// Update the memory state constants
		//
		memoryEnd += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);
		logMemoryUsage("cacheConfigFile", m_memory, memoryEnd, m_memorySize);

		vduConfigPrintSchemasList();
	}

	// Initialize video memory
	//
	moduleRnVbufInitialize((alt_u32*)pVideoBuffer, (alt_u32*)pHdmiBuffer, vduConfig()->display0.width, vduConfig()->display0.height);
	moduleRnPrimInitialize((alt_u32*)pVideoBuffer, vduConfig()->display0.width, vduConfig()->display0.height);

	// Show the logo
	//
	{
		setStartupStage(STAGE_SHOWLOGO);

		char logoFile[STRING_MAX_LEN] = {0};
		(void)strcpy(logoFile, workFolder);
		(void)strcat(logoFile, "\\StartupLogo.bmp");

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

		char luaFile[STRING_MAX_LEN] = {0};
		(void)strcpy(luaFile, workFolder);
		(void)strcat(luaFile, "/GlobalScript.lua");

		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesRead = 0;
		bool result = cacheGlobalScript(luaFile, startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_GLOBALSCRIPT_ERR);
			return false;
		}

		// Update the memory state constants
		//
		memoryEnd += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);
		logMemoryUsage("cacheGlobalScript", m_memory, memoryEnd, m_memorySize);
	}

	// Initialize LUA
	//
	{
		setStartupStage(STAGE_LUA_INIT);
		m_rvduLuaState = moduleRnLapiInitialize(vduConfig()->display0.width, vduConfig()->display0.height);
		if (m_rvduLuaState == NULL)
		{
			setStartupStage(STAGE_LUA_INIT_ERR);
			return false;
		}
	}

	if (vduConfigGlobalScript() != NULL)
	{
		setStartupStage(STAGE_GLOBALSCRIPT_COMPILE);

		bool result = compileGlobalScript(m_rvduLuaState, vduConfigGlobalScript());
		if (result == false)
		{
			setStartupStage(STAGE_GLOBALSCRIPT_COMPILE_ERR);
			return false;
		}
	}

	// Load signals file
	//
	{
		setStartupStage(STAGE_SIGNALS_INIT);

		char signalsFile[STRING_MAX_LEN] = {0};
		(void)strcpy(signalsFile, workFolder);
		strcat(signalsFile, "/VduAppSignals.bin");

		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesRead = 0;
		bool result = cacheSignalsFile(signalsFile, startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_SIGNALS_ERR);
			return false;
		}

		memoryEnd += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);
		logMemoryUsage("cacheSignalsFile", m_memory, memoryEnd, m_memorySize);
	}

	// Load font files
	//
	{
		setStartupStage(STAGE_FONTS_INIT);

		char fontsPath[STRING_MAX_LEN] = {0};
		(void)strcpy(fontsPath, workFolder);
		(void)strcat(fontsPath, "/Fonts");

		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesRead = 0;
		bool result = cacheFonts(fontsPath, startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_FONTS_ERR);
			return false;
		}

		memoryEnd += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);
		logMemoryUsage("cacheFonts", m_memory, memoryEnd, m_memorySize);
	}

	// Load schema files
	//
	{
		setStartupStage(STAGE_SCHEMAS_INIT);

		char schemasPath[STRING_MAX_LEN];
		(void)strcpy(schemasPath, workFolder);
		(void)strcat(schemasPath, "/Schemas");

		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesRead = 0;
		bool result = cacheSchemas(schemasPath, startAddr, memoryAvailable, &bytesRead);
		if (result == false)
		{
			setStartupStage(STAGE_SCHEMAS_ERR);
			return false;
		}

		memoryEnd += bytesRead;
		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesRead);
		logMemoryUsage("cacheSchemas", m_memory, memoryEnd, m_memorySize);
	}

	// Allocate memory for trends
	//
	{
		alt_u8* const startAddr = memoryEnd;

		alt_u32 bytesUsed = 0;
		bool result = allocateTrendsMemory(startAddr, memoryAvailable, &bytesUsed);
		if (result == false)
		{
			logError(InternalError, "allocateTrendsMemory: error allocating memory for trends data.");

			setStartupStage(STAGE_MEM_ERR);
			return false;
		}

		memoryEnd += bytesUsed;

		memoryAvailable = DEC_MEMORY(memoryAvailable, bytesUsed);
		logMemoryUsage("allocateTrendsMemory", m_memory, memoryEnd, m_memorySize);
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
			(void)snprintf(msg,
						   STRING_MAX_LEN,
						   "Specified default schema %s does not exist.",
						   vduConfigString(vduConfig()->display0.startSchemaId));
			logWarning(SchemaWasNotFound, msg);

			setStartupStage(STAGE_SET_DEFAULT_SCHEMA_ERR);
			return false;
		}
	}

	// Initialize trends
	//

	moduleRnTrendProcInitialize();
	moduleRnTrendViewInitialize(vduConfig()->display0.width, vduConfig()->display0.height);

	setStartupStage(STAGE_RUNNING);

	return true;
}

void rnLibStopSim()
{
	moduleRnTsUninitialize();
	moduleRnKbdUninitialize();
	moduleRnVbufUninitialize();
	moduleRnUtilsUninitialize();
}

void rnLibClickEvent(unsigned int x, unsigned int y)
{ 
	setTouchEvent((alt_u16)x, (alt_u16)y);
}

void rnLibKeyPressEvent(unsigned int code)
{ 
	setPressedKey((alt_u16)code); 
}

void rnLibVideoBufferLock()
{
	moduleRnVbufLock();
}

void rnLibVideoBufferUnlock()
{
	moduleRnVbufUnlock();
}

static void outputSignalsLoopback()
{
	externalBusBridgeWriteLock();
	
	alt_u16 outputControlState = readWriteEBB16(ADDR_DISCRETE_OUTPUTS_WRITE);		// Read control from the Write memory
	
	externalBusBridgeWriteUnlock();

	externalBusBridgeReadLock();

	alt_u16 outputsInputState = readEBB16(ADDR_DISCRETE_OUTPUTS_READ);				// Read input from the Read memory

	outputsInputState &= 0x03;	// Keep bits 0 and 1 of outputs we do not control
	outputsInputState |= (outputControlState << ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA); // Add the control bits to bits 5..2

	writeReadEBB16(ADDR_DISCRETE_OUTPUTS_READ, outputsInputState);	// Write control to a READ memory

	externalBusBridgeReadUnlock();
}

bool rnLibLoop()
{
	if (m_memory == NULL || m_externalBusBridgeRead == NULL || m_externalBusBridgeWrite == NULL)
	{
		return false;
	}

	{
		// Get current time with nanosecond precision
		struct timespec ts;
		(void)timespec_get(&ts, TIME_UTC);

		struct tm* t = localtime(&ts.tv_sec);

		writeReadEBB16(ADDR_RTU_YEAR, (alt_u16)t->tm_year + 1900u);
		writeReadEBB16(ADDR_RTU_MONTH, (alt_u16)t->tm_mon + 1u);
		writeReadEBB16(ADDR_RTU_DAY, (alt_u16)t->tm_mday);

		writeReadEBB16(ADDR_RTU_HOUR, (alt_u16)t->tm_hour);
		writeReadEBB16(ADDR_RTU_MINUTE, (alt_u16)t->tm_min);
		writeReadEBB16(ADDR_RTU_SECOND, (alt_u16)t->tm_sec);
		writeReadEBB16(ADDR_RTU_MILLISECOND, (alt_u16)(ts.tv_nsec / 1000000u));
	}

	{
		resetBaseMeasuringTimer();

		startMeasuringTimer(TIMER_MAINLOOP);

		{
			externalBusBridgeReadLock();

			trendsProc();

			externalBusBridgeReadUnlock();
		}

		outputSignalsPreproc();

		bool schemaWasRepainted = false;
		schemaProc(m_rvduLuaState, &schemaWasRepainted);

		outputSignalsLoopback();

		optoTxProc();

		stopMeasuringTimer(TIMER_MAINLOOP);

		if (schemaWasRepainted == true)
		{
			setTimerValue(TIMER_MAINLOOP, measuringTimerValue(TIMER_MAINLOOP));
			setTimerValue(TIMER_PREDRAW, measuringTimerValue(TIMER_PREDRAW));
		}

		writeExternalDiagPacket();
	}

	return true;
}

void rnLibLoopShutdown() // Used to break the pop-up loop
{ 
	popUpSchemaSetTerminate(); 
}

void rnLibSetLogRecordHandler(RnLibLogRecordHandler handler) 
{ 
	setLogRecordHandler(handler); 
}

unsigned int rnLibAppSignalsCount(void)
{ 
	return appSignalsCount(); 
}

const struct VduAppSignalParam* const rnLibAppSignalParam(unsigned int index)
{ 
	return appSignalParamByIndex((alt_u16)index); 
}

const char* const rnLibAppSignalID(unsigned int index)
{ 
	return appSignalID((alt_u16)index); 
}

const char* const rnLibAppSignalCustomID(unsigned int index)
{
	return customAppSignalID((alt_u16)index); 
}

const char* const rnLibAppSignalCaption(unsigned int index)
{ 
	return signalCaption((alt_u16)index); 
}

const char* const rnLibAppSignalUnit(unsigned int index)
{ 
	return signalUnit((alt_u16)index); 
}

bool rnLibIsDiscrete(unsigned int index)
{ 
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL) 
	{
		assert(param);
		return false;
	}
	return param->vduSignalType == Discrete; 
}

bool rnLibIsFloat(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return false;
	}
	return param->vduSignalType == AnalogFloat32;
}

bool rnLibIsInt32(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return false;
	}
	return param->vduSignalType == AnalogSignedInt32;
}

bool rnLibIsInternal(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return false;
	}
	return param->vduSignalInOutType == Internal;
}

bool rnLibIsInput(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return false;
	}
	return param->vduSignalInOutType == Input;
}

bool rnLibIsOutput(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return false;
	}
	return param->vduSignalInOutType == Output;
}

bool rnLibIsRxSignal(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return false;
	}
	return param->vduSignalInOutType == RxSignal;
}

bool rnLibIsTxSignal(unsigned int index)
{
	for (alt_u16 j = 0; j < txAppSignalsCount(); j++)
	{
		const struct VduTxRxAppSignal* const txRxSignal = txRxAppSignal(j);
		if (txRxSignal == NULL)
		{
			assert(txRxSignal);
			return false;
		}

		if (txRxSignal->signalIndex == index)
		{
			return true;
		}
	}
	return false;
}

int rnLibRxPortIndex(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->rxPortIndex;
}

int rnLibSignalOffset(unsigned int index) 
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->optoAddr.offset;
}

int rnLibSignalBit(unsigned int index) 
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->optoAddr.bitNo;
}

bool rnLibSignalTunable(unsigned int index) 
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->props.bits.tunable;
}

float rnLibSignalHighLimitFloat(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->highLimit.floatValue;
}

float rnLibSignalLowLimitFloat(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->lowLimit.floatValue;
}

int rnLibSignalHighLimitInt(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->highLimit.intValue;
}

int rnLibSignalLowLimitInt(unsigned int index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)index);
	if (param == NULL)
	{
		assert(param);
		return -1;
	}
	return param->lowLimit.intValue;
}

bool rnLibSignalValueInt(unsigned int signalIndex, alt_32* res)
{
	*res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)signalIndex);
	if (param == NULL)
	{
		RN_ASSERT(param);
		return false;
	}

	const struct VduAppSignalState* const state = appSignalStateByIndex((alt_u16)signalIndex);
	if (param == NULL)
	{
		RN_ASSERT(state);
		return false;
	}

	bool result = false;

	externalBusBridgeReadLock();

	if (param->vduSignalInOutType == Internal)
	{
		*res = state->value.intValue;

		if (param->vduSignalType == AnalogSignedInt32 || param->vduSignalType == Discrete)
		{
			result = true;
		}
	}
	else if (param->vduSignalInOutType == RxSignal)
	{
		if (param->vduSignalType == Discrete)
		{
			alt_u16 wData = readEBB16(param->optoAddr.offset);

			if ((wData & (1 << param->optoAddr.bitNo)) != 0)
			{
				*res = 1;
			}
			else
			{
				*res = 0;
			}
			result = true;
		}
		else if (param->vduSignalType == AnalogSignedInt32)
		{
			// 32-bit value
			//
			*res = ((alt_32)readEBB16(param->optoAddr.offset) << 16) | readEBB16(param->optoAddr.offset + 1);
			result = true;
		}
	}
	else if (param->vduSignalInOutType == Input)
	{
		if (param->vduSignalType == Discrete)
		{
			alt_u16 wData = readEBB16(param->optoAddr.offset);

			if ((wData & (1 << param->optoAddr.bitNo)) != 0)
			{
				*res = 1;
			}
			else
			{
				*res = 0;
			}
			result = true;
		}
		else if (param->vduSignalType == AnalogSignedInt32)
		{
			// 16-bit value
			//
			*res = readEBB16(param->optoAddr.offset);
			result = true;
		}
	}
	else if (param->vduSignalInOutType == Output)
	{
		if (param->vduSignalType == Discrete)
		{
			// For Output Discretes, we read a WRITE memory
			//
			externalBusBridgeWriteLock();

			alt_u16 outputControlState = readWriteEBB16(ADDR_DISCRETE_OUTPUTS_WRITE);		// Read control from the Write memory

			externalBusBridgeWriteUnlock();

			if ((outputControlState & (1 << param->optoAddr.bitNo)) != 0)
			{
				*res = 1;
			}
			else
			{
				*res = 0;
			}

			result = true;
		}
	}

	externalBusBridgeReadUnlock();

	RN_ASSERT(result);
	return result;
}

bool rnLibSignalValueFloat(unsigned int signalIndex, float* res)
{
	*res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)signalIndex);
	if (param == NULL)
	{
		RN_ASSERT(param);
		return false;
	}

	const struct VduAppSignalState* const state = appSignalStateByIndex((alt_u16)signalIndex);
	if (param == NULL)
	{
		RN_ASSERT(state);
		return false;
	}

	bool result = false;

	externalBusBridgeReadLock();

	if (param->vduSignalType == AnalogFloat32)
	{
		if (param->vduSignalInOutType == Internal)
		{
			*res = state->value.floatValue;
			result = true;
		}
		else
		{
			alt_u32 iRes = ((alt_u32)readEBB16(param->optoAddr.offset) << 16) | readEBB16(param->optoAddr.offset + 1);
			(void)memcpy(res, &iRes, sizeof(*res)); // alt_u32-to-float bit reinterpretation

			result = true;
		}
	}

	externalBusBridgeReadUnlock();

	RN_ASSERT(result);
	return result;
}

bool rnLibWriteSignalValueInt(unsigned int signalIndex, int value)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)signalIndex);
	if (param == NULL)
	{
		RN_ASSERT(param);
		return false;
	}

	struct VduAppSignalState* state = appSignalStateMutableByIndex((alt_u16)signalIndex);
	if (state == NULL)
	{
		RN_ASSERT(state);
		return false;
	}

	bool result = false;

	externalBusBridgeReadLock();

	if (param->vduSignalInOutType == Internal)
	{
		state->value.intValue = value;
		result = true;
	}
	else if (param->vduSignalInOutType == RxSignal)
	{
		if (param->vduSignalType == Discrete)
		{
			alt_u16 discretesState = readEBB16(param->optoAddr.offset);

			if (value != 0)
			{
				writeReadEBB16(param->optoAddr.offset, discretesState | (1 << param->optoAddr.bitNo));
			}
			else
			{
				writeReadEBB16(param->optoAddr.offset, discretesState & (~(1 << param->optoAddr.bitNo)));
			}
			result = true;
		}
		else if (param->vduSignalType == AnalogSignedInt32)
		{
			// write as 32-bit value
			//
			writeReadEBB16(param->optoAddr.offset, (alt_u16)((value & 0xffff0000) >> 16));
			writeReadEBB16(param->optoAddr.offset + 1, (alt_u16)(value & 0x0000ffff));
			result = true;
		}
	}
	else if (param->vduSignalInOutType == Input)
	{
		if (param->vduSignalType == Discrete)
		{
			alt_u16 discretesState = readEBB16(param->optoAddr.offset);

			alt_u16 data = 0;
			if (value != 0)
			{
				data = discretesState | (1 << param->optoAddr.bitNo);
			}
			else
			{
				data = discretesState & (~(1 << param->optoAddr.bitNo));
			}

			writeReadEBB16(param->optoAddr.offset, data);

			{
				// For Discrete Output signals, we write them to a READ memory 20483 (ADDR_DISCRETE_OUTPUTS_READ),
				// and also we to Write memory by address 20480 (ADDR_DISCRETE_OUTPUTS_WRITE)
				//
				if (param->optoAddr.offset == ADDR_DISCRETE_OUTPUTS_READ)
				{
					data >>= 2;
					
					externalBusBridgeWriteLock();
					
					writeEBB16(ADDR_DISCRETE_OUTPUTS_WRITE, data);
					
					externalBusBridgeWriteUnlock();
				}
			}

			result = true;
		}
		else if (param->vduSignalType == AnalogSignedInt32)
		{
			// write as 16-bit value
			//
			writeReadEBB16(param->optoAddr.offset, (alt_u16)(value & 0x0000ffff));
			result = true;
		}
	}
	else if (param->vduSignalInOutType == Output)
	{
		if (param->vduSignalType == Discrete)
		{
			// For Output Discretes, we write a WRITE memory
			//
			alt_u16 outputsState = readEBB16(ADDR_DISCRETE_OUTPUTS_READ);

			// in REAL RVDU State of 6 output discretes are stored in corresponding Input signals.
			// We can control only 4 output discretes with numbers [3..6].
			if (value != 0)
			{
				outputsState = outputsState | (1 << (param->optoAddr.bitNo + ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA));
			}
			else
			{
				outputsState = outputsState & (~(1 << (param->optoAddr.bitNo + ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA)));
			}
			writeEBB16(param->optoAddr.offset, outputsState >> ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA);

			// Also, we write them to a READ memory
			{
				writeReadEBB16(ADDR_DISCRETE_OUTPUTS_READ, outputsState);
			}

			result = true;
		}
	}

	externalBusBridgeReadUnlock();

	RN_ASSERT(result);
	return result;
}

bool rnLibWriteSignalValueFloat(unsigned int signalIndex, float value)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex((alt_u16)signalIndex);
	if (param == NULL)
	{
		RN_ASSERT(param);
		return false;
	}

	struct VduAppSignalState* state = appSignalStateMutableByIndex((alt_u16)signalIndex);
	if (state == NULL)
	{
		RN_ASSERT(state);
		return false;
	}

	bool result = false;

	externalBusBridgeReadLock();

	if (param->vduSignalType == AnalogFloat32)
	{
		if ((param->vduSignalInOutType == Internal))
		{
			state->value.floatValue = value;
			result = true;
		}
		else
		{
			alt_u32 iRes = 0;
			(void)memcpy(&iRes, &value, sizeof(float)); // flot-to-alt_u32 bit reinterpretation

			writeReadEBB16(param->optoAddr.offset, (alt_u16)((iRes & 0xffff0000) >> 16));
			writeReadEBB16(param->optoAddr.offset + 1, (alt_u16)(iRes & 0x0000ffff));

			result = true;
		}
	}

	externalBusBridgeReadUnlock();

	RN_ASSERT(result);
	return result;
}

void alt_printf(const char* format, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    printf("%s", buffer);

#ifdef _MSC_VER
    _RPT1(_CRT_WARN, "%s", buffer);
#endif
}
