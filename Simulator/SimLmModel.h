#ifndef LMMODEL_H
#define LMMODEL_H
#include <memory>
#include <QTextStream>
#include <QByteArray>
#include <QThread>
#include <QtConcurrent>
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimDeviceEmulator.h"
#include "SimTimeController.h"
#include "../lib/LmDescription.h"
#include "../lib/ModuleFirmware.h"


namespace Sim
{
	class LogicModule : public QObject, protected Output
	{
		Q_OBJECT

	public:
		LogicModule();
		virtual ~LogicModule();

	public:
		bool load(const Hardware::LogicModuleInfo& lmInfo,
				  const LmDescription& lmDescription,
				  const Hardware::ModuleFirmware& firmware,
				  const QString& simulationScript);

		void clear();

		// Running LM
		//
		QFuture<bool> asyncRunCycle(std::chrono::microseconds currentTime);				// Start running one cycle

		//bool powerOn(bool autoStart);
		//bool powerOff();

		//bool pause();
		//bool start(int cycles = -1);
		//bool step();

		//bool isPowerOn() const;
		//bool isFaultMode() const;

		// Signals
		//
//	signals:
//		void signal_pause();				// Internal class signal, command to DeviceEmulator
//		void signal_start(int cycles);		// Internal class signal, command to DeviceEmulator

	signals:
		void faulted(QString message);		// Public signal, translated from DeviceEmulator

		// --
		//
	protected:
		bool loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom);

	protected slots:
		void slot_appCodeParsed(bool ok);

	public:
		QString equipmentId() const;
		int lmNumber() const;
		E::Channel channel() const;

		const Hardware::LogicModuleInfo& logicModuleInfo() const;

		LmDescription& lmDescription();
		const LmDescription& lmDescription() const;

		std::chrono::microseconds cycleDuration() const;

		const std::vector<DeviceCommand>& appCommands() const;

		std::map<int, size_t> offsetToCommand() const;
		const DeviceCommand& offsetToCommand(int offset) const;

	private:
		// Loaded LM data
		//
		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;

		Eeprom m_tuningEeprom = Eeprom(UartID::Tuning);
		Eeprom m_confEeprom = Eeprom(UartID::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartID::ApplicationLogic);

		QString m_simulationScript;

		// Running Emulation
		//
		//QThread m_workerThread;
		DeviceEmulator m_device;

		// --
		//
		std::vector<DeviceCommand> m_commands;
		std::map<int, size_t> m_offsetToCommand;		// key: command offset, value: index in m_commands
	};

}

#endif // LMMODEL_H
