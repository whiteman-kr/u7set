#ifndef LMMODEL_H
#define LMMODEL_H
#include <memory>
#include <QTextStream>
#include <QByteArray>
#include <QThread>
#include "../lib/LmDescription.h"
#include "Eeprom.h"
#include "DeviceEmulator.h"


namespace LmModel
{
	class LogicModule : public QObject
	{
		Q_OBJECT

	public:
		explicit LogicModule(QTextStream* textStream = nullptr);
		virtual ~LogicModule();

	public:
		bool load(const QByteArray& logicModuleDescription,
				  const QByteArray& tuningBitsream,
				  const QByteArray& confBitstream,
				  const QByteArray& appLogicBitstream,
				  const QString& simulationScript);

		void clear();

		// Running LM
		//
	public:
		bool powerOn(int logicModuleNumber, bool autoStart);
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
		bool loadLmDescription(const QByteArray& logicModuleDescription);
		bool loadEeprom(const QByteArray& fileData, Eeprom* eeprom);

		QTextStream& output();
		QTextStream& output() const;

	private:
		// Loaded LM data
		//
		std::unique_ptr<LmDescription> m_lmDescription;

		Eeprom m_tuningEeprom = Eeprom(UartID::Tuning);
		Eeprom m_confEeprom = Eeprom(UartID::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartID::ApplicationLogic);

		QString m_simulationScript;

		// Running Emulation
		//
		QThread m_workerThread;
		DeviceEmulator* m_device = nullptr;

		// Text output variables
		//
		mutable QTextStream* m_textStream = nullptr;

		FILE* m_nullDevice = nullptr;			// If m_textStream is null, then m_nullTextStream is used instead
		mutable QTextStream m_nullTextStream;
	};

}

#endif // LMMODEL_H
