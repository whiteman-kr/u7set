#include "SimRamTests.h"
#include <QtTest>

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

void SimRamTests::ramAreaDump()
{
	QString dump = m_ramArea->dump();

	// Just check that some dump is forming
	QVERIFY(dump.startsWith("Memory Area: "));
	QVERIFY(dump.endsWith(" All zeroes....\n"));	// Initially RAM is 0s

	// Add non zero value
	//
	m_ramArea->writeWord(SimRamTests::s_ra_offset + SimRamTests::s_ra_size - 1, 0xFFFF, E::BigEndian);
	dump = m_ramArea->dump();

	QVERIFY(dump.startsWith("Memory Area: "));
	QVERIFY(dump.endsWith(" 0000 ffff\n"));

	return;
}

void SimRamTests::ramDump()
{
	Sim::Ram ram;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");

	QString dump = ram.dump("LMID");
	QVERIFY(dump.startsWith("RAM dump for Module LMID"));

	return;
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

void SimRamTests::ramAreaClearTest()
{
	// Put some data into area
	//
	m_ramArea->writeData<quint16>(SimRamTests::s_ra_offset + 0, 0x4455, E::ByteOrder::BigEndian);
	m_ramArea->writeData<quint16>(SimRamTests::s_ra_offset + SimRamTests::s_ra_size / 2, 0x2233, E::ByteOrder::BigEndian);
	m_ramArea->writeData<quint16>(SimRamTests::s_ra_offset + SimRamTests::s_ra_size - 1, 0x1122, E::ByteOrder::BigEndian);

	// Clear area
	//
	m_ramArea->clear();

	// Test that area was cleared
	//
	for (quint32 i = 0; i < SimRamTests::s_ra_size; i++)
	{
		quint16 data = 0xFFFF;
		bool ok = m_ramArea->readData<quint16>(SimRamTests::s_ra_offset + i, &data, E::ByteOrder::BigEndian, true);
		QVERIFY(ok == true);

		QCOMPARE(data, 0);
	}

	return;
}

void SimRamTests::ramGetMemoryAreasTest()
{
	Sim::Ram ram;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");

	std::vector<Sim::RamArea*> mutableMemoryAreas = ram.memoryAreas();
	std::vector<const Sim::RamArea*> constMemoryAreas = std::as_const(ram).memoryAreas();

	QCOMPARE(mutableMemoryAreas.size(), 2);
	QCOMPARE(constMemoryAreas.size(), 2);

	return;
}

void SimRamTests::ramGetMemoryAreaHandle()
{
	Sim::Ram ram;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 100, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 200, 100, false, "RM1");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 200, 100, false, "WM0");

	{
		Sim::Ram::Handle h = ram.memoryAreaHandle(E::LogicModuleRamAccess::Read, 100);
		QVERIFY(h != Sim::Ram::InvalidHandle);

		Sim::RamArea* area = ram.memoryArea(h);
		QVERIFY(area != nullptr);
		QCOMPARE(area->name(), "RM0");
		QCOMPARE(area->offset(), 100);
		QCOMPARE(area->size(), 100);

		const Sim::RamArea* carea = std::as_const(ram).memoryArea(h);
		QVERIFY(carea != nullptr);
		QCOMPARE(carea->name(), "RM0");
	}

	{
		Sim::Ram::Handle h = ram.memoryAreaHandle(E::LogicModuleRamAccess::Read, 200);
		QVERIFY(h != Sim::Ram::InvalidHandle);

		Sim::RamArea* area = ram.memoryArea(h);
		QVERIFY(area != nullptr);
		QCOMPARE(area->name(), "RM1");

		const Sim::RamArea* carea = std::as_const(ram).memoryArea(h);
		QVERIFY(carea != nullptr);
		QCOMPARE(carea->name(), "RM1");
	}

	{
		Sim::Ram::Handle h = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 200);
		QVERIFY(h != Sim::Ram::InvalidHandle);

		Sim::RamArea* area = ram.memoryArea(h);
		QVERIFY(area != nullptr);
		QCOMPARE(area->name(), "WM0");

		const Sim::RamArea* carea = std::as_const(ram).memoryArea(h);
		QVERIFY(carea != nullptr);
		QCOMPARE(carea->name(), "WM0");

		h = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 200 + 99);
		QVERIFY(h != Sim::Ram::InvalidHandle);

		area = ram.memoryArea(h);
		QVERIFY(area != nullptr);
		QCOMPARE(area->name(), "WM0");
	}

	{
		Sim::Ram::Handle h = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 100);
		QCOMPARE(h, Sim::Ram::InvalidHandle);
	}

	{
		Sim::Ram::Handle h = ram.memoryAreaHandle(E::LogicModuleRamAccess::Write, 200 + 100);
		QCOMPARE(h, Sim::Ram::InvalidHandle);
	}

	return;
}

void SimRamTests::ramGetOverrideData()
{
	const quint32 overrideDataOffsetAreaW = s_ra_size / 2;
	Sim::RamArea ramArea{E::LogicModuleRamAccess::Write, s_ra_offset, s_ra_size, true, "RA"};

	std::vector<Sim::OverrideRamRecord> ovr;
	ovr.resize(s_ra_size);

	ovr[overrideDataOffsetAreaW + 0] = Sim::OverrideRamRecord{0xF00F, qToBigEndian<quint16>(0x1122)};
	ovr[overrideDataOffsetAreaW + 1] = Sim::OverrideRamRecord{0x0FF0, qToBigEndian<quint16>(0x3344)};
	ovr[overrideDataOffsetAreaW + 2] = Sim::OverrideRamRecord{0xFFF0, qToBigEndian<quint16>(0x5566)};

	ramArea.setOverrideData(std::move(ovr));

	// --
	//
	std::vector<Sim::OverrideRamRecord> newOverrideData = ramArea.overrideData();

	bool areEqual = std::equal(ovr.begin(), ovr.end(), newOverrideData.begin());
	QCOMPARE(areEqual, true);

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
		quint16 data = (i % 3) ? 0 : 1;		// Write every third bit to 1

		ok = m_ramArea->writeBit(offset, i, data, E::ByteOrder::LittleEndian);
		QVERIFY(ok == true);

		quint16 readData;
		ok = m_ramArea->readBit(offset, i, &readData, E::ByteOrder::LittleEndian, false);
		QVERIFY(ok == true);

		QCOMPARE(readData, data);
	}

	// Check write the very first and the very last bits
	//
	{
		QCOMPARE(m_ramArea->data()[0], 0);

		ok = m_ramArea->writeBit(s_ra_offset, 15, 1, E::ByteOrder::BigEndian);
		QVERIFY(ok == true);

		QCOMPARE(m_ramArea->data()[0], static_cast<char>(0x80));
	}

	{
		QCOMPARE(m_ramArea->data()[s_ra_size * 2 - 1], 0);

		ok = m_ramArea->writeBit(s_ra_offset + s_ra_size - 1, 0, 1, E::ByteOrder::BigEndian);
		QVERIFY(ok == true);

		QCOMPARE(m_ramArea->data()[s_ra_size * 2 - 1], 0x01);
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
	ovr[s_ra_size / 2 + 1] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0x3344)};

	m_ramArea->setOverrideData(std::move(ovr));

	ok = m_ramArea->readWord(overrideDataOffset, &data, E::ByteOrder::BigEndian, false);	// Not apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x1263);

	ok = m_ramArea->readWord(overrideDataOffset, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x1122);

	ok = m_ramArea->readWord(overrideDataOffset + 1, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x3344);

	// Check after clear
	//
	m_ramArea->clear();

	quint32 zbased = s_ra_size / 2;
	QCOMPARE(m_ramArea->data()[zbased * 2 + 0], 0x11);
	QCOMPARE(m_ramArea->data()[zbased * 2 + 1], 0x22);

	QCOMPARE(m_ramArea->data()[(zbased + 1) * 2 + 0], 0x33);
	QCOMPARE(m_ramArea->data()[(zbased + 1) * 2 + 1], 0x44);

	ok = m_ramArea->readWord(overrideDataOffset, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x1122);

	ok = m_ramArea->readWord(overrideDataOffset + 1, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, 0x3344);

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

	// Overrida data must be the same size as RamArea
	ovr.resize(s_ra_size);
	ovr[s_ra_size / 2] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0x1122)};
	ovr[s_ra_size / 2 + 1] = Sim::OverrideRamRecord{0xFFFF, qToBigEndian<quint16>(0x3344)};

	m_ramArea->setOverrideData(std::move(ovr));

	ok = m_ramArea->readDword(overrideDataOffset, &data, E::ByteOrder::BigEndian, false);		// Not apply override
	QVERIFY(ok == true);
	QCOMPARE(data, static_cast<quint32>(0x12345678));

	ok = m_ramArea->readDword(overrideDataOffset, &data, E::ByteOrder::BigEndian, true);		// Apply override
	QVERIFY(ok == true);
	QCOMPARE(data, static_cast<quint32>(0x11223344));

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

		// check copy
		//
		Sim::Ram copy = ram;
		QCOMPARE(copy.memoryAreas().size(), 4);
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

void SimRamTests::ramClearMemoryAreasOnStartCycle()
{
	Sim::Ram ram;
	bool ok = true;

	const quint32 offset = 200;
	const quint32 size = 200;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, offset, size, true, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, offset, size, false, "WM0");

	for (quint32 i = 0; i < size; i++)
	{
		ok = ram.writeWord(offset + i, i % 0xFFFF, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
		QCOMPARE(ok, true);

		ok = ram.writeWord(offset + i, i % 0xFFFF, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
		QCOMPARE(ok, true);
	}

	ram.clearMemoryAreasOnStartCycle();

	for (quint32 i = 0; i < size; i++)
	{
		quint16 data;
		ok = ram.readWord(offset + i, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
		QCOMPARE(ok, true);
		QCOMPARE(data, 0);

		ok = ram.readWord(offset + i, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
		QCOMPARE(ok, true);
		QCOMPARE(data, i % 0xFFFF);
	}

	return;
}

void SimRamTests::ramClearMemoryAreas()
{
	Sim::Ram ram;
	bool ok = true;

	const quint32 offset = 200;
	const quint32 size = 200;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, offset, size, true, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, offset, size, false, "WM0");

	for (quint32 i = 0; i < size; i++)
	{
		ok = ram.writeWord(offset + i, i % 0xFFFF, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
		QCOMPARE(ok, true);

		ok = ram.writeWord(offset + i, i % 0xFFFF, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
		QCOMPARE(ok, true);
	}

	ram.clearMemoryArea(offset, E::LogicModuleRamAccess::Read);

	for (quint32 i = 0; i < size; i++)
	{
		quint16 data;
		ok = ram.readWord(offset + i, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
		QCOMPARE(ok, true);
		QCOMPARE(data, 0);

		ok = ram.readWord(offset + i, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
		QCOMPARE(ok, true);
		QCOMPARE(data, i % 0xFFFF);
	}

	return;
}

void SimRamTests::ramReadWriteBuffer()
{
	Sim::Ram ram;
	bool ok = true;

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	QCOMPARE(ok, true);

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	QCOMPARE(ok, true);

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 100, 100, false, "RW100");
	QCOMPARE(ok, true);

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite, 200, 100, false, "RW200");
	QCOMPARE(ok, true);

	std::vector<char> buffer;
	buffer.resize(100);
	std::iota(buffer.begin(), buffer.end(), 0);

	ok = ram.writeBuffer(0, E::LogicModuleRamAccess::Write, buffer);
	QVERIFY(ok == true);

	buffer.clear();
	ok = ram.readToBuffer(0, E::LogicModuleRamAccess::Write, 50, &buffer);	// 50 words
	QVERIFY(ok == true);
	QVERIFY(buffer.size() == 100);

	for (size_t i = 0; i < buffer.size(); i++)
	{
		QVERIFY(buffer[i] == static_cast<char>(i));
	}

	// Check close to area begin
	//
	std::vector<char> smallBuffer{0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	QByteArray smallBufferQt = QByteArray::fromRawData(smallBuffer.data(), smallBuffer.size());

	m_ramArea->writeBuffer(s_ra_offset, smallBufferQt);
	m_ramArea->writeBuffer(s_ra_offset + s_ra_size - static_cast<quint32>(smallBuffer.size()) / 2, smallBufferQt);

	quint16 d = 0;

	m_ramArea->readData(s_ra_offset + 0, &d, E::ByteOrder::BigEndian, true);
	QCOMPARE(d, 0x1122);

	m_ramArea->readData(s_ra_offset + 0, &d, E::ByteOrder::BigEndian, true);
	QCOMPARE(d, 0x1122);

	m_ramArea->readData(s_ra_offset + 1, &d, E::ByteOrder::BigEndian, true);
	QCOMPARE(d, 0x3344);

	m_ramArea->readData(s_ra_offset + s_ra_size - 3, &d, E::ByteOrder::BigEndian, true);
	QCOMPARE(d, 0x1122);

	m_ramArea->readData(s_ra_offset + s_ra_size - 2, &d, E::ByteOrder::BigEndian, true);
	QCOMPARE(d, 0x3344);

	m_ramArea->readData(s_ra_offset + s_ra_size - 1, &d, E::ByteOrder::BigEndian, true);
	QCOMPARE(d, 0x5566);

	return;
}

void SimRamTests::ramReadWriteBufferOverride()
{
	bool ok = true;
	const quint32 overrideDataOffsetAreaW = s_ra_size / 2;

	Sim::Ram ram;
	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Write, s_ra_offset, s_ra_size, true, "RA");
	QVERIFY(ok == true);

	Sim::RamArea* ramArea = ram.memoryArea(E::LogicModuleRamAccess::Write, s_ra_offset);
	QVERIFY(ramArea != nullptr);

	std::vector<Sim::OverrideRamRecord> ovr;
	ovr.resize(s_ra_size);

	ovr[overrideDataOffsetAreaW + 0] = Sim::OverrideRamRecord{0xF00F, qToBigEndian<quint16>(0x1122)};
	ovr[overrideDataOffsetAreaW + 1] = Sim::OverrideRamRecord{0x0FF0, qToBigEndian<quint16>(0x3344)};
	ovr[overrideDataOffsetAreaW + 2] = Sim::OverrideRamRecord{0xFFF0, qToBigEndian<quint16>(0x5566)};

	ramArea->setOverrideData(std::move(ovr));

	QByteArray buffer;
	buffer.resize(s_ra_size * 2);
	std::iota(buffer.begin(), buffer.end(), 0);

	ok = ram.writeBuffer(s_ra_offset, E::LogicModuleRamAccess::Write, buffer);
	QVERIFY(ok == true);

	{
		const QByteArray& rawData = ramArea->data();
		QCOMPARE(rawData.size(), buffer.size());

		char ch = 0;
		for (qsizetype i = 0; i < s_ra_size * 2; i++, ch++)
		{
			if (i < overrideDataOffsetAreaW * 2 ||
				i > (overrideDataOffsetAreaW + 3) * 2)
			{
				QCOMPARE(rawData[i], ch);
			}
		}

		quint16 v = 0;

		v = qFromBigEndian<quint16>(rawData.constData() + (overrideDataOffsetAreaW + 0) * 2);
		QCOMPARE(v & ~0xF00F, 0x1122 & ~0xF00F);

		v = qFromBigEndian<quint16>(rawData.constData() + (overrideDataOffsetAreaW + 1) * 2);
		QCOMPARE(v & ~0x0FF0, 0x3344 & ~0x0FF0);

		v = qFromBigEndian<quint16>(rawData.constData() + (overrideDataOffsetAreaW + 2) * 2);
		QCOMPARE(v & ~0xFFF0, 0x5566 & ~0xFFF0);
	}

	{
		QByteArray readBuffer;
		ok = ram.readToBuffer(s_ra_offset, E::LogicModuleRamAccess::Write, s_ra_size, &readBuffer, true);

		QCOMPARE(ok, true);
		QCOMPARE(readBuffer.size(), buffer.size());

		char ch = 0;
		for (qsizetype i = 0; i < s_ra_size * 2; i++, ch++)
		{
			if (i < overrideDataOffsetAreaW * 2 ||
				i > (overrideDataOffsetAreaW + 3) * 2)
			{
				QCOMPARE(readBuffer[i], ch);
			}
		}

		quint16 v = 0;

		v = qFromBigEndian<quint16>(readBuffer.constData() + (overrideDataOffsetAreaW + 0) * 2);
		QCOMPARE(v & ~0xF00F, 0x1122 & ~0xF00F);

		v = qFromBigEndian<quint16>(readBuffer.constData() + (overrideDataOffsetAreaW + 1) * 2);
		QCOMPARE(v & ~0x0FF0, 0x3344 & ~0x0FF0);

		v = qFromBigEndian<quint16>(readBuffer.constData() + (overrideDataOffsetAreaW + 2) * 2);
		QCOMPARE(v & ~0xFFF0, 0x5566 & ~0xFFF0);
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

	// Check close to begin & end
	//
	{
		quint16 data;
		m_ramArea->setMem(s_ra_offset, 3, 0x1234);

		m_ramArea->readWord(s_ra_offset + 0, &data, E::BigEndian, true);
		QCOMPARE(data, 0x1234);

		m_ramArea->readWord(s_ra_offset + 1, &data, E::BigEndian, true);
		QCOMPARE(data, 0x1234);

		m_ramArea->readWord(s_ra_offset + 2, &data, E::BigEndian, true);
		QCOMPARE(data, 0x1234);

		m_ramArea->readWord(s_ra_offset + 3, &data, E::BigEndian, true);
		QCOMPARE(data, 0);
	}

	{
		quint16 data;
		m_ramArea->setMem(s_ra_offset + s_ra_size - 3, 3, 0x1234);

		m_ramArea->readWord(s_ra_offset + s_ra_size - 3, &data, E::BigEndian, true);
		QCOMPARE(data, 0x1234);

		m_ramArea->readWord(s_ra_offset + s_ra_size - 2, &data, E::BigEndian, true);
		QCOMPARE(data, 0x1234);

		m_ramArea->readWord(s_ra_offset + s_ra_size - 1, &data, E::BigEndian, true);
		QCOMPARE(data, 0x1234);
	}

	return;
}

void SimRamTests::ramSetMemWithOverride()
{
	bool ok = true;
	const quint32 overrideDataOffsetAreaW = s_ra_size / 2;

	Sim::Ram ram;
	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Write, s_ra_offset, s_ra_size, true, "RA");
	QVERIFY(ok == true);

	Sim::RamArea* ramArea = ram.memoryArea(E::LogicModuleRamAccess::Write, s_ra_offset);
	QVERIFY(ramArea != nullptr);

	std::vector<Sim::OverrideRamRecord> ovr;
	ovr.resize(s_ra_size);

	ovr[overrideDataOffsetAreaW + 0] = Sim::OverrideRamRecord{0xF00F, qToBigEndian<quint16>(0x1122)};
	ovr[overrideDataOffsetAreaW + 1] = Sim::OverrideRamRecord{0x0FF0, qToBigEndian<quint16>(0x3344)};
	ovr[overrideDataOffsetAreaW + 2] = Sim::OverrideRamRecord{0xFFF0, qToBigEndian<quint16>(0x5566)};

	// setMem
	//
	ramArea->setOverrideData(std::move(ovr));

	ok = ram.setMem(s_ra_offset, s_ra_size, 0xCECE);
	QCOMPARE(ok, true);

	// Checks
	//
	{
		const QByteArray& rawData = ramArea->data();
		QCOMPARE(rawData.size(), s_ra_size * 2);

		char ch = static_cast<char>(0xCE);
		for (qsizetype i = 0; i < s_ra_size * 2; i++)
		{
			if (i < overrideDataOffsetAreaW * 2 ||
				i > (overrideDataOffsetAreaW + 3) * 2)
			{
				QCOMPARE(rawData[i], ch);
			}
		}

		quint16 v = 0;

		v = qFromBigEndian<quint16>(rawData.constData() + (overrideDataOffsetAreaW + 0) * 2);
		QCOMPARE(v & ~0xF00F, 0x1122 & ~0xF00F);

		v = qFromBigEndian<quint16>(rawData.constData() + (overrideDataOffsetAreaW + 1) * 2);
		QCOMPARE(v & ~0x0FF0, 0x3344 & ~0x0FF0);

		v = qFromBigEndian<quint16>(rawData.constData() + (overrideDataOffsetAreaW + 2) * 2);
		QCOMPARE(v & ~0xFFF0, 0x5566 & ~0xFFF0);
	}

	return;
}

void SimRamTests::ramReadWriteBit()
{
	Sim::Ram ram;
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
	quint32 data;

	ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");

	// --
	//
	ram.writeDword(10, 0x11223344, E::BigEndian);

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, static_cast<quint32>(0x11223344));

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, static_cast<quint32>(0));

	// --
	//
	ram.writeDword(10, 0x55667788, E::BigEndian, E::LogicModuleRamAccess::Read);

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, static_cast<quint32>(0x11223344));

	ram.readDword(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, static_cast<quint32>(0x55667788));

	return;
}

void SimRamTests::ramReadWriteFloat()
{
	Sim::Ram ram;
	bool ok = false;
	float data = 0.0;

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	QCOMPARE(ok, true);

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	QCOMPARE(ok, true);

	// --
	//
	ok = ram.writeFloat(10, 123.0, E::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(ok, true);

	ok = ram.readFloat(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(ok, true);
	QCOMPARE(data, 123.0);

	ram.readFloat(8, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0);

	ram.readFloat(12, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0);

	ram.readFloat(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0);

	return;
}

void SimRamTests::ramReadWriteSignedInt32()
{
	Sim::Ram ram;
	bool ok = false;
	qint32 data = 0;

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Read, 0, 100, false, "RM0");
	QCOMPARE(ok, true);

	ok = ram.addMemoryArea(E::LogicModuleRamAccess::Write, 0, 100, false, "WM0");
	QCOMPARE(ok, true);

	// --
	//
	ok = ram.writeSignedInt(10, -123, E::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(ok, true);

	ok = ram.readSignedInt(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(ok, true);
	QCOMPARE(data, -123);

	ram.readSignedInt(8, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0);

	ram.readSignedInt(12, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Write);
	QCOMPARE(data, 0);

	ram.readSignedInt(10, &data, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
	QCOMPARE(data, 0);

	return;
}

void SimRamTests::ramOverrideSignalLastCounter()
{
	Sim::Ram ram;

	QCOMPARE(ram.overrideSignalsLastCounter(0), -1);
	QCOMPARE(ram.overrideSignalsLastCounter(1), 0);
	QCOMPARE(ram.overrideSignalsLastCounter(2), 1);

	return;
}

