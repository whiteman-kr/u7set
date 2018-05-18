#ifndef LMMODEL_H
#define LMMODEL_H
#include <memory>
#include <QTextStream>
#include <QByteArray>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimDeviceEmulator.h"
#include "SimTimeController.h"
#include "SimCommandProcessor.h"
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
				  const Hardware::ModuleFirmware& firmware);

		void clear();

		// Running LM
		//
		QFuture<bool> asyncRunCycle(std::chrono::microseconds currentTime, bool reset);				// Start running one cycle

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

		std::unordered_map<int, size_t> offsetToCommand() const;
		const DeviceCommand& offsetToCommand(int offset) const;

		// Simtime
		//
		const Ram& ram() const;

	private:
		// Loaded LM data
		//
		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;

		Eeprom m_tuningEeprom = Eeprom(UartID::Tuning);
		Eeprom m_confEeprom = Eeprom(UartID::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartID::ApplicationLogic);

		// Running Emulation
		//
		DeviceEmulator m_device;

		// --
		//
		std::vector<DeviceCommand> m_commands;
		std::unordered_map<int, size_t> m_offsetToCommand;		// key: command offset, value: index in m_commands
	};

}

#endif // LMMODEL_H
