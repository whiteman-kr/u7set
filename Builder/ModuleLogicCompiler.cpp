#include "../Builder/ModuleLogicCompiler.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"
#include "../lib/DataSource.h"
#include "../UtilsLib/Crc.h"
#include "../HardwareLib/Connection.h"
#include "../lib/LanControllerInfoHelper.h"

#include "SoftwareCfgGenerator.h"
#include "Parser.h"
#include "LmDescriptionSet.h" 

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
		m_context(appLogicCompiler.context()),
		m_lm(lm),
		m_memoryMap(appLogicCompiler.log()),
		m_ualSignals(*this, appLogicCompiler.log()),
		m_loopbacks(*this)
	{
		m_equipmentSet = appLogicCompiler.equipmentSet();
		m_deviceRoot = m_equipmentSet->root().get();
		m_signals = appLogicCompiler.signalSet();

		m_lmDescription = appLogicCompiler.lmDescriptions()->get(lm);

		assert(m_lmDescription != nullptr);

		m_appLogicData = appLogicCompiler.appLogicData();
		m_resultWriter = appLogicCompiler.buildResultWriter();
		m_log = appLogicCompiler.log();

		m_connections = appLogicCompiler.connectionStorage();
		m_optoModuleStorage = appLogicCompiler.opticModuleStorage();
		m_tuningDataStorage = appLogicCompiler.tuningDataStorage();
		m_cmpSet = appLogicCompiler.comparatorSet();
	}

	ModuleLogicCompiler::~ModuleLogicCompiler()
	{
		cleanup();
	}

	AppSignal* ModuleLogicCompiler::getSignal(const QString& appSignalID)
	{
		return m_chassisSignals.value(appSignalID, nullptr);
	}

	bool ModuleLogicCompiler::pass1()
	{
		LOG_EMPTY_LINE(m_log)

		LOG_MESSAGE(m_log, QString(tr("Compilation pass #1 for LM %1 was started...")).arg(lmEquipmentID()));

		m_chassis = m_lm->getParentChassis();

		if (m_chassis == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("LM %1 must be placed in the chassis!")).arg(lmEquipmentID()));
			return false;
		}

		std::shared_ptr<AppLogicModule> appLogicModule = m_appLogicData->module(lmEquipmentID());

		m_moduleLogic = appLogicModule.get();

		if (m_moduleLogic == nullptr)
		{
			//	Application logic for module '%1' is not found.
			//
			m_log->wrnALC5001(lmEquipmentID());
		}

		ProcsToCallArray procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::loadLMSettings),
			PROC_TO_CALL(ModuleLogicCompiler::loadModulesSettings),
			PROC_TO_CALL(ModuleLogicCompiler::createChassisSignalsMap),
			PROC_TO_CALL(ModuleLogicCompiler::createUalItemsMaps),
			PROC_TO_CALL(ModuleLogicCompiler::createUalAfbsMap),
			PROC_TO_CALL(ModuleLogicCompiler::createUalSignals),
			PROC_TO_CALL(ModuleLogicCompiler::processSignalsWithFlags),
			PROC_TO_CALL(ModuleLogicCompiler::sortUalSignals),
			PROC_TO_CALL(ModuleLogicCompiler::processTxSignals),
			PROC_TO_CALL(ModuleLogicCompiler::processSinglePortRxSignals),
			PROC_TO_CALL(ModuleLogicCompiler::buildTuningData),
			PROC_TO_CALL(ModuleLogicCompiler::disposeSignalsInHeap),
			PROC_TO_CALL(ModuleLogicCompiler::createSignalLists),
//			PROC_TO_CALL(ModuleLogicCompiler::groupTxSignals),
			PROC_TO_CALL(ModuleLogicCompiler::disposeSignalsInMemory),
			PROC_TO_CALL(ModuleLogicCompiler::appendAfbsForAnalogInOutSignalsConversion),
			PROC_TO_CALL(ModuleLogicCompiler::setOutputSignalsAsComputed),
			PROC_TO_CALL(ModuleLogicCompiler::setOptoRawInSignalsAsComputed),
			PROC_TO_CALL(ModuleLogicCompiler::fillComparatorSet),
		};

		bool result = runProcs(procs);

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString(tr("Compilation pass #1 for LM %1 was successfully finished.")).arg(lmEquipmentID()));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Compilation pass #1 for LM %1 was finished with errors")).arg(lmEquipmentID()));
		}

		return result;
	}

	bool ModuleLogicCompiler::pass2()
	{
		LOG_EMPTY_LINE(m_log)

		LOG_MESSAGE(m_log, QString(tr("Compilation pass #2 for LM %1 was started...")).arg(lmEquipmentID()));

		m_code.setMemoryMap(&m_memoryMap, m_log);

		ProcsToCallArray procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::initComparatorSignals),

			PROC_TO_CALL(ModuleLogicCompiler::finalizeOptoConnectionsProcessing),
			PROC_TO_CALL(ModuleLogicCompiler::setOptoUalSignalsAddresses),
			PROC_TO_CALL(ModuleLogicCompiler::writeSignalLists),

			// code generation functions

			PROC_TO_CALL(ModuleLogicCompiler::generateIdrPhaseCode),
			PROC_TO_CALL(ModuleLogicCompiler::generateAlpPhaseCode),
			PROC_TO_CALL(ModuleLogicCompiler::makeAppLogicCode),

			PROC_TO_CALL(ModuleLogicCompiler::finalizeAppLogicCodeGeneration),

			//

			PROC_TO_CALL(ModuleLogicCompiler::setLmAppLANDataSize),
			PROC_TO_CALL(ModuleLogicCompiler::detectUnusedSignals),
			PROC_TO_CALL(ModuleLogicCompiler::fillAnalogSignalsOnSchemas),
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
			LOG_SUCCESS(m_log, QString(tr("Compilation pass #2 for LM %1 was successfully finished.")).arg(lmEquipmentID()));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Compilation pass #2 for LM %1 was finished with errors")).arg(lmEquipmentID()));
		}

		calcOptoDiscretesStatistics();

		cleanup();

		return result;
	}

	QString ModuleLogicCompiler::lmEquipmentID() const
	{
		if (m_lm == nullptr)
		{
			assert(false);
			LOG_NULLPTR_ERROR(m_log);
			return QString();
		}

		return m_lm->equipmentIdTemplate();
	}

	int ModuleLogicCompiler::lmDescriptionNumber() const
	{
		if (m_lmDescription == nullptr)
		{
			assert(false);
			return 0;
		}

		return m_lmDescription->descriptionNumber();
	}

	bool ModuleLogicCompiler::expertMode() const
	{
		return m_context->m_expertMode;
	}

	bool ModuleLogicCompiler::generateExtraDebugInfo() const
	{
		return m_context->generateExtraDebugInfo();
	}

	void ModuleLogicCompiler::setModuleCompilersRef(const QVector<ModuleLogicCompiler*>* moduleCompilers)
	{
		TEST_PTR_LOG_RETURN(moduleCompilers, log());

		m_moduleCompilers = moduleCompilers;
	}

	bool ModuleLogicCompiler::getSignalsAndPinsLinkedToItem(const UalItem* item,
															std::set<QString>* linkedSignals,
															std::set<const UalItem*>* linkedItems,
															std::map<QUuid, const UalItem*>* linkedPins)
	{
		TEST_PTR_LOG_RETURN_FALSE(item, m_log);
		TEST_PTR_LOG_RETURN_FALSE(linkedSignals, m_log);
		TEST_PTR_LOG_RETURN_FALSE(linkedItems, m_log);

		// linkedPins can be null if not required

		bool result = true;

		const std::vector<LogicPin>& outputs = item->outputs();

		for(const LogicPin& outPin : outputs)
		{
			result &= getSignalsAndPinsLinkedToOutPin(item, outPin, linkedSignals, linkedItems, linkedPins);
		}

		return result;
	}

	std::shared_ptr<Hardware::DeviceModule> ModuleLogicCompiler::getLmSharedPtr()
	{
		TEST_PTR_LOG_RETURN_NULLPTR(m_lm, m_log);

		return 	std::dynamic_pointer_cast<Hardware::DeviceModule>(getDeviceSharedPtr(lmEquipmentID()));
	}

	std::shared_ptr<LmDescription> ModuleLogicCompiler::getLmDescription()
	{
		TEST_PTR_LOG_RETURN_NULLPTR(m_lmDescription, m_log);

		return m_lmDescription;
	}

	BusShared ModuleLogicCompiler::getBusShared(const QString& busTypeID)
	{
		return m_signals->getBus(busTypeID);
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
						 m_lmDescription->memory().m_moduleCount,
						 optoInterfaceData,
						 m_lmDescription->optoInterface().m_optoPortCount,
						 appLogicBitData,
						 tuningData,
						 appLogicWordData);

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

		m_lmAppLogicFramePayload = m_lmDescription->flashMemory().m_appLogicFramePayload;
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

		m_modules.insert(lmEquipmentID(), m);

		// chek LM subsystem ID
		//
		m_lmSubsystemKey = m_appLogicCompiler.subsystems()->ssKey(m_lmSubsystemID);

		if (m_lmSubsystemKey == -1)
		{
			// SubsystemID '%1' assigned in LM '%2' is not found in subsystem list.
			//
			m_log->errALC5056(m_lmSubsystemID, lmEquipmentID());
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
		for(int place = 1; place <= static_cast<int>(m_lmDescription->memory().m_moduleCount); place++)
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

		for(int i = 0; i < signalCount; i++)
		{
			AppSignal& s = (*m_signals)[i];

			if (s.equipmentID().isEmpty() == true)
			{
				continue;
			}

			Hardware::DeviceAppSignal* deviceAppSignal = nullptr;

			bool isIoSignal = false;

			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(s.equipmentID()).get();

			if (device == nullptr)
			{
				continue;
			}

			switch(device->deviceType())
			{
			case Hardware::DeviceType::Module:
				{
					Hardware::DeviceModule* deviceModule = device->toModule().get();

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

					if (deviceModule->equipmentIdTemplate() != lmEquipmentID())
					{
						continue;
					}

					// signal is associated with current LM

					isIoSignal = false;
				}
				break;

			case Hardware::DeviceType::AppSignal:
				{
					deviceAppSignal = device->toAppSignal().get();

					if (deviceAppSignal == nullptr)
					{
						assert(false);
						continue;
					}

					const Hardware::DeviceChassis* chassis = deviceAppSignal->getParentChassis();

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

			m_chassisSignals.insert(s.appSignalID(), &s);

			if (isIoSignal == true)
			{
				m_ioSignals.insert(s.appSignalID(), &s);

				if (deviceAppSignal != nullptr)
				{
					m_equipmentSignals.insert(deviceAppSignal->equipmentIdTemplate(), &s);
				}
				else
				{
					assert(false);
				}
			}
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

				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Duplicate GUID %1 of %2 and %3 elements")).
										arg(logicItem.m_fblItem->guid().toString()).arg(firstItem->strID()).
										arg(getUalItemStrID(logicItem)));

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
			for(const LogicPin& input : appItem->inputs())
			{
				UalItem* firstItem = m_pinParent.value(input.guid(), nullptr);

				if (firstItem != nullptr)
				{

					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Duplicate input pin GUID %1 of %2 and %3 elements")).
											arg(input.guid().toString()).
											arg(firstItem->strID()).arg(appItem->strID()));

					result = false;

					continue;
				}

				m_pinParent.insert(input.guid(), appItem);
			}

			// add output pins
			//
			for(const LogicPin& output : appItem->outputs())
			{
				UalItem* firstItem = m_pinParent.value(output.guid(), nullptr);

				if (firstItem != nullptr)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Duplicate output pin GUID %1 of %2 and %3 elements")).
											arg(output.guid().toString()).
											arg(firstItem->strID()).
											arg(appItem->strID()));

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
		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem->isAfb() == false)
			{
				continue;
			}

			UalAfb* ualAfb = createUalAfb(*ualItem);

			if (ualAfb == nullptr)
			{
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createUalSignals()
	{
		m_ualSignals.clear();
		m_outPinSignal.clear();

		bool result = true;

		result &= writeUalItemsFile();

		result &= loopbacksPreprocessing();

		// primarily created signals
		//
		result &= createUalSignalsFromInputAndTuningAcquiredSignals();
		result &= createUalSignalsFromBusComposers();
		result &= createUalSignalsFromReceivers();
		result &= createUalSignalsFromOptoValidity();

		RETURN_IF_FALSE(result);

		// secondary created signals
		//
		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			switch(ualItem->type())
			{
			// UAL items that can generate signals
			//
			case E::UalItemType::Signal:
				result &= createUalSignalFromSignal(ualItem, 1);
				break;

			case E::UalItemType::Const:
				result &= createUalSignalFromConst(ualItem);
				break;

			case E::UalItemType::Afb:
				result &= createUalSignalsFromAfbOuts(ualItem);
				break;

			case E::UalItemType::BusExtractor:
				result &= linkUalSignalsFromBusExtractor(ualItem);
				break;

			// UAL items already processed
			//
			case E::UalItemType::BusComposer:
			case E::UalItemType::Receiver:
				break;

			// UAL items that doesn't generate signals
			//
			case E::UalItemType::Transmitter:
			case E::UalItemType::Terminator:
			case E::UalItemType::LoopbackSource:
			case E::UalItemType::LoopbackTarget:
				break;

			// unknown item's type
			//
			case E::UalItemType::Unknown:
			default:
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		RETURN_IF_FALSE(result);

		result &= checkLoopbacks();

		RETURN_IF_FALSE(result);

		result &= linkLoopbackTargets();

		RETURN_IF_FALSE(result);

		// link signals connected to loopback targets
		//
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

		result &= checkBusProcessingItemsConnections();

		RETURN_IF_FALSE(result);

		signalSet()->buildID2IndexMap();

		return result;
	}

	bool ModuleLogicCompiler::writeUalItemsFile()
	{
		if (m_context->generateExtraDebugInfo() == false)
		{
			return true;
		}

		QStringList items;

		for(const UalItem* ualItem : m_ualItems)
		{
			QString itemStr;

			E::UalItemType itemType = ualItem->type();

			itemStr = QString("%1, %2").arg(E::valueToString(itemType)).arg(ualItem->label());

			if (itemType == E::UalItemType::Signal)
			{
				itemStr += QString(", %1").arg(ualItem->strID());
			}

			items.append(itemStr);
		}

		return m_resultWriter->addFile(m_lmSubsystemID, lmEquipmentID() + ".uil", "", "", items, false);
	}


	bool ModuleLogicCompiler::loopbacksPreprocessing()
	{
		bool result = true;

		result = findAndProcessSingleItemLoopbacks();

		RETURN_IF_FALSE(result);

		result = findLoopbackSources();

		RETURN_IF_FALSE(result);

		result = findLoopbackTargets();

		RETURN_IF_FALSE(result);

		result = m_loopbacks.findSignalsAndPinsLinkedToLoopbacksTargets();

		return result;
	}

	bool ModuleLogicCompiler::findAndProcessSingleItemLoopbacks()
	{
		bool result = true;

		QVector<UalItem*> createdItems;

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isAfb() == false)
			{
				continue;
			}

			std::vector<LogicPin>& outputs = ualItem->outputs();

			for(LogicPin& output : outputs)
			{
				QVector<QUuid> connectedInputsGuids;

				getInputsDirectlyConnectedToOutput(ualItem, output, &connectedInputsGuids);

				if (connectedInputsGuids.isEmpty() == true)
				{
					continue;
				}

				QString autoLoopbackID = Loopbacks::getAutoLoopbackID(ualItem, output);;
				QString autoLoopbackTargetLabel;

				QString connectedLoopbackSourceID = getConnectedLoopbackSourceID(output);

				if (connectedLoopbackSourceID.isEmpty() == true)
				{
					// no LoopbackSources connected to output, create auto LoopbackSource
					//
					QString autoLoopbackSourceLabel = autoLoopbackID;
					autoLoopbackSourceLabel.replace("AUTO_LOOPBACK", "AUTO_LOOPBACK_SOURCE");

					autoLoopbackTargetLabel = autoLoopbackID;
					autoLoopbackTargetLabel.replace("AUTO_LOOPBACK", "AUTO_LOOPBACK_TARGET");

					// LoopbackSource creation
					//
					std::shared_ptr<UalLoopbackSource> loopbackSource = std::make_shared<UalLoopbackSource>();
					loopbackSource->setLoopbackId(autoLoopbackID);
					loopbackSource->setLabel(autoLoopbackSourceLabel);

					UalItem* newSourceUalItem = new UalItem(AppLogicItem(loopbackSource, ualItem->schema()));
					createdItems.append(newSourceUalItem);
					m_pinParent.insert(loopbackSource->inputs()[0].guid(), newSourceUalItem);

					// link ualItem output and loopback source input to each other
					//
					output.AddAssociattedIOs(loopbackSource->inputs()[0].guid());
					loopbackSource->inputs()[0].AddAssociattedIOs(output.guid());
				}
				else
				{
					// has LoopbackSource connected to ouput, auto LoopbackSource is not requred
					// init variables for auto LoopbackTarget creation only
					//
					autoLoopbackTargetLabel = autoLoopbackID;
					autoLoopbackTargetLabel.replace("AUTO_LOOPBACK", "AUTO_LOOPBACK_TARGET");

					autoLoopbackID = connectedLoopbackSourceID;		// use existings LoopbackID
				}

				// LoopbackTargets creation
				//
				for(const QUuid& inputGuid : connectedInputsGuids)
				{
					LogicPin& input = ualItem->input(inputGuid);

					// break output to input link
					//
					output.removeFromAssociatedIo(input.guid());
					input.removeFromAssociatedIo(output.guid());

					// LoopbackTarget creation
					//
					std::shared_ptr<UalLoopbackTarget> loopbackTarget = std::make_shared<UalLoopbackTarget>();
					loopbackTarget->setLoopbackId(autoLoopbackID);
					loopbackTarget->setLabel(autoLoopbackTargetLabel);

					UalItem* newTargetUalItem = new UalItem(AppLogicItem(loopbackTarget, ualItem->schema()));
					createdItems.append(newTargetUalItem);
					m_pinParent.insert(loopbackTarget->outputs()[0].guid(), newTargetUalItem);

					// link loopback target output and ualItem input to each other
					//
					loopbackTarget->outputs()[0].AddAssociattedIOs(input.guid());
					input.AddAssociattedIOs(loopbackTarget->outputs()[0].guid());
				}
			}
		}

		for(UalItem* createItem : createdItems)
		{
			m_ualItems.insert(createItem->guid(), createItem);
		}

		return result;
	}

	void ModuleLogicCompiler::getInputsDirectlyConnectedToOutput(const UalItem* ualItem,
														 const LogicPin& output,
														 QVector<QUuid>* connectedInputsGuids)
	{
		TEST_PTR_RETURN(ualItem);
		TEST_PTR_RETURN(connectedInputsGuids);

		const std::vector<QUuid>& associatedIOsGuids = output.associatedIOs();

		for(const QUuid& pinGuid : associatedIOsGuids)
		{
			UalItem* pinParent = m_pinParent.value(pinGuid, nullptr);

			if (pinParent == nullptr)
			{
				assert(false);
				continue;
			}

			if (pinParent == ualItem)
			{
				connectedInputsGuids->append(pinGuid);
			}
		}
	}

	QString ModuleLogicCompiler::getConnectedLoopbackSourceID(const LogicPin& output)
	{
		const std::vector<QUuid>& associatedIOsGuids = output.associatedIOs();

		for(const QUuid& pinGuid : associatedIOsGuids)
		{
			UalItem* pinParent = m_pinParent.value(pinGuid, nullptr);

			if (pinParent == nullptr)
			{
				assert(false);
				continue;
			}

			if (pinParent->isLoopbackSource() == true)
			{
				const UalLoopbackSource* src = pinParent->ualLoopbackSource();

				if (src != nullptr)
				{
					return src->loopbackId();
				}

				assert(false);
			}
		}

		return QString();
	}

	bool ModuleLogicCompiler::findLoopbackSources()
	{
		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isLoopbackSource() == false)
			{
				continue;
			}

			const UalLoopbackSource* source = ualItem->ualLoopbackSource();

			if (source == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			QString loopbackSourceID = source->loopbackId();

			if (m_loopbacks.isSourceExists(loopbackSourceID) == true)
			{
				// Duplicate loopback source ID %1 (Logic schema %2).
				//
				m_log->errALC5142(loopbackSourceID, ualItem->guid(), ualItem->schemaID());
				result = false;
				continue;
			}

			result &= m_loopbacks.addLoopbackSource(ualItem);
		}

		return result;
	}

	bool ModuleLogicCompiler::findLoopbackTargets()
	{
		bool result = true;

		for(const UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isLoopbackTarget() == false)
			{
				continue;
			}

			const UalLoopbackTarget* target = ualItem->ualLoopbackTarget();

			if (target == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			QString loopbackID = target->loopbackId();

			if (m_loopbacks.isSourceExists(loopbackID) == false)
			{
				// LoopbackSource is not exists for LoopbackTarget with ID %1 (Logic schema %2).
				//
				m_log->errALC5143(loopbackID, ualItem->guid(), ualItem->schemaID());
				result = false;
				continue;
			}

			bool res = m_loopbacks.addLoopbackTarget(loopbackID, ualItem);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::getSignalsAndPinsLinkedToOutPin(const UalItem* ualItem,
															const LogicPin& outPin,
															std::set<QString>* linkedSignals,
															std::set<const UalItem*>* linkedItems,
															std::map<QUuid, const UalItem*>* linkedPins)
	{
		TEST_PTR_LOG_RETURN_FALSE(linkedSignals, m_log);
		TEST_PTR_LOG_RETURN_FALSE(linkedItems, m_log);

		// linkedPins can be null if not required

		bool result = true;

		const std::vector<QUuid>& associatedInPins = outPin.associatedIOs();

		if (linkedPins != nullptr)
		{
			linkedPins->insert(std::pair<QUuid, const UalItem*>(outPin.guid(), ualItem));
		}

		for(QUuid inPin : associatedInPins)
		{
			const UalItem* linkedItem = m_pinParent.value(inPin, nullptr);

			if (linkedItem == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				continue;
			}

			linkedItems->insert(linkedItem);

			if (linkedPins != nullptr)
			{
				linkedPins->insert(std::pair<QUuid, const UalItem*>(inPin, linkedItem));
			}

			if (linkedItem->isSignal() == false)
			{
				// linkedItem is not a Signal - add linked item and pin only
				//
				continue;
			}

			// linkedItem is a Signal
			//
			QString signalID = linkedItem->strID();

			bool signalAlreadyLinked = linkedSignals->count(signalID) > 0;

			if (signalAlreadyLinked == false)
			{
				linkedSignals->insert(signalID);
			}

			if (linkedItem->outputs().size() > 0)
			{
				// Signal has output - link next items
				//
				result &= getSignalsAndPinsLinkedToItem(linkedItem, linkedSignals, linkedItems, linkedPins);
			}

			if (signalAlreadyLinked == false)
			{
				// scan all ualItems (on all schemas!!!) and find SignalItems with signalID that is not in linkedItems
				// (heavy operation !!!)

				for(const UalItem* item : m_ualItems)
				{
					TEST_PTR_CONTINUE(item);

					if (item->isSignal() == false)
					{
						continue;
					}

					if (item->strID() != signalID)
					{
						continue;
					}

					if (linkedItems->count(item) > 0)
					{
						continue;
					}

					linkedItems->insert(item);

					result &= getSignalsAndPinsLinkedToItem(item, linkedSignals, linkedItems, linkedPins);
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalsFromInputAndTuningAcquiredSignals()
	{
		bool result = true;

		// fill m_ualSignals by Input and Tuning Acquired signals
		//
		for(AppSignal* appSignal : m_chassisSignals)
		{
			if (appSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (appSignal->isAcquired() == false)
			{
				continue;
			}

			if (appSignal->isInput() == true)
			{
				m_ualSignals.createSignal(appSignal);
				continue;
			}

			if (appSignal->enableTuning() == true)
			{
				Q_ASSERT(appSignal->isInternal() == true || appSignal->isOutput() == true);
				m_ualSignals.createSignal(appSignal);
				continue;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalsFromBusComposers()
	{
		bool result = true;

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

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalsFromBusComposer(UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

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

		AppSignal* connectedBusSignal = getCompatibleConnectedBusSignal(outPin, busTypeID);

		// connectedBusSignal can be nullptr here, it is Ok! AUTO bus signal will be created

		UalSignal* busParentSignal = createBusParentSignal(ualItem, outPin, connectedBusSignal, busTypeID);

		if (busParentSignal == nullptr)
		{
			return false;
		}

		bool result = linkConnectedItems(ualItem, outPin, busParentSignal);

		return result;
	}

	UalSignal* ModuleLogicCompiler::createBusParentSignal(UalItem* ualItem, const LogicPin& outPin, AppSignal* appBusSignal, const QString& busTypeID)
	{
		BusShared bus = m_signals->getBus(busTypeID);

		if (bus == nullptr)
		{
			// Bus type ID '%1' is undefined (Logic schema '%2').
			//
			m_log->errALC5100(busTypeID, ualItem->guid(), ualItem->schemaID());
			return nullptr;
		}

		std::shared_ptr<Hardware::DeviceModule> lm = getLmSharedPtr();

		if (lm == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		UalSignal* busParentSignal = m_ualSignals.createBusParentSignal(appBusSignal, bus, ualItem, outPin.guid(), outPin.caption());

		return busParentSignal;
	}

	UalSignal* ModuleLogicCompiler::createBusParentSignalFromBusExtractorConnectedToDiscreteSignal(UalItem* ualItem)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		const UalBusExtractor* busExtractor = ualItem->ualBusExtractor();

		if (busExtractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const std::vector<LogicPin>& inputs = busExtractor->inputs();

		if (inputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const LogicPin& inPin = inputs[0];

		QString busTypeID = busExtractor->busTypeId();

		// connectedBusSignal can be nullptr here, it is Ok! AUTO bus signal will be created

		UalSignal* busParentSignal = createBusParentSignal(ualItem, inPin, nullptr, busTypeID);

		return busParentSignal;
	}

	bool ModuleLogicCompiler::createUalSignalsFromReceivers()
	{
		bool result = true;

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

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalsFromReceiver(UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		const UalReceiver* ualReceiver = ualItem->ualReceiver();

		if (ualReceiver == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString connectionID;

		bool res = getReceiverConnectionID(ualReceiver, &connectionID, ualItem->schemaID());

		if (res == false)
		{
			return false;
		}

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

		for(int i = 0; i < static_cast<int>(outputs.size()); i++)
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

		if (ualReceiver->appSignalIdsAsList().size() > 1)
		{
			m_log->errINT1001(QString("SchemaItemReceiver has more then one AppSignalID"), ualItem->schemaID(), ualItem->guid());
			return false;
		}

		QString appSignalID = ualReceiver->appSignalIds();

		bool result = true;

		result &= createUalSignalFromReceiverOutput(ualItem, outPin, appSignalID, connection->isSinglePort());

		// UalSignal creation from receiver's validity pin
		//

		if (validityPinIndex == -1)
		{
			return result;						// receiver hasn't validity pin, it is ok
		}

		const LogicPin& validityPin = outputs[validityPinIndex];

		result &= createUalSignalFromReceiverValidity(ualItem, validityPin, connection);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromReceiverOutput(UalItem* ualItem, const LogicPin& outPin, const QString& receivedAppSignalID, bool isSinglePortConnection)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		bool connectedToTerminatorOnly = isConnectedToTerminatorOnly(outPin);

		if (connectedToTerminatorOnly == true)
		{
			return true;				// not needed to create signal
		}

		bool result = true;

		AppSignal* receivedSignal = m_signals->getSignal(receivedAppSignalID);

		if (receivedSignal == nullptr)
		{
			// Signal identifier %1 is not found (Logic schema %2).
			//
			m_log->errALC5000(receivedAppSignalID, ualItem->guid(), ualItem->schemaID());
			return false;
		}

		AppSignal* compatibleConnectedSignal = nullptr;

		if (isSinglePortConnection == true)
		{
			// in single port connection receiving signals should be native for LM that receive
			//
			compatibleConnectedSignal = receivedSignal;
		}
		else
		{
			if (m_chassisSignals.contains(receivedAppSignalID) == true)
			{
				// LM's %1 native signal %2 can't be received via opto connection (Logic schema %3)
				//
				m_log->errALC5170(lmEquipmentID(), receivedAppSignalID, ualItem->guid(), ualItem->schemaID());
				return false;
			}

			compatibleConnectedSignal = getCompatibleConnectedSignal(outPin, *receivedSignal);
		}

		// after that compatibleConnectedSignal can be nullptr, it is Ok

		UalSignal* ualSignal = nullptr;

		switch(receivedSignal->signalType())
		{
		case E::SignalType::Analog:
		case E::SignalType::Discrete:
			if (compatibleConnectedSignal == nullptr)
			{
				ualSignal = m_ualSignals.createAutoSignal(ualItem, outPin.guid(), *receivedSignal);
			}
			else
			{
				ualSignal = m_ualSignals.createSignal(compatibleConnectedSignal, ualItem, outPin.guid());
			}
			break;

		case E::SignalType::Bus:
			{
				BusShared bus = m_signals->getBus(receivedSignal->busTypeID());

				if (bus == nullptr)
				{
					// Bus type ID '%1' is undefined (Logic schema '%2').
					//
					m_log->errALC5100(receivedSignal->busTypeID(), ualItem->guid(), ualItem->schemaID());
					return false;
				}

				if (compatibleConnectedSignal != nullptr && compatibleConnectedSignal->isBus() == false)
				{
					assert(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				std::shared_ptr<Hardware::DeviceModule> lm = getLmSharedPtr();

				if (lm == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				ualSignal = m_ualSignals.createBusParentSignal(compatibleConnectedSignal, bus, ualItem,
															   outPin.guid(), outPin.caption());
			}
			break;

		default:
			assert(false);
		}

		if (ualSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Receiver %1 schema %2").arg(ualItem->label()).arg(ualItem->schemaID()));
			return false;
		}

		ualSignal->setReceivedOptoAppSignalID(receivedAppSignalID);

		result &= linkConnectedItems(ualItem, outPin, ualSignal);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromReceiverValidity(UalItem* ualItem,
																  const LogicPin& validityPin,
																  std::shared_ptr<Hardware::Connection> connection)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);
		TEST_PTR_LOG_RETURN_FALSE(connection, m_log);

		if (isConnectedToTerminatorOnly(validityPin) == true)
		{
			return true;								// ualSignal is not required
		}

		Hardware::OptoPortShared port = m_optoModuleStorage->getLmAssociatedOptoPort(lmEquipmentID(), connection->connectionID());

		if (port == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);					// port isn't found
			return false;
		}

		QString validitySignalEquipmentID = port->validitySignalEquipmentID();

		AppSignal* validityAppSignal = m_equipmentSignals.value(validitySignalEquipmentID);

		if (validityAppSignal == nullptr)
		{
			m_log->errALC5133(validitySignalEquipmentID, ualItem->guid(), ualItem->label(), ualItem->schemaID());
			return false;
		}

		UalSignal* ualSignal = m_ualSignals.get(validityAppSignal->appSignalID());

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
			ualSignal = m_ualSignals.createSignal(validityAppSignal, ualItem, validityPin.guid());

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

	bool ModuleLogicCompiler::getReceiverConnectionID(const UalReceiver* receiver, QString* connectionID, const QString& schemaID)
	{
		TEST_PTR_RETURN_FALSE(receiver);
		TEST_PTR_RETURN_FALSE(connectionID);

		const QStringList& ids = receiver->connectionIdsAsList();

		if (ids.count() == 1)
		{
			*connectionID = ids.first();
			return true;
		}

		if (ids.count() == 0)
		{
			// Receiver has no connection ID (Schema %1, module %2)
			//
			m_log->errALC5159(receiver->guid(), schemaID, lmEquipmentID());
			return false;
		}

		if (ids.count() > 1)
		{
			// Receiver has more then one connections ID (Schema %1, module %2)
			//
			m_log->errALC5161(receiver->guid(), schemaID, lmEquipmentID());
		}

		return false;
	}

	bool ModuleLogicCompiler::createUalSignalsFromOptoValidity()
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_optoModuleStorage, m_log);

		//
		// Append OptoValidity signals for ALL opto ports in current LM and associated OptoModules
		//

		bool result = true;

		QList<Hardware::OptoPortShared> lmAssociatedPorts;

		result = m_optoModuleStorage->getLmAssociatedOptoPorts(lmEquipmentID(), lmAssociatedPorts);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(result, m_log);

		for(Hardware::OptoPortShared optoPort : lmAssociatedPorts)
		{
			if (optoPort == nullptr)
			{
				result = false;
				continue;
			}

			QString validitySignalEquipmentID = optoPort->validitySignalEquipmentID();
			Address16 validitySignalAbsAddr = optoPort->validitySignalAbsAddr();

			for(AppSignal* appSignal : m_chassisSignals)
			{
				if (appSignal == nullptr)
				{
					LOG_NULLPTR_ERROR(m_log);
					result = false;
					continue;
				}

				if (appSignal->isAcquired() == false)
				{
					continue;
				}

				if (appSignal->isInput() == true)
				{
					m_ualSignals.createSignal(appSignal);
					continue;
				}

		}

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(result, m_log);


		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromSignal(UalItem* ualItem, int passNo)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		if (ualItem->isSignal() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString signalID = ualItem->strID();

		AppSignal* appSignal = m_signals->getSignal(signalID);

		if (appSignal == nullptr)
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
			// UalSignal already created for this signalID
			// add ref pin and link connected items
			//
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

		// UalSignal is not yet created
		//

		// Only Input and Tunable signals really can generate UalSignal
		//
		if (appSignal->isInput() == false && appSignal->enableTuning() == false)
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
			// Can't assign value to input or tunable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5121(appSignal->appSignalID(), ualItem->guid(), ualItem->schemaID());
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

		switch(appSignal->signalType())
		{
		case E::SignalType::Analog:
		case E::SignalType::Discrete:
			ualSignal = m_ualSignals.createSignal(appSignal, ualItem, outPin.guid());
			break;

		case E::SignalType::Bus:
			{
				BusShared bus = getBusShared(appSignal->busTypeID());

				if (bus == nullptr)
				{
					// Bus type ID %1 of signal %2 is undefined.
					//
					m_log->errALC5092(appSignal->busTypeID(), appSignal->appSignalID());
					return false;
				}

				ualSignal = m_ualSignals.createBusParentSignal(appSignal, bus, ualItem,
																	outPin.guid(), outPin.caption());
			}
			break;

		default:
			Q_ASSERT(false);
		}

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
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

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
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		UalAfb* ualAfb = m_ualAfbs.value(ualItem->guid(), nullptr);

		if (ualAfb == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (ualAfb->isSetFlagsItem() == true)
		{
			return true;			// set_flags item isn't create signal
		}

		bool result = true;

		QString outBusTypeID;
		BusShared bus;

		if (ualAfb->isBusProcessing() == true)
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

			AppSignal* connectedSignal = getCompatibleConnectedSignal(outPin, outAfbSignal, outBusTypeID);

			switch(outAfbSignal.type())
			{
			case E::SignalType::Analog:
			case E::SignalType::Discrete:
				if (connectedSignal == nullptr)
				{
					std::optional<int> expectedReadCount = getOutPinExpectedReadCount(outPin);

					ualSignal = m_ualSignals.createAutoSignal(ualItem, outPin.guid(), outAfbSignal, expectedReadCount);
				}
				else
				{
					ualSignal = m_ualSignals.createSignal(connectedSignal, ualItem, outPin.guid());
				}
				break;

			case E::SignalType::Bus:
				{
					std::shared_ptr<Hardware::DeviceModule> lm = getLmSharedPtr();

					if (lm == nullptr)
					{
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					ualSignal = m_ualSignals.createBusParentSignal(connectedSignal, bus, ualItem, outPin.guid(), outAfbSignal.caption());
				}
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

	bool ModuleLogicCompiler::linkUalSignalsFromBusExtractor(UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

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

			m_ualSignals.appendRefPin(ualItem, outPin.guid(), busChildSignal);
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
			case E::UalItemType::Signal:
				result &= linkSignal(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			case E::UalItemType::Afb:
				result &= linkAfbInput(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			case E::UalItemType::BusComposer:
				result &= linkBusComposerInput(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			case E::UalItemType::BusExtractor:
				result &= linkBusExtractorInput(srcUalItem, destUalItem, inPinUuid, ualSignal);
				break;

			// link pins to signal only, any checks is not required
			//
			case E::UalItemType::Transmitter:
				m_ualSignals.appendRefPin(destUalItem, inPinUuid, ualSignal);
				break;

			case E::UalItemType::LoopbackSource:
				result &= linkLoopbackSource(destUalItem, inPinUuid, ualSignal);
				break;

			case E::UalItemType::Terminator:
				break;

			// output can't connect to:
			//
			case E::UalItemType::Const:
			case E::UalItemType::Receiver:
			case E::UalItemType::LoopbackTarget:
				m_log->errALC5116(srcUalItem->guid(), destUalItem->guid(), destUalItem->schemaID());
				result = false;
				break;

			// unknown UalItem type
			//
			case E::UalItemType::Unknown:
			default:
				assert(false);
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::linkSignal(UalItem* srcItem, UalItem* signalItem, QUuid inPinUuid, UalSignal* srcUalSignal)
	{
		if (srcItem == nullptr || signalItem == nullptr || srcUalSignal == nullptr || srcUalSignal->signal() == nullptr)
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

		AppSignal* s = m_signals->getSignal(signalID);

		if (s == nullptr)
		{
			m_log->errALC5000(signalID, signalItem->guid(), signalItem->schemaID());
			return false;
		}

		if (s->isInput() == true)
		{
			// Can't assign value to input signal %1 (Logic schema %2).
			//
			m_log->errALC5087(signalItem->schemaID(), s->appSignalID(), signalItem->guid());
			return false;
		}

		if (s->enableTuning() == true)
		{
			// Can't assign value to tunable signal '%1' (Logic schema '%2').
			//
			m_log->errALC5071(signalItem->schemaID(), s->appSignalID(), signalItem->guid());
			return false;
		}

		UalSignal* existsSignal = m_ualSignals.get(s->appSignalID());

		if (existsSignal != nullptr && existsSignal != srcUalSignal && existsSignal->isSource() == true)
		{
			// Can't assign value to input/tunable/opto/const signal %1 (Logic schema %2).
			//
			m_log->errALC5121(s->appSignalID(), signalItem->guid(), signalItem->schemaID());
			return false;
		}

		// check signals compatibility
		//
		bool result = srcUalSignal->isCompatible(s, log());

		if (result == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(srcItem->guid(), srcItem->label(), signalItem->guid(), signalItem->label(), signalItem->schemaID());
			return false;
		}

/*		if (s->isOutput() == true)
		{
			// create separate UAL signal for each output signal

			UalSignal* outUalSignal = m_ualSignals.createSignal(s);

			if (result == false)
			{
				return false;
			}

			result = m_ualSignals.appendRefPin(signalItem, inPinUuid, outUalSignal);

			if (result == false)
			{
				return false;
			}

			m_outUalSignals.insert(outUalSignal, srcUalSignal);
		}
		else
		{
			result = m_ualSignals.appendRefPin(signalItem, inPinUuid, srcUalSignal);

			if (result == false)
			{
				return false;
			}

			result = m_ualSignals.appendRefSignal(s, srcUalSignal);

			if (result == false)
			{
				return false;
			}
		}*/

		//

		result = m_ualSignals.appendRefPin(signalItem, inPinUuid, srcUalSignal);

		if (result == false)
		{
			return false;
		}

		result = m_ualSignals.appendRefSignal(s, srcUalSignal);

		if (result == false)
		{
			return false;
		}

		//

		const std::vector<LogicPin>& outputs = signalItem->outputs();

		if (outputs.size() > 1)
		{
			LOG_INTERNAL_ERROR(m_log);				// signal cannot have more then 1 output
			return false;
		}

		if (outputs.size() == 1)
		{
			const LogicPin& output = outputs[0];

			m_ualSignals.appendRefPin(signalItem, output.guid(), srcUalSignal);

			// recursive linking of items
			//
			result = linkConnectedItems(signalItem, output, srcUalSignal);
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

		if (ualAfb->isSetFlagsItem() == true && inSignal.caption() == UalAfb::IN_PIN_CAPTION)
		{
			// processing for set_flags item "in" pin is different
			//
			return linkSetFlagsItemInput(srcItem, destItem, inPinUuid, ualSignal);
		}

		result = ualSignal->isCanBeConnectedTo(*destItem, inSignal, log());

		if (result == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(srcItem->guid(), srcItem->label(), destItem->guid(), destItem->label(), destItem->schemaID());
			return false;
		}

		return m_ualSignals.appendRefPin(srcItem, inPinUuid, ualSignal);
	}

	bool ModuleLogicCompiler::linkSetFlagsItemInput(UalItem* srcItem, UalItem* setFlagsItem, QUuid inPinUuid, UalSignal* ualSignal)
	{
		LOG_IF_NULLPTR_RETURN_FALSE(srcItem, m_log);
		LOG_IF_NULLPTR_RETURN_FALSE(setFlagsItem, m_log);
		LOG_IF_NULLPTR_RETURN_FALSE(ualSignal, m_log);

		// linking of set_flags item "in" pin differences:
		//
		// 1) signal type compatibility check is not required. Any type of signal can be connected to "in" of set_flags
		// 2) "in" of set_flags item is directly connected to "out". So "out" set_flags item is not create new Ual signal.
		//    Items connected to "out" also should be linked to "in"
		//

		UalAfb* ualAfb = m_ualAfbs.value(setFlagsItem->guid(), nullptr);

		LOG_IF_NULLPTR_RETURN_FALSE(ualAfb, m_log);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(ualAfb->isSetFlagsItem(), m_log);

		const LogicPin* inPin = ualAfb->getPin(inPinUuid);

		LOG_IF_NULLPTR_RETURN_FALSE(inPin, m_log);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(inPin->caption() == UalAfb::IN_PIN_CAPTION, log());

		if (ualSignal->isConst() == true)
		{
			// Setting of flags to a constant signal (Logic schema %1).
			//
			m_log->wrnALC5178(srcItem->guid(), setFlagsItem->guid(), setFlagsItem->schemaID());
		}

		bool result = m_ualSignals.appendRefPin(setFlagsItem, inPin->guid(), ualSignal);

		RETURN_IF_FALSE(result);

		// linking items connected to "out"
		//
		const std::vector<LogicPin>& outputs = ualAfb->outputs();

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(outputs.size() == 1, log());

		const LogicPin& outPin = outputs[0];

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(outPin.caption() == UalAfb::OUT_PIN_CAPTION, log());

		result = m_ualSignals.appendRefPin(setFlagsItem, outPin.guid(), ualSignal);

		RETURN_IF_FALSE(result);

		result = linkConnectedItems(srcItem, outPin, ualSignal);

		return result;
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

		if (ualSignal->isCompatible(bus, busSignal, log()) == false)
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

		// Discrete signal is connected to bus with discrete in/outs only
		//
		if (ualSignal->isDiscrete() &&
			bus->busDataFormat() == E::BusDataFormat::Discrete)
		{
			UalSignal* busParentSignal = createBusParentSignalFromBusExtractorConnectedToDiscreteSignal(busExtractorItem);

			if (busParentSignal == nullptr)
			{
				return false;
			}

			bool result = linkUalSignalsFromBusExtractor(busExtractorItem);

			result &= m_ualSignals.appendRefPin(busExtractorItem, inPinUuid, busParentSignal);

			return result;
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

	bool ModuleLogicCompiler::linkLoopbackSource(UalItem* loopbackSourceItem, QUuid inPinUuid, UalSignal* ualSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(loopbackSourceItem, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

		assert(loopbackSourceItem->label().isEmpty() == false);

		m_loopbacks.setUalSignalForLoopback(loopbackSourceItem, ualSignal);

		m_ualSignals.appendRefPin(loopbackSourceItem, inPinUuid, ualSignal);

		return true;
	}

	bool ModuleLogicCompiler::checkLoopbacks()
	{
		return m_loopbacks.checkLoopbacksUalSignals();
	}

	bool ModuleLogicCompiler::linkLoopbackTargets()
	{
		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ualItem->type() != E::UalItemType::LoopbackTarget)
			{
				continue;
			}

			result &= linkLoopbackTarget(ualItem);
		}

		return result;
	}

	bool ModuleLogicCompiler::linkLoopbackTarget(UalItem* loopbackTargetItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(loopbackTargetItem, m_log);

		const UalLoopbackTarget* target = loopbackTargetItem->ualLoopbackTarget();

		if  (target == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString loopbackID = target->loopbackId();

		UalSignal* loopbackUalSignal = m_loopbacks.getLoopbackUalSignal(loopbackID);

		if (loopbackUalSignal == nullptr)
		{
			// This is a critical error!
			// Loopback source signal is't connected to any signal source (is not initialized)
			//
			// Corresponding message will display during execution createUalSignalFromSignal(...) on pass 2.
			// So, to continue compilation we return TRUE here!
			//
			return true;
		}

		const std::vector<LogicPin>& outputs = loopbackTargetItem->outputs();

		if (outputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		// link connected items to loopback source signal
		//
		bool result = linkConnectedItems(loopbackTargetItem, outputs[0], loopbackUalSignal);

		return result;
	}

	bool ModuleLogicCompiler::checkBusProcessingItemsConnections()
	{
		bool result = true;

		for(const UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

			if (ualItem->isAfb() == false)
			{
				continue;
			}

			const UalAfb* afb = m_ualAfbs.value(ualItem->guid(), nullptr);

			TEST_PTR_LOG_RETURN_FALSE(afb, m_log);

			if (afb->isBusProcessing() == false)
			{
				continue;
			}

			QString inBusType;
			QString outBusType;

			determineBusTypeByInputs(afb, &inBusType);
			determineBusTypeByOutput(afb, &outBusType);

			if ((inBusType.isEmpty() == true && outBusType.isEmpty() == true) ||
				(inBusType.isEmpty() == false && outBusType.isEmpty() == false && inBusType != outBusType))
			{
				// Output bus type cannot be determined (Logic schema %1, item %2)
				//
				m_log->errALC5127(afb->guid(), afb->label(), afb->schemaID());
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::processSignalsWithFlags()
	{
		m_signalsWithFlagsIDs.clear();

		bool result = true;

		result &= processAcquiredIOSignalsValidity();
		result &= processSimlockItems();
		result &= processMismatchItems();
		result &= processSetFlagsItems();

		result &= setAcquiredForFlagSignals();

		result &= checkSignalsWithFlags();

		writeSignalsWithFlagsReport();

		return result;
	}

	bool ModuleLogicCompiler::processAcquiredIOSignalsValidity()
	{
		bool result = true;

		QVector<QPair<AppSignal*, QString>> acquiredInputSignalToLinkedValiditySignalID;	// Acquired input signal => linked validity signal EquipmentID

		for(const AppSignal* s : m_ioSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false)
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal == nullptr)
			{
				continue;				// this signal is not used in UAL, continue
			}

			QString signalEquipmentID = s->equipmentID();

			if (s->equipmentID().isEmpty() == true)
			{
				assert(false);
				continue;
			}

			assert(s->isInput() == true || s->isOutput() == true);

			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(signalEquipmentID).get();

			TEST_PTR_CONTINUE(device);

			Hardware::DeviceAppSignal* deviceAppSignal = device->toAppSignal().get();

			TEST_PTR_CONTINUE(deviceAppSignal);

			QString validitySignalEquipmentID = deviceAppSignal->validitySignalId();

			if (validitySignalEquipmentID.isEmpty() == true)
			{
				continue;
			}

			AppSignal* linkedValiditySignal = m_equipmentSignals.value(validitySignalEquipmentID, nullptr);

			if (linkedValiditySignal == nullptr)
			{
				// Linked validity app signal with EquipmentID %1 is not found (input signal %2).
				//
				m_log->errALC5155(validitySignalEquipmentID, s->appSignalID());
				result = false;
				continue;
			}

			if (linkedValiditySignal->isInput() == false ||
				linkedValiditySignal->isDiscrete() == false)
			{
				// Linked validity signal %1 shoud have Discrete Input type (input signal %2).
				//
				m_log->errALC5156(linkedValiditySignal->appSignalID(), s->appSignalID());
				result = false;
				continue;
			}

			UalSignal* ualValiditySignal = m_ualSignals.get(linkedValiditySignal->appSignalID());

			if (ualValiditySignal == nullptr)
			{
				m_ualSignals.createSignal(linkedValiditySignal);
			}

			bool res = appendFlagToSignal(	s->appSignalID(),
											E::AppSignalStateFlagType::Validity,
											linkedValiditySignal->appSignalID(),
											nullptr);
			result &= res;
		}

		return result;
	}

	bool ModuleLogicCompiler::processSimlockItems()
	{
		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSimLockItem() == false)
			{
				continue;
			}

			if (ualItem->assignFlags() == false)
			{
				continue;
			}

			UalAfb* simLockItem = m_ualAfbs.value(ualItem->guid(), nullptr);

			if (simLockItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Q_ASSERT(simLockItem->isSimLockItem() == true);

			const LogicPin* outPin = simLockItem->getPin(UalAfb::OUT_PIN_CAPTION);

			if (outPin == nullptr)
			{
				// Pin with caption %1 is not found in schema item (Logic schema %2).
				//
				m_log->errALC5106(UalAfb::OUT_PIN_CAPTION, simLockItem->guid(), simLockItem->schemaID());
				result = false;
				continue;
			}

			QStringList nearestSignalIDs;

			getNearestOutSignalIDs(*outPin, &nearestSignalIDs);

			for(const QString& signalWithFlagID : nearestSignalIDs)
			{
				result &= appendFlagToSignalFromPin(ualItem, UalAfb::SIMLOCK_SIM_PIN_CAPTION, true, E::AppSignalStateFlagType::Simulated, signalWithFlagID, nullptr);
				result &= appendFlagToSignalFromPin(ualItem, UalAfb::SIMLOCK_BLOCK_PIN_CAPTION, true, E::AppSignalStateFlagType::Blocked, signalWithFlagID, nullptr);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::processMismatchItems()
	{
		const int MISMATCH_MIN_PIN_COUNT = 2;
		const int MISMATCH_MAX_PIN_COUNT = 4;

		const QString inPinCaptions[MISMATCH_MAX_PIN_COUNT] =
		{
			UalAfb::IN_1_PIN_CAPTION,
			UalAfb::IN_2_PIN_CAPTION,
			UalAfb::IN_3_PIN_CAPTION,
			UalAfb::IN_4_PIN_CAPTION
		};

		const QString outPinCaptions[MISMATCH_MAX_PIN_COUNT] =
		{
			UalAfb::OUT_1_PIN_CAPTION,
			UalAfb::OUT_2_PIN_CAPTION,
			UalAfb::OUT_3_PIN_CAPTION,
			UalAfb::OUT_4_PIN_CAPTION
		};

		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isMismatchItem() == false)
			{
				continue;
			}

			if (ualItem->assignFlags() == false)
			{
				continue;
			}

			UalAfb* mismatchItem = m_ualAfbs.value(ualItem->guid(), nullptr);

			if (mismatchItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Q_ASSERT(mismatchItem->isMismatchItem() == true);

			int foundPinsCount = 0;

			for(int pinNo = 0; pinNo < MISMATCH_MAX_PIN_COUNT; pinNo++)
			{
				const QString& inPinCaption = inPinCaptions[pinNo];
				const QString& outPinCaption = outPinCaptions[pinNo];

				const LogicPin* inPin = mismatchItem->getPin(inPinCaption);

				if (inPin == nullptr)
				{
					break;			// it is Ok, no more input pins
				}

				foundPinsCount++;

				QString signalWithFlagID;

				getDirectlyConnectedInSignalID(*inPin, &signalWithFlagID);

				if (signalWithFlagID.isEmpty() == true)
				{
					continue;
				}

				result &= appendFlagToSignalFromPin(ualItem, outPinCaption, true, E::AppSignalStateFlagType::Mismatch, signalWithFlagID, nullptr);
			}

			if (foundPinsCount < MISMATCH_MIN_PIN_COUNT)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::processSetFlagsItems()
	{
		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSetFlagsItem() == false)
			{
				continue;
			}

			UalAfb* setFlagsItem = m_ualAfbs.value(ualItem->guid(), nullptr);

			if (setFlagsItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Q_ASSERT(setFlagsItem->isSetFlagsItem() == true);

			const LogicPin* outPin = setFlagsItem->getPin(UalAfb::OUT_PIN_CAPTION);

			if (outPin == nullptr)
			{
				// Pin with caption %1 is not found in schema item (Logic schema %2).
				//
				m_log->errALC5106(UalAfb::IN_PIN_CAPTION, setFlagsItem->guid(), setFlagsItem->schemaID());
				result = false;
				continue;
			}

			QStringList outSignalIDs;

			getNearestOutSignalIDs(*outPin, &outSignalIDs);

			if (outSignalIDs.isEmpty() == true)
			{
				// Named signal isn't connected to set_flags item output. Flags cannot be set. (Item %1, schema %2)
				//
				m_log->errALC5195(setFlagsItem->label(), setFlagsItem->guid(), setFlagsItem->schemaID());
				result = false;
				continue;
			}

			for(const QString& signalWithFlagsID : outSignalIDs)
			{
				bool flagIsSet = false;

				result &= appendFlagToSignalFromPin(ualItem, UalAfb::VALIDITY_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Validity, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, UalAfb::SIMULATED_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Simulated, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, UalAfb::BLOCKED_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Blocked, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, UalAfb::MISMATCH_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Mismatch, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, UalAfb::HIGH_LIMIT_PIN_CAPTION, false,
													E::AppSignalStateFlagType::AboveHighLimit, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, UalAfb::LOW_LIMIT_PIN_CAPTION, false,
													E::AppSignalStateFlagType::BelowLowLimit, signalWithFlagsID, &flagIsSet);
				if (flagIsSet == false)
				{
					// No flags assiged on set_flags item %1 (Schema %2)
					//
					m_log->wrnALC5169(ualItem->label(), ualItem->guid(), ualItem->schemaID());
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::appendFlagToSignalFromPin(const UalItem* ualItem,
												const QString& pinCaption,
												bool pinShouldBeExist,
												E::AppSignalStateFlagType flagType,
												const QString& signalWithFlagID,
												bool* flagIsSet)
	{
		TEST_PTR_RETURN_FALSE(ualItem);

		// flagIsSet can be == nullptr !

		const LogicPin* pin = ualItem->getPin(pinCaption);

		if (pin == nullptr)
		{
			if (pinShouldBeExist == true)
			{
				// Pin with caption %1 is not found in schema item (Logic schema %2).
				//
				m_log->errALC5106(pinCaption, ualItem->guid(), ualItem->schemaID());
				return false;
			}
			else
			{
				return true;				// it is Ok! pin for specified flagType may be not used
			}
		}

		if (pin->IsOutput() == true && isConnectedToTerminatorOnly(*pin) == true)
		{
			return true;			// output is not used (connected to terminator only), it is Ok
		}

		QString nearestID;

		bool result = getNearestSignalID(*pin, &nearestID);

		RETURN_IF_FALSE(result);

		if (nearestID.isEmpty() == true)
		{
			// has no signal item directly connected to pin
			//
			UalSignal* ualSignal = m_ualSignals.get(pin->guid());

			LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(ualSignal != nullptr, log());

			nearestID = ualSignal->appSignalID();
		}

		result = appendFlagToSignal(signalWithFlagID, flagType, nearestID, ualItem);

		if (result == true && flagIsSet != nullptr)
		{
			*flagIsSet = true;
		}

		return result;
	}

	bool ModuleLogicCompiler::appendFlagToSignal(const QString& signalWithFlagID,
												 E::AppSignalStateFlagType flagType,
												 const QString& flagSignalID,
												 const UalItem* setFlagsItem)
	{
		// setFlagsItem can be nullptr!
		//
		AppSignal* signalWithFlag = m_signals->getSignal(signalWithFlagID);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(signalWithFlag != nullptr, m_log);

		QString currentStateFlagSignalID = signalWithFlag->getFlagSignalID(flagType);

		if (currentStateFlagSignalID.isEmpty() == false)
		{
			// Duplicate assigning of signal %1 to flag %2 of signal %3. Signal %4 already assigned to this flag.
			//
			m_log->errALC5168(	flagSignalID,
								E::valueToString<E::AppSignalStateFlagType>(flagType),
								signalWithFlagID,
								currentStateFlagSignalID,
								(setFlagsItem == nullptr ? QUuid() : setFlagsItem->guid()),
								(setFlagsItem == nullptr ? QString() : setFlagsItem->schemaID()));
			return false;
		}

		bool res = signalWithFlag->addFlagSignalID(flagType, flagSignalID);

		Q_ASSERT(res == true);

		m_signalsWithFlagsIDs.insert(signalWithFlagID);

		return true;
	}

	bool ModuleLogicCompiler::setAcquiredForFlagSignals()
	{
		bool result = true;

		for(const QString& signalWithFlagsID : m_signalsWithFlagsIDs)
		{
			AppSignal* signalWithFlags = m_signals->getSignal(signalWithFlagsID);

			TEST_PTR_CONTINUE(signalWithFlags);

			if (signalWithFlags->isAcquired() == false)
			{
				continue;
			}

			UalSignal* ualSignalWithFlags = m_ualSignals.get(signalWithFlagsID);

			TEST_PTR_CONTINUE(ualSignalWithFlags);

			if (ualSignalWithFlags->isAcquired() == false)
			{
				Q_ASSERT(false);			// if signalWithFlagsID is Acquired, ualSignalWithFlags should be Acquired also!
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			QStringList flagSignalIDs = signalWithFlags->getFlagSignalsIDs();

			Q_ASSERT(flagSignalIDs.size() > 0);

			for(const QString& flagSignalID : flagSignalIDs)
			{
				UalSignal* ualFlagSignal = m_ualSignals.get(flagSignalID);

				if (ualFlagSignal == nullptr)
				{
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				ualFlagSignal->setAcquired(true);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::checkSignalsWithFlags()
	{
		bool result = true;

		std::vector<E::AppSignalStateFlagType> flagTypes = E::values<E::AppSignalStateFlagType>();

		m_signalsWithFlagsAndFlagSignals.clear();

		for(const QString& signalWithFlagsID : m_signalsWithFlagsIDs)
		{
			AppSignal* signalWithFlags = m_signals->getSignal(signalWithFlagsID);

			if (signalWithFlags == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(log(), QString("Signal with flags %1 is not found in map m_signals").arg(signalWithFlagsID));
				result = false;
				continue;
			}

			UalSignal* ualSignalWithFlags = m_ualSignals.get(signalWithFlagsID);

			if (ualSignalWithFlags == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(log(), QString("Signal with flags %1 is not found in map m_ualSignals").arg(signalWithFlagsID));
				result = false;
				continue;
			}

			m_signalsWithFlagsAndFlagSignals.insert(ualSignalWithFlags);

			bool signalWithFlagsIsAcquired = signalWithFlags->isAcquired();

			if (signalWithFlagsIsAcquired == true && ualSignalWithFlags->isAcquired() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				continue;
			}

			QStringList flagSignalsIDs = signalWithFlags->getFlagSignalsIDs();

			for(const QString& flagSignalID : flagSignalsIDs)
			{
				AppSignal* flagSignal = m_signals->getSignal(flagSignalID);

				if (flagSignal == nullptr)
				{
					LOG_INTERNAL_ERROR_MSG(log(), QString("Flag signal %1 is not found in map m_signals").arg(flagSignalID));
					result = false;
					continue;
				}

				UalSignal* ualFlagSignal = m_ualSignals.get(flagSignalID);

				if (ualFlagSignal == nullptr)
				{
					LOG_INTERNAL_ERROR_MSG(log(), QString("Flag signal %1 is not found in map m_ualSignals").arg(flagSignalID));
					result = false;
					continue;
				}

				m_signalsWithFlagsAndFlagSignals.insert(ualFlagSignal);

				if (signalWithFlagsIsAcquired == true && ualFlagSignal->isAcquired() == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
				}
			}
		}

		return result;
	}

	void ModuleLogicCompiler::writeSignalsWithFlagsReport()
	{
		if (m_context->generateExtraDebugInfo() == false)
		{
			return;
		}

		QStringList file;

		for(const QString& signalWithFlagsID : m_signalsWithFlagsIDs)
		{
			AppSignal* signalWithFlags = m_signals->getSignal(signalWithFlagsID);

			TEST_PTR_CONTINUE(signalWithFlags);

			QString str = QString("%1:%2;").arg(signalWithFlags->isAcquired() ? "acquired" : "not_acquired").arg(signalWithFlagsID);

			std::vector<E::AppSignalStateFlagType> flagTypes = E::values<E::AppSignalStateFlagType>();

			for(E::AppSignalStateFlagType flagType : flagTypes)
			{
				QString flagSignalID = signalWithFlags->getFlagSignalID(flagType);

				if (flagSignalID.isEmpty() == true)
				{
					continue;
				}

				str += QString("%1:%2;").arg(E::valueToString<E::AppSignalStateFlagType>(flagType)).arg(flagSignalID);
			}

			file.append(str);
		}

		m_resultWriter->addFile(lmSubsystemEquipmentIdPath(), "SignalsWithFlags.csv", file);
	}

	bool ModuleLogicCompiler::sortUalSignals()
	{
		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			ualSignal->sortRefSignals();
		}

		return true;
	}

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal, const QString& busTypeID)
	{
		const std::vector<QUuid>& connectedPinsUuids = outPin.associatedIOs();

		QStringList connectedLoopbacks;

		for(QUuid inPinUuid : connectedPinsUuids)
		{
			UalItem* connectedItem = m_pinParent.value(inPinUuid, nullptr);

			if (connectedItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (connectedItem->isLoopbackSource() == true)
			{
				const UalLoopbackSource* source = connectedItem->ualLoopbackSource();

				if (source == nullptr)
				{
					assert(false);
					continue;
				}

				connectedLoopbacks.append(source->loopbackId());

				continue;
			}

			if (connectedItem->isSetFlagsItem() == true)
			{
				// if 'outPin' is connected to 'in' pin of set_flags item
				// items connected to 'out' pin of set_flags item should be checked

				const LogicPin* inPin = connectedItem->getPin(inPinUuid);

				if (inPin == nullptr)
				{
					assert(false);
					continue;
				}

				if (inPin->caption() != UalAfb::IN_PIN_CAPTION)
				{
					continue;
				}

				// yes, this is 'in' pin of set_flags item

				const LogicPin* connectedItemOutPin = connectedItem->getPin(UalAfb::OUT_PIN_CAPTION);

				if (connectedItemOutPin == nullptr)
				{
					assert(false);
					continue;
				}

				AppSignal* s = getCompatibleConnectedSignal(*connectedItemOutPin, outAfbSignal, busTypeID);

				if (s == nullptr)
				{
					continue;			// it is Ok
				}

				return s;
			}

			if (connectedItem->isSignal() == false)
			{
				continue;
			}

			QString signalID = connectedItem->strID();

			AppSignal* s = m_signals->getSignal(signalID);

			if (s == nullptr)
			{
				continue;
			}

			if (isCompatible(outAfbSignal, busTypeID, s) == true)
			{
				return s;
			}
		}

		// find connected signals in loopbacks
		//
		for(const QString& loopbackID : connectedLoopbacks)
		{
			QStringList signalIDs = m_loopbacks.getLoopbackLinkedSignals(loopbackID);

			for(const QString& signalID : signalIDs)
			{
				AppSignal* s = m_signals->getSignal(signalID);

				if (s == nullptr)
				{
					continue;
				}

				if (isCompatible(outAfbSignal, busTypeID, s) == true)
				{
					return s;
				}
			}
		}

		return nullptr;
	}

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal)
	{
		return getCompatibleConnectedSignal(outPin, outAfbSignal, QString());
	}

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedSignal(const LogicPin& outPin, const AppSignal& s)
	{
		LogicAfbSignal dummySignal;

		dummySignal.setType(s.signalType());

		if (s.isBus() == false)
		{
			dummySignal.setDataFormat(s.dataFormat());
		}

		dummySignal.setSize(s.dataSize());
		dummySignal.setByteOrder(s.byteOrder());

		QString busTypeID = s.busTypeID();

		return getCompatibleConnectedSignal(outPin, dummySignal, busTypeID);
	}

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedBusSignal(const LogicPin& outPin, const QString busTypeID)
	{
		LogicAfbSignal dummyBusSignal;

		dummyBusSignal.setType(E::SignalType::Bus);

		return getCompatibleConnectedSignal(outPin, dummyBusSignal, busTypeID);
	}

	bool ModuleLogicCompiler::isCompatible(const LogicAfbSignal& outAfbSignal, const QString& busTypeID, const AppSignal* s)
	{
		TEST_PTR_LOG_RETURN_FALSE(s, m_log);

		switch(outAfbSignal.type())
		{
		case E::SignalType::Discrete:
		case E::SignalType::Analog:

			return s->isCompatibleFormat(outAfbSignal.type(),
									  outAfbSignal.dataFormat(),
									  outAfbSignal.size(),
									  outAfbSignal.byteOrder());

		case E::SignalType::Bus:

			return s->isCompatibleFormat(outAfbSignal.type(), busTypeID);

		default:
			assert(false);
		}

		return false;
	}

	bool ModuleLogicCompiler::isConnectedToTerminatorOnly(const LogicPin& outPin)
	{
		const std::vector<QUuid>& connectedPinsUuids = outPin.associatedIOs();

		for(QUuid inPinUuid : connectedPinsUuids)
		{
			UalItem* connectedItem = m_pinParent.value(inPinUuid, nullptr);

			TEST_PTR_CONTINUE(connectedItem);

			if (connectedItem->isTerminator() == false)
			{
				return false;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::isConnectedToLoopback(const LogicPin& inPin, std::shared_ptr<Loopback>* loopback)
	{
		TEST_PTR_RETURN_FALSE(loopback);

		*loopback = m_loopbacks.getLoopbackByPin(inPin);

		return *loopback != nullptr;
	}

	bool ModuleLogicCompiler::determineOutBusTypeID(UalAfb* ualAfb, QString* outBusTypeID)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(outBusTypeID, m_log);

		outBusTypeID->clear();

		// AFB's out bus type determination rules:
		//
		// 1) try determinte BusType by input UalSignals
		// 2) try determine BusType by bus signal connected to output

		determineBusTypeByInputs(ualAfb, outBusTypeID);

		if (outBusTypeID->isEmpty() == false)
		{
			return true;
		}

		determineBusTypeByOutput(ualAfb, outBusTypeID);

		return outBusTypeID->isEmpty() == false;
	}

	bool ModuleLogicCompiler::determineBusTypeByInputs(const UalAfb* ualAfb, QString* outBusTypeID)
	{
		QStringList busTypes;

		bool result = true;

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

			// inPin is bus

			UalSignal* ualSignal = m_ualSignals.get(inPin.guid());

			if (ualSignal != nullptr)
			{
				if (ualSignal->isBus() == true)
				{
					busTypes.append(ualSignal->busTypeID());

					continue;			// check remaining inputs
				}

				if (ualSignal->isDiscrete() && afbSignal.isBus() == true &&
						(afbSignal.busDataFormat() == E::BusDataFormat::Discrete ||
							afbSignal.busDataFormat() == E::BusDataFormat::Mixed))
				{
					// discrete signal to "discrete" or "mixed" bus - is allowed connection, but bus type is still unknown
					continue;
				}

				// Uncompatible signals connection (Logic schema '%1').
				//
				assert(false);				// this error must be detected earlier

				m_log->errALC5117(ualSignal->ualItemGuid(), ualSignal->ualItemLabel(),
								  ualAfb->guid(), ualAfb->label(), ualAfb->schemaID());
				return false;
			}

			// ualSignal is not connected to bus input now
			// check, may be input is connected to LoopbackTarget via SignalItem(s),
			// and try get busType from this signal(s)

			LoopbackShared loopback = nullptr;

			if (isConnectedToLoopback(inPin, &loopback) == false)
			{
				// UalSignal is not found for pin '%1' (Logic schema '%2').
				//
				m_log->errALC5120(ualAfb->guid(), ualAfb->label(), inPin.caption(), ualAfb->schemaID());
				result = false;
				continue;
			}

			TEST_PTR_LOG_RETURN_FALSE(loopback, m_log);

			// get any signal of loopback
			// all signals should be a same type (otherwise error should be reported early)
			//

			QString anyLoopbackSignalID = loopback->anyLinkedSignalID();

			if (anyLoopbackSignalID.isEmpty() == true)
			{
				continue;
			}

			// yes, input is connected to Loopback
			AppSignal* s = m_signals->getSignal(anyLoopbackSignalID);

			if (s == nullptr)
			{
				continue;
			}

			if (s->isBus() == true)
			{
				busTypes.append(s->busTypeID());
			}
		}

		if (result == false)
		{
			return false;
		}

		if (busTypes.isEmpty() == true)
		{
			return false;			// no busses on inputs
		}

		if (isBusTypesAreEqual(busTypes) == false)
		{
			// Different busTypes on AFB inputs (Logic schema %1).
			//
			m_log->errALC5123(ualAfb->guid(), ualAfb->schemaID(), ualAfb->label());

			return false;			// bus type is not determined :(
		}

		*outBusTypeID = busTypes.first();

		return true;
	}

	bool ModuleLogicCompiler::determineBusTypeByOutput(const UalAfb* ualAfb, QString* outBusTypeID)
	{
		QStringList busTypes;

		const std::vector<LogicPin> outputs = ualAfb->outputs();

		for(const LogicPin& outPin : outputs)
		{
			LogicAfbSignal afbSignal;

			bool res = ualAfb->getAfbSignalByPin(outPin, &afbSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (afbSignal.isBus() == false)
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(outPin.guid());

			if (ualSignal != nullptr)
			{
				if (ualSignal->isBus() == true)
				{
					busTypes.append(ualSignal->busTypeID());
				}

				continue;
			}

			std::set<QString> linkedSignals;
			std::set<const UalItem*> linkedItems;

			res = getSignalsAndPinsLinkedToOutPin(ualAfb, outPin, &linkedSignals, &linkedItems, nullptr);

			if (res == false)
			{
				return false;
			}

			for(const QString& linkedSignalID : linkedSignals)
			{
				AppSignal* s = m_signals->getSignal(linkedSignalID);

				if (s == nullptr)
				{
					continue;
				}

				if (s->isBus() == true)
				{
					busTypes.append(s->busTypeID());
				}
			}
		}

		if (busTypes.isEmpty() == true)
		{
			return false;			// no bus signals connected to output(s)
		}

		if (isBusTypesAreEqual(busTypes) == false)
		{
			// Different busTypes on AFB output (Logic schema %1).
			//
			m_log->errALC5122(ualAfb->guid(), ualAfb->schemaID(), ualAfb->label());

			return false;			// bus type is not determined :(
		}

		*outBusTypeID = busTypes.first();

		return true;
	}

	bool ModuleLogicCompiler::isBusTypesAreEqual(const QStringList& busTypes)
	{
		if (busTypes.isEmpty() == true)
		{
			return false;
		}

		QString firstBusType;

		for(const QString& busType : busTypes)
		{
			assert(busType.isEmpty() == false);

			if (firstBusType.isEmpty() == true)
			{
				firstBusType = busType;
			}
			else
			{
				if (firstBusType != busType)
				{
					return false;
				}
			}
		}

		return true;
	}

	std::optional<int> ModuleLogicCompiler::getOutPinExpectedReadCount(const LogicPin& outPin)
	{
		Q_ASSERT(outPin.IsOutput() == true);

		// calculate expected read count to place signal in heap
		// if signal can't be placed in heap uninitialized value of std::optional<int> will be returned
		//
		int readCount = 0;

		for(const QUuid& inPinGuid : outPin.associatedIOs())
		{
			UalItem* inPinParentItem = m_pinParent.value(inPinGuid);

			TEST_PTR_CONTINUE(inPinParentItem);

			switch(inPinParentItem->type())
			{
			case E::UalItemType::Afb:
				readCount += getAfbInPinExpectedReadCount(inPinParentItem, inPinGuid);
				break;

			case E::UalItemType::BusComposer:

				readCount++;

				break;

			case E::UalItemType::Terminator:
				break;

			case E::UalItemType::Signal:
			case E::UalItemType::LoopbackSource:
			case E::UalItemType::Transmitter:

				return std::nullopt;			// if any ual item of this types is connected to AFB out - signal can't be placed in heap

			case E::UalItemType::BusExtractor:

				Q_ASSERT(false);				// bus signals in heap is not implemeneted now

				return std::nullopt;

			case E::UalItemType::Const:
			case E::UalItemType::Receiver:
			case E::UalItemType::LoopbackTarget:
			case E::UalItemType::Unknown:

				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);		// Ual items of this types can't be connected to AFB output pin

				return std::nullopt;

			default:
				Q_ASSERT(false);
			}
		}

		if (readCount == 0)
		{
			return std::nullopt;
		}

		return std::optional<int>(readCount);
	}

	int ModuleLogicCompiler::getAfbInPinExpectedReadCount(const UalItem* ualItem, const QUuid& inPinGuid)
	{
		if (ualItem == nullptr)
		{
			Q_ASSERT(false);
			return 0;
		}

		if (ualItem->isSetFlagsItem() == false)
		{
			return 1;			// input pin of usual AFB item is produce 1 reads
		}

		// this is set_flags AFB
		//
		const LogicPin* inPin = ualItem->getPin(inPinGuid);

		if (inPin == nullptr)
		{
			Q_ASSERT(false);
			return 0;
		}

		Q_ASSERT(inPin->IsInput() == true);

		if (inPin->caption() != UalAfb::IN_PIN_CAPTION)
		{
			return 0;			// input pins of set_falgs except "in" is not produce reads
		}

		// this is "in" pin of set_flags AFB
		// calculation of all reads of pins connected to "out" pin is required
		//
		const std::vector<LogicPin>& outs = ualItem->outputs();

		if (outs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return 0;
		}

		if (outs[0].caption() != UalAfb::OUT_PIN_CAPTION)
		{
			LOG_INTERNAL_ERROR(m_log);
			return 0;
		}

		std::optional<int> expReads = getOutPinExpectedReadCount(outs[0]);

		return expReads.value_or(0);
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

		// common code for IPEN (FotipV1) and FotipV2 tuning protocols and data
		//
		bool tuningPropertyExists = false;
		bool tuningEnabled = false;

		bool result = getTuningSettings(&tuningPropertyExists, &tuningEnabled);

		if (result == false)
		{
			return false;
		}

		if (tuningPropertyExists == false)
		{
			return true;
		}

		Tuning::TuningData* tuningData = new Tuning::TuningData(lmEquipmentID(),
																m_lmDescription->flashMemory().m_tuningFrameCount,
																m_lmDescription->flashMemory().m_tuningFramePayload,
																m_lmDescription->flashMemory().m_tuningFrameSize,
																m_lmDescription->memory().m_tuningDataOffset,
																m_lmDescription->memory().m_tuningDataSize,
																m_lmDescription->memory().m_tuningDataFrameCount,
																m_lmDescription->memory().m_tuningDataFramePayload,
																m_lmDescription->memory().m_tuningDataFrameSize);
		result = buildTuningSignalsLists(tuningData);

		if (result == false)
		{
			delete tuningData;
			return false;
		}

		if (tuningData->getSignalsCount() == 0)
		{
			if (tuningEnabled == true)
			{
				// Tuning is enabled for module %1 but tunable signals is not found.
				//
				m_log->wrnALC5165(lmEquipmentID());
			}
		}
		else
		{
			if (tuningEnabled == false)
			{
				// Tunable signals is found in module %1 but tuning is not enabled.
				//
				m_log->errALC5166(lmEquipmentID());
				delete tuningData;
				return false;
			}
		}

		tuningData->buildTuningData();

		int tuningFrameCount = m_lmDescription->flashMemory().m_tuningFrameCount;

		if (tuningData->usedFramesCount() > tuningFrameCount)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("Tuning data of LM '%1' exceed available %2 frames")).
							   arg(lmEquipmentID()).
							   arg(tuningFrameCount));
			delete tuningData;
			return false;
		}

		tuningData->generateUniqueID(lmEquipmentID());

		m_tuningData = tuningData;
		m_tuningDataStorage->insert(lmEquipmentID(), tuningData);

		return true;
	}

	bool ModuleLogicCompiler::buildTuningSignalsLists(Tuning::TuningData* tuningData)
	{
		TEST_PTR_RETURN_FALSE(tuningData);

		tuningData->clearSignalLists();

		bool result = true;

		for(AppSignal* signal : m_chassisSignals)
		{
			if (signal->enableTuning() == false)
			{
				continue;
			}

			switch(signal->signalType())
			{
			case E::SignalType::Analog:

				if (signal->dataSize() != SIZE_32BIT)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString(tr("Analog signal '%1' for tuning must have 32-bit dataSize")).
						  arg(signal->appSignalID()));
					result = false;
				}
				else
				{
					if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
					{
						tuningData->appendTuningSignal(E::TuningSignalType::AnalogFloat, signal);
					}
					else
					{
						if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
						{
							tuningData->appendTuningSignal(E::TuningSignalType::AnalogInt32, signal);
						}
						else
						{
							LOG_INTERNAL_ERROR_MSG(m_log, QString(tr("Analog signal '%1' for tuning must have Float or Signed Int data format")).
													arg(signal->appSignalID()));
							result = false;
						}
					}
				}

				break;

			case E::SignalType::Discrete:

				if (signal->dataSize() != 1)
				{
					LOG_INTERNAL_ERROR_MSG(m_log,  QString(tr("Discrete signal '%1' for tuning must have 1-bit dataSize")).
													arg(signal->appSignalID()));
					result = false;
					continue;
				}
				else
				{
					tuningData->appendTuningSignal(E::TuningSignalType::Discrete, signal);
				}

				break;

			case E::SignalType::Bus:
			default:
				Q_ASSERT(false);				// unknown tuning signal type
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::getTuningSettings(bool* tuningPropertyExists, bool* tuningEnabled)
	{
		TEST_PTR_LOG_RETURN_FALSE(m_lmDescription, m_log);

		const LmDescription::Lan& lan = m_lmDescription->lan();

		int tuningControllerNo = -1;

		// search ethernet controller that provide tuning
		//
		for(const LmDescription::LanController& lanController : lan.m_lanControllers)
		{
			if (lanController.isProvideTuning() == true)
			{
				tuningControllerNo = lanController.m_place;
				break;
			}
		}

		if (tuningControllerNo == -1)
		{
			// no tuning controllers found
			//
			*tuningPropertyExists = false;
			return true;
		}

		QString suffix = LanControllerInfoHelper::getLanControllerSuffix(tuningControllerNo);

		Hardware::DeviceController* adapter = DeviceHelper::getChildControllerBySuffix(m_lm, suffix, m_log);

		if (adapter == nullptr)
		{
			assert(false);
			return false;
		}

		if (DeviceHelper::isPropertyExists(adapter, EquipmentPropNames::TUNING_ENABLE) == false)
		{
			*tuningPropertyExists = false;
			return true;
		}

		*tuningPropertyExists = true;

		bool res = DeviceHelper::getBoolProperty(adapter,
		                                         EquipmentPropNames::TUNING_ENABLE,
												 tuningEnabled,
												 m_log);
		return res;
	}

	bool ModuleLogicCompiler::disposeSignalsInHeap()
	{
		m_ualSignals.disposeSignalsInHeaps(m_signalsWithFlagsAndFlagSignals);

		return true;
	}

	bool ModuleLogicCompiler::createSignalLists()
	{
		bool result = true;

		result &= createAcquiredDiscreteInputSignalsList();
		result &= createAcquiredDiscreteStrictOutputSignalsList();
		result &= createAcquiredDiscreteInternalSignalsList();
		result &= createAcquiredDiscreteOptoSignalsList();
		result &= createAcquiredDiscreteBusChildSignalsList();
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

		result &= createAcquiredInputBusesList();
		result &= createAcquiredOutputBusesList();
		result &= createAcquiredInternalBusesList();
		result &= createAcquiredBusBusChildSignalsList();
		result &= createAcquiredOptoBusesList();

		result &= createNonAcquiredOutputBusesList();
		result &= createNonAcquiredInternalBusesList();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		result &= setSignalsCalculatedAttributes();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		sortSignalList(m_acquiredDiscreteInputSignals);
		sortSignalList(m_acquiredDiscreteStrictOutputSignals);
		sortSignalList(m_acquiredDiscreteInternalSignals);
		sortSignalList(m_acquiredDiscreteOptoSignals);
		sortSignalList(m_acquiredDiscreteBusChildSignals);
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

		sortSignalList(m_acquiredInputBuses);
		sortSignalList(m_acquiredOutputBuses);
		sortSignalList(m_acquiredInternalBuses);
		sortSignalList(m_acquiredBusChildBuses);

		sortSignalList(m_nonAcquiredOutputBuses);
		sortSignalList(m_nonAcquiredInternalBuses);

		return result;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteInputSignalsList()
	{
		m_acquiredDiscreteInputSignals.clear();

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
				s->isInput() == true &&
				s->isBusChild() == false)
			{
				m_acquiredDiscreteInputSignals.append(s);
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
		//  - tunable
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
				s->isTunable() == false)
			{
				m_acquiredDiscreteInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteOptoSignalsList()
	{
		m_acquiredDiscreteOptoSignals.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isDiscrete() == true &&
				ualSignal->isOptoSignal() == true &&
				ualSignal->isConst() == false)
			{
				m_acquiredDiscreteOptoSignals.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteBusChildSignalsList()
	{
		m_acquiredDiscreteBusChildSignals.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isDiscrete() == true &&
				ualSignal->isBusChild() == true &&
				ualSignal->isConst() == false &&
				ualSignal->anyParentBusIsAcquired() == false)
			{
				m_acquiredDiscreteBusChildSignals.append(ualSignal);
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
		//	+ tunable
		//	+ no matter used in UAL or not

		QVector<AppSignal*> tuningSignals;

		m_tuningData->getAcquiredDiscreteSignals(tuningSignals);

		// check signals!

		for(AppSignal* s : tuningSignals)
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
				ualSignal->isTunable() == true)
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
				s->isInput() == true &&
				s->isBusChild() == false)
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
				s->isTunable() == false &&
				s->isHeapPlaced() == false)
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
				s->isInput() == true &&
				s->isBusChild() == false)
			{
				m_acquiredAnalogInputSignals.append(s);
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
		//	+ used in UAL || is a SerialRx signal (condition: m_optoModuleStorage->isSerialRxSignalExists(lmEquipmentID(), s->appSignalID()) == true))

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				s->isBusChild() == false &&
				s->isAcquired() == true &&
				s->isAnalog() == true &&
				s->isInternal() == true &&
				s->isTunable() == false)
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
				s->isConst() == false &&
				s->anyParentBusIsAcquired() == false)
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
		//	+ tunable
		//	+ no matter used in UAL or not

		QVector<AppSignal*> tuningSignals;

		m_tuningData->getAcquiredAnalogSignals(tuningSignals);

		// check signals!

		for(AppSignal* s : tuningSignals)
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
				ualSignal->isTunable() == true)
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
				m_acquiredAnalogConstIntSignals.insert(ualSignal->constAnalogIntValue(), ualSignal);
				continue;

			case E::AnalogAppSignalFormat::Float32:
				m_acquiredAnalogConstFloatSignals.insert(ualSignal->constAnalogFloatValue(), ualSignal);
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

		// sort array be appSignalID

		int count = m_analogOutputSignalsToConversion.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				AppSignal* si = m_analogOutputSignalsToConversion[i];
				AppSignal* sk = m_analogOutputSignalsToConversion[k];

				if (si == nullptr || sk == nullptr)
				{
					assert(false);
					continue;
				}

				if (si->appSignalID() > sk->appSignalID())
				{
					m_analogOutputSignalsToConversion[i] = sk;
					m_analogOutputSignalsToConversion[k] = si;
				}
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
				s->isInput() == true &&
				s->isBusChild() == false)
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
				s->isInternal() == true &&
				s->isHeapPlaced() == false)
			{
				m_nonAcquiredAnalogInternalSignals.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredInputBusesList()
	{
		m_acquiredInputBuses.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isInput() == true &&
				ualSignal->isBus() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_acquiredInputBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredOutputBusesList()
	{
		m_acquiredOutputBuses.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isBus() == true &&
				ualSignal->isOutput() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_acquiredOutputBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredInternalBusesList()
	{
		m_acquiredInternalBuses.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == true &&
				ualSignal->isBus() == true &&
				ualSignal->isInternal() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_acquiredInternalBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredBusBusChildSignalsList()
	{
		m_acquiredBusChildBuses.clear();

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == true &&
				s->isBus() == true &&
				s->isBusChild() == true &&
				s->isConst() == false &&
				s->anyParentBusIsAcquired() == false)
			{
				m_acquiredBusChildBuses.append(s);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createAcquiredOptoBusesList()
	{
		m_acquiredOptoBuses.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if ( ualSignal->isAcquired() == true &&
				 ualSignal->isBus() == true &&
				 ualSignal->isBusChild() == false &&
				 ualSignal->isOptoSignal() == true)
			{
				m_acquiredOptoBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredOutputBusesList()
	{
		m_nonAcquiredOutputBuses.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == false &&
				ualSignal->isBus() == true &&
				ualSignal->isOutput() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_nonAcquiredOutputBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createNonAcquiredInternalBusesList()
	{
		m_nonAcquiredInternalBuses.clear();

		for(UalSignal* ualSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAcquired() == false &&
				ualSignal->isBus() == true &&
				ualSignal->isInternal() == true &&
				ualSignal->isBusChild() == false &&
				ualSignal->isOptoSignal() == false)
			{
				m_nonAcquiredInternalBuses.append(ualSignal);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::setSignalsCalculatedAttributes()
	{
		bool result = true;

		for(AppSignal* s : m_chassisSignals)
		{
			if(s == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			// set signal's E::LogicModuleRamAccess
			//
			switch(s->inOutType())
			{
			case E::SignalInOutType::Input:
				if (s->needConversion() == true)
				{
					s->setLmRamAccess(E::LogicModuleRamAccess::ReadWrite);
				}
				else
				{
					s->setLmRamAccess(E::LogicModuleRamAccess::Read);
				}
				break;

			case E::SignalInOutType::Output:
				s->setLmRamAccess(E::LogicModuleRamAccess::Write);
				break;

			case E::SignalInOutType::Internal:
				if (s->isTunable() == true)
				{
					s->setLmRamAccess(E::LogicModuleRamAccess::Read);
				}
				else
				{
					s->setLmRamAccess(E::LogicModuleRamAccess::ReadWrite);
				}
				break;

			default:
				assert(false);
			}
		}

		for(UalSignal* ualSignal : m_ualSignals)
		{
			if(ualSignal == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			QVector<AppSignal*> refSignals = ualSignal->refSignals();

			if (ualSignal->isConst() == true)
			{
				std::for_each(refSignals.begin(), refSignals.end(),
							[ualSignal](AppSignal* s) {
					if (s != nullptr)
					{
						s->setIsConst(true);
						s->setConstValue(ualSignal->constValue());
						s->setLmRamAccess(E::LogicModuleRamAccess::Undefined);
					}
				});

				continue;
			}

			if (ualSignal->isOutput() == true)
			{
				std::for_each(refSignals.begin(), refSignals.end(),
							[](AppSignal* s) {
					if (s != nullptr)
					{
						s->setLmRamAccess(E::LogicModuleRamAccess::ReadWrite);
					}
				});
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::groupTxSignals()
	{
		QList<Hardware::OptoPortShared> associatedPorts;

		m_optoModuleStorage->getLmAssociatedOptoPorts(lmEquipmentID(), associatedPorts);

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

	bool ModuleLogicCompiler::listsUniquenessCheck() const
	{
		bool result = true;

		QHash<UalSignal*, UalSignal*> signalsMap;

		signalsMap.reserve(m_chassisSignals.count());

		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteInputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteStrictOutputSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteInternalSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteOptoSignals);
		result &= listUniquenessCheck(signalsMap, m_acquiredDiscreteBusChildSignals);
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

		result &= listUniquenessCheck(signalsMap, m_acquiredInputBuses);
		result &= listUniquenessCheck(signalsMap, m_acquiredOutputBuses);
		result &= listUniquenessCheck(signalsMap, m_acquiredInternalBuses);
		result &= listUniquenessCheck(signalsMap, m_acquiredBusChildBuses);
		result &= listUniquenessCheck(signalsMap, m_acquiredOptoBuses);

		result &= listUniquenessCheck(signalsMap, m_nonAcquiredOutputBuses);
		result &= listUniquenessCheck(signalsMap, m_nonAcquiredInternalBuses);

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

			if (setDiscreteAndBusInputSignalsUalAddresses() == false) break;

			if (disposeTunableSignalsUalAddresses() == false) break;

			if (disposeDiscreteSignalsInBitMemory() == false) break;

			if (disposeDiscreteSignalsHeap() == false) break;

			if (disposeAcquiredRawDataInRegBuf() == false) break;

			if (disposeAcquiredAnalogSignalsInRegBuf() == false) break;

			if (disposeAcquiredBusesInRegBuf() == false) break;

			if (disposeAcquiredDiscreteSignalsInRegBuf() == false) break;

			if (disposeNonAcquiredAnalogSignals() == false) break;

			if (disposeNonAcquiredBuses() == false) break;

			if (disposeAnalogAndBusSignalsHeap() == false) break;

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

		for(AppSignal* s : m_ioSignals)
		{
			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

			// retrieve linked device
			//
			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(s->equipmentID()).get();

			if (device == nullptr)
			{
				assert(false);
				continue;
			}

			if (device->isAppSignal() == false)
			{
				assert(false);
				continue;
			}

			Hardware::DeviceAppSignal* deviceAppSignal = device->toAppSignal().get();

			if (deviceAppSignal == nullptr)
			{
				assert(false);
				continue;
			}

			// retrieve associated module
			//
			const Hardware::DeviceModule* deviceModule = deviceAppSignal->getParentModule();

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

			Address16 ioBufAddr(module.moduleDataOffset, deviceAppSignal->valueBit());

			ioBufAddr.addWord(deviceAppSignal->valueOffset());

			switch(deviceAppSignal->memoryArea())
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
					// Internal application signal %1 cannot be linked to equipment input/output signal %2.
					//
					log()->errALC5171(s->appSignalID(), s->equipmentID());
					result = false;
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
					// Internal application signal %1 cannot be linked to equipment input/output signal %2.
					//
					log()->errALC5171(s->appSignalID(), s->equipmentID());
					result = false;
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

	bool ModuleLogicCompiler::disposeTunableSignalsUalAddresses()
	{
		if (m_tuningData == nullptr)
		{
			return true;			// no tuning data, it is ok
		}

		bool result = true;

		QVector<AppSignal*> tunigableSignals;

		m_tuningData->getSignals(tunigableSignals);

		for(AppSignal* s : tunigableSignals)
		{
			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (s->tuningAbsAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal != nullptr)
			{
				ualSignal->setUalAddr(s->tuningAbsAddr());

				result &= m_memoryMap.appendTuningSignal(ualSignal);
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::setDiscreteAndBusInputSignalsUalAddresses()
	{
		bool result = true;

		// set ualAddress of Discrete and Bus input UalSignals reffered by m_ioSignals equal to ioBufAddr of input signal
		//
		for(AppSignal* ioSignal : m_ioSignals)
		{
			if (ioSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ioSignal->isInput() == false ||
				(ioSignal->isDiscrete() == false && ioSignal->isBus() == false))
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(ioSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				continue;						// is not an error
			}

			if (ualSignal->isInput() == false ||
				(ualSignal->isDiscrete() == false && ioSignal->isBus() == false))
			{
				Q_ASSERT(false);					// ualSignal must be Discrete or Bus Input if reffered by ioSignal
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

	bool ModuleLogicCompiler::disposeDiscreteSignalsHeap()
	{
		int bitMemoryStartAddrW = m_memoryMap.appBitMemoryStart();

		Q_ASSERT(m_lmDescription->memory().m_appLogicBitDataOffset == static_cast<quint32>(bitMemoryStartAddrW));

		int discreteSignalsHeapStartAddrW = m_memoryMap.appBitMemoryDiscreteSignalsHeapStart();
		int discreteSignalsHeapSizeW = m_lmDescription->memory().m_appLogicBitDataSize -
											(discreteSignalsHeapStartAddrW - bitMemoryStartAddrW);

		m_ualSignals.initDiscreteSignalsHeap(discreteSignalsHeapStartAddrW, discreteSignalsHeapSizeW);

		return true;
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

		result &= m_memoryMap.appendAcquiredInputBusesInRegBuf(m_acquiredInputBuses);
		result &= m_memoryMap.appendAcquiredOutputBusesInRegBuf(m_acquiredOutputBuses);
		result &= m_memoryMap.appendAcquiredInternalBusesInRegBuf(m_acquiredInternalBuses);
		result &= m_memoryMap.appendAcquiredBusChildBusesInRegBuf(m_acquiredBusChildBuses);
		result &= m_memoryMap.appendAcquiredOptoBusesInRegBuf(m_acquiredOptoBuses);

		return result;
	}

	bool ModuleLogicCompiler::disposeAcquiredDiscreteSignalsInRegBuf()
	{
		bool result = true;

		result &= m_memoryMap.appendAcquiredDiscreteInputSignalsInRegBuf(m_acquiredDiscreteInputSignals);
		result &= m_memoryMap.appendAcquiredDiscreteStrictOutputSignalsInRegBuf(m_acquiredDiscreteStrictOutputSignals);
		result &= m_memoryMap.appendAcquiredDiscreteInternalSignalsInRegBuf(m_acquiredDiscreteInternalSignals);
		result &= m_memoryMap.appendAcquiredDiscreteOptoSignalsInRegBuf(m_acquiredDiscreteOptoSignals);
		result &= m_memoryMap.appendAcquiredDiscreteBusChildSignalsInRegBuf(m_acquiredDiscreteBusChildSignals);
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

		result &= m_memoryMap.appendNonAcquiredOutputBusses(m_nonAcquiredOutputBuses);
		result &= m_memoryMap.appendNonAcquiredInternalBusses(m_nonAcquiredInternalBuses);

		return result;
	}

	bool ModuleLogicCompiler::disposeAnalogAndBusSignalsHeap()
	{
		int wordMemoryStartAddrW = m_memoryMap.appWordMemoryStart();

		Q_ASSERT(m_lmDescription->memory().m_appLogicWordDataOffset == static_cast<quint32>(wordMemoryStartAddrW));

		int heapStartAddrW = m_memoryMap.appWordMemoryAnalogAndBusSignalsHeapStart();
		int heapSizeW = m_lmDescription->memory().m_appLogicWordDataSize -
											(heapStartAddrW - wordMemoryStartAddrW);

		m_ualSignals.initAnalogAndBusSignalsHeap(heapStartAddrW, heapSizeW);

		return true;
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

			AppSignal* s = ualSignal->getInputSignal();

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
		for(AppSignal* s : m_analogOutputSignalsToConversion)
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

			"tconv_fp_si",					// FB_TCONV_FP_SI_INDEX
			"tconv_si_fp",					// FB_TCONV_SI_FP_INDEX
		};

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

				bool isScaleAfb = QString(fbCaption).startsWith("scale_");

				fbFound = true;

				FbScal fb;

				fb.caption = fbCaption;
				fb.pointer = afbElement;

				int index = 0;

				if (isScaleAfb == true)
				{
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
						// Required parameter %1 of AFB %2 is missing.
						//
						m_log->errALC5045(FB_SCALE_X1_OPNAME, fbCaption, QUuid());
						result = false;
					}

					if (fb.x2ParamIndex == -1)
					{
						m_log->errALC5045(FB_SCALE_X2_OPNAME, fbCaption, QUuid());
						result = false;
					}

					if (fb.y1ParamIndex == -1)
					{
						m_log->errALC5045(FB_SCALE_Y1_OPNAME, fbCaption, QUuid());
						result = false;
					}

					if (fb.y2ParamIndex == -1)
					{
						m_log->errALC5045(FB_SCALE_Y2_OPNAME, fbCaption, QUuid());
						result = false;
					}
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
					// Required signal %1 of AFB %2 is missing.
					//
					m_log->errALC5173(FB_SCALE_INPUT_SIGNAL_CAPTION, fbCaption, QUuid());
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
					// Required signal %1 of AFB %2 is missing.
					//
					m_log->errALC5173(FB_SCALE_INPUT_SIGNAL_CAPTION, fbCaption, QUuid());
					result = false;
					break;
				}

				m_fbScal.append(fb);
			}

			if (fbFound == false)
			{
				// Required AFB %1 is missing.
				//
				m_log->errALC5174(fbCaption, QUuid());
				result = false;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createAfbForAnalogInputSignalConversion(const AppSignal& signal, UalItem* appItem, bool* needConversion)
	{
		TEST_PTR_LOG_RETURN_FALSE(appItem, log());
		TEST_PTR_LOG_RETURN_FALSE(needConversion, log());

		assert(signal.isAnalog());
		assert(signal.isInput());
		assert(signal.equipmentID().isEmpty() == false);

		Hardware::DeviceObject* deviceObject = m_equipmentSet->deviceObject(signal.equipmentID()).get();

		if (deviceObject == nullptr)
		{
			// Application signal '%1' is bound to unknown device object '%2'.
			//
			m_log->errALC5013(signal.appSignalID(), signal.equipmentID());
			return false;
		}

		Hardware::DeviceAppSignal* deviceAppSignal = deviceObject->toAppSignal().get();

		if (deviceAppSignal == nullptr)
		{
			// Input/output application signal '%1' should be bound to equipment signal.
			//
			m_log->errALC5091(signal.appSignalID());
			return false;
		}

		bool signalsIsCompatible = isDeviceAndAppSignalsIsCompatible(*deviceAppSignal, signal);

		if (signalsIsCompatible == true)
		{
			*needConversion = false;
			return true;
		}

		*needConversion = true;

		bool conversionIsKnown = false;
		QString errorMsg;
		bool result = true;

		QString inFormat = getFormatStr(*deviceAppSignal);
		QString outFormat = getFormatStr(signal);

		if (deviceAppSignal->format() == E::DataFormat::UnsignedInt && deviceAppSignal->size() == SIZE_16BIT)
		{
			// Unsigned Int 16 bit conversion
			//
			int x1 = signal.lowADC();
			int x2 = signal.highADC();

			if (x2 - x1 == 0)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Low and High ADC values of signal %1 are equal (= %2)")).arg(signal.appSignalID()).arg(x1));
				return false;
			}

			double y1 = signal.lowEngineeringUnits();
			double y2 = signal.highEngineeringUnits();

			switch(signal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				{
					conversionIsKnown = true;

					FbScal fb = m_fbScal[FB_SCALE_16UI_FP_INDEX];

					fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
					fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

					fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1));
					fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2));

					result &= appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_SCALE_16UI_SI_INDEX];

					fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
					fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

					fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
					fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

					result &= appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			default:
				assert(false);
			}
		}

		if (deviceAppSignal->format() == E::DataFormat::Float && deviceAppSignal->size() == SIZE_32BIT)
		{
			// Float 32 conversion
			//
			switch(signal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_TCONV_FP_SI_INDEX];

					result &= appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			default:
				assert(false);
			}
		}

		if (deviceAppSignal->format() == E::DataFormat::SignedInt && deviceAppSignal->size() == SIZE_32BIT)
		{
			// SignedInt 32 conversion
			//
			switch(signal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_TCONV_SI_FP_INDEX];

					result &= appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			default:
				assert(false);
			}
		}

		if (conversionIsKnown == false)
		{
			// Unknown conversion of signal %1 from %2 to %3 format.
			//
			m_log->errALC5175(signal.appSignalID(), inFormat, outFormat);
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::createFbForAnalogOutputSignalConversion(const AppSignal& signal, UalItem* appItem, bool* needConversion)
	{
		assert(signal.isAnalog());
		assert(signal.isOutput());
		assert(signal.equipmentID().isEmpty() == false);

		Hardware::DeviceObject* deviceObject = m_equipmentSet->deviceObject(signal.equipmentID()).get();

		if (deviceObject == nullptr)
		{
			// Application signal '%1' is bound to unknown device object '%2'.
			//
			m_log->errALC5013(signal.appSignalID(), signal.equipmentID());
			return false;
		}

		Hardware::DeviceAppSignal* deviceAppSignal = deviceObject->toAppSignal().get();

		if (deviceAppSignal == nullptr)
		{
			// Input/output application signal '%1' should be bound to equipment signal.
			//
			m_log->errALC5091(signal.appSignalID());
			return false;
		}

		bool signalsIsCompatible = isDeviceAndAppSignalsIsCompatible(*deviceAppSignal, signal);

		if (signalsIsCompatible == true)
		{
			*needConversion = false;
			return true;
		}

		*needConversion = true;

		bool conversionIsKnown = false;
		QString errorMsg;
		bool result = true;

		QString inFormat = getFormatStr(signal);
		QString outFormat = getFormatStr(*deviceAppSignal);

		if (deviceAppSignal->format() == E::DataFormat::UnsignedInt && deviceAppSignal->size() == SIZE_16BIT)
		{
			double x1 = signal.lowEngineeringUnits();
			double x2 = signal.highEngineeringUnits();

			if (x2 - x1 == 0.0)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("Low and High Limit values of signal %1 are equal (= %2)")).arg(signal.appSignalID()).arg(x1));
				return false;
			}

			int y1 = signal.lowDAC();
			int y2 = signal.highDAC();

			switch(signal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_SCALE_FP_16UI_INDEX];

					fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
					fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

					fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
					fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

					result = appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_SCALE_SI_16UI_INDEX];

					fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1).toInt());
					fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2).toInt());

					fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
					fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

					result = appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			default:
				assert(false);
			}
		}

		if (deviceAppSignal->format() == E::DataFormat::Float && deviceAppSignal->size() == SIZE_32BIT)
		{
			// Float 32 conversion
			//
			switch(signal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_TCONV_SI_FP_INDEX];

					result &= appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			default:
				assert(false);
			}
		}

		if (deviceAppSignal->format() == E::DataFormat::SignedInt && deviceAppSignal->size() == SIZE_32BIT)
		{
			// Signed Int 32 conversion
			//
			switch(signal.analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				{
					conversionIsKnown = true;

					FbScal& fb = m_fbScal[FB_TCONV_FP_SI_INDEX];

					result &= appItem->init(fb.pointer, errorMsg);
					appItem->setLabel(signal.appSignalID());

					if (errorMsg.isEmpty() == false)
					{
						LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
						result = false;
					}
				}

				break;

			default:
				assert(false);
			}
		}

		if (conversionIsKnown == false)
		{
			// Unknown conversion of signal %1 from %2 to %3 format.
			//
			m_log->errALC5175(signal.appSignalID(), inFormat, outFormat);
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::isDeviceAndAppSignalsIsCompatible(const Hardware::DeviceAppSignal& deviceAppSignal, const AppSignal& appSignal)
	{
		switch(deviceAppSignal.format())
		{
		case E::DataFormat::Float:

			if (deviceAppSignal.size() == SIZE_32BIT && appSignal.analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
			{
				return true;
			}

			return false;

		case E::DataFormat::SignedInt:

			if (deviceAppSignal.size() == SIZE_32BIT && appSignal.analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
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

		Afbl* afbl = m_afbls.value(appItem.strID(), nullptr);

		if (afbl == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		UalAfb* appFb = new UalAfb(appItem, afbl->isBusProcessingAfb());

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

		QString lmID = lmEquipmentID();

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

		QString lmID = lmEquipmentID();

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
			const QStringList& connectionIDs = transmitter.connectionIdsAsList();

			if (connectionIDs.count() == 0)
			{
				// Transmitter has no connection ID (Schema %1, module %2)
				//
				m_log->errALC5160(ualItem->guid(), ualItem->schemaID(), lmEquipmentID());
				result = false;
				continue;
			}

			for(const QString& connectionID : connectionIDs)
			{
				result &= m_optoModuleStorage->appendTxSignal(ualItem->schemaID(), connectionID, transmitter.guid(),
														   lmEquipmentID(),
														   connectedSignal.first,
														   connectedSignal.second,
														   &signalAlreadyInTxList);
			}
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
				AppSignal* s = m_signals->getSignal(id);

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

			result &= getNearestInSignalID(inPin, &nearestSignalID);

			connectedSignals->append(QPair<QString, UalSignal*>(nearestSignalID, ualSignal));
		}

		return result;
	}

	bool ModuleLogicCompiler::getDirectlyConnectedInSignalID(const LogicPin& inPin, QString* directlyConnectedInSignalID)
	{
		TEST_PTR_LOG_RETURN_FALSE(directlyConnectedInSignalID, log());

		directlyConnectedInSignalID->clear();

		const std::vector<QUuid>& accosiatedOutputsUuids = inPin.associatedIOs();

		for(QUuid outPinUuid : accosiatedOutputsUuids)
		{
			UalItem* ualItem = m_pinParent.value(outPinUuid, nullptr);

			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSignal() == true)
			{
				*directlyConnectedInSignalID = ualItem->strID();

				// directly connected input signal can be only one!

				return true;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::getNearestInSignalIDs(const LogicPin& inPin, QStringList* nearestSignalIDs)
	{
		TEST_PTR_LOG_RETURN_FALSE(nearestSignalIDs, log());

		nearestSignalIDs->clear();

		// searching of directly connected input signal
		//
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
				nearestSignalIDs->append(ualItem->strID());

				// directly connected input signal can be only one!

				return true;
			}
		}

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(accosiatedOutputsUuids.size() == 1, log());

		// searching of nearest signal connected like this
		//
		//                +--[ nearestSignalID ]
		//                |
		//	+-------+     |                    +-------+
		//	|  AFB1 |     |	             inPin |  AFB2 |
		//	|	out	|-----+--------------------+       |
		//	|       |                          |       |
		//	+-------+                          +-------+

		QUuid outPinUuid = accosiatedOutputsUuids[0];						// take out pin of AFB1 item

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
			UalItem* nearestUalItem = m_pinParent.value(inPinUuid, nullptr);

			if (nearestUalItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (nearestUalItem->isSignal() == true)
			{
				nearestSignalIDs->append(nearestUalItem->strID());
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::getNearestInSignalID(const LogicPin& inPin, QString* nearestSignalID)
	{
		TEST_PTR_LOG_RETURN_FALSE(nearestSignalID, log());

		nearestSignalID->clear();

		QStringList nearestSignalIDs;

		bool result = getNearestInSignalIDs(inPin, &nearestSignalIDs);

		if (nearestSignalIDs.size() > 0)
		{
			*nearestSignalID = nearestSignalIDs.first();
		}

		return result;
	}

	bool ModuleLogicCompiler::getNearestOutSignalIDs(const LogicPin& outPin, QStringList* nearestSignalIDs)
	{
		TEST_PTR_LOG_RETURN_FALSE(nearestSignalIDs, log());
		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(outPin.IsOutput(), log());

		nearestSignalIDs->clear();

		::std::vector<QUuid> linkedPins = outPin.associatedIOs();

		for(QUuid linkedPin : linkedPins)
		{
			UalItem* ualItem = m_pinParent.value(linkedPin, nullptr);

			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSignal() == true)
			{
				nearestSignalIDs->append(ualItem->strID());
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::getNearestOutSignalID(const LogicPin& outPin, QString* nearestSignalID)
	{
		TEST_PTR_LOG_RETURN_FALSE(nearestSignalID, log());

		nearestSignalID->clear();

		QStringList nearestSignalIDs;

		bool result = getNearestOutSignalIDs(outPin, &nearestSignalIDs);

		if (nearestSignalIDs.size() > 0)
		{
			*nearestSignalID = nearestSignalIDs.first();
		}

		return result;
	}

	bool ModuleLogicCompiler::getNearestSignalID(const LogicPin& inOutPin, QString* nearestSignalID)
	{
		if (inOutPin.IsInput() == true)
		{
			return getNearestInSignalID(inOutPin, nearestSignalID);
		}

		if (inOutPin.IsOutput() == true)
		{
			return getNearestOutSignalID(inOutPin, nearestSignalID);
		}

		assert(false);
		LOG_INTERNAL_ERROR(log());

		return false;
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

		QString connectionID = receiver.connectionIds();

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

		if (receiver.appSignalIdsAsList().size() > 1)
		{
			m_log->errINT1001(QString("SchemaItemReceiver has more then one AppSignalID"), item->schemaID(), item->guid());
		}

		QString rxSignalID = receiver.appSignalIds();

		if (m_chassisSignals.contains(rxSignalID) == false)
		{
			// Single-port Rx signal '%1' is not associated with LM '%2' (Logic schema '%3').
			//
			m_log->errALC5191(rxSignalID, lmEquipmentID(), item->guid(), item->schemaID());
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
																lmEquipmentID(),
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

		QList<Hardware::OptoModuleShared> optoModules = m_optoModuleStorage->getLmAssociatedOptoModules(lmEquipmentID());

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

	bool ModuleLogicCompiler::fillComparatorSet()
	{
		bool result = true;

		for(UalAfb* ualAfb : m_ualAfbs)
		{
			TEST_PTR_CONTINUE(ualAfb);

			result &= addToComparatorSet(ualAfb);
		}

		return result;
	}

	bool ModuleLogicCompiler::initComparatorSignals()
	{
		return true;
	}

	bool ModuleLogicCompiler::finalizeOptoConnectionsProcessing()
	{
		bool result = true;

		QString lmID = lmEquipmentID();

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

			if (ualReceiver->appSignalIdsAsList().size() > 1)
			{
				m_log->errINT1001(QString("SchemaItemReceiver has more then one AppSignalID"), ualItem->schemaID(), ualItem->guid());
			}

			SignalAddress16 rxAddress;

			bool res = m_optoModuleStorage->getRxSignalAbsAddress(ualItem->schemaID(),
													   ualReceiver->connectionIds(),
													   ualReceiver->appSignalIds(),
													   lmEquipmentID(),
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

	bool ModuleLogicCompiler::generateIdrPhaseCode()
	{
		m_idrCode.clear();

		CodeGenProcsToCallArray procs =
		{
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::generateAfbsVersionCheckingCode),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::generateInitAfbsCode),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::generateLoopbacksRefreshingCode),
		};

		bool result = runCodeGenProcs(procs, &m_idrCode);

		return result;
	}

	bool ModuleLogicCompiler::generateAlpPhaseCode()
	{
		m_alpCode.clear();

		CodeGenProcsToCallArray procs =
		{
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::constBitsInitialization),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredRawDataInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::convertAnalogInputSignals),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::generateAppLogicCode),

			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogOptoSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogBusChildSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredTuningAnalogSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogConstSignalsInRegBuf),

			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredInputBusesInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredBusChildBusesInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredOptoBusesInRegBuf),

			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteInputSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteOutputAndInternalSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteOptoSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteBusChildSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredTuningDiscreteSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteConstSignalsInRegBuf),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyOutputSignalsInOutputModulesMemory),
			CODE_GEN_PROC_TO_CALL(ModuleLogicCompiler::copyOptoConnectionsTxData),
		};

		bool result = runCodeGenProcs(procs, &m_alpCode);

		return result;
	}

	bool ModuleLogicCompiler::makeAppLogicCode()
	{
		CodeItem appStartCmd;

		appStartCmd.appStart(0);					// init appStartCmd with address 0

		CodeItem stopCmd;

		stopCmd.stop();

		int alpCodeStartAddr = appStartCmd.sizeW() + m_idrCode.sizeW() + stopCmd.sizeW();

		//

		m_code.comment("Start of IDR phase code");
		m_code.newLine();

		appStartCmd.appStart(alpCodeStartAddr);		// init appStartCmd with real address
		appStartCmd.setComment("set address of ALP phase code start");

		m_code.append(appStartCmd);
		m_code.newLine();

		m_code.append(m_idrCode);

		stopCmd.setComment("end of IDR phase code");

		m_code.append(stopCmd);
		m_code.newLine();

		//

		m_code.comment("Start of ALP phase code");
		m_code.newLine();

		m_code.append(m_alpCode);

		stopCmd.setComment("end of ALP phase code");

		m_code.append(stopCmd);

		return true;
	}

	bool ModuleLogicCompiler::finalizeAppLogicCodeGeneration()
	{
		m_ualSignals.finalizeHeaps();

		return true;
	}

	bool ModuleLogicCompiler::generateAfbsVersionCheckingCode(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_lmDescription->checkAfbVersions() == false)
		{
			return true;
		}

		int checkResultOffset = static_cast<int>(m_lmDescription->checkAfbVersionsOffset(true));		// absolute address
		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		code->comment("AFBs implementation versions checking");

		const std::map<int, std::shared_ptr<Afb::AfbComponent>>& components = m_lmDescription->afbComponents();

		QHash<int, bool> opCodes;

		for(auto const& component : components)
		{
			opCodes.insert(component.second->opCode(), true);
		}

		const int MIN_AFB_OPCODE = 1;
		const int MAX_AFB_OPCODE = 63;

		CodeItem cmd;
		int resultBitNo = 0;

		QString usedCommentStr("Used AFBs opcodes: ");
		QString unusedCommentStr("Unused AFBs opcodes: ");

		bool firstUsed = true;
		bool firstUnused = true;

		for(int afbOpcode = MIN_AFB_OPCODE; afbOpcode <= MAX_AFB_OPCODE; afbOpcode++)
		{
			if (opCodes.contains(afbOpcode) == true)
			{
				if (firstUsed == true)
				{
					usedCommentStr += QString::number(afbOpcode);
					firstUsed = false;
				}
				else
				{
					usedCommentStr += QString(", %1").arg(afbOpcode);
				}
			}
			else
			{
				if (firstUnused == true)
				{
					unusedCommentStr += QString::number(afbOpcode);
					firstUnused = false;
				}
				else
				{
					unusedCommentStr += QString(", %1").arg(afbOpcode);
				}
			}
		}

		code->comment(usedCommentStr);
		code->comment_nl(unusedCommentStr);

		for(int afbOpcode = MIN_AFB_OPCODE; afbOpcode <= MAX_AFB_OPCODE; afbOpcode++)
		{
			resultBitNo = afbOpcode - 1;		// !!! last (63) bit in results is not used!

			if ((resultBitNo % WORD_SIZE) == 0)
			{
				cmd.movConst(bitAccAddr, 0xFFFF);
				cmd.clearComment();
				code->append(cmd);
			}

			if (opCodes.contains(afbOpcode) == true)
			{
				auto component = components.at(afbOpcode);

				cmd.readFuncBlockCompare(afbOpcode, 0,
										component->versionOpIndex(),
										component->impVersion(),
										component->caption());
				cmd.setComment(QString("AFB opcode %1").arg(afbOpcode));
				code->append(cmd);

				cmd.movCompareFlag(bitAccAddr, (resultBitNo % WORD_SIZE));
				cmd.clearComment();
				code->append(cmd);
			}
			else
			{
			//	code->comment(QString("ABF opcode %1 is not used").arg(afbOpcode));
			}

			if (((resultBitNo + 1) % WORD_SIZE) == 0)
			{
				cmd.mov(checkResultOffset + resultBitNo / WORD_SIZE, bitAccAddr);
				cmd.clearComment();
				code->append(cmd);
				code->newLine();
			}
		}

		cmd.mov(checkResultOffset + MAX_AFB_OPCODE / WORD_SIZE, bitAccAddr);
		cmd.clearComment();
		code->append(cmd);
		code->newLine();

		return true;
	}

	bool ModuleLogicCompiler::generateInitAfbsCode(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		//		m_alpCode_init(&m_resourcesUsageInfo.initAfbs);

		LOG_MESSAGE(m_log, QString(tr("Generation of AFB initialization code...")));

		QHash<QString, QString> instanceUsedBy;

		for(UalAfb* ualAfb : m_ualAfbs)
		{
			TEST_PTR_CONTINUE(ualAfb);

			if (ualAfb->hasRam() == true)
			{
				continue;
			}

			QString instantiatorID = ualAfb->instantiatorID();

			if (instanceUsedBy.contains(instantiatorID) == false)
			{
				instanceUsedBy.insert(instantiatorID, ualAfb->label());
			}
			else
			{
				instanceUsedBy.insert(instantiatorID, QString("%1, %2").arg(instanceUsedBy.value(instantiatorID)).arg(ualAfb->label()));
			}
		}

		bool result = true;

		code->comment_nl("AFBs initialization code");

		QHash<QString, int> instantiatorStrIDsMap;

		for(Afbl* afbl : m_afbls)
		{
			TEST_PTR_CONTINUE(afbl);

			for(UalAfb* ualAfb : m_ualAfbs)
			{
				TEST_PTR_CONTINUE(ualAfb);

				if (ualAfb->afbStrID() != afbl->strID())
				{
					continue;
				}

				if (ualAfb->hasRam() == true)
				{
					// initialize all params for each instance of FB with RAM
					//
					result &= generateInitAppFbParamsCode(code, *ualAfb, ualAfb->label());
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

						result &= generateInitAppFbParamsCode(code, *ualAfb, instanceUsedBy.value(instantiatorID));
					}
				}
			}
		}

		//m_alpCode_calculate(&m_resourcesUsageInfo.initAfbs);

		return result;
	}

	bool ModuleLogicCompiler::generateInitAppFbParamsCode(CodeSnippet* code, const UalAfb& appFb, const QString& usedBy)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		const AppFbParamValuesArray& appFbParamValues = appFb.paramValuesArray();

		if (appFbParamValues.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		QString fbCaption = appFb.caption();
		int fbOpcode = appFb.opcode();
		int fbInstance = appFb.instance();

		code->comment(QString(tr("Initialization of %1 (fbtype %2, opcode %3, instance %4, %5, %6)")).
				arg(fbCaption).
				arg(appFb.typeCaption()).
				arg(fbOpcode).
				arg(fbInstance).
				arg(appFb.instantiatorID()).
			   arg(appFb.hasRam() ? "has RAM" : "non RAM"));

		code->comment(QString(tr("Used by item(s): %1")).arg(usedBy));

		displayAfbParams(code, appFb);

		code->newLine();

		bool commandAdded = false;

		for(const AppFbParamValue& paramValue : appFbParamValues)
		{
			int operandIndex = paramValue.operandIndex();

			if (operandIndex == AppFbParamValue::NOT_FB_OPERAND_INDEX)
			{
				continue;
			}

			QString opName = paramValue.opName();

			CodeItem cmd;

			if (paramValue.type() == E::SignalType::Discrete)
			{
				// for discrete parameters
				//
				cmd.writeFuncBlockConst(fbOpcode, fbInstance, operandIndex, paramValue.unsignedIntValue(), fbCaption);
				cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.unsignedIntValue()));

				code->append(cmd);

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

			code->append(cmd);

			commandAdded = true;
		}

		if (commandAdded == true)
		{
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::displayAfbParams(CodeSnippet* code, const UalAfb& appFb)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		const AppFbParamValuesArray& appFbParamValues = appFb.paramValuesArray();

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

			code->comment(commentStr);
		}

		return true;
	}

	bool ModuleLogicCompiler::generateLoopbacksRefreshingCode(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		QList<const UalSignal*> loopbacksUalSignals = m_loopbacks.getLoopbacksUalSignals();

		if (loopbacksUalSignals.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		CodeSnippet analogsRefreshCode;
		CodeSnippet bussesRefreshCode;

		for(const UalSignal* lbSignal :  loopbacksUalSignals)
		{
			TEST_PTR_LOG_RETURN_FALSE(lbSignal, m_log);

			QList<LoopbackShared> loopbacks = m_loopbacks.getLoopbacksByUalSignal(lbSignal);

			QString loopbackIDs = Loopbacks::joinedLoopbackIDs(loopbacks);

			if (lbSignal->isConst() == true)
			{
				continue;			// for const signals refreshing code is not required
			}

			if (lbSignal->ualAddrIsValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			switch(lbSignal->signalType())
			{
			case E::Analog:
				getRefreshingCode(&analogsRefreshCode, loopbackIDs, lbSignal);
				break;

			case E::Discrete:
				if (m_memoryMap.addressInBitMemory(lbSignal->ualAddr().offset()) == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;					// why discrete in not bit-addressed memory
				}
				break;

			case E::Bus:
				getRefreshingCode(&bussesRefreshCode, loopbackIDs, lbSignal);
				break;

			default:
				assert(false);
			}
		}

		CodeSnippet refreshingCode;

		if (analogsRefreshCode.isEmpty() == false)
		{
			refreshingCode.append(analogsRefreshCode);
			refreshingCode.newLine();
		}

		if (bussesRefreshCode.isEmpty() == false)
		{
			refreshingCode.append(bussesRefreshCode);
			refreshingCode.newLine();
		}

		if (refreshingCode.isEmpty() == false)
		{
			code->comment_nl("Loopback signals refreshing code");
			code->append(refreshingCode);
		}

		return result;
	}

	bool ModuleLogicCompiler::getRefreshingCode(CodeSnippet* code, const QString& loopbackID, const UalSignal* lbSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(lbSignal, m_log);

		int offset = lbSignal->ualAddr().offset();
		int sizeW = 0;

		QString busStr;

		switch(lbSignal->signalType())
		{
		case E::Analog:
			assert((lbSignal->dataSize() % WORD_SIZE) == 0);

			sizeW = lbSignal->dataSize() / WORD_SIZE;
			break;

		case E::Bus:
			sizeW = lbSignal->bus()->sizeW();
			busStr = QString("bustype %1 ").arg(lbSignal->busTypeID());
			break;

		case E::Discrete:
		default:
			assert(false);
			return false;
		}

		bool firstCommand = true;

		for(int w = 0; w < sizeW / 2; w++ )
		{
			CodeItem cmd;

			cmd.prevMov32(offset, offset);

			if (firstCommand == true)
			{
				cmd.setComment(QString("refreshing loopback %1 (%2signal %3)").arg(loopbackID).arg(busStr).arg(lbSignal->refSignalIDsJoined()));
				firstCommand = false;
			}

			code->append(cmd);

			offset += 2;
		}

		if ((sizeW % 2) != 0)
		{
			CodeItem cmd;

			cmd.prevMov(offset, offset);

			if (firstCommand == true)
			{
				cmd.setComment(QString("loopback %1 (%2signal %3)").arg(loopbackID).arg(busStr).arg(lbSignal->signal()->appSignalID()));
				firstCommand = false;
			}

			code->append(cmd);
		}

		return true;
	}

	bool ModuleLogicCompiler::constBitsInitialization(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		code->comment(tr("Constant bits initialization"));
		code->newLine();

		CodeItem cmd;

		cmd.movBitConst(m_memoryMap.constBit0Addr(), 0);
		cmd.setComment("const bit 0");

		code->append(cmd);

		cmd.movBitConst(m_memoryMap.constBit1Addr(), 1);
		cmd.setComment("const bit 1");

		code->append(cmd);
		code->newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredRawDataInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		QString regRawDataDescription;

		if (DeviceHelper::getStrProperty(m_lm, "RegRawDataDescription", &regRawDataDescription, m_log) == false)
		{
			return false;
		}

		if (regRawDataDescription.trimmed().isEmpty() == true)
		{
			return true;
		}

		code->comment_nl("Copy acquired raw data");

		assert(false);			// should be implemented when regRawData will exists

		return true;
	}

	bool ModuleLogicCompiler::convertAnalogInputSignals(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		bool result = true;

		QVector<UalSignal*> analogInputSignals;

		analogInputSignals.append(m_acquiredAnalogInputSignals);
		analogInputSignals.append(m_nonAcquiredAnalogInputSignals);

		if (analogInputSignals.isEmpty() == true)
		{
			return true;
		}

		//m_alpCode_init(&m_resourcesUsageInfo.convertAnalogInputSignals);

		code->comment_nl("Convertion of analog input signals");

		CodeItem cmd;

		for(UalSignal* ualSignal : analogInputSignals)
		{
			if (ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			AppSignal* s = ualSignal->getInputSignal();

			if (s == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			assert(s->isAnalog() == true);
			assert(s->isInput() == true);
			assert(s->dataSize() == SIZE_32BIT);
			assert(s->ualAddrIsValid() == true);
			assert(s->ioBufAddr().isValid() == true);

			if (s->needConversion() == false)
			{
				// signal isn't need conversion
				// copy signal only
				//
				cmd.mov32(s->ualAddr().offset(), s->ioBufAddr().offset());
				cmd.setComment(QString("copy analog input %1").arg(s->appSignalID()));
				code->append(cmd);

				continue;
			}

			UalAfb* appFb = m_inOutSignalsToScalAppFbMap.value(s->appSignalID(), nullptr);

			if (appFb == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			bool isTconvAfb = appFb->caption().startsWith("tconv_");

			FbScal fbScal;

			switch(s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				fbScal = m_fbScal[FB_SCALE_16UI_FP_INDEX];
				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				if (isTconvAfb == true)
				{
					fbScal = m_fbScal[FB_TCONV_FP_SI_INDEX];
				}
				else
				{
					fbScal = m_fbScal[FB_SCALE_16UI_SI_INDEX];
				}
				break;

			default:
				assert(false);			// unknown format
				return false;
			}

			if (isTconvAfb == true)
			{
				cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
								   s->ioBufAddr().offset(), appFb->caption());
			}
			else
			{
				cmd.writeFuncBlock(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
								   s->ioBufAddr().offset(), appFb->caption());
			}

			cmd.setComment(QString(tr("conversion of analog input %1")).arg(s->appSignalID()));
			code->append(cmd);

			cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.clearComment();
			code->append(cmd);

			cmd.readFuncBlock32(s->ualAddr().offset(), appFb->opcode(), appFb->instance(),
								fbScal.outputSignalIndex, appFb->caption());
			code->append(cmd);

			code->newLine();
		}

		code->finalizeByNewLine();

		//m_alpCode_calculate(&m_resourcesUsageInfo.convertAnalogInputSignals);

		return result;
	}

	bool ModuleLogicCompiler::generateAppLogicCode(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_ualItems.isEmpty() == true)
		{
			return true;				// has no ualItems to generate app logic code
		}

		LOG_MESSAGE(m_log, QString("Generation of application logic code..."));

		bool result = true;

		// m_alpCode_init(&m_resourcesUsageInfo.appLogicCode);

		code->comment_nl("Application logic code");

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_RETURN_FALSE(ualItem)

			switch(ualItem->type())
			{
			case E::UalItemType::Afb:
				result &= generateAfbCode(code, ualItem);
				break;

			case E::UalItemType::BusComposer:
				result &= generateBusComposerCode(code, ualItem);
				break;

			case E::UalItemType::BusExtractor:
				// specific code generation ONLY for discrete signals connected to bus extractor
				//
				result &= generateDiscreteSignalToBusExtractorCode(code, ualItem);
				break;

			// UalItems that is not required code generation
			//
			case E::UalItemType::Signal:
			case E::UalItemType::Const:
			case E::UalItemType::Transmitter:
			case E::UalItemType::Receiver:
			case E::UalItemType::Terminator:
			case E::UalItemType::LoopbackSource:
			case E::UalItemType::LoopbackTarget:
				break;

			case E::UalItemType::Unknown:
			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		m_memoryMap.setAppBitMemoryDiscreteSignalsHeapSizeW(m_ualSignals.getDiscreteSignalsHeapSizeW());
		m_memoryMap.setAppWordMemoryAnalogAndBusSignalsHeapSizeW(m_ualSignals.getAnalogAndBusSignalsHeapSizeW());

//		m_alpCode_calculate(&m_resourcesUsageInfo.appLogicCode);

		return result;
	}

	bool ModuleLogicCompiler::generateAfbCode(CodeSnippet* code, const UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		if (m_ualAfbs.contains(ualItem->guid()) == false)
		{
			ASSERT_RETURN_FALSE
		}

		const UalAfb* ualAfb = m_ualAfbs.value(ualItem->guid(), nullptr);

		if (ualAfb->isSetFlagsItem() == true)
		{
			return true;		// no code generation required
		}

		TEST_PTR_RETURN_FALSE(ualAfb)

		bool result = true;

		bool isBusProcAfb = false;

		result = isBusProcessingAfb(ualAfb, &isBusProcAfb);

		RETURN_IF_FALSE(result)

		std::vector<int> busProcessingStepsSizes;

		if (isBusProcAfb == true)
		{
			result = calcBusProcessingSteps(ualAfb, &busProcessingStepsSizes);

			RETURN_IF_FALSE(result)
		}
		else
		{
			busProcessingStepsSizes.push_back(0);		// one dummy processing step for non-bus AFBs
		}

		int busProcessingStepsNumber = static_cast<int>(busProcessingStepsSizes.size());

		if (busProcessingStepsNumber > 1 && ualAfb->hasRam() == true)
		{
			assert(false);				// AFB's with ram is not allow multistep processing now
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		int currentBusSignalOffsetBits = 0;

		for(int stepNo = 0; stepNo < busProcessingStepsNumber; stepNo++)
		{
			BusProcessingStepInfo bpStepInfo;

			bpStepInfo.stepsNumber = busProcessingStepsNumber;
			bpStepInfo.currentStep = stepNo;
			bpStepInfo.currentStepSizeBits = busProcessingStepsSizes[stepNo];
			bpStepInfo.currentBusSignalOffsetW = currentBusSignalOffsetBits / SIZE_16BIT;

			//

			result &= generateSignalsToAfbInputsCode(code, ualAfb, bpStepInfo);

			result &= startAfb(code, ualAfb, bpStepInfo);

			result &= generateAfbOutputsToSignalsCode(code, ualAfb, bpStepInfo);

			//

			currentBusSignalOffsetBits += bpStepInfo.currentStepSizeBits;

			assert((currentBusSignalOffsetBits % SIZE_16BIT) == 0);
		}

		code->newLine();

		return result;
	}

	bool ModuleLogicCompiler::generateSignalsToAfbInputsCode(CodeSnippet* code, const UalAfb* ualAfb,
															 const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);

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

			result &= generateSignalToAfbInputCode(code, ualAfb, inAfbSignal, inUalSignal, bpStepInfo);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateSignalToAfbInputCode(CodeSnippet* code, const UalAfb* ualAfb,
														   const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
														   const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inUalSignal, m_log);

		if (inUalSignal->isCanBeConnectedTo(*ualAfb, inAfbSignal, log()) == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inUalSignal->ualItemGuid(), inUalSignal->ualItemLabel(), ualAfb->schemaID());
			return false;
		}

		// inUalSignal and inAfbSignal are compatible
		//
		if (inUalSignal->checkUalAddr() == false)
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

		CodeItem cmd;

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
				Address16 readUalAddr = m_ualSignals.getSignalReadAddress(*inUalSignal, true);

				if (readUalAddr.isValid() == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				cmd.writeFuncBlockBit(afbOpcode, afbInstance, afbSignalIndex,
									  readUalAddr,
									  afbCaption);
				cmd.setComment(QString("%1.%2 <= %3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->appSignalID()));
			}

			code->append(cmd);

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
				Address16 readUalAddr = m_ualSignals.getSignalReadAddress(*inUalSignal, true);

				if (readUalAddr.isValid() == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				cmd.writeFuncBlock32(afbOpcode, afbInstance, afbSignalIndex,
									 readUalAddr,
									 afbCaption);
				cmd.setComment(QString("%1.%2 <= %3").arg(afbCaption).arg(signalCaption).arg(inUalSignal->appSignalID()));
			}

			code->append(cmd);

			break;

		case E::SignalType::Bus:
			result = generateSignalToAfbBusInputCode(code, ualAfb, inAfbSignal, inUalSignal, bpStepInfo);
			break;

		default:
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
															  const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
															  const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inUalSignal, m_log);

		assert(inAfbSignal.isBus() == true);

		bool result = true;

		switch(inUalSignal->signalType())
		{
		case E::SignalType::Discrete:
			result =  generateDiscreteSignalToAfbBusInputCode(code, ualAfb, inAfbSignal, inUalSignal, bpStepInfo);
			break;

		case E::SignalType::Bus:
			result =  generateBusSignalToAfbBusInputCode(code, ualAfb, inAfbSignal, inUalSignal, bpStepInfo);
			break;

		case E::SignalType::Analog:
			LOG_INTERNAL_ERROR(m_log);			// uncompatible conection, this error must be detected early!
			return false;

		default:
			assert(false);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
																	  const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
																	  const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inUalSignal, m_log);

		if (inUalSignal->isDiscrete() == false || inAfbSignal.isBus() == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (inUalSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(), inUalSignal->ualItemSchemaID());
			return false;
		}

		if (inAfbSignal.busDataFormat() != E::BusDataFormat::Discrete && inAfbSignal.busDataFormat() != E::BusDataFormat::Mixed)
		{
			// Discrete signal %1 is connected to non-discrete or non-mixed bus input (Logic schema %2)
			//
			m_log->errALC5124(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(),
							  ualAfb->guid(), ualAfb->schemaID());

			return false;
		}

		int inputSize = inAfbSignal.size();

		Q_ASSERT(inputSize == bpStepInfo.currentStepSizeBits);

		if (inputSize != SIZE_16BIT && inputSize != SIZE_32BIT)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		QString comment = QString("%1.%2 << %3").arg(ualAfb->caption()).arg(inAfbSignal.caption()).arg(inUalSignal->refSignalIDsJoined());

		CodeItem cmd;

		if (inUalSignal->isConst() == true)
		{
			if (inputSize == SIZE_16BIT)
			{
				quint16 constVal = inUalSignal->constDiscreteValue() == 0 ? 0 : 0xFFFF;

				cmd.writeFuncBlockConst(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), constVal, ualAfb->caption());
			}
			else
			{
				assert(inputSize == SIZE_32BIT);

				qint32 constVal = inUalSignal->constDiscreteValue() == 0 ? 0 : static_cast<qint32>(0xFFFFFFFF);

				cmd.writeFuncBlockConstInt32(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), constVal, ualAfb->caption());
			}

			cmd.setComment(comment);
			code->append(cmd);
		}
		else
		{
			int wordAccAddr = m_memoryMap.wordAccumulatorAddress();

			bool decrementReadCount = bpStepInfo.isLastStep();

			Address16 readUalAddr = m_ualSignals.getSignalReadAddress(*inUalSignal, decrementReadCount);

			if (readUalAddr.isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			cmd.fill(Address16(wordAccAddr, 0), readUalAddr);
			code->append(cmd);

			if (inputSize == SIZE_16BIT)
			{
				cmd.writeFuncBlock(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), wordAccAddr, ualAfb->caption());
			}
			else
			{
				assert(inputSize == SIZE_32BIT);

				cmd.mov(wordAccAddr + 1, wordAccAddr);
				code->append(cmd);

				cmd.writeFuncBlock32(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), wordAccAddr, ualAfb->caption());
			}

			cmd.setComment(comment);
			code->append(cmd);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateBusSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
																 const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
																 const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inUalSignal, m_log);

		if (inUalSignal->isBus() == false || inAfbSignal.isBus() == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = inUalSignal->isCanBeConnectedTo(*ualAfb, inAfbSignal, log());

		if (result == false)
		{
			// Uncompatible signals connection (Logic schema %1).
			//
			log()->errALC5117(inUalSignal->ualItemGuid(), inUalSignal->ualItemLabel(), ualAfb->guid(), ualAfb->label(), ualAfb->schemaID());
			return false;
		}

		if (inUalSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inUalSignal->refSignalIDsJoined(), inUalSignal->ualItemGuid(), inUalSignal->ualItemSchemaID());
			return false;
		}

		Address16 inSignalUalAddr = m_ualSignals.getSignalReadAddress(*inUalSignal, bpStepInfo.isLastStep());

		if (inSignalUalAddr.isValid() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Q_ASSERT(inSignalUalAddr.bit() == 0);

		int addrFrom = inSignalUalAddr.offset();

		CodeItem cmd;

		switch(bpStepInfo.currentStepSizeBits)
		{
		case SIZE_16BIT:

			addrFrom += bpStepInfo.currentBusSignalOffsetW;
			cmd.writeFuncBlock(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), addrFrom, ualAfb->caption());
			break;

		case SIZE_32BIT:

			addrFrom += bpStepInfo.currentBusSignalOffsetW;
			cmd.writeFuncBlock32(ualAfb->opcode(), ualAfb->instance(), inAfbSignal.operandIndex(), addrFrom, ualAfb->caption());
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		cmd.setComment(QString("%1.%2 << %3 (part %4)").
					   arg(ualAfb->caption()).arg(inAfbSignal.caption()).
					   arg(inUalSignal->refSignalIDsJoined()).arg(bpStepInfo.currentStep + 1));

		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::startAfb(CodeSnippet* code, const UalAfb* ualAfb, const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);

		CodeItem cmd;

		cmd.start(ualAfb->opcode(), ualAfb->instance(), ualAfb->caption(), ualAfb->runTime());

		if (bpStepInfo.stepsNumber == 1)
		{
			cmd.setComment(QString(tr("compute %1 @%2")).arg(ualAfb->caption()).arg(ualAfb->label()));
		}
		else
		{
			cmd.setComment(QString(tr("compute %1 @%2 (step %3/%4)")).
							arg(ualAfb->caption()).arg(ualAfb->label()).
							arg(bpStepInfo.currentStep + 1).
							arg(bpStepInfo.stepsNumber));
		}

		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateAfbOutputsToSignalsCode(CodeSnippet* code, const UalAfb* ualAfb,
															  const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);

		bool result = true;

		for(const LogicPin& outPin : ualAfb->outputs())
		{
			if (outPin.IsOutput() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (isConnectedToTerminatorOnly(outPin) == true)
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

			result &= generateAfbOutputToSignalCode(code, ualAfb, outAfbSignal, outUalSignal, bpStepInfo);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAfbOutputToSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
															const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal,
															const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(outUalSignal, m_log);

		if (outUalSignal->isConst() == true)
		{
			assert(false);							// can't assign value to const ual signal
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		if (outUalSignal->isCanBeConnectedTo(*ualAfb, outAfbSignal, log()) == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outUalSignal->ualItemGuid(), outUalSignal->appSignalID(), ualAfb->schemaID());
			return false;
		}

		if (outUalSignal->checkUalAddr() == false)
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

		CodeItem cmd;

		switch(outAfbSignal.type())
		{
		case E::SignalType::Discrete:

			{
				Address16 writeUalAddr = m_ualSignals.getSignalWriteAddress(*outUalSignal);

				if (writeUalAddr.isValid() == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				cmd.readFuncBlockBit(writeUalAddr, afbOpcode, afbInstance, afbSignalIndex, afbCaption);
				cmd.setComment(QString("%1 <= %2.%3").arg(outUalSignal->appSignalID()).arg(afbCaption).arg(signalCaption));

				code->append(cmd);
			}

			break;

		case E::SignalType::Analog:

			{
				Address16 writeUalAddr = m_ualSignals.getSignalWriteAddress(*outUalSignal);

				if (writeUalAddr.isValid() == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				cmd.readFuncBlock32(writeUalAddr, afbOpcode, afbInstance, afbSignalIndex, afbCaption);
				cmd.setComment(QString("%1 <= %2.%3").arg(outUalSignal->appSignalID()).arg(afbCaption).arg(signalCaption));

				code->append(cmd);
			}

			break;

		case E::SignalType::Bus:

			result = generateAfbBusOutputToBusSignalCode(code, ualAfb, outAfbSignal, outUalSignal, bpStepInfo);
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAfbBusOutputToBusSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
																  const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal,
																  const BusProcessingStepInfo& bpStepInfo)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(outUalSignal, m_log);

		if (outUalSignal->isBus() == false || outAfbSignal.isBus() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (outUalSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(outUalSignal->refSignalIDsJoined(), outUalSignal->ualItemGuid(), outUalSignal->ualItemSchemaID());
			return false;
		}

		Address16 outUalSignalAddr = m_ualSignals.getSignalWriteAddress(*outUalSignal);

		if (outUalSignalAddr.isValid() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Q_ASSERT(outUalSignalAddr.bit() == 0);

		int addrTo = outUalSignalAddr.offset();

		CodeItem cmd;

		switch(bpStepInfo.currentStepSizeBits)
		{
		case SIZE_16BIT:

			addrTo += bpStepInfo.currentBusSignalOffsetW;
			cmd.readFuncBlock(addrTo, ualAfb->opcode(), ualAfb->instance(), outAfbSignal.operandIndex(), ualAfb->caption());
			break;

		case SIZE_32BIT:

			addrTo += bpStepInfo.currentBusSignalOffsetW;
			cmd.readFuncBlock32(addrTo, ualAfb->opcode(), ualAfb->instance(), outAfbSignal.operandIndex(), ualAfb->caption());
			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		cmd.setComment(QString("%1 (part %2) << %3.%4").
					   arg(outUalSignal->refSignalIDsJoined()).arg(bpStepInfo.currentStep + 1).
					   arg(ualAfb->caption()).arg(outAfbSignal.caption()));

		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::calcBusProcessingSteps(const UalAfb* ualAfb, std::vector<int>* busProcessingStepsSizes)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busProcessingStepsSizes, m_log);

		bool result = true;

		busProcessingStepsSizes->clear();

		std::vector<std::vector<int>> inputPinsSizes;
		int inputSignalSize = -1;
		bool allBusInputsConnectedToDiscretes = false;

		result = getPinsAndSignalsBusSizes(ualAfb, ualAfb->inputs(), &inputPinsSizes, &inputSignalSize, true, &allBusInputsConnectedToDiscretes);

		RETURN_IF_FALSE(result);

		std::vector<std::vector<int>> outputPinsSizes;
		int outputSignalSize = -1;
		bool dummyBool = false;

		result = getPinsAndSignalsBusSizes(ualAfb, ualAfb->outputs(), &outputPinsSizes, &outputSignalSize, false, &dummyBool);

		RETURN_IF_FALSE(result);

		if (allBusInputsConnectedToDiscretes == true)
		{
			if (outputSignalSize != -1)
			{
				inputSignalSize = outputSignalSize;
			}
			else
			{
				// all inputs connected to discretes
				// and all outputs connected to terminator
				// bus signal size can't be determined!
				// this error should be detected early
				//
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't determine bus signal size on item %1 (Logic schema %2)").
														arg(ualAfb->label()).arg(ualAfb->schemaID()));
				return false;
			}
		}

		if (outputSignalSize == -1)
		{
			outputSignalSize = inputSignalSize;		// may be all bus outputs are connected to terminator
		}

		if (inputSignalSize != outputSignalSize)
		{
			// in and out busses has different sizes
			// this error should be detected early
			//
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Different bus types on input and output of item %1 (Logic schema %2)").
													arg(ualAfb->label()).arg(ualAfb->schemaID()));
			return false;
		}

		// find sizes common for all pins (intersection of all pins sizes)
		//
		std::set<int> allSizes;

		for(const std::vector<int>& inPinSizes : inputPinsSizes)
		{
			allSizes.insert(inPinSizes.begin(), inPinSizes.end());
		}

		for(const std::vector<int>& outPinSizes : outputPinsSizes)
		{
			allSizes.insert(outPinSizes.begin(), outPinSizes.end());
		}

		std::vector<int> commonPinsSizes;

		for(int size : allSizes)
		{
			bool sizeExistsInAllPins = true;

			for(const std::vector<int>& inPinSizes : inputPinsSizes)
			{
				sizeExistsInAllPins &= std::find(inPinSizes.begin(), inPinSizes.end(), size) != inPinSizes.end();
			}

			for(const std::vector<int>& outPinSizes : outputPinsSizes)
			{
				sizeExistsInAllPins &= std::find(outPinSizes.begin(), outPinSizes.end(), size) != outPinSizes.end();
			}

			if (sizeExistsInAllPins == true)
			{
				commonPinsSizes.push_back(size);
			}
		}

		result = partitionOfInteger(inputSignalSize, commonPinsSizes, busProcessingStepsSizes);

		if (result == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't determine bus processing steps sizes on item %1. Bus signal size is not multiple to pins data sizes (Logic schema %2)").
											arg(ualAfb->label()).arg(ualAfb->schemaID()));
		}

		return result;
	}

	bool ModuleLogicCompiler::getPinsAndSignalsBusSizes(const UalAfb* ualAfb, const std::vector<LogicPin>& pins,
														std::vector<std::vector<int>>* pinsSizes, int* busSignalsSize, bool isInputs,
														bool* allBusInputsConnectedToDiscretes)
	{
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(pinsSizes, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busSignalsSize, m_log);
		TEST_PTR_LOG_RETURN_FALSE(allBusInputsConnectedToDiscretes, m_log);

		bool result = true;

		*busSignalsSize = -1;
		*allBusInputsConnectedToDiscretes = true;

		for(const LogicPin& pin : pins)
		{
			LogicAfbSignal afbSignal;

			bool res = ualAfb->getAfbSignalByPin(pin, &afbSignal);

			RETURN_IF_FALSE(res);

			if (afbSignal.isBus() == false)
			{
				continue;
			}

			std::vector<int> pinSizes = afbSignal.allSizes();

			for(int size : pinSizes)
			{
				if ((size % SIZE_16BIT) != 0)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("Pin %1.%2 data size %3 is not multiple to 16 bit (Logic schema %4, item %5)").
													arg(ualAfb->caption()).
													arg(pin.caption()).
													arg(size).
													arg(ualAfb->schemaID()).
													arg(ualAfb->label()));
					result = false;
				}
			}

			pinsSizes->push_back(pinSizes);

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

			*allBusInputsConnectedToDiscretes = false;

			if (ualSignal->isBus() == false)
			{
				assert(false);					// connected signal is not bus
												// this error must be detected early
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			int signalSize = ualSignal->dataSize();			// signalSize should be equal on all bus inputs or outputs

			if ((signalSize % SIZE_16BIT) != 0)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Bus signal %1 data size %2 is not multiple to 16 bit (Logic schema %3, item %4)").
												arg(ualSignal->appSignalID()).
												arg(signalSize).
												arg(ualAfb->schemaID()).
												arg(ualAfb->label()));
				result = false;
				continue;
			}

			if (*busSignalsSize == -1)
			{
				*busSignalsSize = signalSize;
			}
			else
			{
				if (*busSignalsSize != signalSize)
				{
					// Different busTypes on AFB inputs (or outputs) (Logic schema %1, item %2).
					//
					m_log->errALC5123(ualAfb->guid(), ualAfb->schemaID(), ualAfb->label());
					return false;
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::isBusProcessingAfb(const UalAfb* ualAfb, bool* isBusProcessing)
	{
		if (ualAfb == nullptr || isBusProcessing == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		Afbl* afbl = m_afbls.value(ualAfb->strID(), nullptr);

		if (afbl == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		*isBusProcessing = afbl->isBusProcessingAfb();

		return true;
	}

	bool ModuleLogicCompiler::generateBusComposerCode(CodeSnippet* code,  const UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

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
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		BusShared bus = ualBusSignal->bus();

		if (bus == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		bool result = true;

		code->comment_nl(QString("BusComposer %1 processing").arg(ualItem->label()));

		int count = 0;

		code->append(codeSetMemory(ualBusSignal->ualAddr().offset(), 0, bus->sizeW(), QString("init %1").arg(ualBusSignal->appSignalID())));

		for(const BusSignal& busSignal : bus->busSignals())
		{
			UalSignal* inputSignal = getUalSignalByPinCaption(ualItem, busSignal.signalID, true);

			UalSignal* busChildSignal = ualBusSignal->getBusChildSignal(busSignal.signalID);

			if (inputSignal == nullptr || busChildSignal == nullptr)
			{
				result = false;
				continue;
			}

			if (inputSignal->isCanBeConnectedTo(busChildSignal, log()) == false)
			{
				assert(false);						// this error should be detected early
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (inputSignal->checkUalAddr() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(inputSignal->appSignalID(), inputSignal->ualItemGuid(), inputSignal->ualItemSchemaID());
				return false;
			}

			if (busChildSignal->ualAddrIsValid() == false)
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
				res = generateAnalogSignalToBusAnalogInputCode(code, inputSignal, busChildSignal, busSignal);
				break;

			case E::SignalType::Discrete:
				res = generateDiscreteSignalToBusDiscreteInputCode(code, inputSignal, busChildSignal, busSignal);
				break;

			case E::SignalType::Bus:
				{
					switch(inputSignal->signalType())
					{
					case E::SignalType::Discrete:
						res = generateDiscreteSignalToBusBusInputCode(code, inputSignal, busChildSignal);
						break;

					case E::SignalType::Bus:
						res = generateBusSignalToBusBusInputCode(code, inputSignal, busChildSignal, busSignal);
						break;

					default:
						res = false;
						LOG_INTERNAL_ERROR(m_log);
					}
				}
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
			code->newLine();
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

	bool ModuleLogicCompiler::generateAnalogSignalToBusAnalogInputCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);

		Q_ASSERT(busChildSignal->ualAddrIsValid() == true);
		Q_ASSERT(busChildSignal->ualAddr().bit() == 0);

		if (busSignal.conversionRequired() == true)
		{
			return getAnalogSignalToInbusSignalConversionCode(code, inputSignal, busChildSignal, busSignal);
		}

		QString inputSignalIDs = inputSignal->refSignalIDsJoined();
		QString busChildSignalIDs = busChildSignal->refSignalIDsJoined();

		CodeItem cmd;

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
			Address16 inputSignalAddr = m_ualSignals.getSignalReadAddress(*inputSignal, true);

			if (inputSignalAddr.isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			Q_ASSERT(inputSignalAddr.bit() == 0);

			cmd.mov32(busChildSignal->ualAddr().offset(), inputSignalAddr.offset());
			cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignalIDs));
		}

		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::getAnalogSignalToInbusSignalConversionCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		if (busSignal.hasKnownConversion() == false)
		{
			// Unknown conversion of signal %1 to inbus signal %2 (Logic schema %3)
			//
			m_log->errALC5153(inputSignal->appSignalID(), busChildSignal->appSignalID(), busChildSignal->ualItemSchemaID());
			return false;
		}

		if (busSignal.is_SignedInt32_To_Unsigned16_BE_NoScale_coversion() == true)
		{
			return get_SignedInt32_To_Unsigned16_BE_NoScale_inbusSignalCoversionCode(code, inputSignal, busChildSignal, busSignal);
		}

		LOG_INTERNAL_ERROR(m_log);
		return false;
	}

	bool ModuleLogicCompiler::get_SignedInt32_To_Unsigned16_BE_NoScale_inbusSignalCoversionCode(CodeSnippet* code,
																								const UalSignal* inputSignal,
																								const UalSignal* busChildSignal,
																								const BusSignal& busSignal)
	{
		if (busSignal.is_SignedInt32_To_Unsigned16_BE_NoScale_coversion() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		CodeItem cmd;

		if (inputSignal->isConst() == true)
		{
			quint16 constValue = inputSignal->constAnalogIntValue() & 0xFFFF;					// get low word of const

			cmd.movConst(busChildSignal->ualAddr().offset(), constValue);
			cmd.setComment(QString("%1 <= %2 (SignedInt32_To_Unsigned16_BE_NoScale)").arg(busChildSignal->refSignalIDsJoined()).arg(constValue));
		}
		else
		{
			cmd.mov(busChildSignal->ualAddr().offset(), inputSignal->ualAddr().offset() + 1);	// move low word of inputSignal only
			cmd.setComment(QString("%1 <= %2 (SignedInt32_To_Unsigned16_BE_NoScale)").arg(busChildSignal->refSignalIDsJoined()).arg(inputSignal->refSignalIDsJoined()));
		}

		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToBusDiscreteInputCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);

		assert(busChildSignal->ualAddrIsValid() == true);

		if (busSignal.conversionRequired() == true)
		{
			LOG_INTERNAL_ERROR(m_log);				// bus signals conversion is not implemented now
			return false;
		}

		QString inputSignalIDs = inputSignal->refSignalIDsJoined();
		QString busChildSignalIDs = busChildSignal->refSignalIDsJoined();

		CodeItem cmd;

		if (inputSignal->isConst() == true)
		{
			if (inputSignal->constDiscreteValue() != 0)
			{
				cmd.movBitConst(busChildSignal->ualAddr(), inputSignal->constDiscreteValue());
				cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignal->constDiscreteValue()));
				code->append(cmd);
			}
		}
		else
		{
			Address16 readUalAddr = m_ualSignals.getSignalReadAddress(*inputSignal, true);

			if (readUalAddr.isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			cmd.movBit(busChildSignal->ualAddr(), readUalAddr);
			cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignalIDs));
			code->append(cmd);
		}

		return true;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToBusBusInputCode(CodeSnippet* code,
																	  UalSignal* inputSignal,
																	  UalSignal* busChildSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);

		if (inputSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inputSignal->appSignalID(), inputSignal->ualItemGuid(), inputSignal->ualItemSchemaID());
			return false;
		}

		if (busChildSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(busChildSignal->appSignalID(), busChildSignal->ualItemGuid(), busChildSignal->ualItemSchemaID());
			return false;
		}

		Q_ASSERT(inputSignal->isDiscrete() == true);
		Q_ASSERT(busChildSignal->isBus() == true);

		TEST_PTR_LOG_RETURN_FALSE(busChildSignal->bus(), m_log);

		int busSizeW = busChildSignal->bus()->sizeW();

		CodeItem cmd;

		if (inputSignal->isConst() == true)
		{
			if (inputSignal->constValue() == 1)
			{
				switch(busSizeW)
				{
				case 1:
					cmd.movConst(busChildSignal->ualAddr(), 0xFFFF);
					break;

				case 2:
					cmd.movConstUInt32(busChildSignal->ualAddr(), 0xFFFFFFFFl);
					break;

				default:
					cmd.setMem(busChildSignal->ualAddr(), 0xFFFF, busSizeW);
				}

				cmd.setComment(QString("%1 <= %2").arg(busChildSignal->appSignalID()).arg(inputSignal->appSignalID()));

				code->append(cmd);
			}
		}
		else
		{
			int wordAccAddr = m_memoryMap.wordAccumulatorAddress();

			Address16 inputSignalAddr = m_ualSignals.getSignalReadAddress(*inputSignal, true);

			if (inputSignalAddr.isValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			cmd.fill(wordAccAddr, inputSignalAddr.offset(), inputSignalAddr.bit());
			cmd.setComment(QString("%1 <= %2").arg(busChildSignal->appSignalID()).arg(inputSignal->appSignalID()));

			code->append(cmd);

			if (busSizeW > 1)
			{
				cmd.fill(wordAccAddr + 1, inputSignalAddr.offset(), inputSignalAddr.bit());
				code->append(cmd);
			}

			for(int i = 0; i < busSizeW / 2; i++)
			{
				cmd.mov32(busChildSignal->ualAddr().offset() + i * 2, wordAccAddr);
				code->append(cmd);
			}

			if ((busSizeW % 2) != 0)
			{
				cmd.mov(busChildSignal->ualAddr().offset() + busSizeW - 1, wordAccAddr);
				code->append(cmd);
			}

			code->append(cmd);
		}

		return true;
	}

	bool ModuleLogicCompiler::generateBusSignalToBusBusInputCode(CodeSnippet* code, UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);

		if (inputSignal->ualAddrIsValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inputSignal->appSignalID(), inputSignal->ualItemGuid(), inputSignal->ualItemSchemaID());
			return false;
		}

		if (busChildSignal->ualAddrIsValid() == false)
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

		CodeItem cmd;

		int busSizeW = busChildSignal->sizeW();

		if (busSizeW == 0)
		{
			LOG_INTERNAL_ERROR(m_log);			// busSizeW cannot be 0
			return false;
		}

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
		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToBusExtractorCode(CodeSnippet* code, const UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		const UalBusExtractor* busExtractor = ualItem->ualBusExtractor();

		if (busExtractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<LogicPin>& inputs = busExtractor->inputs();

		if (inputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const LogicPin& inPin = inputs[0];

		const std::vector<QUuid>& associatedOutPins = inPin.associatedIOs();

		if (associatedOutPins.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		UalSignal* inputSignal = m_ualSignals.get(associatedOutPins[0]);

		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);

		if (inputSignal->isDiscrete() == false)
		{
			return true;			// it is OK, bus extractor is connected to non-discrete signal
									// specific code generation is not required
		}

		UalSignal* ualBusSignal = m_ualSignals.get(inPin.guid());

		TEST_PTR_LOG_RETURN_FALSE(ualBusSignal, m_log);

		BusShared bus = m_signals->getBus(ualBusSignal->busTypeID());

		TEST_PTR_LOG_RETURN_FALSE(bus, m_log);

		if (bus->busDataFormat() != E::BusDataFormat::Discrete)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		code->comment_nl(QString("Discrete signal to BusExtractor %1 processing").arg(ualItem->label()));

		bool result = true;
		int count = 0;

		code->append(codeSetMemory(ualBusSignal->ualAddr().offset(), 0, bus->sizeW(), QString("init %1").arg(ualBusSignal->appSignalID())));

		for(const BusSignal& busSignal : bus->busSignals())
		{
			UalSignal* busChildSignal = ualBusSignal->getBusChildSignal(busSignal.signalID);

			if (inputSignal == nullptr || busChildSignal == nullptr)
			{
				result = false;
				continue;
			}

			if (busChildSignal->isCompatible(inputSignal, log()) == false)
			{
				assert(false);						// this error should be detected early
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (inputSignal->checkUalAddr() == false)
			{
				// Undefined UAL address of signal '%1' (Logic schema '%2').
				//
				m_log->errALC5105(inputSignal->appSignalID(), inputSignal->ualItemGuid(), inputSignal->ualItemSchemaID());
				return false;
			}

			if (busChildSignal->ualAddrIsValid() == false)
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
			case E::SignalType::Bus:
				LOG_INTERNAL_ERROR(m_log);
				res = false;
				break;

			case E::SignalType::Discrete:
				res = generateDiscreteSignalToBusDiscreteInputCode(code, inputSignal, busChildSignal, busSignal);
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
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::generateMemCopyCode(Address16 toAddr, Address16 fromAddr,
												  int sizeW, const QString& comment,
												  CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (toAddr.isValid() == false ||
			fromAddr.isValid() == false ||
			sizeW <= 0)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		CodeItem cmd;

		switch(sizeW)
		{
		case 1:
			cmd.mov(toAddr, fromAddr);
			break;

		case 2:
			cmd.mov32(toAddr, fromAddr);
			break;

		default:
			cmd.movMem(toAddr, fromAddr, sizeW);
		}

		if (comment.isEmpty() == false)
		{
			cmd.setComment(comment);
		}

		code->append(cmd);

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
		case E::UalItemType::Signal:
			// extractor connected to signal
			//
			srcSignalUuid = extractorSourceItem->guid();
			break;

		case E::UalItemType::Afb:
		case E::UalItemType::BusComposer:
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
				m_log->errALC5120(ualItem->guid(), ualItem->label(), pinCaption, ualItem->schemaID());
				return nullptr;
			}

			return ualSignal;
		}

		// Pin with caption '%1' is not found in schema item (Logic schema '%2').
		//
		m_log->errALC5106(pinCaption, ualItem->guid(), ualItem->schemaID());

		return nullptr;
	}

	bool ModuleLogicCompiler::addToComparatorSet(const UalAfb* appFb)
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

		RETURN_IF_FALSE(result);

		m_cmpSet->insert(lmEquipmentID(), cmp);

		return true;
	}

	bool ModuleLogicCompiler::initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* appFb)
	{
		TEST_PTR_LOG_RETURN_FALSE(appFb, log());

		// set compare type of value
		//
		bool isConstComparator = appFb->isConstComaparator();
		bool hysteresisIsConst = appFb->caption().contains("_dh_") == false;

		//	set comparator type, compare-value, hysteresis-value
		//
		for(Afb::AfbParam pv : appFb->logicFb().params())
		{
			if (pv.opName() == "i_conf") // set comparator type: =(1(SI)), > (2(SI)), < (3(SI)), ? (4(SI)),= (5(FP)), > (6(FP)), < (7(FP)), ? (8(FP))
			{
				switch (pv.value().toInt())
				{
					case 1:
					case 5:			cmp->setCmpType(E::CmpType::Equal);		break;
					case 2:
					case 6:			cmp->setCmpType(E::CmpType::Greate);	break;
					case 3:
					case 7:			cmp->setCmpType(E::CmpType::Less);		break;
					case 4:
					case 8:			cmp->setCmpType(E::CmpType::NotEqual);	break;
				}
			}

			if (pv.opName() == "i_sp_s" && isConstComparator == true) // set compare-value
			{
				switch (pv.dataFormat())
				{
					case E::DataFormat::Float :			cmp->compare().setConstValue(pv.value().toDouble());	break;
					case E::DataFormat::SignedInt :		cmp->compare().setConstValue(pv.value().toInt());		break;
					case E::DataFormat::UnsignedInt :	cmp->compare().setConstValue(pv.value().toInt());		break;
					default:							assert(0);
				}
			}

			if (pv.opName() == "hysteresis" && hysteresisIsConst == true) // set hysteresis-value
			{
				switch (pv.dataFormat())
				{
					case E::DataFormat::Float :			cmp->hysteresis().setConstValue(pv.value().toDouble());	break;
					case E::DataFormat::SignedInt :		cmp->hysteresis().setConstValue(pv.value().toInt());	break;
					case E::DataFormat::UnsignedInt :	cmp->hysteresis().setConstValue(pv.value().toInt());	break;
					default:							assert(0);
				}
			}
		}

		// set comparator signals
		//
		for(const LogicPin& pin : appFb->inputs())
		{

			UalSignal* ualSignal = m_ualSignals.get(pin.guid());

			if (ualSignal == nullptr)
			{
				continue;
			}

			if (ualSignal->isAnalog() == false)
			{
				continue;
			}

			// input Signal
			//
			if (pin.caption() == "in")
			{
				cmp->setInAnalogSignalFormat(ualSignal->analogSignalFormat());

				cmp->input().setSignalParams(	ualSignal->appSignalID(),
												ualSignal->isAcquired(),
												ualSignal->isConst(),
												ualSignal->constValueIfConst());
			}

			// compare Signal
			//
			if (pin.caption() == "set" && isConstComparator == false) //
			{
				cmp->compare().setSignalParams(	ualSignal->appSignalID(),
												ualSignal->isAcquired(),
												ualSignal->isConst(),
												ualSignal->constValueIfConst());
			}

			// hysteresis Signal
			//
			if ((pin.caption() == "hyst" || pin.caption() == "db") && hysteresisIsConst == false)
			{
				cmp->setHysteresisPinCaption(pin.caption());

				cmp->hysteresis().setSignalParams(	ualSignal->appSignalID(),
													ualSignal->isAcquired(),
													ualSignal->isConst(),
													ualSignal->constValueIfConst());

				cmp->setHysteresisIsConstSignal(ualSignal->isConst());
			}
		}

		for(const LogicPin& pin : appFb->outputs())
		{
			UalSignal* ualSignal = m_ualSignals.get(pin.guid());

			if (ualSignal == nullptr)
			{
				continue;
			}

			if (ualSignal->isDiscrete() == false)
			{
				continue;
			}

			// output Signal
			//
			if (pin.caption() == "out")
			{
				cmp->output().setSignalParams(	ualSignal->appSignalID(),
												ualSignal->isAcquired(),
												ualSignal->isConst(),
												ualSignal->constValueIfConst());
			}
		}

		//
		//
		cmp->setLabel(appFb->label());
		cmp->setPrecision(appFb->logicFb().precision());
		cmp->setSchemaID(appFb->schemaID());
		cmp->setSchemaItemUuid(appFb->guid());

		// tests
		//
		if (cmp->input().appSignalID().isEmpty() == true)
		{
			assert(false);
			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty input signal").arg(appFb->caption()).arg(appFb->schemaID());
			LOG_INTERNAL_ERROR_MSG(m_log, strError);
			qDebug() << strError;
			return false;
		}

		//		if (cmp->output().appSignalID().isEmpty() == true)
		//		{
		//			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty output signal").arg(appFb->caption()).arg(appFb->schemaID());
		//			LOG_INTERNAL_ERROR_MSG(m_log, strError);
		//			qDebug() << strError;
		//			return false;
		//		}

		if (isConstComparator == false && cmp->compare().appSignalID().isEmpty() == true)
		{
			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty cmp signal").arg(appFb->caption()).arg(appFb->schemaID());
			LOG_INTERNAL_ERROR_MSG(m_log, strError);
			qDebug() << strError;
			return false;
		}

		if (hysteresisIsConst == false && cmp->hysteresis().appSignalID().isEmpty() == true)
		{
			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty hysteresis signal").arg(appFb->caption()).arg(appFb->schemaID());
			LOG_INTERNAL_ERROR_MSG(m_log, strError);
			qDebug() << strError;
			return false;
		}

		if (cmp->cmpType() == E::CmpType::Equal || cmp->cmpType() == E::CmpType::NotEqual)
		{
			if (cmp->inAnalogSignalFormat() == E::AnalogAppSignalFormat::Float32)
			{
				if (cmp->hysteresisIsConstSignal() == true && cmp->hysteresis().constValue() == 0.0)
				{
					log()->wrnALC5177(appFb->caption(), cmp->hysteresisPinCaption(), appFb->guid(), appFb->schemaID());
				}
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredAnalogOptoSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_acquiredAnalogOptoSignals.isEmpty() == true)
		{
			return true;
		}

		// m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredAnalogOptoSignalsToRegBuf);

		code->comment_nl("Copy acquired Analog opto signals in regBuf");

		bool result = true;

		for(UalSignal* ualSignal : m_acquiredAnalogOptoSignals)
		{
			TEST_PTR_LOG_RETURN_FALSE(code, m_log);

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

			if (ualSignal->ualAddrIsValid() == false ||
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

			CodeItem cmd;

			cmd.mov32(ualSignal->regBufAddr(), ualSignal->ualAddr());
			cmd.setComment(QString("copy %1").arg(ualSignal->acquiredRefSignalsIDs().join(", ")));

			code->append(cmd);
		}

		code->newLine();

		//m_alpCode_calculate(&m_resourcesUsageInfo.copyAcquiredAnalogOptoSignalsToRegBuf);

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredAnalogBusChildSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_acquiredAnalogBusChildSignals.isEmpty() == true)
		{
			return true;
		}

		code->comment_nl("Copy acquired analog bus child signals to reg buf");

		bool result = true;

		for(UalSignal* ualSignal : m_acquiredAnalogBusChildSignals)
		{
			TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

			if (ualSignal->ualAddrIsValid() == false)
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

			CodeItem cmd;

			cmd.mov32(ualSignal->regBufAddr(), ualSignal->ualAddr());
			cmd.setComment(QString("copy %1").arg(ualSignal->refSignalIDsJoined()));

			code->append(cmd);
		}

		code->newLine();

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredTuningAnalogSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_acquiredAnalogTuningSignals.isEmpty() == true)
		{
			return true;
		}

		// m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredTuningAnalogSignalsToRegBuf);

		code->comment_nl("Copy acquired tunable analog signals to regBuf");

		int startUalAddr = -1;
		int startRegBufAddr = -1;

		int prevUalAddr = -1;
		int prevRegBufAddr = -1;
		int prevSignalSizeW = -1;

		CodeItem cmd;

		QString commentStr;

		for(UalSignal* ualSignal : m_acquiredAnalogTuningSignals)
		{
			if(ualSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				return false;
			}

			AppSignal* s = ualSignal->getTunableSignal();

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
				assert(s->ualAddrIsValid() == true);
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
					code->append(cmd);

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
		code->append(cmd);

		code->newLine();

		//m_alpCode_calculate(&m_resourcesUsageInfo.copyAcquiredTuningAnalogSignalsToRegBuf);

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredTuningDiscreteSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_acquiredDiscreteTuningSignals.isEmpty() == true)
		{
			return true;
		}

		//m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredTuningDiscreteSignalsToRegBuf);

		code->comment_nl("Copy acquired tunable discrete signals to regBuf");

		int startUalAddr = -1;
		int startRegBufAddr = -1;

		int prevUalAddr = -1;
		int prevRegBufAddr = -1;

		CodeItem cmd;

		QString commentStr;

		for(UalSignal* ualAddr : m_acquiredDiscreteTuningSignals)
		{
			TEST_PTR_CONTINUE(ualAddr);

			AppSignal* s = ualAddr->getTunableSignal();

			TEST_PTR_CONTINUE(s);

			// check signal!

			assert(s->ualAddrIsValid() == true);
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
					code->append(cmd);

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
		code->append(cmd);

		code->newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredAnalogConstSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_acquiredAnalogConstIntSignals.isEmpty() == true &&
			m_acquiredAnalogConstFloatSignals.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		code->comment_nl("Writing of acquired analog const signals values in reg buf");

		QVector<int> sortedIntConsts = QVector<int>::fromList(m_acquiredAnalogConstIntSignals.uniqueKeys());

		if (sortedIntConsts.isEmpty() == false)
		{
			std::sort(sortedIntConsts.begin(), sortedIntConsts.end());

			for(int intConst : sortedIntConsts)
			{
				QList<UalSignal*> constIntSignals = m_acquiredAnalogConstIntSignals.values(intConst);
				QStringList constIntSignalsIDs;

				Address16 regBufAddr;

				for(UalSignal* constIntSignal : constIntSignals)
				{
					TEST_PTR_LOG_RETURN_FALSE(constIntSignal, m_log);

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

				CodeItem cmd;

				cmd.movConstInt32(regBufAddr.offset(), intConst);
				cmd.setComment(QString("int const %1: %2").arg(intConst).arg(constIntSignalsIDs.join(", ")));

				code->append(cmd);
			}

			code->newLine();
		}

		//

		QVector<float> sortedFloatConsts = QVector<float>::fromList(m_acquiredAnalogConstFloatSignals.uniqueKeys());

		if (sortedFloatConsts.isEmpty() == false)
		{
			std::sort(sortedFloatConsts.begin(), sortedFloatConsts.end());

			for(float floatConst : sortedFloatConsts)
			{
				QList<UalSignal*> constFloatSignals = m_acquiredAnalogConstFloatSignals.values(floatConst);
				QStringList constFloatSignalsIDs;

				Address16 regBufAddr;

				for(UalSignal* constFloatSignal : constFloatSignals)
				{
					TEST_PTR_LOG_RETURN_FALSE(constFloatSignal, m_log);

					if (regBufAddr.isValid() == false)
					{
						// first iteration
						regBufAddr = constFloatSignal->regBufAddr();
					}

					if (regBufAddr.isValid() == false ||
						regBufAddr != constFloatSignal->regBufAddr())				// all const signals with same value must have same reg buf address
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					constFloatSignalsIDs.append(constFloatSignal->acquiredRefSignalsIDs());
				}

				CodeItem cmd;

				cmd.movConstFloat(regBufAddr.offset(), floatConst);
				cmd.setComment(QString("float const %1: %2").arg(floatConst).arg(constFloatSignalsIDs.join(", ")));

				code->append(cmd);
			}

			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredInputBusesInRegBuf(CodeSnippet* code)
	{
		return copyBusesToRegBuf("Copy aquired Input Buses to RegBuf", m_acquiredInputBuses, code);
	}

	bool ModuleLogicCompiler::copyAcquiredBusChildBusesInRegBuf(CodeSnippet* code)
	{
		return copyBusesToRegBuf("Copy aquired bus child Buses to RegBuf", m_acquiredBusChildBuses, code);
	}

	bool ModuleLogicCompiler::copyAcquiredOptoBusesInRegBuf(CodeSnippet* code)
	{
		return copyBusesToRegBuf("Copy aquired opto Buses to regBuf", m_acquiredOptoBuses, code);
	}

	bool ModuleLogicCompiler::copyBusesToRegBuf(const QString& comment, const QVector<UalSignal*>& buses, CodeSnippet* code)
	{
		TEST_PTR_RETURN_FALSE(code);

		if (buses.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		CodeItem cmd;

		if (comment.isEmpty() == false)
		{
			code->comment_nl(comment);
		}

		for(const UalSignal* bus : buses)
		{
			if (checkUalAndRegBufAddrs(bus) == false)
			{
				result = false;
				continue;
			}

			Q_ASSERT(bus->isBus() == true);

			result &= generateMemCopyCode(	bus->regBufAddr(),
											bus->ualAddr(),
											bus->sizeW(),
											QString("copy %1 to RegBuf").arg(bus->refSignalIDsJoined()),
											code);
		}

		code->newLine();

		return result;
	}

	bool ModuleLogicCompiler::checkUalAndRegBufAddrs(const UalSignal* ualSignal) const
	{
		TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

		bool result = true;

		if (ualSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
			result = false;
		}

		if (ualSignal->checkRegBufAddr() == false)
		{
			// Undefined RegBuf address of signal %1 (Logic schema %2).
			//
			m_log->errALC5184(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::checkUalAndIoBufAddrs(const UalSignal* ualSignal) const
	{
		TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

		bool result = true;

		if (ualSignal->checkUalAddr() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
			result = false;
		}

		if (ualSignal->checkIoBufAddr() == false)
		{
			// Undefined IoBuf address of signal %1 (Logic schema %2).
			m_log->errALC5185(ualSignal->appSignalID(), ualSignal->ualItemGuid(), ualSignal->ualItemSchemaID());
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteInputSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		//m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredDiscreteInputSignalsToRegBuf);

		bool result = copyScatteredDiscreteSignalsInRegBuf(code, m_acquiredDiscreteInputSignals, "acquired discrete input signals");

		//m_alpCode_calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteInputSignalsToRegBuf);

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteOptoSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		//m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf);

		bool result = copyScatteredDiscreteSignalsInRegBuf(code, m_acquiredDiscreteOptoSignals, "acquired Discrete opto signals");

		//m_alpCode_calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf);

		return result;
	}


	bool ModuleLogicCompiler::copyAcquiredDiscreteBusChildSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		//m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf);

		bool result = copyScatteredDiscreteSignalsInRegBuf(code, m_acquiredDiscreteBusChildSignals, "acquired discrete bus child signals");

		//m_alpCode_calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf);

		return result;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteOutputAndInternalSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		//m_alpCode_init(&m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf);

		if (m_acquiredDiscreteStrictOutputSignals.isEmpty() == false)
		{
			assert(m_memoryMap.acquiredDiscreteOutputSignalsSizeW() ==
				   m_memoryMap.acquiredDiscreteOutputSignalsInRegBufSizeW());

			code->comment_nl("Copy acquired discrete output signals from bit-addressed memory to regBuf");

			CodeItem cmd;

			cmd.movMem(m_memoryMap.acquiredDiscreteOutputSignalsAddressInRegBuf(),
					   m_memoryMap.acquiredDiscreteOutputSignalsAddress(),
					   m_memoryMap.acquiredDiscreteOutputSignalsSizeW());

			code->append(cmd);
			code->newLine();
		}

		if (m_acquiredDiscreteInternalSignals.isEmpty() == false)
		{
			assert(m_memoryMap.acquiredDiscreteInternalSignalsSizeW() ==
				   m_memoryMap.acquiredDiscreteInternalSignalsInRegBufSizeW());

			code->comment_nl("Copy acquired discrete internal signals from bit-addressed memory to regBuf");

			CodeItem cmd;

			cmd.movMem(m_memoryMap.acquiredDiscreteInternalSignalsAddressInRegBuf(),
					   m_memoryMap.acquiredDiscreteInternalSignalsAddress(),
					   m_memoryMap.acquiredDiscreteInternalSignalsSizeW());

			code->append(cmd);
			code->newLine();
		}

		//calculate(&m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf);

		return true;
	}

	bool ModuleLogicCompiler::copyAcquiredDiscreteConstSignalsInRegBuf(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_memoryMap.acquiredDiscreteConstSignalsInRegBufSizeW() == 0)
		{
			return true;
		}

		assert(m_memoryMap.acquiredDiscreteConstSignalsInRegBufSizeW() == 1);			// if > 0 then always 1 word!

		QStringList const0Signals;
		QStringList const1Signals;

		for(UalSignal* ualSignal : m_acquiredDiscreteConstSignals)
		{
			TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

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

		code->comment("Copy acquired discrete const signals values:");

		code->comment(QString("const 0: %1").arg(const0Signals.join(", ")));
		code->comment(QString("const 1: %1").arg(const1Signals.join(", ")));

		code->newLine();

		CodeItem cmd;
		cmd.movConst(m_memoryMap.acquiredDiscreteConstSignalsAddressInRegBuf(), 2);
		cmd.setComment("bit 0 == 0, bit 1 == 1");

		code->append(cmd);
		code->newLine();

		return true;
	}

	bool ModuleLogicCompiler::copyScatteredDiscreteSignalsInRegBuf(CodeSnippet* code, const QVector<UalSignal*>& signalsList, const QString &description)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (signalsList.isEmpty() == true)
		{
			return true;
		}

		code->comment_nl(QString("Copy %1 in regBuf").arg(description));

		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		int signalsCount = signalsList.count();

		int count = 0;

		CodeItem cmd;

		int countReminder16 = 0;

		bool zeroLastWord = (signalsCount % SIZE_16BIT) != 0 ? true : false;

		for(UalSignal* ualSignal : signalsList)
		{
			TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

			assert(ualSignal->isConst() == false);

			if (ualSignal->ualAddrIsValid() == false ||
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
				code->append(cmd);
				zeroLastWord = false;
			}

			cmd.movBit(bitAccAddr, ualSignal->regBufAddr().bit(), ualSignal->ualAddr().offset(), ualSignal->ualAddr().bit());
			cmd.setComment(QString("copy %1").arg(ualSignal->refSignalIDsJoined()));
			code->append(cmd);

			count++;

			if ((count % SIZE_16BIT) == 0 || count == signalsCount)
			{
				cmd.clearComment();
				cmd.mov(ualSignal->regBufAddr().offset(), bitAccAddr);
				code->append(cmd);
				code->newLine();;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOutputSignalsInOutputModulesMemory(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		//m_alpCode_init(&m_resourcesUsageInfo.copyOutputSignalsInOutputModulesMemory);

		bool result = true;

		result &= convertAndCopyOutputAnalogSignals(code);
		result &= copyOutputBusSignals(code);
		result &= copyOutputDiscreteSignals(code);

		//m_alpCode_calculate(&m_resourcesUsageInfo.copyOutputSignalsInOutputModulesMemory);

		return result;
	}

	bool ModuleLogicCompiler::initOutputModulesMemory(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

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

		Hardware::DeviceAppSignal* lmOut1 = lmOut1Object->toAppSignal().get();
		Hardware::DeviceAppSignal* lmOut6 = lmOut6Object->toAppSignal().get();

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

		code->comment_nl("Init LM's output signals memory");

		CodeItem cmd;

		cmd.movConst(lmOutsMemoryOffset, 0);
		code->append(cmd);

		code->newLine();

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
				code->comment_nl("Init output modules memory");

				firstModule = false;
			}

			cmd.setMem(module.moduleDataOffset, 0, module.rxDataSize);
			cmd.setComment(QString("place %1 module %2").arg(module.place).arg(getModuleFamilyTypeStr(module.familyType())));
			code->append(cmd);
		}

		if (firstModule == false)
		{
			code->newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::convertAndCopyOutputAnalogSignals(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (m_analogOutputSignalsToConversion.isEmpty() == true)
		{
			return true;
		}

		code->comment_nl("Copy output analog signals to output modules memory");

		CodeItem cmd;

		for(AppSignal* s : m_analogOutputSignalsToConversion)
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
				assert(s->ualAddrIsValid() == true);
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
						cmd.setComment(QString("output analog %1 set to const %2").arg(s->appSignalID()).arg(constFloatValue));
					}
					else
					{
						cmd.movConstInt32(s->ioBufAddr().offset(), constIntValue);
						cmd.setComment(QString("output analog %1 set to const %2").arg(s->appSignalID()).arg(constIntValue));
					}
				}
				else
				{
					cmd.mov32(s->ioBufAddr().offset(), s->ualAddr().offset());
					cmd.setComment(QString("copy output analog %1").arg(s->appSignalID()));
				}

				code->append(cmd);
				code->newLine();

				continue;
			}

			UalAfb* appFb = m_inOutSignalsToScalAppFbMap.value(s->appSignalID(), nullptr);

			TEST_PTR_CONTINUE(appFb);

			bool isTconvAfb = appFb->caption().startsWith("tconv_");

			FbScal fbScal;

			switch(s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:

				fbScal = m_fbScal[FB_SCALE_FP_16UI_INDEX];
				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				if (isTconvAfb == true)
				{
					fbScal = m_fbScal[FB_TCONV_SI_FP_INDEX];
				}
				else
				{
					fbScal = m_fbScal[FB_SCALE_SI_16UI_INDEX];
				}
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
					cmd.setComment(QString(tr("float const %1 to output analog %2 conversion")).
										arg(constFloatValue).arg(s->appSignalID()));
				}
				else
				{
					cmd.writeFuncBlockConstInt32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
													constIntValue, appFb->caption());
					cmd.setComment(QString(tr("int const %1 to output analog %2 conversion")).
										arg(constIntValue).arg(s->appSignalID()));
				}

				code->append(cmd);
			}
			else
			{
				cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
								   s->ualAddr().offset(), appFb->caption());
				cmd.setComment(QString(tr("output analog %1 conversion")).arg(s->appSignalID()));
				code->append(cmd);
			}

			cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.clearComment();
			code->append(cmd);

			if (isTconvAfb == true)
			{
				cmd.readFuncBlock32(s->ioBufAddr().offset(),
								  appFb->opcode(), appFb->instance(),
								  fbScal.outputSignalIndex, appFb->caption());
			}
			else
			{
				cmd.readFuncBlock(s->ioBufAddr().offset(),
								  appFb->opcode(), appFb->instance(),
								  fbScal.outputSignalIndex, appFb->caption());
			}
			code->append(cmd);
			code->newLine();
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOutputBusSignals(CodeSnippet* code)
	{
		bool result = true;

		bool first = true;

		CodeItem cmd;

		for(AppSignal* s : m_ioSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isBus() == false || s->isOutput() == false)
			{
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(s->appSignalID());

			if (ualSignal == nullptr)
			{
				continue;			// it is Ok, output signal isn't used in app logic
			}

			if (checkUalAndIoBufAddrs(ualSignal) == false)
			{
				result = false;
				continue;
			}

			QString busTypeID = s->busTypeID();

			BusShared bus = m_signals->getBus(busTypeID);

			TEST_PTR_CONTINUE(bus);

			int busSizeW = bus->sizeW();

			if (first == true)
			{
				code->comment_nl("Copy Output Buses to output modules memory");
				first = false;
			}

			result &= generateMemCopyCode(ualSignal->ioBufAddr(), ualSignal->ualAddr(), busSizeW,
											QString("copy %1").arg(ualSignal->refSignalIDsJoined()), code);
		}

		return true;
	}

	bool ModuleLogicCompiler::copyOutputDiscreteSignals(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		bool result = true;

		QMultiHash<int, AppSignal*> writeAddressesMap;

		for(AppSignal* s : m_ioSignals)
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
				writeAddressesMap.insert(s->ioBufAddr().offset(), s);
			}
		}

		int lmOutputsAddress = m_lmDescription->memory().m_appDataOffset;
		bool lmOutputsIsWritten = false;

		code->comment_nl("Copy output discrete signals to output modules memory");

		QList<int> writeAddreses = writeAddressesMap.uniqueKeys();

		QVector<int> sortedWriteAddress = QVector<int>::fromList(writeAddreses);

		std::sort(sortedWriteAddress.begin(), sortedWriteAddress.end());

		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		for(int writeAddr : sortedWriteAddress)
		{
			QList<AppSignal*> writeSignals = writeAddressesMap.values(writeAddr);

			// signals sorting by ioBufAddr  via std::map
			//
			std::map<int, AppSignal*> sortedWriteSignals;

			for(AppSignal* s : writeSignals)
			{
				sortedWriteSignals.insert(std::make_pair(s->ioBufAddr().bitAddress(), s));
			}

			CodeItem cmd;

			cmd.movConst(bitAccAddr, 0);
			code->append(cmd);

			for(const std::pair<int, AppSignal*> pair: sortedWriteSignals)
			{
				AppSignal* s = pair.second;

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
					cmd.movBitConst(bitAccAddr, s->ioBufAddr().bit(), ualSignal->constDiscreteValue());
				}
				else
				{
					if (s->ualAddrIsValid() == false)
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						result = false;
						continue;
					}

					cmd.movBit(bitAccAddr, s->ioBufAddr().bit(), s->ualAddr().offset(), s->ualAddr().bit());
				}

				cmd.setComment(s->appSignalID());

				code->append(cmd);
			}

			cmd.mov(writeAddr, bitAccAddr);
			cmd.clearComment();
			code->append(cmd);
			code->newLine();

			if (writeAddr == lmOutputsAddress)
			{
				lmOutputsIsWritten = true;
			}
		}

		if (lmOutputsIsWritten == false)
		{
			CodeItem cmd;

			cmd.movConst(lmOutputsAddress, 0);
			cmd.setComment("write #0 to LM's outputs area");
			code->append(cmd);
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoConnectionsTxData(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		bool result = true;

		QVector<Hardware::OptoModuleShared> modules;

		m_optoModuleStorage->getOptoModulesSorted(modules);

		if (modules.count() == 0)
		{
			return true;
		}

		// m_alpCode_init(&m_resourcesUsageInfo.copyOptoConnectionsTxData);

		for(Hardware::OptoModuleShared& module : modules)
		{
			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (module->lmID() != lmEquipmentID())
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
					code->comment_nl(QString(tr("Copying txData of opto-module %1")).arg(module->equipmentID()));

					initialCommentPrinted = true;
				}

				result &= copyOptoPortTxData(code, port);
			}
		}

		// m_alpCode_calculate(&m_resourcesUsageInfo.copyOptoConnectionsTxData);

		return result;
	}


	bool ModuleLogicCompiler::copyOptoPortTxData(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

		if (port->txDataSizeW() == 0)
		{
			return true;
		}

		bool result = true;

		code->comment_nl(QString(tr("Copying txData of opto-port %1 (%2 words)")).
						  arg(port->equipmentID()).arg(port->txDataSizeW()));

		CodeItem cmd;

		// write data port txData identifier
		//
		cmd.movConstUInt32(port->txBufAbsAddress(), port->txDataID());
		cmd.setComment("txData ID");

		code->append(cmd);
		code->newLine();

		result &= copyOptoPortTxRawData(code, port);

		result &= copyOptoPortTxAnalogSignals(code, port);

		result &= copyOptoPortTxBusSignals(code, port);

		result &= copyOptoPortTxDiscreteSignals(code, port);

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxRawData(CodeSnippet* code,  Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

		if (port->txRawDataSizeW() == 0)
		{
			return true;
		}

		bool result = true;

		code->comment_nl(QString(tr("Copying raw data (%1 words) of opto-port %2")).arg(port->txRawDataSizeW()).arg(port->equipmentID()));

		int rawDataOffset = Hardware::OptoPort::TX_DATA_ID_SIZE_W;		// txDataID

		int txRawDataStartAddr = port->txBufAbsAddress() + rawDataOffset;
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
				result &= copyOptoPortAllNativeRawData(code, port, &rawDataOffset);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxModuleRawData:
				result &= copyOptoPortTxModuleOnPlaceRawData(code, port, &rawDataOffset, item.modulePlace);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxPortRawData:
				result &= copyOptoPortTxOptoPortRawData(code, port, &rawDataOffset, item.portEquipmentID);
				break;

			case Hardware::RawDataDescriptionItem::Type::TxConst16:
				result &= copyOptoPortTxConst16RawData(code, port, &rawDataOffset, item.const16Value);
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

		result &= copyOptoPortRawTxAnalogSignals(code, port);

		result &= copyOptoPortRawTxDiscreteSignals(code, port);

		result &= copyOptoPortRawTxBusSignals(code, port);

/*		MemWriteMap::AreaList nonWrittenAreas;

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
		}*/

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxAnalogSignals(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

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

			if (ualSignal->checkUalAddr() == false)
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
				code->comment_nl(QString("Copying regular tx analog signals of opto-port %1").arg(port->equipmentID()));

				first = false;
			}

			CodeItem cmd;

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
					cmd.movConstInt32(txSignalAddress.offset(), ualSignal->constAnalogIntValue());
					cmd.setComment(QString("%1 (const %2) >> %3").arg(ualSignal->refSignalIDsJoined()).
								   arg(ualSignal->constAnalogIntValue()).arg(port->connectionID()));
					break;
				default:
					assert(false);
				}
			}

			code->append(cmd);
		}

		if (first == false)
		{
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxBusSignals(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

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

			if (ualSignal->ualAddrIsValid() == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (first == true)
			{
				code->comment_nl(QString("Copying tx bus signals of opto-port %1").arg(port->equipmentID()));

				first = false;
			}

			CodeItem cmd;

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
			code->append(cmd);
		}

		if (first == false)
		{
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxDiscreteSignals(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

		// copy discrete signals
		//
		QVector<Hardware::TxRxSignalShared> txDiscreteSignals;

		port->getTxDiscreteSignals(txDiscreteSignals, true);

		int count = txDiscreteSignals.count();

		int wordCount = count / WORD_SIZE + ((count % WORD_SIZE) ? 1 : 0);

		int bitAccumulatorAddress = m_memoryMap.bitAccumulatorAddress();

		bool result = true;

		CodeItem cmd;

		int bitCount = 0;

		bool first = true;

		CodeSnippet copyCode;
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

			if (ualSignal->checkUalAddr() == false)
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
				code->comment_nl(QString("Copying regular tx discrete signals of opto-port %1").arg(port->equipmentID()));

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

					code->append(cmd);
				}
				else
				{
					code->append(copyCode);

					// copy bit accumulator to opto interface buffer
					//
					cmd.mov(txSignalAddress, bitAccumulatorAddress);
					cmd.clearComment();

					code->append(cmd);
				}

				copyCode.clear();
				ids.clear();
			}

			bitCount++;
		}

		if (first == false)
		{
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::isCopyOptimizationAllowed(const CodeSnippet& copyCode, int* srcAddr)
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
			const CodeItem& cmd = copyCode[i];

			if (cmd.getOpcode() != LmCommand::Code::MOVB)
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

	bool ModuleLogicCompiler::copyOptoPortAllNativeRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);
		TEST_PTR_LOG_RETURN_FALSE(rawDataOffset, m_log);

		bool result = true;

		for(int place = 1; place <= static_cast<int>(m_lmDescription->memory().m_moduleCount); place++)
		{
			const Hardware::DeviceModule* module = DeviceHelper::getModuleOnPlace(m_lm, place);

			if (module == nullptr)
			{
				continue;
			}

			result &= copyOptoPortTxModuleRawData(code, port, rawDataOffset, module);
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortTxModuleOnPlaceRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, int place)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);
		TEST_PTR_LOG_RETURN_FALSE(rawDataOffset, m_log);

		const Hardware::DeviceModule* module = DeviceHelper::getModuleOnPlace(m_lm, place);

		if (module == nullptr)
		{
			QString msg = QString("OptoPort %1 raw data copying, not found module on place %2.").
					arg(port->equipmentID()).arg(place);
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
			return false;
		}

		return copyOptoPortTxModuleRawData(code, port, rawDataOffset, module);
	}


	bool ModuleLogicCompiler::copyOptoPortTxModuleRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, const Hardware::DeviceModule* module)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);
		TEST_PTR_LOG_RETURN_FALSE(rawDataOffset, m_log);
		TEST_PTR_LOG_RETURN_FALSE(module, m_log);

		int moduleRawDataSize = m_optoModuleStorage->getModuleRawDataSize(module, m_log);

		if (moduleRawDataSize == 0)
		{
			return true;
		}

		ModuleRawDataDescription* desc = m_optoModuleStorage->getModuleRawDataDescription(module);

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
			CodeItem cmd;

			toAddr = port->txBufAbsAddress() + *rawDataOffset + localOffset;

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

				code->append(cmd);
			}
		}

		code->newLine();

		if (autoSize == true)
		{
			assert(localOffset == moduleRawDataSize);
		}

		*rawDataOffset += moduleRawDataSize;

		return true;
	}

	bool ModuleLogicCompiler::copyOptoPortTxOptoPortRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, const QString& portEquipmentID)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);
		TEST_PTR_LOG_RETURN_FALSE(rawDataOffset, m_log);

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

		CodeItem cmd;

		int writeAddr = port->txBufAbsAddress() + *rawDataOffset;
		int writeSizeW = portTxRawDataSizeW;

		cmd.movMem(writeAddr, portWithRxRawData->rxBufAbsAddress() + Hardware::OptoPort::TX_DATA_ID_SIZE_W, writeSizeW);
		cmd.setComment(QString("copying raw data received on port %1").arg(portWithRxRawData->equipmentID()));

		code->append(cmd);
		code->newLine();

		*rawDataOffset += portTxRawDataSizeW;

		return true;
	}

	bool ModuleLogicCompiler::copyOptoPortTxConst16RawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, int const16value)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);
		TEST_PTR_LOG_RETURN_FALSE(rawDataOffset, m_log);

		CodeItem cmd;

		int writeAddr = port->txBufAbsAddress() + *rawDataOffset;

		cmd.movConst(writeAddr, const16value);

		cmd.setComment(QString("copying raw data const16 value = %1").arg(const16value));

		code->append(cmd);
		code->newLine();

		(*rawDataOffset)++;

		return true;

	}

	bool ModuleLogicCompiler::copyOptoPortRawTxAnalogSignals(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

		const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		bool result = true;

		CodeItem cmd;

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

			if (ualSignal->checkUalAddr() == false)
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

			code->append(cmd);

			count++;
		}

		if (count > 0)
		{
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortRawTxDiscreteSignals(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

		const QVector<Hardware::TxRxSignalShared>& txSignals = port->txSignals();

		int count = 0;

		QMultiHash<int, Hardware::TxRxSignalShared> txDiscretes;

		for(const Hardware::TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal->isRaw() == false || txSignal->isDiscrete() == false)
			{
				// skip non-Raw and non-Discrete signals
				//
				continue;
			}

			txDiscretes.insert(txSignal->addrInBuf().offset(), txSignal);

			count++;
		}

		if (count == 0)
		{
			return true;
		}

		QVector<int> offsets(QVector<int>::fromList(txDiscretes.uniqueKeys()));

		std::sort(offsets.begin(), offsets.end());

		int bitAccAddr = m_memoryMap.bitAccumulatorAddress();

		bool result = true;

		CodeItem cmd;

		for(int offset : offsets)
		{
			cmd.movConst(bitAccAddr, 0);
			code->append(cmd);

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

				if (ualSignal->checkUalAddr() == false)
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

				code->append(cmd);

				count++;
			}

			if (count > 0)
			{
				int writeAddr = port->txBufAbsAddress() + offset;

				cmd.mov(writeAddr, bitAccAddr);
				cmd.clearComment();

				code->append(cmd);
			}
		}

		code->newLine();

		return result;
	}

	bool ModuleLogicCompiler::copyOptoPortRawTxBusSignals(CodeSnippet* code, Hardware::OptoPortShared port)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(port, m_log);

		QVector<Hardware::TxRxSignalShared> txSignals;

		port->getTxSignals(txSignals);

		bool result = true;

		CodeItem cmd;

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
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Raw tx UalSignal %1 bus description is undefined (Opto port %2).").
										arg(txSignal->appSignalID()).arg(port->equipmentID()));
				result = false;
				continue;
			}

			if (ualSignal->ualAddrIsValid() == false)
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
			code->append(cmd);

			count++;
		}

		if (count > 0)
		{
			code->newLine();
		}

		return result;
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
		                                    EquipmentPropNames::LM_APP_LAN_DATA_SIZE,
											m_memoryMap.regBufSizeW(),
											m_log);
	}

	bool ModuleLogicCompiler::detectUnusedSignals()
	{
		QStringList unusedSignals;

		for(const AppSignal* s : m_chassisSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isInternal() == true && m_ualSignals.contains(s->appSignalID()) == false)
			{
				unusedSignals.append(s->appSignalID());
			}
		}

		unusedSignals.sort();

		for(const QString& unusedSignal : unusedSignals)
		{
			// Internal signal %1 is unused.
			//
			m_log->wrnALC5148(unusedSignal);
		}

		return true;
	}

	bool ModuleLogicCompiler::fillAnalogSignalsOnSchemas()
	{
		for(const UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSignal() == false)
			{
				continue;
			}

			QString appSignalID = ualItem->strID();

			AppSignal* s = m_signals->getSignal(appSignalID);

			TEST_PTR_CONTINUE(s);					// this error should be detected early

			if (s->isAnalog() == true)
			{
				m_context->m_analogSignalsOnSchemas.insert(appSignalID);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::calculateCodeRunTime()
	{
		bool result = m_code.getRunTimes(&m_idrPhaseClockCount, &m_alpPhaseClockCount);

		if (result == false)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Code runtime calculcation error!")))
		}

		return result;
	}

	QString ModuleLogicCompiler::lmSubsystemEquipmentIdPath() const
	{
		return QString("%1/%2").arg(m_lmSubsystemID).arg(lmEquipmentID());
	}

	bool ModuleLogicCompiler::writeResult()
	{
		bool result = true;

		QByteArray binCode;

		m_code.getBinCode(&binCode);

		result &= calcAppLogicUniqueID(binCode);

		RETURN_IF_FALSE(result);

		if (m_lmDescription->flashMemory().m_appLogicWriteBitstream == true)
		{
			result &= writeBinCodeForLm();

			RETURN_IF_FALSE(result);
		}

		if (m_context->generateExtraDebugInfo() == true)
		{
			BuildFile* binFile = m_resultWriter->addFile(Directory::BIN, QString("%1.bin").arg(lmEquipmentID()), "", "", binCode);

			if (binFile == nullptr)
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

		m_code.getAsmCode(&asmCode);

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

		result &= writeTuningInfoFile();

		result &= writeOcmRsSignalsXml();

		result &= writeLooopbacksReport();

		result &= writeHeapsLog();

		//

		return result;
	}

	bool ModuleLogicCompiler::writeBinCodeForLm()
	{
		bool result = true;

		int metadataFieldsVersion = 0;

		QStringList metadataFields;

		m_code.getAsmMetadataFields(&metadataFields, &metadataFieldsVersion);

		Hardware::ModuleFirmwareWriter* firmwareWriter = m_resultWriter->firmwareWriter();

		if (firmwareWriter == nullptr)
		{
			assert(firmwareWriter);
			return false;
		}

		int appLogicUartID = m_lmDescription->flashMemory().m_appLogicUartId;

		firmwareWriter->createFirmware(m_lmSubsystemID,
									   m_lmSubsystemKey,
									   appLogicUartID,
									   "AppLogic",
									   m_lmAppLogicFramePayload,
									   m_lmAppLogicFrameCount,
									   LmDescription::lmDescriptionFile(m_lm),
									   lmDescriptionNumber());

		firmwareWriter->setDescriptionFields(m_lmSubsystemID, appLogicUartID, metadataFieldsVersion, metadataFields);

		QByteArray binCode;

		m_code.getBinCode(&binCode);

		std::vector<QVariantList> metadata;

		m_code.getAsmMetadata(&metadata);

		result &= firmwareWriter->setChannelData(m_lmSubsystemID,
												 appLogicUartID,
												 lmEquipmentID(),
												 m_lmNumber,
												 m_lmAppLogicFramePayload,
												 m_lmAppLogicFrameCount,
												 m_appLogicUniqueID,
												 binCode,
												 metadata,
												 log());

		return result;
	}


	bool ModuleLogicCompiler::calcAppLogicUniqueID(const QByteArray& lmAppCode)
	{
		QVector<UalSignal*> acquiredSignals;

		acquiredSignals.append(m_acquiredDiscreteInputSignals);
		acquiredSignals.append(m_acquiredDiscreteStrictOutputSignals);
		acquiredSignals.append(m_acquiredDiscreteInternalSignals);
		acquiredSignals.append(m_acquiredDiscreteTuningSignals);
		acquiredSignals.append(m_acquiredDiscreteConstSignals);
		acquiredSignals.append(m_acquiredDiscreteOptoSignals);
		acquiredSignals.append(m_acquiredDiscreteBusChildSignals);
		acquiredSignals.append(m_acquiredAnalogInputSignals);
		acquiredSignals.append(m_acquiredAnalogStrictOutputSignals);
		acquiredSignals.append(m_acquiredAnalogInternalSignals);
		acquiredSignals.append(m_acquiredAnalogOptoSignals);
		acquiredSignals.append(m_acquiredAnalogBusChildSignals);
		acquiredSignals.append(m_acquiredAnalogTuningSignals);
		acquiredSignals.append(m_acquiredInputBuses);
		acquiredSignals.append(m_acquiredOutputBuses);
		acquiredSignals.append(m_acquiredInternalBuses);
		acquiredSignals.append(m_acquiredBusChildBuses);
		acquiredSignals.append(m_acquiredOptoBuses);

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

		m_appLogicUniqueID = crc.result();

		return DeviceHelper::setUIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
		                                    EquipmentPropNames::LM_APP_LAN_DATA_UID,
											crc.result32(),
											m_log);
	}

	bool ModuleLogicCompiler::writeTuningInfoFile()
	{
		if (m_tuningData == nullptr)
		{
			return true;
		}

		QStringList file;
		QString line = QString("----------------------------------------------------------------------------------------------------------");

		file.append(QString("Tuning information file: %1\n").arg(lmEquipmentID()));
		file.append(QString("LM eqipmentID: %1").arg(lmEquipmentID()));
		file.append(QString("LM caption: %1").arg(m_lm->caption()));
		file.append(QString("LM number: %1\n").arg(m_lmNumber));
		file.append(QString("Frames used total: %1").arg(m_tuningData->usedFramesCount()));

		QString s;

		quint64 uniqueID = m_tuningData->uniqueID();

		file.append(QString("Unique data ID: %1 (0x%2)").arg(uniqueID).arg(uniqueID, 16, 16, Latin1Char::ZERO));

		const QVector<AppSignal*>& analogFloatSignals = m_tuningData->getAnalogFloatSignals();

		if (analogFloatSignals.count() > 0)
		{
			file.append(QString("\nAnalog signals, type Float (32 bits)"));
			file.append(line);
			file.append(QString("Address\t\tOffset\t\tAppSignalID\t\t\t\t\t\tDefault\t\tLow Limit\tHigh Limit"));
			file.append(line);

			for(AppSignal* signal : analogFloatSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				file.append(QString("%1:%2\t%3:%4\t%5\t%6\t%7\t%8").
							arg(signal->tuningAbsAddr().offset(), 5, 10, Latin1Char::ZERO).
							arg(signal->tuningAbsAddr().bit(), 2, 10, Latin1Char::ZERO).
							arg(signal->tuningAbsAddr().offset() - m_tuningData->tuningDataOffsetW(), 5, 10, Latin1Char::ZERO).
							arg(signal->tuningAbsAddr().bit(), 2, 10, Latin1Char::ZERO).
							arg(signal->appSignalID(), -48, Latin1Char::SPACE).
							arg(signal->tuningDefaultValue().toFloat()).
							arg(signal->tuningLowBound().floatValue()).
							arg(signal->tuningHighBound().floatValue()));
			}
		}

		const QVector<AppSignal*>& analogIntSignals = m_tuningData->getAnalogIntSignals();

		if (analogIntSignals.count() > 0)
		{
			file.append(QString("\nAnalog signals, type Signed Integer (32 bits)"));
			file.append(line);
			file.append(QString("Address\t\tOffset\t\tAppSignalID\t\t\t\t\t\tDefault\t\tLow Limit\tHigh Limit"));
			file.append(line);

			for(AppSignal* signal : analogIntSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				file.append(QString("%1:%2\t%3:%4\t%5\t%6\t%7\t\t%8").
								arg(signal->tuningAbsAddr().offset(), 5, 10, Latin1Char::ZERO).
								arg(signal->tuningAbsAddr().bit(), 2, 10, Latin1Char::ZERO).
								arg(signal->tuningAbsAddr().offset() - m_tuningData->tuningDataOffsetW(), 5, 10, Latin1Char::ZERO).
								arg(signal->tuningAbsAddr().bit(), 2, 10, Latin1Char::ZERO).
								arg(signal->appSignalID(), -48, Latin1Char::SPACE).
								arg(signal->tuningDefaultValue().int32Value()).
								arg(signal->tuningLowBound().int32Value()).
								arg(signal->tuningHighBound().int32Value()));
			}
		}

		QVector<AppSignal*> discreteSignals = m_tuningData->getDiscreteSignals();

		if (discreteSignals.count() > 0)
		{
			// sort signals by tuningAbsAddr ascending
			//
			for(int i = 0; i < discreteSignals.count() - 1; i++)
			{
				for(int k = i + 1; k < discreteSignals.count(); k++)
				{
					AppSignal* s1 = discreteSignals[i];
					AppSignal* s2 = discreteSignals[k];

					TEST_PTR_CONTINUE(s1);
					TEST_PTR_CONTINUE(s2);

					if (s1->tuningAbsAddr().bitAddress() > s2->tuningAbsAddr().bitAddress())
					{
						discreteSignals[i] = s2;
						discreteSignals[k] = s1;
					}
				}
			}

			file.append(QString("\nDiscrete signals (1 bit)"));
			file.append(line);
			file.append(QString("Address\t\tOffset\t\tAppSignalID\t\t\t\t\t\tDefault\t\tLow Limit\tHigh Limit"));
			file.append(line);

			for(AppSignal* signal : discreteSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				QString str;

				file.append(QString("%1:%2\t%3:%4\t%5\t%6\t%7\t%8").
								arg(signal->tuningAbsAddr().offset(), 5, 10, Latin1Char::ZERO).
								arg(signal->tuningAbsAddr().bit(), 2, 10, Latin1Char::ZERO).
								arg(signal->tuningAbsAddr().offset() - m_tuningData->tuningDataOffsetW(), 5, 10, Latin1Char::ZERO).
								arg(signal->tuningAbsAddr().bit(), 2, 10, Latin1Char::ZERO).
								arg(signal->appSignalID(), -48, Latin1Char::SPACE).
								arg(signal->tuningDefaultValue().discreteValue()).
								arg(0).
								arg(1));
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
		bool result = m_resultWriter->addFile(m_lmSubsystemID, QString("%1-%2.tun").
										 arg(m_lmSubsystemID.toLower()).arg(m_lmNumber), file);
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
		}

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

	bool ModuleLogicCompiler::writeLooopbacksReport()
	{
		if (m_context->generateExtraDebugInfo() == false)
		{
			return true;
		}

		QStringList file;

		m_loopbacks.writeReport(&file);

		BuildFile* bf = m_resultWriter->addFile(lmSubsystemEquipmentIdPath(), "Loopbacks.csv", file);

		return bf != nullptr;
	}

	bool ModuleLogicCompiler::writeHeapsLog()
	{
		if (m_context->generateExtraDebugInfo() == false)
		{
			return true;
		}

		QStringList file;

		m_ualSignals.getHeapsLog(&file);

		BuildFile* bf = m_resultWriter->addFile(lmSubsystemEquipmentIdPath(), "HeapsLog.txt", file);

		return bf != nullptr;
	}

	bool ModuleLogicCompiler::displayResourcesUsageInfo()
	{
		QString str;

		double percentOfUsedCodeMemory = (m_code.commandAddress() * 100.0) / m_lmCodeMemorySize;

		bool result = true;

		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("Used resources of %1:")).arg(lmEquipmentID()));

		str.setNum(percentOfUsedCodeMemory, 'g', 2);

		LOG_MESSAGE(m_log, QString(tr("Code memory - %1%")).arg(str));

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

		str.setNum(percentOfUsedBitMemory, 'g', 2);

		LOG_MESSAGE(m_log, QString(tr("Bit-addressed memory - %1%")).arg(str));

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

		str.setNum(percentOfUsedWordMemory, 'g', 2);

		LOG_MESSAGE(m_log, QString(tr("Word-addressed memory - %1%")).arg(str));

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

		str_percent.setNum(static_cast<float>(idrPhaseTimeUsed), 'g', 2);
		str.setNum(static_cast<float>(idrPhaseTime * 1000000), 'g', 2);

		LOG_MESSAGE(m_log, QString(tr("Input Data Receive phase time - %1% (%2 clocks or %3 &micro;s of %4 &micro;s)")).
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

		str_percent.setNum(static_cast<float>(alpPhaseTimeUsed), 'g', 2);
		str.setNum(static_cast<float>(alpPhaseTime * 1000000), 'g', 2);

		LOG_MESSAGE(m_log, QString(tr("Application Logic Processing phase time - %1% (%2 clocks or %3 &micro;s of %4 &micro;s)")).
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

		m_resourcesUsageInfo.lmEquipmentID = lmEquipmentID();
		m_resourcesUsageInfo.codeMemoryUsed = percentOfUsedCodeMemory;
		m_resourcesUsageInfo.bitMemoryUsed = percentOfUsedBitMemory;
		m_resourcesUsageInfo.wordMemoryUsed = percentOfUsedWordMemory;
		m_resourcesUsageInfo.idrPhaseTimeUsed = idrPhaseTimeUsed;
		m_resourcesUsageInfo.alpPhaseTimeUsed = alpPhaseTimeUsed;

		result &= getAfblUsageInfo();

		//

/*		LOG_MESSAGE(m_log, QString("Init AFBs code: %1").arg(m_resourcesUsageInfo.initAfbs.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Analog inputs conversion code: %1").arg(m_resourcesUsageInfo.convertAnalogInputSignals.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Application logic code: %1").arg(m_resourcesUsageInfo.appLogicCode.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired analog opto signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredAnalogOptoSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired tuning analog signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredTuningAnalogSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired tuning discrete signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredTuningDiscreteSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired discrete input signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredDiscreteInputSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired discrete output and internal signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Acquired discrete opto and bus child signals copying to RegBuf: %1").arg(m_resourcesUsageInfo.copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf.codePercentStr()));
		LOG_MESSAGE(m_log, QString("Output signals copying to output modules: %1").arg(m_resourcesUsageInfo.copyOutputSignalsInOutputModulesMemory.codePercentStr()));

		LOG_MESSAGE(m_log, QString("Opto connections tx data copying: %1").arg(m_resourcesUsageInfo.copyOptoConnectionsTxData.codePercentStr()));*/

/*		CodeFragmentMetrics copyAcquiredRawDataInRegBuf;
		CodeFragmentMetrics copyAcquiredAnalogBusChildSignalsToRegBuf;
		CodeFragmentMetrics copyAcquiredConstAnalogSignalsToRegBuf;
		CodeFragmentMetrics copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf;
		CodeFragmentMetrics copyAcquiredDiscreteConstSignalsToRegBuf;
		CodeFragmentMetrics copyOutputSignalsInOutputModulesMemory;*/

		LOG_EMPTY_LINE(m_log);

		return result;
	}

	void ModuleLogicCompiler::calcOptoDiscretesStatistics()
	{
		QList<Hardware::OptoPortShared> associatedPorts;

		m_optoModuleStorage->getLmAssociatedOptoPorts(lmEquipmentID(), associatedPorts);

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

			if (component->caption() == "SET_FLAGS")
			{
				continue;
			}

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

	bool ModuleLogicCompiler::checkLoopbackTargetSignalsCompatibility(const AppSignal& srcSignal, QUuid srcSignalUuid, const AppSignal& destSignal, QUuid destSignalUuid)
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

	bool ModuleLogicCompiler::checkLoopbackTargetSignalsCompatibility(const AppSignal& srcSignal, QUuid srcSignalUuid, const UalAfb& fb, const LogicAfbSignal& afbSignal)
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

	bool ModuleLogicCompiler::isUsedInUal(const AppSignal* s) const
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
		return E::valueToString<Hardware::DeviceModule::FamilyType>(familyType);
	}

	std::shared_ptr<Hardware::DeviceObject> ModuleLogicCompiler::getDeviceSharedPtr(const Hardware::DeviceObject* device)
	{
		TEST_PTR_LOG_RETURN_NULLPTR(device, m_log);

		return getDeviceSharedPtr(device->equipmentIdTemplate());
	}

	std::shared_ptr<Hardware::DeviceObject> ModuleLogicCompiler::getDeviceSharedPtr(const QString& deviceEquipmentID)
	{
		return m_equipmentSet->deviceObject(deviceEquipmentID);
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
		if (m_context->generateExtraDebugInfo() == false)
		{
			return true;
		}

		bool result = true;

		result &= writeSignalList(m_acquiredDiscreteInputSignals, "acquiredDiscreteInput");
		result &= writeSignalList(m_acquiredDiscreteStrictOutputSignals, "acquiredDiscreteStrictOutput");
		result &= writeSignalList(m_acquiredDiscreteInternalSignals, "acquiredDiscreteInternal");
		result &= writeSignalList(m_acquiredDiscreteTuningSignals, "acquiredDiscreteTuning");
		result &= writeSignalList(m_acquiredDiscreteConstSignals, "acquiredDiscreteConst");
		result &= writeSignalList(m_acquiredDiscreteOptoSignals, "acquiredDiscreteOpto");
		result &= writeSignalList(m_acquiredDiscreteBusChildSignals, "acquiredDiscreteBusChild");

		result &= writeSignalList(m_acquiredInputBuses, "acquiredInputBuses");
		result &= writeSignalList(m_acquiredOutputBuses, "acquiredOutputBuses");
		result &= writeSignalList(m_acquiredInternalBuses, "acquiredInternalBuses");
		result &= writeSignalList(m_acquiredBusChildBuses, "acquiredBusChildBuses");
		result &= writeSignalList(m_acquiredOptoBuses, "acquiredOptoBuses");

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

		result &= writeSignalList(m_nonAcquiredOutputBuses, "nonAcquiredOutputBuses");
		result &= writeSignalList(m_nonAcquiredInternalBuses, "nonAcquiredInternalBuses");

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
						   arg(ualSignal->ualAddrWithoutChecks().offset()).arg(ualSignal->ualAddrWithoutChecks().bit()).
						   arg(ualSignal->regBufAddr().offset()).arg(ualSignal->regBufAddr().bit()).
						   arg(ualSignal->regValueAddr().offset()).arg(ualSignal->regValueAddr().bit()));
		}

		m_resultWriter->addFile(QString("%1/%2").arg(m_lmSubsystemID).arg(lmEquipmentID()),
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
				// %1 has been finished with errors.
				//
				m_log->errALC5999(proc.second);
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::runCodeGenProcs(const CodeGenProcsToCallArray& procArray, CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		bool result = true;

		for(const CodeGenProcToCall& proc : procArray)
		{
			result &= (this->*proc.first)(code);

			if (result == false)
			{
				// %1 has been finished with errors.
				//
				m_log->errALC5999(proc.second);
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

	CodeItem ModuleLogicCompiler::codeSetMemory(int addrFrom, quint16 constValue, int sizeW, const QString& comment)
	{
		assert(addrFrom >=0 && addrFrom < static_cast<int>(m_lmDescription->memory().m_appMemorySize));
		assert(addrFrom + sizeW < static_cast<int>(m_lmDescription->memory().m_appMemorySize));

		CodeItem cmd;

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

		return cmd;
	}

	QString ModuleLogicCompiler::getFormatStr(const Hardware::DeviceAppSignal& ds)
	{
		return getFormatStr(ds.signalType(), ds.format(), ds.size(), ds.byteOrder());
	}

	QString ModuleLogicCompiler::getFormatStr(const AppSignal& s)
	{
		switch(s.signalType())
		{
		case E::SignalType::Discrete:
			 return getFormatStr(s.signalType(), E::DataFormat::UnsignedInt, DISCRETE_SIZE, s.byteOrder());

		case E::SignalType::Analog:
			{
				switch(s.analogSignalFormat())
				{
				case E::AnalogAppSignalFormat::Float32:
					return getFormatStr(s.signalType(), E::DataFormat::Float, s.dataSize(), s.byteOrder());

				case E::AnalogAppSignalFormat::SignedInt32:
					return getFormatStr(s.signalType(), E::DataFormat::SignedInt, s.dataSize(), s.byteOrder());

				default:
					assert(false);
				}
			}
			break;

		case E::SignalType::Bus:
			return getFormatStr(s.signalType(), E::DataFormat::UnsignedInt /* don't matter */, s.dataSize(), s.byteOrder()  /* don't matter */);

		default:
			assert(false);

		}

		return QString("???");
	}

	QString ModuleLogicCompiler::getFormatStr(E::SignalType signalType, E::DataFormat dataFormat, int dataSizeBits, E::ByteOrder byteOrder)
	{
		QString formatStr("???");
		QString be_le("??");

		switch(byteOrder)
		{
		case E::ByteOrder::BigEndian:
			be_le = "BE";
			break;

		case E::ByteOrder::LittleEndian:
			be_le = "LE";
			break;

		default:
			assert(false);
		}

		switch(signalType)
		{
		case E::SignalType::Discrete:
		case E::SignalType::Analog:
			formatStr = QString("%1 %2 bit(s) %3").arg(E::valueToString<E::DataFormat>(dataFormat)).arg(dataSizeBits).arg(be_le);
			break;

		case E::SignalType::Bus:
			formatStr = QString("Bus %1 bits").arg(dataSizeBits);
			break;

		default:
			assert(false);
		}

		return formatStr;
	}

	//
	// Finding partion of integer number as sum of avaliable (specified) integers parts
	//
	bool ModuleLogicCompiler::partitionOfInteger(int number, const std::vector<int>& availableParts,
												 std::vector<int>* resultPartition)
	{
		if (resultPartition == nullptr)
		{
			assert(false);
			return false;
		}

		resultPartition->clear();

		if (availableParts.size() == 0)
		{
			assert(false);
			return false;
		}

		// special case fast processing when parts count == 1
		//
		if (availableParts.size() == 1)
		{
			int part0 = availableParts[0];

			if (part0 == 0)
			{
				assert(false);		//
				return false;
			}

			if ((number % part0) == 0)
			{
				int n = number / part0;

				for(int i = 0; i < n; i++)
				{
					resultPartition->push_back(part0);
				}

				return true;
			}

			return false;
		}

		//

		std::vector<int> parts = availableParts;

		// sort available parts in DESCENDING order
		//
		std::sort(parts.begin(), parts.end());
		std::reverse(parts.begin(), parts.end());

		std::vector<std::pair<int, int>> tmp;

		int startPartIndex = 0;
		int curPartIndex = 0;
		int removeFromIndex = -1;

		int iterationsCounter = 0;

		do
		{
			do
			{
				iterationsCounter++;

				if (iterationsCounter >= 500)
				{
					assert(false);			// difficult solution or looping???
					return false;
				}

				tmp.push_back(std::pair<int, int>(parts[curPartIndex], curPartIndex));

				int tmpSum = 0;

				for(auto& pr : tmp)
				{
					tmpSum += pr.first;
				}

				if (tmpSum == number)
				{
					for(auto& p : tmp)
					{
						resultPartition->push_back(p.first);
					}

					return true;
				}

				if (tmpSum < number)
				{
					continue;
				}

				// here if tmpSum > number

				tmp.pop_back();			// remove last and try next part

				curPartIndex++;

				if (curPartIndex >= static_cast<int>(parts.size()))
				{
					curPartIndex--;
					break;
				}

				continue;
			}
			while(true);

			if (startPartIndex + 1 == static_cast<int>(parts.size()))
			{
				return false;
			}

			if (tmp.size() == 0)
			{
				startPartIndex++;

				if (startPartIndex >=  static_cast<int>(parts.size()))
				{
					return false;
				}

				curPartIndex = startPartIndex;
				removeFromIndex = -1;

				continue;
			}

			// here if tmp.size() > 0
			//
			if (removeFromIndex == -1)
			{
				removeFromIndex = static_cast<int>(tmp.size() - 1);
			}
			else
			{
				// here removeFromIndex can be == 0
				// after decrement it will be -1
				//
				removeFromIndex--;
			}

			if (removeFromIndex <= 0)
			{
				startPartIndex++;

				if (startPartIndex >=  static_cast<int>(parts.size()))
				{
					return false;
				}

				curPartIndex = startPartIndex;
				removeFromIndex = -1;

				tmp.clear();

				continue;
			}

			if (removeFromIndex > 0)
			{
				if (removeFromIndex >= static_cast<int>(tmp.size()))
				{
					assert(false);
					return false;
				}

				tmp.erase(tmp.begin() + removeFromIndex, tmp.end());

				if (tmp.size() != 0)
				{
					int lastItemPartIndex = tmp.back().second;

					if (lastItemPartIndex < static_cast<int>(parts.size() - 1))
					{
						curPartIndex = lastItemPartIndex + 1;
					}
					else
					{
						curPartIndex = lastItemPartIndex;
					}
				}
				else
				{
					assert(false);
				}
			}
			else
			{
				return false;
			}
		}
		while(true);

		return false;
	}

	bool ModuleLogicCompiler::partitionOfInteger(int number, const QVector<int>& availableParts, QVector<int>* partition)
	{
		if (partition == nullptr)
		{
			assert(false);
			return false;
		}

		std::vector<int> parts;

		for(int p : availableParts)
		{
			parts.push_back(p);
		}

		std::vector<int> resultPartition;

		bool result = partitionOfInteger(number, parts, &resultPartition);

		partition->clear();

		for(int p : resultPartition)
		{
			partition->append(p);
		}

		return result;
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
