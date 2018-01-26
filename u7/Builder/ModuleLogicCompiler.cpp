#include "../Builder/ModuleLogicCompiler.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"
#include "../lib/Crc.h"
#include "../TuningIPEN/TuningIPENDataStorage.h"

#include "Connection.h"
#include "Parser.h"
#include "Builder.h"

#define LOG_UNDEFINED_UAL_ADDRESS(log, ualSignal) log->writeError(QString("Undefined signal's ualAddress: %1 (File: %2 Line: %3 Function: %4)").arg(ualSignal->refSignalIDs().join(", ")).arg(__FILE__).arg(__LINE__).arg(SHORT_FUNC_INFO));

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

	const char* ModuleLogicCompiler::BUS_COMPOSER_CAPTION = "BusComposer";
	const char* ModuleLogicCompiler::BUS_EXTRACTOR_CAPTION = "BusExtractor";

	const char* ModuleLogicCompiler::TEST_DATA_DIR = "TestData/";

	ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, const Hardware::DeviceModule* lm) :
		m_appLogicCompiler(appLogicCompiler),
		m_memoryMap(appLogicCompiler.m_log),
		m_ualSignals(*this, appLogicCompiler.m_log)
	{
		m_equipmentSet = appLogicCompiler.m_equipmentSet;
		m_deviceRoot = m_equipmentSet->root();
		m_signals = appLogicCompiler.m_signals;

		m_lmDescription = appLogicCompiler.m_lmDescriptions->get(lm);

		assert(m_lmDescription != nullptr);

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

		ProcsToCallArray procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::loadLMSettings),
			PROC_TO_CALL(ModuleLogicCompiler::loadModulesSettings),
			PROC_TO_CALL(ModuleLogicCompiler::createChassisSignalsMap),
			PROC_TO_CALL(ModuleLogicCompiler::createUalItemsMaps),
			PROC_TO_CALL(ModuleLogicCompiler::createUalAfbsMap),
			PROC_TO_CALL(ModuleLogicCompiler::createUalSignals),
			PROC_TO_CALL(ModuleLogicCompiler::processTxSignals),
			PROC_TO_CALL(ModuleLogicCompiler::processSinglePortRxSignals),
			PROC_TO_CALL(ModuleLogicCompiler::buildTuningData),
			PROC_TO_CALL(ModuleLogicCompiler::createSignalLists),
//			PROC_TO_CALL(ModuleLogicCompiler::groupTxSignals),
			PROC_TO_CALL(ModuleLogicCompiler::disposeSignalsInMemory),
			PROC_TO_CALL(ModuleLogicCompiler::appendAfbsForAnalogInOutSignalsConversion),
			PROC_TO_CALL(ModuleLogicCompiler::setOutputSignalsAsComputed),
			PROC_TO_CALL(ModuleLogicCompiler::setOptoRawInSignalsAsComputed),
		};

		bool result = runProcs(procs);

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

		ProcsToCallArray procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::finalizeOptoConnectionsProcessing),
			PROC_TO_CALL(ModuleLogicCompiler::setOptoUalSignalsAddresses),
			PROC_TO_CALL(ModuleLogicCompiler::writeSignalLists),
			PROC_TO_CALL(ModuleLogicCompiler::initAfbs),
			PROC_TO_CALL(ModuleLogicCompiler::startAppLogicCode),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredRawDataInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::convertAnalogInputSignals),
			PROC_TO_CALL(ModuleLogicCompiler::generateAppLogicCode),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogOptoSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogBusChildSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredTuningAnalogSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogConstSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteInputSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredTuningDiscreteSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteConstSignalsToRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyOutputSignalsInOutputModulesMemory),
			PROC_TO_CALL(ModuleLogicCompiler::copyOptoConnectionsTxData),
			PROC_TO_CALL(ModuleLogicCompiler::finishAppLogicCode),
			PROC_TO_CALL(ModuleLogicCompiler::setLmAppLANDataSize),
			PROC_TO_CALL(ModuleLogicCompiler::calculateCodeRunTime),
			PROC_TO_CALL(ModuleLogicCompiler::writeResult)
		};

		bool result = runProcs(procs);

		if (result == true)
		{
			result &= displayResourcesUsageInfo();
		}

		if (result == true)
		{
			msg = QString(tr("Compilation pass #2 for LM %1 was successfully finished.")).
					arg(m_lm->equipmentIdTemplate());

			LOG_SUCCESS(m_log, msg);
		}
		else
		{
			msg = QString(tr("Compilation pass #2 for LM %1 was finished with errors")).arg(m_lm->equipmentIdTemplate());
			LOG_MESSAGE(m_log, msg);
		}

		calcOptoDiscretesStatistics();

		cleanup();

		return result;
	}

	QString ModuleLogicCompiler::lmEquipmentID()
	{
		if (m_lm == nullptr)
		{
			assert(false);
			LOG_NULLPTR_ERROR(m_log);
			return QString();
		}

		return m_lm->equipmentIdTemplate();
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

		m_memoryMap.init(m_lmDescription->memory().m_appMemorySize,
						 moduleData,
						 optoInterfaceData,
						 appLogicBitData,
						 tuningData,
						 appLogicWordData);

		m_code.setMemoryMap(&m_memoryMap, m_log);

		m_lmCodeMemorySize = m_lmDescription->memory().m_codeMemorySize;
		m_lmAppMemorySize = m_lmDescription->memory().m_appMemorySize;

		if (m_lmCodeMemorySize == 0 || m_lmAppMemorySize == 0)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

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
		if (m_lm->isBvb() == true)
		{
			return true;
		}

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

					if (deviceModule->isLogicModule() == false && deviceModule->isBvb() == false)
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
						if (m_linkedValidtySignalsID.contains(deviceSignal->equipmentIdTemplate()) == false)
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

	bool ModuleLogicCompiler::createUalItemsMaps()
	{
		m_afbls.clear();

		for(std::shared_ptr<Afb::AfbElement> afbl : m_lmDescription->afbs())
		{
			m_afbls.insert(afbl);
		}

		m_ualItems.clear();
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
			if (m_ualItems.contains(logicItem.m_fblItem->guid()) == true)
			{
				UalItem* firstItem = m_ualItems[logicItem.m_fblItem->guid()];

				msg = QString(tr("Duplicate GUID %1 of %2 and %3 elements")).
						arg(logicItem.m_fblItem->guid().toString()).arg(firstItem->strID()).arg(getUalItemStrID(logicItem));

				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

				result = false;

				continue;
			}

			UalItem* appItem = new UalItem(logicItem);

			m_ualItems.insert(appItem->guid(), appItem);

			// build QHash<QUuid, LogicItem*> m_itemsPins;
			// pin GUID -> parent item ptr
			//

			// add input pins
			//
			for(LogicPin input : appItem->inputs())
			{
				if (m_pinParent.contains(input.guid()))
				{
					UalItem* firstItem = m_pinParent[input.guid()];

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
					UalItem* firstItem = m_pinParent[output.guid()];

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

	QString ModuleLogicCompiler::getUalItemStrID(const AppLogicItem& appLogicItem) const
	{
		UalItem appItem(appLogicItem); return appItem.strID();
	}

	bool ModuleLogicCompiler::createUalAfbsMap()
	{
		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem->isAfb() == false)
			{
				continue;
			}

			UalAfb* ualAfb = createUalAfb(*ualItem);

			if (ualAfb == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createUalSignals()
	{
		m_ualSignals.clear();
		m_outPinSignal.clear();

		bool result = true;

		// fill m_ualSignals by Input and Tuning Acquired signals
		//
		for(Signal* s : m_chassisSignals)
		{
			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				continue;
			}

			if (s->isAcquired() == false)
			{
				continue;
			}

			if (s->isInput() == true)
			{
				m_ualSignals.createSignal(s);
				continue;
			}

			if (s->enableTuning() == true)
			{
				assert(s->isInternal());
				m_ualSignals.createSignal(s);
				continue;
			}
		}

		// create Bus parent and Bus child signals from BusComposers
		//
		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualItem->isBusComposer() == true)
			{
				result &= createUalSignalsFromBusComposer(ualItem);
			}
		}

		// create opto signals from Receivers
		//
		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualItem->isReceiver() == true)
			{
				result &= createUalSignalsFromReceiver(ualItem);
			}
		}

		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			bool res = true;

			switch(ualItem->type())
			{
			// UAL items that can generate signals
			//
			case UalItem::Type::Signal:
				res = createUalSignalFromSignal(ualItem, 1);
				break;

			case UalItem::Type::Const:
				res = createUalSignalFromConst(ualItem);
				break;

			case UalItem::Type::Afb:
				res = createUalSignalsFromAfbOuts(ualItem);
				break;

			case UalItem::Type::BusExtractor:
				res = linkUalSignalsFromBusExtractor(ualItem);
				break;

			case UalItem::Type::BusComposer:
			case UalItem::Type::Receiver:
				// already processed in previous 'for's
				break;

			// UAL items that doesn't generate signals
			//
			case UalItem::Type::Transmitter:
			case UalItem::Type::Terminator:
			case UalItem::Type::LoopbackOutput:
			case UalItem::Type::LoopbackInput:
				break;

			// unknown item's type
			//
			case UalItem::Type::Unknown:
			default:
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}

			result &= res;
		}

		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualItem->isSignal() != true)
			{
				continue;
			}

			result &= createUalSignalFromSignal(ualItem, 2);
		}

		for(UalSignal* ualSignal : m_ualSignals)
		{
			ualSignal->sortRefSignals();
		}

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalsFromBusComposer(UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalBusComposer* busComposer = ualItem->ualBusComposer();

		if (busComposer == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<LogicPin>& outputs = busComposer->outputs();

		if (outputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const LogicPin& outPin = outputs[0];

		if (isConnectedToTerminatorOnly(outPin) == true)
		{
			// not need to create bus signal
			return true;
		}

		QString busTypeID = busComposer->busTypeId();

		BusShared bus = m_signals->getBus(busTypeID);

		if (bus == nullptr)
		{
			// Bus type ID '%1' is undefined (Logic schema '%2').
			//
			m_log->errALC5100(busTypeID, ualItem->guid(), ualItem->schemaID());
			return false;
		}

		Signal* connectedBusSignal = getCompatibleConnectedBusSignal(outPin, busTypeID);

		UalSignal* parentBusSignal = m_ualSignals.createBusParentSignal(ualItem, connectedBusSignal, bus, outPin.guid(), outPin.caption());

		if (parentBusSignal == nullptr)
		{
			return false;
		}

		bool result = linkConnectedItems(ualItem, outPin, parentBusSignal);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromSignal(UalItem* ualItem, int passNo)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (ualItem->isSignal() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString signalID = ualItem->strID();

		Signal* s = m_signals->getSignal(signalID);

		if (s == nullptr)
		{
			m_log->errALC5000(signalID, ualItem->guid(), ualItem->schemaID());
			return false;
		}

		if (m_chassisSignals.contains(signalID) == false)
		{
			// The signal '%1' is not associated with LM '%2'.
			//
			m_log->errALC5030(signalID, m_lm->equipmentId(), ualItem->guid());
			return false;
		}

		bool result = true;

		UalSignal* ualSignal = m_ualSignals.get(signalID);

		if (ualSignal != nullptr)
		{
			result = appendRefPinToSignal(ualItem, ualSignal);

			if (result == false)
			{
				return false;
			}

			if (ualItem->outputs().size() > 0)
			{
				if (ualItem->outputs().size() == 1)
				{
					result = linkConnectedItems(ualItem, ualItem->outputs()[0], ualSignal);
				}
				else
				{
					LOG_INTERNAL_ERROR(m_log);			// number of signal's outputs more then 1
					result = false;
				}
			}

			return result;
		}

		// Only Input and Tuningable signals really can generate UalSignal

		if (s->isInput() == false && s->enableTuning() == false)
		{
			result = checkInOutsConnectedToSignal(ualItem, true);

			if (result == false && passNo > 1)
			{
				// Signal '%1' is not connected to any signal source. (Logic schema '%2').
				//
				m_log->errALC5118(signalID, ualItem->guid(), ualItem->schemaID());
				return false;
			}

			return true;
		}

		if (ualItem->inputs().size() != 0)
		{
			// Can't assign value to input or tuningable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5121(s->appSignalID(), ualItem->guid(), ualItem->schemaID());
			return false;
		}

		const std::vector<LogicPin>& outputs = ualItem->outputs();

		if (outputs.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// input signal must have only one output pin
			return false;
		}

		const LogicPin& outPin = ualItem->outputs()[0];

		ualSignal = m_ualSignals.createSignal(ualItem, s, outPin.guid());

		if (ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		// link connected signals to newly created UalSignal
		//
		result = linkConnectedItems(ualItem, outPin, ualSignal);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromConst(UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalConst* ualConst = ualItem->ualConst();

		if (ualConst == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<LogicPin>& outputs = ualItem->outputs();

		if (outputs.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// Const must have only one output pin
			return false;
		}

		const LogicPin& outPin = ualItem->outputs()[0];

		E::SignalType constSignalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat constAnalogFormat = E::AnalogAppSignalFormat::SignedInt32;

		switch(ualConst->type())
		{
		case VFrame30::SchemaItemConst::ConstType::Discrete:

			constSignalType = E::SignalType::Discrete;

			if (ualConst->discreteValue() != 0 && ualConst->discreteValue() != 1)
			{
				// Discrete constant must have value 0 or 1.
				//
				m_log->errALC5086(ualItem->guid(), ualItem->schemaID());
				return false;
			}
			break;

		case VFrame30::SchemaItemConst::ConstType::IntegerType:

			constSignalType = E::SignalType::Analog;
			constAnalogFormat = E::AnalogAppSignalFormat::SignedInt32;

			if (ualConst->intValue() < std::numeric_limits<qint32>::min() || ualConst->intValue() > std::numeric_limits<qint32>::max())
			{
				// Integer constant value out of range (Logic schema %1, item %2)
				//
				m_log->errALC5134(ualItem->guid(), ualItem->label(), ualItem->schemaID());
				return false;
			}

			break;

		case VFrame30::SchemaItemConst::ConstType::FloatType:

			constSignalType = E::SignalType::Analog;
			constAnalogFormat = E::AnalogAppSignalFormat::Float32;

			if (ualConst->floatValue() < std::numeric_limits<float>::lowest() || ualConst->floatValue() > std::numeric_limits<float>::max())
			{
				// Float constant value out of range (Logic schema %1, item %2)
				//
				m_log->errALC5135(ualItem->guid(), ualItem->label(), ualItem->schemaID());
				return false;
			}

			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		UalSignal* ualSignal = m_ualSignals.createConstSignal(ualItem,
															  constSignalType,
															  constAnalogFormat,
															  outPin.guid());
		if (ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		// link connected signals to newly created UalSignal
		//
		bool result = linkConnectedItems(ualItem, outPin, ualSignal);

		return result;
	}


	bool ModuleLogicCompiler::createUalSignalsFromAfbOuts(UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		UalAfb* ualAfb = m_ualAfbs.value(ualItem->guid(), nullptr);

		if (ualAfb == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		bool isBusProcessing = false;

		result = isBusProcessingAfb(ualAfb, &isBusProcessing);

		if (result == false)
		{
			return false;
		}

		QString outBusTypeID;
		BusShared bus;

		if (isBusProcessing == true)
		{
			result = determineOutBusTypeID(ualAfb, &outBusTypeID);

			if (result == false)
			{
				// Output bus type cannot be determined (Logic schema %1, item %2)
				//
				m_log->errALC5127(ualItem->guid(), ualItem->label(), ualItem->schemaID());
				return false;
			}

			bus = m_signals->getBus(outBusTypeID);

			if (bus == nullptr)
			{
				// Bus type ID '%1' is undefined (Logic schema '%2').
				//
				m_log->errALC5100(outBusTypeID, ualItem->guid(), ualItem->schemaID());
				return false;
			}
		}

		const std::vector<LogicPin>& outputs = ualItem->outputs();

		for(const LogicPin& outPin : outputs)
		{
			LogicAfbSignal outAfbSignal;

			if (ualAfb->getAfbSignalByPin(outPin, &outAfbSignal) == false)
			{
				result = false;
				continue;
			}

			bool connectedToTerminatorOnly = isConnectedToTerminatorOnly(outPin);

			if (connectedToTerminatorOnly == true)
			{
				continue;				// not needed to create signal
			}

			UalSignal* ualSignal = nullptr;

			Signal* s = getCompatibleConnectedSignal(outPin, outAfbSignal, outBusTypeID);

			switch(outAfbSignal.type())
			{
			case E::SignalType::Analog:
			case E::SignalType::Discrete:
				if (s == nullptr)
				{
					ualSignal = m_ualSignals.createAutoSignal(ualItem, outPin.guid(), outAfbSignal);
				}
				else
				{
					ualSignal = m_ualSignals.createSignal(ualItem, s, outPin.guid());
				}
				break;

			case E::SignalType::Bus:

				if ((s != nullptr && s->isBus() == false) || (bus == nullptr))
				{
					assert(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				ualSignal = m_ualSignals.createBusParentSignal(ualItem, s, bus, outPin.guid(), outAfbSignal.caption());
				break;

			default:
				assert(false);
			}

			if (ualSignal == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			result &= linkConnectedItems(ualItem, outPin, ualSignal);
		}

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalsFromReceiver(UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalReceiver* ualReceiver = ualItem->ualReceiver();

		if (ualReceiver == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString connectionID = ualReceiver->connectionId();

		std::shared_ptr<Hardware::Connection> connection = m_optoModuleStorage->getConnection(connectionID);

		if (connection == nullptr)
		{
			// Receiver is linked to unknown opto connection '%1'.
			//
			m_log->errALC5025(connectionID, ualReceiver->guid(), ualItem->schemaID());
			return false;
		}

		const std::vector<LogicPin>& outputs = ualItem->outputs();

		int outPinIndex = -1;
		int validityPinIndex = -1;

		for(int i = 0; i < outputs.size(); i++)
		{
			if (ualReceiver->isValidityPin(outputs[i].guid()) == true)
			{
				if (validityPinIndex != -1)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				validityPinIndex = i;
			}

			if (ualReceiver->isOutputPin(outputs[i].guid()) == true)
			{
				if (outPinIndex != -1)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				outPinIndex = i;
			}
		}

		if (outPinIndex == -1)
		{
			LOG_INTERNAL_ERROR(m_log);			// signal out pin is not found, why?
			return false;
		}

		// UalSignal creation from receiver's output pin
		//
		const LogicPin& outPin = outputs[outPinIndex];

		QString appSignalID = ualReceiver->appSignalId();

		bool result = true;

		result &= createUalSignalFromReceiverOutput(ualItem, outPin, appSignalID);

		// UalSignal creation from receiver's validity pin
		//

		if (validityPinIndex == -1)
		{
			return result;						// receiver hasn't validity pin, it is ok
		}

		const LogicPin& validityPin = outputs[validityPinIndex];

		Hardware::OptoPortShared port = m_optoModuleStorage->getOptoPort(connection->port1EquipmentID());

		if (port == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);				// port isn't found
			return false;
		}

		if (port->lmID() != m_lm->equipmentIdTemplate())
		{
			if (connection->isSinglePort() == true)
			{
				LOG_INTERNAL_ERROR(m_log);				// port associated with current LM isn't found
				return false;
			}

			assert(connection->isPortToPort() == true);

			port = m_optoModuleStorage->getOptoPort(connection->port2EquipmentID());

			if (port == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);				// port isn't found
				return false;
			}

			if (port->lmID() != m_lm->equipmentIdTemplate())
			{
				LOG_INTERNAL_ERROR(m_log);				// port associated with current LM isn't found
				return false;
			}
		}

		QString validitySignalID = port->validitySignalID();

		result &= createUalSignalFromReceiverValidity(ualItem, validityPin, validitySignalID);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromReceiverOutput(UalItem* ualItem, const LogicPin& outPin, const QString& appSignalID)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		UalSignal* ualSignal = m_ualSignals.get(appSignalID);

		if (ualSignal != nullptr)
		{
			// signal already in map
			//
			m_ualSignals.appendRefPin(ualItem, outPin.guid(), ualSignal);
		}
		else
		{
			// create opto signal
			//
			Signal* s = m_signals->getSignal(appSignalID);

			if (s == nullptr)
			{
				m_log->errALC5000(appSignalID, ualItem->guid(), ualItem->schemaID());
				return false;
			}

			ualSignal = m_ualSignals.createOptoSignal(ualItem, s, m_lm->equipmentIdTemplate(), outPin.guid());

			if (ualSignal == nullptr)
			{
				return false;
			}
		}

		// link connected signals to UalSignal
		//
		bool result = linkConnectedItems(ualItem, outPin, ualSignal);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromReceiverValidity(UalItem* ualItem, const LogicPin& validityPin, const QString& validitySignalEquipmentID)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		Signal* s = m_equipmentSignals.value(validitySignalEquipmentID);

		if (s == nullptr)
		{
			m_log->errALC5133(validitySignalEquipmentID, ualItem->guid(), ualItem->label(), ualItem->schemaID());
			return false;
		}

		UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

		if (ualSignal != nullptr)
		{
			// signal already in map
			//
			m_ualSignals.appendRefPin(ualItem, validityPin.guid(), ualSignal);
		}
		else
		{
			// create signal (non-opto! validity is Input signal from module's PI controller)
			//

			ualSignal = m_ualSignals.createSignal(ualItem, s, validityPin.guid());

			if (ualSignal == nullptr)
			{
				return false;
			}
		}

		// link connected signals to UalSignal
		//
		bool result = linkConnectedItems(ualItem, validityPin, ualSignal);

		return result;
	}

	bool ModuleLogicCompiler::linkUalSignalsFromBusExtractor(UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalBusExtractor* extractor = ualItem->ualBusExtractor();

		const std::vector<LogicPin>& inputs = extractor->inputs();

		if (inputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		UalSignal* busSignal = m_ualSignals.get(inputs[0].guid());

		if (busSignal == nullptr)
		{
			// UalSignal is not found for pin '%1' (Logic schema '%2').
			//
			m_log->errALC5120(ualItem->guid(), ualItem->label(), "in", ualItem->schemaID());
			return false;
		}

		QString busTypeID = extractor->busTypeId();

		BusShared bus = m_signals->getBus(busTypeID);

		if (bus == nullptr)
		{
			// Bus type ID '%1' is undefined (Logic schema '%2').
			//
			m_log->errALC5100(busTypeID, ualItem->guid(), ualItem->schemaID());
			return false;
		}

		if (busSignal->busTypeID() != busTypeID)
		{
			LOG_INTERNAL_ERROR(m_log);			// this error must be detected early, when link BusExtractor input
			return false;
		}

		const std::vector<LogicPin>& outputs = extractor->outputs();

		bool result = true;

		for(const LogicPin& outPin : outputs)
		{
			UalSignal* busChildSignal = busSignal->getBusChildSignal(outPin.caption());

			if (busChildSignal == nullptr)
			{
				assert(false);					// busChildSignal with ID == outPin.caption() is not found, why?
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			// link connected signals to UalSignal
			//
			result &= linkConnectedItems(ualItem, outPin, busChildSignal);
		}

		return result;
	}

	bool ModuleLogicCompiler::linkConnectedItems(UalItem* srcUalItem, const LogicPin& outPin, UalSignal* ualSignal)
	{
		if (srcUalItem == nullptr || ualSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (outPin.IsOutput() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		const std::vector<QUuid>& associatedInputs = outPin.associatedIOs();

		for(QUuid inPinUuid : associatedInputs)
		{
			UalItem* destUalItem = m_pinParent.value(inPinUuid, nullptr);

			if (destUalItem == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);			// pin's parent is not found
				return false;
			}

			switch(destUalItem->type())
			{
			case UalItem::Type::Signal:
				result &= linkSignal(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			case UalItem::Type::Afb:
				result &= linkAfbInput(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			case UalItem::Type::BusComposer:
				result &= linkBusComposerInput(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			case UalItem::Type::BusExtractor:
				result &= linkBusExtractorInput(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			// link pins to signal only, any checks is not required
			//
			case UalItem::Type::Transmitter:
			case UalItem::Type::LoopbackOutput:
				m_ualSignals.appendRefPin(destUalItem, inPinUuid, ualSignal);
				break;

			case UalItem::Type::Terminator:
				break;

			// output can't connect to:
			//
			case UalItem::Type::Const:
			case UalItem::Type::Receiver:
			case UalItem::Type::LoopbackInput:
				m_log->errALC5116(srcUalItem->guid(), destUalItem->guid(), destUalItem->schemaID());
				result = false;
				break;

			// unknown UalItem type
			//
			case UalItem::Type::Unknown:
			default:
				assert(false);
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::linkSignal(UalItem* srcItem, UalItem* signalItem, QUuid inPinUuid, UalSignal* ualSignal)
	{
		if (srcItem == nullptr || signalItem == nullptr || ualSignal == nullptr || ualSignal->signal() == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (signalItem->isSignal() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString signalID = signalItem->strID();

		Signal* s = m_signals->getSignal(signalID);

		if (s == nullptr)
		{
			m_log->errALC5000(signalID, signalItem->guid(), signalItem->schemaID());
			return false;
		}

		UalSignal* existsSignal = m_ualSignals.get(s->appSignalID());

		if (existsSignal != nullptr && existsSignal != ualSignal && existsSignal->isSource() == true)
		{
			// Can't assign value to input/tuningable/opto/const signal %1 (Logic schema %2).
			//
			m_log->errALC5121(s->appSignalID(), signalItem->guid(), signalItem->schemaID());
			return false;
		}

		// check signals compatibility
		//
		bool result = ualSignal->isCompatible(s);

		if (result == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(srcItem->guid(), srcItem->label(), signalItem->guid(), signalItem->label(), signalItem->schemaID());
			return false;
		}

		result = m_ualSignals.appendRefPin(signalItem, inPinUuid, ualSignal);

		if (result == false)
		{
			return false;
		}

		result = m_ualSignals.appendRefSignal(s, ualSignal);

		if (result == false)
		{
			return false;
		}

		const std::vector<LogicPin>& outputs = signalItem->outputs();

		if (outputs.size() > 1)
		{
			LOG_INTERNAL_ERROR(m_log);				// signal connot have more then 1 output
			return false;
		}

		if (outputs.size() == 1)
		{
			const LogicPin& output = outputs[0];

			m_ualSignals.appendRefPin(signalItem, output.guid(), ualSignal);

			// recursive linking of items
			//
			result = linkConnectedItems(signalItem, output, ualSignal);
		}

		return result;
	}

	bool ModuleLogicCompiler::linkAfbInput(UalItem* srcItem, UalItem* destItem, QUuid inPinUuid, UalSignal* ualSignal)
	{
		if (srcItem == nullptr || destItem == nullptr || ualSignal == nullptr || ualSignal->signal() == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (destItem->isAfb() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		UalAfb* ualAfb = m_ualAfbs.value(destItem->guid(), nullptr);

		if (ualAfb == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		LogicAfbSignal inSignal;

		bool result = ualAfb->getAfbSignalByPinUuid(inPinUuid, &inSignal);

		if (result == false)
		{
			return false;
		}

		result = ualSignal->isCompatible(inSignal);

		if (result == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(srcItem->guid(), srcItem->label(), destItem->guid(), destItem->label(), destItem->schemaID());
			return false;
		}

		return m_ualSignals.appendRefPin(srcItem, inPinUuid, ualSignal);
	}

	bool ModuleLogicCompiler::linkBusComposerInput(UalItem* srcItem, UalItem* busComposerItem, QUuid inPinUuid, UalSignal* ualSignal)
	{
		if (srcItem == nullptr || busComposerItem == nullptr || ualSignal == nullptr || ualSignal->signal() == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalBusComposer* busComposer = busComposerItem->ualBusComposer();

		if (busComposer == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString busTypeID = busComposer->busTypeId();

		BusShared bus = m_signals->getBus(busTypeID);

		if (bus == nullptr)
		{
			// Bus type ID '%1' is undefined (Logic schema '%2').
			//
			m_log->errALC5100(busTypeID, busComposerItem->guid(), busComposerItem->schemaID());
			return false;
		}

		const LogicPin& inPin = busComposer->input(inPinUuid);

		QString busSignalID = inPin.caption();

		const BusSignal& busSignal = bus->signalByID(busSignalID);

		if (busSignal.isValid() == false)
		{
			LOG_INTERNAL_ERROR(m_log);				// bus signal with busSignalID is not found,
			return false;
		}

		if (ualSignal->isCompatible(busSignal) == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualSignal->ualItemGuid(), ualSignal->appSignalID(), busComposerItem->guid(), busComposerItem->label(), busComposerItem->schemaID());
			return false;
		}

		bool result = m_ualSignals.appendRefPin(busComposerItem, inPin.guid(), ualSignal);

		return result;
	}

	bool ModuleLogicCompiler::linkBusExtractorInput(UalItem* srcItem, UalItem* busExtractorItem, QUuid inPinUuid, UalSignal* ualSignal)
	{
		if (srcItem == nullptr || busExtractorItem == nullptr || ualSignal == nullptr || ualSignal->signal() == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalBusExtractor* busExtractor = busExtractorItem->ualBusExtractor();

		if (busExtractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString busTypeID = busExtractor->busTypeId();

		BusShared bus = m_signals->getBus(busTypeID);

		if (bus == nullptr)
		{
			// Bus type ID '%1' is undefined (Logic schema '%2').
			//
			m_log->errALC5100(busTypeID, busExtractorItem->guid(), busExtractorItem->schemaID());
			return false;
		}

		if (ualSignal->isBus() != true || ualSignal->busTypeID() != busTypeID)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualSignal->ualItemGuid(), ualSignal->appSignalID(), busExtractorItem->guid(), busExtractorItem->label(), busExtractorItem->schemaID());
			return false;
		}

		bool result = m_ualSignals.appendRefPin(busExtractorItem, inPinUuid, ualSignal);

		return result;
	}

	Signal* ModuleLogicCompiler::getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal, const QString busTypeID)
	{
		const std::vector<QUuid>& connectedPinsUuids = outPin.associatedIOs();

		for(QUuid inPinUuid : connectedPinsUuids)
		{
			UalItem* connectedItem = m_pinParent.value(inPinUuid, nullptr);

			if (connectedItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (connectedItem->isSignal() == false)
			{
				continue;
			}

			QString signalID = connectedItem->strID();

			Signal* s = m_signals->getSignal(signalID);

			if (s == nullptr)
			{
				continue;
			}

			switch(outAfbSignal.type())
			{
			case E::SignalType::Discrete:
			case E::SignalType::Analog:

				if (s->isCompatibleFormat(outAfbSignal.type(),
										  outAfbSignal.dataFormat(),
										  outAfbSignal.size(),
										  outAfbSignal.byteOrder()) == true)
				{
					return s;
				}

				break;

			case E::SignalType::Bus:

				if (s->isCompatibleFormat(outAfbSignal.type(), busTypeID) == true)
				{
					return s;
				}

				break;

			default:
				assert(false);
			}
		}

		return nullptr;
	}

	Signal* ModuleLogicCompiler::getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal)
	{
		return getCompatibleConnectedSignal(outPin, outAfbSignal, QString());
	}

	Signal* ModuleLogicCompiler::getCompatibleConnectedBusSignal(const LogicPin& outPin, const QString busTypeID)
	{
		LogicAfbSignal dummyBusSignal;

		dummyBusSignal.setType(E::SignalType::Bus);

		return getCompatibleConnectedSignal(outPin, dummyBusSignal, busTypeID);
	}

	bool ModuleLogicCompiler::isConnectedToTerminatorOnly(const LogicPin& outPin)
	{
		const std::vector<QUuid>& connectedPinsUuids = outPin.associatedIOs();

		for(QUuid inPinUuid : connectedPinsUuids)
		{
			UalItem* connectedItem = m_pinParent.value(inPinUuid, nullptr);

			if (connectedItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (connectedItem->isTerminator() == false)
			{
				return false;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::determineOutBusTypeID(UalAfb* ualAfb, QString* outBusTypeID)
	{
		if (ualAfb == nullptr || outBusTypeID == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		outBusTypeID->clear();

		int busInputsCount = 0;
		int discretesToBusConnectedCount = 0;

		for(const LogicPin& inPin : ualAfb->inputs())
		{
			LogicAfbSignal afbSignal;

			bool res = ualAfb->getAfbSignalByPin(inPin, &afbSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (afbSignal.isBus() == false)
			{
				continue;
			}

			busInputsCount++;

			// inPin is bus

			UalSignal* ualSignal = m_ualSignals.get(inPin.guid());

			if (ualSignal == nullptr)
			{
				// UalSignal is not found for pin '%1' (Logic schema '%2').
				//
				m_log->errALC5120(ualAfb->guid(), ualAfb->label(), inPin.caption(), ualAfb->schemaID());
				return false;
			}

			if (ualSignal->isBus() == true)
			{
				if (outBusTypeID->isEmpty() == true)
				{
					*outBusTypeID = ualSignal->busTypeID();
					continue;
				}

				if (*outBusTypeID != ualSignal->busTypeID())
				{
					// Different busTypes on AFB inputs (Logic schema %1).
					m_log->errALC5123(ualAfb->guid(), ualAfb->schemaID());
					return false;
				}

				continue;			// check remaining inputs
			}

			if (ualSignal->isDiscrete() && afbSignal.isBus() == true && afbSignal.busDataFormat() == E::BusDataFormat::Discrete)
			{
				// discrete to "discrete" bus - is allowed connection
				discretesToBusConnectedCount++;
				continue;
			}

			// Uncompatible signals connection (Logic schema '%1').
			//
			assert(false);				// this error must be detected earlier

			m_log->errALC5117(ualSignal->ualItemGuid(), ualSignal->ualItemLabel(),
							  ualAfb->guid(), ualAfb->label(), ualAfb->schemaID());
			return false;
		}

		if (outBusTypeID->isEmpty() == true && busInputsCount == discretesToBusConnectedCount)
		{
			// All AFB's bus inputs connected to discretes (Logic schema %1, item %2).
			//
			m_log->errALC5128(ualAfb->guid(), ualAfb->label(), ualAfb->schemaID());
			return false;
		}

		return outBusTypeID->isEmpty() == false;
	}

	bool ModuleLogicCompiler::checkInOutsConnectedToSignal(UalItem* ualItem, bool shouldConnectToSameSignal)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		UalSignal* sameSignal = nullptr;

		bool result = checkPinsConnectedToSignal(ualItem->inputs(), shouldConnectToSameSignal, &sameSignal);

		if (result == false)
		{
			return false;
		}

		result = checkPinsConnectedToSignal(ualItem->outputs(), shouldConnectToSameSignal, &sameSignal);

		return result;
	}

	bool ModuleLogicCompiler::checkPinsConnectedToSignal(const std::vector<LogicPin>& pins, bool shouldConnectToSameSignal, UalSignal** sameSignalPtr)
	{
		if (sameSignalPtr == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		for(const LogicPin& pin : pins)
		{
			UalSignal* connectedSignal = m_ualSignals.get(pin.guid());

			if (connectedSignal == nullptr)
			{
				return false;
			}

			if (shouldConnectToSameSignal == false)
			{
				continue;
			}

			if (*sameSignalPtr == nullptr)
			{
				*sameSignalPtr = connectedSignal;
			}
			else
			{
				if (*sameSignalPtr != connectedSignal)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::appendRefPinToSignal(UalItem* ualItem, UalSignal* ualSignal)
	{
		bool result = true;

		for(const LogicPin& inPin : ualItem->inputs())
		{
			UalSignal* existsSignal = m_ualSignals.get(inPin.guid());

			if (existsSignal != nullptr && existsSignal != ualSignal)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
			else
			{
				m_ualSignals.appendRefPin(ualItem, inPin.guid(), ualSignal);
			}
		}

		for(const LogicPin& outPin : ualItem->outputs())
		{
			UalSignal* existsSignal = m_ualSignals.get(outPin.guid());

			if (existsSignal != nullptr && existsSignal != ualSignal)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
			else
			{
				m_ualSignals.appendRefPin(ualItem, outPin.guid(), ualSignal);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::checkBusAndAfbInputCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem, QUuid destPinUuid)
	{
		if (srcAppItem == nullptr || bus == nullptr || destAppItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		// input of appItem connected to Bus must have
		// 1) 'bus' type
		// 2) maxBusSize > BusTypeID.sizeW
		// 3) same E::BusDataFormat
		//
		UalAfb* destAppFb = m_ualAfbs.value(destAppItem->guid(), nullptr);

		if (destAppFb == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		LogicAfbSignal destAfbSignal;

		if (destAppFb->getAfbSignalByPinUuid(destPinUuid, &destAfbSignal) == false)
		{
			return false;
		}

		if (destAfbSignal.isBus() == false)
		{
			// Bus output is connected to non-bus input (Logic schema '%1').
			//
			m_log->errALC5113(srcAppItem->guid(), destAppItem->guid(), srcAppItem->schemaID());
			return false;
		}

		if (destAfbSignal.busDataFormat() != bus->busDataFormat())
		{
			// Uncompatible bus data format of UAL elements (Logic schema '%1').
			//
			m_log->errALC5115(srcAppItem->guid(), destAppItem->guid(), srcAppItem->schemaID());
			return false;
		}

		return true;
	}

	bool ModuleLogicCompiler::checkBusAndSignalCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem)
	{
		if (srcAppItem == nullptr || bus == nullptr || destAppItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		// check that connected signal has 'bus' type and apropriate 'busTypeID'
		//
		UalSignal* appSignal = m_ualSignals.get(destAppItem->guid());

		if (appSignal == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		// check that connected signal has 'bus' type
		//
		if (appSignal->isBus() == false)
		{
			// Bus output is connected to non-bus input.
			//
			m_log->errALC5113(srcAppItem->guid(), destAppItem->guid(), srcAppItem->schemaID());
			return false;
		}

		// check that connected signal has apropriate 'busTypeID'
		//
		if (appSignal->busTypeID() != bus->busTypeID())
		{
			// Different bus types on UAL elements (Logic schema %1).
			//
			m_log->errALC5112(srcAppItem->guid(), destAppItem->guid(), srcAppItem->schemaID());
			return false;
		}

		return true;
	}

	bool ModuleLogicCompiler::checkBusAndBusExtractorCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem)
	{
		if (srcAppItem == nullptr || bus == nullptr || destAppItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		const UalBusExtractor* busExtractor = destAppItem->ualBusExtractor();

		if (busExtractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (busExtractor->busTypeId() != bus->busTypeID())
		{
			// Different bus types of UAL elements (Logic schema %1).
			//
			m_log->errALC5112(srcAppItem->guid(), destAppItem->guid(), srcAppItem->schemaID());
			return false;
		}

		return true;
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
		result &= createAcquiredDiscreteStrictOutputSignalsList();
		result &= createAcquiredDiscreteInternalSignalsList();
		result &= createAcquiredDiscreteOptoAndBusChildSignalsList();
		result &= createAcquiredDiscreteTuningSignalsList();
		result &= createAcquiredDiscreteConstSignalsList();

		result &= createNonAcquiredDiscreteInputSignalsList();
		result &= createNonAcquiredDiscreteStrictOutputSignalsList();
		result &= createNonAcquiredDiscreteInternalSignalsList();

		result &= createAcquiredAnalogInputSignalsList();
		result &= createAcquiredAnalogStrictOutputSignalsList();
		result &= createAcquiredAnalogInternalSignalsList();
		result &= createAcquiredAnalogOptoSignalsList();
		result &= createAcquiredAnalogBusChildSignalsList();
		result &= createAcquiredAnalogTuninglSignalsList();
		result &= createAcquiredAnalogConstSignalsList();

		result &= createNonAcquiredAnalogInputSignalsList();
		result &= createNonAcquiredAnalogStrictOutputSignalsList();
		result &= createNonAcquiredAnalogInternalSignalsList();

		result &= createAnalogOutputSignalsToConversionList();

		result &= createAcquiredBusSignalsList();
		result &= createNonAcquiredBusSignalsList();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		/*result = listsUniquenessCheck();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}*/

		sortSignalList(m_acquiredDiscreteInputSignals);
		sortSignalList(m_acquiredDiscreteStrictOutputSignals);
		sortSignalList(m_acquiredDiscreteInternalSignals);
		sortSignalList(m_acquiredDiscreteOptoAndBusChildSignals);
		sortSignalList(m_acquiredDiscreteConstSignals);
		// m_acquiredDiscreteTuningSignals						// Not need to sort!

		sortSignalList(m_nonAcquiredDiscreteInputSignals);
		sortSignalList(m_nonAcquiredDiscreteStrictOutputSignals);
		sortSignalList(m_nonAcquiredDiscreteInternalSignals);

		sortSignalList(m_acquiredAnalogInputSignals);
		sortSignalList(m_acquiredAnalogStrictOutputSignals);
		sortSignalList(m_acquiredAnalogInternalSignals);
		sortSignalList(m_acquiredAnalogOptoSignals);
		sortSignalList(m_acquiredAnalogBusChildSignals);
		// m_acquiredAnalogTuningSignals						// Not need to sort!

		sortSignalList(m_nonAcquiredAnalogInputSignals);
		sortSignalList(m_nonAcquiredAnalogStrictOutputSignals);
		sortSignalList(m_nonAcquiredAnalogInternalSignals);

		sortSignalList(m_acquiredBuses);
		sortSignalList(m_nonAcquiredBuses);

		return result;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteInputSignalsList()
	{
		m_acquiredDiscreteInputSignals.clear();
//		m_acquiredDiscreteInputSignalsMap.clear();

		//	list include signals that:
		//
		//  - const
		//	+ acquired
		//	+ discrete
		//	+ input
		//	+ no matter used in UAL or not

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isInput() == true)
			{
				m_acquiredDiscreteInputSignals.append(s);

				// if input signal is acquired, then validity signal (if exists) also always acquired
				//

				//appendLinkedValiditySignal(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteStrictOutputSignalsList()
	{
		m_acquiredDiscreteStrictOutputSignals.clear();

		//	list include signals that:
		//
		//	+ acquired
		//	+ discrete
		//	+ strict output
		//	+ used in UAL

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isStrictOutput() == true)
			{
				m_acquiredDiscreteStrictOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteInternalSignalsList()
	{
		m_acquiredDiscreteInternalSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	+ acquired
		//	+ discrete
		//	+ internal
		//  - tuningable
		//  - bus child signal
		//	+ used in UAL || is a SerialRx signal

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isBusChild() == false &&
				s->isAcquired() == true &&
				s->isDiscrete() == true &&
				s->isInternal() == true &&
				s->isTuningable() == false)
			{
				m_acquiredDiscreteInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteOptoAndBusChildSignalsList()
	{
		m_acquiredDiscreteOptoAndBusChildSignals.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isDiscrete() == true &&
				(ualSignal->isOptoSignal() == true || ualSignal->isBusChild() == true) &&
				ualSignal->isConst() == false)
			{
				m_acquiredDiscreteOptoAndBusChildSignals.append(ualSignal);
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

		QVector<Signal*> tuningSignals;

		m_tuningData->getAcquiredDiscreteSignals(tuningSignals);

		// check signals!

		for(Signal* s : tuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (ualSignal->isAcquired() == true &&
				ualSignal->isDiscrete() == true &&
				ualSignal->isTuningable() == true)
			{
				m_acquiredDiscreteTuningSignals.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteConstSignalsList()
	{
		m_acquiredDiscreteConstSignals.clear();

		//	list include signals that:
		//
		//  + const
		//	+ acquired
		//	+ discrete

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == true &&
				s->isAcquired() == true &&
				s->isDiscrete() == true)
			{
				m_acquiredDiscreteConstSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredDiscreteInputSignalsList()
	{
		m_nonAcquiredDiscreteInputSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	- acquired
		//	+ discrete
		//	+ input
		//	+ no matter used in UAL or not

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isInput() == true)
			{
				m_nonAcquiredDiscreteInputSignals.append(s);
			}
		}

		return true;
	}


	bool ModuleLogicCompiler::createNonAcquiredDiscreteStrictOutputSignalsList()
	{
		m_nonAcquiredDiscreteStrictOutputSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	+ non acquired
		//	+ discrete
		//	+ strict output
		//	+ used in UAL

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isStrictOutput() == true)
			{
				m_nonAcquiredDiscreteStrictOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredDiscreteInternalSignalsList()
	{
		m_nonAcquiredDiscreteInternalSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	+ non acquired
		//	+ discrete
		//	+ internal
		//  - enableTuning
		//  - bus child
		//	+ used in UAL

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isBusChild() == false &&
				s->isAcquired() == false &&
				s->isDiscrete() == true &&
				s->isInternal() == true &&
				s->isTuningable() == false)
			{
				m_nonAcquiredDiscreteInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogInputSignalsList()
	{
		m_acquiredAnalogInputSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	+ acquired
		//	+ analog
		//	+ input
		//	+ no matter used in UAL or not

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isInput() == true)
			{
				m_acquiredAnalogInputSignals.append(s);

				// if input signal is acquired, then validity signal (if exists) also always acquired
				//
//				appendLinkedValiditySignal(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogStrictOutputSignalsList()
	{
		m_acquiredAnalogStrictOutputSignals.clear();

		//	list include signals that:
		//
		//	- const
		//	+ acquired
		//	+ analog
		//	+ strict output
		//	+ used in UAL

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isStrictOutput() == true)
			{
				m_acquiredAnalogStrictOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogInternalSignalsList()
	{
		m_acquiredAnalogInternalSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	+ acquired
		//	+ analog
		//	+ internal
		//  - enableTuning
		//  - bus child
		//	+ used in UAL || is a SerialRx signal (condition: m_optoModuleStorage->isSerialRxSignalExists(m_lm->equipmentIdTemplate(), s->appSignalID()) == true))

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isBusChild() == false &&
				s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isInternal() == true &&
				s->isTuningable() == false)
			{
				m_acquiredAnalogInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogOptoSignalsList()
	{
		m_acquiredAnalogOptoSignals.clear();

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isOptoSignal() == true &&
				s->isBusChild() == false &&
				s->isConst() == false)
			{
				m_acquiredAnalogOptoSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogBusChildSignalsList()
	{
		m_acquiredAnalogBusChildSignals.clear();

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isBusChild() == true &&
				s->isConst() == false)
			{
				m_acquiredAnalogBusChildSignals.append(s);
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
		//	+ discrete
		//	+ internal
		//	+ tuningable
		//	+ no matter used in UAL or not

		QVector<Signal*> tuningSignals;

		m_tuningData->getAcquiredAnalogSignals(tuningSignals);

		// check signals!

		for(Signal* s : tuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (ualSignal->isAcquired() == true &&
				ualSignal->isAnalog() == true &&
				ualSignal->isTuningable() == true)
			{
				m_acquiredAnalogTuningSignals.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredAnalogConstSignalsList()
	{
		m_acquiredAnalogConstIntSignals.clear();
		m_acquiredAnalogConstFloatSignals.clear();

		bool result = true;

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isConst() == false || ualSignal->isAnalog() == false || ualSignal->isAcquired() == false)
			{
				continue;
			}

			switch(ualSignal->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				m_acquiredAnalogConstIntSignals.insertMulti(ualSignal->constAnalogIntValue(), ualSignal);
				continue;

			case E::AnalogAppSignalFormat::Float32:
				m_acquiredAnalogConstFloatSignals.insertMulti(ualSignal->constAnalogFloatValue(), ualSignal);
				continue;

			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createAnalogOutputSignalsToConversionList()
	{
		m_analogOutputSignalsToConversion.clear();

		//	list include signals that:
		//
		//	+ analog
		//	+ output

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAnalog() == true &&
				s->isOutput() == true)
			{
				m_analogOutputSignalsToConversion.append(s->getAnalogOutputSignals());
			}
		}

		return true;
	}


	bool ModuleLogicCompiler::createNonAcquiredAnalogInputSignalsList()
	{
		m_nonAcquiredAnalogInputSignals.clear();

		//	list include signals that:
		//
		//	- const
		//	+ non acquired
		//	+ analog
		//	+ input
		//	+ used in UAL

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isInput() == true)
			{
				m_nonAcquiredAnalogInputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredAnalogStrictOutputSignalsList()
	{
		m_nonAcquiredAnalogStrictOutputSignals.clear();

		//	list include signals that:
		//
		//	- const
		//	+ non acquired
		//	+ analog
		//	+ output
		//	+ used in UAL

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isStrictOutput() == true)
			{
				m_nonAcquiredAnalogStrictOutputSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredAnalogInternalSignalsList()
	{
		m_nonAcquiredAnalogInternalSignals.clear();

		//	list include signals that:
		//
		//  - const
		//	+ non acquired
		//	+ analog
		//	+ internal
		//  - enableTuning
		//	+ used in UAL
		//  - bus child
		//	+ auto analog internal signals (auto generated in m_appSignals)

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isBusChild() == false &&
				s->isAcquired() == false &&
				s->isAnalog() == true &&
				s->isInternal() == true)
			{
				m_nonAcquiredAnalogInternalSignals.append(s);
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
		//  - opto signal


		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isBus() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_acquiredBuses.append(ualSignal);
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

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == false &&
				ualSignal->isBus() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_nonAcquiredBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::groupTxSignals()
	{
		QList<Hardware::OptoPortShared> associatedPorts;

		m_optoModuleStorage->getLmAssociatedOptoPorts(m_lm->equipmentIdTemplate(), associatedPorts);

		QHash<UalSignal*, UalSignal*> acquiredInternalDiscretes;

		for(UalSignal* s : m_acquiredDiscreteInternalSignals)
		{
			acquiredInternalDiscretes.insert(s, s);
		}

		QHash<QString, QSet<UalSignal*>> portsTxSignalSets;

		for(Hardware::OptoPortShared port : associatedPorts)
		{
			QVector<Hardware::TxRxSignalShared> txSignals;

			port->getTxDiscreteSignals(txSignals, true);

			QSet<UalSignal*> set;

			for(Hardware::TxRxSignalShared txSignal : txSignals)
			{
				UalSignal* s = m_ualSignals.get(txSignal->appSignalID());

				if (s == nullptr)
				{
					assert(false);
					continue;
				}

				if (acquiredInternalDiscretes.contains(s) == false)
				{
					continue;
				}

				set.insert(s);
			}

			if (set.size() > 0)
			{
				portsTxSignalSets.insert(port->equipmentID(), set);
			}
		}

		//

		QStringList portIDs = portsTxSignalSets.keys();

		for(const QString& portID : portIDs)
		{
			QSet<UalSignal*>& set = portsTxSignalSets[portID];

			LOG_MESSAGE(m_log, QString("Port %1 acquired discrete internal txSignals count = %2").arg(portID).arg(set.count()));
		}

		QVector<QString>&& vPortIDs = QVector<QString>::fromList(portIDs);

		int count = vPortIDs.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				QString s1ID = vPortIDs[i];
				QString s2ID = vPortIDs[k];

				QSet<UalSignal*>& set1 = portsTxSignalSets[s1ID];
				QSet<UalSignal*>& set2 = portsTxSignalSets[s2ID];

				LOG_MESSAGE(m_log, QString("%1 intersect %2 = %3").
							arg(s1ID).arg(s2ID).arg(set1.intersect(set2).count()));
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::appendLinkedValiditySignal(const Signal* s)
	{
		TEST_PTR_RETURN_FALSE(s);

		// if signal has linked validity signal, append validity signal to m_acquiredDiscreteInputSignals also
		//
/*		QString linkedValiditySignalEquipmentID  = m_linkedValidtySignalsID.value(s->equipmentID(), QString());

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
		}*/

		return true;
	}

	bool ModuleLogicCompiler::listsUniquenessCheck() const
	{
		bool result = true;

		QHash<UalSignal*, UalSignal*> signalsMap;

		signalsMap.reserve(m_chassisSignals.count());

		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteInputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteStrictOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteOptoAndBusChildSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteTuningSignals);

		result &= listUniquenessCheck(signalsMap, m_nonAcquiredDiscreteStrictOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredDiscreteInternalSignals);

		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogInputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogStrictOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogOptoSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogBusChildSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredAnalogTuningSignals);

		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogInputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogStrictOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredAnalogInternalSignals);

		result &= listUniquenessCheck(signalsMap, m_acquiredBuses);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredBuses);

		return result;
	}

	bool ModuleLogicCompiler::listUniquenessCheck(QHash<UalSignal *, UalSignal *> &signalsMap, const QVector<UalSignal *> &signalList) const
	{
		bool result = true;

		for(UalSignal* s : signalList)
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

	void ModuleLogicCompiler::sortSignalList(QVector<UalSignal*>& signalList)
	{
		int count = signalList.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				UalSignal* s1 = signalList[i];
				UalSignal* s2 = signalList[k];

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

			if (setDiscreteInputSignalsUalAddresses() == false) break;

			if (setTuningableSignalsUalAddresses() == false) break;

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
		bool result = true;

		for(Signal* s : m_ioSignals)
		{
			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
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
					assert(false);							// output diagnostics signals is not exist
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

	bool ModuleLogicCompiler::setTuningableSignalsUalAddresses()
	{
		if (m_tuningData == nullptr)
		{
			return true;			// no tuning data, it is ok
		}

		bool result = true;

		QVector<Signal*> tunigableSignals;

		m_tuningData->getSignals(tunigableSignals);

		for(Signal* s : tunigableSignals)
		{
			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (s->ioBufAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal != nullptr)
			{
				ualSignal->setUalAddr(s->ioBufAddr());
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::setDiscreteInputSignalsUalAddresses()
	{
		bool result = true;

		// set ualAddress of discrete input UalSignals reffered by m_ioSignals equal to ioBufAddr of input signal
		//
		for(Signal* ioSignal : m_ioSignals)
		{
			if (ioSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ioSignal->isInput() == false ||
				ioSignal->isDiscrete() == false)
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(ioSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				continue;			// is not an error
			}

			if (ualSignal->isInput() == false ||
				ualSignal->isDiscrete() == false)
			{
				assert(false);					// ualSignal must be Discrete Input if reffered by ioSignal
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			ualSignal->setUalAddr(ioSignal->ioBufAddr());
		}

		return result;
	}

	bool ModuleLogicCompiler::disposeDiscreteSignalsInBitMemory()
	{
		bool result = true;

		result &= m_memoryMap.appendAcquiredDiscreteStrictOutputSignals(m_acquiredDiscreteStrictOutputSignals);
		result &= m_memoryMap.appendAcquiredDiscreteInternalSignals(m_acquiredDiscreteInternalSignals);
		result &= m_memoryMap.appendNonAcquiredDiscreteStrictOutputSignals(m_nonAcquiredDiscreteStrictOutputSignals);
		result &= m_memoryMap.appendNonAcquiredDiscreteInternalSignals(m_nonAcquiredDiscreteInternalSignals);

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

		result &= m_memoryMap.appendAcquiredAnalogInputSignalsInRegBuf(m_acquiredAnalogInputSignals);
		result &= m_memoryMap.appendAcquiredAnalogStrictOutputSignalsInRegBuf(m_acquiredAnalogStrictOutputSignals);
		result &= m_memoryMap.appendAcquiredAnalogInternalSignalsInRegBuf(m_acquiredAnalogInternalSignals);
		result &= m_memoryMap.appendAcquiredAnalogOptoSignalsInRegBuf(m_acquiredAnalogOptoSignals);
		result &= m_memoryMap.appendAcquiredAnalogBusChildSignalsInRegBuf(m_acquiredAnalogBusChildSignals);
		result &= m_memoryMap.appendAcquiredAnalogTuningSignalsInRegBuf(m_acquiredAnalogTuningSignals);
		result &= m_memoryMap.appendAcquiredAnalogConstSignalsInRegBuf(m_acquiredAnalogConstIntSignals,
																	   m_acquiredAnalogConstFloatSignals);
		return result;
	}

	bool ModuleLogicCompiler::disposeAcquiredBusesInRegBuf()
	{
		bool result = true;

		result &= m_memoryMap.appendAcquiredBussesInRegBuf(m_acquiredBuses);

		return result;
	}

	bool ModuleLogicCompiler::disposeAcquiredDiscreteSignalsInRegBuf()
	{
		bool result = true;

		result &= m_memoryMap.appendAcquiredDiscreteInputSignalsInRegBuf(m_acquiredDiscreteInputSignals);
		result &= m_memoryMap.appendAcquiredDiscreteStrictOutputSignalsInRegBuf(m_acquiredDiscreteStrictOutputSignals);
		result &= m_memoryMap.appendAcquiredDiscreteInternalSignalsInRegBuf(m_acquiredDiscreteInternalSignals);
		result &= m_memoryMap.appendAcquiredDiscreteOptoAndBusChildSignalsInRegBuf(m_acquiredDiscreteOptoAndBusChildSignals);
		result &= m_memoryMap.appendAcquiredDiscreteTuningSignalsInRegBuf(m_acquiredDiscreteTuningSignals);
		result &= m_memoryMap.appendAcquiredDiscreteConstSignalsInRegBuf(m_acquiredDiscreteConstSignals);

		return result;
	}

	bool ModuleLogicCompiler::disposeNonAcquiredAnalogSignals()
	{
		bool result = true;

		result &= m_memoryMap.appendNonAcquiredAnalogInputSignals(m_nonAcquiredAnalogInputSignals);
		result &= m_memoryMap.appendNonAcquiredAnalogStrictOutputSignals(m_nonAcquiredAnalogStrictOutputSignals);
		result &= m_memoryMap.appendNonAcquiredAnalogInternalSignals(m_nonAcquiredAnalogInternalSignals);

		return result;
	}

	bool ModuleLogicCompiler::disposeNonAcquiredBuses()
	{
		bool result = true;

		result &= m_memoryMap.appendNonAcquiredBusses(m_nonAcquiredBuses);

		return result;
	}

	bool ModuleLogicCompiler::appendAfbsForAnalogInOutSignalsConversion()
	{
		if (findFbsForAnalogInOutSignalsConversion() == false)
		{
			return false;
		}

		if (m_lm->isBvb() == true)
		{
			return true;
		}

		bool result = true;

		// append FBs  for analog input signals conversion
		//
		QVector<UalSignal*> analogInputSignals;

		analogInputSignals.append(m_acquiredAnalogInputSignals);
		analogInputSignals.append(m_nonAcquiredAnalogInputSignals);

		for(UalSignal* ualSignal : analogInputSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			assert(ualSignal->isAnalog() == true);
			assert(ualSignal->isInput() == true);

			Signal* s = ualSignal->getInputSignal();

			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			UalItem appItem;

			bool needConversion = false;

			bool res = createAfbForAnalogInputSignalConversion(*s, &appItem, &needConversion);

			if (res == false)
			{
				result = false;
				continue;
			}

			s->setNeedConversion(needConversion);

			if (needConversion == true)
			{
				UalAfb* appFb = createUalAfb(appItem);

				m_inOutSignalsToScalAppFbMap.insert(s->appSignalID(), appFb);
			}
		}

		// append FBs  for analog output signals conversion
		//
		for(Signal* s : m_analogOutputSignalsToConversion)
		{
			TEST_PTR_CONTINUE(s);

			assert(s->isAnalog() == true);
			assert(s->isOutput() == true);

			UalItem appItem;

			bool needConversion = false;

			bool res = createFbForAnalogOutputSignalConversion(*s, &appItem, &needConversion);

			if (res == false)
			{
				result = false;
				continue;
			}

			s->setNeedConversion(needConversion);

			if (needConversion == true)
			{
				UalAfb* appFb = createUalAfb(appItem);

				m_inOutSignalsToScalAppFbMap.insert(s->appSignalID(), appFb);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::findFbsForAnalogInOutSignalsConversion()
	{
		if (m_lm->isBvb() == true)
		{
			return true;
		}

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

	bool ModuleLogicCompiler::createAfbForAnalogInputSignalConversion(const Signal& signal, UalItem* appItem, bool* needConversion)
	{
		if (appItem == nullptr || needConversion == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

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
			*needConversion = false;
			return true;
		}

		*needConversion = true;

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

				result = appItem->init(fb.pointer, errorMsg);

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

				result = appItem->init(fb.pointer, errorMsg);

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

	bool ModuleLogicCompiler::createFbForAnalogOutputSignalConversion(const Signal& signal, UalItem* appItem, bool* needConversion)
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
			*needConversion = false;
			return true;
		}

		*needConversion = true;

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

				result = appItem->init(fb.pointer, errorMsg);

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

				result = appItem->init(fb.pointer, errorMsg);

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

	UalAfb* ModuleLogicCompiler::createUalAfb(const UalItem& appItem)
	{
		if (appItem.isAfb() == false)
		{
			return nullptr;
		}

		UalAfb* appFb = new UalAfb(appItem);

		if (appFb->calculateFbParamValues(this) == false)
		{
			delete appFb;
			return nullptr;
		}

		// get Functional Block instance
		//
		bool result = m_afbls.addInstance(appFb, m_log);

		if (result == false)
		{
			delete appFb;
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("FB '%1' instantiation error")).arg(appItem.caption()));
			return nullptr;
		}

		m_ualAfbs.insert(appFb);

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

			UalSignal* appSignal = m_ualSignals.get(s->appSignalIds());

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

	bool ModuleLogicCompiler::processSinglePortRxSignals()
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
			if (processSinglePortReceivers() == false) break;

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
		for(const UalItem* item : m_ualItems)
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

	bool ModuleLogicCompiler::processTransmitter(const UalItem* ualItem)
	{
		TEST_PTR_RETURN_FALSE(ualItem);

		assert(ualItem->isTransmitter() == true);

		const LogicTransmitter& transmitter = ualItem->logicTransmitter();

		bool result = true;

		QVector<QPair<QString, UalSignal*>> connectedSignals;

		if (getConnectedSignals(ualItem, &connectedSignals) == false)
		{
			return false;
		}

		bool signalAlreadyInTxList = false;

		for(const QPair<QString, UalSignal*>& connectedSignal : connectedSignals)
		{
			result &= m_optoModuleStorage->appendTxSignal(ualItem->schemaID(), transmitter.connectionId(), transmitter.guid(),
													   m_lm->equipmentIdTemplate(),
													   connectedSignal.first,
													   connectedSignal.second,
													   &signalAlreadyInTxList);
		}

		return result;
	}

	bool ModuleLogicCompiler::getConnectedSignals(const UalItem* transmitterItem, QVector<QPair<QString, UalSignal*>>* connectedSignals)
	{
		if (transmitterItem == nullptr || connectedSignals == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		connectedSignals->clear();

		const std::vector<LogicPin>& inPins = transmitterItem->inputs();

		bool result = true;

		for(const LogicPin& inPin : inPins)
		{
			if (inPin.IsInput() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			UalSignal* ualSignal = m_ualSignals.get(inPin.guid());

			if (ualSignal == nullptr)
			{
				// UalSignal is not found for pin '%1' (Logic schema '%2').
				//
				m_log->errALC5120(transmitterItem->guid(), transmitterItem->label(), inPin.caption(), transmitterItem->schemaID());
				return false;
			}

			QStringList ids;

			ualSignal->refSignalIDs(&ids);

			bool find = false;

			for(const QString& id : ids)
			{
				Signal* s = m_signals->getSignal(id);

				if (s != nullptr && m_chassisSignals.contains(s->appSignalID()) == true)
				{
					find = true;
					break;
				}
			}

			if (find == false)
			{
				// Input %1 of transmitter is connected unnamed signal (Logic schema %2).
				//
				m_log->errALC5125(inPin.caption(), transmitterItem->guid(), transmitterItem->schemaID());
				return false;
			}

			QString nearestSignalID;

			result &= getNearestSignalID(inPin, &nearestSignalID);

			connectedSignals->append(QPair<QString, UalSignal*>(nearestSignalID, ualSignal));
		}

		return result;
	}

	bool ModuleLogicCompiler::getNearestSignalID(const LogicPin& inPin, QString* nearestSignalID)
	{
		if (nearestSignalID == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		nearestSignalID->clear();

		const std::vector<QUuid>& accosiatedOutputsUuids = inPin.associatedIOs();

		for(QUuid outPinUuid : accosiatedOutputsUuids)
		{
			UalItem* ualItem = m_pinParent.value(outPinUuid, nullptr);

			if (ualItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (ualItem->isSignal() == true)
			{
				*nearestSignalID = ualItem->strID();
				return true;
			}
		}

		if (accosiatedOutputsUuids.size() != 1)
		{
			return true;
		}

		QUuid outPinUuid = accosiatedOutputsUuids[0];

		UalItem* ualItem = m_pinParent.value(outPinUuid, nullptr);

		if (ualItem == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const LogicPin* outPin = ualItem->getPin(outPinUuid);

		if (outPin == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<QUuid> associatedInputs = outPin->associatedIOs();

		for(QUuid inPinUuid : associatedInputs)
		{
			UalItem* ualItem = m_pinParent.value(inPinUuid, nullptr);

			if (ualItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (ualItem->isSignal() == true)
			{
				*nearestSignalID = ualItem->strID();
				return true;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::processSinglePortReceivers()
	{
		bool result = true;

		// process receivers and add regular tx signals to Serial (only!) ports rxSignals lists
		//
		for(const UalItem* item : m_ualItems)
		{
			if (item == nullptr)
			{
				result = false;
				ASSERT_RESULT_FALSE_BREAK
			}

			result &= processSinglePortReceiver(item);
		}

		return result;
	}

	bool ModuleLogicCompiler::processSinglePortReceiver(const UalItem* item)
	{
		TEST_PTR_RETURN_FALSE(item);

		if (item->isReceiver() == false)
		{
			return true;				// item is not receiver, nothing to processing
		}

		const UalReceiver& receiver = item->logicReceiver();

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

		UalSignal* rxSignal = m_ualSignals.get(rxSignalID);

		if (rxSignal == nullptr)
		{
			m_log->errALC5000(rxSignalID, item->guid(), item->schemaID());
			return false;
		}

		bool result = m_optoModuleStorage->appendSinglePortRxSignal(item->schemaID(),
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
						UalSignal* appSignal = m_ualSignals.get(item.appSignalID);

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

	bool ModuleLogicCompiler::setOptoUalSignalsAddresses()
	{
		bool result = true;

		for(UalSignal* ualSignal : m_ualSignals)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->isOptoSignal() == false || ualSignal->isBusChild() == true)
			{
				continue;
			}

			const UalItem* ualItem = ualSignal->ualItem();

			if (ualItem == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			const UalReceiver* ualReceiver = ualItem->ualReceiver();

			if (ualReceiver == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			SignalAddress16 rxAddress;

			bool res = m_optoModuleStorage->getRxSignalAbsAddress(ualItem->schemaID(),
													   ualReceiver->connectionId(),
													   ualReceiver->appSignalId(),
													   m_lm->equipmentIdTemplate(),
													   ualReceiver->guid(),
													   &rxAddress);
			if (res == false)
			{
				result = false;
				continue;
			}

			ualSignal->setUalAddr(rxAddress);
		}

		return result;
	}

	bool ModuleLogicCompiler::initAfbs()
	{
		m_code.init(&m_resourcesUsageInfo.initAfbs);

		Command cmd;

		// first command in program!
		//
		cmd.appStart(0);		// real address is set in startAppLogicCode function
		cmd.setComment(tr("set address of application logic code start"));

		m_code.append(cmd);
		m_code.newLine();

		LOG_MESSAGE(m_log, QString(tr("Generation of AFB initialization code...")));

		bool result = true;

		m_code.comment("FB's initialization code");
		m_code.newLine();

		QHash<QString, int> instantiatorStrIDsMap;

		for(Afbl* afbl : m_afbls)
		{
			for(UalAfb* ualAfb : m_ualAfbs)
			{
				if (ualAfb->afbStrID() != afbl->strID())
				{
					continue;
				}

				if (ualAfb->hasRam() == true)
				{
					// initialize all params for each instance of FB with RAM
					//
					result &= initAppFbParams(ualAfb, false);
				}
				else
				{
					// FB without RAM initialize once for all instances
					// initialize instantiator params only
					//
					QString instantiatorID = ualAfb->instantiatorID();

					if (instantiatorStrIDsMap.contains(instantiatorID) == false)
					{
						instantiatorStrIDsMap.insert(instantiatorID, 0);

						result &= initAppFbParams(ualAfb, true);
					}
				}
			}
		}

		cmd.stop();

		m_code.comment(tr("End of FB's initialization code section"));
		m_code.newLine();
		m_code.append(cmd);
		m_code.newLine();

		// set APPSTART command to current address
		//
		cmd.appStart(m_code.commandAddress());
		cmd.clearComment();

		m_code.replaceAt(0, cmd);

		m_code.calculate(&m_resourcesUsageInfo.initAfbs);

		return result;
	}

	bool ModuleLogicCompiler::initAppFbParams(UalAfb* appFb, bool /* instantiatorsOnly */)
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


	bool ModuleLogicCompiler::displayAfbParams(const UalAfb& appFb)
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
		m_code.comment(tr("Start of application logic code"));
		m_code.newLine();

		Command cmd;

		cmd.movBitConst(m_memoryMap.constBit0Addr(), 0);
		cmd.setComment("init const bit 0");
		m_code.append(cmd);

		cmd.movBitConst(m_memoryMap.constBit1Addr(), 1);
		cmd.setComment("init const bit 1");
		m_code.append(cmd);

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

		QVector<UalSignal*> analogInputSignals;

		analogInputSignals.append(m_acquiredAnalogInputSignals);
		analogInputSignals.append(m_nonAcquiredAnalogInputSignals);

		if (analogInputSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.init(&m_resourcesUsageInfo.convertAnalogInputSignals);

		m_code.comment("Convertion of analog input signals");
		m_code.newLine();

		Command cmd;

		for(UalSignal* ualSignal : analogInputSignals)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Signal* s = ualSignal->getInputSignal();

			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
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

			UalAfb* appFb = m_inOutSignalsToScalAppFbMap.value(s->appSignalID(), nullptr);

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

		m_code.calculate(&m_resourcesUsageInfo.convertAnalogInputSignals);

		return result;
	}

	bool ModuleLogicCompiler::generateAppLogicCode()
	{
		LOG_MESSAGE(m_log, QString("Generation of application logic code was started..."));

		bool result = true;

		m_code.init(&m_resourcesUsageInfo.appLogicCode);

		m_code.comment("Application logic code");
		m_code.newLine();

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_RETURN_FALSE(ualItem)

			switch(ualItem->type())
			{
			case UalItem::Type::Afb:
				result &= generateAfbCode(ualItem);
				break;

			case UalItem::Type::BusComposer:
				result &= generateBusComposerCode(ualItem);
				break;

			// UalItems that is not required code generation
			//
			case UalItem::Type::Signal:
			case UalItem::Type::Const:
			case UalItem::Type::Transmitter:
			case UalItem::Type::Receiver:
			case UalItem::Type::Terminator:
			case UalItem::Type::BusExtractor:
			case UalItem::Type::LoopbackOutput:
			case UalItem::Type::LoopbackInput:
				break;

			case UalItem::Type::Unknown:
			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		m_code.calculate(&m_resourcesUsageInfo.appLogicCode);

		return result;
	}

	bool ModuleLogicCompiler::generateAfbCode(const UalItem* ualItem)
	{
		if (m_ualAfbs.contains(ualItem->guid()) == false)
		{
			ASSERT_RETURN_FALSE
		}

		const UalAfb* ualAfb = m_ualAfbs.value(ualItem->guid(), nullptr);

		TEST_PTR_RETURN_FALSE(ualAfb)

		bool result = true;

		int busProcessingStepsNumber = 1;

		result = calcBusProcessingStepsNumber(ualAfb, &busProcessingStepsNumber);

		if (result == false)
		{
			return false;
		}

		if (busProcessingStepsNumber > 1 && ualAfb->hasRam() == true)
		{
			assert(false);				// AFB's with ram is not allow multistep processing now
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		for(int busProcessingStep = 0; busProcessingStep < busProcessingStepsNumber; busProcessingStep++)
		{
			result &= generateSignalsToAfbInputsCode(ualAfb, busProcessingStep);

			result &= startAfb(ualAfb, busProcessingStep + 1, busProcessingStepsNumber);

			result &= generateAfbOutputsToSignalsCode(ualAfb, busProcessingStep);
		}

		m_code.newLine();

		return result;
	}

	bool ModuleLogicCompiler::generateSignalsToAfbInputsCode(const UalAfb* ualAfb, int busProcessingStep)
	{
		bool result = true;

		for(const LogicPin& inPin : ualAfb->inputs())
		{
			if (inPin.IsInput() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			UalSignal* inUalSignal = m_ualSignals.get(inPin.guid());

			if (inUalSignal == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			LogicAfbSignal inAfbSignal;

			bool res = ualAfb->getAfbSignalByPin(inPin, &inAfbSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = res;
				continue;
			}

			result &= generateSignalToAfbInputCode(ualAfb, inAfbSignal, inUalSignal, busProcessingStep);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateSignalToAfbInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal, int busProcessingStep)
	{
		if (ualAfb == nullptr || inUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (inUalSignal->isCompatible(inAfbSignal) == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inUalSignal->ualItemGuid(), inUalSignal->ualItemLabel(), ualAfb->schemaID());
			return false;
		}

		// inUalSignal and inAfbSignal are compatible

		if (inUalSignal->isConst() == false && inUalSignal->ualAddr().isValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(), inUalSignal->ualItemSchemaID());
			return false;
		}

		bool result = true;

		int afbOpcode = ualAfb->opcode();
		int afbInstance = ualAfb->instance();
		int afbSignalIndex = inAfbSignal.operandIndex();

		QString afbCaption = ualAfb->caption();
		QString signalCaption = inAfbSignal.caption();

		Command cmd;

		switch(inAfbSignal.type())
		{
		case E::SignalType::Discrete:
			if (inUalSignal->isConst() == true)
			{
				cmd.writeFuncBlockConst(afbOpcode, afbInstance, afbSignalIndex, inUalSignal->constDiscreteValue(), afbCaption);
				cmd.setComment(QString("%1.%2 <= #%3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->constDiscreteValue()));
			}
			else
			{
				cmd.writeFuncBlockBit(afbOpcode, afbInstance, afbSignalIndex, inUalSignal->ualAddr(), afbCaption);
				cmd.setComment(QString("%1.%2 <= %3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->appSignalID()));
			}

			m_code.append(cmd);

			break;

		case E::SignalType::Analog:
			if (inUalSignal->isConst() == true)
			{
				switch(inUalSignal->constAnalogFormat())
				{
				case  E::AnalogAppSignalFormat::Float32:
					cmd.writeFuncBlockConstFloat(afbOpcode, afbInstance, afbSignalIndex, inUalSignal->constAnalogFloatValue(), afbCaption);
					cmd.setComment(QString("%1.%2 <= #%3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->constAnalogFloatValue()));
					break;

				case  E::AnalogAppSignalFormat::SignedInt32:
					cmd.writeFuncBlockConstInt32(afbOpcode, afbInstance, afbSignalIndex, inUalSignal->constAnalogIntValue(), afbCaption);
					cmd.setComment(QString("%1.%2 <= #%3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->constAnalogIntValue()));
					break;

				default:
					assert(false);
				}
			}
			else
			{
				cmd.writeFuncBlock32(afbOpcode, afbInstance, afbSignalIndex, inUalSignal->ualAddr(), afbCaption);
				cmd.setComment(QString("%1.%2 <= %3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->appSignalID()));
			}

			m_code.append(cmd);

			break;

		case E::SignalType::Bus:
			result = generateSignalToAfbBusInputCode(ualAfb, inAfbSignal, inUalSignal, busProcessingStep);
			break;

		default:
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateSignalToAfbBusInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal, int busProcessingStep)
	{
		if (ualAfb == nullptr || inUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		assert(inAfbSignal.isBus() == true);
		assert(busProcessingStep >= 0);

		bool result = true;

		switch(inUalSignal->signalType())
		{
		case E::SignalType::Discrete:

			result =  generateDiscreteSignalToAfbBusInputCode(ualAfb, inAfbSignal, inUalSignal);
			break;

		case E::SignalType::Bus:
			result =  generateBusSignalToAfbBusInputCode(ualAfb, inAfbSignal, inUalSignal, busProcessingStep);

			break;

		case E::SignalType::Analog:
			LOG_INTERNAL_ERROR(m_log);			// uncompatible conection, this error must be detected early!
			return false;

		default:
			assert(false);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToAfbBusInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal)
	{
		if (ualAfb == nullptr || inUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (inUalSignal->isDiscrete() == false || inAfbSignal.isBus() == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (inUalSignal->isConst() == false && inUalSignal->ualAddr().isValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(), inUalSignal->ualItemSchemaID());
			return false;
		}

		if (inAfbSignal.busDataFormat() != E::BusDataFormat::Discrete)
		{
			// Discrete signal %1 is connected to non-discrete bus input (Logic schema %2)
			//
			m_log->errALC5124(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(),
							  ualAfb->guid(), ualAfb->schemaID());	// Discrete signal %1 is connected to non-discrete bus input (Logic schema %2)

			return false;
		}

		int inputSize = inAfbSignal.size();

		if (inputSize != SIZE_16BIT && inputSize != SIZE_32BIT)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Command cmd;

		int wordAccAddr = m_memoryMap.wordAccumulatorAddress();

		if (inUalSignal->isConst() == true)
		{
			switch(inUalSignal->constDiscreteValue())
			{
			case 0:
				cmd.fill(Address16(wordAccAddr, 0), m_memoryMap.constBit0Addr());
				break;

			case 1:
				cmd.fill(Address16(wordAccAddr, 0), m_memoryMap.constBit1Addr());
				break;

			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}
		else
		{
			cmd.fill(Address16(wordAccAddr, 0), inUalSignal->ualAddr());
		}

		m_code.append(cmd);

		if (inputSize == SIZE_16BIT)
		{
			cmd.writeFuncBlock(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), wordAccAddr, ualAfb->caption());
		}
		else
		{
			cmd.mov(wordAccAddr + 1, wordAccAddr);
			m_code.append(cmd);

			assert(inputSize == SIZE_32BIT);
			cmd.writeFuncBlock32(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), wordAccAddr, ualAfb->caption());
		}

		cmd.setComment(QString("%1.%2 << %3").arg(ualAfb->caption()).arg(inAfbSignal.caption()).arg(inUalSignal->refSignalIDsJoined()));

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateBusSignalToAfbBusInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal, int busProcessingStep)
	{
		if (ualAfb == nullptr || inUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (inUalSignal->isBus() == false || inAfbSignal.isBus() == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (inAfbSignal.busDataFormat() != E::BusDataFormat::Discrete)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// unknown BusDataFormat
			return false;
		}

		if (inUalSignal->ualAddr().isValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(), inUalSignal->ualItemSchemaID());
			return false;
		}

		assert(inUalSignal->ualAddr().bit() == 0);

		int addrFrom = inUalSignal->ualAddr().offset();

		Command cmd;

		int inputSize = inAfbSignal.size();

		switch(inputSize)
		{
		case SIZE_16BIT:

			addrFrom += busProcessingStep * 1;			//	+1 word per busProcessingStep
			cmd.writeFuncBlock(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), addrFrom, ualAfb->caption());
			break;

		case SIZE_32BIT:

			addrFrom += busProcessingStep * 2;			//	+2 words per busProcessingStep
			cmd.writeFuncBlock32(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), addrFrom, ualAfb->caption());
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		cmd.setComment(QString("%1.%2 << %3 (part %4)").
					   arg(ualAfb->caption()).arg(inAfbSignal.caption()).
					   arg(inUalSignal->refSignalIDsJoined()).arg(busProcessingStep + 1));

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::startAfb(const UalAfb* ualAfb, int processingStep, int processingStepsNumber)
	{
		Command cmd;

		cmd.start(ualAfb->opcode(), ualAfb->instance(), ualAfb->caption(), ualAfb->runTime());

		if (processingStepsNumber == 1)
		{
			cmd.setComment(QString(tr("compute %1 @%2")).arg(ualAfb->caption()).arg(ualAfb->label()));
		}
		else
		{
			cmd.setComment(QString(tr("compute %1 @%2 (step %3/%4)")).
							arg(ualAfb->caption()).arg(ualAfb->label()).arg(processingStep).arg(processingStepsNumber));
		}

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateAfbOutputsToSignalsCode(const UalAfb* ualAfb, int busProcessingStep)
	{
		if (ualAfb == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		bool result = true;

		for(const LogicPin& outPin : ualAfb->outputs())
		{
			if (outPin.IsOutput() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (isConnectedToTerminator(outPin) == true)
			{
				continue;
			}

			UalSignal* outUalSignal = m_ualSignals.get(outPin.guid());

			if (outUalSignal == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			LogicAfbSignal outAfbSignal;

			bool res = ualAfb->getAfbSignalByPin(outPin, &outAfbSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = res;
				continue;
			}

			result &= generateAfbOutputToSignalCode(ualAfb, outAfbSignal, outUalSignal, busProcessingStep);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAfbOutputToSignalCode(const UalAfb* ualAfb, const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal, int busProcessingStep)
	{
		if (ualAfb == nullptr || outUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (outUalSignal->isConst() == true)
		{
			assert(false);							// can't assign value to const ual signal
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		if (outUalSignal->isCompatible(outAfbSignal) == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outUalSignal->ualItemGuid(), outUalSignal->appSignalID(), ualAfb->schemaID());
			return false;
		}

		if (outUalSignal->ualAddr().isValid() == false)
		{
			LOG_UNDEFINED_UAL_ADDRESS(m_log, outUalSignal);
			return false;
		}

		// inUalSignal and inAfbSignal are compatible

		bool result = true;

		int afbOpcode = ualAfb->opcode();
		int afbInstance = ualAfb->instance();
		int afbSignalIndex = outAfbSignal.operandIndex();

		QString afbCaption = ualAfb->caption();
		QString signalCaption = outAfbSignal.caption();

		Command cmd;

		switch(outAfbSignal.type())
		{
		case E::SignalType::Discrete:

			cmd.readFuncBlockBit(outUalSignal->ualAddr(), afbOpcode, afbInstance, afbSignalIndex, afbCaption);
			cmd.setComment(QString("%1 <= %2.%3").arg(outUalSignal->appSignalID()).arg(afbCaption).arg(signalCaption));

			m_code.append(cmd);

			break;

		case E::SignalType::Analog:

			cmd.readFuncBlock32(outUalSignal->ualAddr(), afbOpcode, afbInstance, afbSignalIndex, afbCaption);
			cmd.setComment(QString("%1 <= %2.%3").arg(outUalSignal->appSignalID()).arg(afbCaption).arg(signalCaption));

			m_code.append(cmd);

			break;

		case E::SignalType::Bus:

			result = generateAfbBusOutputToBusSignalCode(ualAfb, outAfbSignal, outUalSignal, busProcessingStep);
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAfbBusOutputToBusSignalCode(const UalAfb* ualAfb, const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal, int busProcessingStep)
	{
		if (ualAfb == nullptr || outUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (outUalSignal->isBus() == false || outAfbSignal.isBus() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (outAfbSignal.busDataFormat() != E::BusDataFormat::Discrete)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// unknown BusDataFormat
			return false;
		}

		if (outUalSignal->ualAddr().isValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(outUalSignal->refSignalIDsJoined(), outUalSignal->ualItemGuid(), outUalSignal->ualItemSchemaID());
			return false;
		}

		assert(outUalSignal->ualAddr().bit() == 0);

		int addrTo = outUalSignal->ualAddr().offset();

		Command cmd;

		int outputSize = outAfbSignal.size();

		switch(outputSize)
		{
		case SIZE_16BIT:

			addrTo += busProcessingStep * 1;			//	+1 word per busProcessingStep
			cmd.readFuncBlock(addrTo, ualAfb->opcode(), ualAfb->instance(), outAfbSignal.operandIndex(), ualAfb->caption());
			break;

		case SIZE_32BIT:

			addrTo += busProcessingStep * 2;			//	+2 words per busProcessingStep
			cmd.readFuncBlock32(addrTo, ualAfb->opcode(), ualAfb->instance(), outAfbSignal.operandIndex(), ualAfb->caption());
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		cmd.setComment(QString("%1 (part %2) << %3.%4").
					   arg(outUalSignal->refSignalIDsJoined()).arg(busProcessingStep + 1).
					   arg(ualAfb->caption()).arg(outAfbSignal.caption()));

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::calcBusProcessingStepsNumber(const UalAfb* ualAfb, int* busProcessingStepsNumber)
	{
		if (ualAfb == nullptr || busProcessingStepsNumber == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		*busProcessingStepsNumber = 0;

		bool isBusProcAfb = false;

		bool result = isBusProcessingAfb(ualAfb, &isBusProcAfb);

		if (result == false)
		{
			return false;
		}

		if (isBusProcAfb == false)
		{
			*busProcessingStepsNumber = 1;
			return true;
		}

		int inputsBusSize = -1;
		int inputSignalSize = -1;

		result = getPinsAndSignalsBusSizes(ualAfb, ualAfb->inputs(), &inputsBusSize, &inputSignalSize, true);

		if (result == false)
		{
			return false;
		}

		int outputsBusSize = -1;
		int outputSignalSize = -1;

		result = getPinsAndSignalsBusSizes(ualAfb, ualAfb->outputs(), &outputsBusSize, &outputSignalSize, false);

		if (result == false)
		{
			return false;
		}

		if (outputSignalSize == -1)
		{
			outputSignalSize = inputSignalSize;		// may be all bus outputs are connected to terminator
		}

		if (inputsBusSize != outputsBusSize)
		{
			assert(false);						// in and out busses has different sizes
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (inputSignalSize != outputSignalSize)
		{
			assert(false);						// input and output signals has different sizes
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (((inputsBusSize % SIZE_16BIT) != 0) ||
			((outputsBusSize % SIZE_16BIT) != 0) ||
			((inputSignalSize % SIZE_16BIT) != 0) ||
			((outputSignalSize % SIZE_16BIT) != 0))
		{
			assert(false);						// pins or signals size are not multiple 16
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (inputSignalSize < inputsBusSize)
		{
			assert(false);						// is not allowed now
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if ((inputSignalSize % inputsBusSize) != 0 ||
			(outputSignalSize % outputsBusSize) != 0)
		{
			// Signal and bus inputs sizes are not multiples (Logic schema %1).
			//
			m_log->errALC5126(ualAfb->guid(), ualAfb->schemaID());
			return false;
		}

		*busProcessingStepsNumber = inputSignalSize / inputsBusSize;

		return true;
	}

	bool ModuleLogicCompiler::getPinsAndSignalsBusSizes(const UalAfb* ualAfb, const std::vector<LogicPin>& pins, int* pinsSize, int* signalsSize, bool isInputs)
	{
		if (ualAfb == nullptr || pinsSize == nullptr || signalsSize == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		*pinsSize = -1;
		*signalsSize = -1;

		for(const LogicPin& pin : pins)
		{
			LogicAfbSignal afbSignal;

			bool result = ualAfb->getAfbSignalByPin(pin, &afbSignal);

			if (result == false)
			{
				return false;
			}

			if (afbSignal.isBus() == false)
			{
				continue;
			}

			if (*pinsSize == -1)
			{
				*pinsSize = afbSignal.size();
			}
			else
			{
				if (*pinsSize != afbSignal.size())
				{
					assert(false);				// different bus input sizes
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}
			}

			UalSignal* ualSignal = m_ualSignals.get(pin.guid());

			if (ualSignal == nullptr)
			{
				if (isInputs == false)
				{
					continue;					// output can don't have connected signals (can be connected to terminator)
				}

				assert(false);					// connected signal is not found
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (isInputs == true && ualSignal->isDiscrete() == true)
			{
				continue;
			}

			if (ualSignal->isBus() == false)
			{
				assert(false);					// connected signal is not bus
												// this error must be detected early
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (*signalsSize == -1)
			{
				*signalsSize = ualSignal->dataSize();
			}
			else
			{
				if (*signalsSize != ualSignal->dataSize())
				{
					// Different busTypes on AFB inputs (Logic schema %1).
					m_log->errALC5123(ualAfb->guid(), ualAfb->schemaID());
					return false;
				}
			}
		}

		if (*pinsSize == -1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		return true;
	}

	bool ModuleLogicCompiler::isBusProcessingAfb(const UalAfb* ualAfb, bool* isBusProcessing)
	{
		if (ualAfb == nullptr || isBusProcessing == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		Afbl* afbl = m_afbls.value(ualAfb->strID());

		if (afbl == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		*isBusProcessing = afbl->isBusProcessingAfb();

		return true;
	}

	bool ModuleLogicCompiler::generateBusComposerCode(const UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (ualItem->isBusComposer() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool connectedToTerminatorOnly = false;

		UalSignal* ualBusSignal = getBusComposerBusSignal(ualItem, &connectedToTerminatorOnly);

		if (connectedToTerminatorOnly == true)
		{
			// no busComposer code generation required
			//
			return true;
		}

		if (ualBusSignal == nullptr)
		{
			return false;
		}

		BusShared bus = ualBusSignal->bus();

		if (bus == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		bool result = true;

		m_code.comment(QString("BusComposer %1 processing").arg(ualItem->label()));
		m_code.newLine();

		int count = 0;

		m_code.append(codeSetMemory(ualBusSignal->ualAddr().offset(), 0, bus->sizeW(), QString("init %1").arg(ualBusSignal->appSignalID())));

		for(const BusSignal& busSignal : bus->busSignals())
		{
			UalSignal* inputSignal = getUalSignalByPinCaption(ualItem, busSignal.signalID, true);

			UalSignal* busChildSignal = ualBusSignal->getBusChildSignal(busSignal.signalID);

			if (inputSignal == nullptr || busChildSignal == nullptr)
			{
				result = false;
				continue;
			}

			if (busChildSignal->isCompatible(inputSignal) == false)
			{
				assert(false);						// this error should be detected early
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (inputSignal->isConst() == false && inputSignal->ualAddr().isValid() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(inputSignal->appSignalID(), inputSignal->ualItemGuid(), inputSignal->ualItemSchemaID());
				return false;
			}

			if (busChildSignal->ualAddr().isValid() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(busChildSignal->appSignalID(), busChildSignal->ualItemGuid(), busChildSignal->ualItemSchemaID());
				return false;
			}

			bool res = true;

			switch(busChildSignal->signalType())
			{
			case E::SignalType::Analog:
				res = generateAnalogSignalToBusCode(inputSignal, busChildSignal, busSignal);
				break;

			case E::SignalType::Discrete:
				res = generateDiscreteSignalToBusCode(inputSignal, busChildSignal, busSignal);
				break;

			case E::SignalType::Bus:
				res = generateBusSignalToBusCode(inputSignal, busChildSignal, busSignal);
				break;

			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}

			if (res == true)
			{
				count++;
			}
			else
			{
				result = false;
			}
		}

		if (count > 0)
		{
			m_code.newLine();
		}

		return result;
	}

	UalSignal* ModuleLogicCompiler::getBusComposerBusSignal(const UalItem* composerItem, bool* connectedToTedrminatorOnly)
	{
		if (composerItem == nullptr || connectedToTedrminatorOnly == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		*connectedToTedrminatorOnly = false;

		const std::vector<LogicPin>& outputs = composerItem->outputs();

		if (outputs.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const LogicPin& output = outputs[0];

		if (isConnectedToTerminatorOnly(output) == true)
		{
			*connectedToTedrminatorOnly	= true;
			return nullptr;
		}

		UalSignal* busSignal = m_ualSignals.get(output.guid());

		if (busSignal == nullptr)
		{
			// UalSignal is not found for pin '%1' (Logic schema '%2').
			//
			m_log->errALC5120(composerItem->guid(), composerItem->label(), output.caption(), composerItem->schemaID());
			return nullptr;
		}

		if (busSignal->isBus() == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		return busSignal;
	}

	bool ModuleLogicCompiler::generateAnalogSignalToBusCode(UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		if (inputSignal == nullptr || busChildSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		assert(busChildSignal->ualAddr().isValid() == true);
		assert(busChildSignal->ualAddr().bit() == 0);

		if (busSignal.conversionRequired() == true)
		{
			LOG_INTERNAL_ERROR(m_log);				// bus signals conversion is not implemented now
			return false;
		}

		QString inputSignalIDs = inputSignal->refSignalIDsJoined();
		QString busChildSignalIDs = busChildSignal->refSignalIDsJoined();

		Command cmd;

		if (inputSignal->isConst() == true)
		{
			switch(inputSignal->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				cmd.movConstFloat(busChildSignal->ualAddr().offset(), inputSignal->constAnalogFloatValue());
				cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignal->constAnalogFloatValue()));
				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				cmd.movConstInt32(busChildSignal->ualAddr().offset(), inputSignal->constAnalogIntValue());
				cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignal->constAnalogIntValue()));
				break;

			default:
				assert(false);
				return false;
			}
		}
		else
		{
			assert(inputSignal->ualAddr().isValid() == true);
			assert(inputSignal->ualAddr().bit() == 0);

			cmd.mov32(busChildSignal->ualAddr().offset(), inputSignal->ualAddr().offset());
			cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignalIDs));
		}

		m_code.append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToBusCode(UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		if (inputSignal == nullptr || busChildSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		assert(busChildSignal->ualAddr().isValid() == true);

		if (busSignal.conversionRequired() == true)
		{
			LOG_INTERNAL_ERROR(m_log);				// bus signals conversion is not implemented now
			return false;
		}

		QString inputSignalIDs = inputSignal->refSignalIDsJoined();
		QString busChildSignalIDs = busChildSignal->refSignalIDsJoined();

		Command cmd;

		if (inputSignal->isConst() == true)
		{
			if (inputSignal->constDiscreteValue() != 0)
			{
				cmd.movBitConst(busChildSignal->ualAddr(), inputSignal->constDiscreteValue());
				cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignal->constDiscreteValue()));
				m_code.append(cmd);
			}
		}
		else
		{
			assert(inputSignal->ualAddr().isValid() == true);

			cmd.movBit(busChildSignal->ualAddr(), inputSignal->ualAddr());
			cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignalIDs));
			m_code.append(cmd);
		}

		return true;
	}

	bool ModuleLogicCompiler::generateBusSignalToBusCode(UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		if (inputSignal == nullptr || busChildSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (inputSignal->ualAddr().isValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inputSignal->appSignalID(), inputSignal->ualItemGuid(), inputSignal->ualItemSchemaID());
			return false;
		}

		if (busChildSignal->ualAddr().isValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(busChildSignal->appSignalID(), busChildSignal->ualItemGuid(), busChildSignal->ualItemSchemaID());
			return false;
		}

		if (busSignal.conversionRequired() == true)
		{
			LOG_INTERNAL_ERROR(m_log);				// bus signals conversion is not implemented now
			return false;
		}

		if (inputSignal->busTypeID() != busChildSignal->busTypeID() ||
				inputSignal->sizeW() != busChildSignal->sizeW())
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString inputSignalIDs = inputSignal->refSignalIDsJoined();
		QString busChildSignalIDs = busChildSignal->refSignalIDsJoined();

		Command cmd;

		int busSizeW = busChildSignal->sizeW();

		switch(busSizeW)
		{
		case 1:
			cmd.mov(busChildSignal->ualAddr(), inputSignal->ualAddr());
			break;

		case 2:
			cmd.mov32(busChildSignal->ualAddr(), inputSignal->ualAddr());
			break;

		default:
			cmd.movMem(busChildSignal->ualAddr(), inputSignal->ualAddr(), busSizeW);
		}

		cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignalIDs));
		m_code.append(cmd);

		return true;
	}

	UalItem* ModuleLogicCompiler::getInputPinAssociatedOutputPinParent(QUuid appItemUuid, const QString& inPinCaption, QUuid* connectedOutPinUuid) const
	{
		if (connectedOutPinUuid == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		*connectedOutPinUuid = QUuid();

		UalItem* currentItem  = m_ualItems.value(appItemUuid, nullptr);

		if (currentItem == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		UalItem* connectedItem = nullptr;

		const std::vector<LogicPin>& inputs = currentItem->inputs();

		bool pinFound = false;

		for(const LogicPin& input : inputs)
		{
			if (input.caption() != inPinCaption)
			{
				continue;
			}

			pinFound = true;

			const std::vector<QUuid>& associatedOuts = input.associatedIOs();

			if (associatedOuts.size() > 1)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return nullptr;
			}

			*connectedOutPinUuid = associatedOuts[0];

			connectedItem = m_pinParent.value(*connectedOutPinUuid, nullptr);

			if (connectedItem == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return nullptr;
			}

			break;
		}

		if (pinFound == false)
		{
			// Pin with caption '%1' is not found in schema item (Logic schema '%2').
			//
			m_log->errALC5106(inPinCaption, appItemUuid, currentItem->schemaID());
		}

		return connectedItem;
	}

	UalItem* ModuleLogicCompiler::getAssociatedOutputPinParent(const LogicPin& inputPin, QUuid* connectedOutPinUuid) const
	{
		if (inputPin.IsInput() == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const std::vector<QUuid>& associatedOuts = inputPin.associatedIOs();

		if (associatedOuts.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		if (connectedOutPinUuid != nullptr)
		{
			*connectedOutPinUuid = associatedOuts[0];
		}

		UalItem* connectedOutPinParent = m_pinParent.value(associatedOuts[0], nullptr);

		if (connectedOutPinParent == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		return connectedOutPinParent;
	}

	const UalSignal* ModuleLogicCompiler::getExtractorBusSignal(const UalItem* appBusExtractor)
	{
		const UalBusExtractor* extractor = appBusExtractor->ualBusExtractor();

		if (extractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		// getting extractor's input pin
		//
		const std::vector<LogicPin>& inputs = appBusExtractor->inputs();

		if (inputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		QUuid connectedOutPinUuid;

		UalItem* extractorSourceItem = getAssociatedOutputPinParent(inputs[0], &connectedOutPinUuid);

		QUuid srcSignalUuid;

		switch(extractorSourceItem->type())
		{
		// allowed connections
		//
		case UalItem::Signal:
			// extractor connected to signal
			//
			srcSignalUuid = extractorSourceItem->guid();
			break;

		case UalItem::Afb:
		case UalItem::BusComposer:
			// extractor directly connected to 	Fb, BusComposer
			//
			srcSignalUuid = connectedOutPinUuid;
			break;

		// disallowed or unknown connections
		//
		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const UalSignal* srcSignal = m_ualSignals.get(srcSignalUuid);

		if (srcSignal == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Signal is not found, GUID: %1")).arg(srcSignalUuid.toString()));
			return nullptr;
		}

		return srcSignal;
	}

	bool ModuleLogicCompiler::getConnectedAppItems(const LogicPin& pin, ConnectedAppItems* connectedAppItems)
	{
		if (connectedAppItems == nullptr)
		{
			assert(false);
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		connectedAppItems->clear();

		bool result = true;

		for(QUuid connectedPinUuid : pin.associatedIOs())
		{
			UalItem* connectedPinParent = m_pinParent.value(connectedPinUuid, nullptr);

			if (connectedPinParent == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			connectedAppItems->insert(std::pair<QUuid, UalItem*>(connectedPinUuid, connectedPinParent));
		}

		return result;
	}

	bool ModuleLogicCompiler::getBusProcessingParams(const UalAfb* appFb, bool& isBusProcessingAfb, QString& busTypeID)
	{
		if (appFb == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		isBusProcessingAfb = false;
		busTypeID.clear();

		const Afbl* afbl = m_afbls.value(appFb->afbStrID(), nullptr);

		if (afbl == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		isBusProcessingAfb = afbl->isBusProcessingAfb();

		if (isBusProcessingAfb == false)
		{
			return true;			// result of getBusProcessingParams()!
		}

		// this is bus processing AFB, will try identify BusTypeID

		bool result = true;

		QStringList busTypeIDs;

		for(const LogicPin& inPin : appFb->inputs())
		{
			LogicAfbSignal pinSignal;

			result = appFb->getAfbSignalByPin(inPin, &pinSignal);

			if (result == false)
			{
				return false;
			}

			if (pinSignal.type() != E::SignalType::Bus)
			{
				continue;
			}

			UalSignal* appSignal = getPinInputAppSignal(inPin);

			if (appSignal == nullptr)
			{
				// appSignal is not determined
				// is not error, Сonst element may be connected to input pin
				continue;
			}

			if (appSignal->isBus() == false)
			{
				continue;
			}

			QString btypeID = appSignal->busTypeID().trimmed();

			if (btypeID.isEmpty() == true)
			{
				continue;
			}

			busTypeIDs.append(btypeID);
		}

		if (busTypeIDs.isEmpty() == true)
		{
			// Cannot identify AFB bus type (Logic schema %1).
			//
			m_log->errALC5108(appFb->guid(), appFb->schemaID());
			return false;
		}

		QString checkBusTypeID;

		for(const QString& btypeID : busTypeIDs)
		{
			if (checkBusTypeID.isEmpty() == true)
			{
				checkBusTypeID = btypeID;
				continue;
			}

			if (checkBusTypeID != btypeID)
			{
				// Different bus types on AFB inputs (Logic schema %1).
				//
				m_log->errALC5109(appFb->guid(), appFb->schemaID());
				return false;
			}
		}

		busTypeID = checkBusTypeID;

		return true;
	}

	UalSignal* ModuleLogicCompiler::getPinInputAppSignal(const LogicPin& inPin)
	{
		assert(inPin.IsInput());

		std::vector<QUuid> associatedOuts = inPin.associatedIOs();

		if (associatedOuts.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		QUuid associatedOutUuid = associatedOuts[0];

		const UalItem* connectedPinParent = m_pinParent.value(associatedOutUuid, nullptr);

		if (connectedPinParent == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		UalSignal* appSignal = nullptr;

		if (connectedPinParent->isSignal() == true)
		{
			appSignal = m_ualSignals.get(connectedPinParent->guid());
		}
		else
		{
			appSignal = m_ualSignals.get(associatedOutUuid);
		}

		return appSignal;
	}

	UalSignal* ModuleLogicCompiler::getUalSignalByPinCaption(const UalItem* ualItem, const QString& pinCaption, bool isInput)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		const std::vector<LogicPin>* pins = &ualItem->inputs();

		if (isInput == false)
		{
			pins = &ualItem->outputs();
		}

		for(const LogicPin& pin : *pins)
		{
			if (pin.caption() != pinCaption)
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(pin.guid());

			if (ualSignal == nullptr)
			{
				// UalSignal is not found for pin '%1' (Logic schema '%2').
				//
				m_log->errALC5122(ualItem->guid(), pinCaption, ualItem->schemaID());
				return nullptr;
			}

			return ualSignal;
		}

		// Pin with caption '%1' is not found in schema item (Logic schema '%2').
		//
		m_log->errALC5106(pinCaption, ualItem->guid(), ualItem->schemaID());

		return nullptr;
	}

	bool ModuleLogicCompiler::isConnectedToTerminator(const LogicPin& outPin)
	{
		ConnectedAppItems connectedItems;

		bool result = getConnectedAppItems(outPin, &connectedItems);

		if (result == false)
		{
			assert(false);
			return false;
		}

		for(const std::pair<QUuid, UalItem*>& pair : connectedItems)
		{
			if (pair.second == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				continue;
			}

			if (pair.second->isTerminator() == true)
			{
				return true;
			}
		}

		return false;
	}



	bool ModuleLogicCompiler::addToComparatorStorage(const UalAfb* appFb)
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

	bool ModuleLogicCompiler::initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* appFb)
	{
		Q_UNUSED(cmp);

		bool result = true;

		if (appFb->isConstComaparator() == true)
		{

		}

		///////

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredAnalogOptoSignalsToRegBuf()
	{
		if (m_acquiredAnalogOptoSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.init(&m_resourcesUsageInfo.copyAcquiredAnalogOptoSignalsToRegBuf);

		bool result = true;

		m_code.comment("Copy acquired opto signals in regBuf");
		m_code.newLine();

		for(UalSignal* ualSignal : m_acquiredAnalogOptoSignals)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

			if (ualSignal->isAnalog() == false ||
				ualSignal->isAcquired() == false ||
				ualSignal->isOptoSignal() == false ||
				ualSignal->isConst() == true ||
				ualSignal->isBusChild() == true)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->ualAddr().isValid() == false ||
				ualSignal->regBufAddr().isValid() == false ||
				ualSignal->regValueAddr().isValid() == false)
			{
				assert(false);				// signal's ualAddr ot regBufAddr is not initialized!
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (ualSignal->analogSignalFormat() != E::AnalogAppSignalFormat::Float32 &&
				ualSignal->analogSignalFormat() != E::AnalogAppSignalFormat::SignedInt32)
			{
				assert(false);				// unknown analog format!
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			Command cmd;

			cmd.mov32(ualSignal->regBufAddr(), ualSignal->ualAddr());
			cmd.setComment(QString("copy %1").arg(ualSignal->acquiredRefSignalsIDs().join(", ")));

			m_code.append(cmd);
		}

		m_code.newLine();

		m_code.calculate(&m_resourcesUsageInfo.copyAcquiredAnalogOptoSignalsToRegBuf);

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredAnalogBusChildSignalsToRegBuf()
	{
		if (m_acquiredAnalogBusChildSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.comment("Copy acquired analog bus child signals to reg buf");
		m_code.newLine();

		bool result = true;

		for(UalSignal* ualSignal : m_acquiredAnalogBusChildSignals)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->ualAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->regBufAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->sizeW() != 2)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			Command cmd;

			cmd.mov32(ualSignal->regBufAddr(), ualSignal->ualAddr());
			cmd.setComment(QString("copy %1").arg(ualSignal->refSignalIDsJoined()));

			m_code.append(cmd);
		}

		m_code.newLine();

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredTuningAnalogSignalsToRegBuf()
	{
		if (m_acquiredAnalogTuningSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.init(&m_resourcesUsageInfo.copyAcquiredTuningAnalogSignalsToRegBuf);

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

		for(UalSignal* ualSignal : m_acquiredAnalogTuningSignals)
		{
			if(ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

			Signal* s = ualSignal->getTuningableSignal();

			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

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

		m_code.calculate(&m_resourcesUsageInfo.copyAcquiredTuningAnalogSignalsToRegBuf);

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredTuningDiscreteSignalsToRegBuf()
	{
		if (m_acquiredDiscreteTuningSignals.isEmpty() == true)
		{
			return true;
		}

		m_code.init(&m_resourcesUsageInfo.copyAcquiredTuningDiscreteSignalsToRegBuf);

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

		for(UalSignal* ualAddr : m_acquiredDiscreteTuningSignals)
		{
			TEST_PTR_CONTINUE(ualAddr);

			Signal* s = ualAddr->getTuningableSignal();

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

	bool ModuleLogicCompiler::copyAcquiredAnalogConstSignalsToRegBuf()
	{
		if (m_acquiredAnalogConstIntSignals.isEmpty() == true &&
			m_acquiredAnalogConstFloatSignals.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		m_code.comment("Writing of acquired analog const signals values in reg buf");
		m_code.newLine();

		QVector<int> sortedIntConsts = QVector<int>::fromList(m_acquiredAnalogConstIntSignals.uniqueKeys());

		if (sortedIntConsts.isEmpty() == false)
		{
			qSort(sortedIntConsts);

			for(int intConst : sortedIntConsts)
			{
				QList<UalSignal*> constIntSignals = m_acquiredAnalogConstIntSignals.values(intConst);
				QStringList constIntSignalsIDs;

				Address16 regBufAddr;

				for(UalSignal* constIntSignal : constIntSignals)
				{
					if (regBufAddr.isValid() == false)
					{
						// first iteration
						regBufAddr = constIntSignal->regBufAddr();
					}

					if (regBufAddr.isValid() == false ||
						regBufAddr != constIntSignal->regBufAddr())				// all const signals with same value mast have same reg buf address
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					constIntSignalsIDs.append(constIntSignal->acquiredRefSignalsIDs());
				}

				Command cmd;

				cmd.movConstInt32(regBufAddr.offset(), intConst);
				cmd.setComment(QString("int const %1: %2").arg(intConst).arg(constIntSignalsIDs.join(", ")));

				m_code.append(cmd);
			}

			m_code.newLine();
		}

		//

		QVector<float> sortedFloatConsts = QVector<float>::fromList(m_acquiredAnalogConstFloatSignals.uniqueKeys());

		if (sortedFloatConsts.isEmpty() == false)
		{
			qSort(sortedFloatConsts);

			for(float floatConst : sortedFloatConsts)
			{
				QList<UalSignal*> constFloatSignals = m_acquiredAnalogConstFloatSignals.values(floatConst);
				QStringList constFloatSignalsIDs;

				Address16 regBufAddr;

				for(UalSignal* constFloatSignal : constFloatSignals)
				{
					if (regBufAddr.isValid() == false)
					{
						// first iteration
						regBufAddr = constFloatSignal->regBufAddr();
					}

					if (regBufAddr.isValid() == false ||
						regBufAddr != constFloatSignal->regBufAddr())				// all const signals with same value mast have same reg buf address
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					constFloatSignalsIDs.append(constFloatSignal->acquiredRefSignalsIDs());
				}

				Command cmd;

				cmd.movConstFloat(regBufAddr.offset(), floatConst);
				cmd.setComment(QString("float const %1: %2").arg(floatConst).arg(constFloatSignalsIDs.join(", ")));

				m_code.append(cmd);
			}

			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteInputSignalsToRegBuf()
	{
		m_code.init(&m_resourcesUsageInfo.copyAcquiredDiscreteInputSignalsToRegBuf);

		bool result = copyScatteredDiscreteSignalsInRegBuf(m_acquiredDiscreteInputSignals, "acquired discrete input signals");

		m_code.calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteInputSignalsToRegBuf);

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf()
	{
		m_code.init(&m_resourcesUsageInfo.copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf);

		bool result = copyScatteredDiscreteSignalsInRegBuf(m_acquiredDiscreteOptoAndBusChildSignals, "acquired discrete opto and bus child signals");

		m_code.calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf);

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf()
	{
		m_code.init(&m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf);

		if (m_acquiredDiscreteStrictOutputSignals.isEmpty() == false)
		{
			assert(m_memoryMap.acquiredDiscreteOutputSignalsSizeW() ==
				   m_memoryMap.acquiredDiscreteOutputSignalsInRegBufSizeW());

			m_code.comment("Copy acquired discrete output signals from bit-addressed memory to regBuf");
			m_code.newLine();

			Command cmd;

			cmd.movMem(m_memoryMap.acquiredDiscreteOutputSignalsAddressInRegBuf(),
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

			cmd.movMem(m_memoryMap.acquiredDiscreteInternalSignalsAddressInRegBuf(),
					   m_memoryMap.acquiredDiscreteInternalSignalsAddress(),
					   m_memoryMap.acquiredDiscreteInternalSignalsSizeW());

			m_code.append(cmd);
			m_code.newLine();
		}

		m_code.calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf);

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteConstSignalsToRegBuf()
	{
		if (m_memoryMap.acquiredDiscreteConstSignalsInRegBufSizeW() == 0)
		{
			return true;
		}

		assert(m_memoryMap.acquiredDiscreteConstSignalsInRegBufSizeW() == 1);			// if > 0 then always 1 word!

		QStringList const0Signals;
		QStringList const1Signals;

		for(UalSignal* ualSignal : m_acquiredDiscreteConstSignals)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

			if (ualSignal->constDiscreteValue() == 0)
			{
				const0Signals.append(ualSignal->acquiredRefSignalsIDs());
			}
			else
			{
				const1Signals.append(ualSignal->acquiredRefSignalsIDs());
			}
		}

		assert(const0Signals.size() != 0 || const1Signals.size() != 0);		// why m_memoryMap.acquiredDiscreteConstSignalsInRegBufSizeW() !=0, but const signals is not found ???

		m_code.append(Comment("Copy acquired discrete const signals values:"));

		m_code.append(Comment(QString("const 0: %1").arg(const0Signals.join(", "))));
		m_code.append(Comment(QString("const 1: %1").arg(const1Signals.join(", "))));

		m_code.newLine();

		Command cmd;
		cmd.movConst(m_memoryMap.acquiredDiscreteConstSignalsAddressInRegBuf(), 2);
		cmd.setComment("bit 0 == 0, bit 1 == 1");

		m_code.append(cmd);
		m_code.newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyScatteredDiscreteSignalsInRegBuf(const QVector<UalSignal*>& signalsList, QString description)
	{
		if (signalsList.isEmpty() == true)
		{
			return true;
		}

		Comment comment;

		comment.setComment(QString("Copy %1 in regBuf").arg(description));
		m_code.append(comment);
		m_code.newLine();

		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		int signalsCount = signalsList.count();

		int count = 0;

		Command cmd;

		int countReminder16 = 0;

		bool zeroLastWord = (signalsCount % SIZE_16BIT) != 0 ? true : false;

		for(UalSignal* ualSignal : signalsList)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

			assert(ualSignal->isConst() == false);

			if (ualSignal->ualAddr().isValid() == false ||
				ualSignal->regBufAddr().isValid() == false ||
				ualSignal->regValueAddr().isValid() == false)
			{
				assert(false);				// signal's ualAddr ot regBufAddr is not initialized!
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			countReminder16 = count % SIZE_16BIT;

			assert(ualSignal->regBufAddr().bit() == countReminder16);

			if (countReminder16 == 0 && (signalsCount - count) < SIZE_16BIT && zeroLastWord == true)
			{
				cmd.movConst(bitAccAddr, 0);
				cmd.clearComment();
				m_code.append(cmd);
				zeroLastWord = false;
			}

			cmd.movBit(bitAccAddr, ualSignal->regBufAddr().bit(), ualSignal->ualAddr().offset(), ualSignal->ualAddr().bit());
			cmd.setComment(QString("copy %1").arg(ualSignal->refSignalIDsJoined()));
			m_code.append(cmd);

			count++;

			if ((count % SIZE_16BIT) == 0 || count == signalsCount)
			{
				cmd.clearComment();
				cmd.mov(ualSignal->regBufAddr().offset(), bitAccAddr);
				m_code.append(cmd);
				m_code.newLine();;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOutputSignalsInOutputModulesMemory()
	{
		m_code.init(&m_resourcesUsageInfo.copyOutputSignalsInOutputModulesMemory);

		bool result = true;

		//		result &= initOutputModulesMemory(); is now requred now!!
		result &= conevrtOutputAnalogSignals();
		result &= copyOutputDiscreteSignals();

		m_code.calculate(&m_resourcesUsageInfo.copyOutputSignalsInOutputModulesMemory);

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
		if (m_analogOutputSignalsToConversion.isEmpty() == true)
		{
			return true;
		}

		m_code.comment("Convertion of output analog signals");
		m_code.newLine();

		Command cmd;

		for(Signal* s : m_analogOutputSignalsToConversion)
		{
			TEST_PTR_CONTINUE(s);

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			assert(s->isAnalog() == true);
			assert(s->isOutput() == true);
			assert(s->dataSize() == SIZE_32BIT);
			assert(s->ioBufAddr().isValid() == true);

			int constIntValue = 0;
			float constFloatValue = 0;
			bool constIsFloat = false;

			if (ualSignal->isConst() == true)
			{
				switch(ualSignal->analogSignalFormat())
				{
				case E::AnalogAppSignalFormat::Float32:
					constFloatValue = ualSignal->constAnalogFloatValue();
					constIsFloat = true;
					break;

				case E::AnalogAppSignalFormat::SignedInt32:
					constIntValue = ualSignal->constAnalogIntValue();
					constIsFloat = false;
					break;

				default:
					assert(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}
			}
			else
			{
				assert(s->ualAddr().isValid() == true);
			}

			if (s->needConversion() == false)
			{
				// signal isn't need conversion
				// copy signal only
				//
				if (ualSignal->isConst() == true)
				{
					if (constIsFloat == true)
					{
						cmd.movConstFloat(s->ioBufAddr().offset(), constFloatValue);
						cmd.setComment(QString("analog output %1 set to const %2").arg(s->appSignalID()).arg(constFloatValue));
					}
					else
					{
						cmd.movConstInt32(s->ioBufAddr().offset(), constIntValue);
						cmd.setComment(QString("analog output %1 set to const %2").arg(s->appSignalID()).arg(constIntValue));
					}
				}
				else
				{
					cmd.mov32(s->ioBufAddr().offset(), s->ualAddr().offset());
					cmd.setComment(QString("copy analog output %1").arg(s->appSignalID()));
				}

				m_code.append(cmd);
				m_code.newLine();

				continue;
			}

			UalAfb* appFb = m_inOutSignalsToScalAppFbMap.value(s->appSignalID(), nullptr);

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

			if (ualSignal->isConst() == true)
			{
				if(constIsFloat == true)
				{
					cmd.writeFuncBlockConstFloat(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
													constFloatValue, appFb->caption());
					cmd.setComment(QString(tr("float const %1 to analog output %2 conversion")).
										arg(constFloatValue).arg(s->appSignalID()));
				}
				else
				{
					cmd.writeFuncBlockConstInt32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
													constIntValue, appFb->caption());
					cmd.setComment(QString(tr("int const %1 to analog output %2 conversion")).
										arg(constIntValue).arg(s->appSignalID()));
				}

				m_code.append(cmd);
			}
			else
			{
				cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
								   s->ualAddr().offset(), appFb->caption());
				cmd.setComment(QString(tr("analog output %1 conversion")).arg(s->appSignalID()));
				m_code.append(cmd);
			}

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
		bool result = true;

		QHash<int, Signal*> writeAddressesMap;

		for(Signal* s : m_ioSignals)
		{
			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (s->isDiscrete() == false || s->isOutput() == false)
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal != nullptr)
			{
				writeAddressesMap.insertMulti(s->ioBufAddr().offset(), s);
			}
		}

		int lmOutputsAddress = m_lmDescription->memory().m_appDataOffset;
		bool lmOutputsIsWritten = false;

		m_code.comment("Copy discrete output signals to output modules memory");
		m_code.newLine();

		QList<int> writeAddreses = writeAddressesMap.uniqueKeys();

		QVector<int> sortedWriteAddress = QVector<int>::fromList(writeAddreses);

		qSort(sortedWriteAddress);

		int wordAccAddr = m_memoryMap.wordAccumulatorAddress();

		for(int writeAddr : sortedWriteAddress)
		{
			QList<Signal*> writeSignals = writeAddressesMap.values(writeAddr);

			// signals sorting by ioBufAddr  via std::map
			//
			std::map<int, Signal*> sortedWriteSignals;

			for(Signal* s : writeSignals)
			{
				sortedWriteSignals.insert(std::make_pair(s->ioBufAddr().bitAddress(), s));
			}

			Command cmd;

			cmd.movConst(wordAccAddr, 0);
			m_code.append(cmd);

			for(const std::pair<int, Signal*>& pair: sortedWriteSignals)
			{
				Signal* s = pair.second;

				TEST_PTR_CONTINUE(s);

				if (s->ioBufAddr().isValid() == false)
				{
					assert(false);
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

				if (ualSignal == nullptr)
				{
					assert(false);
					LOG_NULLPTR_ERROR(m_log);
					result = false;
					continue;
				}

				if (ualSignal->isConst() == true)
				{
					cmd.movBitConst(wordAccAddr, s->ioBufAddr().bit(), ualSignal->constDiscreteValue());
				}
				else
				{
					if (s->ualAddr().isValid() == false)
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						result = false;
						continue;
					}

					cmd.movBit(wordAccAddr, s->ioBufAddr().bit(), s->ualAddr().offset(), s->ualAddr().bit());
				}

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

		return result;
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

		m_code.init(&m_resourcesUsageInfo.copyOptoConnectionsTxData);

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

		m_code.calculate(&m_resourcesUsageInfo.copyOptoConnectionsTxData);

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

		result &= copyOptoPortTxBusSignals(port);

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

		int txRawDataStartAddr = port->txBufAbsAddress() + offset;
		int txRawDataSizeW = port->txRawDataSizeW();

		MemWriteMap memWriteMap(txRawDataStartAddr, txRawDataSizeW, true);

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
				result &= copyOptoPortAllNativeRawData(port, offset, memWriteMap);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxModuleRawData:
				result &= copyOptoPortTxModuleRawData(port, offset, item.modulePlace, memWriteMap);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxPortRawData:
				result &= copyOptoPortTxOptoPortRawData(port, offset, item.portEquipmentID, memWriteMap);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxConst16:
				result &= copyOptoPortTxConst16RawData(port, item.const16Value, offset, memWriteMap);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxSignal:
				// code for TxSignals is generated after this switch
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

		result &= copyOptoPortRawTxAnalogSignals(port, memWriteMap);

		result &= copyOptoPortRawTxDiscreteSignals(port, memWriteMap);

		result &= copyOptoPortRawTxBusSignals(port, memWriteMap);

		MemWriteMap::AreaList nonWrittenAreas;

		memWriteMap.getNonWrittenAreas(&nonWrittenAreas);

		bool first = true;

		for(MemWriteMap::Area nonWrittenArea : nonWrittenAreas)
		{
			QString comment;

			if (first == true)
			{
				comment = "fill non written txRawData by 0";
				first = false;
			}

			m_code.append(codeSetMemory(nonWrittenArea.first, 0, nonWrittenArea.second, comment));
		}

		if (first == false)
		{
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxAnalogSignals(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

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

			UalSignal* ualSignal = m_ualSignals.get(txSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(txSignal->appSignalID(), QUuid(), "");
				result = false;
				continue;
			}

			if (ualSignal->isConst() == false && ualSignal->ualAddr().isValid() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(ualSignal->refSignalIDsJoined(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
				result = false;
				continue;
			}

			if (ualSignal->isAnalog() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
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

			SignalAddress16 txSignalAddress;

			bool res = port->getTxSignalAbsAddress(txSignal->appSignalID(), &txSignalAddress);

			if (res == false)
			{
				result = false;
				continue;
			}

			if (ualSignal->isConst() == false)
			{
				cmd.mov32(txSignalAddress.offset(), ualSignal->ualAddr().offset());
				cmd.setComment(QString("%1 >> %2").arg(ualSignal->refSignalIDsJoined()).arg(port->connectionID()));
			}
			else
			{
				switch(ualSignal->analogSignalFormat())
				{
				case E::AnalogAppSignalFormat::Float32:
					cmd.movConstFloat(txSignalAddress.offset(), ualSignal->constAnalogFloatValue());
					cmd.setComment(QString("%1 (const %2) >> %3").arg(ualSignal->refSignalIDsJoined()).
								   arg(ualSignal->constAnalogFloatValue()).arg(port->connectionID()));
					break;

				case E::AnalogAppSignalFormat::SignedInt32:
					cmd.movConstFloat(txSignalAddress.offset(), ualSignal->constAnalogIntValue());
					cmd.setComment(QString("%1 (const %2) >> %3").arg(ualSignal->refSignalIDsJoined()).
								   arg(ualSignal->constAnalogIntValue()).arg(port->connectionID()));
					break;
				default:
					assert(false);
				}
			}

			m_code.append(cmd);
		}

		if (first == false)
		{
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxBusSignals(Hardware::OptoPortShared port)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		bool result = true;

		bool first = true;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == true || txSignal->isBus() == false)
			{
				// skip raw and non-bus signals
				//
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(txSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(txSignal->appSignalID(), QUuid(), "");
				result = false;
				continue;
			}

			if (ualSignal->isBus() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->sizeW() == 0)			// may be, but why?
			{
				continue;
			}

			if (ualSignal->ualAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (first == true)
			{
				Comment comment;

				comment.setComment(QString("Copying tx bus signals of opto-port %1").arg(port->equipmentID()));

				m_code.append(comment);
				m_code.newLine();

				first = false;
			}

			Command cmd;

			SignalAddress16 txSignalAddress;

			bool res = port->getTxSignalAbsAddress(txSignal->appSignalID(), &txSignalAddress);

			if (res == false)
			{
				result = false;
				continue;
			}

			switch(ualSignal->sizeW())
			{
			case SIZE_1WORD:
				cmd.mov(txSignalAddress, ualSignal->ualAddr());
				break;

			case SIZE_2WORD:
				cmd.mov32(txSignalAddress, ualSignal->ualAddr());
				break;

			default:
				cmd.movMem(txSignalAddress, ualSignal->ualAddr(), ualSignal->sizeW());
			}

			cmd.setComment(QString("%1 >> %2").arg(txSignal->appSignalIDs().join(", ")).arg(port->connectionID()));
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

		Commands copyCode;
		QString ids;

		for(int i = 0; i < count; i++)
		{
			Hardware::TxRxSignalShared& txSignal = txDiscreteSignals[i];

			if (txSignal->isRaw() == true)
			{
				continue;					// raw signals copying in raw data section code generation
			}

			UalSignal* ualSignal = m_ualSignals.get(txSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				// Signal identifier '%1' is not found.
				//
				m_log->errALC5000(txSignal->appSignalID(), QUuid(), "");
				result = false;
				continue;
			}

			if (ualSignal->isConst() == false && ualSignal->ualAddr().isValid() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(ualSignal->refSignalIDsJoined(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
				result = false;
				continue;
			}

			if (ualSignal->isDiscrete() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
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

					copyCode.append(cmd);
				}
			}

			int bit = bitCount % WORD_SIZE;

			assert(txSignal->addrInBuf().bit() == bit);

			// copy discrete signal value to bit accumulator
			//
			if (ualSignal->isConst() == true)
			{
				cmd.movBitConst(bitAccumulatorAddress, bit, ualSignal->constDiscreteValue());
				cmd.setComment(QString("%1 (const %2) >> %3").arg(ualSignal->refSignalIDsJoined()).
							   arg(ualSignal->constDiscreteValue()).arg(port->connectionID()));

				copyCode.append(cmd);
			}
			else
			{
				cmd.movBit(bitAccumulatorAddress, bit, ualSignal->ualAddr().offset(), ualSignal->ualAddr().bit());
				cmd.setComment(QString("%1 >> %2").arg(ualSignal->refSignalIDsJoined()).arg(port->connectionID()));

				copyCode.append(cmd);

				if (ids.isEmpty() == true)
				{
					ids = ualSignal->refSignalIDsJoined();
				}
				else
				{
					ids += ", " + ualSignal->refSignalIDsJoined();
				}
			}

			if ((bitCount % WORD_SIZE) == (WORD_SIZE -1) ||			// if this is last bit in word or
				i == count -1)									// this is even the last bit
			{
				// txSignal.address.offset() the same for all signals in one word

				int txSignalAddress = port->txBufAbsAddress() + txSignal->addrInBuf().offset();

				int srcAddr = 0;

				if (isCopyOptimizationAllowed(copyCode, &srcAddr) == true)
				{
					cmd.mov(txSignalAddress, srcAddr);
					cmd.setComment(QString("%1 >> %2").arg(ids).arg(port->connectionID()));

					m_code.append(cmd);
				}
				else
				{
					m_code.append(copyCode);

					// copy bit accumulator to opto interface buffer
					//
					cmd.mov(txSignalAddress, bitAccumulatorAddress);
					cmd.clearComment();

					m_code.append(cmd);
				}

				copyCode.clear();
				ids.clear();
			}

			bitCount++;
		}

		if (first == false)
		{
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::isCopyOptimizationAllowed(const Commands& copyCode, int* srcAddr)
	{
		if (srcAddr == nullptr)
		{
			assert(false);
			return false;
		}

		// copy optimization is allowed if:
		//		all 16 commands is MOVB
		//		all offsets in source address is same
		//		all offsets in dest address is same
		//		all bitNo in src and dest addresses is equal

		if (copyCode.size() != 16)
		{
			return false;
		}

		int srcOffset = -1;
		int destOffset = -1;

		for(int i = 0; i < 16; i++)
		{
			const Command& cmd = copyCode[i];

			if (cmd.getOpcode() != LmCommandCode::MOVB)
			{
				return false;
			}

			if (destOffset == -1)
			{
				destOffset = cmd.getWord2();
			}
			else
			{
				if (destOffset != cmd.getWord2())
				{
					return false;
				}
			}

			if (srcOffset == -1)
			{
				srcOffset = cmd.getWord3();
			}
			else
			{
				if (srcOffset != cmd.getWord3())
				{
					return false;
				}
			}

			if (cmd.getBitNo1() != cmd.getBitNo2())
			{
				return false;
			}
		}

		*srcAddr = srcOffset;

		return true;
	}

	bool ModuleLogicCompiler::copyOptoPortAllNativeRawData(Hardware::OptoPortShared port, int& offset, MemWriteMap& memWriteMap)
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

			result &= copyOptoPortTxModuleRawData(port, offset, module, memWriteMap);
		}

		return result;
	}


	bool ModuleLogicCompiler::copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, int place, MemWriteMap& memWriteMap)
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

		return copyOptoPortTxModuleRawData(port, offset, module, memWriteMap);
	}


	bool ModuleLogicCompiler::copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, const Hardware::DeviceModule* module, MemWriteMap& memWriteMap)
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

			int sizeW = 0;

			fromAddr = m_memoryMap.getModuleDataOffset(module->place());

			switch(item.type)
			{
			case ModuleRawDataDescription::ItemType::RawDataSize:
				autoSize = item.rawDataSizeIsAuto;
				break;

			case ModuleRawDataDescription::ItemType::AppData16:

				fromAddr += moduleAppDataOffset + item.offset;

				cmd.mov(toAddr, fromAddr);

				sizeW = 1;

				localOffset++;

				break;

			case ModuleRawDataDescription::ItemType::DiagData16:

				fromAddr += moduleDiagDataOffset + item.offset;

				cmd.mov(toAddr, fromAddr);

				sizeW = 1;

				localOffset++;

				break;


			case ModuleRawDataDescription::ItemType::AppData32:

				fromAddr += moduleAppDataOffset + item.offset;

				cmd.mov32(toAddr, fromAddr);

				sizeW = 2;

				localOffset += 2;

				break;

			case ModuleRawDataDescription::ItemType::DiagData32:

				fromAddr += moduleDiagDataOffset + item.offset;

				cmd.mov32(toAddr, fromAddr);

				sizeW = 2;

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

				//

				memWriteMap.write(toAddr, sizeW);
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


	bool ModuleLogicCompiler::copyOptoPortTxOptoPortRawData(Hardware::OptoPortShared port, int& offset, const QString& portEquipmentID, MemWriteMap& memWriteMap)
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

		int writeAddr = port->txBufAbsAddress() + offset;
		int writeSizeW = portTxRawDataSizeW;
		cmd.movMem(writeAddr, portWithRxRawData->rxBufAbsAddress() + Hardware::OptoPort::TX_DATA_ID_SIZE_W, writeSizeW);

		cmd.setComment(QString("copying raw data received on port %1").arg(portWithRxRawData->equipmentID()));

		m_code.append(cmd);
		m_code.newLine();

		offset += portTxRawDataSizeW;

		//

		memWriteMap.write(writeAddr, writeSizeW);

		return true;
	}

	bool ModuleLogicCompiler::copyOptoPortTxConst16RawData(Hardware::OptoPortShared port, int const16value, int& offset, MemWriteMap& memWriteMap)
	{
		if (port == nullptr)
		{
			assert(false);
			return false;
		}

		Command cmd;

		int writeAddr = port->txBufAbsAddress() + offset;
		int writeSizeW = 1;

		cmd.movConst(writeAddr, const16value);

		cmd.setComment(QString("copying raw data const16 value = %1").arg(const16value));

		m_code.append(cmd);
		m_code.newLine();

		offset++;

		//

		memWriteMap.write(writeAddr, writeSizeW);

		return true;

	}

	bool ModuleLogicCompiler::copyOptoPortRawTxAnalogSignals(Hardware::OptoPortShared port, MemWriteMap& memWriteMap)
	{
		if (port == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

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

			UalSignal* ualSignal = m_ualSignals.get(txSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualSignal->isAnalog() == false)
			{
				// Type of signal %1 connected to opto port %2 isn't correspond to its type specified in raw data description.
				//
				m_log->errALC5131(txSignal->appSignalID(), port->equipmentID());
				result = false;
				continue;
			}

			if (ualSignal->isConst() == false && ualSignal->ualAddr().isValid() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
				result = false;
				continue;
			}

			int writeAddr = port->txBufAbsAddress() + txSignal->addrInBuf().offset();

			if (ualSignal->isConst() == true)
			{
				switch(ualSignal->analogSignalFormat())
				{
				case E::AnalogAppSignalFormat::Float32:
					cmd.movConstFloat(writeAddr, ualSignal->constAnalogFloatValue());
					break;

				case E::AnalogAppSignalFormat::SignedInt32:
					cmd.movConstInt32(writeAddr, ualSignal->constAnalogIntValue());
					break;

				default:
					assert(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}
			}
			else
			{
				cmd.mov32(writeAddr, ualSignal->ualAddr().offset());
			}

			cmd.setComment(QString("%1 >> %2").arg(txSignal->appSignalID()).arg(port->connectionID()));

			m_code.append(cmd);

			//

			MemWriteMap::Error err = memWriteMap.write32(writeAddr);

			if (err != MemWriteMap::Error::Ok)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}

			count++;
		}

		if (count > 0)
		{
			m_code.newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortRawTxDiscreteSignals(Hardware::OptoPortShared port, MemWriteMap& memWriteMap)
	{
		const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		int count = 0;

		QHash<int, Hardware::TxRxSignalShared> txDiscretes;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == false || txSignal->isDiscrete() == false)
			{
				// skip non-Raw and non-Discrete signals
				//
				continue;
			}

			txDiscretes.insertMulti(txSignal->addrInBuf().offset(), txSignal);

			count++;
		}

		if (count == 0)
		{
			return true;
		}

		QVector<int> offsets(QVector<int>::fromList(txDiscretes.uniqueKeys()));

		qSort(offsets);

		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		bool result = true;

		Command cmd;

		for(int offset : offsets)
		{
			cmd.movConst(bitAccAddr, 0);
			m_code.append(cmd);

			QList<Hardware::TxRxSignalShared> discretes = txDiscretes.values(offset);

			count = 0;

			for(Hardware::TxRxSignalShared discrete : discretes)
			{
				UalSignal* ualSignal = m_ualSignals.get(discrete->appSignalID());

				if (ualSignal == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				if (ualSignal->isDiscrete() == false)
				{
					// Type of signal %1 connected to opto port %2 isn't correspond to its type specified in raw data description.
					//
					m_log->errALC5131(discrete->appSignalID(), port->equipmentID());
					result = false;
					continue;
				}

				if (ualSignal->isConst() == false && ualSignal->ualAddr().isValid() == false)
				{
					// Undefined UAL address of signal '%1' (Logic schema '%2').
					//
					m_log->errALC5105(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
					result = false;
					continue;
				}

				Address16 addrInBuf = discrete->addrInBuf();

				if (ualSignal->isConst() == true)
				{
					cmd.movBitConst(bitAccAddr, addrInBuf.bit(), ualSignal->constDiscreteValue());
				}
				else
				{
					cmd.movBit(bitAccAddr, addrInBuf.bit(), ualSignal->ualAddr().offset(), ualSignal->ualAddr().bit());
				}

				cmd.setComment(QString("%1 >> %2").arg(discrete->appSignalID()).arg(port->connectionID()));

				m_code.append(cmd);

				count++;
			}

			if (count > 0)
			{
				int writeAddr = port->txBufAbsAddress() + offset;

				cmd.mov(writeAddr, bitAccAddr);
				cmd.clearComment();

				m_code.append(cmd);

				//

				MemWriteMap::Error err = memWriteMap.write16(writeAddr);

				if (err != MemWriteMap::Error::Ok)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
				}
			}
		}

		m_code.newLine();

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortRawTxBusSignals(Hardware::OptoPortShared port, MemWriteMap& memWriteMap)
	{
		if (port == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		QVector<Hardware::TxRxSignalShared> txSignals;

		port->getTxSignals(txSignals);

		bool result = true;

		Command cmd;

		int count = 0;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == false || txSignal->isBus() == false)
			{
				// skip non-Raw and non-Bus signals
				//
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(txSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				// Signal '%1' is not found (opto port '%2' raw data description).
				//
				m_log->errALC5186(txSignal->appSignalID(), port->equipmentID());
				result = false;
				continue;
			}

			if (ualSignal->isBus() == false || ualSignal->busTypeID() != txSignal->busTypeID())
			{
				// Type of signal %1 connected to opto port %2 isn't correspond to its type specified in raw data description.
				//
				m_log->errALC5131(txSignal->appSignalID(), port->equipmentID());
				result = false;
				continue;
			}

			if (ualSignal->bus() == nullptr)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,
								   QString("Raw tx UalSignal %1 bus description is undefined (Opto port %2).").
										arg(txSignal->appSignalID()).arg(port->equipmentID()));
				result = false;
				continue;
			}

			if (ualSignal->ualAddr().isValid() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
				result = false;
				continue;
			}

			if (txSignal->dataSize() != ualSignal->bus()->sizeBit())
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,
								   QString("DataSize of raw data bus TxSignal %1 is not equal to correspond app signal (Opto port %2).").
										arg(txSignal->appSignalID()).arg(port->equipmentID()));
				result = false;
				continue;
			}

			if ((txSignal->dataSize() % SIZE_16BIT) != 0)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,
								   QString("DataSize of raw data bus TxSignal %1 is not multiple to 16 bit (Opto port %2).").
										arg(txSignal->appSignalID()).arg(port->equipmentID()));
				result = false;
				continue;
			}

			int writeAddr = port->txBufAbsAddress() + txSignal->addrInBuf().offset();
			int writeSizeW = 0;

			switch(txSignal->dataSize())
			{
			case SIZE_16BIT:
				writeSizeW = 1;
				cmd.mov(writeAddr, ualSignal->ualAddr().offset());
				break;

			case SIZE_32BIT:
				writeSizeW = 2;
				cmd.mov32(writeAddr, ualSignal->ualAddr().offset());
				break;

			default:
				writeSizeW = txSignal->dataSize() / SIZE_16BIT;
				cmd.movMem(writeAddr, ualSignal->ualAddr().offset(), writeSizeW);
			}

			cmd.setComment(QString("%1 >> %2").arg(txSignal->appSignalID()).arg(port->connectionID()));
			m_code.append(cmd);

			//

			memWriteMap.write(writeAddr, writeSizeW);

			count++;
		}

		if (count > 0)
		{
			m_code.newLine();
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

		if (m_lmDescription->flashMemory().m_appLogicWriteBitstream == true)
		{
			result &= m_appLogicCompiler.writeBinCodeForLm(m_lmSubsystemID,
														   m_lmSubsystemKey,
														   m_lmDescription->flashMemory().m_appLogicUartId,
														   m_lm->equipmentIdTemplate(),
														   m_lmNumber,
														   m_lmAppLogicFrameSize,
														   m_lmAppLogicFrameCount,
														   uniqueID,
														   m_lmDescription->lmDescriptionFile(m_lm),
														   m_lmDescription->descriptionNumber(),
														   m_code);
			if (result == false)
			{
				return false;
			}
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

		BuildFile* buildFile = m_resultWriter->addFile(m_lmSubsystemID, QString("%1-%2.asm").
													   arg(m_lmSubsystemID.toLower()).arg(m_lmNumber), asmCode);

		if (buildFile == nullptr)
		{
			result = false;
		}

		QStringList memFile;

		m_memoryMap.getFile(memFile);

		buildFile = m_resultWriter->addFile(m_lmSubsystemID, QString("%1-%2.mem").
											arg(m_lmSubsystemID.toLower()).arg(m_lmNumber), memFile);

		if (buildFile == nullptr)
		{
			result = false;
		}

		result &= writeTuningInfoFile(m_lmSubsystemID, m_lmNumber);

		//
		// writeLMCodeTestFile();
		//

		result &= writeOcmRsSignalsXml();

		return result;
	}

	bool ModuleLogicCompiler::setLmAppLANDataUID(const QByteArray& lmAppCode, quint64& uniqueID)
	{
		QVector<UalSignal*> acquiredSignals;

		acquiredSignals.append(m_acquiredDiscreteInputSignals);
		acquiredSignals.append(m_acquiredDiscreteStrictOutputSignals);
		acquiredSignals.append(m_acquiredDiscreteInternalSignals);
		acquiredSignals.append(m_acquiredDiscreteTuningSignals);
		acquiredSignals.append(m_acquiredDiscreteConstSignals);
		acquiredSignals.append(m_acquiredDiscreteOptoAndBusChildSignals);
		acquiredSignals.append(m_acquiredAnalogInputSignals);
		acquiredSignals.append(m_acquiredAnalogStrictOutputSignals);
		acquiredSignals.append(m_acquiredAnalogInternalSignals);
		acquiredSignals.append(m_acquiredAnalogOptoSignals);
		acquiredSignals.append(m_acquiredAnalogBusChildSignals);
		acquiredSignals.append(m_acquiredAnalogTuningSignals);
		acquiredSignals.append(m_acquiredBuses);

		QStringList constSignalsIDs;

		for(const UalSignal* constIntSignal : m_acquiredAnalogConstIntSignals)
		{
			constSignalsIDs.append(constIntSignal->appSignalID());
		}

		for(const UalSignal* constFloatSignal : m_acquiredAnalogConstFloatSignals)
		{
			constSignalsIDs.append(constFloatSignal->appSignalID());
		}

		constSignalsIDs.sort();

		for(const QString& constSignalID : constSignalsIDs)
		{
			UalSignal* constUalSignal = m_ualSignals.get(constSignalID);

			if (constUalSignal == nullptr)
			{
				assert(false);
				continue;
			}

			acquiredSignals.append(constUalSignal);
		}

		Crc64 crc;

		crc.add(lmAppCode);

		// add signals to UID
		//
		for(UalSignal* ualSignal: acquiredSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			crc.add(ualSignal->appSignalID());

			if (ualSignal->regBufAddr().isValid() == true)
			{
				crc.add(ualSignal->regBufAddr().bitAddress());
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

	bool ModuleLogicCompiler::writeTuningInfoFile(const QString& subsystemID, int lmNumber)
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
								signal->tuningDefaultValue().toFloat(),
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
								signal->tuningDefaultValue().intValue(),
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

				str.sprintf("%05d:%02d\t%-24s\t%d\t\t%d\t\t%d",
								signal->tuningAddr().offset(),
								signal->tuningAddr().bit(),
								C_STR(signal->appSignalID()),
								signal->tuningDefaultValue().discreteValue(),
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
										 arg(subsystemID.toLower()).arg(lmNumber), file);
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

	bool ModuleLogicCompiler::displayResourcesUsageInfo()
	{
		QString str;

		double percentOfUsedCodeMemory = (m_code.commandAddress() * 100.0) / m_lmCodeMemorySize;

		bool result = true;

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
				result = false;
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
				result = false;
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
				result = false;
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
				result = false;
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
				result = false;
			}
		}

		m_resourcesUsageInfo.lmEquipmentID = m_lm->equipmentIdTemplate();
		m_resourcesUsageInfo.codeMemoryUsed = percentOfUsedCodeMemory;
		m_resourcesUsageInfo.bitMemoryUsed = percentOfUsedBitMemory;
		m_resourcesUsageInfo.wordMemoryUsed = percentOfUsedWordMemory;
		m_resourcesUsageInfo.idrPhaseTimeUsed = idrPhaseTimeUsed;
		m_resourcesUsageInfo.alpPhaseTimeUsed = alpPhaseTimeUsed;

		result &= getAfblUsageInfo();

		//

		LOG_MESSAGE(m_log, QString("Init AFBs code: %1").arg(m_resourcesUsageInfo.initAfbs.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Analog inputs conversion code: %1").arg(m_resourcesUsageInfo.convertAnalogInputSignals.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Application logic code: %1").arg(m_resourcesUsageInfo.appLogicCode.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired analog opto signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredAnalogOptoSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired tuning analog signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredTuningAnalogSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired tuning discrete signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredTuningDiscreteSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired discrete input signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredDiscreteInputSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired discrete output and internal signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired discrete opto and bus child signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Output signals copying to output modules: %1").arg(m_resourcesUsageInfo.copyOutputSignalsInOutputModulesMemory.codePercentStr()));

		LOG_MESSAGE(m_log, QString("Opto connections tx data copying: %1").arg(m_resourcesUsageInfo.copyOptoConnectionsTxData.codePercentStr()));

/*		CodeFragmentMetrics copyAcquiredRawDataInRegBuf;
		CodeFragmentMetrics copyAcquiredAnalogBusChildSignalsToRegBuf;
		CodeFragmentMetrics copyAcquiredConstAnalogSignalsToRegBuf;
		CodeFragmentMetrics copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf;
		CodeFragmentMetrics copyAcquiredDiscreteConstSignalsToRegBuf;
		CodeFragmentMetrics copyOutputSignalsInOutputModulesMemory;*/

		return result;
	}

	void ModuleLogicCompiler::calcOptoDiscretesStatistics()
	{
		QList<Hardware::OptoPortShared> associatedPorts;

		m_optoModuleStorage->getLmAssociatedOptoPorts(m_lm->equipmentIdTemplate(), associatedPorts);

		QHash<UalSignal*, int> signalsRefs;

		for(Hardware::OptoPortShared port : associatedPorts)
		{
			const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

			for(Hardware::TxRxSignalShared txSignal : txSignals)
			{
				if (txSignal->isDiscrete() == false || txSignal->isRegular() == false)
				{
					continue;
				}

				UalSignal* ualSignal = m_ualSignals.get(txSignal->appSignalID());

				if (ualSignal == nullptr)
				{
					LOG_NULLPTR_ERROR(m_log);
					continue;
				}

				int refCount = signalsRefs.value(ualSignal, 0);

				refCount++;

				signalsRefs.insert(ualSignal, refCount);
			}
		}

		int ref1Count = 0;

		for(int refCount : signalsRefs)
		{
			if (refCount == 1)
			{
				ref1Count++;
			}
		}

		double percent = 0;

		if (signalsRefs.count() != 0)
		{
			percent = static_cast<double>(ref1Count) / static_cast<double>(signalsRefs.count()) * 100;
		}

		LOG_MESSAGE(m_log, QString("Percent of discretes transmitted via 1 opto-port: %1").arg(percent));
	}

	bool ModuleLogicCompiler::getAfblUsageInfo()
	{
		bool result = true;

		if (m_lmDescription == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		m_resourcesUsageInfo.afblUsageInfo.clear();

		const std::map<int, std::shared_ptr<Afb::AfbComponent>>& components = m_lmDescription->afbComponents();

		for(std::pair<int, std::shared_ptr<Afb::AfbComponent>> pair : components)
		{
			int componentOpCode = pair.first;
			std::shared_ptr<Afb::AfbComponent> component = pair.second;

			AfblUsageInfo aui;

			aui.opCode = componentOpCode;
			aui.caption = component->caption();
			aui.maxInstances = component->maxInstCount();
			aui.version = component->impVersion();

			aui.usedInstances = m_afbls.getUsedInstances(componentOpCode);

			if (aui.maxInstances != 0)
			{
				aui.usagePercent = static_cast<double>(aui.usedInstances) * 100.0 / static_cast<double>(aui.maxInstances);
			}

			m_resourcesUsageInfo.afblUsageInfo.append(aui);
		}

		return result;
	}

	void ModuleLogicCompiler::cleanup()
	{
		for(UalItem* appItem : m_ualItems)
		{
			delete appItem;
		}

		m_ualItems.clear();

		for(UalItem* scalAppItem : m_scalAppItems)
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

	bool ModuleLogicCompiler::checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const UalAfb& fb, const LogicAfbSignal& afbSignal)
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
				m_log->errALC5010(srcSignal.appSignalID(), fb.caption(), afbSignal.caption(), srcSignalUuid, fb.schemaID());
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
		return m_ualSignals.contains(appSignalID);
	}

	QString ModuleLogicCompiler::getSchemaID(QUuid itemUuid)
	{
		UalItem* appItem = m_ualItems.value(itemUuid, nullptr);

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


	bool ModuleLogicCompiler::writeSignalLists()
	{
		bool result = true;

		result &= writeSignalList(m_acquiredDiscreteInputSignals, "acquiredDiscreteInput");
		result &= writeSignalList(m_acquiredDiscreteStrictOutputSignals, "acquiredDiscreteStrictOutput");
		result &= writeSignalList(m_acquiredDiscreteInternalSignals, "acquiredDiscreteInternal");
		result &= writeSignalList(m_acquiredDiscreteTuningSignals, "acquiredDiscreteTuning");
		result &= writeSignalList(m_acquiredDiscreteConstSignals, "acquiredDiscreteConst");
		result &= writeSignalList(m_acquiredDiscreteOptoAndBusChildSignals, "acquiredDiscreteOptoAndBusChild");

		result &= writeSignalList(m_nonAcquiredDiscreteInputSignals, "nonAcquiredDiscreteInput");
		result &= writeSignalList(m_nonAcquiredDiscreteStrictOutputSignals, "nonAcquiredDiscreteStrictOutput");
		result &= writeSignalList(m_nonAcquiredDiscreteInternalSignals, "nonAcquiredDiscreteInternal");
		result &= writeSignalList(m_nonAcquiredDiscreteOptoSignals, "nonAcquiredDiscreteOpto");

		result &= writeSignalList(m_acquiredAnalogInputSignals, "acquiredAnalogInput");
		result &= writeSignalList(m_acquiredAnalogStrictOutputSignals, "acquiredAnalogStrictOutput");
		result &= writeSignalList(m_acquiredAnalogInternalSignals, "acquiredAnalogInternal");
		result &= writeSignalList(m_acquiredAnalogOptoSignals, "acquiredAnalogOpto");
		result &= writeSignalList(m_acquiredAnalogBusChildSignals, "acquiredAnalogBusChild");
		result &= writeSignalList(m_acquiredAnalogTuningSignals, "acquiredAnalogTuning");

		result &= writeSignalList(m_nonAcquiredAnalogInputSignals, "nonAcquiredAnalogInput");
		result &= writeSignalList(m_nonAcquiredAnalogStrictOutputSignals, "nonAcquiredAnalogStrictOutput");
		result &= writeSignalList(m_nonAcquiredAnalogInternalSignals, "nonAcquiredAnalogInternal");

		result &= writeSignalList(m_acquiredBuses, "acquiredBuses");
		result &= writeSignalList(m_nonAcquiredBuses, "nonAcquiredBuses");

		result &= writeUalSignalsList();

		return result;
	}

	bool ModuleLogicCompiler::writeSignalList(const QVector<UalSignal*>& signalList, QString listName) const
	{
		QStringList strList;

		bool result = true;

		for(const UalSignal* ualSignal : signalList)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			strList.append(QString("%1;%2;%3;%4;%5;%6;%7").
						   arg(ualSignal->refSignalIDsJoined()).
						   arg(ualSignal->ualAddr().offset()).arg(ualSignal->ualAddr().bit()).
						   arg(ualSignal->regBufAddr().offset()).arg(ualSignal->regBufAddr().bit()).
						   arg(ualSignal->regValueAddr().offset()).arg(ualSignal->regValueAddr().bit()));
		}

		m_resultWriter->addFile(QString("%1/%2").arg(m_lmSubsystemID).arg(m_lm->equipmentIdTemplate()),
								QString("sl_%1.csv").arg(listName), "", "", strList);
		return result;
	}

	bool ModuleLogicCompiler::writeUalSignalsList() const
	{
		QStringList report;

		m_ualSignals.getReport(report);

		BuildFile* buildFile = m_resultWriter->addFile(QString("%1/%2").arg(m_lmSubsystemID).arg(m_lm->equipmentId()), "ualSignals.csv", "", "", report, false);

		return buildFile != nullptr;
	}

	bool ModuleLogicCompiler::runProcs(const ProcsToCallArray& procArray)
	{
		bool result = true;

		for(const ProcToCall& proc : procArray)
		{
			result &= (this->*proc.first)();

			if (result == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,
								   QString(tr("%1 finished with error")).arg(proc.second));
				break;
			}
		}

		return result;
	}

	Address16 ModuleLogicCompiler::getConstBitAddr(UalSignal* constDiscreteUalSignal)
	{
		if (constDiscreteUalSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return Address16();
		}

		if (constDiscreteUalSignal->isConst() == false || constDiscreteUalSignal->isDiscrete() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return Address16();
		}

		if (constDiscreteUalSignal->constDiscreteValue() == 0)
		{
			return constBit0Addr();
		}

		if (constDiscreteUalSignal->constDiscreteValue() == 1)
		{
			return constBit1Addr();
		}

		LOG_INTERNAL_ERROR(m_log);
		return Address16();
	}

	Commands ModuleLogicCompiler::codeSetMemory(int addrFrom, quint16 constValue, int sizeW, const QString& comment)
	{
		assert(addrFrom >=0 && addrFrom < static_cast<int>(m_lmDescription->memory().m_appMemorySize));
		assert(addrFrom + sizeW < static_cast<int>(m_lmDescription->memory().m_appMemorySize));

		Command cmd;

		switch(sizeW)
		{
		case 1:
			cmd.movConst(addrFrom, constValue);
			break;

		case 2:
			{
				quint32 constValue32 = constValue;

				constValue32 <<= 16;
				constValue32 &= constValue;

				cmd.movConstUInt32(addrFrom, constValue32);
			}
			break;

		default:
			cmd.setMem(addrFrom, constValue, sizeW);
		}

		if (comment.isEmpty() == false)
		{
			cmd.setComment(comment);
		}

		Commands code;

		code.append(cmd);

		return code;
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
