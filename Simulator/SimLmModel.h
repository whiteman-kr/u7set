#ifndef LMMODEL_H
#define LMMODEL_H
#include <memory>
#include <QTextStream>
#include <QByteArray>
#include <QThread>
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimDeviceEmulator.h"
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
	public:
		bool powerOn(bool autoStart);
		bool powerOff();

		bool pause();
		bool start(int cycles = -1);
		bool step();

		bool isPowerOn() const;
		bool isFaultMode() const;

	signals:
		void signal_pause();
		void signal_start(int cycles);

		// --
		//
	protected:
		bool loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom);

	public:
		const Hardware::LogicModuleInfo& logicModuleInfo() const;

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
		QThread m_workerThread;
		DeviceEmulator* m_device = nullptr;
	};

}

#endif // LMMODEL_H
