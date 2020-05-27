#pragma once
#include "SimOutput.h"
#include <QQmlEngine>

namespace Sim
{
	class Simulator;
	class ScriptSimulator;

	class ScriptWorkerThread : public QThread, protected Output
	{
		Q_OBJECT

	public:
		ScriptWorkerThread(ScriptSimulator* scriptSimulator);

	private:
		virtual void run() override;

	public:
		void start(QThread::Priority priority = InheritPriority);
		bool interruptScript();

		bool result() const;
		void setScript(QString value);

	private:
		ScriptSimulator* m_scriptSimulator = nullptr;
		QString m_script;

		QJSEngine m_jsEngine;
		std::atomic_bool m_result;
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

		bool runScript(QString script);
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
