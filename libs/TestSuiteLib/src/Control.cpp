#include <TestSuiteLib/Control.h>

#include <QEventLoop>

namespace
{
	constexpr qint64 ServiceConnectTimeoutMs = 5000;
}

namespace TestSuite
{
	//
	// ControlThread
	//

	ControlThread::ControlThread(ILogFile* appLog, TestLog* testLog, const QString& runContext) :
		m_appLog{appLog, runContext},
		m_testLog{testLog}
	{
		Q_ASSERT(m_testLog);
		return;
	}

	ControlThread::~ControlThread() = default;

	void ControlThread::addInputController(std::unique_ptr<IInputController> controller)
	{
		Q_ASSERT(isRunning() == false);
		Q_ASSERT(controller);
		m_inputController = std::move(controller);
		return;
	}

	void ControlThread::addOutputController(std::unique_ptr<IOutputController> controller)
	{
		Q_ASSERT(isRunning() == false);
		Q_ASSERT(controller);
		m_outputController = std::move(controller);
		return;
	}

	void ControlThread::setTestParams(const SoftwareInfo& softwareInfo,
									  const IScriptProvider& scriptProvider,
									  const ControlParams& controlParams,
									  const ComparatorSet* setpoints)
	{
		Q_ASSERT(isRunning() == false);

		m_softwareInfo = softwareInfo;
		m_controlParams = controlParams;
		m_setpoints = setpoints;
		m_scriptProvider = &scriptProvider;

		return;
	}

	int ControlThread::result() const
	{
		Q_ASSERT(isRunning() == false);
		return m_result.load();
	}

	::TestSuite::ControlStatus ControlThread::status() const
	{
		QMutexLocker l(&m_statusMutex);
		return m_status;
	}

	void ControlThread::checkAndInterruptTestExecution()
	{
		if (isInterruptionRequested() == true)
		{
			m_appLog.writeError(tr("Test execution was interrupted by user."));
			throw 1;
		}

		return;
	}

	void ControlThread::init()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::RequestingConfiguration;
		}

		return;
	}

	void ControlThread::cleanUp()
	{
		if (m_inputController)
		{
			m_inputController->shutdown();
		}

		if (m_outputController)
		{
			m_outputController->shutdown();
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.reset();
		}
		return;
	}

	void ControlThread::taskInitInputController()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::InitInputController;
		}

		if (m_inputController == nullptr)
		{
			m_appLog.writeWarning(tr("Input controller is not set."));
			return;
		}

		bool ok = m_inputController->init(ServiceConnectTimeoutMs);

		if (ok == false)
		{
			m_appLog.writeError(tr("Failed to init input controller."));
			throw 1;
		}

		return;
	}

	void ControlThread::taskInitOutputController()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::InitOutputController;
		}

		if (m_outputController == nullptr)
		{
			m_appLog.writeWarning(tr("Output controller is not set before test execution."));
			return;
		}

		m_outputController->init(ServiceConnectTimeoutMs);

		return;
	}

	//
	// Control
	//

	Control::Control(ILogFile* appLog, TestLog* testLog, ControlThread* controlThread) :
		QObject{nullptr},
		m_appLog{appLog},
		m_testLog{testLog},
		m_controlThread{controlThread}
	{
		Q_ASSERT(m_appLog);
		Q_ASSERT(m_testLog);
		Q_ASSERT(m_controlThread);
		m_controlThread->moveToThread(m_controlThread.get());
		return;
	}

	Control::~Control()
	{
		if (isRunning() == true)
		{
			stop();
		}
	}

	void Control::addInputController(std::unique_ptr<IInputController> controller)
	{
		Q_ASSERT(controller);
		m_controlThread->addInputController(std::move(controller));
		return;
	}

	void Control::addOutputController(std::unique_ptr<IOutputController> controller)
	{
		Q_ASSERT(controller);
		m_controlThread->addOutputController(std::move(controller));
		return;
	}

	bool Control::execute(const SoftwareInfo& softwareInfo, const IScriptProvider& scriptProvider, const ControlParams& controlParams, const ComparatorSet* setpoints)
	{
		if (isRunning() == true)
		{
			Q_ASSERT(false);
			return false;
		}

		m_controlThread->setTestParams(softwareInfo, scriptProvider, controlParams, setpoints);

		QEventLoop loop;
		QObject::connect(m_controlThread.get(), &QThread::started, &loop, &QEventLoop::quit);

		m_controlThread->start();

		// Wait that ControlThread actually started.
		//
		loop.exec();

		return true;
	}

	bool Control::stop()
	{
		if (m_stopRequested.load() == false)
		{
			// Request test thread to stop
			//
			m_stopRequested.store(true);
			m_controlThread->requestInterruption();

			// Wait for thread to stop in other thread and do not block interface thread
			//
			auto waitForThreadStop = [this]() -> void
			{
				if (m_controlThread->wait(120'000) == false)
				{
					qDebug() << "Control::stop(): m_controlThread was not finished in time, terminate().";
					m_controlThread->terminate();
				}

				m_stopRequested.store(false);
			};

			[[maybe_unused]] auto f = std::async(std::launch::async, waitForThreadStop);
		}
		else
		{
			m_appLog->writeWarning("Already waiting for testing thread to stop.");
		}

		return true;
	}

	bool Control::isRunning() const
	{
		return m_controlThread->isRunning();
	}

	ControlStatus Control::status() const
	{
		return m_controlThread->status();
	}
} // namespace TestSuite
