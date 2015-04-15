#include "ApplicationLogicCompiler.h"

namespace Builder
{
	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, BuildResultWriter* buildResultWriter, OutputLog *log) :
		m_equipment(equipment),
		m_signals(signalSet),
		m_resultWriter(buildResultWriter),
		m_log(log)
	{
	}


	bool ApplicationLogicCompiler::run()
	{
		if (m_log == nullptr)
		{
			assert(m_log != nullptr);
			return false;
		}

		if (m_equipment == nullptr ||
			m_signals == nullptr ||
			m_resultWriter == nullptr)
		{
			msg = tr("%1: Invalid params. Compilation aborted.").arg(__FUNCTION__);

			m_log->writeError(msg, true, true);

			qDebug() << msg;

			return false;
		}

		m_signals->resetAddresses();

		findLMs(m_equipment);

		if (m_lm.count() == 0)
		{
			m_log->writeMessage(tr("Logic modules (LMs) not found!"), true);
		}
		else
		{
			m_log->writeMessage(QString(tr("Found logic modules (LMs): %1")).arg(m_lm.count()), false);
		}
	}


	// find all logic modules (LMs) in project
	// fills m_lm vector
	//
	void ApplicationLogicCompiler::findLMs(Hardware::DeviceObject* startFromDevice)
	{
		if (startFromDevice == nullptr)
		{
			assert(startFromDevice != nullptr);

			msg = QString(tr("DeviceObject null pointer!")).arg(__FUNCTION__);

			m_log->writeError(msg, false, true);

			qDebug() << msg;
			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Module)
		{
			Hardware::DeviceModule* module = reinterpret_cast<Hardware::DeviceModule*>(startFromDevice);

			if (module->type() == 1 )
			{
				m_lm.append(startFromDevice);
				return;
			}
		}

		int childrenCount = startFromDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = startFromDevice->child(i);

			findLMs(device);
		}
	}
}
