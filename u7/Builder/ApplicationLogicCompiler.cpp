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

		cleanup();

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
		else
		{
			buildServiceMaps();

			createAppSignalsMap();
		}

		return true;
	}


	void ModuleLogicCompiler::buildServiceMaps()
	{
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
		}

		m_fbls.clear();

		for(AfbElement& afbl : m_afbl->items)
		{
			m_fbls.insert(&afbl);
		}

		m_appItems.clear();
		m_pinParent.clear();

		for(const ApplicationLogicScheme& appLogicScheme : m_moduleLogic->appSchemes())
		{
			for(const AppLogicItem& logicItem : appLogicScheme.items())
			{
				// build QHash<QUuid, AppItem*> m_appItems
				// item GUID -> item ptr
				//
				if (m_appItems.contains(logicItem.m_fblItem->guid()))
				{
					assert(false);	// guid already in map!
					continue;
				}

				AppItem* appItem = new AppItem(&logicItem);

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
		}
	}


	bool ModuleLogicCompiler::createAppSignalsMap()
	{
		if (m_moduleLogic == nullptr)
		{
			assert(false);
			return false;
		}

		m_appSignals.clear();

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

		// find fbl's outputs, which connected to signals
		// build map: output GUID -> signal GUID
		//

		for(AppItem* item : m_appItems)
		{
			if (!item->isFb())
			{
				continue;
			}

			if (!item->afbInitialized())
			{
				assert(false);
				continue;
			}

			// get Functional Block instance !!!
			//
			int instance = m_fbls.addInstance(item);

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
							}
						}
					}
				}

				if (connectedToFbl && !connectedToSignal)
				{
					// create shadow signal with Uuid of this output pin
					//
					m_appSignals.insert(item, output);
				}
			}
		}

		return true;
	}


	bool ModuleLogicCompiler::afbInitialization()
	{
		bool result = true;

		m_code.newLine();
		m_code.comment("Functional Blocks initialization code");

		for(Fbl* fbl : m_fbls)
		{
			for(AppFb* appFb : m_appFbs)
			{
				if (appFb->afbGuid() != fbl->guid())
				{
					continue;
				}

				result &= initializeAppFbConstParams(appFb);

				if (!appFb->afb().hasRam())
				{
					// FB without RAM initialize once for all instances
					//
					break;
				}

				// initialize for each instance of FB with RAM
				//
				result &= initializeAppFbVariableParams(appFb);
			}
		}

		return result;
	}




	bool ModuleLogicCompiler::initializeAppFbConstParams(AppFb* appFb)
	{
		bool result = true;

		if (appFb == nullptr)
		{
			assert(false);
			return false;
		}

		const AfbElement& afb = appFb->afb();

		const LogicFb& logicFb = appFb->logicFb();

		quint16 fbInstance = appFb->instance();
		quint16 fbOpcode = afb.opcode();

		qDebug() << logicFb.dynamicPropertyNames();

		m_code.newLine();
		m_code.comment(QString(tr("Initalization of %1 instance %2")).arg(afb.strID()).arg(fbInstance));
		m_code.newLine();

		// iniitalization of constant params
		//
		for(AfbElementParam afbParam : afb.constParams())
		{
			Command cmd;

			quint16 paramValue = 0;

			const AfbParamValue& apv = afbParam.value();

			switch(afbParam.type())
			{
			case AnalogIntegral:
				paramValue = apv.IntegralValue;
#pragma message("###################################### Need calculate value!!!!!!!!! ####################")
				break;

			case AnalogFloatingPoint:
				assert(false);				// not implemented
				break;

			case DiscreteValue:
				paramValue = apv.Discrete;
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


	bool ModuleLogicCompiler::initializeAppFbVariableParams(AppFb* appFb)
	{
		bool result = true;

		if (appFb == nullptr)
		{
			assert(false);
			return false;
		}

		const AfbElement& afb = appFb->afb();

		const LogicFb& logicFb = appFb->logicFb();

		quint16 fbInstance = appFb->instance();
		quint16 fbOpcode = afb.opcode();


		m_code.newLine();

		// iniitalization of variable params
		//
		for(AfbElementParam afbParam : afb.params())
		{
			Command cmd;

			quint16 paramValue = 0;

			QVariant value = logicFb.property(afbParam.caption().toUtf8().constData());

			switch(afbParam.type())
			{
			case AnalogIntegral:
				paramValue = value.toInt();
#pragma message("###################################### Need calculate value!!!!!!!!! ####################")
				break;

			case AnalogFloatingPoint:
				assert(false);				// not implemented
				break;

			case DiscreteValue:
				paramValue = value.toInt();
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

	int Fbl::m_refCount = 0;
	CommandCodesInstanceMap* Fbl::m_commandCodesInstance = nullptr;		// CommandCodes -> current instance
	NonRamFblInstanceMap* Fbl::m_nonRamFblInstance = nullptr;			// Non RAM Fbl StrID -> instance


	Fbl::Fbl(AfbElement* afbElement) :
		m_afbElement(afbElement)
	{
		if (m_refCount == 0)
		{
			// first instance of Fbl class create maps
			//
			m_commandCodesInstance = new CommandCodesInstanceMap;
			m_nonRamFblInstance = new NonRamFblInstanceMap;
		}

		m_refCount++;

		if (m_afbElement == nullptr)
		{
			assert(false);
			return;
		}
	}

	Fbl::~Fbl()
	{
		m_refCount--;

		if (m_refCount == 0)
		{
			// last instance of of Fbl class delete maps
			//of Fbl class create maps
			delete m_commandCodesInstance;
			delete m_nonRamFblInstance;
		}
	}


	quint16 Fbl::addInstance()
	{
		assert(m_commandCodesInstance != nullptr);
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
			if (m_commandCodesInstance->isEmpty())
			{
				// load all CommandCodes in map
				//
				for(int i = 0; i < sizeof(AllCommandCodes)/sizeof(CommandCodes); i++)
				{
					m_commandCodesInstance->insert(AllCommandCodes[i], 0);
				}
			}

			if (m_nonRamFblInstance->contains(m_afbElement->strID()))
			{
				m_currentInstance = m_nonRamFblInstance->value(m_afbElement->strID());
			}
			else
			{
				CommandCodes opcode = static_cast<CommandCodes>(m_afbElement->opcode());

				if (m_commandCodesInstance->contains(opcode))
				{
					int opcodeInstance = (*m_commandCodesInstance)[opcode];

					opcodeInstance++;

					(*m_commandCodesInstance)[opcode] = opcodeInstance;

					m_currentInstance = opcodeInstance;
				}
				else
				{
					assert(false);		// unknown opcode
				}
			}
		}

		return m_currentInstance;
	}


	// ---------------------------------------------------------------------------------------
	//
	// FblsMap class implementation
	//

	void FblsMap::insert(AfbElement* afbElement)
	{
		if (afbElement == nullptr)
		{
			assert(false);
			return;
		}

		if (contains(afbElement->guid()))
		{
			assert(false);	// 	repeated guid
			return;
		}

		Fbl* fbl = new Fbl(afbElement);

		HashedVector<QUuid, Fbl*>::insert(fbl->guid(), fbl);

		// add AfbElement in/out signals to m_fblsSignals map
		//

		std::vector<AfbElementSignal> inputSignals = afbElement->inputSignals();

		for(AfbElementSignal signal : inputSignals)
		{
			GuidIndex gi;

			gi.guid = afbElement->guid();
			gi.index = signal.operandIndex();

			if (m_fblsSignals.contains(gi))
			{
				assert(false);
				continue;
			}

			m_fblsSignals.insert(gi, &signal);
		}

		std::vector<AfbElementSignal> outputSignals = afbElement->outputSignals();

		for(AfbElementSignal signal : outputSignals)
		{
			GuidIndex gi;

			gi.guid = afbElement->guid();
			gi.index = signal.operandIndex();

			if (m_fblsSignals.contains(gi))
			{
				assert(false);
				continue;
			}

			m_fblsSignals.insert(gi, &signal);
		}

		// add AfbElement params to m_fblsParams map
		//

		std::vector<AfbElementParam> params = afbElement->params();

		for(AfbElementParam param : params)
		{
			GuidIndex gi;

			gi.guid = afbElement->guid();
			gi.index = param.operandIndex();

			if (m_fblsParams.contains(gi))
			{
				assert(false);
				continue;
			}

			m_fblsParams.insert(gi, &param);
		}

		std::vector<AfbElementParam> constParams = afbElement->constParams();

		for(AfbElementParam param : constParams)
		{
			GuidIndex gi;

			gi.guid = afbElement->guid();
			gi.index = param.operandIndex();

			if (m_fblsParams.contains(gi))
			{
				assert(false);
				continue;
			}

			m_fblsParams.insert(gi, &param);
		}
	}


	const AfbSignal *FblsMap::getFblSignal(const QUuid& afbElemetGuid, int signalIndex)
	{
		GuidIndex gi;

		gi.guid = afbElemetGuid;
		gi.index = signalIndex;

		if (m_fblsSignals.contains(gi))
		{
			return m_fblsSignals.value(gi);
		}

		return nullptr;
	}


	int FblsMap::addInstance(AppItem* appItem)
	{
		if (appItem == nullptr)
		{
			assert(false);
			return 1;
		}

		if (!contains(appItem->afbGuid()))
		{
			assert(false);			// unknown FBL guid
			return 0;
		}

		Fbl* fbl = (*this)[appItem->afbGuid()];

		return fbl->addInstance();
	}


	void FblsMap::clear()
	{
		for(Fbl* fbl : *this)
		{
			delete fbl;
		}

		HashedVector<QUuid, Fbl*>::clear();
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppItem class implementation
	//

	AppItem::AppItem(const AppLogicItem* appLogicItem)
	{
		if (appLogicItem == nullptr)
		{
			assert(false);
			return;
		}

		m_fblItem = appLogicItem->m_fblItem.get();
		m_scheme = appLogicItem->m_scheme.get();
		m_afbElement = appLogicItem->m_afbElement.get();
	}


	AppItem::AppItem(const AppItem& appItem)
	{
		m_fblItem = appItem.m_fblItem;
		m_scheme = appItem.m_scheme;
		m_afbElement = appItem.m_afbElement;
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppFb class implementation
	//

	AppFb::AppFb(AppItem *appItem, int instance) :
		AppItem(*appItem),
		m_instance(instance)
	{
		assert(m_fblItem != nullptr);

		if (m_fblItem == nullptr || !m_fblItem->isFblElement())
		{
			assert(false);
			return;
		}

		m_logicFb = m_fblItem->toFblElement();
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFbsMap class implementation
	//

	void AppFbsMap::insert(AppItem *appItem, int instance)
	{
		if (appItem == nullptr)
		{
			assert(false);
			return;
		}

		AppFb* appFb = new AppFb(appItem, instance);

		HashedVector<QUuid, AppFb*>::insert(appFb->guid(), appFb);
	}


	void AppFbsMap::clear()
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

	AppSignal::AppSignal(const QUuid& guid, const QString& strID, SignalType signalType, const Signal *signal, const AppItem *appItem) :
		m_guid(guid),
		m_strID(strID),
		m_signalType(signalType),
		m_signal(signal),
		m_appItem(appItem)
	{
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

	AppSignalsMap::AppSignalsMap(ModuleLogicCompiler& compiler) :
		m_compiler(compiler)
	{
	}


	AppSignalsMap::~AppSignalsMap()
	{
		clear();
	}


	// insert signal from application logic scheme
	//
	void AppSignalsMap::insert(const AppItem* appItem)
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

		const Signal* s = m_compiler.getSignal(strID);

		if (s == nullptr)
		{
			// signal identifier is not found

			QString msg = QString(tr("Signal identifier is not found: %1")).arg(strID);

			m_compiler.log().writeError(msg, false, true);

			return;
		}

		insert(appItem->guid(), strID, s->type(), s, appItem);
	}


	// insert "shadow" signal bound to FB output pin
	//
	void AppSignalsMap::insert(const AppItem* appItem, const LogicPin& outputPin)
	{
		if (!appItem->isFb())
		{
			assert(false);
			return;
		}

		const AfbSignal* s = m_compiler.getFblSignal(appItem->afb().guid(), outputPin.afbOperandIndex());

		if (s == nullptr)
		{
			// signal identifier is not found

			QString msg = QString(tr("Signal identifier is not found: %1")).arg(outputPin.guid().toString());

			m_compiler.log().writeError(msg, false, true);

			return;
		}

		SignalType signalType = SignalType::Discrete;

		Afbl::AfbSignalType st = s->type();

		int v = static_cast<int>(st);

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

		insert(outPinGuid, QString("#%1").arg(outPinGuid.toString()), signalType, nullptr, nullptr);
	}


	void AppSignalsMap::insert(const QUuid& guid, const QString& strID, SignalType signalType, const Signal* signal, const AppItem* appItem)
	{
		AppSignal* appSignal = nullptr;

		if (m_signalStrIdMap.contains(strID))
		{
			appSignal = m_signalStrIdMap[strID];

			qDebug() << "Bind appSignal = " << strID;
		}
		else
		{
			appSignal = new AppSignal(guid, strID, signalType, signal, appItem);

			m_signalStrIdMap.insert(strID, appSignal);

			qDebug() << "Create appSignal = " << strID;
		}
		assert(appSignal != nullptr);

		HashedVector<QUuid, AppSignal*>::insert(guid, appSignal);
	}


	void AppSignalsMap::clear()
	{
		for(AppSignal* appSignal : m_signalStrIdMap)
		{
			delete appSignal;
		}

		m_signalStrIdMap.clear();

		HashedVector<QUuid, AppSignal*>::clear();
	}


	void AppSignalsMap::bindRealSignals(SignalSet* signalSet)
	{
		// bind real signals to siganls in Signal Set
		//
		/*if (!m_signalStrIdMap.contains(signalStrID))
		{
			msg = QString(tr("Signal with ID = %1 not found in Application Signals")).arg(signalStrID);

			m_log->writeError(msg, false, true);
		}*/
	}
}



