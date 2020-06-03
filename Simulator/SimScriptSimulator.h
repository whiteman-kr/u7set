#pragma once
#include "SimOutput.h"
#include <QQmlEngine>
#include "../lib/Types.h"
#include "../lib/Signal.h"

namespace Sim
{


	/*! \class RamAddress
		\ingroup simulator
		\brief Represents address in RAM of simulated module.
	*/
	class RamAddress
	{
		Q_GADGET

		/// \brief Offset in RAM, 0xFFFFFFFF means non valid address.
		Q_PROPERTY(quint32 offset READ offset WRITE setOffset)

		/// \brief Bit no for for discerete signals, 0xFFFFFFFF means non valid address.
		Q_PROPERTY(quint32 bit READ bit WRITE setBit)

	public:
		RamAddress() = default;
		RamAddress(const RamAddress&) = default;
		RamAddress(const Address16& addr16);
		RamAddress(quint32 offset, quint32 bit);

		~RamAddress() = default;

	public:
		/// \brief Get address validity, returns true if the address is valid and can be used for RAM operations.
		bool isValid() const;

		quint32 offset() const;
		void setOffset(quint32 value);

		quint32 bit() const;
		void setBit(quint32 value);

		/// \brief Convert address to string representation.
		Q_INVOKABLE QString toString() const;

	private:
		static const quint32 BadAddress = std::numeric_limits<quint32>::max();

		quint32 m_offset = BadAddress;
		quint32 m_bit = BadAddress;
	};

}

Q_DECLARE_METATYPE(Sim::RamAddress);


namespace Sim
{
	class Simulator;
	class ScriptSimulator;
	class LogicModule;


	class ScriptWorkerThread : public QThread, protected Output
	{
		Q_OBJECT

	public:
		ScriptWorkerThread(ScriptSimulator* scriptSimulator);

	private:
		virtual void run() override;

        bool runScriptFunction(const QString& functionName);

	public:
		void start(QThread::Priority priority = InheritPriority);
		bool interruptScript();

		bool result() const;

		void setScript(QString value);
		void setTestName(QString value);

	private:
		ScriptSimulator* m_scriptSimulator = nullptr;

		QString m_script;
		QString m_testName;

        QJSEngine m_jsEngine;
        QJSValue m_jsThis;

        std::atomic_bool m_result{true};
	};


	// Proxy class for using in scripts
	//
	/*! \class ScriptSimulator
		\ingroup simulator
		\brief Represents class that runs all simulations on compiled project.
	*/
	class ScriptSimulator : public QObject, protected Output
	{
		Q_OBJECT

		/// \brief Loaded project build directory, if empty then project is not loaded.
		Q_PROPERTY(QString buildPath READ buildPath)

		/// \brief Script execution timeout in milliseconds, if negative then timeout is not applied.
		Q_PROPERTY(qint64 executionTimeOut READ executionTimeOut WRITE setExecutionTimeOut)

	public:
		explicit ScriptSimulator(Simulator* simulator, QObject* parent = nullptr);
		virtual ~ScriptSimulator();

		bool runScript(QString script, QString testName);
		bool stopScript();

		bool isRunning() const;

		bool wait(unsigned long msecs = ULONG_MAX);		// Wait script to stop
		bool result() const;

		// Public slots which are part of Script API
		//
	public slots:
        void debugOutput(QString str);					// Debug output to qDebug

        /// \brief Run the simulation for \a msec milliseconds, if \a msec is -1 then simulation will last till the programm interrupted.
		/// <b>Note:</b> Simulation process can last longer than \a msec milliseconds, it depends on project size and simulation hardware.
		bool startForMs(int msecs);

		/// \brief Reset all simulations to initial state.
		/// <b>Note:</b> Function sets reset flag and actual reset will be performed on the next \c startForMs call.
		bool reset();

		/// \brief Get signal value, if signal is not found then -1 is returned (what is actully valid value for existing signals too).
		/// <b>Note:</b> This function does not return full signal state with validity and other flags.
		double signalValue(QString appSignalId);

		/// \brief Override signal value. Returns true if signal value is overriden.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply override to signal.
		/// <b>Note:</b> Not all signals can be overriden. For example, some signals can be optimized to constant value, as they don not have location in RAM they connot be overriden.
		bool overrideSignalValue(QString appSignalId, double value);

		/// \brief Remove all overriden signals.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply this function.
		void overridesReset();

		/// \brief Returns signal address in User Application Logic. Return type is Sim::RamAddress
		/// <b>Note:</b> This function is mostly used for internal test cases.
		RamAddress signalUalAddress(QString appSignalId);

		RamAddress signalIoAddress(QString appSignalId);
		RamAddress signalTuningAddr(QString appSignalId);
		RamAddress signalTuningAbsAddr(QString appSignalId);
		RamAddress signalRegBufAddr(QString appSignalId);
		RamAddress signalRegValueAddr(QString appSignalId);
		RamAddress signalRegValidityAddr(QString appSignalId);

		quint16 readRamBit(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access);
		quint16 readRamWord(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access);
		quint32 readRamDword(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access);
		qint32 readRamSignedInt(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access);
		float readRamFloat(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access);

		void writeRamBit(QString lmEquipmentId, RamAddress address, quint16 value, E::LogicModuleRamAccess access);
		void writeRamWord(QString lmEquipmentId, RamAddress address, quint16 value, E::LogicModuleRamAccess access);
		void writeRamDword(QString lmEquipmentId, RamAddress address, quint32 value, E::LogicModuleRamAccess access);
		void writeRamSignedInt(QString lmEquipmentId, RamAddress address, qint32 value, E::LogicModuleRamAccess access);
		void writeRamFloat(QString lmEquipmentId, RamAddress address, float value, E::LogicModuleRamAccess access);

	private:
		// Throws Script Exception if logic module is not found
		//
		std::shared_ptr<LogicModule> logicModule(QString lmEquipmentId);
		void throwScriptException(QString text);

	public:
		QString buildPath() const;

		qint64 executionTimeOut() const;
		void setExecutionTimeOut(qint64 value);

		// Data
		//
	private:
		Simulator* m_simulator = nullptr;
		ScriptWorkerThread m_workerThread{this};

		qint64 m_executionTimeOut = -1;		// Script execution timeout in milliseconds, negative means no timeout
	};

}

