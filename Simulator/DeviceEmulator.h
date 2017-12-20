#ifndef DEVICEEMULATOR_H
#define DEVICEEMULATOR_H

#include <memory>
#include <functional>
#include <map>
#include <QObject>
#include <QTextStream>
#include <QTimerEvent>
#include <QJSEngine>
#include "../lib/LmDescription.h"
#include "../lib/ModuleFirmware.h"
#include "Output.h"
#include "Eeprom.h"
#include "Ram.h"
#include "Component.h"

#ifndef __FUNCTION_NAME__
	#ifdef WIN32   //WINDOWS
		#define __FUNCTION_NAME__   __FUNCTION__
	#else          //*NIX
		#define __FUNCTION_NAME__   __func__
	#endif
#endif

#define FAULT(message) fault(message, __FUNCTION_NAME__);

namespace Sim
{
	enum class DeviceMode
	{
		Start,
		Fault,
		LoadEeprom,
		Operate
	};

	enum class LmCommandCode		// The same enum is in compiler, move it to lib in future
	{
		NoCommand = 0,
		NOP = 1,
		START = 2,
		STOP = 3,
		MOV = 4,
		MOVMEM = 5,
		MOVC = 6,
		MOVBC = 7,
		WRFB = 8,
		RDFB = 9,
		WRFBC = 10,
		WRFBB = 11,
		RDFBB = 12,
		RDFBTS = 13,
		SETMEM = 14,
		MOVB = 15,
		NSTART = 16,
		APPSTART = 17,
		MOV32 = 18,
		MOVC32 = 19,
		WRFB32 = 20,
		RDFB32 = 21,
		WRFBC32 = 22,
		RDFBTS32 = 23,
		MOVCF = 24,
		PMOV = 25,
		PMOV32 = 26,
		FILLB = 27,
	};

	enum class CyclePhase
	{
		IdrPhase,
		AlpPhase,
		ODT,
		ST
	};

	struct LogicUnitData
	{
		int programCounter = 0;					// current offeset of program memory, in words
		CyclePhase phase = CyclePhase::IdrPhase;
		quint16 appStartAddress = 0xFFFF;
	};


	class DeviceEmulator : public QObject, protected Output
	{
		Q_OBJECT

	public:
		explicit DeviceEmulator(const Output& output);
		virtual ~DeviceEmulator();

		bool init(const Hardware::LogicModuleInfo& logicModuleInfo,
				  const LmDescription& lmDescription,
				  const Eeprom& tuningEeprom,
				  const Eeprom& confEeprom,
				  const Eeprom& appLogicEeprom,
				  const QString& simulationScript);

	private:
		bool initMemory();
		bool initEeprom();
		bool parseAppLogicCode();

	public slots:
		void pause();
		void start(int cycles);

	private:
		void fault(QString reasone, QString func);

	private:
		virtual void timerEvent(QTimerEvent* event) override;

		bool processStartMode();
		bool processFaultMode();
		bool processLoadEeprom();
		bool processOperate();

		bool runCommand(LmCommandCode commandCode);

		bool command_nop();			// 1
		bool command_startafb();	// 2
		bool command_stop();		// 3
		bool command_mov();			// 4
		bool command_movmem();		// 5
		bool command_movc();		// 6
		bool command_movbc();		// 7
		bool command_wrbf();		// 8
		bool command_rdbf();		// 9
		bool command_wrfbc();		// 10
		bool command_wrfbb();		// 11
		bool command_rdfbb();		// 12
		bool command_rdfbts();		// 13
		bool command_setmem();		// 14
		bool command_movb();		// 15
		bool command_nstart();		// 16
		bool command_appstart();	// 17
		bool command_mov32();		// 18
		bool command_movc32();		// 19
		bool command_wrfb32();		// 20
		bool command_rdfb32();		// 21
		bool command_wrfbc32();		// 22
		bool command_rdfbts32();	// 23
		bool command_movcf();		// 24
		bool command_pmov();		// 25
		bool command_pmov32();		// 26
		bool command_fillb();		// 27

		// Getting data from m_plainAppLogic
		//
	private:
		quint16 getWord(int wordOffset) const;
		quint32 getDword(int wordOffset) const;

		template <typename TYPE>
		TYPE getData(int eepromOffset) const;

	public:
		const Hardware::LogicModuleInfo& logicModuleInfo() const;

	private:
		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;
		QString m_simulationScript;

		Eeprom m_tuningEeprom = Eeprom(UartID::Tuning);
		Eeprom m_confEeprom = Eeprom(UartID::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartID::ApplicationLogic);

		QByteArray m_plainAppLogic;			// Just AppLogic data for specific m_logicModuleNumber and cleaned CRCs

		// Current state
		//
		DeviceMode m_currentMode = DeviceMode::Start;
		mutable int m_timerId = -1;

		Ram m_ram;
		LogicUnitData m_logicUnit;

		AfbComponentSet m_afbComponents;

		QJSEngine m_jsEngine;
		QJSValue m_evaluatedJs;

		// Info output
		//
		mutable QTextStream* m_output = nullptr;
	};
}

#endif // DEVICEEMULATOR_H
