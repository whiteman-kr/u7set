#ifndef DEVICEEMULATOR_H
#define DEVICEEMULATOR_H

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <array>
#include <cstddef>
#include <chrono>
#include <limits>
#include <QObject>
#include <QMutex>
#include <QTimerEvent>
#include "../lib/LmDescription.h"
#include "../lib/ModuleFirmware.h"
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimRam.h"
#include "SimConnections.h"
#include "SimAfb.h"
#include "SimOverrideSignals.h"


#ifndef __FUNCTION_NAME__
	#ifdef WIN32   //WINDOWS
		#define __FUNCTION_NAME__   __FUNCTION__
	#else          //*NIX
		#define __FUNCTION_NAME__   __func__
	#endif
#endif

// class DeviceEmulator has function DeviceEmulator::fault
// this is convenient call of this func
//
#define SIM_FAULT(message) fault(message, __FUNCTION_NAME__);

namespace Sim
{
	Q_NAMESPACE

	enum class DeviceMode
	{
		Start,
		Fault,
		Operate
	};

	Q_ENUM_NS(DeviceMode)

	enum class CyclePhase
	{
		IdrPhase,
		AlpPhase,
		ODT,
		ST
	};
}

Q_DECLARE_METATYPE(Sim::CyclePhase)
Q_DECLARE_METATYPE(Sim::DeviceMode)

namespace Sim
{
	class CommandProcessor;

	struct LogicUnitData
	{
		int programCounter = 0;					// current offeset of program memory, in words
		CyclePhase phase = CyclePhase::IdrPhase;
		quint16 appStartAddress = 0xFFFF;

		union Flags
		{
			struct
			{
				quint32 cmp : 1;
			};

			quint32 value = 0;
		} flags;
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

		// Specific data for specific CommandController
		//
		std::array<std::byte, 16> m_commandFuncPtr;	// This function can be called ONLY in in class and instance where it was initialized
		std::array<std::byte, 16> m_afbFuncPtr;		// This function can be called ONLY in in class and instance where it was initialized

		Ram::Handle m_memoryAreaFrom = std::numeric_limits<Ram::Handle>::max();
		Ram::Handle m_memoryAreaTo = std::numeric_limits<Ram::Handle>::max();

		AfbComponentParam m_afbParam;
		AfbComponentInstance* m_afbComponentInstance = nullptr;
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

	public:
		bool clear();
		bool init(const Hardware::LogicModuleInfo& logicModuleInfo,		// Run from UI thread
				  const LmDescription& lmDescription,
				  const Eeprom& tuningEeprom,
				  const Eeprom& confEeprom,
				  const Eeprom& appLogicEeprom,
				  const Connections& connections);

		bool reset();
		bool runWorkcycle(std::chrono::microseconds currentTime, qint64 workcycle);

		//	Public methods to access from simulation commands
		//
	public:
		DeviceCommand* command(int index);

		quint16 appStartAddress() const;
		void setAppStartAddress(quint16 value);

		Sim::CyclePhase phase() const;
		void setPhase(Sim::CyclePhase value);

		quint32 programCounter() const;
		void setProgramCounter(quint32 value);

		quint32 flagCmp() const;
		void setFlagCmp(quint32 value);

		Sim::AfbComponent afbComponent(int opCode) const;
		Sim::AfbComponentInstance* afbComponentInstance(int opCode, int instanceNo);

		bool setAfbParam(int afbOpCode, int instanceNo, const AfbComponentParam& param);
		bool setAfbParam(int afbOpCode, int instanceNo, AfbComponentParam&& param);

		// RAM access
		//
		bool movRamMem(quint32 src, quint32 dst, quint32 size);
		bool setRamMem(quint32 address, quint16 data, quint16 size);

		bool writeRamBit(quint32 offsetW, quint16 bitNo, quint16 data);
		bool writeRamBit(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 bitNo, quint16 data);

		quint16 readRamBit(quint32 offsetW, quint16 bitNo);
		quint16 readRamBit(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 bitNo);

		bool writeRamBit(quint32 offsetW, quint16 bitNo, quint16 data, E::LogicModuleRamAccess access);
		quint16 readRamBit(quint32 offsetW, quint16 bitNo, E::LogicModuleRamAccess access);

		bool writeRamWord(quint32 offsetW, quint16 data);
		bool writeRamWord(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 data);

		quint16 readRamWord(quint32 offsetW);
		quint16 readRamWord(Ram::Handle memoryAreaHandle, quint32 offsetW);

		bool writeRamWord(quint32 offsetW, quint16 data, E::LogicModuleRamAccess access);
		quint16 readRamWord(quint32 offsetW, E::LogicModuleRamAccess access);

		bool writeRamDword(quint32 offsetW, quint32 data);
		bool writeRamDword(Ram::Handle memoryAreaHandle, quint32 offsetW, quint32 data);

		quint32 readRamDword(quint32 offsetW);
		quint32 readRamDword(Ram::Handle memoryAreaHandle, quint32 offsetW);

		bool writeRamDword(quint32 offsetW, quint32 data, E::LogicModuleRamAccess access);
		quint32 readRamDword(quint32 offsetW, E::LogicModuleRamAccess access);

		// Getting data from m_plainAppLogic
		//
		quint16 getWord(int wordOffset) const;
		quint32 getDword(int wordOffset) const;

		// --
		//
	private:
		bool initMemory();
		bool initEeprom();
		bool parseAppLogicCode();
		bool parseCommand(const LmCommand& command, int programCounter);

	public:
		void fault(QString reasone, QString func);

	private:
		bool processStartMode();
		bool processFaultMode();

		bool processOperate(std::chrono::microseconds currentTime, qint64 workcycle);

		bool runCommand(DeviceCommand& deviceCommand);

	public:
		bool receiveConnectionsData(std::chrono::microseconds currentTime);	// This one is public to be called from Sim::Control
		bool sendConnectionsData(std::chrono::microseconds currentTime);	// Actually this one is private

	private:
		// Getting data from m_plainAppLogic
		//
		template <typename TYPE>
		TYPE getData(int eepromOffset) const;

		// Props
		//
	public:
		const QString& equipmentId() const;

		int buildNo() const;

		Hardware::LogicModuleInfo logicModuleInfo() const;
		void setLogicModuleInfo(const Hardware::LogicModuleInfo& lmInfo);

		const LmDescription& lmDescription() const;

		void setOverrideSignals(OverrideSignals* overrideSignals);

		std::vector<DeviceCommand> commands() const;
		std::unordered_map<int, size_t> offsetToCommands() const;

		const Ram& ram() const;

		DeviceMode currentMode() const;

	private:
		void setCurrentMode(DeviceMode value);

		// Data
		//
	private:
		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;

		OverrideSignals* m_overrideSignals = nullptr;

		std::unique_ptr<CommandProcessor> m_commandProcessor;

		Eeprom m_tuningEeprom = Eeprom(UartId::Tuning);
		Eeprom m_confEeprom = Eeprom(UartId::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartId::ApplicationLogic);

		QByteArray m_plainAppLogic;			// Just AppLogic data for specific m_logicModuleNumber and cleaned CRCs
		QByteArray m_plainTuningData;		// Just Tuning data for specific m_logicModuleNumber and cleaned CRCs

		// Current state
		//
		DeviceMode m_currentMode = DeviceMode::Start;

		Ram m_ram;
		LogicUnitData m_logicUnit;
		std::vector<ConnectionPtr> m_connections;

		std::vector<DeviceCommand> m_commands;
		std::vector<int> m_offsetToCommand;						// index: command offset, value: index in m_commands
																// empty offsets is -1
																// Programm memory is not so big, max

		AfbComponentSet m_afbComponents;

		// Cached state
		//
		mutable QMutex m_cacheMutex;

		Hardware::LogicModuleInfo m_cachedLogicModuleInfo;

		std::vector<DeviceCommand> m_cachedCommands;
		std::unordered_map<int, size_t> m_cachedOffsetToCommand;	// key: command offset, value: index in m_commands
	};
}

#endif // DEVICEEMULATOR_H
