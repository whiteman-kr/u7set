#include "ApplicationLogicCompiler.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------
	//
	//	ApplicationLogicCompiler class implementation
	//

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

		findLMs();

		return compileModulesLogics();
	}


	// find all logic modules (LMs) in project
	// fills m_lm vector
	//
	void ApplicationLogicCompiler::findLMs()
	{
		m_lm.clear();

		findLM(m_equipment);

		if (m_lm.count() == 0)
		{
			m_log->writeMessage(tr("Logic modules (LMs) not found!"), true);
		}
		else
		{
			m_log->writeMessage(QString(tr("Found logic modules (LMs): %1")).arg(m_lm.count()), false);
		}
	}


	// find logic modules (LMs), recursive
	//
	void ApplicationLogicCompiler::findLM(Hardware::DeviceObject* startFromDevice)
	{
		if (startFromDevice == nullptr)
		{
			assert(startFromDevice != nullptr);

			msg = QString(tr("%1: DeviceObject null pointer!")).arg(__FUNCTION__);

			m_log->writeError(msg, false, true);

			qDebug() << msg;
			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Signal)
		{
			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Module)
		{
			Hardware::DeviceModule* module = reinterpret_cast<Hardware::DeviceModule*>(startFromDevice);

			if (module->moduleFamily() == Hardware::DeviceModule::FamilyType::LM)
			{
				Hardware::DeviceObject* parent = startFromDevice->parent();

				if (parent != nullptr)
				{
					if (parent->deviceType() == Hardware::DeviceType::Chassis)
					{
						// LM must be installed in the chassis
						//
						m_lm.append(reinterpret_cast<Hardware::DeviceModule*>(startFromDevice));
					}
					else
					{
						msg = QString(tr("LM %1 is not installed in the chassis")).arg(module->strId());

						m_log->writeWarning(msg, false, true);

						qDebug() << msg;
					}
				}
			}

			return;
		}

		int childrenCount = startFromDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = startFromDevice->child(i);

			findLM(device);
		}
	}


	bool ApplicationLogicCompiler::compileModulesLogics()
	{
		bool result = true;

		for(int i = 0; i < m_lm.count(); i++)
		{
			ModuleLogicCompiler moduleLogicCompiler(*this, m_lm[i]);

			result &= moduleLogicCompiler.run();
		}

		return result;
	}


	// ---------------------------------------------------------------------------------
	//
	//	ModuleLogicCompiler class implementation
	//

	ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm)
	{
		m_equipment = appLogicCompiler.m_equipment;
		m_signals = appLogicCompiler.m_signals;
		m_resultWriter = appLogicCompiler.m_resultWriter;
		m_log = appLogicCompiler.m_log;
		m_lm = lm;
	}


	bool ModuleLogicCompiler::run()
	{
		m_log->writeMessage("", false);

		msg = QString(tr("Compilation for LM %1 was started...")).arg(m_lm->strId());

		m_log->writeMessage(msg, false);

		bool result = true;

		// 0. Initialization

		result &= init();

		// 1. Copy DiagDataController memory to the registration

		result &= copyDiagData();

		// 2. Copy values of all input & output signals to the registration

		result &= copyInOutSignals();


		if (result == true)
		{
			msg = QString(tr("Compilation for LM %1 was successfully finished")).arg(m_lm->strId());
			m_log->writeSuccess(msg, false);
		}
		else
		{
			msg = QString(tr("Compilation for LM %1 was finished with errors")).arg(m_lm->strId());
			m_log->writeError(msg, false, false);
		}

		return result;
	}


	bool ModuleLogicCompiler::init()
	{
		int addr = 0;

		if (getIntProperty("MemorySettings\\WrAppLogicW", addr) == false )
		{
			return false;
		}

		m_regDataAddress.reset();
		m_regDataAddress.setBase(addr);

		return true;
	}


	bool ModuleLogicCompiler::copyDiagData()
	{
		int rdDiagData = 0;
		int diagDataSize = 0;

		if (getIntProperty("MemorySettings\\RdDiagData", rdDiagData) == false )
		{
			return false;;
		}

		if (getIntProperty("MemorySettings\\DiagDataSize", diagDataSize) == false )
		{
			return false;;
		}


		//m_code.addComand();


		return true;
	}

	bool ModuleLogicCompiler::copyInOutSignals()
	{
		return true;
	}


	bool ModuleLogicCompiler::getIntProperty(const char* propertyName, int& value)
	{
		return getIntProperty(m_lm, propertyName, value);
	}


	bool ModuleLogicCompiler::getIntProperty(Hardware::DeviceModule* module, const char* propertyName, int &value)
	{
		if (module == nullptr)
		{
			assert(module != nullptr);
			return false;
		}

		if (propertyName == nullptr)
		{
			assert(propertyName != nullptr);
			return false;
		}

		QList<QByteArray> bb = module->dynamicPropertyNames();

		for(auto i:bb)
		{
			qDebug() << i;
		}

		QVariant val = module->property(propertyName);

		if (val.isValid() == false)
		{
			m_log->writeError(QString(tr("Property %1 is not found in module %2")).arg(propertyName).arg(module->strId()), false, true);
			return false;
		}

		value = val.toInt();

		return true;
	}

}
