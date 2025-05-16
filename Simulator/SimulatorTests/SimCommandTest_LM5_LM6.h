#pragma once

#include <QObject>
#include <HardwareLib/LmDescription.h>

namespace Sim
{
	class DeviceEmulator;
	class CommandProcessor_LM5_LM6;
}

class SimCommandTest_LM5_LM6 : public QObject
{
	Q_OBJECT

public:
	SimCommandTest_LM5_LM6();
	~SimCommandTest_LM5_LM6() override;

private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void test_command_nop();			// Code: 1 - ext: 0b000000
	void test_command_reset();			// Code: 1 - ext: 0b100000
	void test_command_set();			// Code: 1 - ext: 0b010000
	void test_command_or();				// Code: 1 - ext: 0b110000
	void test_command_and();			// Code: 1 - ext: 0b001000
	void test_command_not();			// Code: 1 - ext: 0b101000
	void test_command_lshift0();		// Code: 1 - ext: 0b011000
	void test_command_lshift1();		// Code: 1 - ext: 0b111000
	void test_command_startafb();		// 2
	void test_command_stop();			// 3
	void test_command_mov();			// Code: 4 - ext: 0b000000
	void test_command_mov_addr_acc();	// Code: 4 - ext: 0b100000
	void test_command_mov_acc_addr();	// Code: 4 - ext: 0b010000
	void test_command_movmem();			// 5
	void test_command_movc();			// Code: 6 - ext: 0xb000000
	void test_command_movc_acc();		// Code: 6 - ext: 0xb100000
	void test_command_movbc();			// 7
	void test_command_wrfb();			// 8
	void test_command_rdfb();			// 9
	void test_command_wrfbc();			// 10
	void test_command_wrfbb();			// 11
	void test_command_rdfbb();			// 12
	void test_command_rdfbcmp();		// 13
	void test_command_setmem();			// 14
	void test_command_movb();			// Code: 15 - ext: 0xb000000
	void test_command_movb_acc_addr();	// Code: 15 - ext: 0xb100000
	void test_command_movb_addr_acc();	// Code: 15 - ext: 0xb010000
	void test_command_appstart();		// 17
	void test_command_mov32();			// 18
	void test_command_movc32();			// 19
	void test_command_wrfb32();			// 20
	void test_command_rdfb32();			// 21
	void test_command_wrfbc32();		// 22
	void test_command_rdfbcmp32();		// 23
	void test_command_movcmpf();		// 24
	void test_command_pmov();			// 25
	void test_command_pmov32();			// 26
	void test_command_fillb();			// 27

private:
	LmCommand lmDescriptionCommand(int code);

private:
	std::unique_ptr<Sim::DeviceEmulator> m_device;
	std::unique_ptr<Sim::CommandProcessor_LM5_LM6> m_cp;

	const quint16 AppLogicWordDataOffset = 53956;
};


union RawCommand
{
	struct
	{
		quint16 w0;
		quint16 w1;
		union
		{
			struct
			{
				quint16 w2;
				quint16 w3;
			};
			quint32 dw23;
		};
	};
	std::array<char, 4 * 2> all = {0, 0, 0, 0, 0, 0, 0, 0};
};
