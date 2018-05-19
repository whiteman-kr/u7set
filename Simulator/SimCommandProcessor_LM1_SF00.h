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

		//
		// AFB's simultaion code
		//

		//	LOGIC, OpCode 1
		//
		void afb_logic(AfbComponentInstance* instance);

		//	NOT, OpCode 2
		//
		void afb_not(AfbComponentInstance* instance);

		//	MATH, OpCode 13
		//
		void afb_math(AfbComponentInstance* instance);

		//	SCALE, OpCode 13
		//
		void afb_scale(AfbComponentInstance* instance);

		//	LIM, OpCode 23
		//
		void afb_lim(AfbComponentInstance* instance);

	private:
		using SimCommandFunc = void(CommandProcessor_LM1_SF00::*)(const DeviceCommand&);
		using SimAfbFunc = void(CommandProcessor_LM1_SF00::*)(AfbComponentInstance*);

		const QHash<QString, SimCommandFunc> m_nameToFuncCommand
		{
			{"command_nop",			&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_startafb",	&CommandProcessor_LM1_SF00::command_startafb},
			{"command_stop",		&CommandProcessor_LM1_SF00::command_stop},
			{"command_mov",			&CommandProcessor_LM1_SF00::command_mov},
			{"command_movmem",		&CommandProcessor_LM1_SF00::command_movmem},
			{"command_movc",		&CommandProcessor_LM1_SF00::command_movc},
			{"command_movbc",		&CommandProcessor_LM1_SF00::command_movbc},
			{"command_wrfb",		&CommandProcessor_LM1_SF00::command_wrfb},
			{"command_rdfb",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_wrfbc",		&CommandProcessor_LM1_SF00::command_wrfbc},
			{"command_wrfbb",		&CommandProcessor_LM1_SF00::command_wrfbb},
			{"command_rdfbb",		&CommandProcessor_LM1_SF00::command_rdfbb},
			{"command_rdfbts",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_setmem",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_movb",		&CommandProcessor_LM1_SF00::command_movb},
			{"command_appstart",	&CommandProcessor_LM1_SF00::command_appstart},
			{"command_mov32",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_movc32",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_rdfb32",		&CommandProcessor_LM1_SF00::command_rdfb32},
			{"command_wrfbc32",		&CommandProcessor_LM1_SF00::command_wrfbc32},
			{"command_rdfbts32",	&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_movcf",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_pmov",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_pmov32",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
			{"command_fillb",		&CommandProcessor_LM1_SF00::command_not_implemented},	// !
		};

		const QHash<QString, SimAfbFunc> m_nameToFuncAfb
		{
			{"afb_logic",		&CommandProcessor_LM1_SF00::afb_logic},			// 1
			{"afb_not",			&CommandProcessor_LM1_SF00::afb_not},			// 2
			{"afb_math",		&CommandProcessor_LM1_SF00::afb_math},			// 13
			{"afb_scale",		&CommandProcessor_LM1_SF00::afb_scale},			// 14
			{"afb_lim",			&CommandProcessor_LM1_SF00::afb_lim},			// 14
		};
	};

}

#endif // SIMCOMMANDPROCESSOR_LM_SF00_H
