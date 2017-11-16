#include "../Builder/ModuleLogicCompiler.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"
#include "../lib/Crc.h"
#include "Connection.h"
#include "../TuningIPEN/TuningIPENDataStorage.h"
#include "Parser.h"
#include "Builder.h"

namespace Builder
{

	// ---------------------------------------------------------------------------------
	//
	//	ModuleLogicCompiler class implementation
	//
	// ---------------------------------------------------------------------------------

	const char* ModuleLogicCompiler::INPUT_CONTROLLER_SUFFIX = "_CTRLIN";
	const char* ModuleLogicCompiler::OUTPUT_CONTROLLER_SUFFIX = "_CTRLOUT";
	const char* ModuleLogicCompiler::PLATFORM_INTERFACE_CONTROLLER_SUFFIX = "_PI";


	ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm) :
		m_appLogicCompiler(appLogicCompiler),
		m_memoryMap(appLogicCompiler.m_log),
		m_appSignals(*this)
	{
		m_equipmentSet = appLogicCompiler.m_equipmentSet;
		m_deviceRoot = m_equipmentSet->root();
		m_signals = appLogicCompiler.m_signals;

		m_lmDescription = appLogicCompiler.m_lmDescriptions->get(lm);

		m_appLogicData = appLogicCompiler.m_appLogicData;
		m_resultWriter = appLogicCompiler.m_resultWriter;
		m_log = appLogicCompiler.m_log;
		m_lm = lm;
		m_connections = appLogicCompiler.m_connections;
		m_optoModuleStorage = appLogicCompiler.m_optoModuleStorage;
		m_tuningDataStorage = appLogicCompiler.m_tuningDataStorage;
		m_cmpStorage = appLogicCompiler.m_cmpStorage;

		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::OTHER, "OTHER");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::LM, "LM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AIM, "AIM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AOM, "AOM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::DIM, "DIM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::DOM, "DOM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AIFM, "AIFM");
		m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::OCM, "OCM");
	}

	ModuleLogicCompiler::~ModuleLogicCompiler()
	{
		cleanup();
	}

	Signal* ModuleLogicCompiler::getSignal(const QString& appSignalID)
	{
		return m_chassisSignals.value(appSignalID, nullptr);
	}

	bool ModuleLogicCompiler::pass1()
	{
		LOG_EMPTY_LINE(m_log)

		msg = QString(tr("Compilation pass #1 for LM %1 was started...")).arg(m_lm->equipmentIdTemplate());

		LOG_MESSAGE(m_log, msg);

		m_chassis = m_lm->getParentChassis();

		if (m_chassis == nullptr)
		{
			msg = QString(tr("LM %1 must be installed in the chassis!")).arg(m_lm->equipmentIdTemplate());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return false;
		}

		std::shared_ptr<AppLogicModule> appLogicModule = m_appLogicData->module(m_lm->equipmentIdTemplate());

		m_moduleLogic = appLogicModule.get();

		if (m_moduleLogic == nullptr)
		{
			//	Application logic for module '%1' is not found.
			//
			m_log->wrnALC5001(m_lm->equipmentIdTemplate());
		}

		bool result = false;

		do
		{
			if (loadLMSettings() == false) break;

			if (loadModulesSettings() == false) break;

			if (createChassisSignalsMap() == false) break;

			if (createAppLogicItemsMaps() == false) break;

			if (createAppFbsMap() == false) break;

			if (createAppSignalsMap() == false) break;

			if (processTxSignals() == false) break;

			if (processSerialRxSignals() == false) break;

			if (buildTuningData() == false) break;

			if (createSignalLists() == false) break;

			if (disposeSignalsInMemory() == false) break;

			if (appendFbsForAnalogInOutSignalsConversion() == false) break;

			if (setOutputSignalsAsComputed() == false) break;

			if (setOptoRawInSignalsAsComputed() == false) break;

			result = true;
		}

		while(false);

		if (result == true)
		{
			msg = QString(tr("Compilation pass #1 for LM %1 was successfully finished.")).
					arg(m_lm->equipmentIdTemplate());

			LOG_SUCCESS(m_log, msg);
		}
		else
		{
			msg = QString(tr("Compilation pass #1 for LM %1 was finished with errors")).arg(m_lm->equipmentIdTemplate());
			LOG_MESSAGE(m_log, msg);
		}

		return result;
	}

	bool ModuleLogicCompiler::pass2()
	{
		LOG_EMPTY_LINE(m_log)

		msg = QString(tr("Compilation pass #2 for LM %1 was started...")).arg(m_lm->equipmentIdTemplate());

		LOG_MESSAGE(m_log, msg);

		bool result = false;

		do
		{
			if (finalizeOptoConnectionsProcessing() == false) break;

			// LM program code generation
			//
			if (generateAppStartCommand() == false) break;

			if (initAfbs() == false) break;

			if (startAppLogicCode() == false) break;

			// if (!initAfbs()) break;

			if (copyAcquiredRawDataInRegBuf() == false) break;

			if (convertAnalogInputSignals() == false) break;

			if (copySerialRxSignals() == false) break;

			if (generateAppLogicCode() == false) break;

			if (copyAcquiredTuningAnalogSignalsToRegBuf() == false) break;

			if (copyAcquiredTuningDiscreteSignalsToRegBuf() == false) break;

			if (copyAcquiredDiscreteInputSignalsToRegBuf() == false) break;

			if (copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf() == false) break;

			if (copyOutputSignalsInOutputModulesMemory() == false) break;

			if (copyOptoConnectionsTxData() == false) break;

			if (finishAppLogicCode() == false) break;

			//

			if (setLmAppLANDataSize() == false) break;

			if (calculateCodeRunTime() == false) break;

			if (writeResult() == false) break;

			result = true;
		}
		while(false);

		if (result == true)
		{
			displayResourcesUsageInfo();

			msg = QString(tr("Compilation pass #2 for LM %1 was successfully finished.")).
					arg(m_lm->equipmentIdTemplate());

			LOG_SUCCESS(m_log, msg);
		}
		else
		{
			msg = QString(tr("Compilation pass #2 for LM %1 was finished with errors")).arg(m_lm->equipmentIdTemplate());
			LOG_MESSAGE(m_log, msg);
		}

		cleanup();

		return result;
	}

	bool ModuleLogicCompiler::loadLMSettings()
	{
		bool result = true;

		MemoryArea moduleData;
		moduleData.setStartAddress(m_lmDescription->memory().m_moduleDataOffset);
		moduleData.setSizeW(m_lmDescription->memory().m_moduleDataSize);

		MemoryArea optoInterfaceData;
		optoInterfaceData.setStartAddress(m_lmDescription->optoInterface().m_optoInterfaceDataOffset);
		optoInterfaceData.setSizeW(m_lmDescription->optoInterface().m_optoPortDataSize);

		MemoryArea appLogicBitData;
		appLogicBitData.setStartAddress(m_lmDescription->memory().m_appLogicBitDataOffset);
		appLogicBitData.setSizeW(m_lmDescription->memory().m_appLogicBitDataSize);

		MemoryArea tuningData;
		tuningData.setStartAddress(m_lmDescription->memory().m_tuningDataOffset);
		tuningData.setSizeW(m_lmDescription->memory().m_tuningDataSize);

		MemoryArea appLogicWordData;
		appLogicWordData.setStartAddress(m_lmDescription->memory().m_appLogicWordDataOffset);
		appLogicWordData.setSizeW(m_lmDescription->memory().m_appLogicWordDataSize);

		m_memoryMap.init(moduleData,
						 optoInterfaceData,
						 appLogicBitData,
						 tuningData,
						 appLogicWordData);

		m_code.setMemoryMap(&m_memoryMap, m_log);

		m_lmClockFrequency = m_lmDescription->logicUnit().m_clockFrequency;
		m_lmALPPhaseTime = m_lmDescription->logicUnit().m_alpPhaseTime;
		m_lmIDRPhaseTime = m_lmDescription->logicUnit().m_idrPhaseTime;
		m_lmCycleDuration = m_lmDescription->logicUnit().m_cycleDuration;

		m_lmAppLogicFrameSize = m_lmDescription->flashMemory().m_appLogicFrameSize;
		m_lmAppLogicFrameCount = m_lmDescription->flashMemory().m_appLogicFrameCount;

		result &= getLMStrProperty("SubsystemID", &m_lmSubsystemID);
		result &= getLMIntProperty("LMNumber", &m_lmNumber);
		result &= getLMIntProperty("SubsystemChannel", &m_lmChannel);

		m_modules.clear();

		Module m;

		m.device = m_lm;
		m.place = LM1_PLACE;

		m.txDiagDataOffset = m_lmDescription->memory().m_txDiagDataOffset;
		m.txDiagDataSize = m_lmDescription->memory().m_txDiagDataSize;

		m.txAppDataOffset = m_lmDescription->memory().m_appDataOffset;
		m.txAppDataSize = m_lmDescription->memory().m_appDataSize;

		m.moduleDataOffset = 0;

		m.rxAppDataOffset = m.txAppDataOffset;
		m.rxAppDataSize = m.txAppDataSize;

		m_modules.insert(m_lm->equipmentIdTemplate(), m);

		if (result == true)
		{
			LOG_MESSAGE(m_log, QString(tr("Loading LMs settings... Ok")));
		}

		// chek LM subsystem ID
		//
		m_lmSubsystemKey = m_appLogicCompiler.m_subsystems->ssKey(m_lmSubsystemID);

		if (m_lmSubsystemKey == -1)
		{
			// SubsystemID '%1' assigned in LM '%2' is not found in subsystem list.
			//
			m_log->errALC5056(m_lmSubsystemID, m_lm->equipmentIdTemplate());
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::loadModulesSettings()
	{
		bool result = true;

		// build Module structures array
		//
		for(int place = FIRST_MODULE_PLACE; place <= LAST_MODULE_PLACE; place++)
		{
			Module m;

			const Hardware::DeviceModule* device = DeviceHelper::getModuleOnPlace(m_lm, place);

			if (device == nullptr)
			{
				continue;
			}

			m.device = device;
			m.place = place;

			const DeviceHelper::IntPropertyNameVar moduleSettings[] =
			{
				{	"TxDataSize", &m.txDataSize },
				{	"TxDiagDataOffset", &m.txDiagDataOffset },
				{	"TxDiagDataSize", &m.txDiagDataSize },
				{	"TxAppDataOffset", &m.txAppDataOffset },
				{	"TxAppDataSize", &m.txAppDataSize },

				{	"RxDataSize", &m.rxDataSize },
				{	"RxAppDataOffset", &m.rxAppDataOffset },
				{	"RxAppDataSize", &m.rxAppDataSize },
			};

			for(DeviceHelper::IntPropertyNameVar moduleSetting : moduleSettings)
			{
				result &= DeviceHelper::getIntProperty(device, moduleSetting.name, moduleSetting.var, m_log);
			}

			m.moduleDataOffset = m_memoryMap.getModuleDataOffset(place);

			m_modules.insert(device->equipmentIdTemplate(), m);
		}

		if (result == true)
		{
			LOG_MESSAGE(m_log, QString(tr("Loading modules settings... Ok")));
		}

		return result;
	}

	bool ModuleLogicCompiler::createChassisSignalsMap()
	{
		bool result = true;

		int signalCount = m_signals->count();

		m_chassisSignals.clear();
		m_ioSignals.clear();
		m_linkedValidtySignalsID.clear();

		for(int i = 0; i < signalCount; i++)
		{
			Signal& s = (*m_signals)[i];

			if (s.equipmentID().isEmpty() == true)
			{
				continue;
			}

			Hardware::DeviceSignal* deviceSignal = nullptr;

			bool isIoSignal = false;

			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(s.equipmentID());

			if (device == nullptr)
			{
				continue;
			}

			switch(device->deviceType())
			{
			case Hardware::DeviceType::Module:
				{
					Hardware::DeviceModule* deviceModule = device->toModule();

					if (deviceModule == nullptr)
					{
						assert(false);
						continue;
					}

					if (deviceModule->isLogicModule() == false)
					{
						assert(false); // signal must be associated with Logic Module only
						continue;
					}

					if (deviceModule->equipmentIdTemplate() != m_lm->equipmentIdTemplate())
					{
						continue;
					}

					// signal is associated with current LM

					isIoSignal = false;
				}
				break;

			case Hardware::DeviceType::Signal:
				{
					deviceSignal = device->toSignal();

					if (deviceSignal == nullptr)
					{
						assert(false);
						continue;
					}

					const Hardware::DeviceChassis* chassis = deviceSignal->getParentChassis();

					if (chassis == nullptr)
					{
						assert(false);
						continue;
					}

					if (chassis != m_chassis)
					{
						continue;
					}

					// signal is associated with current LM

					isIoSignal = true;

					QString validitySignalID = deviceSignal->validitySignalId();

					if (validitySignalID.isEmpty() == false)
					{
						if (m_linkedValidtySignalsID.contains(deviceSignal->equipmentIdTemplate()) == true)
						{
							assert(false);
						}
						else
						{
							// DeviceSignalEquipmentID => LinkedValiditySignalEquipmentID
							//
							m_linkedValidtySignalsID.insert(deviceSignal->equipmentIdTemplate(), validitySignalID);
						}
					}
				}

				break;

			default:
				assert(false); // signal must be associated with DeviceSignal or DeviceModule (LM) only
				continue;
			}

			if (m_chassisSignals.contains(s.appSignalID()) == true)
			{
				assert(false);				// duplicate signal!
				continue;
			}

			if (s.isAnalog() == true && s.dataSize() != SIZE_32BIT)
			{
				assert(false);
			}

			m_chassisSignals.insert(s.appSignalID(), &s);

			if (isIoSignal == true)
			{
				m_ioSignals.insert(s.appSignalID(), &s);

				if (deviceSignal != nullptr)
				{
					m_equipmentSignals.insert(deviceSignal->equipmentIdTemplate(), &s);
				}
				else
				{
					assert(false);
				}
			}

			continue;
		}

		return result;
	}

	bool ModuleLogicCompiler::createAppLogicItemsMaps()
	{
		m_afbs.clear();

		for(std::shared_ptr<Afb::AfbElement> afbl : m_lmDescription->afbs())
		{
			m_afbs.insert(afbl);
		}

		m_appItems.clear();
		m_pinParent.clear();

		if (m_moduleLogic == nullptr)
		{
			return true;
		}

		bool result = true;

		for(const AppLogicItem& logicItem : m_moduleLogic->items())
		{
			// build QHash<QUuid, AppItem*> m_appItems
			// item GUID -> item ptr
			//
			if (m_appItems.contains(logicItem.m_fblItem->guid()) == true)
			{
				AppItem* firstItem = m_appItems[logicItem.m_fblItem->guid()];

				msg = QString(tr("Duplicate GUID %1 of %2 and %3 elements")).
						arg(logicItem.m_fblItem->guid().toString()).arg(firstItem->strID()).arg(getAppLogicItemStrID(logicItem));

				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

				result = false;

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
					AppItem* firstItem = m_pinParent[input.guid()];

					msg = QString(tr("Duplicate input pin GUID %1 of %2 and %3 elements")).
							arg(input.guid().toString()).arg(firstItem->strID()).arg(appItem->strID());

					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

					result = false;

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
					AppItem* firstItem = m_pinParent[output.guid()];

					msg = QString(tr("Duplicate output pin GUID %1 of %2 and %3 elements")).
							arg(output.guid().toString()).arg(firstItem->strID()).arg(appItem->strID());

					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

					result = false;

					continue;
				}

				m_pinParent.insert(output.guid(), appItem);
			}
		}

		return result;
	}

	QString ModuleLogicCompiler::getAppLogicItemStrID(const AppLogicItem& appLogicItem) const
	{
		AppItem appItem(appLogicItem); return appItem.strID();
	}

	bool ModuleLogicCompiler::createAppFbsMap()
	{
		for(AppItem* item : m_appItems)
		{
			if (item->isFb() == false)
			{
				continue;
			}

			AppFb* appFb = createAppFb(*item);

			if (appFb == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAppSignalsMap()
	{
		m_appSignals.clear();
		m_outPinSignal.clear();

		// find signals in algorithms
		// build map: signal GUID -> ApplicationSignal
		//
		bool result = true;

		for(AppItem* item : m_appItems)
		{
			if (item->isSignal() == false)
			{
				continue;
			}

			if (m_chassisSignals.contains(item->strID()) == false)
			{
				// The signal '%1' is not associated with LM '%2'.
				//
				m_log->errALC5030(item->strID(), m_lm->equipmentIdTemplate(), item->guid());
				result = false;
				continue;
			}

			result &= m_appSignals.insert(item);
		}

		if (result == false)
		{
			return false;
		}

		// find fbl's outputs, which NOT connected to signals
		// create and add to m_appSignals map 'shadow' signals
		//

		for(AppItem* item : m_appItems)
		{
			if (item->isFb() == false)
			{
				continue;
			}

			for(LogicPin output : item->outputs())
			{
				bool connectedToFb = false;
				bool connectedToSignal = false;

				/*if (item->label() == "3TQ00SYN18_1_38595")
				{
					int a = 0;
					a++;
				}*/

				for(QUuid connectedPinUuid : output.associatedIOs())
				{
					if (!m_pinParent.contains(connectedPinUuid))
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						AppItem* connectedAppItem = m_pinParent[connectedPinUuid];

						if (connectedAppItem->isFb())
						{
							connectedToFb = true;
						}
						else
						{
							if (connectedAppItem->isSignal())
							{
								connectedToSignal = true;

								m_outPinSignal.insertMulti(output.guid(), connectedAppItem->signal().guid());
							}
						}
					}
				}

				if (connectedToFb == true && connectedToSignal == false)
				{
					// create shadow signal with Uuid of this output pin
					//
					if (m_appFbs.contains(item->guid()))
					{
						const AppFb* appFb = m_appFbs[item->guid()];

						if (appFb != nullptr)
						{
							result &= m_appSignals.insert(appFb, output, m_log);
						}
						else
						{
							ASSERT_RESULT_FALSE_BREAK
						}
					}
					else
					{
						ASSERT_RESULT_FALSE_BREAK			// unknown item->guid(
					}

					// output pin connected to shadow signal with same guid
					//
					m_outPinSignal.insert(output.guid(), output.guid());
				}
			}
		}

		for(AppItem* item : m_appItems)
		{
			if (item->isReceiver() == false)
			{
				continue;
			}

			for(LogicPin output : item->outputs())
			{
				for(QUuid connectedPinUuid : output.associatedIOs())
				{
					if (!m_pinParent.contains(connectedPinUuid))
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						AppItem* connectedAppItem = m_pinParent[connectedPinUuid];

						if (connectedAppItem->isSignal())
						{
							m_outPinSignal.insertMulti(output.guid(), connectedAppItem->signal().guid());
						}
					}
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::buildTuningData()
	{
		assert(m_tuningData == nullptr);
		assert(m_lmDescription);

		int tuningMemoryStartAddrW = m_lmDescription->memory().m_tuningDataOffset;
		int tuningFrameSizeBytes = m_lmDescription->flashMemory().m_tuningFrameSize;
		int tuningFrameCount = m_lmDescription->flashMemory().m_tuningFrameCount;

		// To generate tuning data for IPEN (version 1 of FOTIP protocol)
		// uncomment next 3 lines:
		//
		// TuningIPEN::TuningData* tuningData = new TuningIPEN::TuningData(m_lm->equipmentIdTemplate(),
		//													tuningFrameSizeBytes,
		//													tuningFrameCount);
		//
		// and comment 3 lines below:
		//
		Tuning::TuningData* tuningData = new Tuning::TuningData(m_lm->equipmentIdTemplate(),
												tuningMemoryStartAddrW,
												tuningFrameSizeBytes,
												tuningFrameCount);

		// common code for IPEN (FotipV1) and FotipV2 tuning protocols and data
		//
		bool result = true;

		result &= tuningData->buildTuningSignalsLists(m_chassisSignals, m_log);
		result &= tuningData->buildTuningData();

		if (result == true)
		{
			if (tuningData->usedFramesCount() > tuningFrameCount)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Tuning data of LM '%1' exceed available %2 frames")).
								   arg(m_lm->equipmentIdTemplate()).
								   arg(tuningFrameCount));
				result = false;
			}
			else
			{
				tuningData->generateUniqueID(m_lm->equipmentIdTemplate());

				m_tuningData = tuningData;
				m_tuningDataStorage->insert(m_lm->equipmentIdTemplate(), tuningData);
			}
		}

		if (result == false)
		{
			delete tuningData;
		}

		return result;
	}

	bool ModuleLogicCompiler::createSignalLists()
	{
		bool result = true;

		result &= createAcquiredDiscreteInputSignalsList();
		result &= createAcquiredDiscreteOutputSignalsList();
		result &= createAcquiredDiscreteInternalSignalsList();
		result &= createAcquiredDiscreteTuningSignalsList();

		result &= createNonAcquiredDiscreteInputSignalsList();
		result &= createNonAcquiredDiscreteOutputSignalsList();
		result &= createNonAcquiredDiscreteInternalSignalsList();
		result &= createNonAcquiredDiscreteTuningSignalsList();

		result &= createAcquiredAnalogInputSignalsList();
		result &= createAcquiredAnalogOutputSignalsList();
		result &= createAcquiredAnalogInternalSignalsList();
		result &= createAcquiredAnalogTuninglSignalsList();

		result &= createNonAcquiredAnalogInputSignalsList();
		result &= createNonAcquiredAnalogOutputSignalsList();
		result &= createNonAcquiredAnalogInternalSignalsList();
		result &= createNonAcquiredAnalogTuningSignalsList();

		result &= createAcquiredBusSignalsList();
		result &= createNonAcquiredBusSignalsList();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		result = listsUniquenessCheck();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		sortSignalList(m_acquiredDiscreteInputSignals);
		sortSignalList(m_acquiredDiscreteOutputSignals);
		sortSignalList(m_acquiredDiscreteInternalSignals);
		// sortSignalList(m_acquiredDiscreteTuningSignals);			// Not need to sort!

		sortSignalList(m_nonAcquiredDiscreteInputSignals);
		sortSignalList(m_nonAcquiredDiscreteOutputSignals);
		sortSignalList(m_nonAcquiredDiscreteInternalSignals);
		sortSignalList(m_nonAcquiredDiscreteInternalSignals);

		sortSignalList(m_acquiredAnalogInputSignals);
		sortSignalList(m_acquiredAnalogOutputSignals);
		sortSignalList(m_acquiredAnalogInternalSignals);
		// sortSignalList(m_acquiredAnalogTuningSignals);			// Not need to sort!

		sortSignalList(m_nonAcquiredAnalogInputSignals);
		sortSignalList(m_nonAcquiredAnalogOutputSignals);
		sortSignalList(m_nonAcquiredAnalogInternalSignals);
		sortSignalList(m_nonAcquiredAnalogTuningSignals);

		sortSignalList(m_acquiredBuses);
		sortSignalList(m_nonAcquiredBuses);

		return result;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteInputSignalsList()
	{
		m_acquiredDiscreteInputSignals.clear();
		m_acquiredDiscreteInputSignalsMap.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ discrete
		//	+ input
		//	+ no matter used in UAL or not

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isInput() == true)
			{
				if (m_acquiredDiscreteInputSignalsMap.contains(s) == false)
				{
					m_acquiredDiscreteInputSignals.append(s);
					m_acquiredDiscreteInputSignalsMap.insert(s, s);
				}

				// if input signal is acquired, then validity signal (if exists) also always acquired
				//
				appendLinkedValiditySignal(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteOutputSignalsList()
	{
		m_acquiredDiscreteOutputSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ discrete
		//	+ output
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isOutput() == true &&
				isUsedInUal(s) == true)
			{
				m_acquiredDiscreteOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteInternalSignalsList()
	{
		m_acquiredDiscreteInternalSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ discrete
		//	+ internal
		//  - enableTuning
		//	+ used in UAL || is a SerialRx signal

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isInternal() == true &&
				s->enableTuning() == false &&
				(isUsedInUal(s->appSignalID()) == true ||
				m_optoModuleStorage->isSerialRxSignalExists(m_lm->equipmentIdTemplate(), s->appSignalID())))
			{
				m_acquiredDiscreteInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteTuningSignalsList()
	{
		m_acquiredDiscreteTuningSignals.clear();

		if (m_tuningData == nullptr)
		{
			return true;
		}

		//	list include signals that:
		//
		//	+ acquired
		//	+ discrete
		//	+ internal
		//	+ tuningable
		//	+ no matter used in UAL or not

		m_tuningData->getAcquiredDiscreteSignals(m_acquiredDiscreteTuningSignals);

		// check signals!

		for(Signal* s : m_acquiredDiscreteTuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isInternal() == true &&
				s->enableTuning() == true)
			{
				continue;
			}
			else
			{
				assert(false);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredDiscreteInputSignalsList()
	{
		m_nonAcquiredDiscreteInputSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ discrete
		//	+ input
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isInput() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredDiscreteInputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredDiscreteOutputSignalsList()
	{
		m_nonAcquiredDiscreteOutputSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ discrete
		//	+ output
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isOutput() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredDiscreteOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredDiscreteInternalSignalsList()
	{
		m_nonAcquiredDiscreteInternalSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ discrete
		//	+ internal
		//  - enableTuning
		//	+ used in UAL
		//	+ shadow discrete internal signals (auto generated in m_appSignals)

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isInternal() == true &&
				s->enableTuning() == false &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredDiscreteInternalSignals.append(s);
			}
		}

		// append shadow discrete internal signals (auto generated in m_appSignals)
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			TEST_PTR_CONTINUE(appSignal);

			if (appSignal->isShadowSignal() == true &&
				appSignal->isDiscrete() == true)
			{
				Signal* s = appSignal->signal();

				if (s == nullptr)
				{
					assert(false);
					continue;
				}

				assert(s->isAcquired() == false);
				assert(s->isInternal() == true);

				m_nonAcquiredDiscreteInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredDiscreteTuningSignalsList()
	{
		m_nonAcquiredDiscreteTuningSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ discrete
		//	+ internal
		//	+ tuningable
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isInternal() == true &&
				s->enableTuning() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredDiscreteTuningSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogInputSignalsList()
	{
		m_acquiredAnalogInputSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ analog
		//	+ input
		//	+ no matter used in UAL or not

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isInput() == true)
			{
				m_acquiredAnalogInputSignals.append(s);

				// if input signal is acquired, then validity signal (if exists) also always acquired
				//
				appendLinkedValiditySignal(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogOutputSignalsList()
	{
		m_acquiredAnalogOutputSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ analog
		//	+ output
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isOutput() == true &&
				isUsedInUal(s) == true)
			{
				m_acquiredAnalogOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogInternalSignalsList()
	{
		m_acquiredAnalogInternalSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ analog
		//	+ internal
		//  - enableTuning
		//	+ used in UAL || is a SerialRx signal

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isInternal() == true &&
				s->enableTuning() == false &&
				(isUsedInUal(s->appSignalID()) == true ||
				 m_optoModuleStorage->isSerialRxSignalExists(m_lm->equipmentIdTemplate(), s->appSignalID()) == true))
			{
				m_acquiredAnalogInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogTuninglSignalsList()
	{
		m_acquiredAnalogTuningSignals.clear();

		if (m_tuningData == nullptr)
		{
			return true;
		}

		//	list include signals that:
		//
		//	+ acquired
		//	+ analog
		//	+ internal
		//	+ tuningable
		//	+ no matter used in UAL or not

		m_tuningData->getAcquiredAnalogSignals(m_acquiredAnalogTuningSignals);

		// check signals!

		for(Signal* s : m_acquiredAnalogTuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isInternal() == true &&
				s->enableTuning() == true)
			{
				continue;
			}
			else
			{
				assert(false);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredAnalogInputSignalsList()
	{
		m_nonAcquiredAnalogInputSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ analog
		//	+ input
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isInput() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredAnalogInputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredAnalogOutputSignalsList()
	{
		m_nonAcquiredAnalogOutputSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ analog
		//	+ output
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isOutput() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredAnalogOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredAnalogInternalSignalsList()
	{
		m_nonAcquiredAnalogInternalSignals.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ analog
		//	+ internal
		//  - enableTuning
		//	+ used in UAL
		//	+ shadow analog internal signals (auto generated in m_appSignals)

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isInternal() == true &&
				s->enableTuning() == false &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredAnalogInternalSignals.append(s);
			}
		}

		// append shadow analog internal signals (auto generated in m_appSignals)
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			TEST_PTR_CONTINUE(appSignal);

			if (appSignal->isShadowSignal() == true &&
				appSignal->isAnalog() == true)
			{
				Signal* s = appSignal->signal();

				if (s == nullptr)
				{
					assert(false);
					continue;
				}

				assert(s->isAcquired() == false);
				assert(s->isInternal() == true);
				assert(s->dataSize() == SIZE_32BIT);

				m_nonAcquiredAnalogInternalSignals.append(s);
			}
		}


		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredAnalogTuningSignalsList()
	{
		m_nonAcquiredAnalogTuningSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ analog
		//	+ internal
		//	+ tuningable
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isInternal() == true &&
				s->enableTuning() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredAnalogTuningSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredBusSignalsList()
	{
		m_acquiredBuses.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ bus
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isBus() == true &&
				isUsedInUal(s) == true)
			{
				m_acquiredBuses.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredBusSignalsList()
	{
		m_nonAcquiredBuses.clear();

		//	list include signals that:
		//
		//	+ non acquired
		//	+ bus
		//	+ used in UAL

		for(Signal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false &&
				s->isBus() == true &&
				isUsedInUal(s) == true)
			{
				m_nonAcquiredBuses.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::appendLinkedValiditySignal(const Signal* s)
	{
		TEST_PTR_RETURN_FALSE(s);

		// if signal has linked validity signal, append validity signal to m_acquiredDiscreteInputSignals also
		//
		QString linkedValiditySignalEquipmentID  = m_linkedValidtySignalsID.value(s->equipmentID(), QString());

		if (linkedValiditySignalEquipmentID.isEmpty() == false)
		{
			Signal* linkedValiditySignal = m_equipmentSignals.value(linkedValiditySignalEquipmentID, nullptr);

			if (linkedValiditySignal == nullptr)
			{
				LOG_WARNING_OBSOLETE(m_log, Builder:::IssueType::NotDefined,
						  QString(tr("Linked validity signal with equipmentID '%1' is not found (input signal '%2')")).
									 arg(linkedValiditySignalEquipmentID).
									 arg(s->appSignalID()));
				return true;
			}

			if (linkedValiditySignal->isInput() == false ||
				linkedValiditySignal->isDiscrete() == false)
			{
				assert(false);							// validity signal must be discrete input signal
														// no matter is "acquired" or not
				return false;
			}

			if (m_acquiredDiscreteInputSignalsMap.contains(linkedValiditySignal) == false)
			{
				m_acquiredDiscreteInputSignals.append(linkedValiditySignal);
				m_acquiredDiscreteInputSignalsMap.insert(linkedValiditySignal, linkedValiditySignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::listsUniquenessCheck() const
	{
		bool result = true;

		QHash<Signal*, Signal*> signalsMap;

		signalsMap.reserve(m_chassisSignals.count());

		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteInputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteTuningSignals);

		result &= listUniquenessCheck(signalsMap, m_nonAcquiredDiscreteInputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredDiscreteOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredDiscreteInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredDiscreteTuningSignals);

		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogInputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogTuningSignals);

		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogInputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogTuningSignals);

		result &= listUniquenessCheck(signalsMap, m_acquiredBuses);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredBuses);

		return result;
	}

	bool ModuleLogicCompiler::listUniquenessCheck(QHash<Signal*, Signal*>& signalsMap, const QVector<Signal*>& signalList) const
	{
		bool result = true;

		for(Signal* s : signalList)
		{
			if (signalsMap.contains(s) == true)
			{
				assert(false);				// signal is duplicate in different signals lists!
				result = false;
				continue;
			}

			signalsMap.insert(s, s);
		}

		return result;
	}

	void ModuleLogicCompiler::sortSignalList(QVector<Signal*>& signalList)
	{
		int count = signalList.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				Signal* s1 = signalList[i];
				Signal* s2 = signalList[k];

				if (s1->appSignalID() > s2->appSignalID())
				{
					signalList[i] = s2;
					signalList[k] = s1;
				}
			}
		}
	}

	bool ModuleLogicCompiler::disposeSignalsInMemory()
	{
		bool result = false;

		do
		{
			if (calculateIoSignalsAddresses() == false) break;

			if (disposeDiscreteSignalsInBitMemory() == false) break;

			if (disposeAcquiredRawDataInRegBuf() == false) break;

			if (disposeAcquiredAnalogSignalsInRegBuf() == false) break;

			if (disposeAcquiredBusesInRegBuf() == false) break;

			if (disposeAcquiredDiscreteSignalsInRegBuf() == false) break;

			if (disposeNonAcquiredAnalogSignals() == false) break;

			if (disposeNonAcquiredBuses() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool ModuleLogicCompiler::calculateIoSignalsAddresses()
	{
		// calculation m_ioBufAddr of in/out signals
		//
		LOG_MESSAGE(m_log, QString(tr("Input & Output signals addresses calculation...")));

		bool result = true;

		for(Signal* s : m_ioSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			// retrieve linked device
			//
			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(s->equipmentID());

			if (device == nullptr)
			{
				assert(false);
				continue;
			}

			if (device->isSignal() == false)
			{
				assert(false);
				continue;
			}

			Hardware::DeviceSignal* deviceSignal = device->toSignal();

			if (deviceSignal == nullptr)
			{
				assert(false);
				continue;
			}

			// retrieve associated module
			//
			const Hardware::DeviceModule* deviceModule = deviceSignal->getParentModule();

			if (deviceModule == nullptr)
			{
				//assert(false);
				continue;
			}

			if (m_modules.contains(deviceModule->equipmentIdTemplate()) == false)
			{
				assert(false);
				continue;
			}

			Module module = m_modules.value(deviceModule->equipmentIdTemplate());

			Address16 ioBufAddr(module.moduleDataOffset, deviceSignal->valueBit());

			ioBufAddr.addWord(deviceSignal->valueOffset());

			switch(deviceSignal->memoryArea())
			{
			case E::MemoryArea::ApplicationData:

				switch(s->inOutType())
				{
				case E::SignalInOutType::Input:
					ioBufAddr.addWord(module.txAppDataOffset);
					s->setIoBufAddr(ioBufAddr);
					break;

				case E::SignalInOutType::Output:
					ioBufAddr.addWord(module.rxAppDataOffset);
					s->setIoBufAddr(ioBufAddr);
					break;

				case E::SignalInOutType::Internal:
					assert(false);							// internal signals can't be i/o Signals
					break;

				default:
					assert(false);
				}
				break;

			case E::MemoryArea::DiagnosticsData:

				switch(s->inOutType())
				{
				case E::SignalInOutType::Input:
					ioBufAddr.addWord(module.txDiagDataOffset);
					s->setIoBufAddr(ioBufAddr);
					break;

				case E::SignalInOutType::Output:
					assert(false);							// output diagnostics signals do not exist
					break;

				case E::SignalInOutType::Internal:
					assert(false);							// internal signals can't be i/o Signals
					break;

				default:
					assert(false);
				}
				break;

			default:
				assert(false);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::disposeDiscreteSignalsInBitMemory()
	{
		bool result = true;

		for(Signal* s : m_acquiredDiscreteOutputSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendAcquiredDiscreteOutputSignal(*s);

			s->setUalAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredDiscreteInternalSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendAcquiredDiscreteInternalSignal(*s);

			s->setUalAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_nonAcquiredDiscreteOutputSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendNonAcquiredDiscreteOutputSignal(*s);

			s->setUalAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_nonAcquiredDiscreteInternalSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendNonAcquiredDiscreteInternalSignal(*s);

			s->setUalAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		return result;
	}

	bool ModuleLogicCompiler::disposeAcquiredRawDataInRegBuf()
	{
		if (m_lm->rawDataDescription().isEmpty() == false)
		{
			assert(false);			// set actual raw data size here !!!
		}
		else
		{
			m_memoryMap.setAcquiredRawDataSize(0);
		}

		m_memoryMap.recalculateAddresses();

		return true;
	}

	bool ModuleLogicCompiler::disposeAcquiredAnalogSignalsInRegBuf()
	{
		bool result = true;

		int regBufOffset = -m_memoryMap.regBufStartAddr();	// minus is OK!

		for(Signal* s : m_acquiredAnalogInputSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendAcquiredAnalogInputSignal(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredAnalogOutputSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendAcquiredAnalogOutputSignal(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredAnalogInternalSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendAcquiredAnalogInternalSignal(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredAnalogTuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->tuningAddr().isValid() == true);

			// ualAddr of acquired analog tuning signals is assigned in TuningData::buildTuningData() !
			//
			// s->setUalAddr(s->tuningAddr());

			Address16 addr = m_memoryMap.appendAcquiredAnalogTuningSignal(*s);

			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		return result;
	}

	bool ModuleLogicCompiler::disposeAcquiredBusesInRegBuf()
	{
		bool result = true;

		int regBufOffset = -m_memoryMap.regBufStartAddr();	// minus is OK!

		for(Signal* s : m_acquiredBuses)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendAcquiredBus(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		return result;
	}

	bool ModuleLogicCompiler::disposeAcquiredDiscreteSignalsInRegBuf()
	{
		bool result = true;

		int regBufOffset = -m_memoryMap.regBufStartAddr();	// minus is OK!

		for(Signal* s : m_acquiredDiscreteInputSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->ioBufAddr().isValid() == true);

			s->setUalAddr(s->ioBufAddr());

			Address16 addr = m_memoryMap.appendAcquiredDiscreteInputSignalInRegBuf(*s);

			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredDiscreteOutputSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->ualAddr().isValid() == true);

			Address16 addr = m_memoryMap.appendAcquiredDiscreteOutputSignalInRegBuf(*s);

			s->setRegBufAddr(addr);

			assert(s->ualAddr().bit() == s->regBufAddr().bit());

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredDiscreteInternalSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->ualAddr().isValid() == true);

			Address16 addr = m_memoryMap.appendAcquiredDiscreteInternalSignalInRegBuf(*s);

			s->setRegBufAddr(addr);

			assert(s->ualAddr().bit() == s->regBufAddr().bit());

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_acquiredDiscreteTuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->tuningAddr().isValid() == true);

			// ualAddr of acquired discrete tuning signals is assigned in TuningData::buildTuningData() !
			//
			// s->setUalAddr(s->tuningAddr());

			Address16 addr = m_memoryMap.appendAcquiredDiscreteTuningSignal(*s);

			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		return result;
	}

	bool ModuleLogicCompiler::disposeNonAcquiredAnalogSignals()
	{
		bool result = true;

		int regBufOffset = -m_memoryMap.regBufStartAddr();	// minus is OK!

		for(Signal* s : m_nonAcquiredAnalogInputSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendNonAcquiredAnalogInputSignal(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_nonAcquiredAnalogOutputSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendNonAcquiredAnalogOutputSignal(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		if (result == false)
		{
			return false;
		}

		for(Signal* s : m_nonAcquiredAnalogInternalSignals)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendNonAcquiredAnalogInternalSignal(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		return result;
	}

	bool ModuleLogicCompiler::disposeNonAcquiredBuses()
	{
		bool result = true;

		int regBufOffset = -m_memoryMap.regBufStartAddr();	// minus is OK!

		for(Signal* s : m_nonAcquiredBuses)
		{
			TEST_PTR_CONTINUE(s);

			Address16 addr = m_memoryMap.appendNonAcquiredBus(*s);

			s->setUalAddr(addr);
			s->setRegBufAddr(addr);

			addr.addWord(regBufOffset);
			s->setRegValueAddr(addr);
		}

		result = m_memoryMap.recalculateAddresses();

		return result;
	}

	bool ModuleLogicCompiler::appendFbsForAnalogInOutSignalsConversion()
	{
		if (findFbsForAnalogInOutSignalsConversion() == false)
		{
			return false;
		}

		bool result = true;

		// append FBs  for analog input signals conversion
		//
		QVector<Signal*> analogInputSignals;

		analogInputSignals.append(m_acquiredAnalogInputSignals);
		analogInputSignals.append(m_nonAcquiredAnalogInputSignals);

		for(Signal* s : analogInputSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->isAnalog() == true);
			assert(s->isInput() == true);

			AppItem appItem;

			bool res = createFbForAnalogInputSignalConversion(*s, appItem);

			if (res == true)
			{
				AppFb* appFb = createAppFb(appItem);

				m_inOutSignalsToScalAppFbMap.insert(s->appSignalID(), appFb);
			}

			result &= res;
		}

		// append FBs  for analog output signals conversion
		//
		QVector<Signal*> analogOutputSignals;

		analogOutputSignals.append(m_acquiredAnalogOutputSignals);
		analogOutputSignals.append(m_nonAcquiredAnalogOutputSignals);

		for(Signal* s : analogOutputSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->isAnalog() == true);
			assert(s->isOutput() == true);

			AppItem appItem;

			bool res = createFbForAnalogOutputSignalConversion(*s, appItem);

			if (res == true)
			{
				AppFb* appFb = createAppFb(appItem);

				m_inOutSignalsToScalAppFbMap.insert(s->appSignalID(), appFb);
			}

			result &= res;
		}

		return result;
	}

	bool ModuleLogicCompiler::findFbsForAnalogInOutSignalsConversion()
	{
		bool result = true;

		// find AFB: scal_16ui_32fp, scal_16ui_32si, scal_32fp_16ui, scal_32si_16ui
		//
		const char* const fbScalCaption[] =
		{

			// for input signals conversion
			//

			"scale_16ui_fp",				// FB_SCALE_16UI_FP_INDEX
			"scale_16ui_si",				// FB_SCALE_16UI_SI_INDEX

			// for output signals conversion
			//

			"scale_fp_16ui",				// FB_SCALE_FP_16UI_INDEX
			"scale_si_16ui",				// FB_SCALE_SI_16UI_INDEX
		};

		/*const char* const FB_SCAL_K1_PARAM_CAPTION = "i_scal_k1_coef";
		const char* const FB_SCAL_K2_PARAM_CAPTION = "i_scal_k2_coef";*/

		const char* const FB_SCALE_X1_OPNAME = "x1";
		const char* const FB_SCALE_X2_OPNAME = "x2";
		const char* const FB_SCALE_Y1_OPNAME = "y1";
		const char* const FB_SCALE_Y2_OPNAME = "y2";

		const char* const FB_SCALE_INPUT_SIGNAL_CAPTION = "i_data";
		const char* const FB_SCALE_OUTPUT_SIGNAL_CAPTION = "o_result";

		for(const char* const fbCaption : fbScalCaption)
		{
			bool fbFound = false;

			for(std::shared_ptr<Afb::AfbElement> afbElement : m_lmDescription->afbs())
			{
				if (afbElement->caption() != fbCaption)
				{
					continue;
				}

				fbFound = true;

				FbScal fb;

				fb.caption = fbCaption;
				fb.pointer = afbElement;

				int index = 0;

				for(const Afb::AfbParam& afbParam : afbElement->params())
				{
					if (afbParam.opName() == FB_SCALE_X1_OPNAME)
					{
						fb.x1ParamIndex = index;
					}

					if (afbParam.opName() == FB_SCALE_X2_OPNAME)
					{
						fb.x2ParamIndex = index;
					}

					if (afbParam.opName() == FB_SCALE_Y1_OPNAME)
					{
						fb.y1ParamIndex = index;
					}

					if (afbParam.opName() == FB_SCALE_Y2_OPNAME)
					{
						fb.y2ParamIndex = index;
					}

					index++;
				}

				if (fb.x1ParamIndex == -1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Required parameter 'InputLow' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (fb.x2ParamIndex == -1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Required parameter 'InputHigh' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (fb.y1ParamIndex == -1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Required parameter 'OutputLow' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (fb.y2ParamIndex == -1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Required parameter 'OutputHigh' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (result == false)
				{
					break;
				}

				for(Afb::AfbSignal afbSignal : afbElement->inputSignals())
				{
					if (afbSignal.opName() == FB_SCALE_INPUT_SIGNAL_CAPTION)
					{
						fb.inputSignalIndex = afbSignal.operandIndex();
						break;
					}
				}

				if (fb.inputSignalIndex == -1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Required input signal %1 of AFB %2 is not found")).
							  arg(FB_SCALE_INPUT_SIGNAL_CAPTION).arg(fb.caption))
					result = false;
					break;
				}

				for(Afb::AfbSignal afbSignal : afbElement->outputSignals())
				{
					if (afbSignal.opName() == FB_SCALE_OUTPUT_SIGNAL_CAPTION)
					{
						fb.outputSignalIndex = afbSignal.operandIndex();
						break;
					}
				}

				if (fb.outputSignalIndex == -1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Required output signal %1 of AFB %2 is not found")).
							  arg(FB_SCALE_OUTPUT_SIGNAL_CAPTION).arg(fb.caption))
					result = false;
					break;
				}

				m_fbScal.append(fb);
			}

			if (fbFound == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Required AFB %1 is not found")).arg(fbCaption));
				result = false;
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createFbForAnalogInputSignalConversion(Signal& signal, AppItem& appItem)
	{
		assert(signal.isAnalog());
		assert(signal.isInput());
		assert(signal.equipmentID().isEmpty() == false);

		Hardware::DeviceObject* deviceObject = m_equipmentSet->deviceObject(signal.equipmentID());

		if (deviceObject == nullptr)
		{
			// Application signal '%1' is bound to unknown device object '%2'.
			//
			m_log->errALC5013(signal.appSignalID(), signal.equipmentID());
			return false;
		}

		Hardware::DeviceSignal* deviceSignal = deviceObject->toSignal();

		if (deviceSignal == nullptr)
		{
			// Input/output application signal '%1' should be bound to equipment signal.
			//
			m_log->errALC5091(signal.appSignalID());
			return false;
		}

		bool signalsIsCompatible = isDeviceAndAppSignalsIsCompatible(*deviceSignal, signal);

		if (signalsIsCompatible == true)
		{
			signal.setNeedConversion(false);
			return true;
		}

		signal.setNeedConversion(true);

		if (deviceSignal->format() != E::DataFormat::UnsignedInt || deviceSignal->size() != SIZE_16BIT)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Unknown conversion for signal %1, analogSignalFormat %2")).
					  arg(signal.appSignalID()).arg(static_cast<int>(signal.analogSignalFormat())));
			return false;
		}

		int x1 = signal.lowADC();
		int x2 = signal.highADC();

		if (x2 - x1 == 0)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Low and High ADC values of signal %1 are equal (= %2)")).arg(signal.appSignalID()).arg(x1));
			return false;
		}

		double y1 = signal.lowEngeneeringUnits();
		double y2 = signal.highEngeneeringUnits();

		QString errorMsg;

		bool result = false;

		switch(signal.analogSignalFormat())
		{
		case E::AnalogAppSignalFormat::Float32:
			{
				FbScal fb = m_fbScal[FB_SCALE_16UI_FP_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1));
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2));

				result = appItem.init(fb.pointer, errorMsg);

				if (errorMsg.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorMsg);
					result = false;
				}
			}

			break;

		case E::AnalogAppSignalFormat::SignedInt32:
			{
				FbScal& fb = m_fbScal[FB_SCALE_16UI_SI_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

				result = appItem.init(fb.pointer, errorMsg);

				if (errorMsg.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorMsg);
					result = false;
				}
			}

			break;

		default:
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Unknown conversion for signal %1, analogSignalFormat %2")).
					  arg(signal.appSignalID()).arg(static_cast<int>(signal.analogSignalFormat())));
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::createFbForAnalogOutputSignalConversion(Signal& signal, AppItem& appItem)
	{
		assert(signal.isAnalog());
		assert(signal.isOutput());
		assert(signal.equipmentID().isEmpty() == false);

		Hardware::DeviceObject* deviceObject = m_equipmentSet->deviceObject(signal.equipmentID());

		if (deviceObject == nullptr)
		{
			// Application signal '%1' is bound to unknown device object '%2'.
			//
			m_log->errALC5013(signal.appSignalID(), signal.equipmentID());
			return false;
		}

		Hardware::DeviceSignal* deviceSignal = deviceObject->toSignal();

		if (deviceSignal == nullptr)
		{
			// Input/output application signal '%1' should be bound to equipment signal.
			//
			m_log->errALC5091(signal.appSignalID());
			return false;
		}

		bool signalsIsCompatible = isDeviceAndAppSignalsIsCompatible(*deviceSignal, signal);

		if (signalsIsCompatible == true)
		{
			signal.setNeedConversion(false);
			return true;
		}

		signal.setNeedConversion(true);

		if (deviceSignal->format() != E::DataFormat::UnsignedInt || deviceSignal->size() != SIZE_16BIT)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Unknown conversion for signal %1, analogSignalFormat %2")).
					  arg(signal.appSignalID()).arg(static_cast<int>(signal.analogSignalFormat())));
			return false;
		}

		double x1 = signal.lowEngeneeringUnits();
		double x2 = signal.highEngeneeringUnits();

		if (x2 - x1 == 0.0)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Low and High Limit values of signal %1 are equal (= %2)")).arg(signal.appSignalID()).arg(x1));
			return false;
		}

		int y1 = signal.lowADC();
		int y2 = signal.highADC();

		QString errorMsg;

		bool result = false;

		switch(signal.analogSignalFormat())
		{
		case E::AnalogAppSignalFormat::Float32:
			{
				FbScal& fb = m_fbScal[FB_SCALE_FP_16UI_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

				result = appItem.init(fb.pointer, errorMsg);

				if (errorMsg.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorMsg);
					result = false;
				}
			}

			break;

		case E::AnalogAppSignalFormat::SignedInt32:
			{
				FbScal& fb = m_fbScal[FB_SCALE_SI_16UI_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1).toInt());
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2).toInt());

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

				result = appItem.init(fb.pointer, errorMsg);

				if (errorMsg.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorMsg);
					result = false;
				}
			}

			break;

		default:
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Unknown conversion for signal %1, analogSignalFormat %2")).
					  arg(signal.appSignalID()).arg(static_cast<int>(signal.analogSignalFormat())));
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::isDeviceAndAppSignalsIsCompatible(const Hardware::DeviceSignal& deviceSignal, const Signal& appSignal)
	{
		switch(deviceSignal.format())
		{
		case E::DataFormat::Float:

			if (deviceSignal.size() == SIZE_32BIT && appSignal.analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
			{
				return true;
			}

			return false;

		case E::DataFormat::SignedInt:

			if (deviceSignal.size() == SIZE_32BIT && appSignal.analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
			{
				return true;
			}

			return false;

		case E::DataFormat::UnsignedInt:

			return false;

		default:
			assert(false);
		}

		return false;
	}

	AppFb* ModuleLogicCompiler::createAppFb(const AppItem& appItem)
	{
		if (appItem.isFb() == false)
		{
			return nullptr;
		}

		AppFb* appFb = new AppFb(appItem);

		if (appFb->calculateFbParamValues(this) == false)
		{
			delete appFb;

			/*LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("FB '%1' parameters calculation error")).arg(appItem.caption()));*/
			return nullptr;
		}

		// get Functional Block instance
		//
		bool result = m_afbs.addInstance(appFb);

		if (result == false)
		{
			delete appFb;
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("FB '%1' instantiation error")).arg(appItem.caption()));
			return nullptr;
		}

		m_appFbs.insert(appFb);

		return appFb;
	}

	bool ModuleLogicCompiler::setOutputSignalsAsComputed()
	{
		if (m_moduleLogic == nullptr)
		{
			return true;
		}

		const std::list<AppLogicItem>& logicItems = m_moduleLogic->items();

		if (logicItems.empty() == true)
		{
			return true;
		}

		for(const AppLogicItem& item : logicItems)
		{
			if (item.m_fblItem == nullptr)
			{
				assert(false);
				return false;
			}

			if (item.m_fblItem->isOutputSignalElement() == false &&
				item.m_fblItem->isInOutSignalElement() == false)
			{
				continue;
			}

			const VFrame30::SchemaItemSignal* s = item.m_fblItem->toSignalElement();

			AppSignal* appSignal = m_appSignals.getSignal(s->appSignalIds());

			if (appSignal == nullptr)
			{
				assert(false);
				return false;
			}

			appSignal->setComputed();
		}

		return true;
	}

	bool ModuleLogicCompiler::processTxSignals()
	{
		if (m_optoModuleStorage == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString lmID = m_lm->equipmentIdTemplate();

		bool result = false;

		do
		{
			// add Tx signals from transmitters in txSignal lists of all Optical and Serial ports associated with current LM
			// check that added regulat Tx signals exists in current LM
			//
			if (processTransmitters() == false) break;

			// find raw tx signals and set it addresses
			//
			if (m_optoModuleStorage->initRawTxSignals(lmID) == false) break;

			// sort Tx signals lists of LM's associated opto ports
			//
			if (m_optoModuleStorage->sortTxSignals(lmID) == false) break;

			// calculate relative Tx signals addresses in tx buffers
			//
			if (m_optoModuleStorage->calculateTxSignalsAddresses(lmID) == false) break;

			// calculate txDataID
			//
			if (m_optoModuleStorage->calculateTxDataIDs(lmID) == false) break;

			// calculate tx buffers absolute addresses
			//
			if (m_optoModuleStorage->calculateTxBufAddresses(lmID) == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool ModuleLogicCompiler::processSerialRxSignals()
	{
		if (m_optoModuleStorage == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString lmID = m_lm->equipmentIdTemplate();

		bool result = false;

		do
		{
			// add regular Rx signals from receivers in rxSignal lists of all Serial (only!) ports associated with current LM
			// check that added regulat Rx signals exists in current LM
			//
			if (processSerialReceivers() == false) break;

			if (m_optoModuleStorage->initSerialRawRxSignals(lmID) == false) break;

			// sort Rx signals lists of LM's associated Serial ports
			//
			if (m_optoModuleStorage->sortSerialRxSignals(lmID) == false) break;

			// sort Rx signals lists of LM's associated Serial ports
			//
			if (m_optoModuleStorage->calculateSerialRxSignalsAddresses(lmID) == false) break;

			// calculate rxDataID for serial ports
			//
			if (m_optoModuleStorage->calculateSerialRxDataIDs(lmID) == false) break;

			result = true;
		}
		while (false);

		return result;
	}

	bool ModuleLogicCompiler::processTransmitters()
	{
		bool result = true;

		// process transmitters and add tx signals to Optical and Serial ports txSignals lists
		//
		for(const AppItem* item : m_appItems)
		{
			if (item == nullptr)
			{
				result = false;
				ASSERT_RESULT_FALSE_BREAK
			}

			if (item->isTransmitter() == true)
			{
				result &= processTransmitter(item);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::processTransmitter(const AppItem* item)
	{
		TEST_PTR_RETURN_FALSE(item);

		assert(item->isTransmitter() == true);

		const LogicTransmitter& transmitter = item->logicTransmitter();

		bool result = true;

		QVector<QPair<QString, QUuid>> connectedSignals;

		if (getSignalsConnectedToTransmitter(item, transmitter, connectedSignals) == false)
		{
			return false;
		}

		bool signalAlreadyInTxList = false;

		for(const QPair<QString, QUuid>& connectedSignal : connectedSignals)
		{
			QString connectedSignalID = connectedSignal.first;
			QUuid connectedSignalUuid = connectedSignal.second;

			Signal* s = m_signals->getSignal(connectedSignalID);

			if (s == nullptr)
			{
				m_log->errALC5000(connectedSignalID, connectedSignalUuid);
				ASSERT_RETURN_FALSE
			}

			if (s->isBus() == true)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString("Bus signals connection to transmitters is not implemented now (Logic schema %1)").arg(item->schemaID()));
				m_log->addItemsIssues(OutputMessageLevel::Error, item->guid(), item->schemaID());
				return false;
			}

			result &= m_optoModuleStorage->appendTxSignal(item->schemaID(), transmitter.connectionId(), transmitter.guid(),
													   m_lm->equipmentIdTemplate(),
													   s,
													   &signalAlreadyInTxList);
			if (signalAlreadyInTxList == true)
			{
				// The signal '%1' is repeatedly connected to the transmitter '%2'
				//
				m_log->errALC5029(connectedSignalID, transmitter.connectionId(), connectedSignalUuid, transmitter.guid());
				result = false;
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::getSignalsConnectedToTransmitter(const AppItem* item, const LogicTransmitter& transmitter, QVector<QPair<QString, QUuid>>& connectedSignals)
	{
		connectedSignals.clear();

		const std::vector<LogicPin>& inPins = transmitter.inputs();

		for(const LogicPin& inPin : inPins)
		{
			if (inPin.dirrection() != VFrame30::ConnectionDirrection::Input)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Input pin %1 has wrong direction")).arg(inPin.caption()));
				ASSERT_RETURN_FALSE;
			}

			const std::vector<QUuid>& associatedOutPins = inPin.associatedIOs();

			if (associatedOutPins.size() > 1)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("More than one pin is connected to the input")));
				ASSERT_RETURN_FALSE;
			}

			if (associatedOutPins.size() < 1)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Has no output pins is connected to the input")));
				ASSERT_RETURN_FALSE;
			}

			QUuid connectedPinGuid = associatedOutPins[0];

			AppItem* connectedPinParent = m_pinParent.value(connectedPinGuid, nullptr);

			if (connectedPinParent == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			QUuid connectedSignalUuid;

			if (connectedPinParent->isSignal())
			{
				// input connected to real signal
				//
				connectedSignalUuid = connectedPinParent->guid();
			}
			else
			{
				// connectedPinParent is FB
				//
				if (m_outPinSignal.contains(connectedPinGuid) == false)
				{
					// All transmitter inputs must be directly linked to a signals.
					//
					m_log->errALC5027(transmitter.guid(), item->schemaID());
					return false;
				}

				QList<QUuid> ids = m_outPinSignal.values(connectedPinGuid);

				if (ids.count() >  1)
				{
					// Transmitter input can be linked to one signal only.
					//
					m_log->errALC5026(transmitter.guid(), ids);
					return false;
				}
				else
				{
					connectedSignalUuid = ids.first();
				}
			}

			AppSignal* appSignal = m_appSignals.value(connectedSignalUuid, nullptr);

			if (appSignal == nullptr)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Signal is not found, GUID: %1")).arg(connectedSignalUuid.toString()));
				ASSERT_RETURN_FALSE
			}

			QString connectedSignalID = appSignal->appSignalID();

			connectedSignals.append(QPair<QString, QUuid>(connectedSignalID, connectedSignalUuid));
		}

		return true;
	}

	bool ModuleLogicCompiler::processSerialReceivers()
	{
		bool result = true;

		// process receivers and add regular tx signals to Serial (only!) ports rxSignals lists
		//
		for(const AppItem* item : m_appItems)
		{
			if (item == nullptr)
			{
				result = false;
				ASSERT_RESULT_FALSE_BREAK
			}

			result &= processSerialReceiver(item);
		}

		return result;
	}

	bool ModuleLogicCompiler::processSerialReceiver(const AppItem* item)
	{
		TEST_PTR_RETURN_FALSE(item);

		if (item->isReceiver() == false)
		{
			return true;				// item is not receiver, nothing to processing
		}

		const LogicReceiver& receiver = item->logicReceiver();

		QString connectionID = receiver.connectionId();

		std::shared_ptr<Hardware::Connection> connection = m_optoModuleStorage->getConnection(connectionID);

		if (connection == nullptr)
		{
			// Receiver is linked to unknown opto connection '%1'.
			//
			m_log->errALC5025(connectionID, item->guid(), item->schemaID());
			return false;
		}

		if (connection->isSinglePort() == false)
		{
			return true;				// process Serial connections receivers only
		}

		QString rxSignalID = receiver.appSignalId();

		if (m_chassisSignals.contains(rxSignalID) == false)
		{
			// Serial Rx signal '%1' is not associated with LM '%2' (Logic schema '%3').
			//
			m_log->errALC5191(rxSignalID, m_lm->equipmentIdTemplate(), item->guid(), item->schemaID());
			return false;
		}

		Signal* rxSignal = m_signals->getSignal(rxSignalID);

		if (rxSignal == nullptr)
		{
			m_log->errALC5000(rxSignalID, item->guid());
			return false;
		}

		bool result = m_optoModuleStorage->appendSerialRxSignal(item->schemaID(),
																	connectionID,
																	item->guid(),
																	m_lm->equipmentIdTemplate(),
																	rxSignal);
		return result;
	}

	bool ModuleLogicCompiler::setOptoRawInSignalsAsComputed()
	{
		if (m_optoModuleStorage == nullptr)
		{
			assert(false);
			return false;
		}

		QList<Hardware::OptoModuleShared> optoModules = m_optoModuleStorage->getLmAssociatedOptoModules(m_lm->equipmentIdTemplate());

		if (optoModules.isEmpty())
		{
			return true;
		}

		bool result = true;

		for(Hardware::OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			QList<Hardware::OptoPortShared> optoPorts;

			optoModule->getOptoPorts(optoPorts);

			if (optoPorts.isEmpty())
			{
				continue;
			}

			for(Hardware::OptoPortShared& port : optoPorts)
			{

				const Hardware::RawDataDescription& rd = port->rawDataDescription();

				for(const Hardware::RawDataDescriptionItem& item : rd)
				{
					if (item.type == Hardware::RawDataDescriptionItem::Type::RxSignal)
					{
						AppSignal* appSignal = m_appSignals.getSignal(item.appSignalID);

						if (appSignal != nullptr)
						{
							appSignal->setComputed();
						}
					}
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::finalizeOptoConnectionsProcessing()
	{
		bool result = true;

		QString lmID = m_lm->equipmentIdTemplate();

		// copying optical ports txSignals lists to connected ports rxSignals lists
		//
		result &= m_optoModuleStorage->copyOpticalPortsTxInRxSignals(lmID);

		// calculate absoulute addresses of receving buffers
		//
		result &= m_optoModuleStorage->calculateRxBufAddresses(lmID);

		return result;
	}

	bool ModuleLogicCompiler::generateAppStartCommand()
	{
		Command cmd;

		// first command in program!

		cmd.appStart(0);		// real address is set in startAppLogicCode function

		cmd.setComment(tr("set address of application logic code start"));
		m_code.append(cmd);
		m_code.newLine();

		return true;
	}

	bool ModuleLogicCompiler::initAfbs()
	{
		LOG_MESSAGE(m_log, QString(tr("Generation of AFB initialization code...")));

		bool result = true;

		m_code.comment("FB's initialization code");
		m_code.newLine();

		QHash<QString, int> instantiatorStrIDsMap;

		for(LogicAfb* fbl : m_afbs)
		{
			for(AppFb* appFb : m_appFbs)
			{
				if (appFb->afbStrID() != fbl->strID())
				{
					continue;
				}

				if (appFb->hasRam() == true)
				{
					// initialize all params for each instance of FB with RAM
					//
					result &= initAppFbParams(appFb, false);
				}
				else
				{
					// FB without RAM initialize once for all instances
					// initialize instantiator params only
					//
					QString instantiatorID = appFb->instantiatorID();

					if (instantiatorStrIDsMap.contains(instantiatorID) == false)
					{
						instantiatorStrIDsMap.insert(instantiatorID, 0);

						result &= initAppFbParams(appFb, true);
					}
				}
			}
		}

		Command cmd;

		cmd.stop();

		m_code.comment(tr("End of FB's initialization code section"));
		m_code.newLine();
		m_code.append(cmd);
		m_code.newLine();

		return result;
	}

	bool ModuleLogicCompiler::initAppFbParams(AppFb* appFb, bool /* instantiatorsOnly */)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return false;
		}

		const AppFbParamValuesArray& appFbParamValues = appFb->paramValuesArray();

		if (appFbParamValues.isEmpty())
		{
			return true;
		}

		bool result = true;

		QString fbCaption = appFb->caption();
		int fbOpcode = appFb->opcode();
		int fbInstance = appFb->instance();

		m_code.comment(QString(tr("Initialization of %1 (fbtype %2, opcode %3, instance %4, %5, %6)")).
				arg(fbCaption).
				arg(appFb->typeCaption()).
				arg(fbOpcode).
				arg(fbInstance).
				arg(appFb->instantiatorID()).
			   arg(appFb->hasRam() ? "has RAM" : "non RAM"));

		displayAfbParams(*appFb);

		m_code.newLine();

		bool commandAdded = false;

		for(const AppFbParamValue& paramValue : appFbParamValues)
		{
			int operandIndex = paramValue.operandIndex();

			if (operandIndex == AppFbParamValue::NOT_FB_OPERAND_INDEX)
			{
				continue;
			}

			QString opName = paramValue.opName();

			Command cmd;

			if (paramValue.type() == E::SignalType::Discrete)
			{
				// for discrete parameters
				//
				cmd.writeFuncBlockConst(fbOpcode, fbInstance, operandIndex, paramValue.unsignedIntValue(), fbCaption);
				cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.unsignedIntValue()));

				m_code.append(cmd);

				commandAdded = true;

				continue;
			}

			// for analog parameters
			//

			if (paramValue.dataSize() == SIZE_32BIT)
			{
				switch (paramValue.dataFormat())
				{
				case E::DataFormat::UnsignedInt:
					cmd.writeFuncBlockConstInt32(fbOpcode, fbInstance, operandIndex, paramValue.unsignedIntValue(), fbCaption);
					cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.unsignedIntValue()));
					break;

				case E::DataFormat::SignedInt:
					cmd.writeFuncBlockConstInt32(fbOpcode, fbInstance, operandIndex, paramValue.signedIntValue(), fbCaption);
					cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.signedIntValue()));
					break;

				case E::DataFormat::Float:
					cmd.writeFuncBlockConstFloat(fbOpcode, fbInstance, operandIndex, paramValue.floatValue(), fbCaption);
					cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.floatValue()));
					break;

				default:
					assert(false);
				}
			}
			else
			{
				// other sizes
				//
				switch (paramValue.dataFormat())
				{
				case E::DataFormat::UnsignedInt:
					cmd.writeFuncBlockConst(fbOpcode, fbInstance, operandIndex, paramValue.unsignedIntValue(), fbCaption);
					cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.unsignedIntValue()));
					break;

				case E::DataFormat::SignedInt:
					cmd.writeFuncBlockConst(fbOpcode, fbInstance, operandIndex, paramValue.signedIntValue(), fbCaption);
					cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.signedIntValue()));
					break;

				case E::DataFormat::Float:
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Afb parameter '%1' with Float data format must have dataSize == 32")).arg(opName));
					result = false;
					break;

				default:
					assert(false);

					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Unknown Afb parameter data format"));
					result = false;
				}
			}

			m_code.append(cmd);

			commandAdded = true;
		}

		if (commandAdded == true)
		{
			m_code.newLine();
		}

		return result;
	}


	bool ModuleLogicCompiler::displayAfbParams(const AppFb& appFb)
	{
		const AppFbParamValuesArray& appFbParamValues = appFb.paramValuesArray();

		Comment cmt;

		for(const AppFbParamValue& paramValue : appFbParamValues)
		{
			if (paramValue.isVisible() == false && paramValue.isNoFbOperand() == true)
			{
				continue;
			}

			QString commentStr = paramValue.caption();

			if (paramValue.isNoFbOperand() == false)
			{
				commentStr.append(QString(" (%1)").arg(paramValue.opName()));
			}

			commentStr.append(QString(" = %1").arg(paramValue.toString()));

			cmt.setComment(commentStr);

			m_code.append(cmt);
		}

		return true;
	}

	bool ModuleLogicCompiler::startAppLogicCode()
	{
		// set APPSTART command to current address
		//
		Command cmd;

		cmd.appStart(m_code.commandAddress());

		m_code.replaceAt(0, cmd);

		//
		//

		Comment comment;

		comment.setComment(tr("Start of application logic code"));

		m_code.append(comment);
		m_code.newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredRawDataInRegBuf()
	{
		m_code.comment("Copy acquired raw data");
		m_code.newLine();

		return true;
	}

	bool ModuleLogicCompiler::convertAnalogInputSignals()
	{
		bool result = true;

		QVector<Signal*> analogInputSignals;

		analogInputSignals.append(m_acquiredAnalogInputSignals);
		analogInputSignals.append(m_nonAcquiredAnalogInputSignals);

		if (analogInputSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.comment("Convertion of input analog signals");
		m_code.newLine();

		Command cmd;

		for(Signal* s : analogInputSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			assert(s->isAnalog() == true);
			assert(s->isInput() == true);
			assert(s->dataSize() == SIZE_32BIT);
			assert(s->ualAddr().isValid() == true);
			assert(s->ioBufAddr().isValid() == true);

			if (s->needConversion() == false)
			{
				// signal isn't need conversion
				// copy signal only
				//
				cmd.mov32(s->ualAddr().offset(), s->ioBufAddr().offset());
				cmd.setComment(QString("copy analog input %1").arg(s->appSignalID()));
				m_code.append(cmd);

				continue;
			}

			AppFb* appFb = m_inOutSignalsToScalAppFbMap.value(s->appSignalID(), nullptr);

			if (appFb == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			FbScal fbScal;

			switch(s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				fbScal = m_fbScal[FB_SCALE_16UI_FP_INDEX];
				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				fbScal = m_fbScal[FB_SCALE_16UI_SI_INDEX];
				break;

			default:
				assert(false);			// unknown format
				return false;
			}

			assert(s->ioBufAddr().isValid() == true);
			assert(s->ualAddr().isValid() == true);

			cmd.writeFuncBlock(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
							   s->ioBufAddr().offset(), appFb->caption());
			cmd.setComment(QString(tr("conversion of analog input %1")).arg(s->appSignalID()));
			m_code.append(cmd);

			cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.clearComment();
			m_code.append(cmd);

			cmd.readFuncBlock32(s->ualAddr().offset(), appFb->opcode(), appFb->instance(),
								fbScal.outputSignalIndex, appFb->caption());
			m_code.append(cmd);
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copySerialRxSignals()
	{
		// copying serial rx signals from rx buffers to signals in LM memory
		//
		QList<Hardware::OptoPortShared> ports;

		m_optoModuleStorage->getLmAssociatedOptoPorts(m_lm->equipmentIdTemplate(), ports);

		bool first = true;

		Comment comment;
		Command cmd;

		bool result = true;

		for(const Hardware::OptoPortShared& port : ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->isSinglePortConnection() == false || port->rxSignalsCount() == 0)
			{
				continue;
			}

			if (first == true)
			{
				comment.setComment("Copying serial ports rx signals from rx buffers to signals in LMs memory");

				m_code.append(comment);
				m_code.newLine();

				first = false;
			}

			comment.setComment(QString("Copying rx signals of serial port %1").arg(port->equipmentID()));

			m_code.append(comment);
			m_code.newLine();

			const HashedVector<QString, Hardware::TxRxSignalShared>& rxSignals = port->rxSignals();

			for(const Hardware::TxRxSignalShared& rxSignal : rxSignals)
			{
				if(rxSignal == nullptr)
				{
					assert(false);
					continue;
				}

				switch(rxSignal->signalType())
				{
				case E::SignalType::Analog:
					result &= copySerialRxAnalogSignal(port, rxSignal);
					break;

				case E::SignalType::Discrete:
					result &= copySerialRxDiscreteSignal(port, rxSignal);
					break;

				default:
					assert(false);
				}
			}
		}

		if (first == false)
		{
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copySerialRxAnalogSignal(Hardware::OptoPortShared port, Hardware::TxRxSignalShared rxSignal)
	{
		TEST_PTR_RETURN_FALSE(port);
		TEST_PTR_RETURN_FALSE(rxSignal);

		if (rxSignal->signalType() != E::SignalType::Analog ||
			rxSignal->dataSize() != SIZE_32BIT)
		{
			ASSERT_RETURN_FALSE;
		}

		Signal* s = m_signals->getSignal(rxSignal->appSignalID());

		if (s == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (s->ualAddr().isValid() == false)
		{
			ASSERT_RETURN_FALSE;
		}

		bool res = s->isCompatibleFormat(rxSignal->signalType(), rxSignal->dataFormat(), rxSignal->dataSize(), rxSignal->byteOrder());

		if (res == false)
		{
			ASSERT_RETURN_FALSE;
		}

		Command cmd;

		cmd.mov32(s->ualAddr().offset(), port->rxBufAbsAddress() + rxSignal->addrInBuf().offset());
		cmd.setComment(QString("copy rx signal %1").arg(rxSignal->appSignalID()));

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::copySerialRxDiscreteSignal(Hardware::OptoPortShared port, Hardware::TxRxSignalShared rxSignal)
	{
		TEST_PTR_RETURN_FALSE(port);
		TEST_PTR_RETURN_FALSE(rxSignal);

		if (rxSignal->signalType() != E::SignalType::Discrete)
		{
			ASSERT_RETURN_FALSE;
		}

		Signal* s = m_signals->getSignal(rxSignal->appSignalID());

		if (s == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (s->ualAddr().isValid() == false)
		{
			ASSERT_RETURN_FALSE;
		}

		Command cmd;

		cmd.movBit(s->ualAddr().offset(), s->ualAddr().bit(),
				   port->rxBufAbsAddress() + rxSignal->addrInBuf().offset(),
				   rxSignal->addrInBuf().bit());
		cmd.setComment(QString("copy rx signal %1").arg(rxSignal->appSignalID()));

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateAppLogicCode()
	{
		LOG_MESSAGE(m_log, QString("Generation of application logic code was started..."));

		bool result = true;

		m_code.comment("Application logic code");
		m_code.newLine();

		for(AppItem* appItem : m_appItems)
		{
			TEST_PTR_RETURN_FALSE(appItem)

			if (appItem->isSignal())
			{
				result &= generateAppSignalCode(appItem);

				/*if (result == false)
				{
					break;
				}*/

				continue;
			}

			if (appItem->isFb())
			{
				result &= generateFbCode(appItem);

				/*if (result == false)
				{
					break;
				}*/

				continue;
			}

			if (appItem->isConst())
			{
				continue;
			}

			if (appItem->isTransmitter())
			{
				// no special code generation for transmitter here
				// code for transmitters is generate in copyOptoConnectionsTxData()
				//
				continue;
			}

			if (appItem->isReceiver())
			{
				// no special code generation for receiver here
				// code for receivers is generate in:
				//		generateWriteReceiverToSignalCode()
				//		generateWriteReceiverToFbCode()
				//
				continue;
			}

			if (appItem->isTerminator())
			{
				// no needed special code generation for terminator
				continue;
			}

			m_log->errALC5011(appItem->label(), appItem->schemaID(), appItem->guid());		// Application item '%1' has unknown type, SchemaID '%2'. Contact to the RPCT developers.
			result = false;
			break;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAppSignalCode(const AppItem* appItem)
	{
		if (!m_appSignals.contains(appItem->guid()))
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signal is not found, GUID: %1")).arg(appItem->guid().toString()));
			return false;
		}

		AppSignal* appSignal = m_appSignals[appItem->guid()];

		TEST_PTR_RETURN_FALSE(appSignal)

		bool result = true;

		if (appSignal->isResultSaved() == true)
		{
			return true;				// signal value is already set
		}

		int inPinsCount = 1;

		for(LogicPin inPin : appItem->inputs())
		{
			if (inPin.dirrection() != VFrame30::ConnectionDirrection::Input)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			if (inPinsCount > 1)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			inPinsCount++;

			int connectedPinsCount = 1;

			for(QUuid connectedPinGuid : inPin.associatedIOs())
			{
				if (connectedPinsCount > 1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("More than one pin is connected to the input")));

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

				if (connectedPinParent->isConst())
				{
					result &= generateWriteConstToSignalCode(*appSignal, connectedPinParent->logicConst());
					continue;
				}

				if (connectedPinParent->isReceiver())
				{
					result &= generateWriteReceiverToSignalCode(connectedPinParent->logicReceiver(), *appSignal, connectedPinGuid);
					continue;
				}

				QUuid srcSignalGuid;

				if (connectedPinParent->isSignal())
				{
					// input connected to real signal
					//
					srcSignalGuid = connectedPinParent->guid();
				}
				else
				{
					// connectedPinParent is FB
					//
					if (!m_outPinSignal.contains(connectedPinGuid))
					{
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								  QString(tr("Output pin is not found, GUID: %1")).arg(connectedPinGuid.toString()));

						ASSERT_RESULT_FALSE_BREAK
					}

					srcSignalGuid = m_outPinSignal[connectedPinGuid];
				}

				if (!m_appSignals.contains(srcSignalGuid))
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Signal is not found, GUID: %1")).arg(srcSignalGuid.toString()));

					ASSERT_RESULT_FALSE_BREAK
				}

				AppSignal* srcAppSignal = m_appSignals[srcSignalGuid];

				if (srcAppSignal == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				if (!srcAppSignal->isComputed())
				{
					RESULT_FALSE_BREAK
				}

				result &= generateWriteSignalToSignalCode(*appSignal, *srcAppSignal);
			}

			if (result == false)
			{
				break;
			}
		}

		if(!appSignal->isComputed())
		{
			m_log->errALC5002(appSignal->schemaID(), appSignal->appSignalID(), appSignal->guid());			// Value of signal '%1' is undefined.
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateWriteConstToSignalCode(AppSignal& appSignal, const LogicConst& constItem)
	{
		if (appSignal.enableTuning() == true)
		{
			// Can't assign value to tuningable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5071(appSignal.schemaID(), appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		if (appSignal.isInput() == true)
		{
			// Can't assign value to input signal '%1' (Logic schema '%2').
			//
			m_log->errALC5087(appSignal.schemaID(), appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		quint16 ramAddrOffset = appSignal.ualAddr().offset();
		quint16 ramAddrBit = appSignal.ualAddr().bit();

		Command cmd;

		switch(appSignal.signalType())
		{
		case E::SignalType::Discrete:

			if (constItem.isDiscrete() == false)
			{
				// Uncompatible constant type (Logic schema %1).
				//
				m_log->errALC5028(constItem.guid(), appSignal.schemaID());
				return false;
			}
			else
			{
				int constValue = constItem.discreteValue();

				if (constValue == 0 || constValue == 1)
				{
					cmd.movBitConst(ramAddrOffset, ramAddrBit, constValue);
					cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
								   arg(appSignal.appSignalID()).arg(appSignal.regBufAddr().toString()).arg(constValue));
				}
				else
				{
					// Discrete constant must have value 0 or 1 (Logic schema %1).
					//
					m_log->errALC5086(constItem.guid(), appSignal.schemaID());
					return false;
				}
			}
			break;

		case E::SignalType::Analog:

			switch(appSignal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				if (constItem.isIntegral() == true)
				{
					cmd.movConstInt32(ramAddrOffset, constItem.intValue());
					cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
								   arg(appSignal.appSignalID()).arg(appSignal.regBufAddr().toString()).arg(constItem.intValue()));
				}
				else
				{
					// Uncompatible constant type (Logic schema %1).
					//
					m_log->errALC5028(constItem.guid(), appSignal.schemaID());
					return false;
				}
				break;

			case E::AnalogAppSignalFormat::Float32:
				if (constItem.isFloat() == true)
				{
					cmd.movConstFloat(ramAddrOffset, constItem.floatValue());
					cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
								   arg(appSignal.appSignalID()).arg(appSignal.regBufAddr().toString()).arg(constItem.floatValue()));
				}
				else
				{
					// Uncompatible constant type (Logic schema %1).
					//
					m_log->errALC5028(constItem.guid(), appSignal.schemaID());
					return false;
				}
				break;

			default:
				assert(false);
				return false;
			}

			break;

		default:
			assert(false);
			return false;
		}

		if (cmd.isValidCommand())
		{
			m_code.append(cmd);
		}

		appSignal.setResultSaved();

		return true;
	}

	bool ModuleLogicCompiler::generateWriteReceiverToSignalCode(const LogicReceiver& receiver, AppSignal& appSignal, const QUuid& pinGuid)
	{
		std::shared_ptr<Hardware::Connection> connection = m_optoModuleStorage->getConnection(receiver.connectionId());

		if (connection == nullptr)
		{
			// Receiver is linked to unknown opto connection '%1'.
			//
			m_log->errALC5025(receiver.connectionId(), receiver.guid(), appSignal.schemaID());
			return false;
		}

		if (appSignal.enableTuning() == true)
		{
			// Can't assign value to tuningable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5071(appSignal.schemaID(), appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		if (appSignal.isInput() == true)
		{
			// Can't assign value to input signal '%1' (Logic schema '%2').
			//
			m_log->errALC5087(appSignal.schemaID(), appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		Signal* destSignal = m_signals->getSignal(appSignal.appSignalID());

		if (destSignal == nullptr)
		{
			// Signal identifier '%1' is not found.
			//
			m_log->errALC5000(appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		Command cmd;

		if (receiver.isOutputPin(pinGuid) == true)
		{
			SignalAddress16 rxAddress;

			if (m_optoModuleStorage->getRxSignalAbsAddress(appSignal.schemaID(),
														   receiver.connectionId(),
														   receiver.appSignalId(),
														   m_lm->equipmentIdTemplate(),
														   receiver.guid(),
														   rxAddress) == false)
			{
				return false;
			}

			Signal* srcSignal = m_signals->getSignal(receiver.appSignalId());

			if (srcSignal == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(receiver.appSignalId(), receiver.guid());
				return false;
			}

			if (checkSignalsCompatibility(*srcSignal, receiver.guid(), *destSignal, appSignal.guid()) == false)
			{
				return false;
			}

			QString str;

			str = QString(tr("%1 >> %2 => %3")).arg(receiver.connectionId()).arg(receiver.appSignalId()).arg(destSignal->appSignalID());

			if (destSignal->isAnalog())
			{
				cmd.mov32(destSignal->ualAddr().offset(), rxAddress.offset());
			}
			else
			{
				if (destSignal->isDiscrete())
				{
					cmd.movBit(destSignal->ualAddr().offset(), destSignal->ualAddr().bit(),
							   rxAddress.offset(), rxAddress.bit());
				}
				else
				{
					assert(false);		// unknown type of signal
					return false;
				}
			}

			cmd.setComment(str);
			m_code.append(cmd);
			m_code.newLine();

			return true;
		}

		if (receiver.isValidityPin(pinGuid) == true)
		{
			if (destSignal->isDiscrete() == false)
			{
				// Discrete signal '%1' is connected to analog signal '%2'.
				//
				m_log->errALC5037(QString("%1 validity").arg(receiver.connectionId()),
								  receiver.guid(),
								  destSignal->appSignalID(),
								  appSignal.guid());
				return false;
			}

			Address16 validityAddr;

			bool res = m_optoModuleStorage->getOptoPortValidityAbsAddr(m_lm->equipmentIdTemplate(),
																	   receiver.connectionId(),
																	   appSignal.schemaID(),
																	   receiver.guid(),
																	   validityAddr);
			if (res == false)
			{
				return false;
			}

			QString str;

			str = QString(tr("%1 >> validity => %2")).
					arg(receiver.connectionId()).
					arg(destSignal->appSignalID());

			cmd.movBit(destSignal->ualAddr().offset(), destSignal->ualAddr().bit(),
						validityAddr.offset(), validityAddr.bit());

			cmd.setComment(str);
			m_code.append(cmd);
			return true;
		}

		LOG_INTERNAL_ERROR(m_log);

		assert(false);		// unknown pin type

		return false;
	}

	bool ModuleLogicCompiler::generateWriteSignalToSignalCode(AppSignal& appSignal, const AppSignal& srcSignal)
	{
		if (appSignal.enableTuning() == true)
		{
			// Can't assign value to tuningable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5071(appSignal.schemaID(), appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		if (appSignal.isInput() == true)
		{
			// Can't assign value to input signal '%1' (Logic schema '%2').
			//
			m_log->errALC5087(appSignal.schemaID(), appSignal.appSignalID(), appSignal.guid());
			return false;
		}

		if (appSignal.appSignalID() == srcSignal.appSignalID())
		{
			return true;
		}

		if (appSignal.isAnalog() == true)
		{
			if (srcSignal.isAnalog() == false)
			{
				msg = QString(tr("Discrete signal %1 connected to analog signal %2")).
						arg(srcSignal.appSignalID()).arg(appSignal.appSignalID());

				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

				return false;
			}

			if (appSignal.analogSignalFormat() != srcSignal.analogSignalFormat())
			{
				msg = QString(tr("Signals %1 and %2 data formats are not compatible")).
						arg(appSignal.appSignalID()).arg(srcSignal.appSignalID());

				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

				return false;
			}

			if (appSignal.dataSize() != srcSignal.dataSize())
			{
				msg = QString(tr("Signals %1 and %2 have different data sizes")).
						arg(appSignal.appSignalID()).arg(srcSignal.appSignalID());

				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

				return false;
			}
		}
		else
		{
			if (appSignal.isDiscrete() == true)
			{
				if (srcSignal.isDiscrete() == false)
				{
					msg = QString(tr("Analog signal %1 connected to discrete signal %2")).
							arg(srcSignal.appSignalID()).arg(appSignal.appSignalID());

					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

					return false;
				}
			}
			else
			{
				assert(false);		// unknown afb signal type
				return false;
			}
		}

		if (appSignal.isAnalog() && srcSignal.isAnalog() && appSignal.analogSignalFormat() != srcSignal.analogSignalFormat())
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signals %1 and  %2 is not compatible by dataFormat")).
					  arg(srcSignal.appSignalID()).arg(appSignal.appSignalID()));

			return false;
		}

		if (appSignal.dataSize() != srcSignal.dataSize())
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signals %1 and  %2 is not compatible by dataSize")).
					  arg(srcSignal.appSignalID()).arg(appSignal.appSignalID()));

			return false;
		}

		Command cmd;

		int srcRamAddrOffset = srcSignal.ualAddr().offset();
		int srcRamAddrBit = srcSignal.ualAddr().bit();

		int destRamAddrOffset = appSignal.ualAddr().offset();
		int destRamAddrBit = appSignal.ualAddr().bit();

		if (srcRamAddrOffset == -1 || srcRamAddrBit == -1)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signal %1 RAM addreess is not calculated")).
					  arg(srcSignal.appSignalID()));
			return false;
		}

		if (destRamAddrOffset == -1 || destRamAddrBit == -1)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signal %1 RAM addreess is not calculated")).
					  arg(appSignal.appSignalID()));
			return false;
		}

		if (appSignal.isAnalog() == true)
		{
			// move value of analog signal
			//
			switch(appSignal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
			case E::AnalogAppSignalFormat::SignedInt32:
				cmd.mov32(destRamAddrOffset, srcRamAddrOffset);
				break;

			default:
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Unknown analog signal format of signal %1 - %2 bit")).
						  arg(appSignal.appSignalID()).arg(appSignal.dataSize()));
				return false;
			}
		}
		else
		{
			// move value of discrete signal
			//
			cmd.movBit(destRamAddrOffset, destRamAddrBit, srcRamAddrOffset, srcRamAddrBit);
		}

		cmd.setComment(QString(tr("%1 (reg %2) <= %3 (reg %4)")).
					   arg(appSignal.appSignalID()).arg(appSignal.regBufAddr().toString()).
					   arg(srcSignal.appSignalID()).arg(srcSignal.regBufAddr().toString()));
		m_code.append(cmd);
		m_code.newLine();

		appSignal.setResultSaved();

		return true;
	}

	bool ModuleLogicCompiler::generateFbCode(const AppItem* appItem)
	{
		if (!m_appFbs.contains(appItem->guid()))
		{
			ASSERT_RETURN_FALSE
		}

		const AppFb* appFb = m_appFbs[appItem->guid()];

		TEST_PTR_RETURN_FALSE(appFb)

		bool result = false;

		do
		{
			if (writeFbInputSignals(appFb) == false) break;

			if (startFb(appFb) == false) break;

			if (readFbOutputSignals(appFb) == false) break;

			if (addToComparatorStorage(appFb) == false) break;

			result = true;
		}
		while(false);

		m_code.newLine();

		return result;
	}

	bool ModuleLogicCompiler::writeFbInputSignals(const AppFb* appFb)
	{
		bool result = true;

		for(LogicPin inPin : appFb->inputs())
		{
			if (inPin.dirrection() != VFrame30::ConnectionDirrection::Input)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Input pin %1 of %2 has wrong direction")).arg(inPin.caption()).arg(appFb->strID()));
				RESULT_FALSE_BREAK
			}

			int connectedPinsCount = 1;

			for(QUuid connectedPinGuid : inPin.associatedIOs())
			{
				if (connectedPinsCount > 1)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("More than one pin is connected to the input")));

					RESULT_FALSE_BREAK
				}

				connectedPinsCount++;

				if (!m_pinParent.contains(connectedPinGuid))
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Pin is not found, GUID %1")).arg(connectedPinGuid.toString()));

					RESULT_FALSE_BREAK
				}

				AppItem* connectedPinParent = m_pinParent[connectedPinGuid];

				if (connectedPinParent == nullptr)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Pin parent is NULL, pin GUID ")).arg(connectedPinGuid.toString()));
					RESULT_FALSE_BREAK
				}

				if (connectedPinParent->isConst())
				{
					result &= generateWriteConstToFbCode(*appFb, inPin, connectedPinParent->logicConst());
					continue;
				}

				if (connectedPinParent->isReceiver())
				{
					result &= genearateWriteReceiverToFbCode(*appFb, inPin, connectedPinParent->logicReceiver(), connectedPinGuid);
					continue;
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
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								  QString(tr("Output pin is not found, GUID: %1")).arg(connectedPinGuid.toString()));

						RESULT_FALSE_BREAK
					}

					signalGuid = m_outPinSignal[connectedPinGuid];
				}

				if (!m_appSignals.contains(signalGuid))
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()));

					RESULT_FALSE_BREAK
				}

				AppSignal* appSignal = m_appSignals[signalGuid];

				if (appSignal == nullptr)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Signal pointer is NULL, signal GUID: %1")).arg(signalGuid.toString()));

					RESULT_FALSE_BREAK
				}

				if (!appSignal->isComputed())
				{
					m_log->errALC5002(appSignal->schemaID(), appSignal->appSignalID(), appSignal->guid());			// Value of signal '%1' is undefined.
					RESULT_FALSE_BREAK
				}

				result &= generateWriteSignalToFbCode(*appFb, inPin, *appSignal);
			}

			if (result == false)
			{
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::generateWriteConstToFbCode(const AppFb& appFb, const LogicPin& inPin, const LogicConst& constItem)
	{
		quint16 fbType = appFb.opcode();
		quint16 fbInstance = appFb.instance();
		quint16 fbParamNo = inPin.afbOperandIndex();

		bool result = false;

		LogicAfbSignal fbInput;

		for(LogicAfbSignal input : appFb.afb().inputSignals())
		{
			if (fbParamNo == input.operandIndex())
			{
				fbInput = input;
				result = true;
				break;
			}
		}

		if (result == false)
		{
			// unknown FB input
			//
			assert(false);
			return false;
		}

		Command cmd;

		switch(fbInput.type())
		{
		case E::SignalType::Discrete:
			// input connected to discrete input
			//
			if (constItem.isDiscrete() == false)
			{
				// Uncompatible constant type (Logic schema %1).
				//
				m_log->errALC5028(constItem.guid(), appFb.schemaID());
				result = false;
			}
			else
			{
				int constValue = constItem.discreteValue();

				if (constValue == 1 || constValue == 0)
				{
					cmd.writeFuncBlockConst(fbType, fbInstance, fbParamNo, constValue, appFb.caption());
					cmd.setComment(QString(tr("%1 <= %2")).arg(inPin.caption()).arg(constValue));
				}
				else
				{
					// Constant connected to discrete signal or FB input must have value 0 or 1.
					//
					m_log->errALC5086(constItem.guid(), appFb.schemaID());
					return false;
				}
			}
			break;

		case E::SignalType::Analog:
			// const connected to analog input
			//
			switch(fbInput.size())
			{
			case SIZE_16BIT:
				if (constItem.isIntegral() == false)
				{
					// Uncompatible constant type (Logic schema %1).
					//
					m_log->errALC5028(constItem.guid(), appFb.schemaID());
					result = false;
				}
				else
				{
					cmd.writeFuncBlockConst(fbType, fbInstance, fbParamNo, constItem.intValue(), appFb.caption());
					cmd.setComment(QString(tr("%1 <= %2")).arg(inPin.caption()).arg(constItem.intValue()));
				}
				break;

			case SIZE_32BIT:
				switch(fbInput.dataFormat())
				{
				case E::DataFormat::SignedInt:
					if (constItem.isIntegral() == false)
					{
						// Uncompatible constant type (Logic schema %1).
						//
						m_log->errALC5028(constItem.guid(), appFb.schemaID());
						result = false;
					}
					else
					{
						cmd.writeFuncBlockConstInt32(fbType, fbInstance, fbParamNo, constItem.intValue(), appFb.caption());
						cmd.setComment(QString(tr("%1 <= %2")).arg(fbInput.caption()).arg(constItem.intValue()));
					}
					break;

				case E::DataFormat::Float:
					if (constItem.isFloat() == false)
					{
						// Uncompatible constant type (Logic schema %1).
						//
						m_log->errALC5028(constItem.guid(), appFb.schemaID());
						result = false;
					}
					else
					{
						cmd.writeFuncBlockConstFloat(fbType, fbInstance, fbParamNo, constItem.floatValue(), appFb.caption());
						cmd.setComment(QString(tr("%1 <= %2")).arg(fbInput.caption()).arg(constItem.floatValue()));
					}
					break;

				default:
					assert(false);
				}
			}

			break;

		default:
			assert(false);
		}

		if (cmd.isValidCommand())
		{
			m_code.append(cmd);
		}

		return result;
	}

	bool ModuleLogicCompiler::genearateWriteReceiverToFbCode(const AppFb& fb, const LogicPin& inPin, const LogicReceiver& receiver, const QUuid& receiverPinGuid)
	{
		std::shared_ptr<Hardware::Connection> connection = m_optoModuleStorage->getConnection(receiver.connectionId());

		if (connection == nullptr)
		{
			// Receiver is linked to unknown opto connection '%1'.
			//
			m_log->errALC5025(receiver.connectionId(), receiver.guid(), fb.schemaID());
			return false;
		}

		quint16 fbType = fb.opcode();
		quint16 fbInstance = fb.instance();
		quint16 fbParamNo = inPin.afbOperandIndex();

		LogicAfbSignal afbSignal;

		if (fb.getAfbSignalByIndex(fbParamNo, &afbSignal) == false)
		{
			return false;
		}

		Command cmd;

		if (receiver.isOutputPin(receiverPinGuid) == true)
		{
			SignalAddress16 rxAddress;

			if (m_optoModuleStorage->getRxSignalAbsAddress(fb.schemaID(),
														   receiver.connectionId(),
														   receiver.appSignalId(),
														   m_lm->equipmentIdTemplate(),
														   receiver.guid(),
														   rxAddress) == false)
			{
				return false;
			}

			Signal* srcSignal = m_signals->getSignal(receiver.appSignalId());

			if (srcSignal == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(receiver.appSignalId(), receiver.guid());
				return false;
			}

			if (connection->isSinglePort() == true)
			{
				// check serial InSignal parameters compatibility with srcSignal
				//
				if (srcSignal->isCompatibleFormat(rxAddress) == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString("Uncompatible format of serial IN_SIGNAL with aplication signal '%1'. (Connection '%2')").
										arg(srcSignal->appSignalID()).arg(connection->connectionID()));
					return false;
				}
			}

			if (checkSignalsCompatibility(*srcSignal, receiver.guid(), fb, afbSignal) == false)
			{
				return false;
			}

			QString str;

			str = QString(tr("%1 >> %2 => %3.%4")).arg(receiver.connectionId()).arg(receiver.appSignalId()).
													arg(fb.caption()).arg(afbSignal.caption());

			if (afbSignal.isAnalog())
			{
				cmd.writeFuncBlock32(fbType, fbInstance, fbParamNo, rxAddress.offset(), fb.caption());
			}
			else
			{
				if (afbSignal.isDiscrete())
				{
					cmd.writeFuncBlockBit(fbType, fbInstance, fbParamNo, rxAddress.offset(), rxAddress.bit(), fb.caption());
				}
				else
				{
					assert(false);		// unknown type of signal
					return false;
				}
			}

			cmd.setComment(str);
			m_code.append(cmd);

			return true;
		}

		if (receiver.isValidityPin(receiverPinGuid))
		{
			// Discrete signal '%1' is connected to analog input '%2.%3'.
			//
			if (afbSignal.isDiscrete() == false)
			{
				m_log->errALC5007(QString("%1 validity").arg(receiver.connectionId()),
								  fb.caption(),
								  afbSignal.caption(),
								  fb.guid());
				return false;
			}

			Address16 validityAddr;

			bool res = m_optoModuleStorage->getOptoPortValidityAbsAddr(m_lm->equipmentIdTemplate(),
																	   receiver.connectionId(),
																	   fb.schemaID(),
																	   receiver.guid(),
																	   validityAddr);
			if (res == false)
			{
				return false;
			}

			QString str;

			str = QString(tr("%1 >> validity => %2.%3")).
					arg(receiver.connectionId()).
					arg(fb.caption()).
					arg(afbSignal.caption());

			cmd.writeFuncBlockBit(fbType, fbInstance, fbParamNo, validityAddr.offset(), validityAddr.bit(), fb.caption());

			cmd.setComment(str);
			m_code.append(cmd);
			return true;
		}

		LOG_INTERNAL_ERROR(m_log);

		assert(false);		// unknown pin type

		return false;
	}

	bool ModuleLogicCompiler::generateWriteSignalToFbCode(const AppFb& appFb, const LogicPin& inPin, const AppSignal& appSignal)
	{
		quint16 fbType = appFb.opcode();
		quint16 fbInstance = appFb.instance();
		quint16 fbParamNo = inPin.afbOperandIndex();

		LogicAfbSignal afbSignal;

		if (appFb.getAfbSignalByIndex(fbParamNo, &afbSignal) == false)
		{
			return false;
		}

		if (afbSignal.isAnalog())
		{
			if (!appSignal.isAnalog())
			{
				m_log->errALC5007(appSignal.appSignalID(), appFb.caption(), afbSignal.caption(), appSignal.guid());
				return false;
			}

			if (appSignal.isCompatibleDataFormat(afbSignal) == false)
			{
				m_log->errALC5008(appSignal.appSignalID(), appFb.caption(), afbSignal.caption(), appSignal.guid(), appFb.schemaID());
				return false;
			}
		}
		else
		{
			if (afbSignal.isDiscrete())
			{
				if (!appSignal.isDiscrete())
				{
					m_log->errALC5010(appSignal.appSignalID(), appFb.caption(), afbSignal.caption(), appSignal.guid());
					return false;
				}
			}
			else
			{
				assert(false);		// unknown afb signal type
				return false;
			}
		}

		Command cmd;

		int ramAddrOffset = appSignal.ualAddr().offset();
		int ramAddrBit = appSignal.ualAddr().bit();

		if (ramAddrOffset == -1 || ramAddrBit == -1)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Address of signal '%1' is not calculated")).
					  arg(appSignal.appSignalID()));
			return false;
		}
		else
		{
			if (appSignal.isAnalog())
			{
				// input connected to analog signal
				//
				switch(appSignal.dataSize())
				{
				case SIZE_16BIT:
					cmd.writeFuncBlock(fbType, fbInstance, fbParamNo, ramAddrOffset, appFb.caption());
					break;

				case SIZE_32BIT:
					cmd.writeFuncBlock32(fbType, fbInstance, fbParamNo, ramAddrOffset, appFb.caption());
					break;

				default:
					assert(false);
					return false;
				}
			}
			else
			{
				// input connected to discrete signal
				//
				cmd.writeFuncBlockBit(fbType, fbInstance, fbParamNo, ramAddrOffset, ramAddrBit, appFb.caption());
			}

			cmd.setComment(QString(tr("%1 <= %2 (reg %3)")).
						   arg(inPin.caption()).arg(appSignal.appSignalID()).arg(appSignal.regBufAddr().toString()));

			m_code.append(cmd);
		}

		return true;
	}

	bool ModuleLogicCompiler::startFb(const AppFb* appFb)
	{
		int startCount = 1;

		for(LogicAfbParam param : appFb->afb().params())
		{
			if (param.opName() == "test_start_count")
			{
				startCount = param.value().toInt();
				break;
			}
		}
		Command cmd;

		if (startCount == 1)
		{
			cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.setComment(QString(tr("compute %1 @%2")).arg(appFb->caption()).arg(appFb->label()));
		}
		else
		{
			cmd.nstart(appFb->opcode(), appFb->instance(), startCount, appFb->caption(), appFb->runTime());
			cmd.setComment(QString(tr("compute %1 @%2 %3 times")).arg(appFb->afbStrID()).arg(appFb->label()).arg(startCount));
		}

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::readFbOutputSignals(const AppFb* appFb)
	{
		bool result = true;

		for(LogicPin outPin : appFb->outputs())
		{
			if (outPin.dirrection() != VFrame30::ConnectionDirrection::Output)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			bool connectedToSignal = false;
			bool connectedToTerminator = false;
			bool connectedToFb = false;

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

				AppItem::Type t = connectedPinParent->type();

				switch(t)
				{
				case AppItem::Type::Fb:
					connectedToFb = true;
					break;

				case AppItem::Type::Transmitter:
					break;

				case AppItem::Type::Terminator:
					connectedToTerminator = true;
					break;

				case AppItem::Type::Signal:
					{
						QUuid signalGuid;

						// output connected to real signal
						//
						connectedToSignal = true;

						signalGuid = connectedPinParent->guid();

						result &= generateReadFuncBlockToSignalCode(*appFb, outPin, signalGuid);
						break;
					}

				default:
					{
						std::shared_ptr<VFrame30::FblItemRect> item = connectedPinParent->itemRect();
						qDebug() << item->metaObject()->className();
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
				}

				if (result == false)
				{
					break;
				}
			}

			if (connectedToSignal == false && connectedToTerminator == false)
			{
				// output pin is not connected to any signal or terminator
				//
				// may be it directly connected to FB?
				//
				if (connectedToFb == true)
				{
					// yes, save FB output value to shadow signal with GUID == outPin.guid()
					//
					result &= generateReadFuncBlockToSignalCode(*appFb, outPin, outPin.guid());
				}
				else
				{
					// output pin is not connected?
					//
					LOG_INTERNAL_ERROR(m_log);
					result = false;
				}
			}

			if (result == false)
			{
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::generateReadFuncBlockToSignalCode(const AppFb& appFb, const LogicPin& outPin, const QUuid& signalGuid)
	{
		if (!m_appSignals.contains(signalGuid))
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()));
			return false;
		}

		AppSignal* appSignal = m_appSignals[signalGuid];

		if (appSignal == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		if (appSignal->enableTuning() == true)
		{
			// Can't assign value to tuningable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5071(appSignal->schemaID(), appSignal->appSignalID(), appSignal->guid());
			return false;
		}

		if (appSignal->isInput() == true)
		{
			// Can't assign value to input signal '%1' (Logic schema '%2').
			//
			m_log->errALC5087(appSignal->schemaID(), appSignal->appSignalID(), appSignal->guid());
			return false;
		}

		quint16 fbType = appFb.opcode();
		quint16 fbInstance = appFb.instance();
		quint16 fbParamNo = outPin.afbOperandIndex();

		LogicAfbSignal afbSignal;

		if (appFb.getAfbSignalByIndex(fbParamNo, &afbSignal) == false)
		{
			return false;
		}

		if (afbSignal.isAnalog())
		{
			if (!appSignal->isAnalog())
			{
				m_log->errALC5003(appFb.caption(), afbSignal.caption(), appSignal->appSignalID(), appSignal->guid());
				return false;
			}

			if (appSignal->isCompatibleDataFormat(afbSignal) == false)
			{
				m_log->errALC5004(appFb.caption(), afbSignal.caption(), appSignal->appSignalID(), appSignal->guid());
				return false;
			}
		}
		else
		{
			if (afbSignal.isDiscrete())
			{
				if (!appSignal->isDiscrete())
				{
					m_log->errALC5006(appFb.caption(), afbSignal.caption(), appSignal->appSignalID(), appSignal->guid());
					return false;
				}
			}
			else
			{
				assert(false);		// unknown afb signal type
				return false;
			}
		}

		Command cmd;

		int ualAddrOffset = appSignal->ualAddr().offset();
		int ualAddrBit = appSignal->ualAddr().bit();

		if (ualAddrOffset == -1 || ualAddrBit == -1)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("UAL address of signal %1 is not calculated")).arg(appSignal->appSignalID()));
			return false;
		}
		else
		{
			if (appSignal->isAnalog() == true)
			{
				// output connected to analog signal
				//
				switch(appSignal->analogSignalFormat())
				{
				case E::AnalogAppSignalFormat::Float32:
				case E::AnalogAppSignalFormat::SignedInt32:
					cmd.readFuncBlock32(ualAddrOffset, fbType, fbInstance, fbParamNo, appFb.caption());
					break;

				default:
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							  QString(tr("Signal %1 has unknown analog signal format")).
									   arg(appSignal->appSignalID()));
					return false;
				}
			}
			else
			{
				// output connected to discrete signal
				//
				cmd.readFuncBlockBit(ualAddrOffset, ualAddrBit, fbType, fbInstance, fbParamNo, appFb.caption());
			}

			cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
						   arg(appSignal->appSignalID()).arg(appSignal->regBufAddr().toString()).arg(outPin.caption()));

			m_code.append(cmd);
		}

		appSignal->setResultSaved();

		return true;
	}

	bool ModuleLogicCompiler::addToComparatorStorage(const AppFb* appFb)
	{
		if (appFb == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (appFb->isComparator() == false)
		{
			return true;
		}

		std::shared_ptr<Comparator> cmp = std::make_shared<Comparator>();

		bool result = initComparator(cmp, appFb);

		if (result == true)
		{
			m_cmpStorage->insert(m_lm->equipmentIdTemplate(), cmp);
		}

		return result;
	}

	bool ModuleLogicCompiler::initComparator(std::shared_ptr<Comparator> cmp, const AppFb* appFb)
	{
		Q_UNUSED(cmp);

		bool result = true;

		if (appFb->isConstComaparator() == true)
		{

		}

		///////

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredTuningAnalogSignalsToRegBuf()
	{
		if (m_acquiredAnalogTuningSignals.isEmpty() == true)
		{
			return true;
		}

		Comment cmnt;

		cmnt.setComment(QString(tr("Copy acquired tuningable analog signals to regBuf")));

		m_code.append(cmnt);
		m_code.newLine();

		int startUalAddr = -1;
		int startRegBufAddr = -1;

		int prevUalAddr = -1;
		int prevRegBufAddr = -1;
		int prevSignalSizeW = -1;

		Command cmd;

		QString commentStr;

		for(Signal* s : m_acquiredAnalogTuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			// check signal!
			//
			switch(s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::SignedInt32:
			case E::AnalogAppSignalFormat::Float32:

				assert(s->dataSize() == SIZE_32BIT);
				assert(s->ualAddr().isValid() == true);
				assert(s->regBufAddr().isValid() == true);
				break;

			default:
				assert(false);
			}

			if (prevSignalSizeW == -1)
			{
				// is first signal, init variables
				//
				startUalAddr = prevUalAddr = s->ualAddr().offset();
				startRegBufAddr = prevRegBufAddr = s->regBufAddr().offset();

				prevSignalSizeW = s->dataSize() / SIZE_16BIT;

				commentStr = "copy: " + s->appSignalID();
			}
			else
			{
				if (s->ualAddr().offset() == (prevUalAddr + prevSignalSizeW) &&
					s->regBufAddr().offset() == (prevRegBufAddr + prevSignalSizeW))
				{
					// address is plain
					// reassing variables and continue
					//
					prevUalAddr = s->ualAddr().offset();
					prevRegBufAddr = s->regBufAddr().offset();

					prevSignalSizeW = s->dataSize() / SIZE_16BIT;

					commentStr += " " + s->appSignalID();
				}
				else
				{
					// not plain address!
					//
					// generate command to copy previous signals
					//
					cmd.movMem(startRegBufAddr, startUalAddr, prevRegBufAddr - startRegBufAddr + prevSignalSizeW);
					cmd.setComment(commentStr);
					m_code.append(cmd);

					// init variables for the next block
					//
					startUalAddr = prevUalAddr = s->ualAddr().offset();
					startRegBufAddr = prevRegBufAddr = s->regBufAddr().offset();

					prevSignalSizeW = s->dataSize() / SIZE_16BIT;

					commentStr = "copy: " + s->appSignalID();
				}
			}
		}

		assert(startUalAddr != -1);
		assert(startRegBufAddr != -1);
		assert(prevUalAddr != -1);
		assert(prevRegBufAddr != -1);
		assert(prevSignalSizeW != -1);

		// generate command to copy rest of signals
		//
		cmd.movMem(startRegBufAddr, startUalAddr, prevRegBufAddr - startRegBufAddr + prevSignalSizeW);
		cmd.setComment(commentStr);
		m_code.append(cmd);

		m_code.newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredTuningDiscreteSignalsToRegBuf()
	{
		if (m_acquiredDiscreteTuningSignals.isEmpty() == true)
		{
			return true;
		}

		Comment cmnt;

		cmnt.setComment(QString(tr("Copy acquired tuningable discrete signals to regBuf")));

		m_code.append(cmnt);
		m_code.newLine();

		int startUalAddr = -1;
		int startRegBufAddr = -1;

		int prevUalAddr = -1;
		int prevRegBufAddr = -1;

		Command cmd;

		QString commentStr;

		for(Signal* s : m_acquiredDiscreteTuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			// check signal!

			assert(s->ualAddr().isValid() == true);
			assert(s->regBufAddr().isValid() == true);
			assert(s->dataSize() == SIZE_1BIT);
			assert(s->ualAddr().bit() == s->regBufAddr().bit());

			if (startUalAddr == -1)
			{
				// is first signal, init variables
				//
				startUalAddr = prevUalAddr = s->ualAddr().bitAddress();
				startRegBufAddr = prevRegBufAddr = s->regBufAddr().bitAddress();

				commentStr = "copy: " + s->appSignalID();
			}
			else
			{
				if (s->ualAddr().bitAddress() == (prevUalAddr + SIZE_1BIT) &&
					s->regBufAddr().bitAddress() == (prevRegBufAddr + SIZE_1BIT))
				{
					// address is plain
					// reassing variables and continue
					//
					prevUalAddr = s->ualAddr().bitAddress();
					prevRegBufAddr = s->regBufAddr().bitAddress();

					commentStr += " " + s->appSignalID();
				}
				else
				{
					// not plain address!
					//
					// generate command to copy previous signals
					//
					assert((startRegBufAddr % SIZE_16BIT) == 0);
					assert((startUalAddr % SIZE_16BIT) == 0);

					int copySizeBit = prevRegBufAddr - startRegBufAddr + SIZE_1BIT;
					int copySizeW = (copySizeBit / SIZE_16BIT) + ((copySizeBit % SIZE_16BIT) == 0 ? 0 : 1);

					if (copySizeW > 1)
					{
						cmd.movMem(startRegBufAddr / SIZE_16BIT,
								   startUalAddr / SIZE_16BIT,
								   copySizeW);
					}
					else
					{
						if (copySizeW == 1)
						{
							cmd.mov(startRegBufAddr / SIZE_16BIT,
									   startUalAddr / SIZE_16BIT);
						}
						else
						{
							assert(false);
						}
					}

					cmd.setComment(commentStr);
					m_code.append(cmd);

					// init variables for the next block
					//
					startUalAddr = prevUalAddr = s->ualAddr().bitAddress();
					startRegBufAddr = prevRegBufAddr = s->regBufAddr().bitAddress();

					commentStr = "copy: " + s->appSignalID();
				}
			}
		}

		assert(startUalAddr != -1);
		assert(startRegBufAddr != -1);
		assert(prevUalAddr != -1);
		assert(prevRegBufAddr != -1);

		assert((startRegBufAddr % SIZE_16BIT) == 0);
		assert((startUalAddr % SIZE_16BIT) == 0);

		// generate command to copy rest of signals
		//
		int copySizeBit = prevRegBufAddr - startRegBufAddr + SIZE_1BIT;
		int copySizeW = (copySizeBit / SIZE_16BIT) + ((copySizeBit % SIZE_16BIT) == 0 ? 0 : 1);


		if (copySizeW > 1)
		{
			cmd.movMem(startRegBufAddr / SIZE_16BIT,
					   startUalAddr / SIZE_16BIT,
					   copySizeW);
		}
		else
		{
			if (copySizeW == 1)
			{
				cmd.mov(startRegBufAddr / SIZE_16BIT,
						   startUalAddr / SIZE_16BIT);
			}
			else
			{
				assert(false);
			}
		}

		cmd.setComment(commentStr);
		m_code.append(cmd);

		m_code.newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteInputSignalsToRegBuf()
	{
		if (m_acquiredDiscreteInputSignals.isEmpty() == true)
		{
			return true;
		}

		Comment comment;

		comment.setComment("Copy acquired discrete input signals in regBuf");
		m_code.append(comment);
		m_code.newLine();

		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		int signalsCount = m_acquiredDiscreteInputSignals.count();

		int count = 0;

		Command cmd;

		int countReminder16 = 0;

		for(Signal* s : m_acquiredDiscreteInputSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				return false;
			}

			countReminder16 = count % SIZE_16BIT;

			assert(s->regBufAddr().bit() == countReminder16);

			cmd.movBit(bitAccAddr, s->regBufAddr().bit(), s->ioBufAddr().offset(), s->ioBufAddr().bit());
			cmd.setComment(QString("copy %1").arg(s->appSignalID()));
			m_code.append(cmd);

			count++;

			if ((count % SIZE_16BIT) == 0 || count == signalsCount)
			{
				cmd.clearComment();

				for(int i = countReminder16 + 1; i < SIZE_16BIT; i++)
				{
					cmd.movBitConst(bitAccAddr, i, 0);
					m_code.append(cmd);
				}

				cmd.mov(s->regBufAddr().offset(), bitAccAddr);
				m_code.append(cmd);
				m_code.newLine();;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf()
	{
		if (m_acquiredDiscreteOutputSignals.isEmpty() == false)
		{
			assert(m_memoryMap.acquiredDiscreteOutputSignalsSizeW() ==
				   m_memoryMap.acquiredDiscreteOutputSignalsInRegBufSizeW());

			m_code.comment("Copy acquired discrete output signals from bit-addressed memory to regBuf");
			m_code.newLine();

			Command cmd;

			cmd.movMem(m_memoryMap.aquiredDiscreteOutputSignalsAddressInRegBuf(),
					   m_memoryMap.acquiredDiscreteOutputSignalsAddress(),
					   m_memoryMap.acquiredDiscreteOutputSignalsSizeW());

			m_code.append(cmd);
			m_code.newLine();
		}

		if (m_acquiredDiscreteInternalSignals.isEmpty() == false)
		{
			assert(m_memoryMap.acquiredDiscreteInternalSignalsSizeW() ==
				   m_memoryMap.acquiredDiscreteInternalSignalsInRegBufSizeW());

			m_code.comment("Copy acquired discrete internal signals from bit-addressed memory to regBuf");
			m_code.newLine();

			Command cmd;

			cmd.movMem(m_memoryMap.aquiredDiscreteInternalSignalsAddressInRegBuf(),
					   m_memoryMap.acquiredDiscreteInternalSignalsAddress(),
					   m_memoryMap.acquiredDiscreteInternalSignalsSizeW());

			m_code.append(cmd);
			m_code.newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOutputSignalsInOutputModulesMemory()
	{
		bool result = true;

//		result &= initOutputModulesMemory();
		result &= conevrtOutputAnalogSignals();
		result &= copyOutputDiscreteSignals();

		return result;
	}

	bool ModuleLogicCompiler::initOutputModulesMemory()
	{
		// init LM's outputs memory area
		//
		Hardware::DeviceController* lmOutsController = DeviceHelper::getChildControllerBySuffix(m_lm, "_CTRLOUT", m_log);

		if (lmOutsController == nullptr)
		{
			return false;
		}

		Hardware::DeviceObject* lmOut1Object = DeviceHelper::getChildDeviceObjectBySuffix(lmOutsController, "_OUT01", m_log);
		Hardware::DeviceObject* lmOut6Object = DeviceHelper::getChildDeviceObjectBySuffix(lmOutsController, "_OUT06", m_log);

		if (lmOut1Object == nullptr || lmOut6Object == nullptr)
		{
			return false;
		}

		Hardware::DeviceSignal* lmOut1 = lmOut1Object->toSignal();
		Hardware::DeviceSignal* lmOut6 = lmOut6Object->toSignal();

		if (lmOut1 == nullptr || lmOut6 == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (lmOut1->valueOffset() != lmOut6->valueOffset())
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		int lmOutsMemoryOffset = m_lmDescription->memory().m_appDataOffset + lmOut1->valueOffset();

		m_code.comment("Init LM's output signals memory");
		m_code.newLine();

		Command cmd;

		cmd.movConst(lmOutsMemoryOffset, 0);
		m_code.append(cmd);

		m_code.newLine();

		// init output modules memory
		//
		bool firstModule = true;

		for(Module& module : m_modules)
		{
			if (module.isOutputModule() == false)
			{
				continue;
			}

			if (firstModule == true)
			{
				m_code.comment("Init output modules memory");
				m_code.newLine();

				firstModule = false;
			}

			cmd.setMem(module.moduleDataOffset, 0, module.rxDataSize);
			cmd.setComment(QString("place %1 module %2").arg(module.place).arg(getModuleFamilyTypeStr(module.familyType())));
			m_code.append(cmd);
		}

		if (firstModule == false)
		{
			m_code.newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::conevrtOutputAnalogSignals()
	{
		QVector<Signal*> outAnalogSignals;

		outAnalogSignals.append(m_acquiredAnalogOutputSignals);
		outAnalogSignals.append(m_nonAcquiredAnalogOutputSignals);

		if (outAnalogSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.comment("Convertion of output analog signals");
		m_code.newLine();

		Command cmd;

		for(Signal* s : outAnalogSignals)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->isAnalog() == true);
			assert(s->isOutput() == true);
			assert(s->dataSize() == SIZE_32BIT);
			assert(s->ualAddr().isValid() == true);
			assert(s->ioBufAddr().isValid() == true);

			if (s->needConversion() == false)
			{
				// signal isn't need conversion
				// copy signal only
				//
				cmd.mov32(s->ioBufAddr().offset(), s->ualAddr().offset());
				cmd.setComment(QString("copy analog output %1").arg(s->appSignalID()));
				m_code.append(cmd);

				continue;
			}

			AppFb* appFb = m_inOutSignalsToScalAppFbMap.value(s->appSignalID(), nullptr);

			TEST_PTR_CONTINUE(appFb);

			FbScal fbScal;

			switch(s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:

				fbScal = m_fbScal[FB_SCALE_FP_16UI_INDEX];
				break;

			case E::AnalogAppSignalFormat::SignedInt32:

				fbScal = m_fbScal[FB_SCALE_SI_16UI_INDEX];
				break;

			default:
				assert(false);
			}

			cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
							   s->ualAddr().offset(), appFb->caption());
			cmd.setComment(QString(tr("conversion of analog output %1")).arg(s->appSignalID()));
			m_code.append(cmd);

			cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.clearComment();
			m_code.append(cmd);

			cmd.readFuncBlock(s->ioBufAddr().offset(),
							  appFb->opcode(), appFb->instance(),
							  fbScal.outputSignalIndex, appFb->caption());
			m_code.append(cmd);

			m_code.newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOutputDiscreteSignals()
	{
		QVector<Signal*> outDiscreteSignals;

		outDiscreteSignals.append(m_acquiredDiscreteOutputSignals);
		outDiscreteSignals.append(m_nonAcquiredDiscreteOutputSignals);

//		if (outDiscreteSignals.isEmpty() == true)
//		{
//			return true;
//		}

		int lmOutputsAddress = m_lmDescription->memory().m_appDataOffset;
		bool lmOutputsIsWritten = false;

		m_code.comment("Copy output discrete signals to output modules memory");
		m_code.newLine();

		QHash<int, Signal*> writeAddressesMap;

		for(Signal* s : outDiscreteSignals)
		{
			TEST_PTR_CONTINUE(s);

			writeAddressesMap.insertMulti(s->ioBufAddr().offset(), s);
		}

		QList<int> writeAddreses = writeAddressesMap.uniqueKeys();

		QVector<int> sortedWriteAddress = QVector<int>::fromList(writeAddreses);

		qSort(sortedWriteAddress);

		int wordAccAddr = m_memoryMap.wordAccumulatorAddress();

		for(int writeAddr : sortedWriteAddress)
		{
			QList<Signal*> writeSignals = writeAddressesMap.values(writeAddr);

			Command cmd;

			cmd.movConst(wordAccAddr, 0);
			m_code.append(cmd);

			for(Signal* s : writeSignals)
			{
				TEST_PTR_CONTINUE(s);

				cmd.movBit(wordAccAddr, s->ioBufAddr().bit(), s->ualAddr().offset(), s->ualAddr().bit());
				cmd.setComment(s->appSignalID());
				m_code.append(cmd);
			}

			cmd.mov(writeAddr, wordAccAddr);
			cmd.clearComment();
			m_code.append(cmd);
			m_code.newLine();

			if (writeAddr == lmOutputsAddress)
			{
				lmOutputsIsWritten = true;
			}
		}

		if (lmOutputsIsWritten == false)
		{
			Command cmd;

			cmd.movConst(lmOutputsAddress, 0);
			cmd.setComment("write #0 to LM's outputs area");
			m_code.append(cmd);
			m_code.newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOptoConnectionsTxData()
	{
		bool result = true;

		QVector<Hardware::OptoModuleShared> modules;

		m_optoModuleStorage->getOptoModulesSorted(modules);

		if (modules.count() == 0)
		{
			return true;
		}

		for(Hardware::OptoModuleShared& module : modules)
		{
			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (module->lmID() != m_lm->equipmentIdTemplate())
			{
				continue;
			}

			bool initialCommentPrinted = false;

			const HashedVector<QString, Hardware::OptoPortShared>& ports = module->ports();

			for(const Hardware::OptoPortShared& port : ports)
			{
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				if (port->txDataSizeW() == 0)
				{
					continue;
				}

				if (initialCommentPrinted == false)
				{
					Comment comment;

					comment.setComment(QString(tr("Copying txData of opto-module %1")).arg(module->equipmentID()));
					m_code.append(comment);
					m_code.newLine();

					initialCommentPrinted = true;
				}

				result &= copyOptoPortTxData(port);
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::copyOptoPortTxData(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Comment comment;

		if (port->txDataSizeW() == 0)
		{
			comment.setComment(QString(tr("No txData of opto-port %1")).arg(port->equipmentID()));
			m_code.append(comment);
			m_code.newLine();
			return true;
		}

		bool result = true;

		Command cmd;

		comment.setComment(QString(tr("Copying txData of opto-port %1 (%2 words)")).
						   arg(port->equipmentID()).arg(port->txDataSizeW()));
		m_code.append(comment);
		m_code.newLine();

		// write data port txData identifier
		//
		cmd.movConstUInt32(port->txBufAbsAddress(), port->txDataID());
		cmd.setComment("txData ID");

		m_code.append(cmd);
		m_code.newLine();

		result &= copyOptoPortTxRawData(port);

		result &= copyOptoPortTxAnalogSignals(port);

		result &= copyOptoPortTxDiscreteSignals(port);

		// rest of manually configured buffer fills by 0
		//
		if (port->manualSettings() == true && port->txUsedDataSizeW() < port->txDataSizeW())
		{
			int fillSize = port->txDataSizeW() - port->txUsedDataSizeW();

			cmd.setMem(port->txBufAbsAddress() + port->txUsedDataSizeW(),
					   0, fillSize);
			cmd.setComment("rest of manually configured buffer fills by 0");

			m_code.append(cmd);
			m_code.newLine();
		}

		return result;
	}


	bool ModuleLogicCompiler::copyOptoPortTxRawData(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		if (port->txRawDataSizeW() == 0)
		{
			return true;
		}

		bool result = true;

		Comment comment;

		comment.setComment(QString(tr("Copying raw data (%1 words) of opto-port %2")).arg(port->txRawDataSizeW()).arg(port->equipmentID()));
		m_code.append(comment);
		m_code.newLine();

		int offset = Hardware::OptoPort::TX_DATA_ID_SIZE_W;		// txDataID

		Command cmd;

		// qDebug() << "Fill port " << C_STR(port->equipmentID()) << " raw data";

		int txRawDataSizeW = port->txRawDataSizeW();

		cmd.setMem(port->txBufAbsAddress() + offset, 0, txRawDataSizeW);
		cmd.setComment("initialize tx raw data memory");

		m_code.append(cmd);
		m_code.newLine();

		const Hardware::RawDataDescription& rawDataDescription = port->rawDataDescription();

		for(const Hardware::RawDataDescriptionItem& item : rawDataDescription)
		{
			switch(item.type)
			{
			case Hardware::RawDataDescriptionItem::Type::TxRawDataSize:
				// no code generation required
				//
				break;

			case Hardware::RawDataDescriptionItem::Type::TxAllModulesRawData:
				result &= copyOptoPortAllNativeRawData(port, offset);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxModuleRawData:
				result &= copyOptoPortTxModuleRawData(port, offset, item.modulePlace);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxPortRawData:
				result &= copyOptoPortTxOptoPortRawData(port, offset, item.portEquipmentID);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxConst16:
				result &= copyOptoPortTxConst16RawData(port, item.const16Value, offset);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxSignal:
				// code generate later
				break;

			case Hardware::RawDataDescriptionItem::Type::RxRawDataSize:
			case Hardware::RawDataDescriptionItem::Type::RxSignal:
				// no code generation required
				//
				break;

			default:
				assert(false);
			}
		}

		result &= copyOptoPortRawTxAnalogSignals(port);

		result &= copyOptoPortRawTxDiscreteSignals(port);

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxAnalogSignals(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		const HashedVector<QString, Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		bool result = true;

		bool first = true;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == true || txSignal->isAnalog() == false)
			{
				// skip raw and non-analog signals
				//
				continue;
			}

			Signal* s = m_signals->getSignal(txSignal->appSignalID());

			if (s == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(txSignal->appSignalID(), QUuid());
				result = false;
				continue;
			}

			if (first == true)
			{
				Comment comment;

				comment.setComment(QString("Copying regular tx analog signals of opto-port %1").arg(port->equipmentID()));

				m_code.append(comment);
				m_code.newLine();

				first = false;
			}

			Command cmd;

			int txSignalAddress = port->txBufAbsAddress() + txSignal->addrInBuf().offset();

			cmd.mov32(txSignalAddress, s->ualAddr().offset());
			cmd.setComment(QString("%1 >> %2").arg(s->appSignalID()).arg(port->connectionID()));

			m_code.append(cmd);
		}

		if (first == false)
		{
			m_code.newLine();
		}

		return result;
	}


	bool ModuleLogicCompiler::copyOptoPortTxDiscreteSignals(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		// copy discrete signals
		//
		QVector<Hardware::TxRxSignalShared> txDiscreteSignals;

		port->getTxDiscreteSignals(txDiscreteSignals, true);

		int count = txDiscreteSignals.count();

		int wordCount = count / WORD_SIZE + (count % WORD_SIZE ? 1 : 0);

		int bitAccumulatorAddress = m_memoryMap.bitAccumulatorAddress();

		bool result = true;

		Command cmd;

		int bitCount = 0;

		bool first = true;

		for(int i = 0; i < count; i++)
		{
			Hardware::TxRxSignalShared& txSignal = txDiscreteSignals[i];

			if (txSignal->isRaw() == true)
			{
				assert(false);				// not must be here!
				continue;					// raw signals copying in raw data section code generation
			}

			Signal* s = m_signals->getSignal(txSignal->appSignalID());

			if (s == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(txSignal->appSignalID(), QUuid());
				result = false;
				continue;
			}

			if (first == true)
			{
				Comment comment;

				comment.setComment(QString("Copying regular tx discrete signals of opto-port %1").arg(port->equipmentID()));

				m_code.append(comment);
				m_code.newLine();

				first = false;
			}

			if ((bitCount % WORD_SIZE) == 0)
			{
				// this is new word!
				//
				if ((i / WORD_SIZE) == (wordCount - 1) &&			// if this is last word and
					(count % WORD_SIZE) != 0 )						// signals count is not multiple WORD_SIZE
				{
					// generate bit-accumulator cleaning command
					//
					cmd.movConst(bitAccumulatorAddress, 0);
					cmd.setComment(QString("bit accumulator cleaning"));

					m_code.append(cmd);
				}
			}

			int bit = bitCount % WORD_SIZE;

			assert(txSignal->addrInBuf().bit() == bit);

			// copy discrete signal value to bit accumulator
			//
			cmd.movBit(bitAccumulatorAddress, bit, s->ualAddr().offset(), s->ualAddr().bit());
			cmd.setComment(QString("%1 >> %2").arg(s->appSignalID()).arg(port->connectionID()));

			m_code.append(cmd);

			if ((bitCount % WORD_SIZE) == (WORD_SIZE -1) ||			// if this is last bit in word or
				i == count -1)									// this is even the last bit
			{
				// txSignal.address.offset() the same for all signals in one word

				int txSignalAddress = port->txBufAbsAddress() + txSignal->addrInBuf().offset();

				// copy bit accumulator to opto interface buffer
				//
				cmd.mov(txSignalAddress, bitAccumulatorAddress);
				cmd.clearComment();

				m_code.append(cmd);
			}

			bitCount++;
		}

		if (first == false)
		{
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortAllNativeRawData(Hardware::OptoPortShared port, int& offset)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = true;

		for(int place = FIRST_MODULE_PLACE; place <= LAST_MODULE_PLACE; place++)
		{
			const Hardware::DeviceModule* module = DeviceHelper::getModuleOnPlace(m_lm, place);

			if (module == nullptr)
			{
				continue;
			}

			result &= copyOptoPortTxModuleRawData(port, offset, module);
		}

		return result;
	}


	bool ModuleLogicCompiler::copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, int place)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		const Hardware::DeviceModule* module = DeviceHelper::getModuleOnPlace(m_lm, place);

		if (module == nullptr)
		{
			QString msg = QString("OptoPort %1 raw data copying, not found module on place %2.").
					arg(port->equipmentID()).arg(place);
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return false;
		}

		return copyOptoPortTxModuleRawData(port, offset, module);
	}


	bool ModuleLogicCompiler::copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, const Hardware::DeviceModule* module)
	{
		if (port == nullptr || module == nullptr)
		{
			assert(false);
			return false;
		}

		int moduleRawDataSize = DeviceHelper::getModuleRawDataSize(module, m_log);

		if (moduleRawDataSize == 0)
		{
			return true;
		}

		ModuleRawDataDescription* desc = DeviceHelper::getModuleRawDataDescription(module);

		if (desc == nullptr)
		{
			return true;
		}

		const QVector<ModuleRawDataDescription::Item>& items = desc->items();

		bool result = true;

		int moduleAppDataOffset = 0;
		int moduleDiagDataOffset = 0;

		result &= DeviceHelper::getIntProperty(module, "TxAppDataOffset", &moduleAppDataOffset, m_log);
		result &= DeviceHelper::getIntProperty(module, "TxDiagDataOffset", &moduleDiagDataOffset, m_log);

		if (result == false)
		{
			return false;
		}

		int localOffset = 0;
		int toAddr = 0;
		int fromAddr = 0;

		bool autoSize = false;
		bool firstCommand = true;

		for(const ModuleRawDataDescription::Item& item : items)
		{
			Command cmd;

			toAddr = port->txBufAbsAddress() + offset + localOffset;

			fromAddr = m_memoryMap.getModuleDataOffset(module->place());

			switch(item.type)
			{
			case ModuleRawDataDescription::ItemType::RawDataSize:
				autoSize = item.rawDataSizeIsAuto;
				break;

			case ModuleRawDataDescription::ItemType::AppData16:

				fromAddr += moduleAppDataOffset + item.offset;

				cmd.mov(toAddr, fromAddr);

				localOffset++;

				break;

			case ModuleRawDataDescription::ItemType::DiagData16:

				fromAddr += moduleDiagDataOffset + item.offset;

				cmd.mov(toAddr, fromAddr);

				localOffset++;

				break;


			case ModuleRawDataDescription::ItemType::AppData32:

				fromAddr += moduleAppDataOffset + item.offset;

				cmd.mov32(toAddr, fromAddr);

				localOffset += 2;

				break;

			case ModuleRawDataDescription::ItemType::DiagData32:

				fromAddr += moduleDiagDataOffset + item.offset;

				cmd.mov32(toAddr, fromAddr);

				localOffset += 2;

				break;

			default:
				assert(false);
			}

			if (cmd.isNoCommand() == false)
			{
				if (firstCommand == true)
				{
					cmd.setComment(QString("copying module %1 raw data, place %2").arg(module->equipmentIdTemplate()).arg(module->place()));
					firstCommand = false;
				}

				m_code.append(cmd);
			}
		}

		m_code.newLine();

		if (autoSize == true)
		{
			assert(localOffset == moduleRawDataSize);
		}

		offset += moduleRawDataSize;

		return true;
	}


	bool ModuleLogicCompiler::copyOptoPortTxOptoPortRawData(Hardware::OptoPortShared port, int& offset, const QString& portEquipmentID)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		// get opto port received raw data
		//
		Hardware::OptoPortShared portWithRxRawData = m_optoModuleStorage->getOptoPort(portEquipmentID);

		if (portWithRxRawData == nullptr)
		{
			QString msg = QString("OptoPort %1 is not found (opto port %2 raw data settings).").
					arg(portEquipmentID).arg(port->equipmentID());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return false;
		}

		// get opto port linkerd to portWithRxRawData, that send raw data
		//
		Hardware::OptoPortShared portWithTxRawData = m_optoModuleStorage->getOptoPort(portWithRxRawData->linkedPortID());

		if (portWithTxRawData == nullptr)
		{
			QString msg = QString("OptoPort %1 linked to %2 is not found.").
					arg(portWithRxRawData->linkedPortID()).arg(portWithRxRawData->equipmentID());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return false;
		}

		if (portWithTxRawData->hasTxRawData() == false)
		{
			QString msg = QString("OptoPort %1 has't raw data description. Nothing to copy.").
					arg(portWithTxRawData->equipmentID());
			LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return true;
		}

		int portTxRawDataSizeW = portWithTxRawData->txRawDataSizeW();

		if (portTxRawDataSizeW == 0)
		{
			QString msg = QString("OptoPort %1 raw data size is 0. Nothing to copy.").
					arg(portWithTxRawData->equipmentID());
			LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return true;
		}

		Command cmd;

		cmd.movMem(port->txBufAbsAddress() + offset,
				   portWithRxRawData->rxBufAbsAddress() + Hardware::OptoPort::TX_DATA_ID_SIZE_W,
				   portTxRawDataSizeW);

		cmd.setComment(QString("copying raw data received on port %1").arg(portWithRxRawData->equipmentID()));

		m_code.append(cmd);
		m_code.newLine();

		offset += portTxRawDataSizeW;

		return true;
	}


	bool ModuleLogicCompiler::copyOptoPortTxConst16RawData(Hardware::OptoPortShared port, int const16value, int& offset)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		Command cmd;

		cmd.movConst(port->txBufAbsAddress() + offset, const16value);

		cmd.setComment(QString("copying raw data const16 value = %1").arg(const16value));

		m_code.append(cmd);
		m_code.newLine();

		offset++;

		return true;

	}

	bool ModuleLogicCompiler::copyOptoPortRawTxAnalogSignals(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		const HashedVector<QString, Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		bool result = true;

		Command cmd;

		int count = 0;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == false || txSignal->isAnalog() == false)
			{
				// skip non-Raw and non-Analog signals
				//
				continue;
			}

			Signal* s = m_signals->getSignal(txSignal->appSignalID());

			if (s == nullptr)
			{
				// Signal '%1' is not found (opto port '%2' raw data description).
				//
				m_log->errALC5186(txSignal->appSignalID(), port->equipmentID());
				result = false;
				continue;
			}

			if (s->isCompatibleFormat(txSignal->signalType(), txSignal->dataFormat(), txSignal->dataSize(), txSignal->byteOrder()) == false)
			{
				LOG_ERROR_OBSOLETE(m_log,
								   Builder::IssueType::NotDefined,
								   QString(tr("Format of signal '%1' isn't compatible with format in port '%2' raw data description (Connection '%3')")).
								   arg(txSignal->appSignalID()).
								   arg(port->equipmentID()).
								   arg(port->connectionID()));

				return false;
			}

			cmd.mov32(port->txBufAbsAddress() + txSignal->addrInBuf().offset(), s->ualAddr().offset());

			cmd.setComment(QString("%1 >> %2").arg(txSignal->appSignalID()).arg(port->connectionID()));

			m_code.append(cmd);

			count++;
		}

		if (count > 0)
		{
			m_code.newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOptoPortRawTxDiscreteSignals(Hardware::OptoPortShared port)
	{
		const HashedVector<QString, Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		int count = 0;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == false || txSignal->isDiscrete() == false)
			{
				// skip non-Raw and non-Discrete signals
				//
				continue;
			}

			count++;
		}

		if (count > 0)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("Raw tx discrete signals isn't implement now (port %1).  ")).
							   arg(port->equipmentID()));
			return false;
		}

		return true;
	}

	bool ModuleLogicCompiler::finishAppLogicCode()
	{
		m_code.comment("End of application logic code");
		m_code.newLine();

		Command cmd;

		cmd.stop();

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::setLmAppLANDataSize()
	{
		if (m_lm == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		return DeviceHelper::setIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											"AppLANDataSize",
											m_memoryMap.regBufSizeW(),
											m_log);
	}

	bool ModuleLogicCompiler::calculateCodeRunTime()
	{
		bool result = m_code.getRunTimes(m_idrPhaseClockCount, m_alpPhaseClockCount);

		if (result == false)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Code runtime calculcation error!")))
		}

		return result;
	}

	bool ModuleLogicCompiler::writeResult()
	{
		bool result = true;

		m_code.generateBinCode();

		QByteArray binCode;

		m_code.getBinCode(binCode);

		quint64 uniqueID = 0;

		result &= setLmAppLANDataUID(binCode, uniqueID);

		if (result == false)
		{
			return false;
		}

		result &= m_appLogicCompiler.writeBinCodeForLm(m_lmSubsystemID, m_lmSubsystemKey, m_lm->equipmentIdTemplate(), m_lm->caption(),
														m_lmNumber, m_lmAppLogicFrameSize, m_lmAppLogicFrameCount, uniqueID, m_code);
		if (result == false)
		{
			return false;
		}

/*		QStringList mifCode;

		m_code.getMifCode(mifCode);

		BuildFile* buildFile = m_resultWriter->addFile(m_lmSubsystemID, QString("%1-%2.mif").arg(m_lm->caption()).arg(m_lmNumber), mifCode);

		if (buildFile == nullptr)
		{
			result = false;
		}*/

		QStringList asmCode;

		m_code.getAsmCode(asmCode);

		BuildFile* buildFile = m_resultWriter->addFile(m_lmSubsystemID, QString("%1-%2.asm").arg(m_lm->caption()).arg(m_lmNumber), asmCode);

		if (buildFile == nullptr)
		{
			result = false;
		}

		QStringList memFile;

		m_memoryMap.getFile(memFile);

		buildFile = m_resultWriter->addFile(m_lmSubsystemID, QString("%1-%2.mem").arg(m_lm->caption()).arg(m_lmNumber), memFile);

		if (buildFile == nullptr)
		{
			result = false;
		}

		result &= writeTuningInfoFile(m_lm->caption(), m_lmSubsystemID, m_lmNumber);

		//
		// writeLMCodeTestFile();
		//

		result &= writeOcmRsSignalsXml();

		return result;
	}

	bool ModuleLogicCompiler::setLmAppLANDataUID(const QByteArray& lmAppCode, quint64& uniqueID)
	{
		Crc64 crc;

		crc.add(lmAppCode);

		QVector<Signal*> acquiredSignals;

		acquiredSignals.append(m_acquiredDiscreteInputSignals);
		acquiredSignals.append(m_acquiredDiscreteOutputSignals);
		acquiredSignals.append(m_acquiredDiscreteInternalSignals);
		acquiredSignals.append(m_acquiredDiscreteTuningSignals);

		acquiredSignals.append(m_acquiredAnalogInputSignals);
		acquiredSignals.append(m_acquiredAnalogOutputSignals);
		acquiredSignals.append(m_acquiredAnalogInternalSignals);
		acquiredSignals.append(m_acquiredAnalogTuningSignals);

		acquiredSignals.append(m_acquiredBuses);

		int count = acquiredSignals.count();

		// sort acquiredSignals by bitAddress ascending
		//
		for(int k = 0; k < count - 1; k++)
		{
			Signal* s1 = acquiredSignals[k];

			TEST_PTR_CONTINUE(s1);

			for(int i = k + 1; i < count; i++)
			{
				Signal* s2 = acquiredSignals[i];

				TEST_PTR_CONTINUE(s2);

				if (s1->regBufAddr().bitAddress() > s2->regBufAddr().bitAddress())
				{
					acquiredSignals[k] = s2;
					acquiredSignals[i] = s1;
				}
			}
		}

		// add signals to UID
		//
		for(Signal* s : acquiredSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->regBufAddr().isValid() == true)
			{
				crc.add(s->appSignalID());
				crc.add(s->regBufAddr().bitAddress());
			}
			else
			{
				assert(false);
			}
		}

		uniqueID = crc.result();

		return DeviceHelper::setIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											"AppLANDataUID",
											crc.result32(),
											m_log);
	}

	bool ModuleLogicCompiler::writeTuningInfoFile(const QString& lmCaption, const QString& subsystemID, int lmNumber)
	{
		if (m_tuningData == nullptr)
		{
			return true;
		}

		QStringList file;
		QString line = QString("------------------------------------------------------------------------------------------");

		file.append(QString("Tuning information file: %1\n").arg(m_lm->equipmentIdTemplate()));
		file.append(QString("LM eqipmentID: %1").arg(m_lm->equipmentIdTemplate()));
		file.append(QString("LM caption: %1").arg(m_lm->caption()));
		file.append(QString("LM number: %1\n").arg(lmNumber));
		file.append(QString("Frames used total: %1").arg(m_tuningData->usedFramesCount()));

		QString s;

		quint64 uniqueID = m_tuningData->uniqueID();

		file.append(s.sprintf("Unique data ID: %llu (0x%016llX)", uniqueID, uniqueID));

		const QVector<Signal*>& analogFloatSignals = m_tuningData->getAnalogFloatSignals();

		if (analogFloatSignals.count() > 0)
		{
			file.append(QString("\nAnalog signals, type Float (32 bits)"));
			file.append(line);
			file.append(QString("Address\t\tAppSignalID\t\t\tDefault\t\tLow Limit\tHigh Limit"));
			file.append(line);

			for(Signal* signal : analogFloatSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				QString str;

				str.sprintf("%05d:%02d\t%-24s\t%f\t%f\t%f",
								signal->tuningAddr().offset(),
								signal->tuningAddr().bit(),
								C_STR(signal->appSignalID()),
								signal->tuningDefaultValue(),
								signal->lowEngeneeringUnits(),
								signal->highEngeneeringUnits());
				file.append(str);
			}
		}

		const QVector<Signal*>& analogIntSignals = m_tuningData->getAnalogIntSignals();

		if (analogIntSignals.count() > 0)
		{
			file.append(QString("\nAnalog signals, type Signed Integer (32 bits)"));
			file.append(line);
			file.append(QString("Address\t\tAppSignalID\t\t\tDefault\t\tLow Limit\tHigh Limit"));
			file.append(line);

			for(Signal* signal : analogIntSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				QString str;

				str.sprintf("%05d:%02d\t%-24s\t%d\t\t%d\t\t%d",
								signal->tuningAddr().offset(),
								signal->tuningAddr().bit(),
								C_STR(signal->appSignalID()),
								static_cast<qint32>(signal->tuningDefaultValue()),
								static_cast<qint32>(signal->lowEngeneeringUnits()),
								static_cast<qint32>(signal->highEngeneeringUnits()));
				file.append(str);
			}
		}

		const QVector<Signal*>& discreteSignals = m_tuningData->getDiscreteSignals();

		if (discreteSignals.count() > 0)
		{
			file.append(QString("\nDiscrete signals (1 bit)"));
			file.append(line);
			file.append(QString("Address\t\tAppSignalID\t\t\tDefault\t\tLow Limit\tHigh Limit"));
			file.append(line);

			for(Signal* signal : discreteSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				QString str;

				int defaultValue = (static_cast<int>(signal->tuningDefaultValue()) == 0) ? 0 : 1;

				str.sprintf("%05d:%02d\t%-24s\t%d\t\t%d\t\t%d",
								signal->tuningAddr().offset(),
								signal->tuningAddr().bit(),
								C_STR(signal->appSignalID()),
								defaultValue,
								0,
								1);
				file.append(str);
			}
		}

/*
 * Generation of binary representation of tuning frames data
 *
 * removed to decrease size of *.tun files (issue RPCT-1601)
 *
 *
		QByteArray data;

		m_tuningData->getTuningData(&data);

		int size = data.count();

		int frameCount = size / FotipV2::TX_RX_DATA_SIZE;

		assert((size % FotipV2::TX_RX_DATA_SIZE) == 0);

		file.append(QString("\n"));

		int addr = 0;

		for(int f = 0; f < frameCount; f++)
		{
			QString s;

			file.append(QString("\nFrame: %1\n").arg(f));

			for(int i = 0; i < FotipV2::TX_RX_DATA_SIZE; i++)
			{
				quint8 byte = data[f * FotipV2::TX_RX_DATA_SIZE + i];

				QString sv;

				if ((i % 16) == 0)
				{
					s.sprintf("%04X:  ", addr);
				}

				sv.sprintf("%02X ", static_cast<unsigned int>(byte));

				s += sv;

				if ((i % 8) == 7)
				{
					s += " ";
				}

				if ((i % 16) == 15)
				{
					file.append(s);
					s.clear();
				}

				addr++;
			}

			if (s.isEmpty() == false)
			{
				file.append(s);
			}
		}
*/
		bool result = m_resultWriter->addFile(subsystemID, QString("%1-%2.tun").
										 arg(lmCaption).arg(lmNumber), file);
		return result;
	}

	bool ModuleLogicCompiler::writeOcmRsSignalsXml()
	{
		/*if (!m_signals || m_signals->isEmpty())
		{
			LOG_MESSAGE(m_log, tr("Signals not found!"));
			return true;
		}

		if (!m_connections)
		{
			LOG_MESSAGE(m_log, tr("Connections not found!"));
			return true;
		}

		if (m_signalsID.isEmpty())
		{
			createDeviceBoundSignalsMap();
		}*/
	/*
		equipmentWalker(m_chassis, [this](const Hardware::DeviceObject* device)
		{
			if (device->parent() == nullptr || !device->parent()->isModule())
			{
				return;
			}
			const Hardware::DeviceModule* module = device->getParentModule();

			if (module == nullptr || module->moduleFamily() != Hardware::DeviceModule::OCM)
			{
				return;
			}

			const Hardware::DeviceController* port = dynamic_cast<const Hardware::DeviceController*>(device);

			if (port == nullptr)
			{
				return;
			}
			for (int i = 0; i < m_connections->count(); i++)
			{
				auto connection = m_connections->get(i);

				if (connection->mode() != Hardware::OptoPort::Mode::Serial)
				{
					continue;
				}
				if (connection->port1StrID() != port->strId())
				{
					continue;
				}

				QByteArray data;
				QXmlStreamWriter serialDataXml(&data);

				serialDataXml.setAutoFormatting(true);
				serialDataXml.writeStartDocument();
				serialDataXml.writeStartElement("SerialData");

				m_resultWriter->buildInfo().writeToXml(serialDataXml);

				serialDataXml.writeStartElement("PortInfo");

				serialDataXml.writeAttribute("StrID", connection->port1StrID());
				serialDataXml.writeAttribute("ID", QString::number(connection->index()));
				serialDataXml.writeAttribute("DataID", "12334");
				serialDataXml.writeAttribute("Speed", "115200");
				serialDataXml.writeAttribute("Bits", "8");
				serialDataXml.writeAttribute("StopBits", "2");
				serialDataXml.writeAttribute("ParityControl", "false");
				serialDataXml.writeAttribute("DataSize", "512");

				serialDataXml.writeEndElement();	// </PortInfo>

				serialDataXml.writeStartElement("Signals");

				QList<Signal*> connectionSignalList;

				for (QString signalId : connection->signalList())
				{
					Signal* s = getSignal(signalId);
					if (s == nullptr)
					{
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Signal %1 listed in OCM connections not found in database").arg(signalId));
						continue;
					}
					connectionSignalList << s;
				}

				serialDataXml.writeAttribute("Count", QString::number(connectionSignalList.count()));

				for (auto s : connectionSignalList)
				{
					serialDataXml.writeStartElement("Signal");

					serialDataXml.writeAttribute("StrID", s->strID());
					serialDataXml.writeAttribute("ExtStrID", s->extStrID());
					serialDataXml.writeAttribute("Name", s->name());
					serialDataXml.writeAttribute("Type", QMetaEnum::fromType<E::SignalType>().valueToKey(s->typeInt()));
					serialDataXml.writeAttribute("Unit", Signal::m_unitList->valueAt(s->unitID()));
					serialDataXml.writeAttribute("DataSize", QString::number(s->dataSize()));
					serialDataXml.writeAttribute("DataFormat", QMetaEnum::fromType<E::DataFormat>().valueToKey(s->dataFormatInt()));
					serialDataXml.writeAttribute("ByteOrder", QMetaEnum::fromType<E::ByteOrder>().valueToKey(s->byteOrderInt()));
					serialDataXml.writeAttribute("Offset", "1234");
					serialDataXml.writeAttribute("BitNo", "0.." + QString::number(s->dataSize() - 1));

					serialDataXml.writeEndElement();	// </Signal>
				}

				serialDataXml.writeEndElement();	// </Signals>

				serialDataXml.writeEndElement();	// </SerialData>
				serialDataXml.writeEndDocument();

				m_resultWriter->addFile(m_lm->propertyValue("SubsystemID").toString(), QString("rs-%1-ocm.xml").arg(connection->port1StrID()), data);
			}
		});*/

		return true;
	}

	void ModuleLogicCompiler::writeLMCodeTestFile()
	{
		/*

		ApplicationLogicCode m_testCode;

		Command cmd;

		cmd.nop();

		m_testCode.append(cmd);

		m_resultWriter->addFile(m_lm->subSysID(), QString("lm_test_code.mif"), mifCode);

		*/
	}

	void ModuleLogicCompiler::displayResourcesUsageInfo()
	{
		QString str;

		double percentOfUsedCodeMemory = (m_code.commandAddress() * 100.0) / 65536.0;

		str.sprintf("%.2f", percentOfUsedCodeMemory);
		LOG_MESSAGE(m_log, QString(tr("Code memory used - %1%")).arg(str));

		if (percentOfUsedCodeMemory > 95)
		{
			if (percentOfUsedCodeMemory < 100)
			{
				// Usage of code memory exceed 95%.
				//
				m_log->wrnALC5073();
			}
			else
			{
				// Usage of code memory exceed 100%.
				//
				m_log->errALC5074();
			}
		}

		//

		double percentOfUsedBitMemory = m_memoryMap.bitAddressedMemoryUsed();

		str.sprintf("%.2f", percentOfUsedBitMemory);
		LOG_MESSAGE(m_log, QString(tr("Bit-addressed memory used - %1%")).arg(str));

		if (percentOfUsedBitMemory > 95)
		{
			if (percentOfUsedBitMemory < 100)
			{
				// Usage of bit-addressed memory exceed 95%.
				//
				m_log->wrnALC5075();
			}
			else
			{
				// Usage of bit-addressed memory exceed 100%.
				//
				m_log->errALC5076();
			}
		}

		//

		double percentOfUsedWordMemory = m_memoryMap.wordAddressedMemoryUsed();

		str.sprintf("%.2f", percentOfUsedWordMemory);
		LOG_MESSAGE(m_log, QString(tr("Word-addressed memory used - %1%")).arg(str));

		if (percentOfUsedWordMemory > 95)
		{
			if (percentOfUsedWordMemory < 100)
			{
				// Usage of word-addressed memory exceed 95%.
				//
				m_log->wrnALC5077();
			}
			else
			{
				// Usage of word-addressed memory exceed 100%.
				//
				m_log->errALC5078();
			}
		}

		//

		QString str_percent;

		// display IDR phase timing
		//
		double idrPhaseTime = (1.0/m_lmClockFrequency) * m_idrPhaseClockCount;
		double idrPhaseTimeUsed = 0;

		assert(m_lmIDRPhaseTime != 0);

		if (m_lmIDRPhaseTime != 0)
		{
			idrPhaseTimeUsed = (idrPhaseTime * 100) / (static_cast<double>(m_lmIDRPhaseTime) / 1000000.0);
		}

		str_percent.sprintf("%.2f", static_cast<float>(idrPhaseTimeUsed));
		str.sprintf("%.2f", static_cast<float>(idrPhaseTime * 1000000));

		LOG_MESSAGE(m_log, QString(tr("Input Data Receive phase time used - %1% (%2 clocks or %3 &micro;s of %4 &micro;s)")).
					arg(str_percent).arg(m_idrPhaseClockCount).arg(str).arg(m_lmIDRPhaseTime));

		if (idrPhaseTimeUsed > 90)
		{
			if (idrPhaseTimeUsed < 100)
			{
				// Usage of IDR phase time exceed 90%.
				//
				m_log->wrnALC5079();
			}
			else
			{
				// Usage of IDR phase time exceed 100%.
				//
				m_log->errALC5080();
			}
		}

		// display ALP phase timing
		//
		double alpPhaseTime = (1.0/m_lmClockFrequency) * m_alpPhaseClockCount;
		double alpPhaseTimeUsed = 0;

		assert(m_lmALPPhaseTime != 0);

		if (m_lmALPPhaseTime != 0)
		{
			alpPhaseTimeUsed = (alpPhaseTime * 100) / (static_cast<double>(m_lmALPPhaseTime) / 1000000.0);
		}

		str_percent.sprintf("%.2f", static_cast<float>(alpPhaseTimeUsed));
		str.sprintf("%.2f", static_cast<float>(alpPhaseTime * 1000000));

		LOG_MESSAGE(m_log, QString(tr("Application Logic Processing phase time used - %1% (%2 clocks or %3 &micro;s of %4 &micro;s)")).
					arg(str_percent).arg(m_alpPhaseClockCount).arg(str).arg(m_lmALPPhaseTime));

		if (alpPhaseTimeUsed > 90)
		{
			if (alpPhaseTimeUsed < 100)
			{
				// Usage of ALP phase time exceed 90%.
				//
				m_log->wrnALC5081();
			}
			else
			{
				// Usage of ALP phase time exceed 100%.
				//
				m_log->errALC5082();
			}
		}

		m_resourcesUsageInfo.lmEquipmentID = m_lm->equipmentIdTemplate();
		m_resourcesUsageInfo.codeMemoryUsed = percentOfUsedCodeMemory;
		m_resourcesUsageInfo.bitMemoryUsed = percentOfUsedBitMemory;
		m_resourcesUsageInfo.wordMemoryUsed = percentOfUsedWordMemory;
		m_resourcesUsageInfo.idrPhaseTimeUsed = idrPhaseTimeUsed;
		m_resourcesUsageInfo.alpPhaseTimeUsed = alpPhaseTimeUsed;
	}

	void ModuleLogicCompiler::cleanup()
	{
		for(AppItem* appItem : m_appItems)
		{
			delete appItem;
		}

		m_appItems.clear();

		for(AppItem* scalAppItem : m_scalAppItems)
		{
			delete scalAppItem;
		}

		m_scalAppItems.clear();
	}

	bool ModuleLogicCompiler::checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const Signal& destSignal, QUuid destSignalUuid)
	{
		if (srcSignal.isDiscrete())
		{
			if (destSignal.isAnalog())
			{
				// Discrete signal '%1' is connected to analog signal '%2'.
				//
				m_log->errALC5037(srcSignal.appSignalID(), srcSignalUuid, destSignal.appSignalID(), destSignalUuid);
				return false;
			}

			assert(destSignal.isDiscrete());

			// Both signals are discret

			return true;
		}

		if (srcSignal.isAnalog())
		{
			if (destSignal.isDiscrete())
			{
				// Analog signal '%1' is connected to discrete signal '%2'.
				//
				m_log->errALC5036(srcSignal.appSignalID(), srcSignalUuid, destSignal.appSignalID(), destSignalUuid);
				return false;
			}

			if (srcSignal.analogSignalFormat() != destSignal.analogSignalFormat())
			{
				// Signals '%1' and '%2' have different data format.
				//
				m_log->errALC5038(srcSignal.appSignalID(), srcSignalUuid, destSignal.appSignalID(), destSignalUuid);
				return false;
			}

			if (srcSignal.dataSize() != destSignal.dataSize())
			{
				// Signals '%1' and '%2' have different data size.
				//
				m_log->errALC5039(srcSignal.appSignalID(), srcSignalUuid, destSignal.appSignalID(), destSignalUuid);
				return false;
			}

			return true;
		}

		assert(false);		// unknown signal type

		return false;
	}

	bool ModuleLogicCompiler::checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const AppFb& fb, const LogicAfbSignal& afbSignal)
	{
		if (srcSignal.isDiscrete())
		{
			if (afbSignal.isAnalog())
			{
				// Discrete signal '%1' is connected to analog input '%2.%3'.
				//
				m_log->errALC5007(srcSignal.appSignalID(), fb.caption(), afbSignal.caption(), srcSignalUuid);
				return false;
			}

			// Both signals are discret
			return true;
		}

		if (srcSignal.isAnalog())
		{
			if (afbSignal.isDiscrete())
			{
				// Analog signal '%1' is connected to discrete input '%2.%3'.
				//
				m_log->errALC5010(srcSignal.appSignalID(), fb.caption(), afbSignal.caption(), srcSignalUuid);
				return false;
			}

			if (srcSignal.isCompatibleFormat(afbSignal.type(), afbSignal.dataFormat(), afbSignal.size(), afbSignal.byteOrder()) == false)
			{
				// Signal '%1' is connected to input '%2.%3' with uncompatible data format.
				//
				m_log->errALC5008(srcSignal.appSignalID(), fb.caption(), afbSignal.caption(), srcSignalUuid, fb.schemaID());
				return false;
			}

			return true;
		}

		assert(false);		// unknown signal type

		return false;
	}

	bool ModuleLogicCompiler::isUsedInUal(const Signal* s) const
	{
		TEST_PTR_RETURN_FALSE(s);

		return isUsedInUal(s->appSignalID());
	}

	bool ModuleLogicCompiler::isUsedInUal(const QString& appSignalID) const
	{
		return m_appSignals.containsSignal(appSignalID);
	}

	QString ModuleLogicCompiler::getSchemaID(const LogicConst& constItem)
	{
		AppItem* appItem = m_appItems.value(constItem.guid(), nullptr);

		if (appItem != nullptr)
		{
			return appItem->schemaID();
		}

		return QString();
	}

	bool ModuleLogicCompiler::getLMIntProperty(const QString& name, int *value)
	{
		return DeviceHelper::getIntProperty(m_lm, name, value, m_log);
	}

	bool ModuleLogicCompiler::getLMStrProperty(const QString& name, QString *value)
	{
		return DeviceHelper::getStrProperty(m_lm, name, value, m_log);
	}

	QString ModuleLogicCompiler::getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType)
	{
		QString typeStr = m_moduleFamilyTypeStr.value(familyType, "???");

		assert(typeStr != "???");

		return typeStr;
	}

	void ModuleLogicCompiler::dumpApplicationLogicItems()
	{
		if (m_moduleLogic == nullptr)
		{
			return;
		}

		const std::list<AppLogicItem>& logicItems = m_moduleLogic->items();

		if (logicItems.empty() == true)
		{
			return;
		}

		qDebug() << "----------------------------- APPLICATION LOGIC BEGIN --------------------------";

		for(const AppLogicItem& item : logicItems)
		{
			if (item.m_fblItem->isSignalElement())
			{
				const VFrame30::SchemaItemSignal* s = item.m_fblItem->toSignalElement();

				if (item.m_fblItem->isInputSignalElement())
				{
					qDebug() << QString("Input signal %1").arg(s->appSignalIds());
					continue;
				}

				if (item.m_fblItem->isOutputSignalElement())
				{
					qDebug() << QString("Output signal %1").arg(s->appSignalIds());
					continue;
				}
			}

			if (item.m_fblItem->isAfbElement())
			{
				const VFrame30::SchemaItemAfb* afb = item.m_fblItem->toAfbElement();
				qDebug() << QString("Afb %1").arg(afb->afbStrID());
				continue;
			}

			if (item.m_fblItem->isConstElement())
			{
				const VFrame30::SchemaItemConst* c = item.m_fblItem->toSchemaItemConst();
				qDebug() << QString("Const %1").arg(c->valueToString());
				continue;
			}
		}
		qDebug() << "----------------------------- APPLICATION LOGIC END --------------------------";
	}

	// ---------------------------------------------------------------------------------------
	//
	// ModuleLogicCompiler::Module class implementation
	//
	// ---------------------------------------------------------------------------------------

	bool ModuleLogicCompiler::Module::isInputModule() const
	{
		if (device == nullptr)
		{
			assert(false);
			return false;
		}

		return device->isInputModule();
	}


	bool ModuleLogicCompiler::Module::isOutputModule() const
	{
		if (device == nullptr)
		{
			assert(false);
			return false;
		}

		return device->isOutputModule();
	}

	Hardware::DeviceModule::FamilyType ModuleLogicCompiler::Module::familyType() const
	{
		if (device == nullptr)
		{
			assert(false);
			return Hardware::DeviceModule::FamilyType::OTHER;
		}

		return device->moduleFamily();
	}
}
