#pragma once

#include <memory>
#include <QObject>

#include <SimCommandProcessor_LM5_LM6.h>

class SimCommandTest_LM5_LM6 : public QObject
{
	Q_OBJECT

public:
	SimCommandTest_LM5_LM6();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void testCommandNop();			// 1
	void testCommandStartAfb();		// 2
	void testCommandStop();			// 3
	void testCommandMov();			// 4
	void testCommandMovMem();		// 5
	void testCommandMovc();			// 6
	void testCommandMovbc();		// 7
	void testCommandWrfb();			// 8
	void testCommandRdfb();			// 9
	void testCommandWrfbc();		// 10
	void testCommandWrfbb();		// 11
	void testCommandRdfbb();		// 12
	void testCommandRdfbCmp();		// 13
	void testCommandSetMem();		// 14
	void testCommandMovb();			// 15
	void testCommandAppStart();		// 17
	void testCommandMov32();		// 18
	void testCommandMovc32();		// 19
	void testCommandWrfb32();		// 20
	void testCommandRdfb32();		// 21
	void testCommandWrfbc32();		// 22
	void testCommandRdfbCmp32();	// 23
	void testCommandMovCmpf();		// 24
	void testCommandPmov();			// 25
	void testCommandPmov32();		// 26
	void testCommandFillb();		// 27

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
