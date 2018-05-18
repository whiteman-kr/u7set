#ifndef DEVICEEMULATOR_H
#define DEVICEEMULATOR_H

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <QObject>
#include <QMutex>
#include <QTimerEvent>
#include "../lib/LmDescription.h"
#include "../lib/ModuleFirmware.h"
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimRam.h"
#include "SimAfb.h"


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
	class CommandProcessor;

	struct LogicUnitData
	{
		int programCounter = 0;					// current offeset of program memory, in words
		CyclePhase phase = CyclePhase::IdrPhase;
		quint16 appStartAddress = 0xFFFF;
	};


	class DeviceCommand
	{
	public:
		DeviceCommand() = delete;
		DeviceCommand(const LmCommand& command);
		DeviceCommand(const DeviceCommand& that) = default;
		DeviceCommand& operator=(const DeviceCommand& that) = default;
		DeviceCommand(DeviceCommand&&) = default;
		DeviceCommand& operator=(DeviceCommand&&) = default;

	public:
		void dump() const;

		QString caption() const;

	public:
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
	};

	//
	// class DeviceEmulator script wrapper
	//
	class DeviceEmulator;

	class ScriptDeviceEmulator
	{
	public:
		explicit ScriptDeviceEmulator(DeviceEmulator* device);

		// Script functins for AFB instances
		//
public:
		DeviceCommand* command(int index);

		quint16 appStartAddress() const;
		void setAppStartAddress(quint16 value);

		Sim::CyclePhase phase() const;
		void setPhase(Sim::CyclePhase value);

		quint32 programCounter() const;
		void setProgramCounter(quint32 value);

		Sim::AfbComponent afbComponent(int opCode) const;
		Sim::AfbComponentInstance* afbComponentInstance(int opCode, int instanceNo);

		bool setAfbParam(int afbOpCode, int instanceNo, const AfbComponentParam& param);

		// RAM access
		//
		bool movRamMem(quint32 src, quint32 dst, quint32 size);

		bool writeRamBit(quint32 offsetW, quint32 bitNo, quint32 data);
		quint16 readRamBit(quint32 offsetW, quint32 bitNo);

		bool writeRamWord(quint32 offsetW, quint16 data);
		quint16 readRamWord(quint32 offsetW);

		bool writeRamDword(quint32 offsetW, quint32 data);
		quint32 readRamDword(quint32 offsetW);

		// Getting data from m_plainAppLogic
		//
		quint16 getWord(int wordOffset) const;
		quint32 getDword(int wordOffset) const;

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

		bool clear();
		bool init(const Hardware::LogicModuleInfo& logicModuleInfo,		// Run from UI thread
				  const LmDescription& lmDescription,
				  const Eeprom& tuningEeprom,
				  const Eeprom& confEeprom,
				  const Eeprom& appLogicEeprom);

		bool reset();
		bool run(int cycles = -1);

	private:
		bool initMemory();
		bool initEeprom();
		bool parseAppLogicCode();
		bool parseCommand(const LmCommand& command, int programCounter);

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

		const LmDescription& lmDescription() const;

		std::vector<DeviceCommand> commands() const;
		std::map<int, size_t> offsetToCommands() const;

		const Ram& ram() const;

	private:
		friend class ScriptDeviceEmulator;

		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;

		std::unique_ptr<CommandProcessor> m_commandProcessor;

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

		// Cached state
		//
		mutable QMutex m_cacheMutex;

		Hardware::LogicModuleInfo m_cachedLogicModuleInfo;

		std::vector<DeviceCommand> m_cachedCommands;
		std::map<int, size_t> m_cachedOffsetToCommand;		// key: command offset, value: index in m_commands
	};
}

#endif // DEVICEEMULATOR_H
