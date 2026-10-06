#include "SimVduModuleImpl.h"

#include "../SimulatorPrivate.h"
#include <HardwareLib/ModuleFirmware.h>

namespace
{
	struct ExternalSimProcesses
	{
		struct Node
		{
			QString sharedMemoryKey;
			QProcess process;
		};

		std::map<QString, std::unique_ptr<Node>> processes; // Key is a shared memory key.
		bool shutdownHandlerInstalled = false;

		// --
		//
		void installShutdownHandler()
		{
			auto* application = QCoreApplication::instance();
			if (application == nullptr || shutdownHandlerInstalled)
				return;

			QObject::connect(application,
							 &QCoreApplication::aboutToQuit,
							 application,
							 [this]()
							 {
								 stopAll();
							 });

			shutdownHandlerInstalled = true;
		}

		~ExternalSimProcesses()
		{
			// Do not call terminate(), waitForFinished(), or other Qt APIs here.
			//
			// stopAll() must already have been called by aboutToQuit().
			assert(processes.empty());
		}

		void stopAll()
		{
			for (auto& [_, node] : processes)
			{
				assert(node);

				QProcess& process = node->process;

				if (process.state() == QProcess::NotRunning)
				{
					continue;
				}

				process.terminate();

				if (process.waitForFinished(3000) == false)
				{
					process.kill();
					process.waitForFinished();
				}
			}

			// This destroys all QProcess objects while Qt is still alive.
			//
			processes.clear();
		}

		bool startProcess(const QString& sharedMemoryKey, const QString& simAppPath, const QStringList& arguments)
		{
			installShutdownHandler();

			if (processes.contains(sharedMemoryKey) == true &&
				processes[sharedMemoryKey]->process.state() == QProcess::ProcessState::Running)
			{
				// Activate already running process.
				//
				QSharedMemory sharedMemory{sharedMemoryKey};
				bool attached = sharedMemory.attach(QSharedMemory::ReadWrite);
				if (attached == false)
				{
					return false;
				}

				std::scoped_lock lock{sharedMemory};

				auto sharedData = static_cast<SimVduSharedData::SharedData*>(sharedMemory.data());
				if (sharedData == nullptr)
				{
					return false;
				}
				sharedData->simulatorState.updateCounter++;
				sharedData->simulatorState.bringToFront = 1;

				return true;
			}

			auto node = std::make_unique<Node>();
			node->sharedMemoryKey = sharedMemoryKey;
			node->process.start(simAppPath, arguments);

			if (node->process.waitForStarted() == false)
			{
				return false; // Failed to start the process.
			}

			processes[sharedMemoryKey] = std::move(node);
			return true;
		}
	};

	ExternalSimProcesses g_externalSimProcesses;
}; // namespace


namespace Sim
{
	//
	// class VduModuleImpl
	//
	VduModuleImpl::VduModuleImpl(SimulatorPrivate* simulator) :
		BaseModuleImpl{simulator}
	{
		m_log.setOutputScope("VduModule");
	}

	VduModuleImpl::~VduModuleImpl() = default;

	bool VduModuleImpl::load(const Hardware::LogicModuleInfo& lmInfo,
							 const LmDescription& lmDescription,
							 const Hardware::ModuleFirmware& firmware,
							 const ConnectionsImpl& connections,
							 const LogicModulesInfo& logicModulesExtraInfo)
	{
		m_log.setOutputScope(QString("VDU %1").arg(lmInfo.equipmentId));

		clear();

		m_logicModuleInfo = lmInfo;
		m_lmDescription = lmDescription;

		bool ok = true;

		if (m_lmDescription.flashMemory().m_configWriteBitstream == true)
		{
			bool loadEepromOk = loadEeprom(firmware, m_lmDescription.flashMemory().m_configUartId, &m_confEeprom);
			if (loadEepromOk == false)
			{
				m_log.writeError(tr("LogicModule: Loading Configuration EEPROM error"));
				ok = false;
			}
		}

		if (ok == false)
		{
			return false;
		}

		// Init DeviceEmulator
		//
		QString subsystemPath = m_simulator->buildPath() + "Subsytems/" + m_logicModuleInfo.subsystemId;
		DeviceError de = m_device.init(m_logicModuleInfo, m_lmDescription, m_confEeprom, subsystemPath, connections, logicModulesExtraInfo);

		if (de != DeviceError::Ok)
		{
			ok = false;
		}

		return ok;
	}

	void VduModuleImpl::clear()
	{
		m_log.writeDebug("Clear");

		m_lmDescription.clear();
		m_confEeprom.clear();
		m_device.clear();

		return;
	}

	QFuture<bool> VduModuleImpl::asyncRunCycle(std::chrono::microseconds currentTime,
											   const QDateTime& currentDateTime,
											   qint64 workcycle,
											   bool reset,
											   std::condition_variable& cvFinished)
	{
		if (reset == true)
		{
			m_device.reset();
		}

		auto f = [this, &cvFinished](std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle) -> bool
		{
			bool result = this->m_device.runWorkcycle(currentTime, currentDateTime, workcycle);
			cvFinished.notify_one();
			return result;
		};

		return QtConcurrent::run(f, currentTime, currentDateTime, workcycle);
	}

	bool VduModuleImpl::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		return m_device.receiveConnectionsData(currentTime);
	}

	bool VduModuleImpl::startSimAppProcess(const QString& simAppPath)
	{
		return g_externalSimProcesses.startProcess(sharedMemoryKey(), simAppPath, simAppArguments());
	}

	QStringList VduModuleImpl::simAppArguments() const
	{
		QStringList args;

		QDir dir{m_simulator->buildPath() + QDir::separator() + Directory::SUBSYSTEMS + QDir::separator() +
				 logicModuleExtraInfo().subsystemID};
		args << QString("--build=") + dir.path();
		args << QString("--key=%1").arg(moduleJumpers().jumpersValue);
		args << QString("--memory=%1").arg(sharedMemoryKey());

		return args;
	}

	const ::LogicModuleInfo& VduModuleImpl::logicModuleExtraInfo() const
	{
		return m_device.logicModuleExtraInfo();
	}

	void VduModuleImpl::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		m_device.setLogicModuleExtraInfo(value);
	}

	std::chrono::microseconds VduModuleImpl::cycleDuration() const
	{
		return std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration};
	}

	const Eeprom& VduModuleImpl::confEeprom() const
	{
		return m_confEeprom;
	}

	QString VduModuleImpl::sharedMemoryKey() const
	{
		return m_device.sharedMemoryKey();
	}

	RuntimeMode VduModuleImpl::runtimeMode() const
	{
		return m_device.runtimeMode();
	}

	DeviceState VduModuleImpl::deviceState() const
	{
		return m_device.deviceState();
	}

	bool VduModuleImpl::isPowerOff() const
	{
		return deviceState() == DeviceState::Off;
	}

	void VduModuleImpl::setPowerOff(bool value)
	{
		if (value == true)
		{
			if (isPowerOff() == false)
			{
				m_device.powerOff();
			}
		}
		else
		{
			m_device.reset();
		}
	}

	bool VduModuleImpl::discreteInputState(size_t index) const
	{
		return m_device.discreteInputState(index);
	}

	void VduModuleImpl::setDiscreteInputState(size_t index, bool value)
	{
		m_device.setDiscreteInputState(index, value);
	}

	bool VduModuleImpl::discreteInputValidity(size_t index) const
	{
		return m_device.discreteInputValidity(index);
	}

	void VduModuleImpl::setDiscreteInputValidity(size_t index, bool value)
	{
		m_device.setDiscreteInputValidity(index, value);
	}

	std::optional<bool> VduModuleImpl::discreteOutputState(size_t index) const
	{
		return m_device.discreteOutputState(index);
	}

	std::array<std::optional<bool>, SimVduSharedData::DiscreteOutputCount> VduModuleImpl::discreteOutputStates() const
	{
		return m_device.discreteOutputStates();
	}

	void VduModuleImpl::setDiscreteOutputState(size_t index, std::optional<bool> value)
	{
		m_device.setDiscreteOutputState(index, value);
	}
} // namespace Sim
