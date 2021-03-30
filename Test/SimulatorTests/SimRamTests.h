#pragma once

#include <QObject>
#include <memory>
#include "../../Simulator/SimRam.h"

class SimRamTests : public QObject
{
	Q_OBJECT

public:
	SimRamTests();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void ramAreaCreateTest();
	void ramAreaWriteBitTest();
	void ramAreaWriteWordTest();
	void ramAreaWriteDwordTest();
	void ramAreaWriteSignedIntTest();

	void ramCreate();
	void ramIsNull();
	void ramReset();
	void ramUpdateFrom();
	void ramMemoryAreaHandle();

	void ramReadWriteBuffer();
	void ramMovMem();
	void ramSetMem();
	void ramReadWriteBit();
	void ramReadWriteWord();
	void ramReadWriteDword();

private:
    inline static const quint32 s_ra_offset = 12000;
    inline static const quint32 s_ra_size = 400;
	std::unique_ptr<Sim::RamArea> m_ramArea;
};


