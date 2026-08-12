#ifndef RNSW_LIB_H
#define RNSW_LIB_H

#include "msvc_alt_types.h"

#include <stdbool.h>

#ifdef __cplusplus
	#define RNSW_INLINE inline        // C++
#else
	#define RNSW_INLINE static inline // C
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	// Run and exit
	//
	bool rnLibInit();
	void rnLibShutdown();

	bool rnLibStartSim(const char* buildFolder, alt_u16 subsystemKey, alt_u32* pVideoBuffer, alt_u32* pHdmiBuffer, int screenWidth, int screenHeight);
	void rnLibStopSim();

	// HMI interaction
	//
	void rnLibClickEvent(alt_u16 x, alt_u16 y);
	void rnLibKeyPressEvent(alt_u8 code);

	void rnLibVideoBufferLock();
	void rnLibVideoBufferUnlock();
	
	// Main loop
	//
	bool rnLibLoop();
	void rnLibLoopShutdown();	// Used to break the pop-up loop

	// Signals processing
	//
	alt_u16 rnLibAppSignalsCount(void);
	const struct VduAppSignalParam* const rnLibAppSignalParam(alt_u16 index);

	const char* const rnLibAppSignalID(alt_u16 index);
	const char* const rnLibAppSignalCustomID(alt_u16 index);
	const char* const rnLibAppSignalCaption(alt_u16 index);
	const char* const rnLibAppSignalUnit(alt_u16 index);

	alt_u16 rnLibTxAppSignalsCount(void);
	const struct VduTxRxAppSignal* const rnLibTxAppSignal(alt_u16 index);

	bool rnLibSignalValueInt(alt_u16 signalIndex, alt_32* res);
	bool rnLibSignalValueFloat(alt_u16 signalIndex, float* res);

	bool rnLibWriteSignalValueInt(alt_u16 signalIndex, alt_32 value);
	bool rnLibWriteSignalValueFloat(alt_u16 signalIndex, float value);

#ifdef __cplusplus
}
#endif

#endif // RNSW_LIB_H