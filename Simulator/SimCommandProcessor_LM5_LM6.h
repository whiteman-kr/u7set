#pragma once
#include <SimCommandProcessor.h>
#include <SimDeviceEmulator.h>

namespace Sim
{

	class CommandProcessor_LM5_LM6 : public CommandProcessor
	{
		Q_OBJECT

	public:
		explicit CommandProcessor_LM5_LM6(DeviceEmulator* device);
		virtual ~CommandProcessor_LM5_LM6();

	public:

		virtual void cacheCommands(std::vector<DeviceCommand>* commands) override;

		// Update platform interface, this function is called before work cyle,
		// to update such platform inteface signals as Blink.
		// Update mustb be done directly in RAM
		//
		virtual bool updatePlatformInterfaceState() override;

		virtual bool runCommand(const DeviceCommand& command) override;

	public slots:
		void command_not_implemented(const DeviceCommand& command);

		// Command: nop
		// Code: 1
		// Description: No operation
		//
		void parse_nop(DeviceCommand* command) const;
		void command_nop(const DeviceCommand& command);

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

		// Command: rdfb
		// Code: 9
		// Description: Read 16-bit word from AFB output and write to memory
		//
		void parse_rdfb(DeviceCommand* command) const;
		void command_rdfb(const DeviceCommand& command);

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

		// Command: rdfbcmp
		// Code: 13
		// Description: Read 16-bit data from AFB instance and compare it with constant, set compare bit if equal
		//
		void parse_rdfbcmp(DeviceCommand* command) const;
		void command_rdfbcmp(const DeviceCommand& command);

		// Command: setmem
		// Code: 14
		// Description: Set memory area to 16-bit word constant
		//
		void parse_setmem(DeviceCommand* command) const;
		void command_setmem(const DeviceCommand& command);

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

		// Command: mov32
		// Code: 18
		// Description: Move 32-bit data from RAM to RAM
		//
		void parse_mov32(DeviceCommand* command) const;
		void command_mov32(const DeviceCommand& command);

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

		// Command: rdfbcmp32
		// Code: 23
		// Description: Read 32-bit data from AFB instance and compare it with constant, set compare bit if equal
		//
		void parse_rdfbcmp32(DeviceCommand* command) const;
		void command_rdfbcmp32(const DeviceCommand& command);

		// Command: movcmpf
		// Code: 24
		// Description: Write compare flag to memory [flag result from rdfbcmp(32)]
		//
		void parse_movcmpf(DeviceCommand* command) const;
		void command_movcmpf(const DeviceCommand& command);

		// Command: pmov
		// Code: 25
		// Description: Copy 16-bit word from memory to memory written in prior cycle
		//
		void parse_pmov(DeviceCommand* command) const;
		void command_pmov(const DeviceCommand& command);

		// Command: pmov32
		// Code: 26
		// Description: Copy 32-bit data from memory to memory written in prior cycle
		//
		void parse_pmov32(DeviceCommand* command) const;
		void command_pmov32(const DeviceCommand& command);

		// Command: fillb
		// Code: 27
		// Description: Fill 16-bit word with 1-bit constant and write it to memory
		//
		void parse_fillb(DeviceCommand* command) const;
		void command_fillb(const DeviceCommand& command);

		//
		// AFB's simultaion code
		//

		//	LOGIC, OpCode 1
		//
		void afb_logic_v207(AfbComponentInstance* instance);

		//	NOT, OpCode 2
		//
		void afb_not_v103(AfbComponentInstance* instance);

		//	TCT, OpCode 3
		//
		void afb_tct_v208(AfbComponentInstance* instance);
		void afb_tct_v209(AfbComponentInstance* instance);
		void afb_tct_v210(AfbComponentInstance* instance);

		//	FLIP FLOP, OpCode 4
		//
		void afb_flipflop_v106(AfbComponentInstance* instance);

		//	CTUD, OpCode 5
		//  Counter Up/Down
		//
		void afb_ctud_v106(AfbComponentInstance* instance);

		//	MAJ, OpCode 6
		//  Majority Block
		//
		void afb_maj_v107(AfbComponentInstance* instance);

		//	SRSST, OpCode 7
		//  SimLock
		//
		void afb_srsst_v104(AfbComponentInstance* instance);

		//	BCOD, OpCode 8
		//
		void afb_bcod_v103(AfbComponentInstance* instance);
		void afb_bcod_v104(AfbComponentInstance* instance);

		//	BDEC, OpCode 9
		//
		void afb_bdec_v103(AfbComponentInstance* instance);

		//	BCOMP, OpCode 10
		//
		void afb_bcomp_v111(AfbComponentInstance* instance);

		//	DAMPER, OpCode 11
		//
		void afb_damper_v112(AfbComponentInstance* instance);

		//	MEM - Median and Extremum, OpCode 12
		//
		void afb_mem_v7(AfbComponentInstance* instance);

		//	MATH, OpCode 13
		//
		void afb_math_v104(AfbComponentInstance* instance);

		//	SCALE, OpCode 14
		//
		void afb_scale_v108(AfbComponentInstance* instance);

		//	FUNC, OpCode 16
		//
		void afb_func_v3(AfbComponentInstance* instance);

		//	INTEGRATOR, OpCode 17
		//
		void afb_int_v6_tiunlim(AfbComponentInstance* instance);
		void afb_int_v6_ti350000(AfbComponentInstance* instance);
		void afb_int_v6(AfbComponentInstance* instance, qint32 maxTiValue);

		//	DPCOMP, OpCode 20
		//
		void afb_dpcomp_v3(AfbComponentInstance* instance);
		void afb_dpcomp_v4(AfbComponentInstance* instance);
		void afb_dpcomp_v5(AfbComponentInstance* instance);

		//	MUX, OpCode 21
		//
		void afb_mux_v1(AfbComponentInstance* instance);

		//	LATCH, OpCode 22
		//
		void afb_latch_v4(AfbComponentInstance* instance);
		void afb_latch_v5(AfbComponentInstance* instance);

		//	LIM, OpCode 23
		//
		void afb_lim_v7(AfbComponentInstance* instance);

		//	DEADZONE, OpCode 24
		//
		void fb_deadzone_v5(AfbComponentInstance* instance);
		void fb_deadzone_v6(AfbComponentInstance* instance);

		//	POL, OpCode 25
		//
		void afb_pol_v3(AfbComponentInstance* instance);

		//	DER, OpCode 26
		//
		void afb_der_v5_tdunlim(AfbComponentInstance* instance);
		void afb_der_v5_td350000(AfbComponentInstance* instance);
		void afb_der_v5(AfbComponentInstance* instance, qint32 maxTdValue);

		// MISMATCH, OpCode 27
		// Analog Mismatch
		//
		void afb_mismatch_v2(AfbComponentInstance* instance);
		void afb_mismatch_v3(AfbComponentInstance* instance);
		void afb_mismatch_v4(AfbComponentInstance* instance);

		void afb_mismatch_impl(AfbComponentInstance* instance, int version);
		void afb_mismatch_impl_si(AfbComponentInstance* instance, int version);
		void afb_mismatch_impl_fp(AfbComponentInstance* instance, int version);

		// TCONV, OpCode 28
		// Analog Conversion
		//
		void afb_tconv_v0(AfbComponentInstance* instance);

		// INDICATION, OpCode 29
		//
		void afb_indication_v1(AfbComponentInstance* instance);

		// PULSE_GEN, OpCode 30
		//
		void afb_pulse_gen_v0(AfbComponentInstance* instance);

		// --
		//
	private:
		using SimCommandFunc = void (CommandProcessor_LM5_LM6::*)(const DeviceCommand&);
		using SimCommandFuncCast = u_ptm_cast<CommandProcessor_LM5_LM6, void(const DeviceCommand&)>;

		using SimAfbFunc = void (CommandProcessor_LM5_LM6::*)(AfbComponentInstance*);
		using SimAfbFuncCast = u_ptm_cast<CommandProcessor_LM5_LM6, void(AfbComponentInstance*)>;

		const std::map<QString, SimCommandFunc> m_nameToFuncCommand
		{
			{QStringLiteral("command_nop"),			&CommandProcessor_LM5_LM6::command_nop},				// 1
			{QStringLiteral("command_startafb"),	&CommandProcessor_LM5_LM6::command_startafb},			// 2
			{QStringLiteral("command_stop"),		&CommandProcessor_LM5_LM6::command_stop},				// 3
			{QStringLiteral("command_mov"),			&CommandProcessor_LM5_LM6::command_mov},				// 4
			{QStringLiteral("command_movmem"),		&CommandProcessor_LM5_LM6::command_movmem},				// 5
			{QStringLiteral("command_movc"),		&CommandProcessor_LM5_LM6::command_movc},				// 6
			{QStringLiteral("command_movbc"),		&CommandProcessor_LM5_LM6::command_movbc},				// 7
			{QStringLiteral("command_wrfb"),		&CommandProcessor_LM5_LM6::command_wrfb},				// 8
			{QStringLiteral("command_rdfb"),		&CommandProcessor_LM5_LM6::command_rdfb},				// 9
			{QStringLiteral("command_wrfbc"),		&CommandProcessor_LM5_LM6::command_wrfbc},				// 10
			{QStringLiteral("command_wrfbb"),		&CommandProcessor_LM5_LM6::command_wrfbb},				// 11
			{QStringLiteral("command_rdfbb"),		&CommandProcessor_LM5_LM6::command_rdfbb},				// 12
			{QStringLiteral("command_rdfbcmp"),		&CommandProcessor_LM5_LM6::command_rdfbcmp},			// 13
			{QStringLiteral("command_setmem"),		&CommandProcessor_LM5_LM6::command_setmem},				// 14
			{QStringLiteral("command_movb"),		&CommandProcessor_LM5_LM6::command_movb},				// 15
			{QStringLiteral("command_nstartafb"),	&CommandProcessor_LM5_LM6::command_not_implemented},	// 16 Not supported?
			{QStringLiteral("command_appstart"),	&CommandProcessor_LM5_LM6::command_appstart},			// 17
			{QStringLiteral("command_mov32"),		&CommandProcessor_LM5_LM6::command_mov32},				// 18
			{QStringLiteral("command_movc32"),		&CommandProcessor_LM5_LM6::command_movc32},				// 19
			{QStringLiteral("command_wrfb32"),		&CommandProcessor_LM5_LM6::command_wrfb32},				// 20
			{QStringLiteral("command_rdfb32"),		&CommandProcessor_LM5_LM6::command_rdfb32},				// 21
			{QStringLiteral("command_wrfbc32"),		&CommandProcessor_LM5_LM6::command_wrfbc32},			// 22
			{QStringLiteral("command_rdfbcmp32"),	&CommandProcessor_LM5_LM6::command_rdfbcmp32},			// 23
			{QStringLiteral("command_movcmpf"),		&CommandProcessor_LM5_LM6::command_movcmpf},			// 24
			{QStringLiteral("command_pmov"),		&CommandProcessor_LM5_LM6::command_pmov},				// 25
			{QStringLiteral("command_pmov32"),		&CommandProcessor_LM5_LM6::command_pmov32},				// 26
			{QStringLiteral("command_fillb"),		&CommandProcessor_LM5_LM6::command_fillb},				// 27
		};


		const std::map<QString, SimAfbFunc> m_nameToFuncAfb
		{
			{QStringLiteral("afb_logic_v207"),		&CommandProcessor_LM5_LM6::afb_logic_v207},				// 1
			{QStringLiteral("afb_not_v103"),		&CommandProcessor_LM5_LM6::afb_not_v103},				// 2
			{QStringLiteral("afb_tct_v208"),		&CommandProcessor_LM5_LM6::afb_tct_v208},				// 3
			{QStringLiteral("afb_tct_v209"),		&CommandProcessor_LM5_LM6::afb_tct_v209},				// 3
			{QStringLiteral("afb_tct_v210"),		&CommandProcessor_LM5_LM6::afb_tct_v210},				// 3
			{QStringLiteral("afb_flipflop_v106"),	&CommandProcessor_LM5_LM6::afb_flipflop_v106},			// 4
			{QStringLiteral("afb_ctud_v106"),		&CommandProcessor_LM5_LM6::afb_ctud_v106},				// 5
			{QStringLiteral("afb_maj_v107"),		&CommandProcessor_LM5_LM6::afb_maj_v107},				// 6
			{QStringLiteral("afb_srsst_v104"),		&CommandProcessor_LM5_LM6::afb_srsst_v104},				// 7
			{QStringLiteral("afb_bcod_v103"),		&CommandProcessor_LM5_LM6::afb_bcod_v103},				// 8
			{QStringLiteral("afb_bcod_v104"),		&CommandProcessor_LM5_LM6::afb_bcod_v104},				// 8
			{QStringLiteral("afb_bdec_v103"),		&CommandProcessor_LM5_LM6::afb_bdec_v103},				// 9
			{QStringLiteral("afb_bcomp_v111"),		&CommandProcessor_LM5_LM6::afb_bcomp_v111},				// 10
			{QStringLiteral("afb_damper_v112"),		&CommandProcessor_LM5_LM6::afb_damper_v112},			// 11
			{QStringLiteral("afb_mem_v7"),			&CommandProcessor_LM5_LM6::afb_mem_v7},					// 12
			{QStringLiteral("afb_math_v104"),		&CommandProcessor_LM5_LM6::afb_math_v104},				// 13
			{QStringLiteral("afb_scale_v108"),		&CommandProcessor_LM5_LM6::afb_scale_v108},				// 14
			{QStringLiteral("afb_func_v3"),			&CommandProcessor_LM5_LM6::afb_func_v3},				// 16
			{QStringLiteral("afb_int_v6_tiunlim"),	&CommandProcessor_LM5_LM6::afb_int_v6_tiunlim},			// 17	ti is unlimited
			{QStringLiteral("afb_int_v6_ti350000"),	&CommandProcessor_LM5_LM6::afb_int_v6_ti350000},		// 17	ti is limited to 350000ms
			{QStringLiteral("afb_dpcomp_v3"),		&CommandProcessor_LM5_LM6::afb_dpcomp_v3},				// 20
			{QStringLiteral("afb_dpcomp_v4"),		&CommandProcessor_LM5_LM6::afb_dpcomp_v4},				// 20
			{QStringLiteral("afb_dpcomp_v5"),		&CommandProcessor_LM5_LM6::afb_dpcomp_v5},				// 20
			{QStringLiteral("afb_mux_v1"),			&CommandProcessor_LM5_LM6::afb_mux_v1},					// 21
			{QStringLiteral("afb_latch_v4"),		&CommandProcessor_LM5_LM6::afb_latch_v4},				// 22
			{QStringLiteral("afb_latch_v5"),		&CommandProcessor_LM5_LM6::afb_latch_v5},				// 22
			{QStringLiteral("afb_lim_v7"),			&CommandProcessor_LM5_LM6::afb_lim_v7},					// 23
			{QStringLiteral("fb_deadzone_v5"),		&CommandProcessor_LM5_LM6::fb_deadzone_v5},				// 24
			{QStringLiteral("fb_deadzone_v6"),		&CommandProcessor_LM5_LM6::fb_deadzone_v6},				// 24
			{QStringLiteral("afb_pol_v3"),			&CommandProcessor_LM5_LM6::afb_pol_v3},					// 25
			{QStringLiteral("afb_der_v5_tdunlim"),	&CommandProcessor_LM5_LM6::afb_der_v5_tdunlim},			// 26	td is unlimited
			{QStringLiteral("afb_der_v5_td350000"),	&CommandProcessor_LM5_LM6::afb_der_v5_td350000},		// 26	td is limited to 350000ms
			{QStringLiteral("afb_mismatch_v2"),		&CommandProcessor_LM5_LM6::afb_mismatch_v2},			// 27
			{QStringLiteral("afb_mismatch_v3"),		&CommandProcessor_LM5_LM6::afb_mismatch_v3},			// 27
			{QStringLiteral("afb_mismatch_v4"),		&CommandProcessor_LM5_LM6::afb_mismatch_v4},			// 27
			{QStringLiteral("afb_tconv_v0"),		&CommandProcessor_LM5_LM6::afb_tconv_v0},				// 28
			{QStringLiteral("afb_indication_v1"),	&CommandProcessor_LM5_LM6::afb_indication_v1},			// 29
			{QStringLiteral("afb_pulse_gen_v0"),	&CommandProcessor_LM5_LM6::afb_pulse_gen_v0},			// 30
		};

		static const int m_cycleDurationMs = 5;
		qint64 m_blinkCounter = 0;
	};

}

