#include "ApplicationLogicCompiler.h"

namespace Builder
{

	// LM's memory settings
	//
	const char	*LM_REG_DATA_ADDRESS = "MemorySettings\\AppLogicW",
				*LM_DIAG_DATA = "MemorySettings\\DiagData",
				*LM_DIAG_DATA_SIZE = "MemorySettings\\DiagDataSize",
				*LM_IO_MODULE_DATA = "MemorySettings\\Module";		// + 2 character module place, like "02"


	// IO modules settings
	//
	const char	*IO_MODULE_DATA_SIZE = "IODataSize";


	const int	LM1_PLACE = 0,
				LM2_PLACE = 15,

				FIRST_MODULE_PLACE = 1,
				LAST_MODULE_PLCE = 14;



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
		m_log->writeEmptyLine();

		msg = QString(tr("Compilation for LM %1 was started...")).arg(m_lm->strId());

		m_log->writeMessage(msg, false);

		bool result = true;

		// 0. Initialization

		result &= init();

		// 1. Copy DiagDataController memory to the registration

		result &= copyDiagData();

		// 2. Copy values of all input & output signals to the registration

		result &= copyInOutSignals();

		Command cmd;

		cmd.nop();
		m_code.append(cmd);

		cmd.mov(12, 23);
		m_code.append(cmd);


		cmd.movConst(222, 23);
		m_code.append(cmd);

		cmd.movBitConst(1, 33, 3);
		m_code.append(cmd);


		cmd.writeFuncBlock(3333, 3, 1, 2);
		m_code.append(cmd);


		cmd.stop();
		cmd.setComment("End of programm");

		m_code.append(cmd);

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

		m_code.toString();

		return result;
	}


	bool ModuleLogicCompiler::init()
	{
		Hardware::DeviceObject* parent = m_lm->parent();

		if (parent->deviceType() != Hardware::DeviceType::Chassis)
		{
			msg = QString(tr("LM %1 must be installed in the chassis!")).arg(m_lm->strId());
			m_log->writeError(msg, false, true);
			return false;
		}

		m_chassis = reinterpret_cast<Hardware::DeviceChassis*>(parent);

		int addr = 0;

		if (getLMIntProperty(LM_REG_DATA_ADDRESS, addr) == false )
		{
			return false;
		}

		m_regDataAddress.reset();
		m_regDataAddress.setBase(addr);

		return true;
	}


	bool ModuleLogicCompiler::copyDiagData()
	{
		int diagData = 0;
		int diagDataSize = 0;

		if (getLMIntProperty(LM_DIAG_DATA, diagData) == false )
		{
			return false;;
		}

		if (getLMIntProperty(LM_DIAG_DATA_SIZE, diagDataSize) == false )
		{
			return false;;
		}

		Command cmd;

		cmd.movMem(diagData, m_regDataAddress.address(), diagDataSize);
		cmd.setComment(tr("Move LM diagnostics data to registration"));

		m_code.append(cmd);

		m_regDataAddress.addWord(diagDataSize);

		return true;
	}


	bool ModuleLogicCompiler::copyInOutSignals()
	{
		bool result = true;

		for(int place = FIRST_MODULE_PLACE; place <= LAST_MODULE_PLCE; place++)
		{
			Hardware::DeviceModule* module = getModuleOnPlace(place);

			if (module == nullptr || module->isIOModule() == false)
			{
				Comment c(QString(tr("No I/O module installed on place %1")).arg(place));

				m_code.append(c);
			}
			else
			{
				int ioDataSize = -1;
				int ioModuleData = -1;

				result &= getDeviceIntProperty(module, IO_MODULE_DATA_SIZE, ioDataSize);

				QString propertyName(LM_IO_MODULE_DATA);
				propertyName += QString().sprintf("%02d", place);

				result &= getLMIntProperty(propertyName.toUtf8().data(), ioModuleData);

				if (ioDataSize == -1 || ioModuleData == -1)
				{
					continue;
				}
				else
				{
					Command cmd;

					cmd.movMem(ioModuleData, m_regDataAddress.address(), ioDataSize);

					cmd.setComment(QString(tr("Move I/O module data (place %1) to registration")).arg(place));

					m_code.append(cmd);

					m_regDataAddress.addWord(ioDataSize);

				}
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::getLMIntProperty(const char* propertyName, int& value)
	{
		return getDeviceIntProperty(m_lm, propertyName, value);
	}


	bool ModuleLogicCompiler::getDeviceIntProperty(Hardware::DeviceObject* device, const char* propertyName, int &value)
	{
		if (device == nullptr)
		{
			assert(device != nullptr);
			return false;
		}

		if (propertyName == nullptr)
		{
			assert(propertyName != nullptr);
			return false;
		}

		QList<QByteArray> bb = device->dynamicPropertyNames();

		for(auto i:bb)
		{
			qDebug() << i;
		}

		QVariant val = device->property(propertyName);

		if (val.isValid() == false)
		{
			m_log->writeError(QString(tr("Property %1 is not found in device %2")).arg(propertyName).arg(device->strId()), false, true);
			return false;
		}

		value = val.toInt();

		return true;
	}


	Hardware::DeviceModule* ModuleLogicCompiler::getModuleOnPlace(int place)
	{
		if (m_chassis == nullptr)
		{
			assert(false);
			return nullptr;
		}

		int count = m_chassis->childrenCount();

		for(int i = 0; i < count; i++)
		{
			Hardware::DeviceObject* device = m_chassis->child(i);

			if (device == nullptr)
			{
				assert(false);
				continue;
			}

			if (device->deviceType() != Hardware::DeviceType::Module ||
				device->place() != place)
			{
				continue;
			}

			return reinterpret_cast<Hardware::DeviceModule*>(device);
		}

		return nullptr;
	}

}
