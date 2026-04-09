#include <TestSuiteLib/MatsTestSuite.h>
#include <TestSuiteLib/TestControl.h>
#include <TestSuiteLib/TestLog.h>
#include <TestSuiteLib/TestSuite.h>

#include "AdsInputController.h"
#include "RunControl.h"
#include "TunsOutputController.h"

#include <ClientLib/AppSignalManager.h>
#include <ClientLib/TuningUserManager.h>
#include <TestSuiteLib/TestReport.h>

#include <QEventLoop>
#include <QTimer>

namespace
{
	constexpr qint64 ServiceConnectTimeoutMs = 15'000;

	using CreateAdsControllerFunc = std::function<std::unique_ptr<::TestSuite::AdsInputController>(const ::TestSuite::ConfigSettings&)>;
	using CreateTunControllerFunc =
		std::function<std::unique_ptr<::TestSuite::TunsOutputController>(const ::TestSuite::ConfigSettings&, QString username)>;


	class MatsTestControlThread : public ::TestSuite::TestControlThread
	{
	public:
		MatsTestControlThread(ILogFile* appLog,
							  ::TestSuite::TestLog* testLog,
							  ::TestSuite::TestSuiteConfigController& configController,
							  CreateAdsControllerFunc createAdsInputControllerFunc,
							  CreateTunControllerFunc createTunOutputControllerFunc) :
			::TestSuite::TestControlThread{appLog, testLog},
			m_configController{configController},
			m_createAdsInputControllerFunc{std::move(createAdsInputControllerFunc)},
			m_createTunOutputControllerFunc{std::move(createTunOutputControllerFunc)}
		{
			Q_ASSERT(m_createAdsInputControllerFunc);
			Q_ASSERT(m_createTunOutputControllerFunc);
		}

	protected:
		virtual void init() override
		{
			::TestSuite::TestControlThread::init();

			m_configuration = {};
			m_configData = {};

			QDeadlineTimer timer{ServiceConnectTimeoutMs};

			do
			{
				m_configuration = m_configController.configuration();
				m_configData = m_configController.configData();

				if (m_configuration.isValid() == false)
				{
					QThread::msleep(100);
				}
			} while (timer.hasExpired() == false && m_configuration.isValid() == false);

			if (m_configuration.isValid() == false)
			{
				m_appLog.writeError(tr("Cannot get configuration from CfgService."));
				throw 1;
			}

			return;
		}

		virtual void cleanUp() override
		{
			::TestSuite::TestControlThread::cleanUp();

			m_configuration = {};
			m_configData = {};

			return;
		}

		virtual void taskInitInputController() override
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_state = TestSuite::ControlState::InitInputController;
			}

			if (m_createAdsInputControllerFunc == nullptr || m_configuration.isValid() == false)
			{
				Q_ASSERT(m_createAdsInputControllerFunc);
				Q_ASSERT(m_configuration.isValid());

				m_appLog.writeError(tr("Internal error: AdsInputController creation function is not set or configuration is invalid. "
									   "Please contact the developer."));
				throw 1;
			}

			auto ads = m_createAdsInputControllerFunc(m_configuration);
			if (ads == nullptr)
			{
				Q_ASSERT(ads);
				m_appLog.writeError(tr("Internal error: AdsInputController is not created. Please contact the developer."));
				throw 1;
			}

			bool initOk = ads->init(ServiceConnectTimeoutMs);
			if (initOk == false)
			{
				m_appLog.writeError(tr("Failed to init AdsInputController."));
				throw 1;
			}

			m_inputController = std::move(ads);

			return;
		}

		virtual void taskInitOutputController() override
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_state = TestSuite::ControlState::InitOutputController;
			}

			if (m_createTunOutputControllerFunc == nullptr || m_configuration.isValid() == false)
			{
				Q_ASSERT(m_createTunOutputControllerFunc);
				Q_ASSERT(m_configuration.isValid());

				m_appLog.writeError(tr("Internal error: TunsOutputController creation function is not set or configuration is invalid. "
									   "Please contact the developer."));
				throw 1;
			}

			if (m_configuration.tuningEnabled == false)
			{
				m_appLog.writeWarning(tr("Tuning is disabled in the configuration."));
				return;
			}

			auto tuns = m_createTunOutputControllerFunc(m_configuration, m_controlParams.userName);
			if (tuns == nullptr)
			{
				Q_ASSERT(tuns);
				m_appLog.writeError(tr("Internal error: TunsOutputController is not created. Please contact the developer."));
				throw 1;
			}

			bool initOk = tuns->init(ServiceConnectTimeoutMs);
			if (initOk == false)
			{
				m_appLog.writeError(tr("Failed to init TunsOutputController."));
				throw 1;
			}

			m_outputController = std::move(tuns);

			return;
		}

		virtual void taskCheckLogin() override
		{
			// Check user name
			//
			auto m_configuration = m_configController.configuration();

			if (m_configuration.login == true)
			{
				if (m_controlParams.userName.isEmpty() == true)
				{
					m_appLog.writeError(tr("Tests execution failed: no user name is supplied! Please check the configuration."));
					throw 1;
				}

				if (ClientLib::TuningUserManager::checkPassword(m_controlParams.userName, m_controlParams.password) == false)
				{
					m_appLog.writeError(tr("Tests execution failed: authorization failed!"));
					throw 1;
				}
			}
		}

		virtual void taskCreateReports() override
		{
			TestControlThread::taskCreateReports();

			QString reportsPath{m_controlParams.reportsPath};

			if (reportsPath.contains("default") == true)
			{
				reportsPath.replace("default", QString("TestReport_%1").arg(DateTimeToString::fileName(QDateTime::currentDateTime())));
			}

			auto configData = m_configController.configData();

			TestSuite::TestReport::generateReports(configData.reportTemplates, *m_testLog, QString(), reportsPath, m_appLog.logFile());
		}

		[[nodiscard]] virtual QString plant() const override { return m_configuration.plant; }
		[[nodiscard]] virtual QString unit() const override { return m_configuration.unit; }
		[[nodiscard]] virtual QString system() const override { return m_configuration.system; }

		[[nodiscard]] virtual QString projectName() const override { return m_configuration.configInfo.project; }
		[[nodiscard]] virtual int buildNo() const override { return m_configuration.configInfo.buildNo; }

	private:
		::TestSuite::TestSuiteConfigController& m_configController;

		CreateAdsControllerFunc m_createAdsInputControllerFunc;
		CreateTunControllerFunc m_createTunOutputControllerFunc;

		::TestSuite::ConfigSettings m_configuration;
		::TestSuite::ConfigData m_configData;
	};
} // namespace

namespace TestSuite
{
	//
	// MatsTestSuite
	//
	MatsTestSuite::MatsTestSuite(::TestSuite::TestSuiteConfigController& configController, ILogFile* appLog, ITestLogOutput* testOutput) :
		m_appLog{appLog, "MatsTestSuite"},
		m_runControlSignals{std::make_unique<ClientLib::AppSignalManager>(appLog)},
		m_appSignals{std::make_unique<ClientLib::AppSignalManager>(appLog)},
		m_configController{configController},
		m_testLog{std::make_unique<::TestSuite::TestLog>(testOutput)},
		m_runControl{std::make_unique<::TestSuite::RunControl>(appLog, m_testLog.get())}
	{
		// Create TestSuite object
		//
		CreateAdsControllerFunc createAdsControllerFunc = [this](const ::TestSuite::ConfigSettings& configuration)
		{
			auto ads = std::make_unique<::TestSuite::AdsInputController>(*m_appSignals, *m_appLog.logFile());
			ads->updateConnections(m_testSuite->softwareInfo(), configuration.appDataServices);
			return ads;
		};

		CreateTunControllerFunc createTunControllerFunc = [this](const ::TestSuite::ConfigSettings& configuration, QString username)
		{
			auto tuns = std::make_unique<::TestSuite::TunsOutputController>(m_appLog.logFile());
			tuns->setUserName(username);
			tuns->updateConnections(m_testSuite->softwareInfo(),
									configuration.tuningServices,
									configuration.tuningSignalsFile,
									TuningClientSettings::LmStatusFlagMode::None); // Access key?
			return tuns;
		};

		auto testControlThread =
			new MatsTestControlThread{appLog, m_testLog.get(), m_configController, createAdsControllerFunc, createTunControllerFunc};

		auto testControl = std::make_unique<::TestSuite::TestControl>(appLog, m_testLog.get(), testControlThread);

		m_testSuite =
			std::make_unique<::TestSuite::TestSuite>(m_configController.softwareInfo(), appLog, *m_testLog, std::move(testControl));

		// Connect signals
		//
		connect(m_testSuite.get(), &TestSuite::testStarted, this, &MatsTestSuite::testStarted);
		connect(m_testSuite.get(), &TestSuite::testFinished, this, &MatsTestSuite::testFinished);
		connect(m_testSuite.get(), &TestSuite::finished, this, &MatsTestSuite::finished);

		connect(m_runControl.get(), &RunControl::scriptPermissionChanged, this, &MatsTestSuite::scriptPermissionChanged);
		connect(m_runControl.get(), &RunControl::globalPermissionChanged, this, &MatsTestSuite::globalPermissionChanged);
		connect(m_runControl.get(), &RunControl::noPermissionsExist, this, &MatsTestSuite::noPermissionsExist);

		// Add input controller to RunControl(!) - Runs permission scripts.
		//
		{
			auto ads = std::make_unique<AdsInputController>(*m_runControlSignals, *m_appLog.logFile());

			// Update AdsInputController connection settings from TestSuiteConfigController.
			//
			connect(
				&m_configController,
				&TestSuiteConfigController::configurationArrived,
				this,
				[a = ads.get(), this](const ConfigSettings& configuration)
				{
					a->updateConnections(m_testSuite->softwareInfo(), configuration.appDataServices);
					m_appLog.writeMessage(
						tr("AppDataServices connection(s) updated (RunControl), %1 services.").arg(configuration.appDataServices.size()));
				});

			auto configuration = m_configController.configuration();
			if (configuration.isValid() == true)
			{
				ads->updateConnections(m_testSuite->softwareInfo(), configuration.appDataServices);
			}

			m_runControl->addInputController(std::move(ads));
		}

		return;
	}

	MatsTestSuite::~MatsTestSuite()
	{
		// Disconnect all signals so configurationArrived will not happen anymore,
		// and AdsInputController can be deleted safely.
		//
		m_configController.disconnect(this);

		stopRunControl();
		return;
	}

	bool MatsTestSuite::executeRunControl(const ::TestSuite::IScriptProvider& scriptProvider)
	{
		return m_runControl->execute(m_testSuite->softwareInfo(), scriptProvider, ControlParams{});
	}

	bool MatsTestSuite::hasRunControl()
	{
		return m_runControl->isRunning();
	}

	void MatsTestSuite::resetRunControl()
	{
		m_runControl->reset();
	}

	void MatsTestSuite::stopRunControl()
	{
		if (m_runControl->isRunning() == true)
		{
			m_runControl->stop();
		}
	}

	bool MatsTestSuite::execute(const ::TestSuite::ControlParams& controlParams)
	{
		// Get scripts from the CfgService
		//
		::TestSuite::IScriptProvider& scriptProvider = m_configController;
		return execute(scriptProvider, controlParams);
	}

	bool MatsTestSuite::execute(const ::TestSuite::IScriptProvider& scriptProvider, const ::TestSuite::ControlParams& controlParams)
	{
		// Make sure that confguration has been received
		//
		QElapsedTimer timer;
		timer.start();
		do
		{
			if (m_configController.getConnectionState().isConnected == false || m_configController.configuration().isValid() == false)
			{
				// Process the message loop for 100 ms; otherwise, we will not receive signals from CfgService.
				//
				QEventLoop loop;
				QTimer::singleShot(100, &loop, &QEventLoop::quit);
				loop.exec();
			}
			else
			{
				break;
			}

			if (timer.hasExpired(ServiceConnectTimeoutMs) == true)
			{
				m_appLog.writeError(tr("Configuration has not been received in %1 seconds!").arg(ServiceConnectTimeoutMs / 1000));
				if (m_configController.getConnectionState().isConnected == false)
				{
					m_appLog.writeError(
						tr("Connection to CfgService is not established whitin %1 seconds!").arg(ServiceConnectTimeoutMs / 1000));
				}

				if (m_configController.configuration().isValid() == false)
				{
					m_appLog.writeError(tr("Configuration is not valid!"));
				}

				return false;
			}
		} while (true);

		m_appLog.writeMessage(tr("Configuration received, tests about to start."));

		// Execute test suite
		//
		return m_testSuite->execute(scriptProvider, controlParams);
	}

	void MatsTestSuite::stop()
	{
		return m_testSuite->stop();
	}

	bool MatsTestSuite::isRunning() const
	{
		return m_testSuite->isRunning();
	}

	TestLog& MatsTestSuite::testLog()
	{
		return m_testSuite->testLog();
	}

	::TestSuite::ControlStatus MatsTestSuite::testStatus() const
	{
		return m_testSuite->testStatus();
	}

	::TestSuite::ControlStatus MatsTestSuite::runStatus() const
	{
		return m_runControl->status();
	}

	bool MatsTestSuite::scriptPermission(const QString& fileName) const
	{
		return m_runControl->scriptPermission(fileName);
	}

	bool MatsTestSuite::globalPermission() const
	{
		return m_runControl->globalPermission();
	}
} // namespace TestSuite