#ifndef SIMCOMMANDPROCESSOR_LM1_SF00_H
#define SIMCOMMANDPROCESSOR_LM1_SF00_H
#include <SimCommandProcessor.h>
#include <SimDeviceEmulator.h>

namespace Sim
{

	class CommandProcessor_LM1_SF00 : public CommandProcessor
	{
		Q_OBJECT

	public:
		explicit CommandProcessor_LM1_SF00(DeviceEmulator* device);
		virtual ~CommandProcessor_LM1_SF00();

		virtual bool runCommand(const DeviceCommand& command) override;

	public slots:
		void command_not_implemented(const DeviceCommand& command);

		// Command: startafb
		// Code: 2
		// Description: Execute AFB
		//
		void parse_startafb(DeviceCommand* command) const;
		void command_startafb(const DeviceCommand& command);

		// Command: stop
		// Code: 3
		// Description: Stop IDR phase and start ALP, if ALP is current phase then stop work cycle
		//
		void parse_stop(DeviceCommand* command) const;
		void command_stop(const DeviceCommand& command);

		// Command: mov
		// Code: 4
		// Description: Move word from RAM to RAM
		//
		void parse_mov(DeviceCommand* command) const;
		void command_mov(const DeviceCommand& command);

		// Command: movmem
		// Code: 5
		// Description: Move N words from RAM to RAM
		//
		void parse_movmem(DeviceCommand* command) const;
		void command_movmem(const DeviceCommand& command);

		// Command: movc
		// Code: 6
		// Description: Write word const to RAM
		//
		void parse_movc(DeviceCommand* command) const;
		void command_movc(const DeviceCommand& command);

		// Command: movbc
		// Code: 7
		// Description: Write constant bit to RAM
		//
		void parse_movbc(DeviceCommand* command) const;
		void command_movbc(const DeviceCommand& command);

		// Command: wrfb
		// Code: 8
		// Description: Read 16bit data from RAM and write to AFB input
		//
		void parse_wrfb(DeviceCommand* command) const;
		void command_wrfb(const DeviceCommand& command);

		// Command: wrfbc
		// Code: 10
		// Description: Write constant word to AFB input
		//
		void parse_wrfbc(DeviceCommand* command) const;
		void command_wrfbc(const DeviceCommand& command);

		// Command: wrfbb
		// Code: 11
		// Description: Read bit from RAM and write it to AFB
		//
		void parse_wrfbb(DeviceCommand* command) const;
		void command_wrfbb(const DeviceCommand& command);

		// Command: rdfbb
		// Code: 12
		// Description: Read bit from AFB and write it to RAM
		//
		void parse_rdfbb(DeviceCommand* command) const;
		void command_rdfbb(const DeviceCommand& command);

		// Command: movb
		// Code: 15
		// Description: Move bit from RAM to RAM
		//
		void parse_movb(DeviceCommand* command) const;
		void command_movb(const DeviceCommand& command);

		// Command: appstart
		// Code: 17
		// Description: Save ALP phase start address
		//
		void parse_appstart(DeviceCommand* command) const;
		void command_appstart(const DeviceCommand& command);

		// Command: movc32
		// Code: 19
		// Description: Move 32bit constant to RAM
		//
		void parse_movc32(DeviceCommand* command) const;
		void command_movc32(const DeviceCommand& command);

		// Command: wrfb32
		// Code: 20
		// Description: Read 32bit data from RAM and write to AFB input
		//
		void parse_wrfb32(DeviceCommand* command) const;
		void command_wrfb32(const DeviceCommand& command);

		// Command: rdfb32
		// Code: 21
		// Description: Read 32bit data from AFB output and write it to RAM
		//
		void parse_rdfb32(DeviceCommand* command) const;
		void command_rdfb32(const DeviceCommand& command);

		// Command: wrfbc32
		// Code: 22
		// Description: Write 32bit constant to FunctionalBlock input
		//
		void parse_wrfbc32(DeviceCommand* command) const;
		void command_wrfbc32(const DeviceCommand& command);

		// Command: pmov32
		// Code: 26
		// Description: Write 32bit constant to FunctionalBlock input
		//
		void parse_pmov32(DeviceCommand* command) const;
		void command_pmov32(const DeviceCommand& command);

		//
		// AFB's simultaion code
		//

		//	LOGIC, OpCode 1
		//
		void afb_logic(AfbComponentInstance* instance);

		//	NOT, OpCode 2
		//
		void afb_not(AfbComponentInstance* instance);

		//	TCT, OpCode 3
		//
		void afb_tct(AfbComponentInstance* instance);

		//	CTUD, OpCode 5
		//  Counter Up/Down
		//
		void afb_ctud(AfbComponentInstance* instance);

		//	BCOD, OpCode 8
		//
		void afb_bcod(AfbComponentInstance* instance);

		//	BDEC, OpCode 9
		//
		void afb_bdec(AfbComponentInstance* instance);

		//	BCOMP, OpCode 10
		//
		void afb_bcomp(AfbComponentInstance* instance);

		//	DAMPER, OpCode 11
		//
		void afb_damper(AfbComponentInstance* instance);

		//	MATH, OpCode 13
		//
		void afb_math(AfbComponentInstance* instance);

		//	SCALE, OpCode 14
		//
		void afb_scale(AfbComponentInstance* instance);

		//	DPCOMP, OpCode 20
		//
		void afb_dpcomp(AfbComponentInstance* instance);

		//	MUX, OpCode 21
		//
		void afb_mux(AfbComponentInstance* instance);

		//	LIM, OpCode 23
		//
		void afb_lim(AfbComponentInstance* instance);

	private:
		using SimCommandFunc = void(CommandProcessor_LM1_SF00::*)(const DeviceCommand&);
		using SimAfbFunc = void(CommandProcessor_LM1_SF00::*)(AfbComponentInstance*);

		const std::map<Hash, SimCommandFunc> m_nameToFuncCommand
		{
			{::calcHash(QStringLiteral("command_nop")),			&CommandProcessor_LM1_SF00::command_not_implemented},	// 1 Not impemented
			{::calcHash(QStringLiteral("command_startafb")),	&CommandProcessor_LM1_SF00::command_startafb},			// 2
			{::calcHash(QStringLiteral("command_stop")),		&CommandProcessor_LM1_SF00::command_stop},				// 3
			{::calcHash(QStringLiteral("command_mov")),			&CommandProcessor_LM1_SF00::command_mov},				// 4
			{::calcHash(QStringLiteral("command_movmem")),		&CommandProcessor_LM1_SF00::command_movmem},			// 5
			{::calcHash(QStringLiteral("command_movc")),		&CommandProcessor_LM1_SF00::command_movc},				// 6
			{::calcHash(QStringLiteral("command_movbc")),		&CommandProcessor_LM1_SF00::command_movbc},				// 7
			{::calcHash(QStringLiteral("command_wrfb")),		&CommandProcessor_LM1_SF00::command_wrfb},				// 8
			{::calcHash(QStringLiteral("command_rdfb")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 9 Not impemented
			{::calcHash(QStringLiteral("command_wrfbc")),		&CommandProcessor_LM1_SF00::command_wrfbc},				// 10
			{::calcHash(QStringLiteral("command_wrfbb")),		&CommandProcessor_LM1_SF00::command_wrfbb},				// 11
			{::calcHash(QStringLiteral("command_rdfbb")),		&CommandProcessor_LM1_SF00::command_rdfbb},				// 12
			{::calcHash(QStringLiteral("command_rdfbts")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 13 Not impemented
			{::calcHash(QStringLiteral("command_setmem")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 14 Not impemented
			{::calcHash(QStringLiteral("command_movb")),		&CommandProcessor_LM1_SF00::command_movb},				// 15
			{::calcHash(QStringLiteral("command_nstartafb")),	&CommandProcessor_LM1_SF00::command_not_implemented},	// 16 Not supported?
			{::calcHash(QStringLiteral("command_appstart")),	&CommandProcessor_LM1_SF00::command_appstart},			// 17
			{::calcHash(QStringLiteral("command_mov32")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 18 Not impemented
			{::calcHash(QStringLiteral("command_movc32")),		&CommandProcessor_LM1_SF00::command_movc32},			// 19
			{::calcHash(QStringLiteral("command_wrfb32")),		&CommandProcessor_LM1_SF00::command_wrfb32},			// 20
			{::calcHash(QStringLiteral("command_rdfb32")),		&CommandProcessor_LM1_SF00::command_rdfb32},			// 21
			{::calcHash(QStringLiteral("command_wrfbc32")),		&CommandProcessor_LM1_SF00::command_wrfbc32},			// 22
			{::calcHash(QStringLiteral("command_rdfbts32")),	&CommandProcessor_LM1_SF00::command_not_implemented},	// 23 Not impemented
			{::calcHash(QStringLiteral("command_movcf")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 24 Not impemented
			{::calcHash(QStringLiteral("command_pmov")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 25 Not impemented
			{::calcHash(QStringLiteral("command_pmov32")),		&CommandProcessor_LM1_SF00::command_pmov32},			// 26
			{::calcHash(QStringLiteral("command_fillb")),		&CommandProcessor_LM1_SF00::command_not_implemented},	// 27 Not impemented
		};


		const std::map<Hash, SimAfbFunc> m_nameToFuncAfb
		{
			{::calcHash(QStringLiteral("afb_logic")),			&CommandProcessor_LM1_SF00::afb_logic},					// 1
			{::calcHash(QStringLiteral("afb_not")),				&CommandProcessor_LM1_SF00::afb_not},					// 2
			{::calcHash(QStringLiteral("afb_tct")),				&CommandProcessor_LM1_SF00::afb_tct},					// 3
			{::calcHash(QStringLiteral("afb_ctud")),			&CommandProcessor_LM1_SF00::afb_ctud},					// 5
			{::calcHash(QStringLiteral("afb_bcod")),			&CommandProcessor_LM1_SF00::afb_bcod},					// 8
			{::calcHash(QStringLiteral("afb_bdec")),			&CommandProcessor_LM1_SF00::afb_bdec},					// 9
			{::calcHash(QStringLiteral("afb_bcomp")),			&CommandProcessor_LM1_SF00::afb_bcomp},					// 10
			{::calcHash(QStringLiteral("afb_damper")),			&CommandProcessor_LM1_SF00::afb_damper},				// 11
			{::calcHash(QStringLiteral("afb_math")),			&CommandProcessor_LM1_SF00::afb_math},					// 13
			{::calcHash(QStringLiteral("afb_scale")),			&CommandProcessor_LM1_SF00::afb_scale},					// 14
			{::calcHash(QStringLiteral("afb_dpcomp")),			&CommandProcessor_LM1_SF00::afb_dpcomp},				// 20
			{::calcHash(QStringLiteral("afb_mux")),				&CommandProcessor_LM1_SF00::afb_mux},					// 21
			{::calcHash(QStringLiteral("afb_lim")),				&CommandProcessor_LM1_SF00::afb_lim},					// 23
		};

		static const int m_cycleDurationMs = 5;
	};

}

#endif // SIMCOMMANDPROCESSOR_LM_SF00_H
