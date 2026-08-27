#ifndef RNSW_LIB_H
#define RNSW_LIB_H

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

#define TO_EXTERNAL_BUS_BRIDGE_1_BASE_SIZE 65536

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

	int rnLibWidth();
	int rnLibHeight();

	const char* rnLibEqipmentId();

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

	// Signals properties
	
	unsigned int rnLibAppSignalsCount(void);
	const struct VduAppSignalParam* const rnLibAppSignalParam(unsigned int index);

	const char* const rnLibAppSignalID(unsigned int index);
	const char* const rnLibAppSignalCustomID(unsigned int index);
	const char* const rnLibAppSignalCaption(unsigned int index);
	const char* const rnLibAppSignalUnit(unsigned int index);

	bool rnLibIsDiscrete(unsigned int index);
	bool rnLibIsFloat(unsigned int index);
	bool rnLibIsInt32(unsigned int index);

	bool rnLibIsInternal(unsigned int index);
	bool rnLibIsInput(unsigned int index);
	bool rnLibIsOutput(unsigned int index);
	bool rnLibIsRxSignal(unsigned int index);
	bool rnLibIsTxSignal(unsigned int index);

	int rnLibRxPortIndex(unsigned int index);

	int rnLibSignalOffset(unsigned int index);
	int rnLibSignalBit(unsigned int index);
	bool rnLibSignalTunable(unsigned int index);

	float rnLibSignalHighLimitFloat(unsigned int index);
	float rnLibSignalLowLimitFloat(unsigned int index);

	int rnLibSignalHighLimitInt(unsigned int index);
	int rnLibSignalLowLimitInt(unsigned int index);

	// Signals read/write

	void rnLibReadMemoryLock();
	void rnLibReadMemoryUnlock();
	bool rnLibReadReadMemory(unsigned int offset, unsigned int count, unsigned char* const result);
	bool rnLibWriteReadMemory(unsigned int offset, const unsigned char* const data, unsigned int count);

	void rnLibWriteMemoryLock();
	void rnLibWriteMemoryUnlock();
	bool rnLibReadWriteMemory(unsigned int offset, unsigned int count, unsigned char* const result);

	bool rnLibSignalValueInt(unsigned int signalIndex, int* res);
	bool rnLibSignalValueFloat(unsigned int signalIndex, float* res);

	bool rnLibWriteSignalValueInt(unsigned int signalIndex, int value);
	bool rnLibWriteSignalValueFloat(unsigned int signalIndex, float value);

#ifdef __cplusplus
}
#endif

#endif // RNSW_LIB_H