#pragma once

#ifndef __FUNCTION_NAME__
	#ifdef WIN32 // WINDOWS
		#define __FUNCTION_NAME__ __FUNCTION__
	#else        //*NIX
		#define __FUNCTION_NAME__ __FUNCTION__
	#endif
#endif

// class DeviceEmulator has function DeviceEmulator::fault
// this is convenient call of this func
//
#define SIM_FAULT(message) fault(message, QLatin1String(__FUNCTION_NAME__));

namespace Sim
{
	enum class DeviceError
	{
		Ok,
		NoCommandProcessor,
		AfbComponentNotFound,
		InitEepromError,
		ParsingAppLogicError,
		ModuleExtraInfoNotFound,
		LanControllerError
	};
} // namespace Sim