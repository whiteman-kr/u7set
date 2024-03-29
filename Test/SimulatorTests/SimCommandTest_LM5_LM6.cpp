#include "SimCommandTest_LM5_LM6.h"

#include <Simulator/SimConsoleLogFile.h>

#include "../../libs/Simulator/SimCommandProcessor_LM5_LM6.h"
#include "../../libs/Simulator/SimDeviceEmulator.h"
#include "../../libs/Simulator/SimException.h"
#include "../../libs/Simulator/SimulatorPrivate.h"

#include <QtTest>


SimCommandTest_LM5_LM6::SimCommandTest_LM5_LM6() = default;
SimCommandTest_LM5_LM6::~SimCommandTest_LM5_LM6() = default;

void SimCommandTest_LM5_LM6::initTestCase()
{
	Sim::ConsoleLogFile consoleLog;
	Sim::SimulatorPrivate simulator{&consoleLog, false, nullptr};

	m_device = std::make_unique<Sim::DeviceEmulator>(&simulator);

	QFile lmDescriptionFile(":/LM1_SR05.xml");

	if (lmDescriptionFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		qDebug() << lmDescriptionFile.errorString();
		QFAIL(lmDescriptionFile.errorString().toStdString().data());
		return;
	}

	QString errorMessage;
	bool ok = m_device->m_lmDescription.load(lmDescriptionFile.readAll(), &errorMessage);
	if (ok == false)
	{
		QFAIL(errorMessage.toStdString().data());
		return;
	}

	// Create specific CommandProcessor
	//
	Sim::CommandProcessor_LM5_LM6* inst =
		dynamic_cast<Sim::CommandProcessor_LM5_LM6*>(Sim::CommandProcessor::createInstance(m_device.get()));
	m_cp.reset(inst);

	QVERIFY(m_cp);

	// --
	//
	ok = m_device->m_afbComponents.init(m_device->m_lmDescription);
	QVERIFY(ok);

	// Init RAM
	// Command processor needs initialized memory areas, so it can cache them to DeviceCommand
	//
	ok = m_device->initMemory();
	QVERIFY(ok);

	// Fill sanitizer array, so we will not have `uninitialized memory read` errors
	//
	for (quint16 i = 0; i < std::numeric_limits<quint16>::max(); i++)
	{
		m_cp->m_parseMemorySanitizer.insert(i);
	}

	return;
}

void SimCommandTest_LM5_LM6::cleanupTestCase()
{
	m_cp.reset();
}

void SimCommandTest_LM5_LM6::init() {}

void SimCommandTest_LM5_LM6::cleanup() {}

// Test command nop, opcode 1
//
void SimCommandTest_LM5_LM6::test_command_nop()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0040);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(1 << 6); // CommandOpCode

	try
	{
		m_cp->parse_nop(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: reset acc
// opcode: 1 - ext: 0b100000
//
void SimCommandTest_LM5_LM6::test_command_reset()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0060);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b100000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0xABCD);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command: set acc
// opcode: 1 - ext: 0b010000
//
void SimCommandTest_LM5_LM6::test_command_set()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0050);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b010000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0xABCD);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xFFFF);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xFFFF);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command: or acc
// opcode: 1 - ext: 0b110000
//
void SimCommandTest_LM5_LM6::test_command_or()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0070);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b110000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0xABCD);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 1);

		m_device->setAcc(0xFFFF);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 1);

		m_device->setAcc(0x8000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 1);

		m_device->setAcc(0x0001);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 1);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command: and acc
// opcode: 1 - ext: 0b001000
//
void SimCommandTest_LM5_LM6::test_command_and()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0048);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b001000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0xABCD);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);

		m_device->setAcc(0x8000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);

		m_device->setAcc(0x0001);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);

		m_device->setAcc(0xFFFF);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command: not acc
// opcode: 1 - ext: 0b101000
//
void SimCommandTest_LM5_LM6::test_command_not()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0068);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b101000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0xABCD);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x5432); // ~0xABCD = 0x5432

		m_device->setAcc(0x8000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x7FFF);

		m_device->setAcc(0x0001);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xFFFE);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xFFFF);

		m_device->setAcc(0xFFFF);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command: lshift0 acc
// opcode: 1 - ext: 0b011000
//
void SimCommandTest_LM5_LM6::test_command_lshift0()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0058);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b011000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(1);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 2);

		m_device->setAcc(0x7777);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xEEEE);

		m_device->setAcc(0x8000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command: lshift1 acc
// opcode: 1 - ext: 0b111000
//
void SimCommandTest_LM5_LM6::test_command_lshift1()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x0078);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((1 << 6) | 0b111000); // CommandOpCode

	try
	{
		m_cp->parse_reset(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0x0001);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0003);

		m_device->setAcc(0x7777);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xEEEF);

		m_device->setAcc(0x8000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0001);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0003);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0007);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x000F);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x001F);

		m_device->setAcc(0);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	return;
}

// Test command wrfb, opcode 2
//
void SimCommandTest_LM5_LM6::test_command_startafb()
{
	const quint16 opcode = 2;
	const quint16 afbOpCode = 1;
	const quint16 afbInstance = 0;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(afbInstance << 6);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_startafb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// No test for run
		//
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command stop, opcode 3
//
void SimCommandTest_LM5_LM6::test_command_stop()
{
	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(0x00C0);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(3 << 6); // CommandOpCode

	try
	{
		m_cp->parse_stop(&command);
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setPhase(Sim::CyclePhase::IdrPhase);
		m_cp->runCommand(command); // IDS->ALP
		QCOMPARE(m_device->phase(), Sim::CyclePhase::AlpPhase);

		m_device->setPhase(Sim::CyclePhase::AlpPhase);
		m_cp->runCommand(command); // IDT->ODT
		QCOMPARE(m_device->phase(), Sim::CyclePhase::ODT);
	}
	catch (...)
	{
		QFAIL("");
	}

	try
	{
		m_device->setPhase(Sim::CyclePhase::ST);
		m_cp->runCommand(command); // ST->Exception
		QVERIFY(false);
	}
	catch (const Sim::SimException& e)
	{
		// This exception is expected
		//
		Q_UNUSED(e)
	}
	catch (...)
	{
		QFAIL("Unexpected execptiom STOP command");
	}

	try
	{
		m_device->setPhase(Sim::CyclePhase::ODT);
		m_cp->runCommand(command); // ODT->Exception
		QVERIFY(false);
	}
	catch (const Sim::SimException& e)
	{
		// This exception is expected
		//
		Q_UNUSED(e)
	}
	catch (...)
	{
		QFAIL("Unexpected execptiom STOP command");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: mov
// Code: 4 - ext: 0b000000
//
void SimCommandTest_LM5_LM6::test_command_mov()
{
	const quint16 opCode = 4;

	quint16 appLogicWordDataOffset = AppLogicWordDataOffset;
	const quint16 dst = appLogicWordDataOffset + 10u;
	const quint16 src = AppLogicWordDataOffset + 20u;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.w2 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_mov(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeWord(src, 0xABCD, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 0xABCD);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: mov_addr_acc
// Code: 4 - ext: 0b100000
//
void SimCommandTest_LM5_LM6::test_command_mov_addr_acc()
{
	const quint16 opCode = 4;
	const quint16 opExt = 0b100000;

	quint16 appLogicWordDataOffset = AppLogicWordDataOffset;
	const quint16 dst = appLogicWordDataOffset + 10u;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand((opCode << 6) | opExt);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((opCode << 6) | opExt); // CommandOpCode
	rc.w1 = qToBigEndian(dst);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_mov_addr_acc(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_word0, dst);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAcc(0xABCD);
		m_cp->runCommand(command);
		QCOMPARE(m_device->readRamWord(dst), 0xABCD);

		m_device->setAcc(0xFFFF);
		m_cp->runCommand(command);
		QCOMPARE(m_device->readRamWord(dst), 0xFFFF);

		m_device->setAcc(0x0000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->readRamWord(dst), 0x0000);

		m_device->setAcc(0x8000);
		m_cp->runCommand(command);
		QCOMPARE(m_device->readRamWord(dst), 0x8000);

		m_device->setAcc(0x0001);
		m_cp->runCommand(command);
		QCOMPARE(m_device->readRamWord(dst), 0x0001);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: mov_acc_addr
// Code: 4 - ext: 0b010000
//
void SimCommandTest_LM5_LM6::test_command_mov_acc_addr()
{
	const quint16 opCode = 4;
	const quint16 opExt = 0b010000;

	quint16 appLogicWordDataOffset = AppLogicWordDataOffset;
	const quint16 src = appLogicWordDataOffset + 20u;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand((opCode << 6) | opExt);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((opCode << 6) | opExt); // CommandOpCode
	rc.w1 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_mov_acc_addr(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_word0, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->mutableRam().writeWord(src, 0xABCD, E::BigEndian);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xABCD);

		m_device->mutableRam().writeWord(src, 0xFFFF, E::BigEndian);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0xFFFF);

		m_device->mutableRam().writeWord(src, 0x0000, E::BigEndian);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0000);

		m_device->mutableRam().writeWord(src, 0x8000, E::BigEndian);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x8000);

		m_device->mutableRam().writeWord(src, 0x0001, E::BigEndian);
		m_cp->runCommand(command);
		QCOMPARE(m_device->acc(), 0x0001);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command movmem, opcode 5
//
void SimCommandTest_LM5_LM6::test_command_movmem()
{
	const quint16 opCode = 5;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 src = AppLogicWordDataOffset + 200;
	const quint16 size = 16;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(dst);
	rc.w2 = qToBigEndian<quint16>(src);
	rc.w3 = qToBigEndian<quint16>(size);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movmem(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, src);
		QCOMPARE(command.m_word2, size);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		std::array<quint16, size> data;
		std::iota(data.begin(), data.end(), 0xAB00);

		for (size_t i = 0; i < size; i++)
		{
			bool ok = m_device->mutableRam().writeWord(static_cast<quint32>(i + src), data[i], E::BigEndian);
			QVERIFY(ok);
		}

		m_cp->runCommand(command);

		for (size_t i = 0; i < size; i++)
		{
			QCOMPARE(m_device->readRamWord(static_cast<quint32>(dst + i)), data[i]);
		}
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: movc
// Code: 6 - ext: 0b000000
//
void SimCommandTest_LM5_LM6::test_command_movc()
{
	const quint16 opCode = 6;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 data = 0x98FD;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.w2 = qToBigEndian(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movc(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), data);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: movc_acc
// Code: 6 - ext: 0b100000
//
void SimCommandTest_LM5_LM6::test_command_movc_acc()
{
	const quint16 opCode = 6;
	const quint16 opExt = 0b100000;
	const quint16 data = 0x98FD;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand((opCode << 6) | opExt);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((opCode << 6) | opExt); // CommandOpCode
	rc.w1 = qToBigEndian(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movc_acc(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_word0, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);

		QCOMPARE(m_device->acc(), data);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command movc, opcode 7
//
void SimCommandTest_LM5_LM6::test_command_movbc()
{
	const quint16 opCode = 7;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 data = 1;
	const quint16 bitNo = 5;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.w2 = qToBigEndian(data);
	rc.w3 = qToBigEndian(bitNo);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movbc(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, data);
		QCOMPARE(command.m_bitNo0, bitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeWord(dst, 0x0000, E::BigEndian);

		Q_UNUSED(ok)

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamBit(dst, bitNo), data);
		QCOMPARE(m_device->readRamBit(dst, bitNo - 1), 0);
		QCOMPARE(m_device->readRamBit(dst, bitNo + 1), 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command wrfb, opcode 8
//
void SimCommandTest_LM5_LM6::test_command_wrfb()
{
	const quint16 opcode = 8;
	const quint16 src = AppLogicWordDataOffset + 200u;
	const quint16 afbOpCode = 1;
	const quint16 afbInstance = 0;
	const quint16 afbPinOpCode = 1;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_wrfb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeWord(src, 0x123A, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		quint16 afbData =
			m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance)->param(command.m_afbPinOpCode)->wordValue();

		QCOMPARE(afbData, 0x123A);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command rdfb, opcode 9
//
void SimCommandTest_LM5_LM6::test_command_rdfb()
{
	const quint16 opcode = 9;
	const quint16 dst = AppLogicWordDataOffset + 15u;
	const quint16 afbOpCode = 1;
	const quint16 afbInstance = 0;
	const quint16 afbPinOpCode = 1;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian(dst);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_rdfb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, dst);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamWord(afbPinOpCode, 0x7624);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 0x7624);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command wrfbc, opcode 10
//
void SimCommandTest_LM5_LM6::test_command_wrfbc()
{
	const quint16 opCode = 10;
	const quint16 afbOpCode = 10;
	const quint16 afbInstance = 1;
	const quint16 afbPinOpCode = 1;
	const quint16 data = 0x59BA;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opCode << 6) | afbOpCode);
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_wrfbc(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);

		quint16 afbData =
			m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance)->param(command.m_afbPinOpCode)->wordValue();
		QCOMPARE(afbData, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command wrfbb, opcode 11
//
void SimCommandTest_LM5_LM6::test_command_wrfbb()
{
	const quint16 opcode = 11;
	const quint16 src = AppLogicWordDataOffset + 90u;
	const quint16 bitNo = 5;
	const quint16 afbOpCode = 4;
	const quint16 afbInstance = 99;
	const quint16 afbPinOpCode = 6;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian<quint16>(src);
	rc.w3 = qToBigEndian<quint16>(bitNo);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_wrfbb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, src);
		QCOMPARE(command.m_bitNo0, bitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// Write 1 and check
		//
		bool ok = m_device->mutableRam().writeWord(src, 1 << bitNo, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		quint16 afbData =
			m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance)->param(command.m_afbPinOpCode)->wordValue();
		QCOMPARE(afbData, 1);

		// Write 0 and check
		//
		ok = m_device->mutableRam().writeWord(src, 0, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		afbData = m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance)->param(command.m_afbPinOpCode)->wordValue();
		QCOMPARE(afbData, 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command rdfbb, opcode 12
//
void SimCommandTest_LM5_LM6::test_command_rdfbb()
{
	const quint16 opcode = 12;
	const quint16 dst = AppLogicWordDataOffset + 15u;
	const quint16 bitNo = 5;
	const quint16 afbOpCode = 4;
	const quint16 afbInstance = 12;
	const quint16 afbPinOpCode = 6;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian<quint16>(dst);
	rc.w3 = qToBigEndian<quint16>(bitNo);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_rdfbb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_bitNo0, bitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->writeRamWord(dst, 0);
		QVERIFY(ok);

		ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamWord(afbPinOpCode, 1);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 1 << bitNo);
		QCOMPARE(m_device->readRamBit(dst, bitNo), 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command rdfbcmp, opcode 13
//
void SimCommandTest_LM5_LM6::test_command_rdfbcmp()
{
	const quint16 opCode = 13;
	const quint16 afbOpCode = 4;
	const quint16 afbInstance = 11;
	const quint16 afbPinOpCode = 9;
	const quint16 data = 0x9871;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opCode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_rdfbcmp(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// Try equal
		//
		m_device->setFlagCmp(0);
		QCOMPARE(m_device->flagCmp(), static_cast<quint32>(0));

		bool ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamWord(afbPinOpCode, data);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->flagCmp(), static_cast<quint32>(1));

		// try not equal
		//
		ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamWord(afbPinOpCode, 0x0000);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->flagCmp(), static_cast<quint32>(0));
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}


// Test command setmem, opcode 14
//
void SimCommandTest_LM5_LM6::test_command_setmem()
{
	const quint16 opCode = 14;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 data = 0x9876;
	const quint16 size = 300;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(dst);
	rc.w2 = qToBigEndian<quint16>(data);
	rc.w3 = qToBigEndian<quint16>(size);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movmem(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, data);
		QCOMPARE(command.m_word2, size);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);

		for (size_t i = 0; i < size; i++)
		{
			QCOMPARE(m_device->readRamWord(static_cast<quint32>(dst + i)), data);
		}
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: movb
// Code: 15 - ext: 0b000000
//
void SimCommandTest_LM5_LM6::test_command_movb()
{
	const quint16 opCode = 15;
	const quint16 src = AppLogicWordDataOffset + 55u;
	const quint16 srcBitNo = 15;
	const quint16 dst = AppLogicWordDataOffset + 0u;
	const quint16 dstBitNo = 2;


	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(dst);
	rc.w2 = qToBigEndian<quint16>(src);
	rc.w3 = qToBigEndian<quint16>((dstBitNo << 8) | srcBitNo);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movb(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_word0, src);
		QCOMPARE(command.m_word1, dst);
		QCOMPARE(command.m_bitNo0, srcBitNo);
		QCOMPARE(command.m_bitNo1, dstBitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// 1
		//
		bool ok = m_device->mutableRam().writeWord(src, 1 << srcBitNo, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 1 << dstBitNo);

		// 0
		//
		ok = m_device->mutableRam().writeWord(src, 0, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 0);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: movb_acc_addr
// Code: 15 - ext: 0b100000
//
void SimCommandTest_LM5_LM6::test_command_movb_acc_addr()
{
	const quint16 opCode = 15;
	const quint16 opExt = 0b100000;
	const quint16 src = AppLogicWordDataOffset + 55u;
	const quint16 srcBitNo = 15;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand((opCode << 6) | opExt);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((opCode << 6) | opExt | srcBitNo); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movb_acc_addr(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_word0, src);
		QCOMPARE(command.m_bitNo0, srcBitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// --
		//
		bool ok = m_device->mutableRam().writeWord(src, 1 << srcBitNo, E::BigEndian);
		QVERIFY(ok);
		m_device->setAcc(0x8101);

		m_cp->runCommand(command);

		QCOMPARE(m_device->acc(), 0x0203);

		// --
		//
		ok = m_device->mutableRam().writeWord(src, 0, E::BigEndian);
		QVERIFY(ok);
		m_device->setAcc(0x8000);

		m_cp->runCommand(command);

		QCOMPARE(m_device->acc(), 0);

		// --
		//
		ok = m_device->mutableRam().writeWord(src, 1 << srcBitNo, E::BigEndian);
		QVERIFY(ok);
		m_device->setAcc(0x0000);

		m_cp->runCommand(command);

		QCOMPARE(m_device->acc(), 1);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command: movb_addr_acc
// Code: 15 - ext: 0b010000
//
void SimCommandTest_LM5_LM6::test_command_movb_addr_acc()
{
	const quint16 opCode = 15;
	const quint16 opExt = 0b010000;
	const quint16 dst = AppLogicWordDataOffset + 0u;
	const quint16 dstBitNo = 2;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand((opCode << 6) | opExt);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>((opCode << 6) | opExt | dstBitNo); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(dst);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movb_addr_acc(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_bitNo0, dstBitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		{
			quint16 acc = 0x81FE;

			m_device->setAcc(acc);
			m_device->mutableRam().writeWord(dst, 0xFFFF, E::BigEndian);

			m_cp->runCommand(command);

			QCOMPARE(m_device->readRamWord(dst), 0xFFFF & ~(1 << dstBitNo));
			QCOMPARE(m_device->acc(), acc >> 1);
		}

		{
			quint16 acc = 0xFFFF;

			m_device->setAcc(acc);
			m_device->mutableRam().writeWord(dst, 0x0000, E::BigEndian);

			m_cp->runCommand(command);

			QCOMPARE(m_device->readRamWord(dst), 1 << dstBitNo);
			QCOMPARE(m_device->acc(), acc >> 1);
		}


		{
			m_device->setAcc(0);
			m_device->mutableRam().writeWord(dst, 0, E::BigEndian);

			m_cp->runCommand(command);

			QCOMPARE(m_device->readRamWord(dst), 0);
			QCOMPARE(m_device->acc(), 0);
		}
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command appstart, opcode 17
//
void SimCommandTest_LM5_LM6::test_command_appstart()
{
	const quint16 opcode = 17;
	const quint16 addr = 0x1234;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opcode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(addr);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_appstart(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 2);
		QCOMPARE(command.m_word0, addr);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_device->setAppStartAddress(0);

		m_cp->runCommand(command);

		QCOMPARE(m_device->appStartAddress(), addr);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command mov32, opcode 18
//
void SimCommandTest_LM5_LM6::test_command_mov32()
{
	const quint16 opcode = 18;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 src = AppLogicWordDataOffset + 20;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opcode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.w2 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_mov32(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeDword(src, 0xEDAB1234, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamDword(dst), 0xEDAB1234);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command movc32, opcode 19
//
void SimCommandTest_LM5_LM6::test_command_movc32()
{
	const quint16 opcode = 19;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint32 data = 0x98FD;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opcode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.dw23 = qToBigEndian(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movc32(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_dword0, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamDword(dst), data);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command wrfb32, opcode 20
//
void SimCommandTest_LM5_LM6::test_command_wrfb32()
{
	const quint16 opcode = 20;
	const quint16 src = AppLogicWordDataOffset + 200u;
	const quint16 afbOpCode = 1;
	const quint16 afbInstance = 0;
	const quint16 afbPinOpCode = 1;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_wrfb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeDword(src, 0x123ABCAD, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		quint32 afbData =
			m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance)->param(command.m_afbPinOpCode)->dwordValue();
		QCOMPARE(afbData, static_cast<quint32>(0x123ABCAD));
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command rdfb, opcode 21
//
void SimCommandTest_LM5_LM6::test_command_rdfb32()
{
	const quint16 opcode = 21;
	const quint16 dst = AppLogicWordDataOffset + 15u;
	const quint16 afbOpCode = 4;
	const quint16 afbInstance = 7;
	const quint16 afbPinOpCode = 3;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opcode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.w2 = qToBigEndian(dst);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_rdfb32(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_word0, dst);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamDword(afbPinOpCode, 0x76241122);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamDword(dst), static_cast<quint32>(0x76241122));
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}


// Test command wrfbc32, opcode 22
//
void SimCommandTest_LM5_LM6::test_command_wrfbc32()
{
	const quint16 opCode = 22;
	const quint16 afbOpCode = 22;
	const quint16 afbInstance = 1;
	const quint16 afbPinOpCode = 1;
	const quint32 data = 0x59BA3214;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opCode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.dw23 = qToBigEndian<quint32>(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_wrfbc32(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_dword0, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		m_cp->runCommand(command);

		quint32 afbData =
			m_device->afbComponentInstance(command.m_afbOpCode, command.m_afbInstance)->param(command.m_afbPinOpCode)->dwordValue();
		QCOMPARE(afbData, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command rdfbcmp32, opcode 23
//
void SimCommandTest_LM5_LM6::test_command_rdfbcmp32()
{
	const quint16 opCode = 23;
	const quint16 afbOpCode = 4;
	const quint16 afbInstance = 11;
	const quint16 afbPinOpCode = 9;
	const quint32 data = 0x9871ABCD;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;

	rc.w0 = qToBigEndian<quint16>((opCode << 6) | afbOpCode); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>((afbInstance << 6) | afbPinOpCode);
	rc.dw23 = qToBigEndian<quint32>(data);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_rdfbcmp32(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_afbOpCode, afbOpCode);
		QCOMPARE(command.m_afbInstance, afbInstance);
		QCOMPARE(command.m_afbPinOpCode, afbPinOpCode);
		QCOMPARE(command.m_dword0, data);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// Try equal
		//
		m_device->setFlagCmp(0);
		QCOMPARE(m_device->flagCmp(), static_cast<quint32>(0));

		bool ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamDword(afbPinOpCode, data);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->flagCmp(), static_cast<quint32>(1));

		// try not equal
		//
		ok = m_device->afbComponentInstance(afbOpCode, afbInstance)->addParamDword(afbPinOpCode, 0x0000);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->flagCmp(), static_cast<quint32>(0));
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command movcmpf, opcode 24
//
void SimCommandTest_LM5_LM6::test_command_movcmpf()
{
	const quint16 opCode = 24;
	const quint16 dst = AppLogicWordDataOffset + 10u;
	const quint16 bitNo = 13;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(dst);
	rc.w2 = qToBigEndian<quint16>(bitNo);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_movcmpf(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_bitNo0, bitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// Mov cmp 0
		//
		m_device->setFlagCmp(0);

		bool ok = m_device->mutableRam().writeWord(dst, 0xFFFF, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), static_cast<quint16>(~(1 << bitNo)));

		// Mov cmp 1
		//
		m_device->setFlagCmp(1);

		ok = m_device->mutableRam().writeWord(dst, 0x0000, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 1 << bitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}


// Test command pmov, opcode 25
//
void SimCommandTest_LM5_LM6::test_command_pmov()
{
	const quint16 opcode = 25;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 src = AppLogicWordDataOffset + 20;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opcode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.w2 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_pmov(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeWord(src, 0xEDAB, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 0xEDAB);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command pmov32, opcode 26
//
void SimCommandTest_LM5_LM6::test_command_pmov32()
{
	const quint16 opcode = 26;
	const quint16 dst = AppLogicWordDataOffset + 10;
	const quint16 src = AppLogicWordDataOffset + 20;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opcode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opcode << 6); // CommandOpCode
	rc.w1 = qToBigEndian(dst);
	rc.w2 = qToBigEndian(src);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_pmov32(&command); // <<<<<<< PARSE
		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 3);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, src);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		bool ok = m_device->mutableRam().writeDword(src, 0xEDAC1234, E::BigEndian);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamDword(dst), 0xEDAC1234);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

// Test command fillb, opcode 27
//
void SimCommandTest_LM5_LM6::test_command_fillb()
{
	const quint16 opCode = 27;
	const quint16 dst = AppLogicWordDataOffset + 90u;
	const quint16 src = AppLogicWordDataOffset + 77u;
	const quint16 srcBitNo = 7;

	// Parse
	//
	LmCommand lmc = lmDescriptionCommand(opCode << 6);
	Sim::EmulatorCommand command{lmc};

	RawCommand rc;
	rc.w0 = qToBigEndian<quint16>(opCode << 6); // CommandOpCode
	rc.w1 = qToBigEndian<quint16>(dst);
	rc.w2 = qToBigEndian<quint16>(src);
	rc.w3 = qToBigEndian<quint16>(srcBitNo);

	m_device->m_plainAppLogic.setRawData(rc.all.data(), sizeof(rc));

	try
	{
		m_cp->parse_fillb(&command); // <<<<<<< PARSE

		m_cp->setCommandFuncPtr(&command);

		QCOMPARE(command.m_size, 4);
		QCOMPARE(command.m_word0, dst);
		QCOMPARE(command.m_word1, src);
		QCOMPARE(command.m_bitNo0, srcBitNo);
	}
	catch (...)
	{
		QFAIL("");
	}

	// Run
	//
	try
	{
		// Write 1
		//
		bool ok = m_device->writeRamWord(dst, 0x0000);
		QVERIFY(ok);

		ok = m_device->writeRamWord(src, 1 << srcBitNo);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 0xFFFF);

		// Write 0
		//
		ok = m_device->writeRamWord(src, 0x0000);
		QVERIFY(ok);

		m_cp->runCommand(command);

		QCOMPARE(m_device->readRamWord(dst), 0x0000);
	}
	catch (...)
	{
		QFAIL("");
	}

	QVERIFY(m_device->deviceState() != Sim::DeviceState::Fault);
	return;
}

LmCommand SimCommandTest_LM5_LM6::lmDescriptionCommand(int code)
{
	return m_device->lmDescription().command(code);
}
