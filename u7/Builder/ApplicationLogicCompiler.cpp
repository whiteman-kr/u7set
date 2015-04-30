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

	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, AfblSet* afblSet,
													   ApplicationLogicData* appLogicData, BuildResultWriter* buildResultWriter, OutputLog *log) :
		m_equipment(equipment),
		m_signals(signalSet),
		m_afbl(afblSet),
		m_appLogicData(appLogicData),
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
			m_afbl == nullptr ||
			m_appLogicData == nullptr ||
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
		m_afbl = appLogicCompiler.m_afbl;
		m_appLogicData = appLogicCompiler.m_appLogicData;
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

		// Initialization

		result &= init();

		// Functionals Block's initialization

		result &= afbInitialization();

		// Copy DiagDataController memory to the registration

		result &= copyDiagData();

		// Copy values of all input & output signals to the registration

		result &= copyInOutSignals();

		// Generate Application Logic code

		result &= generateApplicationLogicCode();


		result &= writeResult();

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


	bool ModuleLogicCompiler::writeResult()
	{
		bool result = true;

		m_code.generateBinCode();

		QByteArray binCode;

		m_code.getBinCode(binCode);

		result &= m_resultWriter->addFile(m_lm->subSysID(), QString("%1.alc").arg(m_lm->caption()), binCode);

		QStringList asmCode;

		m_code.getAsmCode(asmCode);

		result = m_resultWriter->addFile(m_lm->subSysID(), QString("%1.asm").arg(m_lm->caption()), asmCode);

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

		std::shared_ptr<ApplicationLogicModule> appLogicModule = m_appLogicData->getModuleLogicData(m_lm->strId());

		m_moduleLogic = appLogicModule.get();

		if (m_moduleLogic == nullptr)
		{
			msg = QString(tr("Application logic not found for module %1")).arg(m_lm->strId());
			m_log->writeWarning(msg, false, true);
		}

		return true;
	}


	bool ModuleLogicCompiler::afbInitialization()
	{
		m_code.comment("Functional Blocks initialization code");
		m_code.newLine();

/*		for(AfbElement afbElement : m_afbl->items)
		{
			AlgFb fb(afbElement);
		}*/

		bool result = true;

		result &= getUsedAfbs();

		/*AlgFbParamArray param;

		AlgFbParam p;

		p.caption = "param1";
		p.index = 1;
		p.size = 16;
		p.value = 2;

		param.append(p);

		p.caption = "param2";
		p.index = 2;
		p.size = 1;
		p.value = 1;

		param.append(p);

		generateAfbInitialization(1, 1, param);*/

		return result;
	}


	bool ModuleLogicCompiler::getUsedAfbs()
	{
		// get all Afbs from algorithms
		//
		return true;
	}


	/*bool ModuleLogicCompiler::generateAfbInitialization(int fbType, int fbInstance, AlgFbParamArray& params)
	{
		m_code.newLine();

		Command command;

		for(AlgFbParam param : params)
		{
			command.writeFuncBlockConst(fbType, fbInstance, param.index, param.value);

			QString commentStr;

			commentStr.sprintf("%s <= #0x%04X", param.caption.toUtf8().data(), param.value);

			command.setComment(commentStr);

			m_code.append(command);
		}

		return true;
	}*/


	bool ModuleLogicCompiler::copyDiagData()
	{
		m_code.newLine();
		m_code.comment("Copy LM diagnostics data to registration");
		m_code.newLine();

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

		m_code.append(cmd);

		m_regDataAddress.addWord(diagDataSize);

		return true;
	}


	bool ModuleLogicCompiler::copyInOutSignals()
	{
		bool result = true;

		m_code.newLine();
		m_code.comment("Copy input/output signals to registration");
		m_code.newLine();

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


	bool ModuleLogicCompiler::generateApplicationLogicCode()
	{
		m_code.newLine();
		m_code.comment("Start of Application Logic code");
		m_code.newLine();

		Command cmd;

		cmd.stop();

		cmd.setComment("End of application logic code");

		m_code.append(cmd);

		return true;
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
