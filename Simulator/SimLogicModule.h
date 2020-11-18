#pragma once

#include <memory>
#include <QTextStream>
#include <QByteArray>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "SimEeprom.h"
#include "SimDeviceEmulator.h"
#include "SimTimeController.h"
#include "SimCommandProcessor.h"
#include "../lib/LmDescription.h"
#include "../lib/ModuleFirmware.h"
#include "../lib/LogicModulesInfo.h"


namespace Sim
{
	class Simulator;
	class Connections;


	class LogicModule : public QObject
	{
		Q_OBJECT

	public:
		LogicModule(Simulator* simulator);
		virtual ~LogicModule();

	public:
		bool load(const Hardware::LogicModuleInfo& lmInfo,
				  const LmDescription& lmDescription,
				  const Hardware::ModuleFirmware& firmware,
				  const Connections& connections,
				  const LogicModulesInfo& logicModulesExtraInfo);

		void clear();

		// Running LM
		//
		QFuture<bool> asyncRunCycle(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 workcycle,  bool reset);				// Start running one cycle

		bool receiveConnectionsData(std::chrono::microseconds currentTime);

		// --
		//
	protected:
		bool loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom);

	protected slots:
		void setAppCommands(bool ok);

	public:
		const QString& equipmentId() const;
		int lmNumber() const;
		E::Channel channel() const;

		const Hardware::LogicModuleInfo& logicModuleInfo() const;

		LmDescription& lmDescription();
		const LmDescription& lmDescription() const;

		const ::LogicModuleInfo& logicModuleExtraInfo() const;
		void setLogicModuleExtraInfo(const ::LogicModuleInfo& value);

		std::chrono::microseconds cycleDuration() const;

		const std::vector<DeviceCommand>& appCommands() const;

		std::unordered_map<int, size_t> offsetToCommand() const;
		const DeviceCommand& offsetToCommand(int offset) const;

		const Ram& ram() const;	// This RAM access is not protected by any mutext, use it only when no concurent thread is accessing it!
		Ram& mutableRam();		// This RAM access is not protected by any mutext, use it only when no concurent thread is accessing it!

		DeviceMode deviceMode() const;

		bool isPowerOff() const;
		void setPowerOff(bool value);

	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		// Loaded LM data
		//
		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;

		Eeprom m_tuningEeprom = Eeprom(UartId::Tuning);
		Eeprom m_confEeprom = Eeprom(UartId::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartId::ApplicationLogic);

		// Running Emulation
		//
		DeviceEmulator m_device{m_simulator};

		// --
		//
		std::vector<DeviceCommand> m_commands;
		std::unordered_map<int, size_t> m_offsetToCommand;		// key: command offset, value: index in m_commands
	};

}

