#ifndef RNSW_LIB_H
#define RNSW_LIB_H

//#include "msvc_alt_types.h"

#include <stdbool.h>

#ifdef __cplusplus
	#define RNSW_INLINE inline        // C++
#else
	#define RNSW_INLINE static inline // C
#endif

enum RnLibLogRecordType
{
	RnLibError,
	RnLibWarning,
	RnLibMessage
};

typedef void (*RnLibLogRecordHandler)(int /*RnLibLogRecordType*/, const char*);

#ifdef __cplusplus
extern "C"
{
#endif
	// Run and exit
	//
	bool rnLibInit();
	void rnLibShutdown();

	bool rnLibStartSim(const char* buildFolder, unsigned int subsystemKey, char* pVideoBuffer, char* pHdmiBuffer, int screenWidth, int screenHeight);
	void rnLibStopSim();

	// HMI interaction
	//
	void rnLibClickEvent(unsigned int x, unsigned int y);
	void rnLibKeyPressEvent(unsigned int code);

	void rnLibVideoBufferLock();
	void rnLibVideoBufferUnlock();
	
	// Main loop
	//
	bool rnLibLoop();
	void rnLibLoopShutdown();	// Used to break the pop-up loop

	void rnLibSetLogRecordHandler(RnLibLogRecordHandler handler);

	// Signals processing
	//
	unsigned int rnLibAppSignalsCount(void);
	const struct VduAppSignalParam* const rnLibAppSignalParam(unsigned int index);

	const char* const rnLibAppSignalID(unsigned int index);
	const char* const rnLibAppSignalCustomID(unsigned int index);
	const char* const rnLibAppSignalCaption(unsigned int index);
	const char* const rnLibAppSignalUnit(unsigned int index);

	unsigned int rnLibTxAppSignalsCount(void);
	const struct VduTxRxAppSignal* const rnLibTxAppSignal(unsigned int index);

	bool rnLibSignalValueInt(unsigned int signalIndex, int* res);
	bool rnLibSignalValueFloat(unsigned int signalIndex, float* res);

	bool rnLibWriteSignalValueInt(unsigned int signalIndex, int value);
	bool rnLibWriteSignalValueFloat(unsigned int signalIndex, float value);

#ifdef __cplusplus
}
#endif

#endif // RNSW_LIB_H