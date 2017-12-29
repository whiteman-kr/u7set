#ifndef DEVICEEMULATOR_H
#define DEVICEEMULATOR_H

#include <memory>
#include <functional>
#include <map>
#include <vector>
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
		Operate
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


	class DeviceCommand : public QObject, public LmCommand
	{
		Q_OBJECT

		Q_PROPERTY(QString Caption MEMBER (m_command.caption) CONSTANT)

		Q_PROPERTY(int Offset MEMBER m_offset)
		Q_PROPERTY(int Size MEMBER m_size)
		Q_PROPERTY(QString AsString MEMBER m_string)

		Q_PROPERTY(quint16 AfbOpCode MEMBER m_afbOpCode)
		Q_PROPERTY(quint16 AfbInstance MEMBER m_afbInstance)
		Q_PROPERTY(quint16 AfbPinOpCode MEMBER m_afbPinOpCode)

		Q_PROPERTY(quint16 BitNo0 MEMBER m_bitNo0)
		Q_PROPERTY(quint16 BitNo1 MEMBER m_bitNo1)

		Q_PROPERTY(quint16 Word0 MEMBER m_word0)
		Q_PROPERTY(quint16 Word1 MEMBER m_word1)

		Q_PROPERTY(quint32 Dword0 MEMBER m_dword0)
		Q_PROPERTY(quint32 Dword1 MEMBER m_dword1)
	public:
		DeviceCommand(const LmCommand& command);
		DeviceCommand(const DeviceCommand& that);
		DeviceCommand& operator=(const DeviceCommand& that);

	public:
		// WARNING: Copy constructor is defined, do not forget to add there new members
		//
		LmCommand m_command;

		int m_offset = 0;				// Offset in Code Memory, words

		int m_size = 0;					// Command size in words. Set in parse script
		QString m_string;				// Set in parse script

		quint16 m_afbOpCode = 0;		// Set in parse script
		quint16 m_afbInstance = 0;		// Set in parse script
		quint16 m_afbPinOpCode = 0;		// Set in parse script

		quint16 m_bitNo0 = 0;			// Set in parse script
		quint16 m_bitNo1 = 0;			// Set in parse script

		quint16 m_word0 = 0;			// Set in parse script
		quint16 m_word1 = 0;			// Set in parse script

		quint32 m_dword0 = 0;			// Set in parse script
		quint32 m_dword1 = 0;			// Set in parse script

		// WARNING: Copy constructor is defined, do not forget to add there new members
		//
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
		bool parseCommand(const LmCommand& command, int programCounter);

		void dumpJsError(const QJSValue& value);

	public slots:
		void pause();
		void start(int cycles);

	private:
		void fault(QString reasone, QString func);

	private:
		virtual void timerEvent(QTimerEvent* event) override;

		bool processStartMode();
		bool processFaultMode();
//		bool processLoadEeprom();
		bool processOperate();

//		bool runCommand(LmCommandCode commandCode);

//		bool command_nop();			// 1
//		bool command_startafb();	// 2
//		bool command_stop();		// 3
//		bool command_mov();			// 4
//		bool command_movmem();		// 5
//		bool command_movc();		// 6
//		bool command_movbc();		// 7
//		bool command_wrbf();		// 8
//		bool command_rdbf();		// 9
//		bool command_wrfbc();		// 10
//		bool command_wrfbb();		// 11
//		bool command_rdfbb();		// 12
//		bool command_rdfbts();		// 13
//		bool command_setmem();		// 14
//		bool command_movb();		// 15
//		bool command_nstart();		// 16
//		bool command_appstart();	// 17
//		bool command_mov32();		// 18
//		bool command_movc32();		// 19
//		bool command_wrfb32();		// 20
//		bool command_rdfb32();		// 21
//		bool command_wrfbc32();		// 22
//		bool command_rdfbts32();	// 23
//		bool command_movcf();		// 24
//		bool command_pmov();		// 25
//		bool command_pmov32();		// 26
//		bool command_fillb();		// 27

		// Getting data from m_plainAppLogic
		//
	public:
		Q_INVOKABLE QObject* afbComponent(int opCode);

		Q_INVOKABLE quint16 getWord(int wordOffset) const;
		Q_INVOKABLE quint32 getDword(int wordOffset) const;

	private:
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

		std::vector<DeviceCommand> m_commands;

		AfbComponentSet m_afbComponents;

		QJSEngine m_jsEngine;
		QJSValue m_evaluatedJs;
		QJSValue m_thisJsValue;
	};
}

#endif // DEVICEEMULATOR_H
