#include "SimRamTests.h"
#include <QtTest>
#include "SimOverrideSignals.h"
#include <algorithm>


SimRamTests::SimRamTests()
{
}


void SimRamTests::initTestCase()
{
}

void SimRamTests::cleanupTestCase()
{
}

void SimRamTests::init()
{
    m_ramArea = std::make_unique<Sim::RamArea>(E::LogicModuleRamAccess::Read, SimRamTests::s_ra_offset, SimRamTests::s_ra_size, true, "RA0");
	return;
}

void SimRamTests::cleanup()
{
}

void SimRamTests::ramAreaCreateTest()
{
	QVERIFY(m_ramArea->access() == E::LogicModuleRamAccess::Read);
	QVERIFY(m_ramArea->offset() == s_ra_offset);
	QVERIFY(m_ramArea->size() == s_ra_size);
	QVERIFY(m_ramArea->name() == "RA0");
	QVERIFY(m_ramArea->clearOnStartCycle() == true);
	return;
}

void SimRamTests::ramAreaWriteBitTest()
{
	const quint32 offsetInArea = 100;
	const quint32 offset = s_ra_offset + offsetInArea;
	bool ok = false;

	// Check write in LE
	//
	for (quint16 i = 0; i < 16; i += 3)
	{
		quint16 data = i % 3 ? 0 : 1;		// Write every third bit to 1

		ok = m_ramArea->writeBit(offset, i, data, E::ByteOrder::LittleEndian);
		QVERIFY(ok == true);

		quint16 readData;
		ok = m_ramArea->readBit(offset, i, &readData, E::ByteOrder::LittleEndian, false);
		QVERIFY(ok == true);

		QCOMPARE(readData, data);
	}

	return;
}

void SimRamTests::ramAreaWriteWordTest()
{
	quint16 data;
	const quint16 testValue = 0x1234;
	const quint32 offsetInArea = 100;
	const quint32 offset = s_ra_offset + offsetInArea;
	bool ok = false;

	// Check write in LE
	//
	ok = m_ramArea->writeWord(offset, testValue, E::ByteOrder::LittleEndian);
	QVERIFY(ok == true);
	QCOMPARE(testValue, *reinterpret_cast<quint16*>(m_ramArea->m_data.data() + offsetInArea * 2));

	// Check read in LE
	//
	ok = m_ramArea->readWord(offset, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == true);
	QCOMPARE(data, testValue);

	// Check write in BE
	//
	ok = m_ramArea->writeWord(offset, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == true);
	QCOMPARE(testValue, qToBigEndian<quint16>(*reinterpret_cast<quint16*>(m_ramArea->m_data.data() + offsetInArea * 2)));

	// Check read in BE
	//
	ok = m_ramArea->readWord(offset, &data, E::ByteOrder::BigEndian, false);
	QVERIFY(ok == true);
	QCOMPARE(data, testValue);

	// Test apply override
	//
	quint32 overrideDataOffset = s_ra_offset + s_ra_size / 2;

	m_ramArea->clear();
	ok = m_ramArea->writeWord(overrideDataOffset, 0x1263, E::ByteOrder::BigEndian);		// Set to cleared memory some val
	QVERIFY(ok == true);

	std::vector<Sim::OverrideRamRecord> ovr;
	ovr.resize(s_ra_size);

	ovr[s_ra_size / 2] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0x1122)};
	m_ramArea->setOverrideData(std::move(ovr));

	ok = m_ramArea->readWord(overrideDataOffset, &data, E::ByteOrder::BigEndian, false);	// Not apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x1263);

	ok = m_ramArea->readWord(overrideDataOffset, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x1122);

	// Test errors
	//
#ifdef QT_NO_DEBUG
	ok = m_ramArea->writeWord(s_ra_offset - 1, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == false);

	ok = m_ramArea->writeWord(s_ra_offset + s_ra_size, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == false);

	ok = m_ramArea->readWord(s_ra_offset - 1, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == false);

	ok = m_ramArea->readWord(s_ra_offset + s_ra_size, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == false);
#endif

	return;
}

void SimRamTests::ramAreaWriteDwordTest()
{
	quint32 data;
	const quint32 testValue = 0x12345678;
	const quint32 offsetInArea = 100;
	const quint32 offset = s_ra_offset + offsetInArea;
	bool ok = false;

	// Check write in LE
	//
	ok = m_ramArea->writeDword(offset, testValue, E::ByteOrder::LittleEndian);
	QVERIFY(ok == true);
	QCOMPARE(testValue, *reinterpret_cast<quint32*>(m_ramArea->m_data.data() + offsetInArea * 2));

	// Check read in LE
	//
	ok = m_ramArea->readDword(offset, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == true);
	QCOMPARE(data, testValue);

	// Check write in BE
	//
	ok = m_ramArea->writeDword(offset, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == true);
	QCOMPARE(testValue, qToBigEndian<quint32>(*reinterpret_cast<quint32*>(m_ramArea->m_data.data() + offsetInArea * 2)));

	// Check read in BE
	//
	ok = m_ramArea->readDword(offset, &data, E::ByteOrder::BigEndian, false);
	QVERIFY(ok == true);
	QCOMPARE(data, testValue);

	// Test apply override
	//
	quint32 overrideDataOffset = s_ra_offset + s_ra_size / 2;

	m_ramArea->clear();
	ok = m_ramArea->writeDword(overrideDataOffset, 0x12345678, E::ByteOrder::BigEndian);		// Set to cleared memory some val
	QVERIFY(ok == true);

	std::vector<Sim::OverrideRamRecord> ovr;
	ovr.resize(s_ra_size);

	ovr[s_ra_size / 2] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0x1122)};
	ovr[s_ra_size / 2 + 1] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0x3344)};
	m_ramArea->setOverrideData(std::move(ovr));

	ok = m_ramArea->readDword(overrideDataOffset, &data, E::ByteOrder::BigEndian, false);		// Not apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x12345678);

	ok = m_ramArea->readDword(overrideDataOffset, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x11223344);

	// Test errors
	//
#ifdef QT_NO_DEBUG
	ok = m_ramArea->writeDword(s_ra_offset - 1, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == false);

	ok = m_ramArea->writeDword(s_ra_offset + s_ra_size, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == false);

	ok = m_ramArea->readDword(s_ra_offset - 1, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == false);

	ok = m_ramArea->readDword(s_ra_offset + s_ra_size, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == false);
#endif

	return;
}

void SimRamTests::ramAreaWriteSignedIntTest()
{
	qint32 data;
	const qint32 testValue = -2'090'123'456;
	const quint32 offsetInArea = 100;
	const quint32 offset = s_ra_offset + offsetInArea;
	bool ok = false;

	// Check write in LE
	//
	ok = m_ramArea->writeSignedInt(offset, testValue, E::ByteOrder::LittleEndian);
	QVERIFY(ok == true);
	QCOMPARE(testValue, *reinterpret_cast<qint32*>(m_ramArea->m_data.data() + offsetInArea * 2));

	// Check read in LE
	//
	ok = m_ramArea->readSignedInt(offset, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == true);
	QCOMPARE(data, testValue);

	// Check write in BE
	//
	ok = m_ramArea->writeSignedInt(offset, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == true);
	QCOMPARE(testValue, qToBigEndian<qint32>(*reinterpret_cast<qint32*>(m_ramArea->m_data.data() + offsetInArea * 2)));

	// Check read in BE
	//
	ok = m_ramArea->readSignedInt(offset, &data, E::ByteOrder::BigEndian, false);
	QVERIFY(ok == true);
	QCOMPARE(data, testValue);

	// Test apply override
	//
	quint32 overrideDataOffset = s_ra_offset + s_ra_size / 2;

	m_ramArea->clear();
	ok = m_ramArea->writeSignedInt(overrideDataOffset, -1'234'567'891, E::ByteOrder::BigEndian);		// Set to cleared memory some val
	QVERIFY(ok == true);

	std::vector<Sim::OverrideRamRecord> ovr;
	ovr.resize(s_ra_size);

	// B6 69 FB 66 == -1234568346
	ovr[s_ra_size / 2] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0xB669)};
	ovr[s_ra_size / 2 + 1] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0xFB66)};
	m_ramArea->setOverrideData(std::move(ovr));

	ok = m_ramArea->readSignedInt(overrideDataOffset, &data, E::ByteOrder::BigEndian, false);		// Not apply override
	QVERIFY(ok == true);
	QCOMPARE(data, -1'234'567'891);

	ok = m_ramArea->readSignedInt(overrideDataOffset, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, -1234568346);

	// Test errors
	//
#ifdef QT_NO_DEBUG
	ok = m_ramArea->writeSignedInt(s_ra_offset - 1, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == false);

	ok = m_ramArea->writeSignedInt(s_ra_offset + s_ra_size, testValue, E::ByteOrder::BigEndian);
	QVERIFY(ok == false);

	ok = m_ramArea->readSignedInt(s_ra_offset - 1, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == false);

	ok = m_ramArea->readSignedInt(s_ra_offset + s_ra_size, &data, E::ByteOrder::LittleEndian, false);
	QVERIFY(ok == false);
#endif

	return;
}

void SimRamTests::ramCreate()
{
	// Test adding memory areas
	//
	{
		Sim::Ram ram;
		bool ok = true;

		ok &= ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
		ok &= ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
		ok &= ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100, 100, false, "RW100");
		ok &= ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");

		QVERIFY(ok == true);
		QVERIFY(ram.m_memoryAreas.size() == 4);
		QVERIFY(ram.m_readAreas.size() == 3);
		QVERIFY(ram.m_writeAreas.size() == 3);
	}

	// Test error for overlapping memory areas
	//
	{
		Sim::Ram ram;
		bool ok = true;

		ok = ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
		QVERIFY(ok == true);

		ok = ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
		QVERIFY(ok == true);

		ok = ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100 - 1, 100, false, "RW100");	// Ovelapping must occur
		QVERIFY(ok == false);

		ok = ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");
		QVERIFY(ok == true);

		QVERIFY(ram.m_memoryAreas.size() == 3);
		QVERIFY(ram.m_readAreas.size() == 2);
		QVERIFY(ram.m_writeAreas.size() == 2);
	}

	return;
}

void SimRamTests::ramIsNull()
{
	Sim::Ram ram;
	QVERIFY(ram.isNull() == true);

	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	QVERIFY(ram.isNull() == false);

	ram.m_memoryAreas.clear();
	QVERIFY(ram.isNull() == true);

	return;
}

void SimRamTests::ramReset()
{
	Sim::Ram ram;

	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	QVERIFY(ram.isNull() == false);

	ram.reset();
	QVERIFY(ram.isNull() == true);

	return;
}

void SimRamTests::ramUpdateFrom()
{
	Sim::Ram ramSrc;
	bool ok = true;

	ok &= ramSrc.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ok &= ramSrc.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	ok &= ramSrc.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100, 100, false, "RW100");
	ok &= ramSrc.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");

	QVERIFY(ok == true);
	QVERIFY(ramSrc.m_memoryAreas.size() == 4);
	QVERIFY(ramSrc.m_readAreas.size() == 3);
	QVERIFY(ramSrc.m_writeAreas.size() == 3);

	Sim::Ram ramDst;
	ramDst.updateFrom(ramSrc);						// <<<
	QVERIFY(ramSrc.m_memoryAreas.size() == 4);
	QVERIFY(ramSrc.m_readAreas.size() == 3);
	QVERIFY(ramSrc.m_writeAreas.size() == 3);

	return;
}

void SimRamTests::ramMemoryAreaHandle()
{
	Sim::Ram ram;
	static const Sim::Ram::Handle Invalidhandle = std::numeric_limits<size_t>::max();

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 100, 100, false, "RM100");
	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");
	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 300, 100, false, "RW300");

	Sim::Ram::Handle mah = ram.memoryAreaHandle(E::LogicModuleRamAccess::Read, 10);
	QVERIFY(mah != Invalidhandle);
	QVERIFY(ram.m_memoryAreas[mah].name() == "RM0");

	mah = ram.memoryAreaHandle(E::LogicModuleRamAccess::Read, 210);
	QVERIFY(mah != Invalidhandle);
	QVERIFY(ram.memoryArea(mah)->name() == "RW200");

	mah = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 10);
	QVERIFY(mah != Invalidhandle);
	QVERIFY(ram.memoryArea(mah)->name() == "WM0");

	mah = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 110);	// No such area!
	QVERIFY(mah == Invalidhandle);
	QVERIFY(ram.memoryArea(mah) == nullptr);

	mah = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 210);
	QVERIFY(mah != Invalidhandle);
	QVERIFY(ram.memoryArea(mah)->name() == "RW200");

	return;
}

void SimRamTests::ramReadWriteBuffer()
{
	Sim::Ram ram;
	bool ok = true;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100, 100, false, "RW100");
	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");

	std::vector<char> buffer;
	buffer.resize(100);
	std::iota(buffer.begin(), buffer.end(), 0);

	ok = ram.writeBuffer(0, E::LogicModuleRamAccess::Write, buffer);
	QVERIFY(ok == true);

	buffer.clear();
	ok = ram.readToBuffer(0, E::LogicModuleRamAccess::Write, 50, &buffer);	// 50 words
	QVERIFY(ok == true);
	QVERIFY(buffer.size() == 100);

	for (int i = 0; i < buffer.size(); i++)
	{
		QVERIFY(buffer[i] == i);
	}

	return;
}

void SimRamTests::ramMovMem()
{
	Sim::Ram ram;
	bool ok = true;

	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100, 100, false, "RW100");
	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");

	std::vector<char> buffer;
	buffer.resize(200);
	std::iota(buffer.begin(), buffer.end(), 0);

	ok = ram.writeBuffer(100, E::LogicModuleRamAccess::Write, buffer);
	QVERIFY(ok == true);

	ok = ram.movMem(100, 200, 100);
	QVERIFY(ok == true);

	buffer.clear();
	ok = ram.readToBuffer(200, E::LogicModuleRamAccess::Read, 100, &buffer);	// 50 words
	QVERIFY(ok == true);
	QVERIFY(buffer.size() == 200);

	for (size_t i = 0; i < buffer.size(); i++)
	{
		QVERIFY(buffer[i] == static_cast<char>(i));		// char is signed
	}

	return;
}

void SimRamTests::ramSetMem()
{
	Sim::Ram ram;
	bool ok = true;

	ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100, 100, false, "RW100");

	ok = ram.setMem(110, 10, 0x1234);
	QVERIFY(ok == true);

	for (quint32 a = 100; a < 110; a++)
	{
		quint16 data;
		ram.readWord(a, &data, E::BigEndian);
		QCOMPARE(data, 0x0000);
	}

	for (quint32 a = 110; a < 120; a++)
	{
		quint16 data;
		ram.readWord(a, &data, E::BigEndian);
		QCOMPARE(data, 0x1234);
	}

	for (quint32 a = 120; a < 200; a++)
	{
		quint16 data;
		ram.readWord(a, &data, E::BigEndian);
		QCOMPARE(data, 0x0000);
	}

	return;
}

void SimRamTests::ramReadWriteBit()
{
	Sim::Ram ram;
	bool ok = true;
	quint16 data;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");

	// --
	//
	ram.writeBit(10, 5, 1, E::BigEndian);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 32);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0);

	// --
	//
	ram.writeBit(10, 6, 1, E::BigEndian, E::LogicModuleRamAccess::Read);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 32);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 64);

	// --
	//
	ram.readBit(10, 5, &data, E::BigEndian);
	QCOMPARE(data, 0);

	ram.readBit(10, 5, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 1);

	ram.readBit(10, 6, &data, E::BigEndian);
	QCOMPARE(data, 1);

	ram.readBit(10, 6, &data, E::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0);

	return;
}

void SimRamTests::ramReadWriteWord()
{
	Sim::Ram ram;
	bool ok = true;
	quint16 data;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");

	// --
	//
	ram.writeWord(10, 0x1122, E::BigEndian);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0x1122);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0);

	// --
	//
	ram.writeWord(10, 0x3344, E::BigEndian, E::LogicModuleRamAccess::Read);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0x1122);

	ram.readWord(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0x3344);

	return;
}

void SimRamTests::ramReadWriteDword()
{
	Sim::Ram ram;
	bool ok = true;
	quint32 data;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");

	// --
	//
	ram.writeDword(10, 0x11223344, E::BigEndian);

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0x11223344);

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0);

	// --
	//
	ram.writeDword(10, 0x55667788, E::BigEndian, E::LogicModuleRamAccess::Read);

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0x11223344);

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0x55667788);

	return;
}
