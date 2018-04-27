#ifndef DEVICEEMULATOR_H
#define DEVICEEMULATOR_H

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <QObject>
#include <QMutex>
#include <QTimerEvent>
#include <QJSEngine>
#include "../lib/LmDescription.h"
#include "../lib/ModuleFirmware.h"
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimRam.h"
#include "SimComponent.h"

extern "C" {
	#include "../Lua/lua.h"
	#include "../Lua/lauxlib.h"
	#include "../Lua/lualib.h"
}
#include "../LuaIntf/LuaIntf.h"


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
}

Q_DECLARE_METATYPE(Sim::CyclePhase)

namespace Sim
{
	struct LogicUnitData
	{
		int programCounter = 0;					// current offeset of program memory, in words
		CyclePhase phase = CyclePhase::IdrPhase;
		quint16 appStartAddress = 0xFFFF;
	};


	class DeviceCommand : public LmCommand
	{
//		Q_OBJECT

//		Q_PROPERTY(QString Caption MEMBER (m_command.caption) CONSTANT)

//		Q_PROPERTY(int Offset MEMBER m_offset)
//		Q_PROPERTY(int Size MEMBER m_size)
//		Q_PROPERTY(QString AsString MEMBER m_string)

//		Q_PROPERTY(quint16 AfbOpCode MEMBER m_afbOpCode)
//		Q_PROPERTY(quint16 AfbInstance MEMBER m_afbInstance)
//		Q_PROPERTY(quint16 AfbPinOpCode MEMBER m_afbPinOpCode)

//		Q_PROPERTY(quint16 BitNo0 MEMBER m_bitNo0)
//		Q_PROPERTY(quint16 BitNo1 MEMBER m_bitNo1)

//		Q_PROPERTY(quint16 Word0 MEMBER m_word0)
//		Q_PROPERTY(quint16 Word1 MEMBER m_word1)
//		Q_PROPERTY(quint16 Word2 MEMBER m_word2)

//		Q_PROPERTY(quint32 Dword0 MEMBER m_dword0)
//		Q_PROPERTY(quint32 Dword1 MEMBER m_dword1)

	public:
		DeviceCommand(const LmCommand& command);
		DeviceCommand(const DeviceCommand& that);
		DeviceCommand& operator=(const DeviceCommand& that);

		static void registerLuaClass(lua_State* L);

		void dump() const;

		std::string scriptTest(std::string inputParam)
		{
			//return QString("ScriptTest" + inputParam).toStdString();
			return "ScriptTest" + inputParam;
		}

		// Properties for Lua
	public:
		std::string caption() const;

		std::string asString() const;
		void setAsString(const std::string& value);

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
		quint16 m_word2 = 0;			// Set in parse script

		quint32 m_dword0 = 0;			// Set in parse script
		quint32 m_dword1 = 0;			// Set in parse script

		// WARNING: Copy constructor is defined, do not forget to add there new members
		//
	};


	//
	// class DeviceEmulator script wrapper
	//
	class DeviceEmulator;

	class ScriptDeviceEmulator /*: public QObject*/
	{
//		Q_OBJECT

//		Q_PROPERTY(quint16 AppStartAddress MEMBER (m_device->m_logicUnit.appStartAddress))
//		Q_PROPERTY(Sim::CyclePhase Phase MEMBER (m_device->m_logicUnit.phase))
//		Q_PROPERTY(quint32 ProgramCounter MEMBER (m_device->m_logicUnit.programCounter))

	public:
		explicit ScriptDeviceEmulator(DeviceEmulator* device);

		static void registerLuaClass(lua_State* L);

		// Script functins for AFB instances
		//
public:
		Afb::AfbComponent* afbComponent(int opCode);
		AfbComponentInstance* afbComponentInstance(int opCode, int instanceNo);

//	public slots:
//		QObject* afbComponent(int opCode);
//		QObject* afbComponentInstance(int opCode, int instanceNo);

//		QObject* createComponentParam();
//		bool setAfbParam(int afbOpCode, int instanceNo, ComponentParam* param);

//		// RAM access
//		//
//		bool writeRamBit(quint32 offsetW, quint32 bitNo, quint32 data);
//		quint16 readRamBit(quint32 offsetW, quint32 bitNo);

//		bool writeRamWord(quint32 offsetW, quint16 data);
//		quint16 readRamWord(quint32 offsetW);

//		bool writeRamDword(quint32 offsetW, quint32 data);
//		quint32 readRamDword(quint32 offsetW);

		// Getting data from m_plainAppLogic
		//
		quint16 getWord(int wordOffset);
		quint32 getDword(int wordOffset);

	private:
		DeviceEmulator* m_device = nullptr;
	};


	//
	// DeviceEmulator
	//
	class DeviceEmulator : public QObject, protected Output
	{
		Q_OBJECT

	public:
		DeviceEmulator();
		virtual ~DeviceEmulator();

		bool init(const Hardware::LogicModuleInfo& logicModuleInfo,		// Run from UI thread
				  const LmDescription& lmDescription,
				  const Eeprom& tuningEeprom,
				  const Eeprom& confEeprom,
				  const Eeprom& appLogicEeprom,
				  const QString& simulationScript);

		bool reset();
		bool run(int cycles = -1);

	private:
		bool initMemory();
		bool initEeprom();
		bool parseAppLogicCode();
		bool parseCommand(const LmCommand& command, int programCounter);

		void dumpJsError(const QJSValue& value);
		void dumpLuaError(int result, QString function);

	public slots:
		//void pause();
		//void start(int cycles);

	private:
		void fault(QString reasone, QString func);

		//virtual void timerEvent(QTimerEvent* event) override;

		bool processStartMode();
		bool processFaultMode();
		bool processOperate();

		bool runCommand(DeviceCommand& deviceCommand);

	private:
		// Getting data from m_plainAppLogic
		//
		quint16 getWord(int wordOffset);
		quint32 getDword(int wordOffset);

		template <typename TYPE>
		TYPE getData(int eepromOffset);

	signals:
		void appCodeParsed(bool ok);
		void faulted(QString message);

		// Props
		//
	public:
		Hardware::LogicModuleInfo logicModuleInfo() const;
		void setLogicModuleInfo(const Hardware::LogicModuleInfo& lmInfo);

		std::vector<DeviceCommand> commands() const;
		std::map<int, size_t> offsetToCommands() const;

		const Ram& ram() const;

	private:
		friend class ScriptDeviceEmulator;

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
		std::atomic<int> m_timerId = -1;

		Ram m_ram;
		LogicUnitData m_logicUnit;

		std::vector<DeviceCommand> m_commands;
		std::map<int, size_t> m_offsetToCommand;		// key: command offset, value: index in m_commands

		AfbComponentSet m_afbComponents;

		// JS
		//
		QJSEngine m_jsEngine;

		QJSValue m_evaluatedJs;
		QJSValue m_thisJsValue;

		// Lua
		//
		lua_State* m_luaState = nullptr;

		// Cached state
		//
		mutable QMutex m_cacheMutex;

		Hardware::LogicModuleInfo m_cachedLogicModuleInfo;

		std::vector<DeviceCommand> m_cachedCommands;
		std::map<int, size_t> m_cachedOffsetToCommand;		// key: command offset, value: index in m_commands
	};
}

#endif // DEVICEEMULATOR_H
