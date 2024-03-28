#pragma once

#include <QObject>
#include <Simulator/SimRam.h>

class SimRamTests : public QObject
{
	Q_OBJECT

public:
	SimRamTests() = default;

private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void ramAreaDump();
	void ramDump();

	void ramAreaCreateTest();
	void ramAreaClearTest();
	void ramGetMemoryAreasTest();
	void ramGetMemoryAreaHandle();

	void ramGetOverrideData();

	void ramAreaWriteBitTest();
	void ramAreaWriteWordTest();
	void ramAreaWriteDwordTest();
	void ramAreaWriteSignedIntTest();

	void ramCreate();
	void ramIsNull();
	void ramReset();
	void ramUpdateFrom();
	void ramMemoryAreaHandle();

	void ramClearMemoryAreasOnStartCycle();
	void ramClearMemoryAreas();

	void ramReadWriteBuffer();
	void ramReadWriteBufferOverride();
	void ramMovMem();
	void ramSetMem();
	void ramSetMemWithOverride();
	void ramReadWriteBit();
	void ramReadWriteWord();
	void ramReadWriteDword();
	void ramReadWriteFloat();
	void ramReadWriteSignedInt32();

	void ramOverrideSignalLastCounter();

private:
	inline static const quint32 s_ra_offset = 12000;	// word
	inline static const quint32 s_ra_size = 400;		// words
	std::unique_ptr<Sim::RamArea> m_ramArea;
};
