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

		// Command: startafb
		// Code: 2
		// Description: Execute AFB
		//
		bool parse_startafb(DeviceCommand* command) const;
		bool command_startafb(const DeviceCommand& command);

		// Command: stop
		// Code: 3
		// Description: Stop IDR phase and start ALP, if ALP is current phase then stop work cycle
		//
		bool parse_stop(DeviceCommand* command) const;
		bool command_stop(const DeviceCommand& command);

		// Command: movmem
		// Code: 5
		// Description: Move N words from RAM to RAM
		//
		bool parse_movmem(DeviceCommand* command) const;
		bool command_movmem(const DeviceCommand& command);

		// Command: movc
		// Code: 6
		// Description: Write word const to RAM
		//
		bool parse_movc(DeviceCommand* command) const;
		bool command_movc(const DeviceCommand& command);

		// Command: movbc
		// Code: 7
		// Description: Write constant bit to RAM
		//
		bool parse_movbc(DeviceCommand* command) const;
		bool command_movbc(const DeviceCommand& command);

		// Command: wrfbc
		// Code: 10
		// Description: Write constant word to AFB input
		//
		bool parse_wrfbc(DeviceCommand* command) const;
		bool command_wrfbc(const DeviceCommand& command);

		// Command: wrfbb
		// Code: 11
		// Description: Read bit from RAM and write it to AFB
		//
		bool parse_wrfbb(DeviceCommand* command) const;
		bool command_wrfbb(const DeviceCommand& command);

		// Command: rdfbb
		// Code: 12
		// Description: Read bit from AFB and write it to RAM
		//
		bool parse_rdfbb(DeviceCommand* command) const;
		bool command_rdfbb(const DeviceCommand& command);

		// Command: appstart
		// Code: 17
		// Description: Save ALP phase start address
		//
		bool parse_appstart(DeviceCommand* command) const;
		bool command_appstart(const DeviceCommand& command);

		// Command: rdfb32
		// Code: 21
		// Description: Read 32bit data from AFB output and write it to RAM
		//
		bool parse_rdfb32(DeviceCommand* command) const;
		bool command_rdfb32(const DeviceCommand& command);

		// Command: wrfbc32
		// Code: 22
		// Description: Write 32bit constant to FunctionalBlock input
		//
		bool parse_wrfbc32(DeviceCommand* command) const;
		bool command_wrfbc32(const DeviceCommand& command);

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

	private:
		using SimCommandFunc = std::function<bool(CommandProcessor_LM1_SF00*, const DeviceCommand&)>;

		static const QHash<QString, SimCommandFunc> m_nameToFuncCommand;
	};

}

#endif // SIMCOMMANDPROCESSOR_LM_SF00_H
