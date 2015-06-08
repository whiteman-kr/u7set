#include "ApplicationLogicCompiler.h"

namespace Builder
{

#define ASSERT_RESULT_FALSE_BREAK	assert(false); \
									result = false; \
									break;

#define ASSERT_RETURN_FALSE			assert(false); \
									return false;

	// Signals properties
	//
	const char	*VALUE_OFFSET = "ValueOffset",
				*VALUE_BIT = "ValueBit";

	const char* SECTION_MEMORY_SETTINGS = "MemorySettings";


	const int ERR_VALUE = -1;


	const int	LM1_PLACE = 0,
				LM2_PLACE = 15,

				FIRST_MODULE_PLACE = 1,
				LAST_MODULE_PLACE = 14;


	// ---------------------------------------------------------------------------------
	//
	//	ApplicationLogicCompiler class implementation
	//

	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, Afbl::AfbElementCollection *afblSet,
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

	ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm) :
		m_appSignals(*this)
	{
		m_equipment = appLogicCompiler.m_equipment;
		m_signals = appLogicCompiler.m_signals;
		m_afbl = appLogicCompiler.m_afbl;
		m_appLogicData = appLogicCompiler.m_appLogicData;
		m_resultWriter = appLogicCompiler.m_resultWriter;
		m_log = appLogicCompiler.m_log;
		m_lm = lm;

		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::OTHER, "OTHER");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::LM, "LM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AIM, "AIM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AOM, "AOM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::DIM, "DIM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::DOM, "DOM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AIFM, "AIFM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::OCM, "OCM");
	}


	const Signal* ModuleLogicCompiler::getSignal(const QString& strID)
	{
		if (m_signalsStrID.contains(strID))
		{
			return m_signalsStrID.value(strID);
		}

		return nullptr;
	}


	bool ModuleLogicCompiler::run()
	{
		Hardware::DeviceObject* parent = m_lm->parent();

		if (parent->deviceType() != Hardware::DeviceType::Chassis)
		{
			msg = QString(tr("LM %1 must be installed in the chassis!")).arg(m_lm->strId());
			m_log->writeError(msg, false, true);
			return false;
		}

		m_chassis = dynamic_cast<Hardware::DeviceChassis*>(parent);

		if (m_chassis == nullptr)
		{
			assert(false);
			return false;
		}

		m_log->writeEmptyLine();

		msg = QString(tr("Compilation for LM %1 was started...")).arg(m_lm->strId());

		m_log->writeMessage(msg, false);

		bool result = false;

		do
		{
			if (!loadLMSettings()) break;

			if (!loadModulesSettings()) break;

			if (!prepareAppLogicGeneration()) break;

			if (!initAfbs()) break;

			if (!copyLMDiagDataToRegBuf()) break;

			if (!copyInModulesAppLogicDataToRegBuf()) break;

			if (!initOutModulesAppLogicDataInRegBuf()) break;

			if (!generateAppLogicCode()) break;

			if (!copyDiscreteSignalsToRegBuf()) break;

			if (!copyOutModulesAppLogicDataToModulesMemory()) break;

			if (!finishLMCode()) break;

			if (!writeResult()) break;

			result = true;
		}
		while(false);

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

		cleanup();

		return result;
	}


	bool ModuleLogicCompiler::loadLMSettings()
	{
		bool result = true;

		const PropertyNameVar memSettings[] =
		{
			{	"ModuleDataOffset", &m_moduleDataOffset },
			{	"ModuleDataSize", &m_moduleDataSize },

			{	"OptoInterfaceDataOffset", &m_optoInterfaceDataOffset },
			{	"OptoInterfaceDataSize", &m_optoInterfaceDataSize },

			{	"AppLogicBitDataOffset", &m_appLogicBitDataOffset },
			{	"AppLogicBitDataSize", &m_appLogicBitDataSize },

			{	"TuningDataOffset", &m_tuningDataOffset },
			{	"TuningDataSize", &m_tuningDataSize },

			{	"AppLogicWordDataOffset", &m_appLogicWordDataOffset },
			{	"AppLogicWordDataSize", &m_appLogicWordDataSize },

			{	"LMDiagDataOffset", &m_LMDiagDataOffset },
			{	"LMDiagDataSize", &m_LMDiagDataSize },

			{	"LMIntOutDataOffset", &m_LMIntOutDataOffset },
			{	"LMIntOutDataSize", &m_LMIntOutDataSize }
		};

		for(PropertyNameVar memSetting : memSettings)
		{
			result &= getLMIntProperty(SECTION_MEMORY_SETTINGS, memSetting.name, memSetting.var);
		}

		if (result)
		{
			m_log->writeMessage(QString(tr("Loading LM's settings... Ok")), false);
		}
		else
		{
			m_log->writeError(QString(tr("LM settings are not loaded")), false, true);
		}

		return true;
	}


	bool ModuleLogicCompiler::loadModulesSettings()
	{
		bool result = true;

		m_modules.clear();

		int moduleAppDataOffset = m_appLogicWordDataOffset + m_LMDiagDataSize;

		// build Module structures array
		//
		for(int place = FIRST_MODULE_PLACE; place <= LAST_MODULE_PLACE; place++)
		{
			Module m;

			int placeIndex = place - 1;

			Hardware::DeviceModule* device = getModuleOnPlace(place);

			if (device == nullptr)
			{
				continue;
			}

			m.device = device;
			m.place = place;

			const PropertyNameVar moduleSettings[] =
			{
				{	"TxDataSize", &m.txDataSize },
				{	"RxDataSize", &m.rxDataSize },

				{	"DiagDataOffset", &m.diagDataOffset },
				{	"DiagDataSize", &m.diagDataSize },

				{	"AppLogicDataOffset", &m.appLogicDataOffset },
				{	"AppLogicDataSize", &m.appLogicDataSize },
				{	"AppLogicDataSizeWithReserve", &m.appLogicDataSizeWithReserve }
			};

			for(PropertyNameVar moduleSetting : moduleSettings)
			{
				result &= getDeviceIntProperty(device, SECTION_MEMORY_SETTINGS, moduleSetting.name, moduleSetting.var);
			}

			m.rxTxDataOffset = m_moduleDataOffset + m_moduleDataSize * placeIndex;
			m.moduleAppDataOffset = m.rxTxDataOffset + m.appLogicDataOffset;
			m.appDataOffset = moduleAppDataOffset;

			moduleAppDataOffset += m.appLogicDataSize;

			m_modules.append(m);
		}

		m_registeredInternalAnalogSignalsOffset = moduleAppDataOffset;
		m_registeredInternalDiscreteSignalsOffset = m_appLogicBitDataOffset;

		// the actual values of:
		//
		// m_registeredInternalAnalogSignalsSize
		// m_internalAnalogSignalsOffset
		// m_internalAnalogSignalsSize
		// m_registeredInternalDiscreteSignalsSize
		// m_internalDiscreteSignalsOffset
		// m_internalDiscreteSignalsSize
		// m_regBufferInternalDiscreteSignalsOffset
		// m_regBufferInternalDiscreteSignalsSize
		//
		// will be calculated later in calculateInternalSignalsAddresses function

		if (result)
		{
			m_log->writeMessage(QString(tr("Loading modules settings... Ok")), false);
		}
		else
		{
			m_log->writeError(QString(tr("Modules settings are not loaded")), false, true);
		}

		return result;
	}


	bool ModuleLogicCompiler::prepareAppLogicGeneration()
	{
		bool result = false;

		std::shared_ptr<ApplicationLogicModule> appLogicModule = m_appLogicData->getModuleLogicData(m_lm->strId());

		m_moduleLogic = appLogicModule.get();

		if (m_moduleLogic == nullptr)
		{
			msg = QString(tr("Application logic not found for module %1")).arg(m_lm->strId());
			m_log->writeWarning(msg, false, true);

			return true;
		}

		do
		{
			if (!buildServiceMaps()) break;

			if (!createAppSignalsMap()) break;

			if (!calculateInOutSignalsAddresses()) break;

			if (!calculateInternalSignalsAddresses()) break;

			result = true;
		}
		while(false);

		return result;
	}


	bool ModuleLogicCompiler::initAfbs()
	{
		if (m_moduleLogic == nullptr)
		{
			return true;			// nothing to init
		}

		m_log->writeMessage(QString(tr("Generation of AFB initialization code...")), false);

		bool result = true;

		m_code.newLine();
		m_code.comment("Functional Blocks initialization code");

		for(Afb* fbl : m_afbs)
		{
			for(AppFb* appFb : m_appFbs)
			{
				if (appFb->afbStrID() != fbl->strID())
				{
					continue;
				}

				if (!appFb->hasRam())
				{
					// FB without RAM initialize once for all instances
					// initialize instantiator params only
					//
					result &= initAppFbParams(appFb, true);
					break;
				}
				else
				{
					// initialize all params for each instance of FB with RAM
					//
					result &= initAppFbParams(appFb, false);
				}
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::initAppFbParams(AppFb* appFb, bool instantiatorOnly)
	{
		bool result = true;

		if (appFb == nullptr)
		{
			assert(false);
			return false;
		}

		const Afbl::AfbElement& afb = appFb->afb();

		quint16 fbOpcode = appFb->opcode();
		quint16 fbInstance = appFb->instance();

		m_code.newLine();
		m_code.comment(QString(tr("Initialization of %1 instance %2")).arg(afb.strID()).arg(fbInstance));
		m_code.newLine();

		// iniitalization of constant params
		//
		for(Afbl::AfbElementParam afbParam : afb.params())
		{
			if (instantiatorOnly && !afbParam.instantiator())
			{
				continue;
			}

			Command cmd;

			quint16 paramValue = 0;

			QVariant qv = afbParam.value();

			switch(afbParam.type())
			{
			case Afbl::AnalogIntegral:
				paramValue = qv.toInt();
#pragma message("###################################### Need calculate value!!!!!!!!! ####################")
				break;

			case Afbl::AnalogFloatingPoint:
				assert(false);				// not implemented
				break;

			case Afbl::DiscreteValue:
				paramValue = qv.toInt();
				break;

			default:
				assert(false);
			}


			cmd.writeFuncBlockConst(fbOpcode, fbInstance, afbParam.operandIndex(), paramValue);

			cmd.setComment(QString("%1 <= %2").arg(afbParam.caption()).arg(paramValue));

			m_code.append(cmd);
		}

		return result;
	}


	bool ModuleLogicCompiler::copyLMDiagDataToRegBuf()
	{
		m_code.newLine();
		m_code.comment("Copy LM diagnostics data to RegBuf");
		m_code.newLine();

		Command cmd;

		cmd.movMem(m_appLogicWordDataOffset, m_LMDiagDataOffset, m_LMDiagDataSize);

		m_code.append(cmd);

		return true;
	}


	bool ModuleLogicCompiler::copyInModulesAppLogicDataToRegBuf()
	{
		m_code.newLine();
		m_code.comment("Copy input modules application logic data to RegBuf");
		m_code.newLine();

		for(Module module : m_modules)
		{
			if (!module.device->isInputModule())
			{
				continue;
			}

			Command cmd;

			cmd.movMem(module.appDataOffset, module.moduleAppDataOffset, module.appLogicDataSize);

			cmd.setComment(QString(tr("copy %1 data (place %2) to RegBuf")).arg(getModuleFamilyTypeStr(module.familyType())).arg(module.place));

			m_code.append(cmd);
		}

		return true;
	}


	bool ModuleLogicCompiler::initOutModulesAppLogicDataInRegBuf()
	{
		m_code.newLine();
		m_code.comment("Init output modules application logic data in RegBuf");
		m_code.newLine();

		for(Module module : m_modules)
		{
			if (!module.device->isOutputModule())
			{
				continue;
			}

			Command cmd;

			cmd.setMem(module.appDataOffset, module.appLogicDataSize, 0);

			cmd.setComment(QString(tr("init %1 data (place %2) in RegBuf")).arg(getModuleFamilyTypeStr(module.familyType())).arg(module.place));

			m_code.append(cmd);
		}

		return true;
	}


	bool ModuleLogicCompiler::generateAppLogicCode()
	{
		m_log->writeMessage(QString("Generation of application logic code was started..."), false);

		bool result = true;

		m_code.newLine();
		m_code.comment("Application logic code");

		for(AppItem* appItem : m_appItems)
		{
			if (appItem == nullptr)
			{
				assert(false);
				return false;
			}

			if (appItem->isSignal())
			{
				// appItem is signal
				//
			}
			else
			{
				// appItem is functional block
				//

				if (m_appFbs.contains(appItem->guid()))
				{
					result &= generateFbCode(m_appFbs[appItem->guid()]);
				}
				else
				{
					assert(false);			// FB with appItem->guid() was not found!
					result = false;
				}
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::generateFbCode(const AppFb* appFb)
	{
		bool result = false;

		m_code.newLine();

		do
		{
			if (!writeFbInputSignals(appFb)) break;

			Command cmd;

			cmd.setComment(appFb->afbStrID());

			cmd.start(appFb->opcode(), appFb->instance());

			m_code.append(cmd);

			if (!readFbOutputSignals(appFb)) break;

			result = true;
		}
		while(false);

		return result;
	}


	bool ModuleLogicCompiler::writeFbInputSignals(const AppFb* appFb)
	{
		bool result = true;

		quint16 fbType = appFb->opcode();
		quint16 fbInstance = appFb->instance();

		for(LogicPin inPin : appFb->inputs())
		{
			if (inPin.dirrection() != VFrame30::ConnectionDirrection::Input)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			quint16 fbParamNo = inPin.afbOperandIndex();

			int connectedPinsCount = 1;

			for(QUuid connectedPinGuid : inPin.associatedIOs())
			{
				if (connectedPinsCount > 1)
				{
					m_log->writeError(QString(tr("More than one pin is connected to the input")), false, true);

					ASSERT_RESULT_FALSE_BREAK
				}

				connectedPinsCount++;

				if (!m_pinParent.contains(connectedPinGuid))
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				AppItem* connectedPinParent = m_pinParent[connectedPinGuid];

				if (connectedPinParent == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				QUuid signalGuid;

				if (connectedPinParent->isSignal())
				{
					// input connected to real signal
					//
					signalGuid = connectedPinParent->guid();
				}
				else
				{
					// connectedPinParent is FB
					//
					if (!m_outPinSignal.contains(connectedPinGuid))
					{
						m_log->writeError(QString(tr("Output pin is not found, GUID: %1")).arg(connectedPinGuid.toString()), false, true);

						ASSERT_RESULT_FALSE_BREAK
					}

					signalGuid = m_outPinSignal[connectedPinGuid];
				}

				if (!m_appSignals.contains(signalGuid))
				{
					m_log->writeError(QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()), false, true);

					ASSERT_RESULT_FALSE_BREAK
				}

				AppSignal* appSignal = m_appSignals[signalGuid];

				if (appSignal == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				if (!appSignal->isComputed())
				{
					m_log->writeError(QString(tr("Signal value undefined: %1")).arg(appSignal->strID()), false, true);

					ASSERT_RESULT_FALSE_BREAK
				}

				Command cmd;

				int ramAddrOffset = appSignal->ramAddr().offset();
				int ramAddrBit = appSignal->ramAddr().bit();

				if (ramAddrOffset == -1 || ramAddrBit == -1)
				{
					assert(false);		// signal ramAddr is not calculated!!!
				}
				else
				{
					if (appSignal->isAnalog())
					{
						// input connected to analog signal
						//
						cmd.writeFuncBlock(fbType, fbInstance, fbParamNo, ramAddrOffset);
					}
					else
					{
						// input connected to discrete signal
						//
						cmd.writeFuncBlockBit(fbType, fbInstance, fbParamNo, ramAddrOffset, ramAddrBit);
					}

					cmd.setComment(QString(tr("<= %1")).arg(appSignal->strID()));

					m_code.append(cmd);
				}
			}

			if (result == false)
			{
				break;
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::readFbOutputSignals(const AppFb* appFb)
	{
		bool result = true;

		quint16 fbType = appFb->opcode();
		quint16 fbInstance = appFb->instance();

		for(LogicPin outPin : appFb->outputs())
		{
			if (outPin.dirrection() != VFrame30::ConnectionDirrection::Output)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			quint16 fbParamNo = outPin.afbOperandIndex();

			int connectedSignals = 0;

			for(QUuid connectedPinGuid : outPin.associatedIOs())
			{
				if (!m_pinParent.contains(connectedPinGuid))
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				AppItem* connectedPinParent = m_pinParent[connectedPinGuid];

				if (connectedPinParent == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				if (connectedPinParent->isFb())
				{
					continue;
				}

				assert(connectedPinParent->isSignal());

				QUuid signalGuid;

				// output connected to real signal
				//
				signalGuid = connectedPinParent->guid();

				connectedSignals++;

				result &= generateReadFuncBlockToSignalCode(fbType, fbInstance, fbParamNo, signalGuid);
			}

			if (connectedSignals == 0)
			{
				// output pin is not connected to signal
				// save FB output value to shadow signal with GUID == outPin.guid()
				//
				result &= generateReadFuncBlockToSignalCode(fbType, fbInstance, fbParamNo, outPin.guid());
			}

			if (result == false)
			{
				break;
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::generateReadFuncBlockToSignalCode(quint16 fbType, quint16 fbInstance, quint16 fbParamNo, QUuid signalGuid)
	{
		if (!m_appSignals.contains(signalGuid))
		{
			m_log->writeError(QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()), false, true);

			ASSERT_RETURN_FALSE
		}

		AppSignal* appSignal = m_appSignals[signalGuid];

		if (appSignal == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		Command cmd;

		int ramAddrOffset = appSignal->ramAddr().offset();
		int ramAddrBit = appSignal->ramAddr().bit();

		if (ramAddrOffset == -1 || ramAddrBit == -1)
		{
			ASSERT_RETURN_FALSE		// signal ramAddr is not calculated!!!
		}
		else
		{
			if (appSignal->isAnalog())
			{
				// input connected to analog signal
				//
				cmd.readFuncBlock(ramAddrOffset, fbType, fbInstance, fbParamNo);
			}
			else
			{
				// input connected to discrete signal
				//
				cmd.readFuncBlockBit(ramAddrOffset, ramAddrBit, fbType, fbInstance, fbParamNo);
			}

			cmd.setComment(QString(tr("=> %1")).arg(appSignal->strID()));

			m_code.append(cmd);
		}

		appSignal->setComputed();

		return true;
	}


	bool ModuleLogicCompiler::copyDiscreteSignalsToRegBuf()
	{
		if (m_registeredInternalDiscreteSignalsSize == 0)
		{
			return true;
		}

		m_code.newLine();
		m_code.comment("Copy internal discrete signals from bit-addressed memory to RegBuf");
		m_code.newLine();

		Command cmd;

		cmd.movMem(m_regBufferInternalDiscreteSignalsOffset,
				   m_registeredInternalDiscreteSignalsOffset, m_registeredInternalDiscreteSignalsSize);

		m_code.append(cmd);

		return true;
	}


	bool ModuleLogicCompiler::copyOutModulesAppLogicDataToModulesMemory()
	{
		m_code.newLine();
		m_code.comment("Copy output modules application logic data to modules memory");
		m_code.newLine();

		for(Module module : m_modules)
		{
			if (!module.device->isOutputModule())
			{
				continue;
			}

			Command cmd;

			cmd.movMem(module.moduleAppDataOffset, module.appDataOffset, module.appLogicDataSize);

			cmd.setComment(QString(tr("copy %1 data (place %2) to modules memory")).arg(getModuleFamilyTypeStr(module.familyType())).arg(module.place));

			m_code.append(cmd);

			if (module.appLogicDataSize < module.appLogicDataSizeWithReserve)
			{
				cmd.setMem(module.moduleAppDataOffset + module.appLogicDataSize, module.appLogicDataSizeWithReserve - module.appLogicDataSize, 0);
				cmd.setComment(QString(tr("set reserv data to 0")));

				m_code.append(cmd);
			}
		}

		return true;
	}


	bool ModuleLogicCompiler::finishLMCode()
	{
		m_code.newLine();
		m_code.comment("End of program");
		m_code.newLine();

		Command cmd;

		cmd.stop();

		m_code.append(cmd);

		return true;
	}


	bool ModuleLogicCompiler::writeResult()
	{
		bool result = true;

		m_code.generateBinCode();

		/*QByteArray binCode;

		m_code.getBinCode(binCode);

		result &= m_resultWriter->addFile(m_lm->subSysID(), QString("%1.alc").arg(m_lm->caption()), binCode);*/

		QStringList mifCode;

		m_code.getMifCode(mifCode);

		result &= m_resultWriter->addFile(m_lm->subSysID(), QString("%1.mif").arg(m_lm->caption()), mifCode);

		QStringList asmCode;

		m_code.getAsmCode(asmCode);

		result = m_resultWriter->addFile(m_lm->subSysID(), QString("%1.asm").arg(m_lm->caption()), asmCode);

		//

		writeLMCodeTestFile();

		//

		return result;
	}


	void ModuleLogicCompiler::writeLMCodeTestFile()
	{
		ApplicationLogicCode m_testCode;

		Command cmd;

		cmd.nop();
		m_testCode.append(cmd);

		cmd.start(1, 1);
		m_testCode.append(cmd);

		cmd.mov(200, 400);
		m_testCode.append(cmd);

		cmd.movMem(300, 100, 50);
		m_testCode.append(cmd);

		cmd.movConst(50, 123);
		m_testCode.append(cmd);

		cmd.movBitConst(51, 2, 1);
		m_testCode.append(cmd);

		cmd.writeFuncBlock(1, 3, 4, 10);
		m_testCode.append(cmd);

		cmd.readFuncBlock(77, 1, 4, 5);
		m_testCode.append(cmd);

		cmd.writeFuncBlockConst(1, 2, 3, 55);
		m_testCode.append(cmd);

		cmd.writeFuncBlockBit(1, 2, 3, 12, 7);
		m_testCode.append(cmd);

		cmd.readFuncBlockBit(88, 9, 1, 3, 4);
		m_testCode.append(cmd);

		cmd.readFuncBlockTest(1, 2, 3, 1);
		m_testCode.append(cmd);

		cmd.setMem(500, 10, 22);
		m_testCode.append(cmd);

		cmd.moveBit(20, 1, 30, 2);
		m_testCode.append(cmd);

		cmd.stop();
		m_testCode.append(cmd);

		m_testCode.generateBinCode();

		QStringList mifCode;

		m_testCode.getMifCode(mifCode);

		m_resultWriter->addFile(m_lm->subSysID(), QString("lm_test_code.mif"), mifCode);
	}


	bool ModuleLogicCompiler::buildServiceMaps()
	{
		m_afbs.clear();

		for(std::shared_ptr<LogicAfb> afbl : m_afbl->elements())
		{
			m_afbs.insert(afbl);
		}

		m_appItems.clear();
		m_pinParent.clear();

		for(const AppLogicItem& logicItem : m_moduleLogic->items())
		{
			// build QHash<QUuid, AppItem*> m_appItems
			// item GUID -> item ptr
			//
			if (m_appItems.contains(logicItem.m_fblItem->guid()))
			{
				assert(false);	// guid already in map!
				continue;
			}

			AppItem* appItem = new AppItem(logicItem);

			m_appItems.insert(appItem->guid(), appItem);

			// build QHash<QUuid, LogicItem*> m_itemsPins;
			// pin GUID -> parent item ptr
			//

			// add input pins
			//
			for(LogicPin input : appItem->inputs())
			{
				if (m_pinParent.contains(input.guid()))
				{
					assert(false);	// guid already in map!
					continue;
				}

				m_pinParent.insert(input.guid(), appItem);
			}

			// add output pins
			//
			for(LogicPin output : appItem->outputs())
			{
				if (m_pinParent.contains(output.guid()))
				{
					assert(false);	// guid already in map!
					continue;
				}

				m_pinParent.insert(output.guid(), appItem);
			}
		}

		return true;
	}


	bool ModuleLogicCompiler::createAppSignalsMap()
	{
		if (m_moduleLogic == nullptr)
		{
			assert(false);
			return false;
		}

		int count = m_signals->count();

		for(int i = 0; i < count; i++)
		{
			Signal* s = &(*m_signals)[i];

			if (m_signalsStrID.contains(s->strID()))
			{
				msg = QString(tr("Duplicate signal identifier: %1")).arg(s->strID());
				m_log->writeWarning(msg, false, true);
			}
			else
			{
				m_signalsStrID.insert(s->strID(), s);
			}

			if (!s->deviceStrID().isEmpty())
			{
				m_deviceBoundSignals.insertMulti(s->deviceStrID(), s);
			}
		}

		m_appSignals.clear();
		m_outPinSignal.clear();

		// find signals in algorithms
		// build map: signal GUID -> ApplicationSignal
		//

		for(AppItem* item : m_appItems)
		{
			if (!item->isSignal())
			{
				continue;
			}

			m_appSignals.insert(item);
		}

		// find fbl's outputs, which NOT connected to signals
		// create and add to m_appSignals map 'shadow' signals
		//

		for(AppItem* item : m_appItems)
		{
			if (!item->isFb())
			{
				continue;
			}

			// get Functional Block instance !!!
			//
			int instance = m_afbs.addInstance(item);

			m_appFbs.insert(item, instance);

			for(LogicPin output : item->outputs())
			{
				bool connectedToFbl = false;
				bool connectedToSignal = false;

				for(QUuid connectedPinUuid : output.associatedIOs())
				{
					if (!m_pinParent.contains(connectedPinUuid))
					{
						assert(false);		// pin not found!!!
					}
					else
					{
						AppItem* connectedAppItem = m_pinParent[connectedPinUuid];

						if (connectedAppItem->isFb())
						{
							connectedToFbl = true;
						}
						else
						{
							if (connectedAppItem->isSignal())
							{
								connectedToSignal = true;

								m_outPinSignal.insert(output.guid(), connectedAppItem->signal().guid());
							}
						}
					}
				}

				if (connectedToFbl && !connectedToSignal)
				{
					// create shadow signal with Uuid of this output pin
					//
					m_appSignals.insert(item, output);

					// output pin connected to shadow signal with same guid
					//
					m_outPinSignal.insert(output.guid(), output.guid());
				}
			}
		}

		return true;
	}


	bool ModuleLogicCompiler::calculateInOutSignalsAddresses()
	{
		m_log->writeMessage(QString(tr("Input & Output signals addresses calculation...")), false);

		bool result = true;

		for(const Module& module : m_modules)
		{
			if (module.device == nullptr)
			{
				assert(false);
				return false;
			}

			// calculate addresses of signals bound to module In/Out
			//

			std::vector<std::shared_ptr<Hardware::DeviceSignal>> moduleSignals = module.device->getAllSignals();

			for(std::shared_ptr<Hardware::DeviceSignal>& deviceSignal : moduleSignals)
			{
				if (!m_deviceBoundSignals.contains(deviceSignal->strId()))
				{
					continue;
				}

				QList<Signal*> boundSignals = m_deviceBoundSignals.values(deviceSignal->strId());

				if (boundSignals.count() > 1)
				{
					m_log->writeWarning(QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()), false, true);
				}

				for(Signal* signal : boundSignals)
				{
					if (signal == nullptr)
					{
						assert(false);
						continue;
					}

					int signalOffset = ERR_VALUE;
					int bit = ERR_VALUE;

					getDeviceIntProperty(deviceSignal.get(), QString(VALUE_OFFSET), &signalOffset);
					getDeviceIntProperty(deviceSignal.get(), QString(VALUE_BIT), &bit);

					if (signalOffset != ERR_VALUE && bit != ERR_VALUE)
					{
						// !!! signal - pointer to Signal objects in build-time SignalSet (ModuleLogicCompiler::m_signals member) !!!
						//
						Address16 ramRegAddr(module.appDataOffset + signalOffset, bit);

						signal->ramAddr() = ramRegAddr;
						signal->regAddr() = ramRegAddr;

						// set same ramAddr & regAddr for corresponding signals in m_appSignals map
						//
						AppSignal* appSignal = m_appSignals.getByStrID(signal->strID());

						if (appSignal != nullptr)
						{
							// not all device-bound signals must be in m_appSignals map
							//
							appSignal->ramAddr() = ramRegAddr;
							appSignal->regAddr() = ramRegAddr;
						}
					}
					else
					{
						m_log->writeError(QString(tr("Can't calculate RAM address of application signal %1")).arg(signal->strID()), false, true);
					}
				}
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::calculateInternalSignalsAddresses()
	{
		m_log->writeMessage(QString(tr("Internal signals addresses calculation...")), false);

		bool result = true;

		m_registeredInternalAnalogSignalsSize = 0;
		m_internalAnalogSignalsSize = 0;

		m_registeredInternalDiscreteSignalsSize = 0;
		m_registeredInternalDiscreteSignalsCount = 0;

		m_internalDiscreteSignalsSize = 0;
		m_internalDiscreteSignalsCount = 0;

		m_regBufferInternalDiscreteSignalsOffset = 0;
		m_regBufferInternalDiscreteSignalsSize = 0;

		// calculate internal analog & discrete signals sizes
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			if (appSignal == nullptr)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			if (!appSignal->isInternal())
			{
				continue;
			}

			if (appSignal->isAnalog())
			{
				// analog signal
				//
				if (appSignal->isRegistered())
				{
					m_registeredInternalAnalogSignalsSize += appSignal->sizeInWords();
				}
				else
				{
					m_internalAnalogSignalsSize += appSignal->sizeInWords();
				}
			}
			else
			{
				// discrete signal
				//
				if (appSignal->isRegistered())
				{
					m_registeredInternalDiscreteSignalsCount++;
				}
				else
				{
					m_internalDiscreteSignalsCount++;
				}
			}
		}

		// size of registered internal discrete signals in words
		//
		m_registeredInternalDiscreteSignalsSize = m_registeredInternalDiscreteSignalsCount / 16 +
				(m_registeredInternalDiscreteSignalsCount % 16 ? 1 : 0);

		// size of internal discrete signals in words
		//
		m_internalDiscreteSignalsSize = m_internalDiscreteSignalsCount / 16 +
				(m_internalDiscreteSignalsCount % 16 ? 1 : 0);

		m_internalDiscreteSignalsOffset = m_registeredInternalDiscreteSignalsOffset + m_registeredInternalDiscreteSignalsSize;

		m_regBufferInternalDiscreteSignalsOffset = m_registeredInternalAnalogSignalsOffset + m_registeredInternalAnalogSignalsSize;
		m_regBufferInternalDiscreteSignalsSize = m_registeredInternalDiscreteSignalsSize;

		m_internalAnalogSignalsOffset = m_registeredInternalAnalogSignalsOffset +
										m_registeredInternalAnalogSignalsSize +
										m_registeredInternalDiscreteSignalsSize;


		// calculate internal analog & discrete signals addresses
		//

		Address16 regInternalAnalog(m_registeredInternalAnalogSignalsOffset, 0);
		Address16 internalAnalog(m_internalAnalogSignalsOffset, 0);

		Address16 regInternalDiscrete(m_registeredInternalDiscreteSignalsOffset, 0);
		Address16 regBufferInternalDiscrete(m_regBufferInternalDiscreteSignalsOffset, 0);
		Address16 internalDiscrete(m_internalDiscreteSignalsOffset, 0);

		for(AppSignal* appSignal : m_appSignals)
		{
			if (appSignal == nullptr)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			if (!appSignal->isInternal())
			{
				continue;
			}

			Address16 ramAddr;
			Address16 regAddr;

			if (appSignal->isAnalog())
			{
				// analog signal
				//
				if (appSignal->isRegistered())
				{
					ramAddr = regInternalAnalog;
					regAddr = regInternalAnalog;

					regInternalAnalog.addWord(appSignal->sizeInWords());
				}
				else
				{
					ramAddr = internalAnalog;

					internalAnalog.addWord(appSignal->sizeInWords());
				}
			}
			else
			{
				// discrete signal
				//
				if (appSignal->isRegistered())
				{
					ramAddr = regInternalDiscrete;
					regAddr = regBufferInternalDiscrete;

					regInternalDiscrete.addBit();
					regBufferInternalDiscrete.addBit();
				}
				else
				{
					ramAddr = internalDiscrete;

					internalDiscrete.addBit();
				}
			}

			appSignal->ramAddr() = ramAddr;
			appSignal->regAddr() = regAddr;

			if (!appSignal->isShadowSignal())
			{
				// set same ramAddr & regAddr for corresponding signals in build-time SignalSet (ModuleLogicCompiler::m_signals member) !!!
				//
				int index = m_signals->keyIndex(appSignal->ID());

				if (index == -1)
				{
					assert(false);
				}
				else
				{
					(*m_signals)[index].regAddr() = regAddr;
					(*m_signals)[index].ramAddr() = ramAddr;
				}
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::getLMIntProperty(const QString &section, const QString& name, int *value)
	{
		return getDeviceIntProperty(m_lm, section, name, value);
	}


	bool ModuleLogicCompiler::getLMIntProperty(const QString& name, int *value)
	{
		return getDeviceIntProperty(m_lm, QString(""), name, value);
	}


	bool ModuleLogicCompiler::getDeviceIntProperty(Hardware::DeviceObject* device, const QString& name, int *value)
	{
		return getDeviceIntProperty(device, QString(""), name, value);
	}


	bool ModuleLogicCompiler::getDeviceIntProperty(Hardware::DeviceObject* device, const QString &section, const QString& name, int *value)
	{
		if (device == nullptr)
		{
			assert(false);
			return false;
		}

		if (value == nullptr)
		{
			assert(false);
			return false;
		}

		QString propertyName;

		if (!section.isEmpty())
		{
			propertyName = QString("%1\\%2").arg(section).arg(name);
		}
		else
		{
			propertyName = name;
		}

		QVariant val = device->property(C_STR(propertyName));

		if (val.isValid() == false)
		{
			m_log->writeError(QString(tr("Property %1 is not found in device %2")).arg(propertyName).arg(device->strId()), false, true);
			return false;
		}

		*value = val.toInt();

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

			Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(device);

			if (module == nullptr)
			{
				assert(false);
			}

			return module;
		}

		return nullptr;
	}


	QString ModuleLogicCompiler::getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType)
	{
		if (m_moduleFamilyTypeStr.contains(familyType))
		{
			return m_moduleFamilyTypeStr[familyType];
		}

		assert(false);

		return tr("UNKNOWN MODULE TYPE");
	}


	void ModuleLogicCompiler::cleanup()
	{
		for(AppItem* appItem : m_appItems)
		{
			delete appItem;
		}

		m_appItems.clear();
	}


	// ---------------------------------------------------------------------------------------
	//
	// Fbl class implementation
	//

	Afb::Afb(std::shared_ptr<LogicAfb> afb) :
		m_afb(afb)
	{
		if (m_afb == nullptr)
		{
			assert(false);
			return;
		}
	}

	Afb::~Afb()
	{
	}


/*	quint16 Fbl::addInstance()
	{
		assert(m_fblInstance != nullptr);
		assert(m_nonRamFblInstance != nullptr);

		if (m_afbElement->hasRam())
		{
			m_currentInstance++;

			if (m_currentInstance > MAX_FB_INSTANCE)
			{
				assert(false);			// reach max instance
			}
		}
		else
		{
			// Calculate non-RAM Fbl instance
			//
			if (m_fblInstance->isEmpty())
			{
				// load all CommandCodes in map
				//
				for(unsigned int i = 0; i < sizeof(AllCommandCodes)/sizeof(CommandCodes); i++)
				{
					m_fblInstance->insert(AllCommandCodes[i], 0);
				}
			}

			if (m_nonRamFblInstance->contains(m_afbElement->strID()))
			{
				m_currentInstance = m_nonRamFblInstance->value(m_afbElement->strID());
			}
			else
			{
				CommandCodes opcode = static_cast<CommandCodes>(m_afbElement->opcode());

				if (m_fblInstance->contains(opcode))
				{
					int opcodeInstance = (*m_commandCodesInstance)[opcode];

					opcodeInstance++;

					(*m_fblInstance)[opcode] = opcodeInstance;

					m_currentInstance = opcodeInstance;
				}
				else
				{
					assert(false);		// unknown opcode
				}
			}
		}

		return m_currentInstance;
	}*/


	// ---------------------------------------------------------------------------------------
	//
	// FblsMap class implementation
	//

	void AfbMap::insert(std::shared_ptr<LogicAfb> logicAfb)
	{
		if (logicAfb == nullptr)
		{
			assert(false);
			return;
		}

		if (contains(logicAfb->strID()))
		{
			assert(false);	// 	repeated guid
			return;
		}

		Afb* afb = new Afb(logicAfb);

		HashedVector<QString, Afb*>::insert(afb->strID(), afb);

		// initialize map Fbl opCode -> current instance
		//
		if (!m_fblInstance.contains(logicAfb->opcode()))
		{
			m_fblInstance.insert(logicAfb->opcode(), 0);
		}

		// add AfbElement in/out signals to m_fblsSignals map
		//

		const std::vector<LogicAfbSignal>& inputSignals = logicAfb->inputSignals();

		for(LogicAfbSignal signal : inputSignals)
		{
			StrIDIndex si;

			si.strID = logicAfb->strID();
			si.index = signal.operandIndex();

			if (m_afbSignals.contains(si))
			{
				assert(false);
				continue;
			}

			m_afbSignals.insert(si, signal);
		}

		const std::vector<LogicAfbSignal>& outputSignals = logicAfb->outputSignals();

		for(LogicAfbSignal signal : outputSignals)
		{
			StrIDIndex si;

			si.strID = logicAfb->strID();
			si.index = signal.operandIndex();

			if (m_afbSignals.contains(si))
			{
				assert(false);
				continue;
			}

			m_afbSignals.insert(si, signal);
		}

		// add AfbElement params to m_fblsParams map
		//

		std::vector<LogicAfbParam>& params = logicAfb->params();

		for(LogicAfbParam param : params)
		{
			StrIDIndex si;

			si.strID = logicAfb->strID();
			si.index = param.operandIndex();

			if (m_afbParams.contains(si))
			{
				assert(false);
				continue;
			}

			m_afbParams.insert(si, &param);
		}
	}


	int AfbMap::addInstance(AppItem* appItem)
	{
		if (appItem == nullptr)
		{
			assert(false);
			return 1;
		}

		if (!contains(appItem->afbStrID()))
		{
			assert(false);			// unknown FBL guid
			return 0;
		}

		Afb* fbl = (*this)[appItem->afbStrID()];

		if (fbl == nullptr)
		{
			assert(false);
			return 0;
		}

		int instance = 0;

		if (fbl->hasRam())
		{
			instance = fbl->incInstance();
		}
		else
		{
			// Calculate non-RAM Fbl instance
			//
			if (m_nonRamFblInstance.contains(fbl->strID()))
			{
				instance = m_nonRamFblInstance.value(fbl->strID());
			}
			else
			{
				int fblOpcode = fbl->opcode();

				if (m_fblInstance.contains(fblOpcode))
				{
					instance = m_fblInstance[fblOpcode];

					instance++;

					m_fblInstance[fblOpcode] = instance;

					m_nonRamFblInstance.insert(fbl->strID(), instance);
				}
				else
				{
					assert(false);		// unknown opcode
				}
			}
		}

		if (instance == 0)
		{
			assert(false);				// invalid instance number
		}

		if (instance > MAX_FB_INSTANCE)
		{
			assert(false);				// reached the max instance number
		}

		return instance;
	}


	const LogicAfbSignal AfbMap::getAfbSignal(const QString& afbStrID, int signalIndex)
	{
		StrIDIndex si;

		si.strID = afbStrID;
		si.index = signalIndex;

		if (m_afbSignals.contains(si))
		{
			return m_afbSignals.value(si);
		}

		assert(false);

		return LogicAfbSignal();
	}


	void AfbMap::clear()
	{
		for(Afb* fbl : *this)
		{
			delete fbl;
		}

		HashedVector<QString, Afb*>::clear();
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppItem class implementation
	//

	AppItem::AppItem(const Builder::AppLogicItem &appLogicItem) :
		m_appLogicItem(appLogicItem)
	{
	}


	AppItem::AppItem(const AppItem& appItem)
	{
		m_appLogicItem = appItem.m_appLogicItem;
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppFb class implementation
	//

	AppFb::AppFb(AppItem *appItem, int instance) :
		AppItem(*appItem),
		m_instance(instance)
	{
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFbsMap class implementation
	//

	void AppFbMap::insert(AppItem *appItem, int instance)
	{
		if (appItem == nullptr)
		{
			assert(false);
			return;
		}

		AppFb* appFb = new AppFb(appItem, instance);

		HashedVector<QUuid, AppFb*>::insert(appFb->guid(), appFb);
	}


	void AppFbMap::clear()
	{
		for(AppFb* appFb : *this)
		{
			delete appFb;
		}

		HashedVector<QUuid, AppFb*>::clear();
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppSignal class implementation
	//


	AppSignal::AppSignal(const Signal *signal, const AppItem *appItem) :
		m_appItem(appItem)
	{
		// construct AppSignal based on real signal
		//
		if (signal == nullptr)
		{
			assert(false);
		}

		*dynamic_cast<Signal*>(this) = *signal;

		m_isShadowSignal = false;

		// believe that all input signals have already been computed
		//
		if (isInput())
		{
			setComputed();
		}
	}


	AppSignal::AppSignal(const QUuid& guid, SignalType signalType, int dataSize, const AppItem *appItem) :
		m_appItem(appItem),
		m_guid(guid)
	{
		// construct shadow AppSignal based on OutputPin
		//
		setStrID(QString("#%1").arg(guid.toString()));
		setType(signalType);
		setDataSize(dataSize);
		setInOutType(SignalInOutType::Internal);

		m_isShadowSignal = true;
	}


	const AppItem& AppSignal::appItem() const
	{
		assert(m_appItem != nullptr);

		return *m_appItem;
	}



	// ---------------------------------------------------------------------------------------
	//
	// AppSignalsMap class implementation
	//

	AppSignalMap::AppSignalMap(ModuleLogicCompiler& compiler) :
		m_compiler(compiler)
	{
	}


	AppSignalMap::~AppSignalMap()
	{
		clear();
	}


	// insert signal from application logic scheme
	//
	void AppSignalMap::insert(const AppItem* appItem)
	{
		if (!appItem->isSignal())
		{
			assert(false);
			return;
		}

		QString strID = appItem->strID();

		if (strID[0] != '#')
		{
			strID = "#" + strID;
		}

		if (strID == "#SYSTEMID_RACKID_CHASSISID_MD03_CTRLIN_INH25B")
		{
			int a = 0;
			a++;
		}

		const Signal* s = m_compiler.getSignal(strID);

		if (s == nullptr)
		{
			// signal identifier is not found

			QString msg = QString(tr("Signal identifier is not found: %1")).arg(strID);

			m_compiler.log().writeError(msg, false, true);

			return;
		}

		AppSignal* appSignal = nullptr;

		if (m_signalStrIdMap.contains(strID))
		{
			appSignal = m_signalStrIdMap[strID];

			qDebug() << "Bind appSignal = " << strID;
		}
		else
		{
			appSignal = new AppSignal(s, appItem);

			m_signalStrIdMap.insert(strID, appSignal);

			qDebug() << "Create appSignal = " << strID;

			incCounters(appSignal);
		}

		assert(appSignal != nullptr);

		HashedVector<QUuid, AppSignal*>::insert(appItem->guid(), appSignal);
	}


	// insert "shadow" signal bound to FB output pin
	//
	void AppSignalMap::insert(const AppItem* appItem, const LogicPin& outputPin)
	{
		if (!appItem->isFb())
		{
			assert(false);
			return;
		}

		const LogicAfbSignal s = m_compiler.getAfbSignal(appItem->afb().strID(), outputPin.afbOperandIndex());

/*		if (s == nullptr)
		{
			// signal identifier is not found

			QString msg = QString(tr("Signal identifier is not found: %1")).arg(outputPin.guid().toString());

			m_compiler.log().writeError(msg, false, true);

			return;
		}*/

		SignalType signalType = SignalType::Discrete;

		Afbl::AfbSignalType st = s.type();

		switch(st)
		{
		case Afbl::AfbSignalType::Analog:
			signalType = SignalType::Analog;
			break;

		case Afbl::AfbSignalType::Discrete:
			signalType = SignalType::Discrete;
			break;

		default:
			assert(false);
		}

		QUuid outPinGuid = outputPin.guid();

		QString strID = QString("#%1").arg(outPinGuid.toString());

		AppSignal* appSignal = nullptr;

		if (m_signalStrIdMap.contains(strID))
		{
			assert(false);			// duplicate pin GUID

			appSignal = m_signalStrIdMap[strID];

			qDebug() << "Bind appSignal = " << strID;
		}
		else
		{
			appSignal = new AppSignal(outPinGuid, signalType, s.size(), appItem);

			m_signalStrIdMap.insert(strID, appSignal);

			qDebug() << "Create appSignal = " << strID;

			incCounters(appSignal);
		}

		assert(appSignal != nullptr);

		HashedVector<QUuid, AppSignal*>::insert(outPinGuid, appSignal);
	}


	void AppSignalMap::incCounters(const AppSignal* appSignal)
	{
		if (appSignal->isInternal())
		{
			if (appSignal->isAnalog())
			{
				// analog signal
				//
				if (appSignal->isRegistered())
				{
					m_registeredAnalogSignalCount++;
				}
				else
				{
					m_notRegisteredAnalogSignalCount++;
				}
			}
			else
			{
				// discrete signal
				//
				if (appSignal->isRegistered())
				{
					m_registeredDiscreteSignalCount++;
				}
				else
				{
					m_notRegisteredDiscreteSignalCount++;
				}
			}
		}
	}


	AppSignal* AppSignalMap::getByStrID(const QString& strID)
	{
		if (m_signalStrIdMap.contains(strID))
		{
			return m_signalStrIdMap[strID];
		}

		return nullptr;
	}


	void AppSignalMap::clear()
	{
		for(AppSignal* appSignal : m_signalStrIdMap)
		{
			delete appSignal;
		}

		m_signalStrIdMap.clear();

		HashedVector<QUuid, AppSignal*>::clear();

		m_registeredAnalogSignalCount = 0;
		m_registeredDiscreteSignalCount = 0;

		m_notRegisteredAnalogSignalCount = 0;
		m_notRegisteredDiscreteSignalCount = 0;
	}



	// ---------------------------------------------------------------------------------------
	//
	// ModuleLogicCompiler::Module class implementation
	//
	// ---------------------------------------------------------------------------------------


	bool ModuleLogicCompiler::Module::isInputModule()
	{
		if (device == nullptr)
		{
			assert(false);
			return false;
		}

		return device->isInputModule();
	}


	bool ModuleLogicCompiler::Module::isOutputModue()
	{
		if (device == nullptr)
		{
			assert(false);
			return false;
		}

		return device->isOutputModule();
	}

	Hardware::DeviceModule::FamilyType ModuleLogicCompiler::Module::familyType()
	{
		if (device == nullptr)
		{
			assert(false);
			return Hardware::DeviceModule::FamilyType::OTHER;
		}

		return device->moduleFamily();
	}
}



