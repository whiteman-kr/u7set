#pragma once
#include "SimOutput.h"
#include <QQmlEngine>
#include "../lib/Types.h"
#include "../lib/Signal.h"
#include "../lib/AppSignal.h"
#include "SimScriptSignal.h"
#include "SimScriptDevUtils.h"


namespace Sim
{
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

		/// \brief Get signal value, if signal is not found then exception is thrown.
		/// <b>Note:</b> This function does not return full signal state with validity and other flags.
		double signalValue(QString appSignalId);

		/// \brief Override signal value. Returns true if signal value is overriden.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply override to signal.
		/// <b>Note:</b> Not all signals can be overriden. For example, some signals can be optimized to constant value, as they don not have location in RAM they connot be overriden.
		bool overrideSignalValue(QString appSignalId, double value);

		/// \brief Remove all overriden signals.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply this function.
		void overridesReset();

		/// \brief Checks if a LogicModule exists.
		bool logicModuleExists(QString equipmentId) const;

		/// \brief Checks if a signal exists.
		bool signalExists(QString appSignalId) const;

		/// \brief Get signal description, if a signal is not found then exception is thrown.
		AppSignalParam signalParam(QString appSignalId);

		/// \brief Get full signal description, if a signal is not found then exception is thrown.
		ScriptSignal signalParamExt(QString appSignalId);

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

		ScriptDevUtils devUtils();

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

