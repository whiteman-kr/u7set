#include "../Builder/ModuleLogicCompiler.h"
#include "../Builder/AppLogicCompiler.h"
#include "../UtilsLib/Crc.h"

#include <DbLib/DbControllerTools.h>
#include <HardwareLib/Connection.h>
#include <HardwareLib/DeviceChassis.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/DeviceController.h>
#include <HardwareLib/DeviceAppSignal.h>

#include "LanControllerInfoHelper.h"
#include "DeviceHelper.h"
#include "SoftwareCfgGenerator.h"
#include "DiagDataServiceCfgGenerator.h"
#include "Parser.h"
#include "LmDescriptionSet.h"

#define LOG_UNDEFINED_UAL_ADDRESS(log, ualSignal) log->writeError(QString("Undefined signal's ualAddress: %1 (File: %2 Line: %3 Function: %4)").arg(ualSignal->refSignalIDs().join(", ")).arg(__FILE__).arg(__LINE__).arg(SHORT_FUNC_INFO));

namespace Builder
{
	// ---------------------------------------------------------------------------------
	//
	//	ModuleLogicCompiler::BusFilling class implementation
	//
	// ---------------------------------------------------------------------------------

	ModuleLogicCompiler::BusFilling::BusFilling(BusShared bus)
	{
		TEST_PTR_RETURN(bus);

		m_busArea.assign(bus->sizeW(), 0);
	}

	void ModuleLogicCompiler::BusFilling::fillWord(int offsetInBus)
	{
		fill(offsetInBus, 1);
	}

	void ModuleLogicCompiler::BusFilling::fillDword(int offsetInBus)
	{
		fill(offsetInBus, 2);
	}

	void ModuleLogicCompiler::BusFilling::fill(int offsetInBus, int sizeW)
	{
		int busSizeW = static_cast<int>(m_busArea.size());

		if (sizeW == 0)
		{
			Q_ASSERT(false);
			return;
		}

		if (offsetInBus < 0 ||
			(offsetInBus + sizeW) > busSizeW)
		{
			Q_ASSERT(false);
			return;
		}

		for(int i = offsetInBus; i < offsetInBus + sizeW; i++)
		{
			Q_ASSERT(m_busArea[i] == 0);

			m_busArea[i] = 0xFFFF;
		}
	}

	void ModuleLogicCompiler::BusFilling::getUnfilled(std::vector<std::pair<int, int>>* unfilledAreas) const
	{
		TEST_PTR_RETURN(unfilledAreas);

		unfilledAreas->clear();

		int startUnfilled = -1;
		int sizeW = 0;

		for(int i = 0; i < static_cast<int>(m_busArea.size()); i++)
		{
			if (m_busArea[i] == 0)
			{
				if (startUnfilled == -1)
				{
					startUnfilled = i;
					sizeW = 1;
				}
				else
				{
					sizeW++;
				}

				continue;
			}

			// w != 0

			if (startUnfilled != -1)
			{
				unfilledAreas->push_back({startUnfilled, sizeW});

				startUnfilled = -1;
				sizeW = 0;
			}
		}

		if (startUnfilled != -1)
		{
			unfilledAreas->push_back({startUnfilled, sizeW});
		}
	}

	// ---------------------------------------------------------------------------------
	//
	//	ModuleLogicCompiler class implementation
	//
	// ---------------------------------------------------------------------------------

//	const char* ModuleLogicCompiler::INPUT_CONTROLLER_SUFFIX = "_CTRLIN";
//	const char* ModuleLogicCompiler::OUTPUT_CONTROLLER_SUFFIX = "_CTRLOUT";
//	const char* ModuleLogicCompiler::PLATFORM_INTERFACE_CONTROLLER_SUFFIX = "_PI";

//	const char* ModuleLogicCompiler::BUS_COMPOSER_CAPTION = "BusComposer";
//	const char* ModuleLogicCompiler::BUS_EXTRACTOR_CAPTION = "BusExtractor";

//	const char* ModuleLogicCompiler::TEST_DATA_DIR = "TestData/";

	const QString ModuleLogicCompiler::EMPTY_STR(QStringLiteral(""));

	ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, const Hardware::DeviceModule* lm) :
		m_appLogicCompiler(appLogicCompiler),
		m_context(appLogicCompiler.context()),
		m_lm(lm),
		m_memoryMap(appLogicCompiler.log()),
		m_ualSignals(*this, appLogicCompiler.log()),
		m_loopbacks(*this),
		m_appLogicCode(AppLogicCode::Type::AllCode, false),
		m_idrCode(AppLogicCode::Type::IDR_Code, false),
		m_alpCode(AppLogicCode::Type::ALP_Code, false),
		m_optiAppLogicCode(AppLogicCode::Type::AllCode, true),
		m_optiIdrCode(AppLogicCode::Type::IDR_Code, true),
		m_optiAlpCode(AppLogicCode::Type::ALP_Code, true)
	{
		m_equipmentSet = appLogicCompiler.equipmentSet();
		m_deviceRoot = m_equipmentSet->root().get();
		m_signals = appLogicCompiler.signalSet();

		m_lmDescription = appLogicCompiler.lmDescriptions()->get(lm);

		Q_ASSERT(m_lmDescription != nullptr);

		m_afbComponents.init(m_lmDescription);

		m_lmShared = getLmSharedPtr();

		Q_ASSERT(m_lm == m_lmShared.get());

		m_appLogicData = appLogicCompiler.appLogicData();
		m_resultWriter = appLogicCompiler.buildResultWriter();
		m_log = appLogicCompiler.log();

		m_connections = appLogicCompiler.connectionStorage();
		m_optoModuleStorage = appLogicCompiler.opticModuleStorage();
		m_tuningDataStorage = appLogicCompiler.tuningDataStorage();
		m_cmpSet = appLogicCompiler.comparatorSet();

		m_bitAccAvailable = m_lmDescription->isBitAccAvailable();
	}

	ModuleLogicCompiler::~ModuleLogicCompiler()
	{
		cleanup();
	}

	AppSignal* ModuleLogicCompiler::getSignal(const QString& appSignalID)
	{
		auto it = m_moduleSignals.find(calcHash(appSignalID));

		if (it == m_moduleSignals.end())
		{
			return nullptr;
		}

		return it->second;
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

		std::vector<ProcToCall> procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::loadLMSettings),
			PROC_TO_CALL(ModuleLogicCompiler::loadModulesSettings),
			PROC_TO_CALL(ModuleLogicCompiler::createModuleSignalsMap),
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
			PROC_TO_CALL(ModuleLogicCompiler::appendAfbsForInOutSignalsConversion),
			PROC_TO_CALL(ModuleLogicCompiler::setOutputSignalsAsComputed),
			PROC_TO_CALL(ModuleLogicCompiler::setOptoRawInSignalsAsComputed),
			PROC_TO_CALL(ModuleLogicCompiler::fillComparatorSet),
			PROC_TO_CALL(ModuleLogicCompiler::findEndpointSignals),
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

		std::vector<ProcToCall> procs =
		{
			PROC_TO_CALL(Builder::ModuleLogicCompiler::initComparatorSignals),

			PROC_TO_CALL(ModuleLogicCompiler::finalizeOptoConnectionsProcessing),
			PROC_TO_CALL(ModuleLogicCompiler::setOptoUalSignalsAddresses),
			//PROC_TO_CALL(ModuleLogicCompiler::writeSignalLists),			// extra debug info signal lists

			// code generation functions

			PROC_TO_CALL(ModuleLogicCompiler::generateAlpPhaseCode),

			// Some UalAfb items dynamically creating in time of generateAlpPhaseCode processing.
			// Therefore generateIdrPhaseCode, that produce UalAfb params initialization code,
			// called AFTER generateAlpPhaseCode!
			//
			PROC_TO_CALL(ModuleLogicCompiler::generateIdrPhaseCode),
			PROC_TO_CALL(ModuleLogicCompiler::cleanupHeaps),

			PROC_TO_CALL(ModuleLogicCompiler::makeSourceAppLogicCode),
			PROC_TO_CALL(ModuleLogicCompiler::writeLmInfoFiles),
			PROC_TO_CALL(ModuleLogicCompiler::checkAppLogicCode),

			PROC_TO_CALL(ModuleLogicCompiler::optimizeAppLogicCode),
			PROC_TO_CALL(ModuleLogicCompiler::makeOptimizedAppLogicCode),
			PROC_TO_CALL(ModuleLogicCompiler::writeInfoLmFilesAfterOptimization),
			PROC_TO_CALL(ModuleLogicCompiler::checkOptimizedAppLogicCode),

			//

			PROC_TO_CALL(ModuleLogicCompiler::setLmAppLanDataSize),
			PROC_TO_CALL(ModuleLogicCompiler::setLmDiagLanDataSize),

			PROC_TO_CALL(ModuleLogicCompiler::detectUnusedSignals),
			PROC_TO_CALL(ModuleLogicCompiler::detectUsedReservedSignals),
			PROC_TO_CALL(ModuleLogicCompiler::fillAnalogSignalsOnSchemas),

			PROC_TO_CALL(ModuleLogicCompiler::calcAppDataUID),
			PROC_TO_CALL(ModuleLogicCompiler::calcDiagDataUID),

			PROC_TO_CALL(ModuleLogicCompiler::writeResult),
			PROC_TO_CALL(ModuleLogicCompiler::writeNonPlatformRegInfoFile)
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
			return EMPTY_STR;
		}

		return m_lm->equipmentIdTemplate();
	}

	int ModuleLogicCompiler::lmDescriptionNumber() const
	{
		if (m_lmDescription == nullptr)
		{
			Q_ASSERT(false);
			return 0;
		}

		return m_lmDescription->descriptionNumber();
	}

	QString ModuleLogicCompiler::lmDescriptionName() const
	{
		if (m_lmDescription == nullptr)
		{
			Q_ASSERT(false);
			return 0;
		}

		return m_lmDescription->name();
	}

	bool ModuleLogicCompiler::expertMode() const
	{
		return m_context->m_expertMode;
	}

	bool ModuleLogicCompiler::generateExtraDebugInfo() const
	{
		return m_context->generateExtraDebugInfo();
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

		const std::vector<SchemaPin>& outputs = item->outputs();

		for(const SchemaPin& outPin : outputs)
		{
			result &= getSignalsAndPinsLinkedToOutPin(item, outPin, linkedSignals, linkedItems, linkedPins);
		}

		return result;
	}

	std::shared_ptr<Hardware::DeviceModule> ModuleLogicCompiler::getLmSharedPtr()
	{
		TEST_PTR_LOG_RETURN_NULLPTR(m_lm, m_log);

		if (m_lmShared == nullptr)
		{
			m_lmShared = std::dynamic_pointer_cast<Hardware::DeviceModule>(getDeviceSharedPtr(lmEquipmentID()));
		}

		return 	m_lmShared;
	}

	std::shared_ptr<const LmDescription> ModuleLogicCompiler::getLmDescription() const
	{
		TEST_PTR_LOG_RETURN_NULLPTR(m_lmDescription, m_log);

		return std::const_pointer_cast<const LmDescription>(m_lmDescription);
	}

	BusShared ModuleLogicCompiler::getBusShared(const QString& busTypeID)
	{
		return m_signals->getBus(busTypeID);
	}

	bool ModuleLogicCompiler::getTuningSignalsFramesInfo(std::vector<std::pair<quint32, quint32>>* framesInfo) const
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_tuningDataStorage, m_log);
		TEST_PTR_LOG_RETURN_FALSE(framesInfo, m_log);

		framesInfo->clear();

		Tuning::TuningDataShared tuningData = m_tuningDataStorage->getTuningData(lmEquipmentID());

		if (tuningData == nullptr)
		{
			return true;
		}

		tuningData->getTuningSignalsFramesInfo(framesInfo);

		return true;
	}

	bool ModuleLogicCompiler::getLmAssociatedOptoPortsRxAreas(std::vector<CodeChecker::MemArea>* optoRxAreas) const
	{
		return getLmAssociatedOptoPortsAreas(optoRxAreas, true);
	}

	bool ModuleLogicCompiler::getLmAssociatedOptoPortsTxAreas(std::vector<CodeChecker::MemArea>* optoTxAreas) const
	{
		return getLmAssociatedOptoPortsAreas(optoTxAreas, false);
	}

	bool ModuleLogicCompiler::noCodeGenRequired() const
	{
		TEST_PTR_RETURN_FALSE(m_lm);

		return m_lm->isBvb() || m_lm->isMso();
	}

	const UalAfbs& ModuleLogicCompiler::ualAfbs() const
	{
		return m_ualAfbs;
	}

	QList<const UalSignal*> ModuleLogicCompiler::getLoopbacksUalSignals() const
	{
		return m_loopbacks.getLoopbacksUalSignals();
	}

	bool ModuleLogicCompiler::optimizeCode( CodeOptimizationType optimizationType,
											const CodeSnippet& srcCode,
											CodeSnippetConstIterator start,
											CodeSnippetConstIterator end,
											CodeSnippet& optimizedCode,
											const CodeSnippet& replacementCode)
	{
		m_optimizationNo++;

		Q_ASSERT(start != end && start != srcCode.end());

		CodeSnippetConstIterator prev = start - 1;

		if (prev != srcCode.end())
		{
			if (prev->isNewLine() == false)
			{
				optimizedCode << CodeItem();
			}
		}

		optimizedCode << CodeItem().setComment(QString("Optimization (%1) ----- Begin ----- %2").
												arg(m_optimizationNo).
												arg(OptimizationInfo::typeStr(optimizationType)));
		optimizedCode << CodeItem();

		int replacementCodeSizeW  = replacementCode.codeSizeW(m_lmDescription);
		int replacedCodeSizeW = 0;

		do
		{
			if (start == srcCode.end())
			{
				break;
			}

			const CodeItem& srcCodeItem = *start;

			replacedCodeSizeW += srcCodeItem.sizeW();

			QString mnemo = srcCodeItem.getAsmCode(m_lmDescription, true, false);

			optimizedCode << CodeItem().setComment(mnemo);

			if (start == end)
			{
				break;
			}

			start++;
		}
		while(true);

		optimizedCode << CodeItem();

		optimizedCode << replacementCode;

		optimizedCode << CodeItem();

		optimizedCode << CodeItem().setComment(QString("Optimization (%1) ------ End ------ Code size %2").
											  arg(m_optimizationNo).
											  arg(replacementCodeSizeW - replacedCodeSizeW));
		if (start != srcCode.end())
		{
			CodeSnippetConstIterator next = start + 1;

			if (next != srcCode.end())
			{
				if (next->isNewLine() == false)
				{
					optimizedCode << CodeItem();
				}
			}
		}

		//

		auto it = m_optimizationsInfo.find(optimizationType);

		if (it == m_optimizationsInfo.end())
		{
			auto p = m_optimizationsInfo.insert({optimizationType, OptimizationInfo(optimizationType)});

			it = p.first;
		}

		OptimizationInfo& optiInfo = it->second;

		optiInfo.optimizationsCount++;
		optiInfo.codeReductionSizeW += replacedCodeSizeW - replacementCodeSizeW;

		return true;
	}

	int ModuleLogicCompiler::bitAccumulatorAddress() const
	{
		return m_memoryMap.bitAccumulatorAddress();
	}

	Address16 ModuleLogicCompiler::bitAccumulatorAddress16() const
	{
		return Address16(m_memoryMap.bitAccumulatorAddress(), 0);
	}

	int ModuleLogicCompiler::wordAccumulatorAddress() const
	{
		return m_memoryMap.wordAccumulatorAddress();
	}

	Address16 ModuleLogicCompiler::wordAccumulatorAddress16() const
	{
		return Address16(m_memoryMap.wordAccumulatorAddress(), 0);
	}

	int ModuleLogicCompiler::wordAccumulator2Address() const
	{
		return m_memoryMap.wordAccumulator2Address();
	}

	Address16 ModuleLogicCompiler::getDiscreteUalAddrBitConstIncluded(const UalSignal* ualSignal) const
	{
		if (ualSignal->isConstDiscrete() == true)
		{
			return ualSignal->constDiscreteValue() == 0 ?
							constBit0Addr() :
							constBit1Addr();
		}

		if (ualSignal->isDiscrete() == true)
		{
			return ualSignal->ualAddr();
		}

		Q_ASSERT(false);

		return Address16();
	}

	bool ModuleLogicCompiler::getLmAssociatedOptoPortsAreas(std::vector<CodeChecker::MemArea>* optoAreas, bool rx) const
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(optoAreas, m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_optoModuleStorage, m_log);

		QList<Hardware::OptoPortShared> ports;

		m_optoModuleStorage->getLmAssociatedOptoPorts(lmEquipmentID(), ports);

		for(Hardware::OptoPortShared& port : ports)
		{
			TEST_PTR_CONTINUE(port);

			if (rx == true)
			{
				if (port->rxDataSizeW() > 0)
				{
					optoAreas->emplace_back(port->rxBufAddress(), port->rxDataSizeW());
				}
			}
			else
			{
				if (port->txDataSizeW() > 0)
				{
					optoAreas->emplace_back(port->txBufAddress(), port->txDataSizeW());
				}
			}
		}

		return true;
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

		if (m_lm->isLogicModule() && (m_lmCodeMemorySize == 0 || m_lmAppMemorySize == 0))
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

		m.device = m_lmShared;
		m.place = m_lmShared->place();

		if (m_lmShared->isLogicModule())
		{
			if (m.place != DeviceHelper::LM_PLACE1)
			{
				// Module %1 should be installed on place %2.
				//
				m_log->errEQP6012(m_lmShared->equipmentIdTemplate(), DeviceHelper::LM_PLACE1);
				return false;
			}
		}

		if	(m_lmShared->isBvb())
		{
			if (m.place != DeviceHelper::BVB_PLACE1 && m.place != DeviceHelper::BVB_PLACE2)
			{
				// Module %1 should be installed on place %2 or %3.
				//
				m_log->errEQP6013(m_lmShared->equipmentIdTemplate(), DeviceHelper::BVB_PLACE1, DeviceHelper::BVB_PLACE2);
				return false;
			}
		}

		if (m_lmShared->isMso())
		{
			if (m.place != DeviceHelper::MSO_PLACE1 && m.place != DeviceHelper::MSO_PLACE2)
			{
				// Module %1 should be installed on place %2 or %3.
				//
				m_log->errEQP6013(m_lmShared->equipmentIdTemplate(), DeviceHelper::MSO_PLACE1, DeviceHelper::MSO_PLACE2);
				return false;
			}
		}

		m.txDiagDataOffset = m_lmDescription->memory().m_txDiagDataOffset;
		m.txDiagDataSize = m_lmDescription->memory().m_txDiagDataSize;

		m.txAppDataOffset = m_lmDescription->memory().m_appDataOffset;
		m.txAppDataSize = m_lmDescription->memory().m_appDataSize;

		m.moduleDataOffset = 0;

		m.rxAppDataOffset = m.txAppDataOffset;
		m.rxAppDataSize = m.txAppDataSize;

		Q_ASSERT(m_modules.contains(m.place) == false);

		m_modules.emplace(m.place, m);

		// check LM subsystem ID
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
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_lm, m_log);

		bool result = true;

		const Hardware::DeviceChassis* chassis = m_lm->getParentChassis();

		if (chassis == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't find parent chassis for LM %1").
											arg(m_lm->equipmentIdTemplate()));
			return false;
		}

		const std::vector<std::shared_ptr<Hardware::DeviceObject>>& chassisChildren = chassis->children();

		// fill m_modules ordered by place
		// LM or BVB already in m_modules!
		//
		for(const std::shared_ptr<Hardware::DeviceObject>& device : chassisChildren)
		{
			if (device == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (device->isModule() == false)
			{
				continue;
			}

			std::shared_ptr<const Hardware::DeviceModule> module = device->toModule();

			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if ((module->isLogicModule() &&module->place() == DeviceHelper::LM_PLACE1) ||
					//
				(module->isBvb() && (module->place() == DeviceHelper::BVB_PLACE1 ||
									 module->place() == DeviceHelper::BVB_PLACE2)) ||
					//
				(module->isMso() && (module->place() == DeviceHelper::MSO_PLACE1 ||
									 module->place() == DeviceHelper::MSO_PLACE2))	)
			{
				if (module->place() != m_lm->place())
				{
					continue;
				}

				if (module != m_lmShared)
				{
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					result = false;
				}

				continue;			// LM (or BVB) module already added into m_modules in ModuleLogicCompiler::loadLMSettings()
			}

			Module m;

			m.device = module;
			m.place = module->place();

			m_modules.emplace(m.place, m);

		}

		bool isLmChassis = m_lm->isLogicModule();
		bool isBvbChassis = m_lm->isBvb();
		bool isMsoChassis = m_lm->isMso();

		int appRegDataOffset = 0;
		int diagRegDataOffset = 0;
		int prevModulePlace = 0;

		// read modules properties
		//
		for(auto& [place, module] : m_modules)
		{
			const Hardware::DeviceModule* deviceModule = module.device.get();

			TEST_PTR_CONTINUE(deviceModule);

			if (isLmChassis == true)
			{
				if (deviceModule->isLogicModule())
				{
					continue;
				}

				// parameters of data RECEIVED from module to LM (i.e. TRANSMITTED by module)
				//
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_DATA_SIZE, &module.txDataSize, m_log);

				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_DIAG_DATA_OFFSET, &module.txDiagDataOffset, m_log);
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_DIAG_DATA_SIZE, &module.txDiagDataSize, m_log);

				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_APP_DATA_OFFSET, &module.txAppDataOffset, m_log);
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_APP_DATA_SIZE, &module.txAppDataSize, m_log);

				// parameters of data TRANSMITTED from LM to module (i.e. RECEIVED by module)
				//
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::RX_DATA_SIZE, &module.rxDataSize, m_log);

				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::RX_APP_DATA_OFFSET, &module.rxAppDataOffset, m_log);
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::RX_APP_DATA_SIZE, &module.rxAppDataSize, m_log);

				module.moduleDataOffset = m_memoryMap.getModuleDataOffset(module.place);

				continue;
			}

			//

			if (isBvbChassis == true)
			{
				if (deviceModule->isBvb())
				{
					continue;
				}

				const int BUIM_REG_INFO_SIZE_W = 14;

				if (prevModulePlace + 1 != module.place)
				{
					// in BVB registration packet empty space reserved for not installed modules
					//
					appRegDataOffset = (module.place - prevModulePlace - 1) * BUIM_REG_INFO_SIZE_W;
				}

				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_APP_DATA_SIZE, &module.txAppDataSize, m_log);
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_DIAG_DATA_SIZE, &module.txDiagDataSize, m_log);

				if (module.txAppDataSize != BUIM_REG_INFO_SIZE_W)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("BUIM %1 reg info size is not equal %2 words!").
											arg(deviceModule->equipmentIdTemplate()).
											arg(BUIM_REG_INFO_SIZE_W));
					result = false;
				}

				module.txAppDataOffset = 0;
				module.txDiagDataOffset = 0;
				module.txDataSize = 0;				// its Ok, because it is not LM and app and diag data transmit in separate streams

				module.appRegDataOffset = appRegDataOffset;
				module.diagRegDataOffset = diagRegDataOffset;

				appRegDataOffset += module.txAppDataSize;
				diagRegDataOffset += module.txDiagDataSize;

				continue;
			}

			//

			if (isMsoChassis == true)
			{
				if (deviceModule->isMso())
				{
					continue;
				}

				const int MSO_IO_MODULE_REG_INFO_SIZE_W = 14;

				if (prevModulePlace + 1 != module.place)
				{
					// in MSO registration packet empty space reserved for not installed modules
					//
					appRegDataOffset = (module.place - prevModulePlace - 1) * MSO_IO_MODULE_REG_INFO_SIZE_W;
				}

				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_APP_DATA_SIZE, &module.txAppDataSize, m_log);
				result &= DeviceHelper::getIntProperty(deviceModule, EquipmentPropNames::TX_DIAG_DATA_SIZE, &module.txDiagDataSize, m_log);

				if (module.txAppDataSize != MSO_IO_MODULE_REG_INFO_SIZE_W)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("MSO i/o module %1 reg info size is not equal %2 words!").
											arg(deviceModule->equipmentIdTemplate()).
											arg(MSO_IO_MODULE_REG_INFO_SIZE_W));
					result = false;
				}

				module.txAppDataOffset = 0;
				module.txDiagDataOffset = 0;
				module.txDataSize = 0;				// its Ok, because it is not LM and app and diag data transmit in separate streams

				module.appRegDataOffset = appRegDataOffset;
				module.diagRegDataOffset = diagRegDataOffset;

				appRegDataOffset += module.txAppDataSize;
				diagRegDataOffset += module.txDiagDataSize;

				continue;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::createModuleSignalsMap()
	{
		bool result = true;

		m_moduleSignals.clear();
		m_ioSignals.clear();
		m_swCalcSignals.clear();

		for(AppSignal* sg : *m_signals)
		{
			TEST_PTR_CONTINUE(sg);

			if (sg->equipmentID().isEmpty() == true)
			{
				continue;
			}

			Hardware::DeviceAppSignal* deviceAppSignal = nullptr;

			bool isIoSignal = false;

			Hardware::DeviceObject* device = m_equipmentSet->deviceObject(sg->equipmentID()).get();

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

					if (!(deviceModule->isLogicModule() ||
						  deviceModule->isNonPlatformAppDataSourceModule()))
					{
						assert(false); // signal must be associated with LM or non-platform app data module
						continue;
					}

					if (deviceModule->equipmentIdTemplate() != lmEquipmentID())
					{
						continue;
					}

					// signal is associated with current LM

					if (sg->swCalcFunction() != E::SoftwareCalcFunction::None)
					{
						m_swCalcSignals.emplace_back(sg);
					}

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

					if (deviceAppSignal->isInputSignal() ||
						deviceAppSignal->isOutputSignal() ||
						deviceAppSignal->isValiditySignal())
					{
						isIoSignal = true;
					}

					if (sg->swCalcFunction() != E::SoftwareCalcFunction::None)
					{
						m_swCalcSignals.emplace_back(sg);
					}
				}

				break;

			default:
				assert(false); // signal must be associated with DeviceSignal or DeviceModule (LM) only
				continue;
			}

			auto [newIt, inserted] = m_moduleSignals.emplace(calcHash(sg->appSignalID()), sg);

			if (inserted == false)
			{
				Q_ASSERT(false);				// duplicate AppSignalID !!!
				continue;
			}

			if (isIoSignal == true)
			{
				m_ioSignals.emplace_back(sg);

				if (deviceAppSignal != nullptr)
				{
					m_equipmentSignals.emplace(calcHash(deviceAppSignal->equipmentIdTemplate()), sg);
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
				UalItem* firstItem = m_ualItems.value(logicItem.m_fblItem->guid());

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
			for(const SchemaPin& input : appItem->inputs())
			{
				UalItem* firstItem = getValueOrNullptr(m_pinParent, input.guid());
				//UalItem* firstItem = m_pinParent.value(input.guid(), nullptr);

				if (firstItem != nullptr)
				{

					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Duplicate input pin GUID %1 of %2 and %3 elements")).
											arg(input.guid().toString()).
											arg(firstItem->strID()).arg(appItem->strID()));

					result = false;

					continue;
				}

				m_pinParent.emplace(input.guid(), appItem);
			}

			// add output pins
			//
			for(const SchemaPin& output : appItem->outputs())
			{
				UalItem* firstItem = getValueOrNullptr(m_pinParent, output.guid());

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

				m_pinParent.emplace(output.guid(), appItem);
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

		bool packedLogicExists = false;

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
				continue;
			}

			packedLogicExists |= ualAfb->isPackedLogic();
		}

		RETURN_IF_FALSE(result);

		findLogicAfbsForBitAccReplacing(Afb::AFB_AND, 1, &m_afbsAndForBitAccReplacing);
		findLogicAfbsForBitAccReplacing(Afb::AFB_OR, 2, &m_afbsOrForBitAccReplacing);

		if (packedLogicExists && m_lmDescription->isBitAccAvailable() == false)
		{
			// reserve one instance of AFB LOGIC to process Packed Logic items
			// this instance will fully configured just before packed logic processing
			//
			result &= m_afbComponents.addInstance(TO_INT(Afb::AfbType::LOGIC), &m_packedLogicAfbInstance, m_log);

			Q_ASSERT(m_packedLogicAfbInstance >= 0);
		}

		return result;
	}

	UalAfb* ModuleLogicCompiler::createUalAfb(const UalItem& ualItem)
	{
		if (ualItem.isAfb() == false)
		{
			return nullptr;
		}

		UalAfb* ualAfb = new UalAfb(ualItem, m_afbComponents.isBusProcessingAfb(ualItem.strID()));

		if (ualAfb->calculateFbParamValues(this) == false)
		{
			delete ualAfb;
			return nullptr;
		}

		// get Functional Block instance
		//
		bool result = m_afbComponents.addInstance(ualAfb, m_log);

		if (result == false)
		{
			delete ualAfb;
			return nullptr;
		}

		m_ualAfbs.insert(ualAfb);

		return ualAfb;
	}

	bool ModuleLogicCompiler::createUalSignals()
	{
		m_ualSignals.clear();

		bool result = true;

		result &= writeUalItemsFile();

		result &= createUalItemSignalsList();

		result &= loopbacksPreprocessing();

		// primarily created signals
		//
		result &= createUalSignalsFromInputAndTuningAcquiredSignals();
		result &= createUalSignalsForNonPlatformModules();
		result &= createUalSignalsFromBusComposers();
		result &= createUalSignalsFromOptoValidity();
		result &= createUalSignalsFromReceivers();

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

		return result;
	}

	bool ModuleLogicCompiler::writeUalItemsFile()
	{
		if (m_context->generateExtraDebugInfo() == false ||
			noCodeGenRequired() == true)
		{
			return true;
		}

		QStringList items;

		for(const UalItem* ualItem : m_ualItems)
		{
			QString itemStr;

			E::UalItemType itemType = ualItem->type();

			itemStr = QString("%1, %2").arg(E::valueToString(itemType)).arg(ualItem->label().leftJustified(16));

			switch(itemType)
			{
			case E::UalItemType::Unknown:
				break;

			case E::UalItemType::Signal:
				{
					const SchemaSignal* l = ualItem->schemaSignal();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", ids=%1").arg(l->appSignalIdList().join(","));
				}
				break;

			case E::UalItemType::Afb:
				{
					const SchemaAfb* l = ualItem->schemaAfb();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", %1, opcode=%2").arg(l->afbStrID()).arg(l->afbElement().opCode());
				}
				break;

			case E::UalItemType::Const:
				{
					const SchemaConst* l = ualItem->schemaConst();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", type=%1").
									arg(E::valueToString(l->type()));

					switch(l->type())
					{
					case VFrame30::SchemaItemConst::ConstType::IntegerType:
							itemStr += QString(", val=%1").arg(l->signedInt32NativeValue());
							break;
					case VFrame30::SchemaItemConst::ConstType::FloatType:
							itemStr += QString(", val=%1").arg(l->floatNativeValue());
							break;
					case VFrame30::SchemaItemConst::ConstType::Discrete:
							itemStr += QString(", val=%1").arg(l->discreteNativeValue());
							break;
					}
				}
				break;

			case E::UalItemType::Transmitter:
				{
					const SchemaTransmitter* l = ualItem->schemaTransmitter();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", conn=%1").arg(l->connectionIdsAsList().join(", "));
				}
				break;

			case E::UalItemType::Receiver:
				{
					const SchemaReceiver* l = ualItem->schemaReceiver();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", conn=%1").arg(l->connectionIdsAsList().join(", "));
				}
				break;

			case E::UalItemType::Terminator:
				break;

			case E::UalItemType::BusComposer:
				{
					const SchemaBusComposer* l = ualItem->schemaBusComposer();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", busType=%1").arg(l->busTypeId());
				}
				break;

			case E::UalItemType::BusExtractor:
				{
					const SchemaBusExtractor* l = ualItem->schemaBusExtractor();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", busType=%1").arg(l->busTypeId());
				}
				break;

			case E::UalItemType::LoopbackSource:
				{
					const SchemaLoopbackSource* l = ualItem->schemaLoopbackSource();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", loopbackID=%1").arg(l->loopbackId());
				}
				break;

			case E::UalItemType::LoopbackTarget:
				{
					const SchemaLoopbackTarget* l = ualItem->schemaLoopbackTarget();
					TEST_PTR_CONTINUE(l);
					itemStr += QString(", loopbackID=%1").arg(l->loopbackId());
				}
				break;

			default:
				Q_ASSERT(false);
			}

			items.append(itemStr);
		}

		return m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
									   getInfoFileName("uil"), "", "", items, false);
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

			std::vector<SchemaPin>& outputs = ualItem->outputs();

			for(SchemaPin& output : outputs)
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
					std::shared_ptr<SchemaLoopbackSource> loopbackSource = std::make_shared<SchemaLoopbackSource>();
					loopbackSource->setLoopbackId(autoLoopbackID);
					loopbackSource->setLabel(autoLoopbackSourceLabel);

					UalItem* newSourceUalItem = new UalItem(AppLogicItem(loopbackSource, ualItem->schema()));
					createdItems.append(newSourceUalItem);
					m_pinParent.emplace(loopbackSource->inputs()[0].guid(), newSourceUalItem);

					// link ualItem output and loopback source input to each other
					//
					output.AddAssociattedIOs(loopbackSource->inputs()[0].guid());
					loopbackSource->inputs()[0].AddAssociattedIOs(output.guid());
				}
				else
				{
					// has LoopbackSource connected to ouput, auto LoopbackSource is not required
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
					SchemaPin& input = ualItem->input(inputGuid);

					// break output to input link
					//
					output.removeFromAssociatedIo(input.guid());
					input.removeFromAssociatedIo(output.guid());

					// LoopbackTarget creation
					//
					std::shared_ptr<SchemaLoopbackTarget> loopbackTarget = std::make_shared<SchemaLoopbackTarget>();
					loopbackTarget->setLoopbackId(autoLoopbackID);
					loopbackTarget->setLabel(autoLoopbackTargetLabel);

					UalItem* newTargetUalItem = new UalItem(AppLogicItem(loopbackTarget, ualItem->schema()));
					createdItems.append(newTargetUalItem);
					m_pinParent.emplace(loopbackTarget->outputs()[0].guid(), newTargetUalItem);

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
														 const SchemaPin& output,
														 QVector<QUuid>* connectedInputsGuids)
	{
		TEST_PTR_RETURN(ualItem);
		TEST_PTR_RETURN(connectedInputsGuids);

		const std::vector<QUuid>& associatedIOsGuids = output.associatedIOs();

		for(const QUuid& pinGuid : associatedIOsGuids)
		{
			UalItem* pinParent = getValueOrNullptr(m_pinParent, pinGuid);

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

	QString ModuleLogicCompiler::getConnectedLoopbackSourceID(const SchemaPin& output)
	{
		const std::vector<QUuid>& associatedIOsGuids = output.associatedIOs();

		for(const QUuid& pinGuid : associatedIOsGuids)
		{
			UalItem* pinParent = getValueOrNullptr(m_pinParent, pinGuid);

			if (pinParent == nullptr)
			{
				assert(false);
				continue;
			}

			if (pinParent->isLoopbackSource() == true)
			{
				const SchemaLoopbackSource* src = pinParent->schemaLoopbackSource();

				if (src != nullptr)
				{
					return src->loopbackId();
				}

				assert(false);
			}
		}

		return EMPTY_STR;
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

			const SchemaLoopbackSource* source = ualItem->schemaLoopbackSource();

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

			const SchemaLoopbackTarget* target = ualItem->schemaLoopbackTarget();

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
															const SchemaPin& outPin,
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
			const UalItem* linkedItem = getValueOrNullptr(m_pinParent, inPin);

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

	bool ModuleLogicCompiler::createUalItemSignalsList()
	{
		m_ualItemsSignals.clear();

		for(const UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSignal() == false)
			{
				continue;
			}

			const SchemaSignal* s = ualItem->schemaSignal();

			TEST_PTR_CONTINUE(s);

			QStringList appSignalIDs = s->appSignalIdList();

			for(const QString& id : appSignalIDs)
			{
				Hash idHash = calcHash(id);

				auto it = m_ualItemsSignals.find(idHash);

				if (it == m_ualItemsSignals.end())
				{
					auto [newIt, b] = m_ualItemsSignals.emplace(idHash, std::set<QUuid>());

					it = newIt;
				}

				it->second.insert(ualItem->guid());
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::createUalSignalsFromInputAndTuningAcquiredSignals()
	{
		bool result = true;

		// fill m_ualSignals by Input and Tuning Acquired signals
		//
		for(const auto& [appSignalID, appSignal] : m_moduleSignals)
		{
			TEST_PTR_CONTINUE(appSignal);

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

	bool ModuleLogicCompiler::createUalSignalsForNonPlatformModules()
	{
		if (m_lm->isNonPlatformAppDataSourceModule() == false)
		{
			return true;
		}

		bool result = true;

		for(const auto& [appSignalID, appSignal] : m_moduleSignals)
		{
			TEST_PTR_CONTINUE(appSignal);

			if (appSignal->isAcquired() == false)
			{
				continue;
			}

			if (appSignal->isOutput() == true)
			{
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

		const SchemaBusComposer* busComposer = ualItem->schemaBusComposer();

		if (busComposer == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<SchemaPin>& outputs = busComposer->outputs();

		if (outputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const SchemaPin& outPin = outputs[0];

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

	UalSignal* ModuleLogicCompiler::createBusParentSignal(UalItem* ualItem, const SchemaPin& outPin, AppSignal* appBusSignal, const QString& busTypeID)
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

		const SchemaBusExtractor* busExtractor = ualItem->schemaBusExtractor();

		if (busExtractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const std::vector<SchemaPin>& inputs = busExtractor->inputs();

		if (inputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const SchemaPin& inPin = inputs[0];

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

		const SchemaReceiver* ualReceiver = ualItem->schemaReceiver();

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

		const std::vector<SchemaPin>& outputs = ualItem->outputs();

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
		const SchemaPin& outPin = outputs[outPinIndex];

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

		const SchemaPin& validityPin = outputs[validityPinIndex];

		result &= createUalSignalFromReceiverValidity(ualItem, validityPin, connection);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromReceiverOutput(UalItem* ualItem, const SchemaPin& outPin, const QString& receivedAppSignalID, bool isSinglePortConnection)
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
			if (m_moduleSignals.contains(calcHash(receivedAppSignalID)) == true)
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

		ualSignal->setReceivedOptoAppSignalID(receivedAppSignalID, ualItem->schemaReceiver());

		result &= linkConnectedItems(ualItem, outPin, ualSignal);

		return result;
	}

	bool ModuleLogicCompiler::createUalSignalFromReceiverValidity(UalItem* ualItem,
																  const SchemaPin& validityPin,
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

		auto it = m_equipmentSignals.find(calcHash(validitySignalEquipmentID));

		if (it == m_equipmentSignals.end())
		{
			m_log->errALC5133(validitySignalEquipmentID, ualItem->guid(), ualItem->label(), ualItem->schemaID());
			return false;
		}

		AppSignal* validityAppSignal = it->second;

		TEST_PTR_RETURN_FALSE(validityAppSignal);

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

	bool ModuleLogicCompiler::getReceiverConnectionID(const SchemaReceiver* receiver, QString* connectionID, const QString& schemaID)
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

		if (m_lm->isLogicModule() == false)
		{
			return true;
		}

		//
		// Appending OptoValidity signals for USED opto ports in current LM and associated OptoModules
		//

		bool result = true;

		QList<Hardware::OptoPortShared> lmAssociatedPorts;

		result = m_optoModuleStorage->getLmAssociatedOptoPorts(lmEquipmentID(), lmAssociatedPorts);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(result, m_log);

		for(Hardware::OptoPortShared optoPort : lmAssociatedPorts)
		{
			if (optoPort == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (optoPort->isUsedInConnection() == false)
			{
				continue;
			}

			QString validitySignalEquipmentID = optoPort->validitySignalEquipmentID();

			AppSignal* validitySignal = nullptr;

			auto it = m_equipmentSignals.find(calcHash(validitySignalEquipmentID));

			if (it != m_equipmentSignals.end())
			{
				validitySignal = it->second;
			}
			else
			{
				// validity signal is not exists, create corresponding AppSignal
				//
				QString optoPortID = optoPort->equipmentID();

				const std::shared_ptr<Hardware::DeviceObject> deviceShared = m_equipmentSet->deviceObject(validitySignalEquipmentID);

				if (deviceShared == nullptr)
				{
					// Device Object %1 not found.
					//
					m_log->errEQP6010(validitySignalEquipmentID);
					result = false;
					continue;
				}

				const Hardware::DeviceAppSignal* deviceAppSignal = dynamic_cast<const Hardware::DeviceAppSignal*>(deviceShared.get());

				if (deviceAppSignal == nullptr)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("Device object %1 is not a DeviceAppSignal type").
											arg(validitySignalEquipmentID));
					result = false;
					continue;
				}

				validitySignal = new AppSignal;

				QString errStr = DbControllerTools::initAppSignalFromDeviceAppSignal(*deviceAppSignal, validitySignal);

				if (errStr.isEmpty() == false)
				{
					delete validitySignal;		// !!!
					validitySignal = nullptr;
					LOG_INTERNAL_ERROR_MSG(m_log, errStr);
					result = false;
					continue;
				}

				// Opto validity signal IDs reserv templates to avoid coincidence with exists signals
				//
				static const QStringList reservAppSignalIDTemplates(
							{QString("#%1_VALID"),
							 QString("#%1_VALIDITY")});

				int templateNo = 0;
				bool canCreate = false;

				do
				{
					const AppSignal* existSignal = m_signals->getSignal(validitySignal->appSignalID());

					if (existSignal == nullptr)
					{
						canCreate = true;
						break;
					}

					validitySignal->setAppSignalID(reservAppSignalIDTemplates[templateNo].arg(optoPortID));

					templateNo++;
				}
				while(templateNo < reservAppSignalIDTemplates.count());

				if (canCreate == false)
				{
					delete validitySignal;		// !!!
					validitySignal = nullptr;

					LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't auto create opto validity signal for port %1").
													arg(optoPortID));
					result = false;
				}
				else
				{
					m_signals->append(validitySignal, m_lmShared);

					m_moduleSignals.emplace(calcHash(validitySignal->appSignalID()), validitySignal);
					m_ioSignals.emplace_back(validitySignal);
					m_equipmentSignals.emplace(calcHash(validitySignalEquipmentID), validitySignal);
				}
			}

			TEST_PTR_CONTINUE(validitySignal);

			validitySignal->setAcquire(true);

			UalSignal* validtyUalSignal = m_ualSignals.createSignal(validitySignal);

			if (validtyUalSignal == nullptr)
			{
				result = false;
			}

			m_optoPortValiditySignal.emplace(calcHash(optoPort->equipmentID()), validtyUalSignal);
		}

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

		if (m_moduleSignals.contains(calcHash(signalID)) == false)
		{
			// The signal '%1' is not associated with LM '%2'.
			//
			m_log->errALC5030(signalID, m_lm->equipmentId(), ualItem->guid());
			return false;
		}

		if (appSignal->isSwCalculated() == true)
		{
			// Software calculated signal %1 cannot be used in user application logic (schema %2).
			//
			m_log->errALC5205(appSignal->appSignalID(), ualItem->guid(), ualItem->schemaID());
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

		const std::vector<SchemaPin>& outputs = ualItem->outputs();

		if (outputs.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// input signal must have only one output pin
			return false;
		}

		const SchemaPin& outPin = ualItem->outputs()[0];

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

		const SchemaConst* ualConst = ualItem->schemaConst();

		if (ualConst == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<SchemaPin>& outputs = ualItem->outputs();

		if (outputs.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// Const must have only one output pin
			return false;
		}

		const SchemaPin& outPin = ualItem->outputs()[0];

		E::SignalType constSignalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat constAnalogFormat = E::AnalogAppSignalFormat::SignedInt32;

		switch(ualConst->type())
		{
		case VFrame30::SchemaItemConst::ConstType::Discrete:
			Q_ASSERT(ualConst->discreteValue().hasReference() == false);

			constSignalType = E::SignalType::Discrete;

			if (ualConst->discreteNativeValue() != 0 && ualConst->discreteNativeValue() != 1)
			{
				// Discrete constant must have value 0 or 1.
				//
				m_log->errALC5086(ualItem->guid(), ualItem->schemaID());
				return false;
			}
			break;

		case VFrame30::SchemaItemConst::ConstType::IntegerType:
			Q_ASSERT(ualConst->signedInt32Value().hasReference() == false);

			constSignalType = E::SignalType::Analog;
			constAnalogFormat = E::AnalogAppSignalFormat::SignedInt32;

			if (ualConst->signedInt32NativeValue() < std::numeric_limits<qint32>::min() || ualConst->signedInt32NativeValue() > std::numeric_limits<qint32>::max())
			{
				// Integer constant value out of range (Logic schema %1, item %2)
				//
				m_log->errALC5134(ualItem->guid(), ualItem->label(), ualItem->schemaID());
				return false;
			}

			break;

		case VFrame30::SchemaItemConst::ConstType::FloatType:
			Q_ASSERT(ualConst->floatValue().hasReference() == false);

			constSignalType = E::SignalType::Analog;
			constAnalogFormat = E::AnalogAppSignalFormat::Float32;

			if (ualConst->floatNativeValue() < std::numeric_limits<float>::lowest() || ualConst->floatNativeValue() > std::numeric_limits<float>::max())
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

		UalAfb* ualAfb = m_ualAfbs.getAfb(ualItem->guid());

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

		const std::vector<SchemaPin>& outputs = ualItem->outputs();

		for(const SchemaPin& outPin : outputs)
		{
			AfbSignal outAfbSignal;

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

		const SchemaBusExtractor* extractor = ualItem->schemaBusExtractor();

		const std::vector<SchemaPin>& inputs = extractor->inputs();

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

		const std::vector<SchemaPin>& outputs = extractor->outputs();

		bool result = true;

		for(const SchemaPin& outPin : outputs)
		{
			UalSignal* busChildSignal = busSignal->getBusChildSignal(outPin.caption());

			if (busChildSignal == nullptr)
			{
				assert(false);					// busChildSignal with ID == outPin.caption() is not found, why?
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			BusSignal inbusSignal = bus->signalByID(outPin.caption());

			if (inbusSignal.conversionRequired() == true)
			{
				busChildSignal->setFrombusConversionRequired(true);

				// flag FrombusConversionRequired will set to FALSE after conversion code generation
			}

			// link connected signals to UalSignal
			//
			result &= linkConnectedItems(ualItem, outPin, busChildSignal);

			m_ualSignals.appendRefPin(ualItem, outPin.guid(), busChildSignal);
		}

		return result;
	}


	bool ModuleLogicCompiler::linkConnectedItems(UalItem* srcUalItem, const SchemaPin& outPin, UalSignal* ualSignal)
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
			UalItem* destUalItem = getValueOrNullptr(m_pinParent, inPinUuid);

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
			// Incompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(srcItem->guid(), srcItem->label(), signalItem->guid(), signalItem->label(), signalItem->schemaID());
			return false;
		}

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

		const std::vector<SchemaPin>& outputs = signalItem->outputs();

		if (outputs.size() > 1)
		{
			LOG_INTERNAL_ERROR(m_log);				// signal cannot have more then 1 output
			return false;
		}

		if (outputs.size() == 1)
		{
			const SchemaPin& output = outputs[0];

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

		UalAfb* ualAfb = m_ualAfbs.getAfb(destItem->guid());

		if (ualAfb == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		AfbSignal inSignal;

		bool result = ualAfb->getAfbSignalByPinUuid(inPinUuid, &inSignal);

		if (result == false)
		{
			return false;
		}

		if (ualAfb->isSetFlagsItem() == true && inSignal.caption() == Afb::IN_PIN_CAPTION)
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

		UalAfb* ualAfb = m_ualAfbs.getAfb(setFlagsItem->guid());

		LOG_IF_NULLPTR_RETURN_FALSE(ualAfb, m_log);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(ualAfb->isSetFlagsItem(), m_log);

		const SchemaPin* inPin = ualAfb->getPin(inPinUuid);

		LOG_IF_NULLPTR_RETURN_FALSE(inPin, m_log);

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(inPin->caption() == Afb::IN_PIN_CAPTION, log());

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
		const std::vector<SchemaPin>& outputs = ualAfb->outputs();

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(outputs.size() == 1, log());

		const SchemaPin& outPin = outputs[0];

		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(outPin.caption() == Afb::OUT_PIN_CAPTION, log());

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

		const SchemaBusComposer* busComposer = busComposerItem->schemaBusComposer();

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

		const SchemaPin& inPin = busComposer->input(inPinUuid);

		QString busSignalID = inPin.caption();

		const BusSignal& busSignal = bus->signalByID(busSignalID);

		if (busSignal.isValid() == false)
		{
			LOG_INTERNAL_ERROR(m_log);				// bus signal with busSignalID is not found,
			return false;
		}

		if (ualSignal->isCompatible(bus, busSignal, log()) == false)
		{
			// Incompatible signals connection (Logic schema '%1').
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

		const SchemaBusExtractor* busExtractor = busExtractorItem->schemaBusExtractor();

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
			// Incompatible signals connection (Logic schema '%1').
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

		const SchemaLoopbackTarget* target = loopbackTargetItem->schemaLoopbackTarget();

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

		const std::vector<SchemaPin>& outputs = loopbackTargetItem->outputs();

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

			const UalAfb* afb = m_ualAfbs.getAfb(ualItem->guid());

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
		result &= processAcquiredOptoSignalsValidity();
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

			auto it = m_equipmentSignals.find(calcHash(validitySignalEquipmentID));

			if (it == m_equipmentSignals.end())
			{
				// Linked validity app signal with EquipmentID %1 is not found (input signal %2).
				//
				m_log->errALC5155(validitySignalEquipmentID, s->appSignalID());
				result = false;
				continue;
			}

			AppSignal* linkedValiditySignal = it->second;

			if (linkedValiditySignal->isInput() == false ||
				linkedValiditySignal->isDiscrete() == false)
			{
				// Linked validity signal %1 should have Discrete Input type (input signal %2).
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

	bool ModuleLogicCompiler::processAcquiredOptoSignalsValidity()
	{
		bool result = true;

		for(const UalSignal* optoSignal : m_ualSignals)
		{
			TEST_PTR_CONTINUE(optoSignal);

			if (!(optoSignal->isOptoSignal() == true && optoSignal->isAcquired() == true))
			{
				continue;
			}

			Hardware::OptoPortShared optoPort = m_optoModuleStorage->getLmAssociatedOptoPort(m_lm->equipmentIdTemplate(),
																				   optoSignal->optoConnectionID());
			if(optoPort == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			auto it = m_optoPortValiditySignal.find(calcHash(optoPort->equipmentID()));

			if (it == m_optoPortValiditySignal.end())
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("OptoPort %1 validity signal should be exists").arg(optoPort->equipmentID()));
				result = false;
				continue;
			}

			const UalSignal* validitySignal = it->second;

			if (validitySignal == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (validitySignal->isInput() == false ||
				validitySignal->isDiscrete() == false)
			{
				// Linked validity signal %1 shoud have Discrete Input type (input signal %2).
				//
				m_log->errALC5156(validitySignal->appSignalID(), optoSignal->appSignalID());
				result = false;
				continue;
			}

			const SchemaReceiver* receiver = optoSignal->ualReceiver();

			TEST_PTR_CONTINUE(receiver);

			QStringList nearestSignalsIDs;

			getNearestOutSignalIDs(receiver->outputPin(), &nearestSignalsIDs);

			for(const QString& signalID : nearestSignalsIDs)
			{
				bool res = appendFlagToSignal(signalID,
											E::AppSignalStateFlagType::Validity,
											validitySignal->appSignalID(),
											nullptr);
				result &= res;
			}
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

			if (ualItem->assignFlags(log()) == false)
			{
				continue;
			}

			UalAfb* simLockItem = m_ualAfbs.getAfb(ualItem->guid());

			if (simLockItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Q_ASSERT(simLockItem->isSimLockItem() == true);

			const SchemaPin* outPin = simLockItem->getPin(Afb::OUT_PIN_CAPTION);

			if (outPin == nullptr)
			{
				// Pin with caption %1 is not found in schema item (Logic schema %2).
				//
				m_log->errALC5106(Afb::OUT_PIN_CAPTION, simLockItem->guid(), simLockItem->schemaID());
				result = false;
				continue;
			}

			QStringList nearestSignalIDs;

			getNearestOutSignalIDs(*outPin, &nearestSignalIDs);

			for(const QString& signalWithFlagID : nearestSignalIDs)
			{
				result &= appendFlagToSignalFromPin(ualItem, Afb::SIMLOCK_SIM_PIN_CAPTION, true, E::AppSignalStateFlagType::Simulated, signalWithFlagID, nullptr);
				result &= appendFlagToSignalFromPin(ualItem, Afb::SIMLOCK_BLOCK_PIN_CAPTION, true, E::AppSignalStateFlagType::Blocked, signalWithFlagID, nullptr);
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
			Afb::IN_1_PIN_CAPTION,
			Afb::IN_2_PIN_CAPTION,
			Afb::IN_3_PIN_CAPTION,
			Afb::IN_4_PIN_CAPTION
		};

		const QString outPinCaptions[MISMATCH_MAX_PIN_COUNT] =
		{
			Afb::OUT_1_PIN_CAPTION,
			Afb::OUT_2_PIN_CAPTION,
			Afb::OUT_3_PIN_CAPTION,
			Afb::OUT_4_PIN_CAPTION
		};

		bool result = true;

		for(UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isMismatchItem() == false)
			{
				continue;
			}

			if (ualItem->assignFlags(log()) == false)
			{
				continue;
			}

			UalAfb* mismatchItem = m_ualAfbs.getAfb(ualItem->guid());

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

				const SchemaPin* inPin = mismatchItem->getPin(inPinCaption);

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

			UalAfb* setFlagsItem = m_ualAfbs.getAfb(ualItem->guid());

			if (setFlagsItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Q_ASSERT(setFlagsItem->isSetFlagsItem() == true);

			const SchemaPin* outPin = setFlagsItem->getPin(Afb::OUT_PIN_CAPTION);

			if (outPin == nullptr)
			{
				// Pin with caption %1 is not found in schema item (Logic schema %2).
				//
				m_log->errALC5106(Afb::IN_PIN_CAPTION, setFlagsItem->guid(), setFlagsItem->schemaID());
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

				result &= appendFlagToSignalFromPin(ualItem, Afb::VALIDITY_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Validity, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, Afb::SIMULATED_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Simulated, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, Afb::BLOCKED_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Blocked, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, Afb::MISMATCH_PIN_CAPTION, false,
													E::AppSignalStateFlagType::Mismatch, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, Afb::HIGH_LIMIT_PIN_CAPTION, false,
													E::AppSignalStateFlagType::AboveHighLimit, signalWithFlagsID, &flagIsSet);

				result &= appendFlagToSignalFromPin(ualItem, Afb::LOW_LIMIT_PIN_CAPTION, false,
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

		const SchemaPin* pin = ualItem->getPin(pinCaption);

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

		bool res = true;

		if (currentStateFlagSignalID.isEmpty() == true)
		{
			res = signalWithFlag->addFlagSignalID(flagType, flagSignalID);
			Q_ASSERT(res == true);
		}
		else
		{
			if (currentStateFlagSignalID != flagSignalID)
			{
				// Duplicate assigning of signal %1 to flag %2 of signal %3. Signal %4 already assigned to this flag.
				//
				m_log->errALC5168(	flagSignalID,
									E::valueToString<E::AppSignalStateFlagType>(flagType),
									signalWithFlagID,
									currentStateFlagSignalID,
									(setFlagsItem == nullptr ? QUuid() : setFlagsItem->guid()),
									(setFlagsItem == nullptr ? EMPTY_STR : setFlagsItem->schemaID()));
				return false;
			}

			// this signal already assigned to flag
		}

		m_signalsWithFlagsIDs.insert(signalWithFlagID);

		return res;
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

		m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
								getInfoFileName("swf"), file);		// Signals With Flags
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

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedSignal(const SchemaPin& outPin, const AfbSignal& outAfbSignal, const QString& busTypeID)
	{
		const std::vector<QUuid>& connectedPinsUuids = outPin.associatedIOs();

		QStringList connectedLoopbacks;

		for(QUuid inPinUuid : connectedPinsUuids)
		{
			UalItem* connectedItem = getValueOrNullptr(m_pinParent, inPinUuid);

			if (connectedItem == nullptr)
			{
				assert(false);
				continue;
			}

			if (connectedItem->isLoopbackSource() == true)
			{
				const SchemaLoopbackSource* source = connectedItem->schemaLoopbackSource();

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

				const SchemaPin* inPin = connectedItem->getPin(inPinUuid);

				if (inPin == nullptr)
				{
					assert(false);
					continue;
				}

				if (inPin->caption() != Afb::IN_PIN_CAPTION)
				{
					continue;
				}

				// yes, this is 'in' pin of set_flags item

				const SchemaPin* connectedItemOutPin = connectedItem->getPin(Afb::OUT_PIN_CAPTION);

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

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedSignal(const SchemaPin& outPin, const AfbSignal& outAfbSignal)
	{
		return getCompatibleConnectedSignal(outPin, outAfbSignal, EMPTY_STR);
	}

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedSignal(const SchemaPin& outPin, const AppSignal& s)
	{
		AfbSignal dummySignal;

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

	AppSignal* ModuleLogicCompiler::getCompatibleConnectedBusSignal(const SchemaPin& outPin, const QString& busTypeID)
	{
		AfbSignal dummyBusSignal;

		dummyBusSignal.setType(E::SignalType::Bus);

		return getCompatibleConnectedSignal(outPin, dummyBusSignal, busTypeID);
	}

	bool ModuleLogicCompiler::isCompatible(const AfbSignal& outAfbSignal, const QString& busTypeID, const AppSignal* s)
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

	bool ModuleLogicCompiler::isConnectedToTerminatorOnly(const SchemaPin& outPin) const
	{
		const std::vector<QUuid>& connectedPinsUuids = outPin.associatedIOs();

		for(QUuid inPinUuid : connectedPinsUuids)
		{
			const UalItem* connectedItem = getValueOrNullptr(m_pinParent, inPinUuid);

			TEST_PTR_CONTINUE(connectedItem);

			if (connectedItem->isTerminator() == false)
			{
				return false;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::isOutConnectedToTerminatorOnly(const UalAfb* ualItem) const
	{
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		const auto& outs = ualItem->outputs();

		for(const SchemaPin& out : outs)
		{
			if (isConnectedToTerminatorOnly(out) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::isConnectedToLoopback(const SchemaPin& inPin, std::shared_ptr<Loopback>* loopback)
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
		// 1) try to determine BusType by input UalSignals
		// 2) try to determine BusType by bus signal connected to output

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

		for(const SchemaPin& inPin : ualAfb->inputs())
		{
			AfbSignal afbSignal;

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

				// Incompatible signals connection (Logic schema '%1').
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

		const std::vector<SchemaPin> outputs = ualAfb->outputs();

		for(const SchemaPin& outPin : outputs)
		{
			AfbSignal afbSignal;

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

	std::optional<int> ModuleLogicCompiler::getOutPinExpectedReadCount(const SchemaPin& outPin)
	{
		Q_ASSERT(outPin.IsOutput() == true);

		// calculate expected read count to place signal in heap
		// if signal can't be placed in heap uninitialized value of std::optional<int> will be returned
		//
		int readCount = 0;

		for(const QUuid& inPinGuid : outPin.associatedIOs())
		{
			UalItem* inPinParentItem = getValueOrNullptr(m_pinParent, inPinGuid);

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
		const SchemaPin* inPin = ualItem->getPin(inPinGuid);

		if (inPin == nullptr)
		{
			Q_ASSERT(false);
			return 0;
		}

		Q_ASSERT(inPin->IsInput() == true);

		if (inPin->caption() != Afb::IN_PIN_CAPTION)
		{
			return 0;			// input pins of set_falgs except "in" is not produce reads
		}

		// this is "in" pin of set_flags AFB
		// calculation of all reads of pins connected to "out" pin is required
		//
		const std::vector<SchemaPin>& outs = ualItem->outputs();

		if (outs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return 0;
		}

		if (outs[0].caption() != Afb::OUT_PIN_CAPTION)
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

	bool ModuleLogicCompiler::checkPinsConnectedToSignal(const std::vector<SchemaPin>& pins, bool shouldConnectToSameSignal, UalSignal** sameSignalPtr)
	{
		if (sameSignalPtr == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		for(const SchemaPin& pin : pins)
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

		for(const SchemaPin& inPin : ualItem->inputs())
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

		for(const SchemaPin& outPin : ualItem->outputs())
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
		UalAfb* destAppFb = m_ualAfbs.getAfb(destAppItem->guid());

		if (destAppFb == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		AfbSignal destAfbSignal;

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

		const SchemaBusExtractor* busExtractor = destAppItem->schemaBusExtractor();

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

		// common code for IPEN (Fotip::V1) and Fotip::V2 tuning protocols and data
		//
		bool tuningLanExists = false;
		bool tuningEnabled = false;

		bool result = getTuningSettings(&tuningLanExists, &tuningEnabled);

		if (result == false)
		{
			return false;
		}

		if (tuningLanExists == false)
		{
			return true;
		}

		Tuning::TuningDataShared tuningData =
				std::make_shared<Tuning::TuningData>(lmEquipmentID(),
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
			return false;
		}

		if (tuningData->getSignalsCount() == 0)
		{
			if (tuningEnabled == true)
			{
				// Tuning is enabled for module %1 but tunable signals are not found.
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
			return false;
		}

		tuningData->calcTuningDataUID(lmEquipmentID());

		m_rupTuningDataUID = tuningData->rupTuningDataUID();
		m_fotipTuningDataUID = tuningData->fotipTuningDataUID();

		m_tuningData = tuningData;
		m_tuningDataStorage->appendTuningData(lmEquipmentID(), tuningData);

		result &= DeviceHelper::setUIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											EquipmentPropNames::TUNING_LAN_DATA_UID,
											m_rupTuningDataUID,
											m_log);
		return result;
	}

	bool ModuleLogicCompiler::buildTuningSignalsLists(Tuning::TuningDataShared tuningData)
	{
		TEST_PTR_RETURN_FALSE(tuningData);

		tuningData->clearSignalLists();

		bool result = true;

		for(const auto& [hash, signal] : m_moduleSignals)
		{
			TEST_PTR_CONTINUE(signal);

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

	bool ModuleLogicCompiler::getTuningSettings(bool* tuningLanExists, bool* tuningEnabled)
	{
		TEST_PTR_LOG_RETURN_FALSE(m_lmDescription, m_log);
		TEST_PTR_LOG_RETURN_FALSE(tuningLanExists, m_log);
		TEST_PTR_LOG_RETURN_FALSE(tuningEnabled, m_log);

		*tuningLanExists = false;
		*tuningEnabled = false;

		const LmDescription::Lan& lan = m_lmDescription->lan();

		// search ethernet controller(s) that provide tuning
		//
		std::vector<int> tuningLans;

		for(const LmDescription::LanController& lanController : lan.m_lanControllers)
		{
			if (lanController.isProvideTuning() == true)
			{
				tuningLans.push_back(lanController.m_place);
			}
		}

		if (tuningLans.size() == 0)
		{
			// no tuning controllers found
			//
			return true;
		}

		*tuningLanExists = true;

		for(int tuningLanNo : tuningLans)
		{
			QString suffix = LanControllerInfoHelper::getLanControllerSuffix(tuningLanNo);

			Hardware::DeviceController* adapter = DeviceHelper::getChildControllerBySuffix(m_lm, suffix, m_log);

			if (adapter == nullptr)
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Ethernet controller with suffix %1 is not found in module %2").
											arg(suffix).arg(m_lm->equipmentIdTemplate()));
				return false;
			}

			if (DeviceHelper::isPropertyExists(adapter, EquipmentPropNames::TUNING_ENABLE) == false)
			{
				continue;
			}

			bool tunEnabled = false;

			bool res = DeviceHelper::getBoolProperty(adapter, EquipmentPropNames::TUNING_ENABLE,
													 &tunEnabled, m_log);
			if (res == false)
			{
				return false;
			}

			*tuningEnabled |= tunEnabled;
		}

		return true;
	}

	bool ModuleLogicCompiler::disposeSignalsInHeap()
	{
		m_ualSignals.disposeSignalsInHeaps(m_signalsWithFlagsAndFlagSignals);

		return true;
	}

	bool ModuleLogicCompiler::createSignalLists()
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_lm, m_log);

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

		result &= createDiscreteInvertedOutputSignalsList();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		RETURN_IF_FALSE(listsUniquenessCheck());

		result &= setSignalsCalculatedAttributes();

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		// Discretes
		//
		sortSignalList(m_acquiredDiscreteInputSignals);
		sortSignalList(m_acquiredDiscreteInvertedInputSignals);
		sortSignalList(m_acquiredDiscreteStrictOutputSignals);
		sortSignalList(m_acquiredDiscreteInternalSignals);
		sortSignalList(m_acquiredDiscreteOptoSignals);
		sortSignalList(m_acquiredDiscreteBusChildSignals);
		sortSignalList(m_acquiredDiscreteConstSignals);
		//	m_acquiredDiscreteTuningSignals;		sorting not required

		// Analogs
		//
		sortSignalList(m_nonAcquiredDiscreteInputSignals);
		sortSignalList(m_nonAcquiredDiscreteInvertedInputSignals);
		sortSignalList(m_nonAcquiredDiscreteStrictOutputSignals);
		sortSignalList(m_nonAcquiredDiscreteInternalSignals);

		sortSignalList(m_acquiredAnalogInputSignals);
		sortSignalList(m_acquiredAnalogStrictOutputSignals);
		sortSignalList(m_acquiredAnalogInternalSignals);
		sortSignalList(m_acquiredAnalogOptoSignals);
		sortSignalList(m_acquiredAnalogBusChildSignals);
		//	m_acquiredAnalogTuningSignals;			sorting not required

		sortSignalList(m_nonAcquiredAnalogInputSignals);
		sortSignalList(m_nonAcquiredAnalogStrictOutputSignals);
		sortSignalList(m_nonAcquiredAnalogInternalSignals);

		// Buses
		//
		sortSignalList(m_acquiredInputBuses);
		sortSignalList(m_acquiredOutputBuses);
		sortSignalList(m_acquiredInternalBuses);

		sortSignalList(m_acquiredOptoBuses);		// To DO sorting - group by OptoPortID, and next by addr in port buf for sequential move optimization

		sortSignalList(m_acquiredBusChildBuses);

		sortSignalList(m_nonAcquiredOutputBuses);
		sortSignalList(m_nonAcquiredInternalBuses);

		sortSignalList(m_discreteInvertedOutputSignals);

		return result;
	}

	bool ModuleLogicCompiler::createAcquiredDiscreteInputSignalsList()
	{
		m_acquiredDiscreteInputSignals.clear();
		m_acquiredDiscreteInvertedInputSignals.clear();

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
				if (s->invertSignal() == true)
				{
					m_acquiredDiscreteInvertedInputSignals.append(s);
				}
				else
				{
					m_acquiredDiscreteInputSignals.append(s);
				}
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
		m_nonAcquiredDiscreteInvertedInputSignals.clear();

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
				if (s->invertSignal() == true)
				{
					m_nonAcquiredDiscreteInvertedInputSignals.append(s);
				}
				else
				{
					m_nonAcquiredDiscreteInputSignals.append(s);
				}
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

	bool ModuleLogicCompiler::createDiscreteInvertedOutputSignalsList()
	{
		m_discreteInvertedOutputSignals.clear();

		//	list include signals that:
		//
		//	+ discrete
		//	+ output
		//	+ invertSignal

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isDiscrete() == true &&
				s->isOutput() == true &&
				s->invertSignal() == true)
			{
				m_discreteInvertedOutputSignals.append(s);
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
		//  + bus child == false
		//  + bus_child == true && FromBusConversionRequierd == true
		//	+ used in UAL || is a SerialRx signal (condition: m_optoModuleStorage->isSerialRxSignalExists(lmEquipmentID(), s->appSignalID()) == true))

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				(s->isBusChild() == false || (s->isBusChild() == true && s->isFrombusConversionRequired() == true)) &&
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
				s->isFrombusConversionRequired() == false &&	// if isFromBusConversionRequired() == true,
																// this signal acquired as Internal Analog (after frombus conversion)
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

		qsizetype count = m_analogOutputSignalsToConversion.count();

		for(qsizetype i = 0; i < count - 1; i++)
		{
			for(qsizetype k = i + 1; k < count; k++)
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
		//	+ bus child == false
		//  + bus_child == true && FromBusConversionRequired == true
		//	+ auto analog internal signals (auto generated in m_appSignals)

		for(UalSignal* s : m_ualSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isConst() == false &&
				(s->isBusChild() == false || (s->isBusChild() == true && s->isFrombusConversionRequired() == true )) &&
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

		for(auto& [hash, s] : m_moduleSignals)
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

			case E::SignalInOutType::SoftwareCalculated:
				s->setLmRamAccess(E::LogicModuleRamAccess::Undefined);
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

		qsizetype count = vPortIDs.count();

		for(qsizetype i = 0; i < count - 1; i++)
		{
			for(qsizetype k = i + 1; k < count; k++)
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

		std::set<UalSignal*> presentSignalsMap;

		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteInputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteInvertedInputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteStrictOutputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteInternalSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteOptoSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteBusChildSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredDiscreteTuningSignals);

		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredDiscreteInvertedInputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredDiscreteStrictOutputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredDiscreteInternalSignals);

		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredAnalogInputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredAnalogStrictOutputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredAnalogInternalSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredAnalogOptoSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredAnalogBusChildSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredAnalogTuningSignals);

		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredAnalogInputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredAnalogStrictOutputSignals);
		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredAnalogInternalSignals);

		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredInputBuses);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredOutputBuses);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredInternalBuses);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredBusChildBuses);
		result &= listUniquenessCheck(&presentSignalsMap, m_acquiredOptoBuses);

		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredOutputBuses);
		result &= listUniquenessCheck(&presentSignalsMap, m_nonAcquiredInternalBuses);

		return result;
	}

	bool ModuleLogicCompiler::listUniquenessCheck(std::set<UalSignal*>* presentSignalsSet, const QVector<UalSignal*>& signalList) const
	{
		TEST_PTR_RETURN_FALSE(presentSignalsSet);

		bool result = true;

		for(UalSignal* s : signalList)
		{
			if (presentSignalsSet->contains(s) == true)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal %1 present in several signals processing lists!").
														arg(s->appSignalID()));
				result = false;
				continue;
			}

			presentSignalsSet->insert(s);
		}

		return result;
	}

	void ModuleLogicCompiler::sortSignalList(QVector<UalSignal*>& signalList)
	{
		std::sort(signalList.begin(), signalList.end(),
				  [] (UalSignal* a, UalSignal* b)
					{
						return a->appSignalID() < b->appSignalID();
					});
	}

	void ModuleLogicCompiler::sortSignalList(QVector<const UalSignal*>& signalList)
	{
		std::sort(signalList.begin(), signalList.end(),
				  [] (const UalSignal* a, const UalSignal* b)
					{
						return a->appSignalID() < b->appSignalID();
					});
	}

	void ModuleLogicCompiler::sortSignalListByUalAddr(QVector<UalSignal*>& signalList)
	{
		std::sort(signalList.begin(), signalList.end(),
				  [] (UalSignal* a, UalSignal* b)
					{
						return a->ualAddr() < b->ualAddr();
					});
	}

	bool ModuleLogicCompiler::disposeSignalsInMemory()
	{
		bool result = false;

		do
		{
			if (m_lm->isLogicModule() == true)
			{
				// platform-based LM processing
				//
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

				if (disposeNonAcquiredDiscreteInvertedInputSignals() == false) break;

				if (disposeAnalogAndBusSignalsHeap() == false) break;

				if (setSignalsRegValidityAddr() == false) break;

				result = true;
				break;
			}

			if (m_lm->isNonPlatformAppDataSourceModule())
			{
				// Non platform chassis processing
				//
				if (disposeNonPlatformAppSignalsInRegBuf() == false) break;

				result = true;
				break;
			}
		}
		while(false);

		return result;
	}

	bool ModuleLogicCompiler::calculateIoSignalsAddresses()
	{
		// calculation m_ioBufAddr of in/out signals
		//
		bool result = true;

		for(AppSignal* ioSignal : m_ioSignals)
		{
			TEST_PTR_CONTINUE(ioSignal);

			Module module;
			Hardware::DeviceAppSignal* deviceAppSignal = nullptr;

			bool res = getIoSignalModule(*ioSignal, &module, &deviceAppSignal);

			if (res == false)
			{
				result = false;
				continue;
			}

			TEST_PTR_CONTINUE(deviceAppSignal);

			Address16 ioBufAddr(deviceAppSignal->valueOffset(), deviceAppSignal->valueBit());

			ioBufAddr.addWord(module.moduleDataOffset);

			switch(deviceAppSignal->memoryArea())
			{
			case E::MemoryArea::ApplicationData:

				switch(ioSignal->inOutType())
				{
				case E::SignalInOutType::Input:
					ioBufAddr.addWord(module.txAppDataOffset);
					ioSignal->setIoBufAddr(ioBufAddr);
					break;

				case E::SignalInOutType::Output:
					ioBufAddr.addWord(module.rxAppDataOffset);
					ioSignal->setIoBufAddr(ioBufAddr);
					break;

				case E::SignalInOutType::Internal:
					// Internal application signal %1 cannot be linked to equipment input/output signal %2.
					//
					log()->errALC5171(ioSignal->appSignalID(), ioSignal->equipmentID());
					result = false;
					break;

				case E::SignalInOutType::SoftwareCalculated:
					ioBufAddr.clear();
					break;

				default:
					assert(false);
				}
				break;

			case E::MemoryArea::DiagnosticsData:

				switch(ioSignal->inOutType())
				{
				case E::SignalInOutType::Input:
					ioBufAddr.addWord(module.txDiagDataOffset);
					ioSignal->setIoBufAddr(ioBufAddr);
					break;

				case E::SignalInOutType::Output:
					assert(false);							// output diagnostics signals is not exist
					break;

				case E::SignalInOutType::Internal:
					// Internal application signal %1 cannot be linked to equipment input/output signal %2.
					//
					log()->errALC5171(ioSignal->appSignalID(), ioSignal->equipmentID());
					result = false;
					break;

				case E::SignalInOutType::SoftwareCalculated:
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

	bool ModuleLogicCompiler::getIoSignalModule(const AppSignal& ioSignal, Module* module,
												Hardware::DeviceAppSignal** deviceAppSignal) const
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(module, m_log);
		TEST_PTR_LOG_RETURN_FALSE(deviceAppSignal, m_log);

		if (ioSignal.isInput() == false &&
			ioSignal.isOutput() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		// retrieve linked device
		//
		Hardware::DeviceObject* device = m_equipmentSet->deviceObject(ioSignal.equipmentID()).get();

		if (device == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't find DeviceObject with equipmentID %1").
												arg(ioSignal.equipmentID()));
			return false;
		}

		if (device->isAppSignal() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("DeviceObject %1 is not a DeviceAppSignal").
												arg(ioSignal.equipmentID()));
			return false;
		}

		Hardware::DeviceAppSignal* devAppSignal = device->toAppSignal().get();

		if (devAppSignal == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		*deviceAppSignal = devAppSignal;

		// retrieve associated module
		//
		const Hardware::DeviceModule* deviceModule = devAppSignal->getParentModule();

		if (deviceModule == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't find parent DeviceModule for DeviceAppSignal %1").
												arg(devAppSignal->equipmentIdTemplate()));
			return false;
		}

		int modulePlace = deviceModule->place();

		if (modulePlace < 0)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("DeviceModule %1 has wrong place %2").
												arg(deviceModule->equipmentIdTemplate()).
												arg(modulePlace));
			return false;
		}

		auto it = m_modules.find(modulePlace);

		if (it == m_modules.end())
		{
			for(const auto& p : m_modules)
			{
				const Module& m = p.second;
				LOG_MESSAGE(m_log, QString("Place %1 module %2").arg(p.first).arg(m.device->equipmentIdTemplate()));
			}

			LOG_INTERNAL_ERROR_MSG(m_log, QString("Module for IO signal %1 not found").arg(ioSignal.appSignalID()));
			return false;
		}

		*module = it->second;

		return true;
	}

	bool ModuleLogicCompiler::disposeTunableSignalsUalAddresses()
	{
		if (m_tuningData == nullptr)
		{
			return true;			// no tuning data, it is ok
		}

		bool result = true;

		QVector<AppSignal*> tunigableSignals;

		m_tuningData->getSignals(&tunigableSignals);

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
		for(const AppSignal* ioSignal : m_ioSignals)
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

			if (ualSignal->isDiscrete() && ualSignal->invertSignal())
			{
				continue;		// UalAddr for inverted signals will be set later
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

	bool ModuleLogicCompiler::disposeNonAcquiredDiscreteInvertedInputSignals()
	{
		bool result = true;

		result &= m_memoryMap.appendNonAcquiredDiscreteInvertedInputSignals(m_nonAcquiredDiscreteInvertedInputSignals);

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

		return m_memoryMap.setAcquiredRawDataSize(0);
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
		result &= m_memoryMap.appendAcquiredDiscreteInvertedInputSignalsInRegBuf(m_acquiredDiscreteInvertedInputSignals);
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

	bool ModuleLogicCompiler::setSignalsRegValidityAddr()
	{
		bool result = true;

		for(const QString& signalWithFlagID : m_signalsWithFlagsIDs)
		{
			AppSignal* signalWithFlag = m_signals->getSignal(signalWithFlagID);

			if (signalWithFlag == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (signalWithFlag->isAcquired() == false)
			{
				continue;
			}

			QString validitySignalID = signalWithFlag->getFlagSignalID(E::AppSignalStateFlagType::Validity);

			if (validitySignalID.isEmpty() == true)
			{
				continue;			// it is Ok, signal has no validity signal
			}

			AppSignal* validitySignal = m_signals->getSignal(validitySignalID);

			if (validitySignal == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			signalWithFlag->setRegValidityAddr(validitySignal->regValueAddr());
		}

		return result;
	}

	bool ModuleLogicCompiler::disposeNonPlatformAppSignalsInRegBuf()
	{
		TEST_PTR_RETURN_FALSE(m_lmDescription);

		// calculation regBufAddr of non-platform chassis I/O signals
		//
		bool result = true;

		int bvbAppDataOffset = m_lmDescription->memory().m_appDataOffset;

		std::map<QString, Address16> validitySignalAddr;		// validity signal EquipmentID => validity signal regValueAddr

		for(AppSignal* ioSignal : m_ioSignals)
		{
			TEST_PTR_CONTINUE(ioSignal);

			Module module;
			Hardware::DeviceAppSignal* deviceSignal = nullptr;

			bool res = getIoSignalModule(*ioSignal, &module, &deviceSignal);

			if (res == false)
			{
				result = false;
				continue;
			}

			TEST_PTR_CONTINUE(deviceSignal);
			TEST_PTR_CONTINUE(module.device);

			Address16 regValueAddr(deviceSignal->valueOffset(), deviceSignal->valueBit());

			regValueAddr.addWord(module.appRegDataOffset);

			Address16 regValidityAddr;

			QString validitySignalID = deviceSignal->validitySignalId();

			if (validitySignalID.isEmpty() == false)
			{
				auto it = validitySignalAddr.find(validitySignalID);

				if (it != validitySignalAddr.end())
				{
					regValidityAddr = it->second;
				}
				else
				{
					auto it2 = m_equipmentSignals.find(calcHash(validitySignalID));

					if (it2 != m_equipmentSignals.end())
					{
						const AppSignal* validityAppSignal = it2->second;

						TEST_PTR_CONTINUE(validityAppSignal);

						Module valModule;
						Hardware::DeviceAppSignal* validityDeviceSignal = nullptr;

						res = getIoSignalModule(*validityAppSignal, &valModule, &validityDeviceSignal);

						if (res == false)
						{
							result = false;
							continue;
						}

						TEST_PTR_CONTINUE(validityDeviceSignal);

						Address16 valSignalRegValueAddr(validityDeviceSignal->valueOffset(), validityDeviceSignal->valueBit());

						valSignalRegValueAddr.addWord(valModule.appRegDataOffset);

						validitySignalAddr.emplace(validitySignalID, valSignalRegValueAddr);

						regValidityAddr = valSignalRegValueAddr;
					}
					else
					{
						LOG_INTERNAL_ERROR_MSG(m_log, QString("AppSignal with EquipmentID %1 is not fond.").arg(validitySignalID));
						result = false;
						continue;
					}
				}
			}

			//

			UalSignal* ualIoSignal = m_ualSignals.get(ioSignal);

			if (ualIoSignal != nullptr)
			{
				// ioSignal->setRegValueAddr(regValueAddr) doing inside next call
				//
				ualIoSignal->setRegValueAddr(regValueAddr);

				Address16 ualAddr(regValueAddr);
				ualAddr.addWord(bvbAppDataOffset);

				ualIoSignal->setUalAddr(ualAddr);
			}
			else
			{
				Q_ASSERT(false);
			}

			ioSignal->setRegValidityAddr(regValidityAddr);

			auto it = m_nonPlatformRegSignals.find(ioSignal->regValueAddr());

			if (it == m_nonPlatformRegSignals.end())
			{
				auto [newIt, b] = m_nonPlatformRegSignals.emplace(ioSignal->regValueAddr(), std::vector<NonPlatformAppSignal>{});
				it = newIt;
			}

			it->second.emplace_back(module.place,
									module.device->caption(),
									ioSignal);
		}

		int acquiredRawDataSizeW = 0;

		if (m_nonPlatformRegSignals.empty() == false)
		{
			Address16 ioSignalMaxRegAddr = m_nonPlatformRegSignals.rbegin()->first;

			int maxRegDataSize = m_lmDescription->memory().m_moduleCount * m_lmDescription->memory().m_moduleDataSize;

			if (ioSignalMaxRegAddr.offset() >= maxRegDataSize)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, "Non-platform AppSignal regAddr out of range");
				return false;
			}

			acquiredRawDataSizeW = ROUND_TO(ioSignalMaxRegAddr.offset(), m_lmDescription->memory().m_moduleDataSize);
		}

		bool res = m_memoryMap.setAcquiredRawDataSize(acquiredRawDataSizeW);

		return res;
	}

	bool ModuleLogicCompiler::appendAfbsForInOutSignalsConversion()
	{
		if (noCodeGenRequired())
		{
			return true;
		}

		if (findFbsForInOutSignalsConversion() == false)
		{
			return false;
		}

		auto assignUalAfbToFbConv = [this](const UalAfb* ualAfb) -> bool
									{
										TEST_PTR_RETURN_FALSE(ualAfb);

										auto it = this->m_fbConv.find(ualAfb->caption());

										if (it == this->m_fbConv.end())
										{
											LOG_INTERNAL_ERROR(m_log);
											return false;
										}

										it->second.ualAfbs.insert(ualAfb);

										return true;
									};

		bool result = true;

		// append FBs for analog input signals conversion
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
				UalAfb* ualAfb = createUalAfb(appItem);

				RETURN_IF_FALSE(assignUalAfbToFbConv(ualAfb));

				m_inOutSignalsToScalAppFbMap.insert(s->appSignalID(), ualAfb);
			}
		}

		RETURN_IF_FALSE(result);

		// append FBs for analog output signals conversion
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
				UalAfb* ualAfb = createUalAfb(appItem);

				RETURN_IF_FALSE(assignUalAfbToFbConv(ualAfb));

				m_inOutSignalsToScalAppFbMap.insert(s->appSignalID(), ualAfb);
			}
		}

		RETURN_IF_FALSE(result);

		// append FBs for discrete in/output signals conversion
		//
		if ((m_acquiredDiscreteInvertedInputSignals.size() +
			m_nonAcquiredDiscreteInvertedInputSignals.size() +
			m_discreteInvertedOutputSignals.size()) > 0)
		{
			static const std::vector<QString> afbsNotCaptions =
			{
				Afb::AFB_BUS_NOT,
				Afb::AFB_NOT
			};

			for(const QString& afbCaption : afbsNotCaptions)
			{
				UalAfb* ualAfb = nullptr;

				auto it = this->m_fbConv.find(afbCaption);

				if (it == this->m_fbConv.end())
				{
					LOG_INTERNAL_ERROR(this->m_log);
					result = false;
					continue;
				}

				FbConv& fb = it->second;

				QString errorMsg;
				UalItem appItem;

				appItem.init(fb.pointer, errorMsg);

				if (errorMsg.isEmpty() == false)
				{
					LOG_INTERNAL_ERROR_MSG(this->m_log, errorMsg);
					result = false;
					continue;
				}

				ualAfb = createUalAfb(appItem);

				if (ualAfb != nullptr)
				{
					fb.ualAfbs.insert(ualAfb);
				}
				else
				{
					LOG_INTERNAL_ERROR(this->m_log);
					result = false;
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::findFbsForInOutSignalsConversion()
	{
		Q_ASSERT(m_lm->isLogicModule() == true);

		bool result = true;

		static const QString fbConvCaption[] =
		{
			// for analog input signals conversion
			//
			Afb::SCALE_UI16_FP32,
			Afb::SCALE_UI16_SI32,

			// for analog output signals conversion
			//
			Afb::SCALE_FP32_UI16,
			Afb::SCALE_SI32_UI16,

			// for analog int/out signals conversion
			//
			Afb::TCONV_FP32_SI32,
			Afb::TCONV_SI32_FP32,

			// for discrete int/out signals inversion
			//
			Afb::AFB_BUS_NOT,
			Afb::AFB_NOT,
		};

		static const QString FB_SCALE_PREFIX("scale_");

		for(const QString& fbCaption : fbConvCaption)
		{
			bool fbFound = false;

			for(const std::shared_ptr<Afb::AfbElement>& afbElement : m_lmDescription->afbElements())
			{
				if (afbElement->caption() != fbCaption)
				{
					continue;
				}

				bool isScaleAfb = fbCaption.startsWith(FB_SCALE_PREFIX);

				fbFound = true;

				FbConv fb;

				fb.caption = fbCaption;
				fb.pointer = afbElement;

				if (isScaleAfb == true)
				{
					int index = 0;

					for(const Afb::AfbParam& afbParam : afbElement->params())
					{
						// do not replace 'index' on 'afbParam.operandIndex()' !
						//
						// operandIndex() for parameters always == -1
						//

						if (afbParam.caption() == Afb::SCALE_PARAM_X1)
						{
							fb.x1ParamIndex = index;
						}

						if (afbParam.caption() == Afb::SCALE_PARAM_X2)
						{
							fb.x2ParamIndex = index;
						}

						if (afbParam.caption() == Afb::SCALE_PARAM_Y1)
						{
							fb.y1ParamIndex = index;
						}

						if (afbParam.caption() == Afb::SCALE_PARAM_Y2)
						{
							fb.y2ParamIndex = index;
						}

						index++;
					}

					if (fb.x1ParamIndex == -1)
					{
						// Required parameter %1 of AFB %2 is missing.
						//
						m_log->errALC5045(Afb::SCALE_PARAM_X1, fbCaption, QUuid());
						result = false;
					}

					if (fb.x2ParamIndex == -1)
					{
						m_log->errALC5045(Afb::SCALE_PARAM_X2, fbCaption, QUuid());
						result = false;
					}

					if (fb.y1ParamIndex == -1)
					{
						m_log->errALC5045(Afb::SCALE_PARAM_Y1, fbCaption, QUuid());
						result = false;
					}

					if (fb.y2ParamIndex == -1)
					{
						m_log->errALC5045(Afb::SCALE_PARAM_Y2, fbCaption, QUuid());
						result = false;
					}
				}

				if (result == false)
				{
					break;
				}

				for(const Afb::AfbSignal& afbSignal : afbElement->inputSignals())
				{
					if (afbSignal.caption() == Afb::IN_PIN_CAPTION)
					{
						fb.inputSignalIndex = afbSignal.operandIndex();
						fb.inputSignalDataSize = afbSignal.size();
						break;
					}
				}

				if (fb.inputSignalIndex == -1)
				{
					// Required pin %1 of AFB %2 is missing.
					//
					m_log->errALC5173(Afb::IN_PIN_CAPTION, fbCaption, QUuid());
					result = false;
					break;
				}

				for(const Afb::AfbSignal& afbSignal : afbElement->outputSignals())
				{
					if (afbSignal.caption() == Afb::OUT_PIN_CAPTION)
					{
						fb.outputSignalIndex = afbSignal.operandIndex();
						fb.outputSignalDataSize = afbSignal.size();
						break;
					}
				}

				if (fb.outputSignalIndex == -1)
				{
					// Required pin %1 of AFB %2 is missing.
					//
					m_log->errALC5173(Afb::OUT_PIN_CAPTION, fbCaption, QUuid());
					result = false;
					break;
				}

				m_fbConv.emplace(fb.caption, fb);
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

					auto it = m_fbConv.find(Afb::SCALE_UI16_FP32);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						fb.pointer->params()[fb.x1ParamIndex].afbParamValue().setValue(QVariant(x1));
						fb.pointer->params()[fb.x2ParamIndex].afbParamValue().setValue(QVariant(x2));

						fb.pointer->params()[fb.y1ParamIndex].afbParamValue().setValue(QVariant(y1));
						fb.pointer->params()[fb.y2ParamIndex].afbParamValue().setValue(QVariant(y2));

						result &= appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
					}
				}

				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				{
					conversionIsKnown = true;

					auto it = m_fbConv.find(Afb::SCALE_UI16_SI32);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						fb.pointer->params()[fb.x1ParamIndex].afbParamValue().setValue(QVariant(x1));
						fb.pointer->params()[fb.x2ParamIndex].afbParamValue().setValue(QVariant(x2));

						fb.pointer->params()[fb.y1ParamIndex].afbParamValue().setValue(QVariant(y1).toInt());
						fb.pointer->params()[fb.y2ParamIndex].afbParamValue().setValue(QVariant(y2).toInt());

						result &= appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
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

					auto it = m_fbConv.find(Afb::TCONV_FP32_SI32);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						result &= appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
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

					auto it = m_fbConv.find(Afb::TCONV_SI32_FP32);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						result &= appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
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

					auto it = m_fbConv.find(Afb::SCALE_FP32_UI16);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						fb.pointer->params()[fb.x1ParamIndex].afbParamValue().setValue(QVariant(x1));
						fb.pointer->params()[fb.x2ParamIndex].afbParamValue().setValue(QVariant(x2));

						fb.pointer->params()[fb.y1ParamIndex].afbParamValue().setValue(QVariant(y1).toInt());
						fb.pointer->params()[fb.y2ParamIndex].afbParamValue().setValue(QVariant(y2).toInt());

						result = appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
					}
				}

				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				{
					conversionIsKnown = true;

					auto it = m_fbConv.find(Afb::SCALE_SI32_UI16);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						fb.pointer->params()[fb.x1ParamIndex].afbParamValue().setValue(QVariant(x1).toInt());
						fb.pointer->params()[fb.x2ParamIndex].afbParamValue().setValue(QVariant(x2).toInt());

						fb.pointer->params()[fb.y1ParamIndex].afbParamValue().setValue(QVariant(y1).toInt());
						fb.pointer->params()[fb.y2ParamIndex].afbParamValue().setValue(QVariant(y2).toInt());

						result = appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
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

					auto it = m_fbConv.find(Afb::TCONV_SI32_FP32);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						result &= appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
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

					auto it = m_fbConv.find(Afb::TCONV_FP32_SI32);

					if (it == m_fbConv.end())
					{
						LOG_INTERNAL_ERROR(m_log);
						result = false;
					}
					else
					{
						const FbConv& fb = it->second;

						result &= appItem->init(fb.pointer, errorMsg);
						appItem->setLabel(signal.appSignalID());

						if (errorMsg.isEmpty() == false)
						{
							LOG_INTERNAL_ERROR_MSG(m_log, errorMsg);
							result = false;
						}
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
			// check that added regular Tx signals exists in current LM
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

		const SchemaTransmitter* transmitter = ualItem->schemaTransmitter();

		TEST_PTR_RETURN_FALSE(transmitter);

		bool result = true;

		QVector<QPair<QString, UalSignal*>> connectedSignals;

		if (getConnectedSignals(ualItem, &connectedSignals) == false)
		{
			return false;
		}

		bool signalAlreadyInTxList = false;

		for(const QPair<QString, UalSignal*>& connectedSignal : connectedSignals)
		{
			const QStringList& connectionIDs = transmitter->connectionIdsAsList();

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
				result &= m_optoModuleStorage->appendTxSignal(ualItem->schemaID(), connectionID, transmitter->guid(),
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

		const std::vector<SchemaPin>& inPins = transmitterItem->inputs();

		bool result = true;

		for(const SchemaPin& inPin : inPins)
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

				if (s != nullptr && m_moduleSignals.contains(calcHash(s->appSignalID())) == true)
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

	bool ModuleLogicCompiler::getDirectlyConnectedInSignalID(const SchemaPin& inPin, QString* directlyConnectedInSignalID)
	{
		TEST_PTR_LOG_RETURN_FALSE(directlyConnectedInSignalID, log());

		directlyConnectedInSignalID->clear();

		const std::vector<QUuid>& accosiatedOutputsUuids = inPin.associatedIOs();

		for(QUuid outPinUuid : accosiatedOutputsUuids)
		{
			UalItem* ualItem = getValueOrNullptr(m_pinParent, outPinUuid);

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

	bool ModuleLogicCompiler::getNearestInSignalIDs(const SchemaPin& inPin, QStringList* nearestSignalIDs)
	{
		TEST_PTR_LOG_RETURN_FALSE(nearestSignalIDs, log());

		nearestSignalIDs->clear();

		// searching of directly connected input signal
		//
		const std::vector<QUuid>& accosiatedOutputsUuids = inPin.associatedIOs();

		for(QUuid outPinUuid : accosiatedOutputsUuids)
		{
			UalItem* ualItem = getValueOrNullptr(m_pinParent, outPinUuid);

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

		UalItem* ualItem = getValueOrNullptr(m_pinParent, outPinUuid);

		if (ualItem == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const SchemaPin* outPin = ualItem->getPin(outPinUuid);

		if (outPin == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<QUuid> associatedInputs = outPin->associatedIOs();

		for(QUuid inPinUuid : associatedInputs)
		{
			UalItem* nearestUalItem = getValueOrNullptr(m_pinParent, inPinUuid);

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

	bool ModuleLogicCompiler::getNearestInSignalID(const SchemaPin& inPin, QString* nearestSignalID)
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

	bool ModuleLogicCompiler::getNearestOutSignalIDs(const SchemaPin& outPin, QStringList* nearestSignalIDs)
	{
		TEST_PTR_LOG_RETURN_FALSE(nearestSignalIDs, log());
		LOG_INTERNAL_ERROR_IF_FALSE_RETURN_FALSE(outPin.IsOutput(), log());

		nearestSignalIDs->clear();

		::std::vector<QUuid> linkedPins = outPin.associatedIOs();

		for(QUuid linkedPin : linkedPins)
		{
			UalItem* ualItem = getValueOrNullptr(m_pinParent, linkedPin);

			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSignal() == true)
			{
				nearestSignalIDs->append(ualItem->strID());
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::getNearestOutSignalID(const SchemaPin& outPin, QString* nearestSignalID)
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

	bool ModuleLogicCompiler::getNearestSignalID(const SchemaPin& inOutPin, QString* nearestSignalID)
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

		const SchemaReceiver* receiver = item->schemaReceiver();

		TEST_PTR_RETURN_FALSE(receiver);

		QString connectionID = receiver->connectionIds();

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

		if (receiver->appSignalIdsAsList().size() > 1)
		{
			m_log->errINT1001(QString("SchemaItemReceiver has more then one AppSignalID"), item->schemaID(), item->guid());
		}

		QString rxSignalID = receiver->appSignalIds();

		if (m_moduleSignals.contains(calcHash(rxSignalID)) == false)
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

		for(UalAfb* afb : m_ualAfbs)
		{
			TEST_PTR_CONTINUE(afb);

			result &= addToComparatorSet(afb);
		}

		return result;
	}

	bool ModuleLogicCompiler::findEndpointSignals()
	{
		bool result = true;

		for(const UalItem* ualItem : m_ualItems)
		{
			TEST_PTR_CONTINUE(ualItem);

			if (ualItem->isSignal() == false)
			{
				continue;
			}

			bool isEndpoint = false;

			if (ualItem->outputs().size() == 0)
			{
				isEndpoint = true;
			}

			QString appSignalID = ualItem->strID();

			AppSignal* s = m_signals->getSignal(appSignalID);

			if (s == nullptr)
			{
				Q_ASSERT(false);	// this error should be detected early!

				// Signal identifier %1 is not found (Logic schema %2).
				//
				m_log->errALC5000(appSignalID, ualItem->guid(), ualItem->schemaID());

				result = false;
				continue;
			}

			s->setEndpoint(isEndpoint);
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

			const SchemaReceiver* ualReceiver = ualItem->schemaReceiver();

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
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_RETURN_FALSE(m_lmDescription);

		if (noCodeGenRequired() == true)
		{
			return true;
		}

		m_idrCode.clear();

		//

		std::vector<CodeGenProcToCall> procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::generateIdrCodeStart),
			PROC_TO_CALL(ModuleLogicCompiler::generateCustomCode),
			PROC_TO_CALL(ModuleLogicCompiler::generateAfbsVersionCheckingCode),
			PROC_TO_CALL(ModuleLogicCompiler::generateInitAfbsCode),
			PROC_TO_CALL(ModuleLogicCompiler::generateLoopbacksRefreshingCode),
			PROC_TO_CALL(ModuleLogicCompiler::generateConstBitsInitialization),
			PROC_TO_CALL(ModuleLogicCompiler::generateIdrCodeStop),
		};

		bool result = runCodeGenProcs(procs, &m_idrCode);

		return result;
	}

	bool ModuleLogicCompiler::generateAlpPhaseCode()
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		m_alpCode.clear();

		m_alpCode.comment("Start of ALP phase code");
		m_alpCode.newLine();

		//

		std::vector<CodeGenProcToCall> procs =
		{
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredRawDataInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::invertDiscreteInputSignals),
			PROC_TO_CALL(ModuleLogicCompiler::convertAnalogInputSignals),
			PROC_TO_CALL(ModuleLogicCompiler::generateAppLogicCode),

			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogOptoSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogBusChildSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredTuningAnalogSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredAnalogConstSignalsInRegBuf),

			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredInputBusesInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredBusChildBusesInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredOptoBusesInRegBuf),

			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteInputSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteOutputAndInternalSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteOptoSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteBusChildSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredTuningDiscreteSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyAcquiredDiscreteConstSignalsInRegBuf),
			PROC_TO_CALL(ModuleLogicCompiler::copyOutputSignalsInOutputModulesMemory),
			PROC_TO_CALL(ModuleLogicCompiler::copyOptoConnectionsTxData),
		};

		bool result = runCodeGenProcs(procs, &m_alpCode);

		//

		CodeItem stopCmd;

		stopCmd.stop();
		stopCmd.setComment("end of ALP phase code");

		m_alpCode.append(stopCmd);

		result &= m_alpCode.finalize(getLmDescription());

		return result;
	}

	bool ModuleLogicCompiler::makeSourceAppLogicCode()
	{
		return makeAppLogicCode(m_idrCode, m_alpCode, &m_appLogicCode);
	}

	bool ModuleLogicCompiler::makeAppLogicCode(AppLogicCode& idrCode,
											   AppLogicCode& alpCode,
											   AppLogicCode* appCode)
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		TEST_PTR_RETURN_FALSE(appCode);

		appCode->clear();

		bool result = true;

		result &= idrCode.finalize(getLmDescription());		// required to calc codeSizeW

		int alpCodeStartAddr = idrCode.codeSizeW();

		idrCode.setAppStartAddr(alpCodeStartAddr);

		result &= alpCode.finalize(getLmDescription());

		appCode->reserve(idrCode.itemsCount() + alpCode.itemsCount());

		appCode->append(idrCode);
		appCode->append(alpCode);

		result &= appCode->finalize(getLmDescription());

		return result;
	}

	bool ModuleLogicCompiler::checkAppLogicCode()
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		CodeChecker checker(*this);

		return checker.check(m_appLogicCode);
	}

	bool ModuleLogicCompiler::cleanupHeaps()
	{
		bool result = true;

		result &= m_ualSignals.finalizeHeaps();
		result &= writeHeapsLog();

		return result;
	}

	bool ModuleLogicCompiler::optimizeAppLogicCode()
	{
		m_optiIdrCode = m_idrCode;
		m_optiAlpCode = m_alpCode;

		m_optiIdrCode.setOptimized(true);
		m_optiAlpCode.setOptimized(true);

		//m_optiIdrCode.removeStopCommand();

		CodeOptimizationProcToCall procs[] =
		{
			PROC_TO_CALL(ModuleLogicCompiler::optimizeSequentialMoves),
			PROC_TO_CALL(ModuleLogicCompiler::optimizeSequentialConstMoves),
			PROC_TO_CALL(ModuleLogicCompiler::optimizeSequentialBitMoves),
			PROC_TO_CALL(ModuleLogicCompiler::optimizeBitFilling),
		};

		bool result = true;

		for(const CodeOptimizationProcToCall& p : procs)
		{
			std::function<bool(ModuleLogicCompiler*, CodeSnippet&)> proc = p.first;

			result &= std::invoke(proc, this, m_optiIdrCode);
			result &= std::invoke(proc, this, m_optiAlpCode);

			if (result == false)
			{
				// %1 has been finished with errors.
				//
				const QString& procName = p.second;
				m_log->errALC5999(procName);
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::makeOptimizedAppLogicCode()
	{
		return makeAppLogicCode(m_optiIdrCode, m_optiAlpCode, &m_optiAppLogicCode);
	}

	bool ModuleLogicCompiler::writeInfoLmFilesAfterOptimization()
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		bool result = true;

		result &= writeAsmFile(m_optiAppLogicCode);

		result &= writeStatisticsFile(m_optiAppLogicCode,
									  m_optiIdrCode,
									  m_optiAlpCode);

		result &= writeOptimizationReportFile();

		return result;
	}

	bool ModuleLogicCompiler::optimizeSequentialMoves(CodeSnippet& srcCode)
	{
		SequentialMovesOptimization smo(*this, srcCode, m_memoryMap);

		return smo.optimize();
	}

	bool ModuleLogicCompiler::optimizeSequentialConstMoves(CodeSnippet& srcCode)
	{
		SequentialConstMovesOptimization scmo(*this, srcCode, m_memoryMap);

		return scmo.optimize();
	}

	bool ModuleLogicCompiler::optimizeSequentialBitMoves(CodeSnippet& srcCode)
	{
		SequentialBitMovesOptimization sbmo(*this, srcCode);

		return sbmo.optimize();
	}

	bool ModuleLogicCompiler::optimizeBitFilling(CodeSnippet& srcCode)
	{
		BitFillingOptimization bfo(*this, srcCode);

		return bfo.optimize();
	}

	bool ModuleLogicCompiler::checkOptimizedAppLogicCode()
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		CodeChecker checker(*this);

		return checker.check(m_optiAppLogicCode);
	}

	bool ModuleLogicCompiler::generateIdrCodeStart(CodeSnippet* code)
	{
		TEST_PTR_RETURN_FALSE(code);

		code->comment(QString("LM equipmentID: %1").arg(lmEquipmentID()));
		code->comment_nl(QString("LM description: %1, version %2").
								arg(m_lmDescription->name()).
								arg(m_lmDescription->version()));

		code->comment_nl("Start of IDR phase code");

		CodeItem appStartCmd;

		// ALP phase code start addr will set to actual value later
		//
		appStartCmd.appStart(0, "set address of ALP phase code start");

		code->append(appStartCmd);
		code->newLine();

		return true;
	}

	bool ModuleLogicCompiler::generateCustomCode(CodeSnippet* code)
	{
		TEST_PTR_RETURN_FALSE(code);
		TEST_PTR_RETURN_FALSE(m_context);
		TEST_PTR_RETURN_FALSE(m_context->m_buildResultWriter);

		if (m_context->m_buildResultWriter->buildInfo().project != "time_tests")
		{
			return true;
		}

		int wordAcc1 = m_memoryMap.wordAccumulatorAddress();
		int wordAcc2 = m_memoryMap.wordAccumulator2Address();

		int bitAcc = m_memoryMap.bitAccumulatorAddress();

		CodeSnippet& cd = *code;
		CodeItem cmd;

		cd.comment_nl("Commands testing code START");

		cd << cmd.movConstInt32(wordAcc1, 0, "init word accumulator 1 (4 words)");
		cd << cmd.movConstInt32(wordAcc1 + 2, 0);

		cd.newLine();

		cd << cmd.movConstInt32(wordAcc2, 0, "init word accumulator 2 (4 words)");
		cd << cmd.movConstInt32(wordAcc2 + 2, 0);

		cd.newLine();

		cd << cmd.movConst(bitAcc, 0, "init bit accumulator (2 words)");
		cd << cmd.movConst(bitAcc + 1, 0);

		cd.newLine();

		cd << cmd.mov(wordAcc1, wordAcc2, "word <= word");
		cd << cmd.mov(bitAcc, wordAcc2, "bit <= word");
		cd << cmd.mov(wordAcc1, bitAcc, "word <= bit");
		cd << cmd.mov(bitAcc, bitAcc, "bit <= bit");

		cd.newLine();

		cd << cmd.movMem(wordAcc1, wordAcc1, 2, "word <= word");

		cd.newLine();

		cd << cmd.movConst(wordAcc1, 1234, "word <= const");
		cd << cmd.movConst(bitAcc, 5678, "bit <= const");

		cd.newLine();

		cd << cmd.movBitConst(wordAcc1, 3, 1, "word <= const bit");
		cd << cmd.movBitConst(bitAcc, 3, 1, "bit <= const bit");

		cd.newLine();

		cd << cmd.writeFuncBlockConst(14, 1, 0, 1, "SCALE_16UI_16UI", "init SCALE_16UI_16UI");
		cd << cmd.writeFuncBlock(14, 1, 5, wordAcc1, "SCALE_16UI_16UI", "fb.input16 <= word");
		cd << cmd.writeFuncBlock(14, 1, 5, bitAcc, "SCALE_16UI_16UI", "fb.input16 <= bit");
		cd << cmd.startafb(14, 1, "SCALE_16UI_16UI", 4);
		cd << cmd.readFuncBlock(wordAcc1, 14, 1, 8, "SCALE_16UI_16UI", "word <= fb.output16");
		cd << cmd.readFuncBlock(bitAcc, 14, 1, 8, "SCALE_16UI_16UI", "bit <= fb.output16");

		cd.newLine();

		cd << cmd.writeFuncBlockConst(1, 0, 0, 2, "AND", "init 2 AND");
		cd << cmd.writeFuncBlockConst(1, 0, 1, 1, "AND");
		cd << cmd.writeFuncBlockConst(1, 0, 2, 1, "AND");

		cd << cmd.writeFuncBlockBit(1, 0, 3, bitAcc, 7, "AND", "fb.input <= bit");
		cd << cmd.writeFuncBlockBit(1, 0, 4, wordAcc1, 11, "AND", "fb.input <= word");

		cd << cmd.startafb(1, 0, "AND", 5);

		cd << cmd.readFuncBlockBit(wordAcc1, 12, 1, 0, 20, "AND", "word <= fb.output");
		cd << cmd.readFuncBlockBit(bitAcc,4, 1, 0, 20, "AND", "bit <= fb.output");

		cd.newLine();

		cd << cmd.setMem(wordAcc1, 3456, 3);

		cd.newLine();

		cd << cmd.movBit(wordAcc1, 3, wordAcc2, 3, "word bit <= word bit");
		cd << cmd.movBit(wordAcc1, 5, bitAcc, 5, "word bit <= bit bit");
		cd << cmd.movBit(bitAcc, 6, wordAcc1, 6, "bit bit <= word bit");
		cd << cmd.movBit(bitAcc, 15, bitAcc + 1, 15, "bit <= bit");

		cd.newLine();

		cd << cmd.nstart(1, 0, 3, "AND", 5);

		cd.newLine();

		cd << cmd.mov32(wordAcc1 + 2, wordAcc1, "word <= word");

		cd.newLine();

		cd << cmd.movConstInt32(wordAcc1, 23456, "word <= const32");

		cd.newLine();

		cd << cmd.writeFuncBlockConst(14, 2, 0, 2, "SCALE_16UI_SI", "init SCALE_16UI_SI");
		cd << cmd.writeFuncBlockConstInt32(14, 2, 1, 32768, "SCALE_16UI_SI", "fb.input32 <= const32");
		cd << cmd.writeFuncBlock32(14, 2, 2, wordAcc1, "SCALE_16UI_SI", "fb.input32 <= word");
		cd << cmd.startafb(14, 2, "SCALE_16UI_SI", 4);
		cd << cmd.readFuncBlock32(wordAcc1, 14, 2, 9, "SCALE_16UI_SI", "word <= fb.output32");

		cd.newLine();

		cd << cmd.readFuncBlockCompareInt32(1, 0, 22, 2345678, "LOGIC");

		cd.newLine();

		cd << cmd.movCompareFlag(wordAcc1, 10, "word <= cmp flag");
		cd << cmd.movCompareFlag(bitAcc, 10, "bit <= cmp flag");

		cd.newLine();

		cd << cmd.prevMov(wordAcc1, wordAcc2, "word <= word");
		cd << cmd.prevMov(bitAcc, wordAcc2, "bit <= word");

		cd.newLine();

		cd << cmd.prevMov32(wordAcc1, wordAcc1 + 2, "word <= word");

		cd.newLine();

		cd << cmd.fillb(wordAcc1, bitAcc, 3, "word <= bit");
		cd << cmd.fillb(wordAcc1, wordAcc2, 3, "word <= from word");
		cd << cmd.fillb(bitAcc, wordAcc2, 3, "bit <= from word");
		cd << cmd.fillb(bitAcc, bitAcc + 1, 4, "bit <= from bit");

		cd.newLine();

		cd.comment_nl("Commands testing code END");

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
				const auto& component = components.at(afbOpcode);

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

		LOG_MESSAGE(m_log, QString(tr("Generation of AFB initialization code...")));

		std::map<QString, QString> instanceUsedBy;

		for(UalAfb* ualAfb : m_ualAfbs)
		{
			TEST_PTR_CONTINUE(ualAfb);

			if (ualAfb->hasRam() == true)
			{
				continue;
			}

			QString instantiatorID = ualAfb->instantiatorID();

			auto it = instanceUsedBy.find(instantiatorID);

			if (it == instanceUsedBy.end())
			{
				instanceUsedBy.emplace(instantiatorID, ualAfb->label());
			}
			else
			{	QString& labels = it->second;

				labels += QStringLiteral(", ");
				labels += ualAfb->label();
			}
		}

		bool result = true;

		code->comment_nl("AFBs initialization code");

		std::set<QString> processedInstantiatorsID;

		for(const AfbElementShared& afbElement : m_lmDescription->afbElements())
		{
			TEST_PTR_CONTINUE(afbElement);

			for(UalAfb* ualAfb : m_ualAfbs)
			{
				TEST_PTR_CONTINUE(ualAfb);

				if (ualAfb->afbStrID() != afbElement->strID())
				{
					continue;
				}

				if (ualAfb->hasRam() == true)
				{
					// initialize all params for each instance of FB with RAM
					//
					result &= generateIDRPhaseInitAppFbParamsCode(code, *ualAfb, ualAfb->label());
				}
				else
				{
					// FB without RAM initialize once for all instances
					// initialize instantiator params only
					//
					QString instantiatorID = ualAfb->instantiatorID();

					if (processedInstantiatorsID.contains(instantiatorID) == false)
					{
						auto it = instanceUsedBy.find(instantiatorID);

						if (it == instanceUsedBy.end())
						{
							Q_ASSERT(false);
						}
						else
						{
							result &= generateIDRPhaseInitAppFbParamsCode(code, *ualAfb, it->second);
							processedInstantiatorsID.emplace(instantiatorID);
						}
					}
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::generateIDRPhaseInitAppFbParamsCode(CodeSnippet* code, const UalAfb& appFb, const QString& usedBy)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		const AfbParamValuesArray& appFbParamValues = appFb.paramValuesArray();

		if (appFbParamValues.hasParamsToInitialization() == false)
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

		CodeSnippet initCode;

		result &= generateInitAppFbParamsCode(&initCode, appFb, true);

		if (initCode.isEmpty() == false)
		{
			code->append(initCode);
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::generateInitAppFbParamsCode(CodeSnippet* code, const UalAfb& appFb, bool instantiator)
	{
		const AfbParamValuesArray& appFbParamValues = appFb.paramValuesArray();

		QString fbCaption = appFb.caption();
		int fbOpcode = appFb.opcode();
		int fbInstance = appFb.instance();

		bool result = true;

		for(const AfbParamValue& paramValue : appFbParamValues)
		{
			if (paramValue.isNoFbOperand() == true)
			{
				continue;
			}

			if (paramValue.instantiator() != instantiator)
			{
				continue;
			}

			int operandIndex = paramValue.operandIndex();

			QString opName = paramValue.opName();

			CodeItem cmd;

			if (paramValue.type() == E::SignalType::Discrete)
			{
				// for discrete parameters
				//
				cmd.writeFuncBlockConst(fbOpcode, fbInstance, operandIndex, paramValue.unsignedIntValue(), fbCaption);
				cmd.setComment(QString("%1 <= %2").arg(opName).arg(paramValue.unsignedIntValue()));

				code->append(cmd);
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
					LOG_INTERNAL_ERROR_MSG(m_log, QString("Afb parameter '%1' with Float data format must have dataSize == 32").arg(opName));
					result = false;
					break;

				default:

					LOG_INTERNAL_ERROR_MSG(m_log, QString("Unknown Afb parameter data format"));
					result = false;
				}
			}

			code->append(cmd);
		}

		return result;
	}

	bool ModuleLogicCompiler::displayAfbParams(CodeSnippet* code, const UalAfb& appFb)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		const AfbParamValuesArray& appFbParamValues = appFb.paramValuesArray();

		for(const AfbParamValue& paramValue : appFbParamValues)
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

			if (paramValue.instantiator() == true)
			{
				commentStr.append(QStringLiteral(" (instantiator)"));
			}

			code->comment(commentStr);
		}

		return true;
	}

	bool ModuleLogicCompiler::generateLoopbacksRefreshingCode(CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		QVector<const UalSignal*> loopbacksUalSignals = QVector<const UalSignal*>::fromList(m_loopbacks.getLoopbacksUalSignals());

		if (loopbacksUalSignals.isEmpty() == true)
		{
			return true;
		}

		sortSignalList(loopbacksUalSignals);

		bool result = true;

		CodeSnippet analogsRefreshCode;
		CodeSnippet discreteRefreshCode;
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
					// refeshing code is generated only for discretes NOT placed in bit-addressed memory
					//
					getRefreshingCode(&discreteRefreshCode, loopbackIDs, lbSignal);
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

		if (discreteRefreshCode.isEmpty() == false)
		{
			refreshingCode.append(discreteRefreshCode);
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

		case E::Discrete:
			sizeW = 1;
			break;

		case E::Bus:
			sizeW = lbSignal->bus()->sizeW();
			busStr = QString("bustype %1 ").arg(lbSignal->busTypeID());
			break;

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
				cmd.setComment(QString("refreshing loopback %1 (%2signal %3)").arg(loopbackID).arg(busStr).arg(lbSignal->signal()->appSignalID()));
			}

			code->append(cmd);
		}

		return true;
	}

	bool ModuleLogicCompiler::generateConstBitsInitialization(CodeSnippet* code)
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

	bool ModuleLogicCompiler::generateIdrCodeStop(CodeSnippet* code)
	{
		TEST_PTR_RETURN_FALSE(code);

		CodeItem stopCmd;

		stopCmd.stop("end of IDR phase code");

		code->append(stopCmd);
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

	bool ModuleLogicCompiler::invertDiscreteInputSignals(CodeSnippet* code)
	{
		bool result = true;

		result &= generateInvertDiscreteInputsCode(code, m_acquiredDiscreteInvertedInputSignals,
												   QStringLiteral("Inversion of acquired discrete inputs"));

		result &= generateInvertDiscreteInputsCode(code, m_nonAcquiredDiscreteInvertedInputSignals,
												   QStringLiteral("Inversion of non acquired discrete inputs"));

		return result;
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
				LOG_INTERNAL_ERROR(m_log);
				Q_ASSERT(false);
				return false;
			}

			auto it = m_fbConv.find(appFb->caption());

			if (it == m_fbConv.end())
			{
				LOG_INTERNAL_ERROR(m_log);
				Q_ASSERT(false);
				return false;
			}

			const FbConv& fbConv = it->second;

			switch(fbConv.inputSignalDataSize)
			{
			case SIZE_32BIT:
				cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbConv.inputSignalIndex,
								   s->ioBufAddr().offset(), appFb->caption());
				break;

			case SIZE_16BIT:
				cmd.writeFuncBlock(appFb->opcode(), appFb->instance(), fbConv.inputSignalIndex,
								   s->ioBufAddr().offset(), appFb->caption());
				break;

			default:
				LOG_INTERNAL_ERROR(m_log);
				Q_ASSERT(false);
				return false;
			}

			cmd.setComment(QString(tr("conversion of analog input %1")).arg(s->appSignalID()));
			code->append(cmd);

			cmd.startafb(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.clearComment();
			code->append(cmd);

			Q_ASSERT(fbConv.outputSignalDataSize == SIZE_32BIT);

			cmd.readFuncBlock32(s->ualAddr().offset(), appFb->opcode(), appFb->instance(),
								fbConv.outputSignalIndex, appFb->caption());
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
				result &= generateBusExtractorCode(code, ualItem);
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

		const UalAfb* ualAfb = m_ualAfbs.getAfb(ualItem->guid());

		if (ualAfb->isSetFlagsItem() == true)
		{
			return true;		// no code generation required
		}

		TEST_PTR_RETURN_FALSE(ualAfb)

		bool result = true;

		bool isBusProcAfb = m_afbComponents.isBusProcessingAfb(ualAfb->strID());

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

		for(int stepNo = 0; stepNo < busProcessingStepsNumber && result == true; stepNo++)
		{
			BusProcessingStepInfo bpStepInfo;

			bpStepInfo.stepsNumber = busProcessingStepsNumber;
			bpStepInfo.currentStep = stepNo;
			bpStepInfo.currentStepSizeBits = busProcessingStepsSizes[stepNo];
			bpStepInfo.currentBusSignalOffsetW = currentBusSignalOffsetBits / SIZE_16BIT;

			//

			if (ualAfb->isPackedLogic() == true)
			{
				Q_ASSERT(busProcessingStepsNumber == 1);

				result &= generatePackedAfbCode(code, ualAfb);

				continue;
			}

			//

			bool bitAccCodeGenerated = false;

			if (m_bitAccAvailable == true)
			{
				bool res = true;
				bitAccCodeGenerated = generateAfbBitAccCode(code, ualAfb, bpStepInfo, &res);
				result &= res;

				if (res == false)
				{
					continue;
				}
			}

			if (bitAccCodeGenerated == false)
			{
				result &= generateInitAppFbParamsCode(code, *ualAfb, false);

				result &= generateSignalsToAfbInputsCode(code, ualAfb, bpStepInfo);

				result &= startAfb(code, ualAfb, bpStepInfo);

				result &= generateAfbOutputsToSignalsCode(code, ualAfb, bpStepInfo);
			}

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

		for(const SchemaPin& inPin : ualAfb->inputs())
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

			AfbSignal inAfbSignal;

			bool res = ualAfb->getAfbSignalByPin(inPin, &inAfbSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = res;
				continue;
			}

			result &= generateSignalToAfbInputCode(code, ualAfb, inAfbSignal, inUalSignal, bpStepInfo, Address16(), false);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateSignalToAfbInputCode(CodeSnippet* code,
														   const UalAfb* ualAfb,
														   const AfbSignal& inAfbSignal,
														   const UalSignal* inUalSignal,
														   const BusProcessingStepInfo& bpStepInfo,
														   const Address16& readAddr,
														   bool ignoreTypeChecking)
	{
		// inUalSignal can be NULL
		//
		// if inUalSignal is NULL, readAddr must be Valid!
		//
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);

		if (inUalSignal != nullptr)
		{
			if (ignoreTypeChecking == false)
			{
				if (inUalSignal->isCanBeConnectedTo(*ualAfb, inAfbSignal, log()) == false)
				{
					// Uncompatible signals connection (Logic schema '%1').
					//
					m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inUalSignal->ualItemGuid(), inUalSignal->ualItemLabel(), ualAfb->schemaID());
					return false;
				}
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
		}
		else
		{
			if (readAddr.isValid() == false)
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
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

			if (inUalSignal != nullptr)
			{
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
			}
			else
			{
				cmd.writeFuncBlockBit(afbOpcode, afbInstance, afbSignalIndex,
									  readAddr,
									  afbCaption);
				code->append(cmd);
			}

			break;

		case E::SignalType::Analog:

			if (inUalSignal != nullptr)
			{
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
						Q_ASSERT(false);
						LOG_INTERNAL_ERROR(m_log);
						result = false;
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
			}
			else
			{
				switch(inAfbSignal.size())
				{
				case SIZE_32BIT:
					cmd.writeFuncBlock32(afbOpcode, afbInstance, afbSignalIndex, readAddr.offset(), afbCaption);
					break;

				case SIZE_16BIT:
					cmd.writeFuncBlock(afbOpcode, afbInstance, afbSignalIndex, readAddr.offset(), afbCaption);
					break;

				default:
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				code->append(cmd);
			}
			break;

		case E::SignalType::Bus:

			if (inUalSignal != nullptr)
			{
				result = generateSignalToAfbBusInputCode(code, ualAfb, inAfbSignal, inUalSignal, bpStepInfo);
			}
			else
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			break;

		default:
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
															  const AfbSignal& inAfbSignal,
															  const UalSignal* inUalSignal,
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
																	  const AfbSignal& inAfbSignal, const UalSignal* inUalSignal,
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

		QString comment = QString("%1.%2 <= %3").arg(ualAfb->caption()).arg(inAfbSignal.caption()).arg(inUalSignal->refSignalIDsJoined());

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

			cmd.fillb(Address16(wordAccAddr, 0), readUalAddr);
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
																 const AfbSignal& inAfbSignal, const UalSignal* inUalSignal,
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

		cmd.setComment(QString("%1.%2 <= %3 (part %4)").
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

		cmd.startafb(ualAfb->opcode(), ualAfb->instance(), ualAfb->caption(), ualAfb->runTime());

		if (ualAfb->isBusProcessing() == false || bpStepInfo.stepsNumber == 1)
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

		for(const SchemaPin& outPin : ualAfb->outputs())
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

			AfbSignal outAfbSignal;

			bool res = ualAfb->getAfbSignalByPin(outPin, &outAfbSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = res;
				continue;
			}

			result &= generateAfbOutputToSignalCode(code, ualAfb, outAfbSignal, outUalSignal, bpStepInfo, Address16(), false);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAfbOutputToSignalCode(CodeSnippet* code,
															const UalAfb* ualAfb,
															const AfbSignal& outAfbSignal,
															const UalSignal* outUalSignal,
															const BusProcessingStepInfo& bpStepInfo,
															const Address16& writeAddr,
															bool ignoreTypeChecking)
	{
		// outUalSignal can be NULLL
		//
		// If outUalSignal is NULL, writeAddr should be Valid!
		//

		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);

		if (outUalSignal != nullptr)
		{
			if (outUalSignal->isConst() == true)
			{
				assert(false);							// can't assign value to const ual signal
				LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
				return false;
			}

			if (ignoreTypeChecking == false)
			{
				if (outUalSignal->isCanBeConnectedTo(*ualAfb, outAfbSignal, log()) == false)
				{
					// Uncompatible signals connection (Logic schema '%1').
					//
					m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outUalSignal->ualItemGuid(), outUalSignal->appSignalID(), ualAfb->schemaID());
					return false;
				}
			}

			// outUalSignal and outAfbSignal are compatible

			if (outUalSignal->checkUalAddr() == false)
			{
				LOG_UNDEFINED_UAL_ADDRESS(m_log, outUalSignal);
				return false;
			}
		}
		else
		{
			if (writeAddr.isValid() == false)
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}

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

			if (outUalSignal != nullptr)
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
			else
			{
				cmd.readFuncBlockBit(writeAddr, afbOpcode, afbInstance, afbSignalIndex, afbCaption);
				code->append(cmd);
			}

			break;

		case E::SignalType::Analog:

			if (outUalSignal != nullptr)
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
			else
			{
				switch(outAfbSignal.size())
				{
				case SIZE_32BIT:
					cmd.readFuncBlock32(writeAddr.offset(), afbOpcode, afbInstance, afbSignalIndex, afbCaption);
					break;

				case SIZE_16BIT:
					cmd.readFuncBlock(writeAddr.offset(), afbOpcode, afbInstance, afbSignalIndex, afbCaption);
					break;

				default:
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				code->append(cmd);
			}

			break;

		case E::SignalType::Bus:

			if (outUalSignal != nullptr)
			{
				result = generateAfbBusOutputToBusSignalCode(code, ualAfb, outAfbSignal, outUalSignal, bpStepInfo);
			}
			else
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			break;

		default:
			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// this error should be detect early
			return false;
		}

		return result;
	}

	bool ModuleLogicCompiler::generateAfbBusOutputToBusSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
																  const AfbSignal& outAfbSignal, const UalSignal* outUalSignal,
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

		cmd.setComment(QString("%1 (part %2) <= %3.%4").
					   arg(outUalSignal->refSignalIDsJoined()).arg(bpStepInfo.currentStep + 1).
					   arg(ualAfb->caption()).arg(outAfbSignal.caption()));

		code->append(cmd);

		return true;
	}

	bool ModuleLogicCompiler::generatePackedAfbCode(CodeSnippet* code, const UalAfb* afb)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(afb, m_log);

		Q_ASSERT(afb->isPackedLogic());

		bool result = true;

		const std::vector<SchemaPin>& inputs = afb->inputs();

		std::vector<std::pair<const UalSignal*, Address16>> nonConstInSignals;	// vector of pair<inputSignal, readAddr>
		std::set<const UalSignal*> uniqueInSignals;

		int packedAfbOpcode = afb->opcode();

		int const0Count = 0;
		int const1Count = 0;

		auto it = m_context->m_packedLogicSources.find(afb->label());

		if (it != m_context->m_packedLogicSources.end())
		{
#ifdef QT_DEBUG
			const std::list<LmPackedLogicSources>& sources = it->second;

			for(const LmPackedLogicSources& src : sources)
			{
				if (src.lmID == lmEquipmentID())
				{
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}
			}
#endif
		}
		else
		{
			auto [newIt, b] = m_context->m_packedLogicSources.emplace(afb->label(), std::list<LmPackedLogicSources>{});
			it = newIt;
		}

		LmPackedLogicSources& lmSources = it->second.emplace_back(LmPackedLogicSources{});

		lmSources.lmID = lmEquipmentID();

		// input signals checking
		//
		for(const SchemaPin& inPin : inputs)
		{
			const UalSignal* inSignal = getUalSignalByPinCaption(afb, inPin.caption(), true);

			if (inSignal == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for '%1' pin of AFB %2 (schema %3)").
													arg(inPin.caption()).arg(afb->label()).arg(afb->schemaID()));
				result = false;
				continue;
			}

			PackedLogicSource src;

			src.appSignalID = inSignal->appSignalID();
			src.sourceItemLabelOut = inSignal->ualItemLabelOutPinCaption();

			lmSources.sources.push_back(src);

			Address16 readAddr;

			if (inSignal->isConstDiscrete() == true)
			{
				if (inSignal->constDiscreteValue() == 0)
				{
					if (packedAfbOpcode == Afb::PACKED_AND_OPCODE)
					{
						// const value 0 on input
						//
						const UalItem* inItem = inSignal->ualItem();

						TEST_PTR_LOG_RETURN_FALSE(inItem, m_log);

						// Permanent const 0 on output of packed_and %1 (item %2, schema %3) due to const 0 on input (item %4, schema %5).
						//
						m_log->wrnALC5204(afb->packedLogicID(), afb->label(), afb->guid(), afb->schemaID(),
										inItem->label(), inItem->guid(), inItem->schemaID());
					}

					const0Count++;
				}
				else
				{
					if (packedAfbOpcode == Afb::PACKED_OR_OPCODE)
					{
						// const value 1 on input
						//
						const UalItem* inItem = inSignal->ualItem();

						TEST_PTR_LOG_RETURN_FALSE(inItem, m_log);

						// Permanent const 1 on output of packed_or %1 (item %2, schema %3) due to const 1 on input (item %4, schema %5).
						//
						m_log->wrnALC5203(afb->packedLogicID(), afb->label(), afb->guid(), afb->schemaID(),
										inItem->label(), inItem->guid(), inItem->schemaID());
					}

					const1Count++;
				}

				continue;
			}

			// non-const signals processing

			if (inSignal->isDiscrete() == true)
			{
				readAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);

				if(readAddr.isValid() == false)
				{
					// Undefined UAL address of signal %1 (Logic schema %2).
					//
					m_log->errALC5105(inSignal->appSignalID(), inSignal->ualItemGuid(), inSignal->ualItemSchemaID());
					result = false;
					continue;
				}
			}
			else
			{
				// Uncompatible signals connection (Logic schema '%1').
				//
				m_log->errALC5117(afb->guid(), afb->label(), inSignal->ualItemGuid(), inSignal->ualItemLabel(), afb->schemaID());
				result = false;
				continue;
			}

			if (uniqueInSignals.contains(inSignal) == true)
			{
				continue;
			}

			nonConstInSignals.emplace_back(inSignal, readAddr);
			uniqueInSignals.insert(inSignal);
		}

		RETURN_IF_FALSE(result);

		if (isOutConnectedToTerminatorOnly(afb) == true)
		{
			return true;			// no code generation required
		}

		//

		const UalSignal* outSignal = getUalSignalByPinCaption(afb, Afb::OUT_PIN_CAPTION, false);

		if (outSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for 'out' pin of AFB %1 (schema %2)").
												arg(afb->label()).arg(afb->schemaID()));
			return false;
		}

		if (outSignal->isDiscrete() == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(afb->guid(), afb->label(), outSignal->ualItemGuid(), outSignal->ualItemLabel(), afb->schemaID());
			return false;
		}

		Address16 outWriteAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

		if(outWriteAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
			return false;
		}

		//
		// optimized code generation
		//

		std::optional<int> constOutValue;

		switch(packedAfbOpcode)
		{
		case Afb::PACKED_AND_OPCODE:

			if (const0Count > 0)
			{
				constOutValue = 0;
			}
			else
			{
				if (const1Count > 0 && nonConstInSignals.size() == 0)
				{
					constOutValue = 1;
				}
			}

			break;

		case Afb::PACKED_OR_OPCODE:

			if (const1Count > 0)
			{
				constOutValue = 1;
			}
			else
			{
				if (const0Count > 0 && nonConstInSignals.size() == 0)
				{
					constOutValue = 0;
				}
			}

			break;

		default:
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (constOutValue.has_value() == true)
		{
			*code << CodeItem().movBitConst(outWriteAddr, constOutValue.value(),
											QString("@%1 optimized processing: out <= const %2").
													arg(afb->label()).arg(constOutValue.value()));
			return true;
		}

		if (nonConstInSignals.size() == 1)
		{
			const UalSignal* inSignal = nonConstInSignals[0].first;
			Address16 readAddr = nonConstInSignals[0].second;

			*code << CodeItem().movBit(outWriteAddr, readAddr,
									   QString("@%1 optimized processing: out <= %2").
											arg(afb->label()).arg(inSignal->appSignalID()));
			return true;
		}

		//

		if (m_lmDescription->isBitAccAvailable() == true)
		{
			result &= generateBitAccBasedPackedAfbCode(code, afb, nonConstInSignals,
																outSignal, outWriteAddr);
		}
		else
		{
			result &= generateAfbBasedPackedAfbCode(code, afb, nonConstInSignals,
													outSignal, outWriteAddr);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateBitAccBasedPackedAfbCode(CodeSnippet* code, const UalAfb* afb,
													const std::vector<std::pair<const UalSignal*, Address16>>& inSignals,
													const UalSignal* outSignal, const Address16& outWriteAddr)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(afb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(outSignal, m_log);

		Q_ASSERT(m_lmDescription->isBitAccAvailable() == true);

		//
		// inSignals - is unique non const input signals
		//

		bool isAndLogic = false;
		QString afbCaption;

		if (afb->isPackedAndLogic() == true)
		{
			isAndLogic = true;
			afbCaption = Afb::AFB_AND;
		}
		else
		{
			if (afb->isPackedOrLogic() == true)
			{
				isAndLogic = false;
				afbCaption = Afb::AFB_OR;
			}
			else
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}

		code->comment_nl(QString("Packed_%1 %2 processing").arg(afbCaption).arg(afb->label()));

		CodeItem cmd;

		int inSignalsCount = static_cast<int>(inSignals.size());

		if (inSignalsCount < 2)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		int inSignalIndex = 0;

		int step = 1;

		int canProcessBits = SIZE_16BIT;

		while(inSignalIndex < inSignals.size())
		{
			int remainSignalsCount = inSignalsCount - inSignalIndex;

			if (step > 1)
			{
				canProcessBits = SIZE_16BIT - 1;	//	take into consideration prev step result saved in ACC[0]
			}

			remainSignalsCount = std::min(remainSignalsCount, canProcessBits);

			if (remainSignalsCount < canProcessBits)
			{
				if (step > 1)
				{
					*code << cmd.movBitAddrAcc(bitAccumulatorAddress16());
				}

				// initialization of bits that will not used
				//
				if (isAndLogic)
				{
					*code << cmd.setAcc();
				}
				else
				{
					*code << cmd.resetAcc();
				}

				if (step > 1)
				{
					*code << cmd.movBitAccAddr(bitAccumulatorAddress16());
				}
			}

			int bitNo = 0;

			while(bitNo < canProcessBits && inSignalIndex < inSignals.size())
			{
				const auto& [inSignal, readAddr] = inSignals[inSignalIndex];

				inSignalIndex++;

				// load next input signal
				//
				*code << cmd.movBitAccAddr(readAddr, QString("ACC[0] <= %1").arg(inSignal->appSignalID()));

				bitNo++;
			}

			if (isAndLogic)
			{
				*code << cmd.andAcc(QString("compute @%1 step %2").arg(afb->label()).arg(step));
			}
			else
			{
				*code << cmd.orAcc(QString("compute @%1 step %2").arg(afb->label()).arg(step));
			}

			step++;


			if (inSignalIndex >= inSignals.size())
			{
				// finalize processing
				//
				*code << cmd.movBitAddrAcc(outWriteAddr, QString("%1 <= ACC[0] (final result)").
															arg(outSignal->appSignalID()));
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::generateAfbBasedPackedAfbCode(CodeSnippet* code, const UalAfb* afb,
													const std::vector<std::pair<const UalSignal*, Address16>>& inSignals,
													const UalSignal* outSignal, const Address16& outWriteAddr)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(afb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(outSignal, m_log);

		Q_ASSERT(m_lmDescription->isBitAccAvailable() == false);

		//
		// inSignals - is unique non const input signals
		//

		if (m_packedLogicAfbInstance == -1)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const AfbComponents::LogicInfo& li = m_afbComponents.logicInfo();

		if (li.isValid() == false)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString afbCaption;
		int confValue = -1;
		int instance = m_packedLogicAfbInstance;

		if (afb->isPackedAndLogic() == true)
		{
			afbCaption = Afb::AFB_AND;
			confValue = li.confAnd;
		}
		else
		{
			if (afb->isPackedOrLogic() == true)
			{
				afbCaption = Afb::AFB_OR;
				confValue = li.confOr;
			}
			else
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}

		code->comment_nl(QString("Packed_%1 %2 processing").arg(afbCaption).arg(afb->label()));

		CodeItem cmd;

		// set operation AND or OR
		//
		*code << cmd.writeFuncBlockConst(li.opCode, instance, li.confIndex, confValue, afbCaption,
										 QString("%1.%2 <= %3").arg(afbCaption).
										 arg(Afb::PARAM_I_CONF).arg(confValue));

		// bus width set to 1
		//
		*code << cmd.writeFuncBlockConst(li.opCode, instance, li.busWidthIndex, 1, afbCaption,
										 QString("%1.%2 <= %3").arg(afbCaption).
										 arg(Afb::PARAM_I_BUS_WIDTH).arg(1));

		//

		int inSignalsCount = static_cast<int>(inSignals.size());

		if (inSignalsCount < li.minOperandsCount)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}
		int inSignalIndex = 0;

		int step = 1;
		int prevOperandsCount = -1;

		while(inSignalIndex < inSignals.size())
		{
			int remainSignalsCount = inSignalsCount - inSignalIndex;

			if (step > 1)
			{
				remainSignalsCount++;	//	take into consideration prev step result saved in bit accumulator
			}

			remainSignalsCount = std::min(remainSignalsCount, li.maxOperandsCount);

			Q_ASSERT(remainSignalsCount >= li.minOperandsCount);

			if (prevOperandsCount != remainSignalsCount)
			{
				*code << cmd.writeFuncBlockConst(li.opCode, instance, li.operandQuantityIndex,
												 remainSignalsCount, afbCaption,
												 QString("%1.%2 <= %3").arg(afbCaption).
												 arg(Afb::PARAM_I_OPRD_QUANT).arg(remainSignalsCount));

				prevOperandsCount = remainSignalsCount;
			}

			int inputNo = 0;

			if (step > 1)
			{
				// load prev step result from bit accumulator
				//
				*code << cmd.writeFuncBlockBit(li.opCode, instance, li.firstInIndex + inputNo,
														bitAccumulatorAddress16(), afbCaption,
														QString("%1.in_%2 <= prev step result").arg(afbCaption).
															arg(inputNo + 1));
				inputNo++;
			}

			while(inputNo < remainSignalsCount && inSignalIndex < inSignals.size())
			{
				const auto& [inSignal, readAddr] = inSignals[inSignalIndex];

				inSignalIndex++;

				// load next input signal
				//
				*code << cmd.writeFuncBlockBit(li.opCode, instance, li.firstInIndex + inputNo,
														readAddr, afbCaption,
														QString("%1.in_%2 <= %3").arg(afbCaption).
																arg(inputNo + 1).arg(inSignal->appSignalID()));
				inputNo++;
			}

			*code << CodeItem().startafb(li.opCode, instance, afbCaption, 2,
											QString("compute @%1 step %2").arg(afb->label()).arg(step));

			if (inSignalIndex < inSignals.size())
			{
				// save intermediate result into bit accumulator
				//
				*code << cmd.readFuncBlockBit(bitAccumulatorAddress16(), li.opCode, instance,
													 li.resultIndex, afbCaption,
														QString("bitAccumulator <= @%1.out (intermediate result)").
															arg(afb->label()));
				step++;			// go to next step
			}
			else
			{
				// finalize processing
				//
				*code << cmd.readFuncBlockBit(outWriteAddr, li.opCode, instance,
													 li.resultIndex, afbCaption,
														QString("%1 <= @%2.out (final result)").
															arg(outSignal->appSignalID()).arg(afb->label()));
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::generateAfbBitAccCode(CodeSnippet* code, const UalAfb* ualAfb,
													const BusProcessingStepInfo& bpStepInfo, bool* result)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualAfb, m_log);
		TEST_PTR_LOG_RETURN_FALSE(result, m_log);
		Q_ASSERT(m_bitAccAvailable == true);

		if ((ualAfb->caption() == Afb::AFB_NOT ||
			ualAfb->caption() == Afb::AFB_BUS_NOT) &&
			ualAfb->opcode() == Afb::AFB_NOT_ACC_OPCODE)
		{
			return generateAfbBitAccNotCode(code, ualAfb, bpStepInfo, result);
		}

		if (ualAfb->opcode() == TO_INT(Afb::AfbType::LOGIC))
		{
			if (m_afbsOrForBitAccReplacing.contains(ualAfb->guid()))
			{
				return generateAfbBitAccOrCode(code, ualAfb, result);
			}

			if (m_afbsAndForBitAccReplacing.contains(ualAfb->guid()))
			{
				return generateAfbBitAccAndCode(code, ualAfb, result);
			}
		}

		return false;
	}

	bool ModuleLogicCompiler::generateAfbBitAccNotCode(CodeSnippet* code, const UalAfb* ualAfb,
													const BusProcessingStepInfo& bpStepInfo, bool* result)
	{
		Q_ASSERT(ualAfb->opcode() == Afb::AFB_NOT_ACC_OPCODE);

		*result = true;

		UalSignal* inSignal = getUalSignalByPinCaption(ualAfb, Afb::IN_PIN_CAPTION, true);

		if (inSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for 'in' pin of AFB %1 (schema %2)").
												arg(ualAfb->label()).arg(ualAfb->schemaID()));
			*result = false;
			return false;
		}

		UalSignal* outSignal = getUalSignalByPinCaption(ualAfb, Afb::OUT_PIN_CAPTION, false);

		if (outSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for 'out' pin of AFB %1 (schema %2)").
												arg(ualAfb->label()).arg(ualAfb->schemaID()));
			*result = false;
			return false;
		}

		if (ualAfb->caption() == Afb::AFB_NOT)
		{
			return generateAfbBitAcc1NotCode(code, ualAfb, inSignal, outSignal, result);
		}

		if (ualAfb->caption() == Afb::AFB_BUS_NOT)
		{
			return generateAfbBitAccBusNotCode(code, ualAfb, bpStepInfo, inSignal, outSignal, result);
		}

		Q_ASSERT(false);
		return false;
	}

	bool ModuleLogicCompiler::generateAfbBitAcc1NotCode(CodeSnippet* code, const UalAfb* ualAfb,
														UalSignal* inSignal, UalSignal* outSignal,
														bool* result)
	{
		if (inSignal->isDiscrete() == false &&
			(inSignal->isConstDiscrete() == false))
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inSignal->ualItemGuid(), inSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		if (outSignal->isDiscrete() == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outSignal->ualItemGuid(), outSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		if (inSignal->isConstDiscrete() == true)
		{
			Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

			if(writeAddr.isValid() == false)
			{
				// Undefined UAL address of signal %1 (Logic schema %2).
				//
				m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
				*result = false;
				return false;
			}

			*code << CodeItem().movBitConst(writeAddr, inSignal->constDiscreteValue() == 0 ? 1 : 0,
								QString("compute not @%1 (optimized)").arg(ualAfb->label()));
			return true;
		}

		Address16 readAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);

		if(readAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(inSignal->appSignalID(), inSignal->ualItemGuid(), inSignal->ualItemSchemaID());
			*result = false;
			return false;
		}

		if (isOutConnectedToTerminatorOnly(ualAfb) == true)
		{
			*result = true;
			return true;		// report like a "code generated"
		}

		Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

		if(writeAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
			*result = false;
			return false;
		}

		*code << CodeItem().movBitAccAddr(readAddr, QString("ACC[0] <= %1").
											arg(inSignal->refSignalIDsJoined()));
		*code << CodeItem().notAcc(QString("compute not @%1").arg(ualAfb->label()));
		*code << CodeItem().movBitAddrAcc(writeAddr, QString("%1 <= ACC[0]").
											arg(outSignal->refSignalIDsJoined()));
		return true;
	}

	bool ModuleLogicCompiler::generateAfbBitAccBusNotCode(CodeSnippet* code, const UalAfb* ualAfb,
														const BusProcessingStepInfo& bpStepInfo,
														UalSignal* inSignal, UalSignal* outSignal, bool* result)
	{
		if (inSignal->isDiscrete() == false &&
			inSignal->isConstDiscrete() == false &&
			inSignal->isBus() == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inSignal->ualItemGuid(), inSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		AfbSignal logicInSignal;

		if (ualAfb->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &logicInSignal) == false)
		{
			*result = false;
			return false;
		}

		if (inSignal->isCanBeConnectedTo(*ualAfb, logicInSignal, m_log) == false)
		{
			*result = false;
			return false;
		}

		if (outSignal->isBus() == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outSignal->ualItemGuid(), outSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		AfbSignal logicOutSignal;

		if (ualAfb->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &logicOutSignal) == false)
		{
			*result = false;
			return false;
		}

		if (outSignal->isCanBeConnectedTo(*ualAfb, logicOutSignal, m_log) == false)
		{
			*result = false;
			return false;
		}

		if (inSignal->isConstDiscrete() == true)
		{
			Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

			if(writeAddr.isValid() == false)
			{
				// Undefined UAL address of signal %1 (Logic schema %2).
				//
				m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
				*result = false;
				return false;
			}

			writeAddr.addWord(bpStepInfo.currentBusSignalOffsetW);

			*code << CodeItem().movConst(writeAddr, inSignal->constDiscreteValue() == 0 ? 0xFFFF : 0,
										QString("compute bus_not @%1 (step %2/%3 optimized)").
										 arg(ualAfb->label()).
										 arg(bpStepInfo.currentStep + 1).
										 arg(bpStepInfo.stepsNumber));
			return true;
		}

		if (inSignal->isDiscrete() == true)
		{
			Address16 readAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);

			if(readAddr.isValid() == false)
			{
				// Undefined UAL address of signal %1 (Logic schema %2).
				//
				m_log->errALC5105(inSignal->appSignalID(), inSignal->ualItemGuid(), inSignal->ualItemSchemaID());
				*result = false;
				return false;
			}

			if (isOutConnectedToTerminatorOnly(ualAfb) == true)
			{
				*result = true;
				return true;		// report like a "code generated"
			}

			Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

			if(writeAddr.isValid() == false)
			{
				// Undefined UAL address of signal %1 (Logic schema %2).
				//
				m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
				*result = false;
				return false;
			}

			writeAddr.addWord(bpStepInfo.currentBusSignalOffsetW);

			*code << CodeItem().fillb(wordAccumulatorAddress16(), readAddr,
									  QString("WordACC <= %1").arg(inSignal->refSignalIDsJoined()));
			*code << CodeItem().movAccAddr(wordAccumulatorAddress());
			*code << CodeItem().notAcc(QString("compute bus_not @%1 (step %2/%3)").
														arg(ualAfb->label()).
														arg(bpStepInfo.currentStep + 1).
														arg(bpStepInfo.stepsNumber));
			*code << CodeItem().movAddrAcc(writeAddr, QString("%1 (part %2)) <= bus_not.out").
										   arg(outSignal->appSignalID()).arg(bpStepInfo.currentStep + 1));
			return true;
		}

		// inSignal and outSignal is busses

		Address16 readAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);

		if(readAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(inSignal->appSignalID(), inSignal->ualItemGuid(), inSignal->ualItemSchemaID());
			*result = false;
			return false;
		}

		readAddr.addWord(bpStepInfo.currentBusSignalOffsetW);

		Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

		if(writeAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
			*result = false;
			return false;
		}

		writeAddr.addWord(bpStepInfo.currentBusSignalOffsetW);

		*code << CodeItem().movAccAddr(readAddr, QString("ACC <= %1 (part %2)").
														arg(inSignal->appSignalID()).
														arg(bpStepInfo.currentStep + 1));

		*code << CodeItem().notAcc(QString("compute bus_not @%1 (step %2/%3)").
											arg(ualAfb->label()).
											arg(bpStepInfo.currentStep + 1).
											arg(bpStepInfo.stepsNumber));

		*code << CodeItem().movAddrAcc(writeAddr, QString("%1 (part %2)) <= bus_not.out").
														arg(outSignal->appSignalID()).
														arg(bpStepInfo.currentStep + 1));
		return true;
	}

	bool ModuleLogicCompiler::generateAfbBitAccOrCode(CodeSnippet* code, const UalAfb* ualAfb, bool* result)
	{
		TEST_PTR_RETURN_FALSE(code);
		TEST_PTR_RETURN_FALSE(ualAfb);
		TEST_PTR_RETURN_FALSE(result);

		*result = true;

		std::vector<std::pair<const UalSignal*, Address16>> inSignals;			// pair<ualSignal, readAddress>

		const std::vector<SchemaPin>& inputs = ualAfb->inputs();

		bool hasConst1 = false;
		bool allInputsConst0 = true;

		for(const SchemaPin& inPin : inputs)
		{
			UalSignal* inSignal = getUalSignalByPinCaption(ualAfb, inPin.caption(), true);

			if (inSignal == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for '%1' pin of AFB %2 (schema %3)").
													arg(inPin.caption()).arg(ualAfb->label()).arg(ualAfb->schemaID()));
				*result = false;
				return false;
			}

			if (inSignal->isConstDiscrete() == true)
			{
				if (inSignal->constDiscreteValue() == 1)
				{
					hasConst1 = true;
				}

				continue;			// skip const 0 value
			}

			if (inSignal->isDiscrete() == true)
			{
				allInputsConst0 = false;

				Address16 readAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);

				if(readAddr.isValid() == false)
				{
					// Undefined UAL address of signal %1 (Logic schema %2).
					//
					m_log->errALC5105(inSignal->appSignalID(), inSignal->ualItemGuid(), inSignal->ualItemSchemaID());
					*result = false;
					return false;
				}

				inSignals.emplace_back(inSignal, readAddr);
				continue;
			}

			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inSignal->ualItemGuid(), inSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		if (isOutConnectedToTerminatorOnly(ualAfb) == true)
		{
			*result = true;
			return true;		// report like a "code generated"
		}

		UalSignal* outSignal = getUalSignalByPinCaption(ualAfb, Afb::OUT_PIN_CAPTION, false);

		if (outSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for 'out' pin of AFB %1 (schema %2)").
												arg(ualAfb->label()).arg(ualAfb->schemaID()));
			*result = false;
			return false;
		}

		if (outSignal->isDiscrete() == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outSignal->ualItemGuid(), outSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

		if(writeAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
			*result = false;
			return false;
		}

		//

		if (hasConst1 == true)
		{
			*code << CodeItem().movBitConst(writeAddr, 1, QString("compute or @%1 (optimized) %2 <= 1").
											arg(ualAfb->label()).
											arg(outSignal->refSignalIDsJoined()));
			return true;
		}

		if (allInputsConst0 == true)
		{
			*code << CodeItem().movBitConst(writeAddr, 0, QString("compute or @%1 (optimized) %2 <= 0").
											arg(ualAfb->label()).
											arg(outSignal->refSignalIDsJoined()));
			return true;
		}

		//

		if (inSignals.size() == 1)
		{
			const UalSignal* inSignal = inSignals.begin()->first;
			Address16 readAddr = inSignals.begin()->second;

			*code << CodeItem().movBit(writeAddr, readAddr, QString("compute or @%1 (optimized) %2 <= %3").
									   arg(ualAfb->label()).
									   arg(outSignal->refSignalIDsJoined()).
									   arg(inSignal->refSignalIDsJoined()));
			return true;
		}

		*code << CodeItem().resetAcc();

		for(auto const& [inSignal, readAddr] : inSignals)
		{
			*code << CodeItem().movBitAccAddr(readAddr, QString("ACC <= %1").arg(inSignal->refSignalIDsJoined()));
		}

		*code << CodeItem().orAcc(QString("compute or @%1").arg(ualAfb->label()));
		*code << CodeItem().movBitAddrAcc(writeAddr, QString("%1 <= ACC[0]").arg(outSignal->refSignalIDsJoined()));

		return true;
	}

	bool ModuleLogicCompiler::generateAfbBitAccAndCode(CodeSnippet* code, const UalAfb* ualAfb, bool* result)
	{
		TEST_PTR_RETURN_FALSE(code);
		TEST_PTR_RETURN_FALSE(ualAfb);
		TEST_PTR_RETURN_FALSE(result);

		*result = true;

		std::vector<std::pair<const UalSignal*, Address16>> inSignals;	// pair<ualSignal, readAddress>

		const std::vector<SchemaPin>& inputs = ualAfb->inputs();

		bool hasConst0 = false;
		bool allInputsConst1 = true;

		for(const SchemaPin& inPin : inputs)
		{
			UalSignal* inSignal = getUalSignalByPinCaption(ualAfb, inPin.caption(), true);

			if (inSignal == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for '%1' pin of AFB %2 (schema %3)").
													arg(inPin.caption()).arg(ualAfb->label()).arg(ualAfb->schemaID()));
				*result = false;
				return false;
			}

			if (inSignal->isConstDiscrete() == true)
			{
				if (inSignal->constDiscreteValue() == 0)
				{
					hasConst0 = true;
				}

				continue;			// skip const 1 value
			}

			if (inSignal->isDiscrete() == true)
			{
				allInputsConst1 = false;

				Address16 readAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);

				if(readAddr.isValid() == false)
				{
					// Undefined UAL address of signal %1 (Logic schema %2).
					//
					m_log->errALC5105(inSignal->appSignalID(), inSignal->ualItemGuid(), inSignal->ualItemSchemaID());
					*result = false;
					return false;
				}

				inSignals.emplace_back(inSignal, readAddr);
				continue;
			}

			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), inSignal->ualItemGuid(), inSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		if (isOutConnectedToTerminatorOnly(ualAfb) == true)
		{
			*result = true;
			return true;		// report like a "code generated"
		}

		UalSignal* outSignal = getUalSignalByPinCaption(ualAfb, Afb::OUT_PIN_CAPTION, false);

		if (outSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal not found for 'out' pin of AFB %1 (schema %2)").
												arg(ualAfb->label()).arg(ualAfb->schemaID()));
			*result = false;
			return false;
		}

		if (outSignal->isDiscrete() == false)
		{
			// Uncompatible signals connection (Logic schema '%1').
			//
			m_log->errALC5117(ualAfb->guid(), ualAfb->label(), outSignal->ualItemGuid(), outSignal->ualItemLabel(), ualAfb->schemaID());
			*result = false;
			return false;
		}

		Address16 writeAddr = m_ualSignals.getSignalWriteAddress(*outSignal);

		if(writeAddr.isValid() == false)
		{
			// Undefined UAL address of signal %1 (Logic schema %2).
			//
			m_log->errALC5105(outSignal->appSignalID(), outSignal->ualItemGuid(), outSignal->ualItemSchemaID());
			*result = false;
			return false;
		}

		//

		if (hasConst0 == true)
		{
			*code << CodeItem().movBitConst(writeAddr, 0, QString("compute and @%1 (optimized) %2 <= 0").
											arg(ualAfb->label()).
											arg(outSignal->refSignalIDsJoined()));
			return true;
		}

		if (allInputsConst1 == true)
		{
			*code << CodeItem().movBitConst(writeAddr, 1, QString("compute and @%1 (optimized) %2 <= 1").
											arg(ualAfb->label()).
											arg(outSignal->refSignalIDsJoined()));
			return true;
		}

		//

		if (inSignals.size() == 1)
		{
			const UalSignal* inSignal = inSignals.begin()->first;
			Address16 readAddr = inSignals.begin()->second;

			*code << CodeItem().movBit(writeAddr, readAddr, QString("compute and @%1 (optimized) %2 <= %3").
									   arg(ualAfb->label()).
									   arg(outSignal->refSignalIDsJoined()).
									   arg(inSignal->refSignalIDsJoined()));
			return true;
		}

		*code << CodeItem().setAcc();

		for(const auto& [inSignal, readAddr] : inSignals)
		{
			*code << CodeItem().movBitAccAddr(readAddr, QString("ACC <= %1").arg(inSignal->refSignalIDsJoined()));
		}

		*code << CodeItem().andAcc(QString("compute and @%1").arg(ualAfb->label()));
		*code << CodeItem().movBitAddrAcc(writeAddr, QString("%1 <= ACC[0]").arg(outSignal->refSignalIDsJoined()));

		return true;
	}

	bool ModuleLogicCompiler::generateInvertDiscreteInputsCode(CodeSnippet* code,
															   const QVector<UalSignal*>& inDiscreteSignals,
															   const QString& comment)
	{
		TEST_PTR_RETURN_FALSE(code);

		if (inDiscreteSignals.isEmpty())
		{
			return true;
		}

		bool result = true;

		// map of inSignals arrays placed in same offset
		//
		std::map<int, std::vector<const UalSignal*>> offsetSignals;		// inSignal->ualAddr().offset() => vector of inSignal

		// sort signals by offsets
		//
		for(const UalSignal* inSignal : inDiscreteSignals)
		{
			TEST_PTR_CONTINUE(inSignal);

			if (inSignal->ioBufAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal %1 ioBufAddr is NOT valid!").arg(inSignal->appSignalID()));
				result = false;
				continue;
			}

			if (inSignal->ualAddr().isValid() == false)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal %1 ualAddr is NOT valid!").arg(inSignal->appSignalID()));
				result = false;
				continue;
			}

			int offset = inSignal->ualAddr().offset();

			auto it = offsetSignals.find(offset);

			if (it == offsetSignals.end())
			{
				auto [new_it, b] = offsetSignals.emplace(offset, std::vector<const UalSignal*>{});

				it = new_it;
			}

			auto& offsetInSignals = it->second;

			offsetInSignals.push_back(inSignal);

			if (offsetInSignals.size() > SIZE_16BIT)
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
			}
		}

		RETURN_IF_FALSE(result);

		const FbConv* fbBusNot = nullptr;
		const UalAfb* afbBusNot = nullptr;

		if (m_bitAccAvailable == false)
		{
			auto it = m_fbConv.find(Afb::AFB_BUS_NOT);

			if (it == m_fbConv.end())
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			fbBusNot = &it->second;

			if (fbBusNot->ualAfbs.size() != 1)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			afbBusNot = *fbBusNot->ualAfbs.begin();

			if (afbBusNot == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}
		}

		RETURN_IF_FALSE(result);

		int bitAccAddr = bitAccumulatorAddress();
		CodeItem cmd;

		code->comment_nl(comment);

		for(auto& [offset, inSignals] : offsetSignals)
		{
			// sort iSignals by ualAddr().bitAddress() ascending
			//
			std::sort(	inSignals.begin(), inSignals.end(),
						[](const UalSignal* a, const UalSignal* b)
						{
							return a->ualAddr().bitAddress() < b->ualAddr().bitAddress();
						});

			// sequential signals placement checking
			//
			int bitNo = -1;

			for(const UalSignal* inSignal : inSignals)
			{
				int ualAddrBitNo = inSignal->ualAddr().bit();

				if (bitNo == -1)
				{
					result &= (ualAddrBitNo == 0);
				}
				else
				{
					result &= (ualAddrBitNo == bitNo + 1);
				}

				Q_ASSERT(result);

				bitNo = ualAddrBitNo;
			}

			RETURN_IF_FALSE(result);

			//

			if (m_bitAccAvailable == true)
			{
				// generation of code that use bit accumulator
				//
				if (inSignals.size() < SIZE_16BIT)
				{
					*code << cmd.resetAcc();
				}

				auto reverseIt = inSignals.rbegin();

				while(reverseIt != inSignals.rend())
				{
					*code << cmd.movBitAccAddr((*reverseIt)->ioBufAddr(),
												  QString("acc <= %1").arg((*reverseIt)->appSignalID()));
					reverseIt++;
				}

				*code << cmd.notAcc();
				*code << cmd.movAddrAcc(offset);
			}
			else
			{
				// old style code generation (AFB BUS_NOT used)
				//
				int bitNo2 = 0;

				if (inSignals.size() < SIZE_16BIT)
				{
					*code << cmd.movConst(bitAccAddr, 0);
				}

				for(const UalSignal* inSignal : inSignals)
				{
					*code << cmd.movBit(Address16(bitAccAddr, bitNo2), inSignal->ioBufAddr());
					bitNo2++;
				}

				*code << cmd.writeFuncBlock(afbBusNot->opcode(),
										   afbBusNot->instance(),
										   fbBusNot->inputSignalIndex,
										   bitAccAddr,
										   afbBusNot->caption());

				*code << cmd.startafb(afbBusNot->opcode(),
									 afbBusNot->instance(),
									 afbBusNot->caption(),
									 afbBusNot->runTime());

				*code << cmd.readFuncBlock(offset,
										  afbBusNot->opcode(),
										  afbBusNot->instance(),
										  fbBusNot->outputSignalIndex,
										  afbBusNot->caption());
			}

			code->newLine();
		}

		return result;
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

	bool ModuleLogicCompiler::getPinsAndSignalsBusSizes(const UalAfb* ualAfb, const std::vector<SchemaPin>& pins,
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

		for(const SchemaPin& pin : pins)
		{
			AfbSignal afbSignal;

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

		code->comment_nl(QString("BusComposer %1 processing (BusType %2) start").
									arg(ualItem->label()).
									arg(bus->busTypeID()));

		BusFilling busFilling(bus);

		// std::map<inbusOffset, std::map<bitNo, std::pair<inputSignal, busChildSignal>>>
		//
		std::map<int, std::map<int, std::pair<UalSignal*, UalSignal*>>> busDiscretes;

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
			case E::SignalType::Discrete:
				{
					//res = generateDiscreteSignalToBusDiscreteInputCode(code, inputSignal, busChildSignal, busSignal);

					auto it = busDiscretes.find(busSignal.inbusOffset());

					if (it == busDiscretes.end())
					{
						auto p = busDiscretes.insert({busSignal.inbusOffset(), {}});
						it = p.first;
					}

					it->second.insert({busSignal.inbusAddr.bit(), {inputSignal, busChildSignal}});
				}
				break;

			case E::SignalType::Analog:
				res = generateAnalogSignalToBusAnalogInputCode(code, inputSignal, busChildSignal, busSignal, ualItem->label(), &busFilling);
				break;

			case E::SignalType::Bus:
				{
					switch(inputSignal->signalType())
					{
					case E::SignalType::Discrete:
						res = generateDiscreteSignalToBusBusInputCode(code, inputSignal, busChildSignal, busSignal, &busFilling);
						break;

					case E::SignalType::Bus:
						res = generateBusSignalToBusBusInputCode(code, inputSignal, busChildSignal, busSignal, &busFilling);
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

			if (res == false)
			{
				result = false;
			}
		}

		result &= generateDiscreteSignalsToBusDiscreteInputsCode(code, busDiscretes, *ualBusSignal, &busFilling);

		clearUnusedBusSpace(code, *ualBusSignal, busFilling);

		code->finalizeByNewLine();
		code->comment_nl(QString("BusComposer %1 processing (BusType %2) end").
									arg(ualItem->label()).
									arg(bus->busTypeID()));
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

		const std::vector<SchemaPin>& outputs = composerItem->outputs();

		if (outputs.size() != 1)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const SchemaPin& output = outputs[0];

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

	bool ModuleLogicCompiler::generateAnalogSignalToBusAnalogInputCode(CodeSnippet* code,
																	   const UalSignal* inputSignal,
																	   const UalSignal* busChildSignal,
																	   const BusSignal& busSignal,
																	   const QString& busComposerLabel,
																	   BusFilling* busFilling)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busFilling, m_log);

		Q_ASSERT(busChildSignal->ualAddrIsValid() == true);
		Q_ASSERT(busChildSignal->ualAddr().bit() == 0);

		if (busSignal.conversionRequired() == true)
		{
			bool res = generateInbusConversionCode(code, inputSignal, busChildSignal, busSignal, busComposerLabel);

			if (res == true)
			{
				busFilling->fill(busSignal.inbusOffset(), busSignal.inbusSizeBits / 16);
			}
			return res;
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

		busFilling->fillDword(busSignal.inbusOffset());

		return true;
	}

	bool ModuleLogicCompiler::generateInbusConversionCode(CodeSnippet* code,
															const UalSignal* inputSignal,
															const UalSignal* busChildSignal,
															const BusSignal& busSignal,
															const QString& busComposerLabel)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);

		const UalSignal* parentBusSignal = busChildSignal->getParentBusSignal();

		if (parentBusSignal == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		InbusConvDescription convDesc = busSignal.getInbusConvDescription();

		if (convDesc.isValid() == false)
		{
			// Unknown conversion of signal %1 to inbus signal %2 (Logic schema %3)
			//
			m_log->errALC5153(inputSignal->appSignalID(), parentBusSignal->appSignalID() + "." + busSignal.caption,
								busChildSignal->ualItemSchemaID());
			return false;
		}

		if (parentBusSignal->ualAddrIsValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(parentBusSignal->appSignalID(),
							  parentBusSignal->ualItemGuid(),
							  parentBusSignal->ualItemSchemaID());
			return false;
		}

		Address16 inbusSignalAddr = parentBusSignal->ualAddr();

		inbusSignalAddr.addWord(busSignal.inbusAddr.offset());

		bool scalingRequired = busSignal.scalingRequired();
		bool typeConvRequired = busSignal.typeConversionRequired();
		bool byteOrderConvRequired = busSignal.byteOrderConversionRequired();

		bool saveResultToAccumulator = false;
		bool readValueFromAccumulator = false;

		bool result = true;

		CodeSnippet inbusConvCode;

		if (scalingRequired == true)
		{
			saveResultToAccumulator = convDesc.inbusTypeConvAfterScalingRequired == true ||
										byteOrderConvRequired == true;

			result &= genInbusScalingCode(&inbusConvCode, inputSignal, busChildSignal, busSignal, busComposerLabel,
										  convDesc, false, saveResultToAccumulator, inbusSignalAddr);

			readValueFromAccumulator = saveResultToAccumulator;

			if (convDesc.inbusTypeConvAfterScalingRequired == true)
			{

				saveResultToAccumulator = byteOrderConvRequired;

				result &= genInbusTypeConversionCode(&inbusConvCode, inputSignal, busChildSignal, busSignal, busComposerLabel,
													 convDesc, readValueFromAccumulator, saveResultToAccumulator,
													 inbusSignalAddr);

				readValueFromAccumulator = saveResultToAccumulator;
			}
		}
		else
		{
			if (typeConvRequired == true )
			{
				saveResultToAccumulator = byteOrderConvRequired;

				result &= genInbusTypeConversionCode(&inbusConvCode, inputSignal, busChildSignal, busSignal, busComposerLabel,
													 convDesc, readValueFromAccumulator, saveResultToAccumulator,
													 inbusSignalAddr);

				readValueFromAccumulator = saveResultToAccumulator;
			}
		}

		if (byteOrderConvRequired == true)
		{
			result &= genInbusByteOrderConversionCode(&inbusConvCode, inputSignal, busChildSignal, busSignal, busComposerLabel,
													convDesc, readValueFromAccumulator, false /* always save to inBusSignal */,
													inbusSignalAddr);
		}

		if (result == true)
		{
			code->comment_nl(QString("Inbus conversion code for signal %1 -> %2.%3").
							 arg(inputSignal->appSignalID()).
							 arg(parentBusSignal->appSignalID()).arg(busSignal.caption));
			code->append(inbusConvCode);

			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::genInbusScalingCode(CodeSnippet* code,
													const UalSignal* inputSignal,
													const UalSignal* busChildSignal,
													const BusSignal& busSignal,
													const QString& busComposerLabel,
													const InbusConvDescription& convDesc,
													bool readValueFromAccumulator,
													bool saveResultToAccumulator,
													const Address16& inbusSignalAddr)
	{
		if (readValueFromAccumulator == true)
		{
			// readValueFromAccumulator must be FALSE
			//
			LOG_INTERNAL_ERROR(m_log);
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(busSignal.scalingRequired() == true);

		QString scaleAfbCaption = convDesc.inbusScalingAfb;

		if (scaleAfbCaption.isEmpty() == true)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't find AFB for scaling from %1 to bus child signal %2").
											arg(inputSignal->appSignalID()).arg(busChildSignal->appSignalID()));
			return false;
		}

		std::shared_ptr<Afb::AfbElement> scaleElem = m_lmDescription->afbElement(scaleAfbCaption);

		if (scaleElem == nullptr)
		{
			// Required AFB %1 is missing.
			//
			m_log->errALC5174(scaleAfbCaption, QUuid());
			return false;
		}

		QString errMsg;

		UalItem scaleItem(scaleElem, errMsg);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
			return false;
		}

		scaleItem.setLabel(busComposerLabel);

		bool result = true;

		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_X1, busSignal.inOutAnalogLowLimit);
		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_X2, busSignal.inOutAnalogHighLimit);

		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_Y1, busSignal.inbusAnalogLowLimit);
		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_Y2, busSignal.inbusAnalogHighLimit);

		UalAfb* scale = createUalAfb(scaleItem);

		if (scale == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		RETURN_IF_FALSE(result);

		// No param initialization code required here.
		// Params initialization code will be generated in generateInitAfbsCode.

		AfbSignal inSignal;
		AfbSignal outSignal;

		result &= scale->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &inSignal);
		result &= scale->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &outSignal);

		RETURN_IF_FALSE(result);

		Address16 accAddr(wordAccumulatorAddress(), 0);

		if (readValueFromAccumulator == true)
		{
			result &= generateSignalToAfbInputCode(code, scale, inSignal, nullptr, BusProcessingStepInfo(),	accAddr, false);
		}
		else
		{
			result &= generateSignalToAfbInputCode(code, scale, inSignal, inputSignal,
												   BusProcessingStepInfo(), Address16(), false);
		}

		result &= startAfb(code, scale, BusProcessingStepInfo());

		if (saveResultToAccumulator == true)
		{
			result &= generateAfbOutputToSignalCode(code, scale, outSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateAfbOutputToSignalCode(code, scale, outSignal, nullptr,
													BusProcessingStepInfo(), inbusSignalAddr, true);
		}

		return result;
	}

	bool ModuleLogicCompiler::genInbusTypeConversionCode(CodeSnippet* code,
														const UalSignal* inputSignal,
														const UalSignal* busChildSignal,
														const BusSignal& busSignal,
														const QString& busComposerLabel,
														const InbusConvDescription& convDesc,
														bool readValueFromAccumulator,
														bool saveResultToAccumulator,
														const Address16& inbusSignalAddr)
	{
		Q_ASSERT(busSignal.typeConversionRequired() == true);
		Q_ASSERT(inputSignal->isAnalog() == true);

		QString tconvAfbCaption = convDesc.inbusTypeConversionAfb;

		if (tconvAfbCaption.isEmpty() == true)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AFB is't assigned for type conversion from %1 to bus child signal %2").
											arg(inputSignal->appSignalID()).arg(busChildSignal->appSignalID()));
			return false;
		}

		Address16 accAddr(wordAccumulatorAddress(), 0);

		if (tconvAfbCaption == Afb::SW_TCONV_SI32_SI16 ||
			tconvAfbCaption == Afb::SW_TCONV_SI32_UI16 ||
			tconvAfbCaption == Afb::SW_TCONV_FP32_SI16 ||
			tconvAfbCaption == Afb::SW_TCONV_FP32_UI16)
		{
			// software type conversion
			//
			CodeItem cmd;

			if (tconvAfbCaption == Afb::SW_TCONV_FP32_SI16 ||
				tconvAfbCaption == Afb::SW_TCONV_FP32_UI16)
			{
				bool res = genInbusTconvCode(code, inputSignal, busChildSignal, busSignal, busComposerLabel, Afb::TCONV_FP32_SI32,
										readValueFromAccumulator, true, inbusSignalAddr);

				RETURN_IF_FALSE(res);

				readValueFromAccumulator = true;
			}

			Address16 writeAddr = inbusSignalAddr;

			if (saveResultToAccumulator == true)
			{
				writeAddr = accAddr;
			}

			if (readValueFromAccumulator == true)
			{
				cmd.mov(writeAddr.offset(), accAddr.offset() + 1);			// read low word
				*code << cmd;
			}
			else
			{
				if (inputSignal->isConst() == true)
				{
					quint16 constValue = 0;

					switch(inputSignal->analogSignalFormat())
					{
					case E::AnalogAppSignalFormat::SignedInt32:
						constValue = inputSignal->constAnalogIntValue() & 0xFFFF;					// get low word of const

						cmd.setComment(QString("%1 <= %2 (low word of %3)").
									   arg(busChildSignal->refSignalIDsJoined()).
									   arg(constValue).
									   arg(inputSignal->constAnalogIntValue()));
						break;

					case E::AnalogAppSignalFormat::Float32:
						constValue = static_cast<qint32>(inputSignal->constAnalogFloatValue()) & 0xFFFF;

						cmd.setComment(QString("%1 <= %2 (low word of int(%3))").
									   arg(busChildSignal->refSignalIDsJoined()).
									   arg(constValue).
									   arg(inputSignal->constAnalogFloatValue()));
						break;

					default:
						Q_ASSERT(false);
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					cmd.movConst(writeAddr.offset(), constValue);
				}
				else
				{
					cmd.mov(writeAddr.offset(),
							inputSignal->ualAddr().offset() + 1);	// move low word of inputSignal only
					cmd.setComment(QString("%1 <= low word of %2").
								   arg(busChildSignal->refSignalIDsJoined()).
								   arg(inputSignal->refSignalIDsJoined()));
				}

				*code << cmd;
			}

			return true;
		}

		bool result = genInbusTconvCode(code, inputSignal, busChildSignal, busSignal, busComposerLabel, tconvAfbCaption,
								   readValueFromAccumulator, saveResultToAccumulator, inbusSignalAddr);

		return result;
	}

	bool ModuleLogicCompiler::genInbusTconvCode(CodeSnippet* code,
										   const UalSignal* inputSignal,
										   const UalSignal* busChildSignal,
										   const BusSignal& busSignal,
										   const QString& busComposerLabel,
										   const QString& tconvAfbCaption,
										   bool readValueFromAccumulator,
										   bool saveResultToAccumulator,
										   const Address16& inbusSignalAddr)
	{
		Q_UNUSED(busSignal);

		if (tconvAfbCaption.isEmpty() == true)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AFB is't assigned for type conversion from %1 to bus child signal %2").
											arg(inputSignal->appSignalID()).arg(busChildSignal->appSignalID()));
			return false;
		}

		std::shared_ptr<Afb::AfbElement> tconvElem = m_lmDescription->afbElement(tconvAfbCaption);

		if (tconvElem == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Can't find AFB '%1' for type conversion from %2 to bus child signal %3").
											arg(tconvAfbCaption).
											arg(inputSignal->appSignalID()).
											arg(busChildSignal->appSignalID()));
			return false;
		}

		QString errMsg;

		UalItem tconvItem(tconvElem, errMsg);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
			return false;
		}

		tconvItem.setLabel(busComposerLabel);

		UalAfb* tconv = createUalAfb(tconvItem);

		if (tconv == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Address16 accAddr(wordAccumulatorAddress(), 0);

		bool result = true;

		AfbSignal inSignal;
		AfbSignal outSignal;

		result &= tconv->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &inSignal);
		result &= tconv->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &outSignal);

		RETURN_IF_FALSE(result);

		if (readValueFromAccumulator == true)
		{
			result &= generateSignalToAfbInputCode(code, tconv, inSignal, nullptr, BusProcessingStepInfo(),	accAddr, false);
		}
		else
		{
			result &= generateSignalToAfbInputCode(code, tconv, inSignal, inputSignal,
												   BusProcessingStepInfo(), Address16(), false);
		}

		result &= startAfb(code, tconv, BusProcessingStepInfo());

		if (saveResultToAccumulator == true)
		{
			result &= generateAfbOutputToSignalCode(code, tconv, outSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateAfbOutputToSignalCode(code, tconv, outSignal, nullptr,
													BusProcessingStepInfo(), inbusSignalAddr, true);
		}

		return result;
	}

	bool ModuleLogicCompiler::genInbusByteOrderConversionCode(CodeSnippet* code,
															const UalSignal* inputSignal,
															const UalSignal* busChildSignal,
															const BusSignal& busSignal,
															const QString& busComposerLabel,
															const InbusConvDescription& convDesc,
															bool readValueFromAccumulator,
															bool saveResultToAccumulator,
															const Address16& inbusSignalAddr)
	{
		Q_UNUSED(busChildSignal);
		Q_UNUSED(convDesc);

		if (saveResultToAccumulator == true)
		{
			// saveResultToAccumulator must be FALSE
			//
			LOG_INTERNAL_ERROR(m_log);
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(busSignal.byteOrderConversionRequired() == true);

		QString boTconvCaption;

		switch(busSignal.inbusSizeBits)
		{
		case SIZE_16BIT:
			boTconvCaption = Afb::TCONV_BO_16;
			break;

		case SIZE_32BIT:
			boTconvCaption = Afb::TCONV_BO_32;
			break;

		default:
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		std::shared_ptr<Afb::AfbElement> boTconvElem = m_lmDescription->afbElement(boTconvCaption);

		if (boTconvElem == nullptr)
		{
			// Required AFB %1 is missing.
			//
			m_log->errALC5174(boTconvCaption, QUuid());
			return false;
		}

		QString errMsg;

		UalItem boTconvItem(boTconvElem, errMsg);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
			return false;
		}

		boTconvItem.setLabel(busComposerLabel);

		UalAfb* boTconv = createUalAfb(boTconvItem);

		if (boTconv == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		AfbSignal inSignal;
		AfbSignal outSignal;

		result &= boTconv->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &inSignal);
		result &= boTconv->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &outSignal);

		RETURN_IF_FALSE(result);

		Address16 accAddr(wordAccumulatorAddress(), 0);

		if (readValueFromAccumulator == true)
		{
			result &= generateSignalToAfbInputCode(code, boTconv, inSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateSignalToAfbInputCode(code, boTconv, inSignal, inputSignal,
												   BusProcessingStepInfo(), Address16(), true);
		}

		result &= startAfb(code, boTconv, BusProcessingStepInfo());

		if (saveResultToAccumulator == true)
		{
			result &= generateAfbOutputToSignalCode(code, boTconv, outSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateAfbOutputToSignalCode(code, boTconv, outSignal, nullptr,
													BusProcessingStepInfo(), inbusSignalAddr, true);
		}

		return result;
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
//			if (inputSignal->constDiscreteValue() != 0)
//			{
//				cmd.movBitConst(busChildSignal->ualAddr(), inputSignal->constDiscreteValue());
//				cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignal->constDiscreteValue()));
//				code->append(cmd);
//			}

			cmd.movBitConst(busChildSignal->ualAddr(), inputSignal->constDiscreteValue());
			cmd.setComment(QString("%1 <= %2").arg(busChildSignalIDs).arg(inputSignal->constDiscreteValue()));
			code->append(cmd);
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

	bool ModuleLogicCompiler::generateDiscreteSignalsToBusDiscreteInputsCode(CodeSnippet* code,
			const std::map<int, std::map<int, std::pair<UalSignal*, UalSignal*>>>& busDiscretes,
			const UalSignal& busSignal, BusFilling* busFilling)
	{
		// std::map<inbusOffset, std::map<bitNo, std::pair<inputSignal, busChildSignal>>>

		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busFilling, m_log);

		bool result = true;

		int busSignalUalAddrOffset = m_ualSignals.getSignalWriteAddress(busSignal).offset();

		for(const auto& [inbusOffset, offsetDiscretes] : busDiscretes)
		{
			if (offsetDiscretes.empty() == true)
			{
				Q_ASSERT(false);
				continue;
			}

			//std::map<Address16, std::tuple<const UalSignal*, Address16, QString>> srcSignals;
			CopyBitsMap srcSignals;

			int busChildSignalsOffset = -1;

			for(const auto& [bitNo, inBusSignals] : offsetDiscretes)
			{
				const auto [inSignal, busChildSignal] = inBusSignals;

				if (inSignal == nullptr || busChildSignal == nullptr)
				{
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				if (busChildSignalsOffset == -1)
				{
					busChildSignalsOffset = busSignalUalAddrOffset + inbusOffset;
				}

				CopyBitInfo cbi;

				cbi.ualSignal = inSignal;

				if (inSignal->isConstDiscrete() == false)
				{
					cbi.srcBitAddr = m_ualSignals.getSignalReadAddress(*inSignal, true);
				}

				cbi.comment = QString("%1 <= %2").
										arg(busChildSignal->appSignalID()).
										arg(inSignal->refSignalIDsJoined());

				srcSignals.emplace(busChildSignal->ualAddr(), cbi);
			}

			result &= codeCopyBits(code, busChildSignalsOffset, srcSignals);
			code->newLine();

			busFilling->fillWord(inbusOffset);
		}

		return true;
	}

	void ModuleLogicCompiler::clearUnusedBusSpace(CodeSnippet* code,
												const UalSignal& busSignal,
												const BusFilling& busFilling)
	{
		int busSignalAddr = busSignal.ualAddr().offset();

		std::vector<std::pair<int, int>> unfilledAreas;

		busFilling.getUnfilled(&unfilledAreas);

		for (auto const& p : unfilledAreas)
		{
			int startAddr = p.first;
			int sizeW = p.second;

			*code << codeSetMemory(busSignalAddr + startAddr, 0, sizeW);
		}
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToBusBusInputCode(CodeSnippet* code,
																	  UalSignal* inputSignal,
																	  UalSignal* busChildSignal,
																	  const BusSignal& busSignal,
																	  BusFilling* busFilling)
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

			cmd.fillb(wordAccAddr, inputSignalAddr.offset(), inputSignalAddr.bit());
			cmd.setComment(QString("%1 <= %2").arg(busChildSignal->appSignalID()).arg(inputSignal->appSignalID()));

			code->append(cmd);

			if (busSizeW > 1)
			{
				cmd.fillb(wordAccAddr + 1, inputSignalAddr.offset(), inputSignalAddr.bit());
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

		busFilling->fill(busSignal.inbusOffset(), busSizeW);
		return true;
	}

	bool ModuleLogicCompiler::generateBusSignalToBusBusInputCode(CodeSnippet* code,
																 UalSignal* inputSignal,
																 UalSignal* busChildSignal,
																 const BusSignal& busSignal,
																 BusFilling* busFilling)
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

		busFilling->fill(busSignal.inbusOffset(), busSizeW);

		return true;
	}

	bool ModuleLogicCompiler::generateBusExtractorCode(CodeSnippet* code, const UalItem* ualItem)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);

		const SchemaBusExtractor* busExtractor = ualItem->schemaBusExtractor();

		if (busExtractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<SchemaPin>& inputs = busExtractor->inputs();

		if (inputs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const SchemaPin& inPin = inputs[0];

		const std::vector<QUuid>& associatedOutPins = inPin.associatedIOs();

		if (associatedOutPins.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		UalSignal* inputSignal = m_ualSignals.get(associatedOutPins[0]);

		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);

		if (inputSignal->isBus() == true)
		{
			return generateBusExtractorCode(code, ualItem, inputSignal);
		}

		if (inputSignal->isDiscrete() == true)
		{
			return generateDiscreteSignalToBusExtractorCode(code, ualItem, inPin, inputSignal);
		}

		LOG_INTERNAL_ERROR(m_log);
		return false;
	}

	bool ModuleLogicCompiler::generateBusExtractorCode(CodeSnippet* code,
													   const UalItem* ualItem,
													   UalSignal* inputBusSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputBusSignal, m_log);

		if (inputBusSignal->isBus() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		BusShared bus = getBusShared(inputBusSignal->busTypeID());

		if (bus == nullptr)
		{
			// Bus type ID %1 of signal %2 is undefined.
			//
			m_log->errALC5092(inputBusSignal->busTypeID(), inputBusSignal->appSignalID());
			return false;
		}

		const QVector<BusSignal>& busSignals = bus->busSignals();

		bool result = true;

		for(const BusSignal& bs : busSignals)
		{
			UalSignal* busChildSignal = inputBusSignal->getBusChildSignal(bs.signalID);

			if (busChildSignal == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("UAL bus child signal %1.%2 isn't found"));
				result = false;
				continue;
			}

			if (busChildSignal->isFrombusConversionRequired() == false ||
				busChildSignal->isFrombusConversionCodeAlreadyGenerated() == true)
			{
				continue;
			}

			result &= generateFrombusConversionCode(code, inputBusSignal, bs, busChildSignal, ualItem->label());
		}

		return result;
	}

	bool ModuleLogicCompiler::generateFrombusConversionCode(CodeSnippet* code,
														   const UalSignal* inputBusSignal,
														   const BusSignal& busSignal,
														   UalSignal* busChildSignal,
														   const QString& busExtractorLabel)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputBusSignal, m_log);
		TEST_PTR_LOG_RETURN_FALSE(busChildSignal, m_log);

		InbusConvDescription convDesc = busSignal.getInbusConvDescription();

		if (convDesc.isValid() == false)
		{
			// Unknown conversion from inbus signal %1 to app signal %2 (Logic schema %3)
			//
			m_log->errALC5196(busChildSignal->appSignalID(), inputBusSignal->appSignalID() + "." + busSignal.signalID,
							  busChildSignal->ualItemSchemaID());
			return false;
		}

		if (inputBusSignal->ualAddrIsValid() == false)
		{
			// Undefined UAL address of signal '%1' (Logic schema '%2').
			//
			m_log->errALC5105(inputBusSignal->appSignalID(),
							  inputBusSignal->ualItemGuid(),
							  inputBusSignal->ualItemSchemaID());
			return false;
		}

		Address16 inbusSignalAddr = inputBusSignal->ualAddr();

		inbusSignalAddr.addWord(busSignal.inbusAddr.offset());

		bool scalingRequired = busSignal.scalingRequired();
		bool typeConvRequired = busSignal.typeConversionRequired();
		bool byteOrderConvRequired = busSignal.byteOrderConversionRequired();

		bool readValueFromAccumulator = false;
		bool saveResultToAccumulator = false;

		bool result = true;

		CodeSnippet frombusConvCode;

		if (byteOrderConvRequired == true)
		{
			saveResultToAccumulator = scalingRequired || typeConvRequired;

			result &= genFrombusByteOrderConversionCode(&frombusConvCode, inputBusSignal, busSignal, busChildSignal,
														busExtractorLabel, convDesc, readValueFromAccumulator, saveResultToAccumulator,
														inbusSignalAddr);

			readValueFromAccumulator = saveResultToAccumulator;
		}

		if (scalingRequired == true)
		{
			if (convDesc.frombusTypeConvBeforeScalingRequired == true)
			{
				saveResultToAccumulator = true;		// for subsequent scaling

				result &= genFrombusTypeConversionCode(&frombusConvCode, inputBusSignal, busSignal, busChildSignal,
													   busExtractorLabel, convDesc, readValueFromAccumulator,
													   saveResultToAccumulator, inbusSignalAddr);

				readValueFromAccumulator = true;
			}

			result &= genFrombusScalingCode(&frombusConvCode, inputBusSignal, busSignal, busChildSignal,
											busExtractorLabel, convDesc, readValueFromAccumulator, false, inbusSignalAddr);
		}
		else
		{
			if (typeConvRequired == true )
			{
				saveResultToAccumulator = false;

				result &= genFrombusTypeConversionCode(&frombusConvCode, inputBusSignal, busSignal, busChildSignal,
													   busExtractorLabel, convDesc, readValueFromAccumulator,
													   saveResultToAccumulator, inbusSignalAddr);
			}
		}

		if (result == true)
		{
			code->comment_nl(QString("Frombus coversion code for signal %1.%2 -> %3").
								arg(inputBusSignal->appSignalID()).arg(busSignal.signalID).
								arg(busChildSignal->refSignalIDsJoined()));
			code->append(frombusConvCode);

			code->newLine();

			busChildSignal->setFrombusConversionCodeIsAlreadyGenerated();
		}

		return result;
	}

	bool ModuleLogicCompiler::genFrombusByteOrderConversionCode(CodeSnippet* code,
																const UalSignal* inputBusSignal,
																const BusSignal& busSignal,
																const UalSignal* busChildSignal,
																const QString& busExtractorLabel,
																const InbusConvDescription& convDesc,
																bool readValueFromAccumulator,
																bool saveResultToAccumulator,
																const Address16& inbusSignalAddr)
	{
		Q_UNUSED(inputBusSignal);
		Q_UNUSED(convDesc);

		if (readValueFromAccumulator == true)
		{
			// readValueFromAccumulator must be FALSE
			//
			LOG_INTERNAL_ERROR(m_log);
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(busSignal.byteOrderConversionRequired() == true);

		QString boTconvCaption;

		switch(busSignal.inbusSizeBits)
		{
		case SIZE_16BIT:
			boTconvCaption = Afb::TCONV_BO_16;
			break;

		case SIZE_32BIT:
			boTconvCaption = Afb::TCONV_BO_32;
			break;

		default:
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		std::shared_ptr<Afb::AfbElement> boTconvElem = m_lmDescription->afbElement(boTconvCaption);

		if (boTconvElem == nullptr)
		{
			// Required AFB %1 is missing.
			//
			m_log->errALC5174(boTconvCaption, QUuid());
			return false;
		}

		QString errMsg;

		UalItem boTconvItem(boTconvElem, errMsg);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
			return false;
		}

		boTconvItem.setLabel(busExtractorLabel);

		UalAfb* boTconv = createUalAfb(boTconvItem);

		if (boTconv == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		AfbSignal inSignal;
		AfbSignal outSignal;

		result &= boTconv->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &inSignal);
		result &= boTconv->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &outSignal);

		RETURN_IF_FALSE(result);

		Address16 accAddr(wordAccumulatorAddress(), 0);

		if (readValueFromAccumulator == true)
		{
			result &= generateSignalToAfbInputCode(code, boTconv, inSignal, nullptr,
												   BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateSignalToAfbInputCode(code, boTconv, inSignal, nullptr,
												   BusProcessingStepInfo(), inbusSignalAddr, true);
		}

		result &= startAfb(code, boTconv, BusProcessingStepInfo());

		if (saveResultToAccumulator == true)
		{
			result &= generateAfbOutputToSignalCode(code, boTconv, outSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateAfbOutputToSignalCode(code, boTconv, outSignal, busChildSignal,
													BusProcessingStepInfo(), Address16(), true);
		}

		return result;
	}

	bool ModuleLogicCompiler::genFrombusTypeConversionCode(CodeSnippet* code,
															const UalSignal* inputBusSignal,
															const BusSignal& busSignal,
															const UalSignal* busChildSignal,
															const QString& busExtractorLabel,
															const InbusConvDescription& convDesc,
															bool readValueFromAccumulator,
															bool saveResultToAccumulator,
															const Address16& inbusSignalAddr)
	{
		Q_ASSERT(busSignal.typeConversionRequired() == true);

		QString tconvAfbCaption = convDesc.frombusTypeConversionAfb;

		if (tconvAfbCaption.isEmpty() == true)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AFB isn't assigned for frombus type conversion of bus child signal %1").
											arg(busChildSignal->appSignalID()));
			return false;
		}

		QStringList tconvAfbCaptions = tconvAfbCaption.split(Afb::OR);

		if (tconvAfbCaptions.size() < 1)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		std::shared_ptr<Afb::AfbElement> tconvElem = m_lmDescription->afbElement(tconvAfbCaptions[0]);

		if (tconvElem != nullptr)
		{
			bool result = genFrombusTconvCode(code, inputBusSignal, busSignal, busChildSignal, busExtractorLabel, tconvAfbCaptions[0],
											 readValueFromAccumulator, saveResultToAccumulator, inbusSignalAddr);

			return result;
		}

		Address16 accAddr(wordAccumulatorAddress(), 0);

		if (tconvAfbCaptions.size() < 2)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AFB isn't assigned for frombus type conversion of bus child signal %1").
											arg(busChildSignal->appSignalID()));
			return false;
		}

		tconvAfbCaption = tconvAfbCaptions[1];

		bool prevSaveResultToAccumulator = saveResultToAccumulator;
		bool toFloatConversionRequired = false;

		if (tconvAfbCaption == Afb::SW_TCONV_SI16_FP32 ||
			tconvAfbCaption == Afb::SW_TCONV_UI16_FP32)
		{
			saveResultToAccumulator = true;
			toFloatConversionRequired = true;

			if (tconvAfbCaption == Afb::SW_TCONV_UI16_FP32)
			{
				tconvAfbCaption = Afb::SW_TCONV_UI16_SI32;
			}

			if (tconvAfbCaption == Afb::SW_TCONV_SI16_FP32)
			{
				tconvAfbCaption = Afb::SW_TCONV_SI16_SI32;
			}
		}

		if (tconvAfbCaption == Afb::SW_TCONV_UI16_SI32)
		{
			CodeItem cmd;

			Address16 readAddr = inbusSignalAddr;

			if (readValueFromAccumulator == true)
			{
				readAddr = accAddr;
			}

			Address16 writeAddr = busChildSignal->ualAddr();

			if (saveResultToAccumulator == true)
			{
				writeAddr = accAddr;
			}

			Address16 accAddr2(wordAccumulator2Address(), 0);

			cmd.movConst(accAddr2.offset(), 0);						// write 0 in high word
			code->append(cmd);
			cmd.mov(accAddr2.offset() + 1, readAddr.offset());			// write value in low word
			code->append(cmd);
			cmd.mov32(writeAddr, accAddr2);
			code->append(cmd);

			if (toFloatConversionRequired == false)
			{
				return true;
			}
		}

		if (tconvAfbCaption == Afb::SW_TCONV_SI16_SI32)
		{
			std::shared_ptr<Afb::AfbElement> switchElem = m_lmDescription->afbElement(Afb::SWITCH_SI);

			if (switchElem == nullptr)
			{
				// Required AFB %1 is missing.
				//
				m_log->errALC5174(Afb::SWITCH_SI, QUuid());
				return false;
			}

			QString errMsg;

			UalItem switchItem(switchElem, errMsg);

			switchItem.setLabel(busExtractorLabel);

			UalAfb* swtch = createUalAfb(switchItem);

			const SchemaPin* select = swtch->getPin(Afb::SWITCH_SI_PIN_SELECT);
			const SchemaPin* x1 = swtch->getPin(Afb::SWITCH_SI_PIN_X1);
			const SchemaPin* x2 = swtch->getPin(Afb::SWITCH_SI_PIN_X2);
			const SchemaPin* output = swtch->getPin(Afb::SWITCH_SI_PIN_OUTPUT);

			if (select == nullptr ||
				x1 == nullptr ||
				x2 == nullptr ||
				output == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			CodeItem cmd;

			Address16 readAddr = inbusSignalAddr;

			if (readValueFromAccumulator == true)
			{
				readAddr = accAddr;
			}

			Address16 writeAddr = busChildSignal->ualAddr();

			if (saveResultToAccumulator == true)
			{
				writeAddr = accAddr;
			}

			Address16 acc2Addr(wordAccumulator2Address(), 0);

			// Construct unsigned value

			cmd.movConst(acc2Addr.offset(), 0);
			code->append(cmd);

			cmd.mov(acc2Addr.offset() + 1, readAddr.offset());
			code->append(cmd);

			cmd.writeFuncBlock32(swtch->opcode(), swtch->instance(),
								x1->afbOperandIndex(), acc2Addr,
								swtch->caption());
			code->append(cmd);

			// Construct signed value

			cmd.movConst(acc2Addr.offset(), 0xFFFF);
			code->append(cmd);

			cmd.mov(acc2Addr.offset() + 1, readAddr.offset());
			code->append(cmd);

			cmd.writeFuncBlock32(swtch->opcode(), swtch->instance(),
								x2->afbOperandIndex(), acc2Addr,
								swtch->caption());
			code->append(cmd);

			// Move sign bit to Select pin

			readAddr.setBit(15);

			cmd.writeFuncBlockBit(swtch->opcode(), swtch->instance(),
								  select->afbOperandIndex(), readAddr, swtch->caption());
			code->append(cmd);

			cmd.startafb(swtch->opcode(), swtch->instance(), swtch->caption(), swtch->runTime());
			code->append(cmd);

			cmd.readFuncBlock32(writeAddr,
							  swtch->opcode(), swtch->instance(),
							  output->afbOperandIndex(),
							  swtch->caption());
			code->append(cmd);

			if (toFloatConversionRequired == false)
			{
				return true;
			}
		}

		if (toFloatConversionRequired == true)
		{
			bool result = genFrombusTconvCode(code, inputBusSignal, busSignal, busChildSignal, busExtractorLabel,
											  Afb::TCONV_SI32_FP32,
											 true, prevSaveResultToAccumulator, inbusSignalAddr);
			return result;
		}

		LOG_INTERNAL_ERROR_MSG(m_log, QString("AFB is't assigned for frombus type conversion of bus child signal %1").
											arg(busChildSignal->appSignalID()));
		return false;
	}

	bool ModuleLogicCompiler::genFrombusTconvCode(CodeSnippet* code,
												const UalSignal* inputBusSignal,
												const BusSignal& busSignal,
												const UalSignal* busChildSignal,
												const QString& busExtractorLabel,
												const QString& tconvAfbCaption,
												bool readValueFromAccumulator,
												bool saveResultToAccumulator,
												const Address16& inbusSignalAddr)
	{
		Q_UNUSED(inputBusSignal);
		Q_UNUSED(busSignal);

		std::shared_ptr<Afb::AfbElement> tconvElem = m_lmDescription->afbElement(tconvAfbCaption);

		if (tconvElem == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString errMsg;

		UalItem tconvItem(tconvElem, errMsg);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
			return false;
		}

		tconvItem.setLabel(busExtractorLabel);

		UalAfb* tconv = createUalAfb(tconvItem);

		if (tconv == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		AfbSignal inSignal;
		AfbSignal outSignal;

		result &= tconv->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &inSignal);
		result &= tconv->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &outSignal);

		RETURN_IF_FALSE(result);

		Address16 accAddr(wordAccumulatorAddress(), 0);

		Address16 readAddr(inbusSignalAddr);

		if (readValueFromAccumulator == true)
		{
			readAddr = accAddr;
		}

		result &= generateSignalToAfbInputCode(code, tconv, inSignal, nullptr, BusProcessingStepInfo(), readAddr, false);

		result &= startAfb(code, tconv, BusProcessingStepInfo());

		if (saveResultToAccumulator == true)
		{
			result &= generateAfbOutputToSignalCode(code, tconv, outSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateAfbOutputToSignalCode(code, tconv, outSignal, busChildSignal,
													BusProcessingStepInfo(), Address16(), true);
		}

		return result;
	}


	bool ModuleLogicCompiler::genFrombusScalingCode(CodeSnippet* code,
													const UalSignal* inputBusSignal,
													const BusSignal& busSignal,
													const UalSignal* busChildSignal,
													const QString& busExtractorLabel,
													const InbusConvDescription& convDesc,
													bool readValueFromAccumulator,
													bool saveResultToAccumulator,
													const Address16& inbusSignalAddr)
	{
		Q_UNUSED(inputBusSignal);

		if (saveResultToAccumulator == true)
		{
			// saveResultToAccumulator must be FALSE
			//
			LOG_INTERNAL_ERROR(m_log);
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(busSignal.scalingRequired() == true);

		QString scaleAfbCaption = convDesc.frombusScalingAfb;

		if (scaleAfbCaption.isEmpty() == true)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AFB isn't assigned for frombus scaling of bus child signal %1").
											arg(busChildSignal->appSignalID()));
			return false;
		}

		std::shared_ptr<Afb::AfbElement> scaleElem = m_lmDescription->afbElement(scaleAfbCaption);

		if (scaleElem == nullptr)
		{
			// Required AFB %1 is missing.
			//
			m_log->errALC5174(scaleAfbCaption, QUuid());
			return false;
		}

		QString errMsg;

		UalItem scaleItem(scaleElem, errMsg);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
			return false;
		}

		scaleItem.setLabel(busExtractorLabel);

		bool result = true;

		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_X1, busSignal.inbusAnalogLowLimit);
		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_X2, busSignal.inbusAnalogHighLimit);

		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_Y1, busSignal.inOutAnalogLowLimit);
		result &=scaleItem.setParamValueByCaption(Afb::SCALE_PARAM_Y2, busSignal.inOutAnalogHighLimit);

		UalAfb* scale = createUalAfb(scaleItem);

		if (scale == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		RETURN_IF_FALSE(result);

		// No param initialization code required here.
		// Params initialization code will be generated in generateInitAfbsCode.

		AfbSignal inSignal;
		AfbSignal outSignal;

		result &= scale->getAfbSignalByCaption(Afb::IN_PIN_CAPTION, &inSignal);
		result &= scale->getAfbSignalByCaption(Afb::OUT_PIN_CAPTION, &outSignal);

		RETURN_IF_FALSE(result);

		Address16 accAddr(wordAccumulatorAddress(), 0);

		Address16 readAddr(inbusSignalAddr);

		if (readValueFromAccumulator == true)
		{
			readAddr = accAddr;
		}

		result &= generateSignalToAfbInputCode(code, scale, inSignal, nullptr, BusProcessingStepInfo(),	readAddr, false);

		result &= startAfb(code, scale, BusProcessingStepInfo());

		if (saveResultToAccumulator == true)
		{
			result &= generateAfbOutputToSignalCode(code, scale, outSignal, nullptr, BusProcessingStepInfo(), accAddr, true);
		}
		else
		{
			result &= generateAfbOutputToSignalCode(code, scale, outSignal, busChildSignal,
													BusProcessingStepInfo(), Address16(), false);
		}

		return result;
	}

	bool ModuleLogicCompiler::generateDiscreteSignalToBusExtractorCode(CodeSnippet* code,
																	   const UalItem* ualItem,
																	   const SchemaPin& inPin,
																	   const UalSignal* inputSignal)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);
		TEST_PTR_LOG_RETURN_FALSE(ualItem, m_log);
		TEST_PTR_LOG_RETURN_FALSE(inputSignal, m_log);

		// specific code generation ONLY for discrete signals connected to bus extractor Input
		//
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

		const std::vector<SchemaPin>& inputs = currentItem->inputs();

		bool pinFound = false;

		for(const SchemaPin& input : inputs)
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

			connectedItem = getValueOrNullptr(m_pinParent, *connectedOutPinUuid);

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

	UalItem* ModuleLogicCompiler::getAssociatedOutputPinParent(const SchemaPin& inputPin, QUuid* connectedOutPinUuid) const
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

		UalItem* connectedOutPinParent = getValueOrNullptr(m_pinParent, associatedOuts[0]);

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
		const SchemaBusExtractor* extractor = appBusExtractor->schemaBusExtractor();

		if (extractor == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		// getting extractor's input pin
		//
		const std::vector<SchemaPin>& inputs = appBusExtractor->inputs();

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

	bool ModuleLogicCompiler::getConnectedAppItems(const SchemaPin& pin, ConnectedAppItems* connectedAppItems)
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
			UalItem* connectedPinParent = getValueOrNullptr(m_pinParent, connectedPinUuid);

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

	bool ModuleLogicCompiler::getBusProcessingParams(const UalAfb* ualAfb, bool& isBusProcessingAfb, QString& busTypeID)
	{
		if (ualAfb == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		busTypeID.clear();

		isBusProcessingAfb = m_afbComponents.isBusProcessingAfb(ualAfb->afbStrID());

		if (isBusProcessingAfb == false)
		{
			return true;			// result of getBusProcessingParams()!
		}

		// this is bus processing AFB, will try identify BusTypeID

		bool result = true;

		QStringList busTypeIDs;

		for(const SchemaPin& inPin : ualAfb->inputs())
		{
			AfbSignal pinSignal;

			result = ualAfb->getAfbSignalByPin(inPin, &pinSignal);

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
			m_log->errALC5108(ualAfb->guid(), ualAfb->schemaID());
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
				m_log->errALC5109(ualAfb->guid(), ualAfb->schemaID());
				return false;
			}
		}

		busTypeID = checkBusTypeID;

		return true;
	}

	UalSignal* ModuleLogicCompiler::getPinInputAppSignal(const SchemaPin& inPin)
	{
		assert(inPin.IsInput());

		std::vector<QUuid> associatedOuts = inPin.associatedIOs();

		if (associatedOuts.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		QUuid associatedOutUuid = associatedOuts[0];

		const UalItem* connectedPinParent = getValueOrNullptr(m_pinParent, associatedOutUuid);

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

		const std::vector<SchemaPin>* pins = &ualItem->inputs();

		if (isInput == false)
		{
			pins = &ualItem->outputs();
		}

		for(const SchemaPin& pin : *pins)
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

	bool ModuleLogicCompiler::addToComparatorSet(const UalAfb* afb)
	{
		if (afb == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (afb->isComparator() == false)
		{
			return true;
		}

		std::shared_ptr<Comparator> cmp = std::make_shared<Comparator>();

		bool result = initComparator(cmp, afb);

		RETURN_IF_FALSE(result);

		m_cmpSet->insert(lmEquipmentID(), cmp);

		return true;
	}

	bool ModuleLogicCompiler::initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* afb)
	{
		TEST_PTR_LOG_RETURN_FALSE(afb, log());

		// set compare type of value
		//
		bool isConstComparator = afb->isConstComaparator();
		bool hysteresisIsConst = afb->caption().contains("_dh_") == false;

		//	set comparator type, compare-value, hysteresis-value
		//

		const std::vector<Afb::AfbParam>& params = afb->params();

		for(const AfbParam& pv : params)
		{
			if (pv.opName() == "i_conf") // set comparator type: =(1(SI)), > (2(SI)), < (3(SI)), != (4(SI)),= (5(FP)), > (6(FP)), < (7(FP)), != (8(FP)), >= (9 (SI)), <= (10 (SI)),  >= (11 (FP)),  <= (12 (FP))
			{
				switch (pv.afbParamValue().value().toInt())
				{
					case 1:
					case 5:			cmp->setCmpType(E::CmpType::Equal);			break;
					case 2:
					case 6:			cmp->setCmpType(E::CmpType::Greate);		break;
					case 3:
					case 7:			cmp->setCmpType(E::CmpType::Less);			break;
					case 4:
					case 8:			cmp->setCmpType(E::CmpType::NotEqual);		break;
					case 9:
					case 11:		cmp->setCmpType(E::CmpType::GreateEqual);	break;
					case 10:
					case 12:		cmp->setCmpType(E::CmpType::LessEqual);		break;

					default:
						assert(0);
				}
			}

			if (pv.opName() == "i_sp_s" && isConstComparator == true) // set compare-value
			{
				switch (pv.dataFormat())
				{
					case E::DataFormat::Float:			cmp->compare().setConstValue(pv.afbParamValue().value().toDouble());	break;
					case E::DataFormat::SignedInt:		cmp->compare().setConstValue(pv.afbParamValue().value().toInt());		break;
					case E::DataFormat::UnsignedInt:	cmp->compare().setConstValue(pv.afbParamValue().value().toInt());		break;

					default:
						assert(0);
				}
			}

			if (pv.opName() == "hysteresis" && hysteresisIsConst == true) // set hysteresis-value
			{
				switch (pv.dataFormat())
				{
					case E::DataFormat::Float:			cmp->hysteresis().setConstValue(pv.afbParamValue().value().toDouble());	break;
					case E::DataFormat::SignedInt:		cmp->hysteresis().setConstValue(pv.afbParamValue().value().toInt());	break;
					case E::DataFormat::UnsignedInt:	cmp->hysteresis().setConstValue(pv.afbParamValue().value().toInt());	break;

					default:
						assert(0);
				}
			}
		}

		// set comparator signals
		//
		for(const SchemaPin& pin : afb->inputs())
		{

			UalSignal* ualSignal = m_ualSignals.get(pin.guid());

			TEST_PTR_CONTINUE(ualSignal);

			if (ualSignal->isAnalog() == false)
			{
				continue;
			}

			// input Signal
			//
			if (pin.caption() == "in")
			{
				cmp->setInAnalogSignalFormat(ualSignal->analogSignalFormat());

				QString appSignalID;
				bool result = getNearestInSignalID(pin, &appSignalID);

				if ( result == false || appSignalID.isEmpty())
				{
					appSignalID = ualSignal->appSignalID();
				}

				cmp->input().setSignalParams(	appSignalID,
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

		for(const SchemaPin& pin : afb->outputs())
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
		cmp->setLabel(afb->label());
		cmp->setPrecision(afb->precision());
		cmp->setSchemaID(afb->schemaID());
		cmp->setSchemaItemUuid(afb->guid());

		// tests
		//
		if (cmp->input().appSignalID().isEmpty() == true)
		{
			assert(false);
			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty input signal").arg(afb->caption()).arg(afb->schemaID());
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
			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty cmp signal").arg(afb->caption()).arg(afb->schemaID());
			LOG_INTERNAL_ERROR_MSG(m_log, strError);
			qDebug() << strError;
			return false;
		}

		if (hysteresisIsConst == false && cmp->hysteresis().appSignalID().isEmpty() == true)
		{
			QString strError = QString("Error of comparator: %1 , schema: %2 - Empty hysteresis signal").arg(afb->caption()).arg(afb->schemaID());
			LOG_INTERNAL_ERROR_MSG(m_log, strError);
			qDebug() << strError;
			return false;
		}

		if (	cmp->cmpType() == E::CmpType::Equal || cmp->cmpType() == E::CmpType::NotEqual ||
				cmp->cmpType() == E::CmpType::GreateEqual || cmp->cmpType() == E::CmpType::LessEqual)
		{
			if (cmp->inAnalogSignalFormat() == E::AnalogAppSignalFormat::Float32)
			{
				if (cmp->hysteresisIsConstSignal() == true && cmp->hysteresis().constValue() == 0.0)
				{
					log()->wrnALC5177(afb->caption(), cmp->hysteresisPinCaption(), afb->guid(), afb->schemaID());
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

		for(UalSignal* ualSignal : m_acquiredDiscreteTuningSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			AppSignal* s = ualSignal->getTunableSignal();

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
		return copyBusesToRegBuf("Copy acquired Input Buses to RegBuf", m_acquiredInputBuses, code);
	}

	bool ModuleLogicCompiler::copyAcquiredBusChildBusesInRegBuf(CodeSnippet* code)
	{
		return copyBusesToRegBuf("Copy acquired bus child Buses to RegBuf", m_acquiredBusChildBuses, code);
	}

	bool ModuleLogicCompiler::copyAcquiredOptoBusesInRegBuf(CodeSnippet* code)
	{
		return copyBusesToRegBuf("Copy acquired opto Buses to regBuf", m_acquiredOptoBuses, code);
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

	bool ModuleLogicCompiler::copyScatteredDiscreteSignalsInRegBuf(CodeSnippet* code, const QVector<UalSignal*>& signalsList, const QString& description)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (signalsList.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		code->comment_nl(QString("Copy %1 in regBuf").arg(description));

		CopyBitsMap srcSignals;

		TEST_PTR_LOG_RETURN_FALSE(signalsList[0], m_log);

		int destAddressOffset = signalsList[0]->regBufAddr().offset();

		for(const UalSignal* ualSignal : signalsList)
		{
			TEST_PTR_LOG_RETURN_FALSE(ualSignal, m_log);

			if (ualSignal->isDiscrete() == false ||
				ualSignal->regBufAddr().isValid() == false ||
				ualSignal->regValueAddr().isValid() == false)
			{
				assert(false);				// signal's ualAddr ot regBufAddr is not initialized!
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			if (destAddressOffset != ualSignal->regBufAddr().offset())
			{
				if (srcSignals.empty() == false)
				{
					result &= codeCopyBits(code, destAddressOffset, srcSignals);
					code->newLine();

					srcSignals.clear();
				}

				destAddressOffset = ualSignal->regBufAddr().offset();
			}

			CopyBitInfo cbi;

			cbi.ualSignal = ualSignal;
			cbi.srcBitAddr = ualSignal->ualAddr();

			srcSignals.emplace(ualSignal->regBufAddr(), cbi);
		}

		if (srcSignals.empty() == false)
		{
			result &= codeCopyBits(code, destAddressOffset, srcSignals);
			code->newLine();
		}

		return result;
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

		for(const auto& [place, module] : m_modules)
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

			auto it = m_fbConv.find(appFb->caption());

			if (it == m_fbConv.end())
			{
				LOG_INTERNAL_ERROR(m_log);
				Q_ASSERT(false);
				return false;
			}

			const FbConv& fbConv = it->second;

			Q_ASSERT(fbConv.inputSignalDataSize == SIZE_32BIT);

			if (ualSignal->isConst() == true)
			{
				if(constIsFloat == true)
				{
					cmd.writeFuncBlockConstFloat(appFb->opcode(), appFb->instance(), fbConv.inputSignalIndex,
													constFloatValue, appFb->caption());
					cmd.setComment(QString(tr("float const %1 to output analog %2 conversion")).
										arg(constFloatValue).arg(s->appSignalID()));
				}
				else
				{
					cmd.writeFuncBlockConstInt32(appFb->opcode(), appFb->instance(), fbConv.inputSignalIndex,
													constIntValue, appFb->caption());
					cmd.setComment(QString(tr("int const %1 to output analog %2 conversion")).
										arg(constIntValue).arg(s->appSignalID()));
				}

				code->append(cmd);
			}
			else
			{
				cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbConv.inputSignalIndex,
								   s->ualAddr().offset(), appFb->caption());
				cmd.setComment(QString(tr("output analog %1 conversion")).arg(s->appSignalID()));
				code->append(cmd);
			}

			cmd.startafb(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
			cmd.clearComment();
			code->append(cmd);

			switch(fbConv.outputSignalDataSize)
			{
			case SIZE_32BIT:
				cmd.readFuncBlock32(s->ioBufAddr().offset(),
								  appFb->opcode(), appFb->instance(),
								  fbConv.outputSignalIndex, appFb->caption());
				break;

			case SIZE_16BIT:
				cmd.readFuncBlock(s->ioBufAddr().offset(),
								  appFb->opcode(), appFb->instance(),
								  fbConv.outputSignalIndex, appFb->caption());
				break;

			default:
				LOG_INTERNAL_ERROR(m_log);
				Q_ASSERT(false);
				return false;
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

		for(const AppSignal* s : m_ioSignals)
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

		std::map<int, CopyBitsMap> destCopyMaps;	// destAddrOffset => CopyBitsMap

		for(const AppSignal* s : m_ioSignals)
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

			if (ualSignal == nullptr)
			{
				continue;
			}

			Address16 destAddr = s->ioBufAddr();
			int destAddrOffset = destAddr.offset();

			auto it = destCopyMaps.find(destAddrOffset);

			if (it == destCopyMaps.end())
			{
				auto [new_it, b] = destCopyMaps.emplace(destAddrOffset,
											CopyBitsMap{});
				it = new_it;
			}

			CopyBitsMap& copyBitsMap = it->second;

			if (copyBitsMap.contains(destAddr))
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Signal %1 has duplicate IO buf addr").arg(s->appSignalID()));
				result = false;
				continue;
			}

			CopyBitInfo cbi;

			cbi.ualSignal = ualSignal;

			if (ualSignal->isConstDiscrete() == false)
			{
				cbi.srcBitAddr = ualSignal->ualAddr();
			}

			cbi.invertBit = s->invertSignal();

			copyBitsMap.emplace(destAddr, cbi);
		}

		RETURN_IF_FALSE(result);

		int lmOutputsAddress = m_lmDescription->memory().m_appDataOffset;
		bool lmOutputsIsWritten = false;

		code->comment_nl("Copy output discrete signals to output modules memory");

		for(const auto& [destAddrOffset, copyBitsMap] : destCopyMaps)
		{
			bool res = codeCopyBits(code, destAddrOffset, copyBitsMap);

			if (res == false)
			{
				result = false;
				continue;
			}

			code->newLine();

			if (destAddrOffset == lmOutputsAddress)
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

			for(const auto& [equipmentID, port] : module->ports())
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
		cmd.movConstUInt32(port->txBufAddress(), port->txDataID());
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

		int txRawDataStartAddr = port->txBufAddress() + rawDataOffset;
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
				cmd.setComment(QString("%1 <= %2").
								arg(port->connectionID()).
								arg(ualSignal->refSignalIDsJoined()));
			}
			else
			{
				switch(ualSignal->analogSignalFormat())
				{
				case E::AnalogAppSignalFormat::Float32:
					cmd.movConstFloat(txSignalAddress.offset(), ualSignal->constAnalogFloatValue());
					cmd.setComment(QString("%1 <= %2 (const %3)").
									arg(port->connectionID()).
									arg(ualSignal->refSignalIDsJoined()).
									arg(ualSignal->constAnalogFloatValue()));
					break;

				case E::AnalogAppSignalFormat::SignedInt32:
					cmd.movConstInt32(txSignalAddress.offset(), ualSignal->constAnalogIntValue());
					cmd.setComment(QString("%1 <= %2 (const %3)").
									arg(port->connectionID()).
									arg(ualSignal->refSignalIDsJoined()).
									arg(ualSignal->constAnalogIntValue()));
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

			cmd.setComment(QString("%1 <= %2").
								arg(port->connectionID()).
								arg(txSignal->appSignalIDs().join(", ")));
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

		qsizetype count = txDiscreteSignals.count();

		qsizetype wordCount = count / WORD_SIZE + ((count % WORD_SIZE) ? 1 : 0);

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
				cmd.setComment(QString("%1 <= %2 (const %3)").
									arg(port->connectionID()).
									arg(ualSignal->refSignalIDsJoined()).
									arg(ualSignal->constDiscreteValue()));

				copyCode.append(cmd);
			}
			else
			{
				cmd.movBit(bitAccumulatorAddress, bit, ualSignal->ualAddr().offset(), ualSignal->ualAddr().bit());
				cmd.setComment(QString("%1 <= %2").
										arg(port->connectionID()).
										arg(ualSignal->refSignalIDsJoined()));

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

				int txSignalAddress = port->txBufAddress() + txSignal->addrInBuf().offset();

				int srcAddr = 0;

				if (isCopyOptimizationAllowed(copyCode, &srcAddr) == true)
				{
					cmd.mov(txSignalAddress, srcAddr);
					cmd.setComment(QString("%1 <= %2").arg(port->connectionID()).arg(ids));

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
		Q_UNUSED(copyCode);
		Q_UNUSED(srcAddr);

		return false;
/*		if (srcAddr == nullptr)
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

		return true;*/
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

			toAddr = port->txBufAddress() + *rawDataOffset + localOffset;

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

		int writeAddr = port->txBufAddress() + *rawDataOffset;
		int writeSizeW = portTxRawDataSizeW;

		cmd.movMem(writeAddr, portWithRxRawData->rxBufAddress() + Hardware::OptoPort::TX_DATA_ID_SIZE_W, writeSizeW);
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

		int writeAddr = port->txBufAddress() + *rawDataOffset;

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

			int writeAddr = port->txBufAddress() + txSignal->addrInBuf().offset();

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

			cmd.setComment(QString("%1 <= %2").
							arg(port->connectionID()).
							arg(txSignal->appSignalID()));

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

				cmd.setComment(QString("%1 <= %2").
								arg(port->connectionID()).
								arg(discrete->appSignalID()));

				code->append(cmd);

				count++;
			}

			if (count > 0)
			{
				int writeAddr = port->txBufAddress() + offset;

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

			int writeAddr = port->txBufAddress() + txSignal->addrInBuf().offset();
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

			cmd.setComment(QString("%1 <= %2").
							arg(port->connectionID()).
							arg(txSignal->appSignalID()));
			code->append(cmd);

			count++;
		}

		if (count > 0)
		{
			code->newLine();
		}

		return result;
	}

	bool ModuleLogicCompiler::setLmAppLanDataSize()
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_lm, m_log);

		int regBufSizeW = m_memoryMap.regBufSizeW();

		return DeviceHelper::setIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											EquipmentPropNames::APP_LAN_DATA_SIZE,
											regBufSizeW,
											m_log);
	}

	bool ModuleLogicCompiler::setLmDiagLanDataSize()
	{
		int diagDataSize = 0;

		for(const auto& [place, module] : m_modules)
		{
			diagDataSize += module.txDiagDataSize;
		}

		return DeviceHelper::setIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											EquipmentPropNames::DIAG_LAN_DATA_SIZE,
											diagDataSize,
											m_log);
	}

	bool ModuleLogicCompiler::detectUnusedSignals()
	{
		for(const auto& [hash, s] : m_moduleSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isInternal() == true &&
				s->reserved() == false &&
				s->isSwCalculated() == false &&
				m_ualSignals.contains(s->appSignalID()) == false)
			{
				m_log->wrnALC5148(s->appSignalID());
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::detectUsedReservedSignals()
	{
		bool result = true;

		for(const auto& [hash, s] : m_moduleSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->reserved() == false)
			{
				continue;
			}

			auto it = m_ualItemsSignals.find(calcHash(s->appSignalID()));

			if (it == m_ualItemsSignals.end())
			{
				continue;
			}

			const std::set<QUuid>& ualItemGuids = it->second;

			for(auto const& guid : ualItemGuids)
			{
				const UalItem* ualItem = m_ualItems.value(guid, nullptr);

				TEST_PTR_CONTINUE(ualItem);

				// Reserved signal %1 used on schema %2.
				//
				m_log->wrnALC5201(s->appSignalID(), ualItem->guid(), ualItem->schemaID());
			}
		}

		return result;
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

	QString ModuleLogicCompiler::lmSubsystemEquipmentIdPath() const
	{
		return QString("%1/%2").arg(m_lmSubsystemID).arg(lmEquipmentID());
	}

	QString ModuleLogicCompiler::getInfoFileName(const QString& fileNameExtension) const
	{
		return (QString("%1-%2.%3").
					arg(m_lmSubsystemID).
					arg(m_lmNumber).
					arg(fileNameExtension)).toLower();
	}

	QString ModuleLogicCompiler::getSrcInfoFileName(const QString& fileNameExtension) const
	{
		return (QString("%1-%2-src.%3").
					arg(m_lmSubsystemID).
					arg(m_lmNumber).
					arg(fileNameExtension)).toLower();
	}

	bool ModuleLogicCompiler::writeLmInfoFiles()
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		TEST_PTR_RETURN_FALSE(m_lm);

		bool result = true;

		result &= writeAsmFile(m_appLogicCode);

		result &= writeMemFile();

		result &= writeStatisticsFile(m_appLogicCode,
									  m_idrCode,
									  m_alpCode);

		result &= writeTuningInfoFile();

		result &= writeOptoModulesReport();

		result &= writeLoopbacksReport();

		return result;
	}

	bool ModuleLogicCompiler::writeAsmFile(const AppLogicCode& code) const
	{
		if (code.optimized() == false &&
			m_context->generateExtraDebugInfo() == false)
		{
			// no generate ASM code before optimization if GenerateExtraDebugInfo is OFF
			return true;
		}

		QStringList asmCode;

		code.getAsmCode(m_lmDescription, &asmCode);

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
									(code.optimized() ? getInfoFileName("asm") : getSrcInfoFileName("asm")),
									 asmCode);

		return (buildFile != nullptr);
	}

	bool ModuleLogicCompiler::writeMemFile() const
	{
		QStringList memFile;

		m_memoryMap.getFile(memFile,
							m_ualSignals.discreteSignalsHeap().getHeapItemsLog(),
							m_ualSignals.analogAndBusSignalsHeap().getHeapItemsLog());

		// Append Software Calculated Signals section

		if (m_swCalcSignals.size() > 0)
		{
			static const QString line(QString().fill('-', 80));

			memFile.append(line);
			memFile.append("Software Calculated Signals (not disposed in LM memory)");
			memFile.append(line);

			for(const AppSignal* s : m_swCalcSignals)
			{
				TEST_PTR_CONTINUE(s);

				memFile.append(QString(" %1 %2").arg(E::valueToString(s->swCalcFunction()), -25, QChar(' ')).
						arg(s->appSignalID()));

			}
		}

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
												getInfoFileName("mem"), memFile);
		return (buildFile != nullptr);
	}

	bool ModuleLogicCompiler::writeStatisticsFile(const AppLogicCode& code,
												  const AppLogicCode& idrCode,
												  const AppLogicCode& alpCode) const
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		if (code.optimized() == false &&
			m_context->generateExtraDebugInfo() == false)
		{
			// no generate statistics file before optimization if GenerateExtraDebugInfo is OFF
			return true;
		}

		TEST_PTR_RETURN_FALSE(m_lmDescription);

		QStringList file;

		file << QString("LM equipmentID: %1").arg(lmEquipmentID());

		printCodeStatistics(idrCode, file, true);
		printCodeStatistics(alpCode, file, true);

		printCodeStatistics(code, file, false);

		//

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
										(code.optimized() ? getInfoFileName("stat") : getSrcInfoFileName("stat")),
										 file);
		return (buildFile != nullptr);
	}

	bool ModuleLogicCompiler::writeOptimizationReportFile() const
	{
		QStringList file;

		file << QString("LM equipmentID: %1").arg(lmEquipmentID());
		file << Separator::EMPTY_STR;
		file << Separator::EMPTY_STR;

		printOptiStatistics(m_idrCode, m_optiIdrCode, &file);
		printOptiStatistics(m_alpCode, m_optiAlpCode, &file);
		printOptiStatistics(m_appLogicCode, m_optiAppLogicCode, &file);
		printOptimizationsInfo(&file, m_appLogicCode.codeSizeW());

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
												getInfoFileName("orpt"), file);
		return (buildFile != nullptr);
	}

	void ModuleLogicCompiler::printOptiStatistics(const AppLogicCode& code,
												  const AppLogicCode& optiCode,
												  QStringList* outFile) const
	{
		TEST_PTR_RETURN(outFile);
		Q_ASSERT(code.codeType() == optiCode.codeType());

		QStringList& file = *outFile;

		switch(code.codeType())
		{
		case AppLogicCode::Type::IDR_Code:
			file << QString("IDR phase code metrics before and after optimization");
			break;

		case AppLogicCode::Type::ALP_Code:
			file << QString("ALP phase code metrics before and after optimization");
			break;

		case AppLogicCode::Type::AllCode:
			file << QString("All code metrics before and after optimization");
			break;

		default:
			Q_ASSERT(false);
		}

		file << Separator::EMPTY_STR;

		file << QString("         Metrics        |  Before  |  After   |   Diff   |  Diff, %");
		file << QString("------------------------+----------+----------+----------+----------");

		int intDiff = optiCode.codeSizeW() - code.codeSizeW();
		double percentDiff = (static_cast<double>(intDiff) * 100.0) /
							  static_cast<double>(code.codeSizeW());

		file << QString(" Code size              | %1 | %2 | %3 | %4").
					arg(code.codeSizeW(), 8).
					arg(optiCode.codeSizeW(), 8).
					arg(intDiff, 8).
					arg(percentDiff, 8, 'f', 3);

		intDiff = optiCode.clockCount() - code.clockCount();
		percentDiff = (static_cast<double>(intDiff) * 100.0) /
						  static_cast<double>(code.clockCount());

		file << QString(" Code exec time, clocks | %1 | %2 | %3 | %4").
					arg(code.clockCount(), 8).
					arg(optiCode.clockCount(), 8).
					arg(intDiff, 8).
					arg(percentDiff, 8, 'f', 3);

		double doubleDiff = optiCode.execTimeMcs() - code.execTimeMcs();
		percentDiff = (doubleDiff * 100.0) / code.execTimeMcs();

		file << QString(" Code exec time, mcs    | %1 | %2 | %3 | %4").
					arg(code.execTimeMcs(), 8, 'f', 2).
					arg(optiCode.execTimeMcs(), 8, 'f', 2).
					arg(doubleDiff, 8, 'f', 2).
					arg(percentDiff, 8, 'f', 3);

		file << Separator::EMPTY_STR;
		file << Separator::EMPTY_STR;
	}

	void ModuleLogicCompiler::printOptimizationsInfo(QStringList* outFile, int srcCodeSize) const
	{
		TEST_PTR_RETURN(outFile);

		QStringList& file = *outFile;

		file << QString("   Optimization         |  Count   | Code     |  Abs, %  |  Rel, %");
		file << QString("   type                 |          | decrease |          |");
		file << QString("------------------------+----------+----------+----------+----------");

		int totalOptimizationCount = 0;
		int totalCodeReduction = 0;

		for(const auto& p : m_optimizationsInfo)
		{
			const OptimizationInfo& oi = p.second;

			totalOptimizationCount += oi.optimizationsCount;
			totalCodeReduction += oi.codeReductionSizeW;
		}

		double totalAbsDiff = 0;
		double totalRelDiff = 0;

		for(const auto& p : m_optimizationsInfo)
		{
			const OptimizationInfo& oi = p.second;

			double absDiff = ((-oi.codeReductionSizeW * 100.0) / srcCodeSize);
			double relDiff = ((oi.codeReductionSizeW * 100.0) / totalCodeReduction);

			totalAbsDiff += absDiff;
			totalRelDiff += relDiff;

			file << QString(" %1 | %2 | %3 | %4 | %5").
							arg(OptimizationInfo::typeStr(oi.type), -22).
							arg(oi.optimizationsCount, 8).
							arg(oi.codeReductionSizeW, 8).
							arg(absDiff, 8, 'f', 3).
							arg(relDiff, 8, 'f', 3);
		}

		file << QString("------------------------+----------+----------+----------+----------");
		file << QString("         Total          | %1 | %2 | %3 | %4").
						arg(totalOptimizationCount, 8).
						arg(totalCodeReduction, 8).
						arg(totalAbsDiff, 8, 'f', 3).
						arg(totalRelDiff, 8, 'f', 3);
	}

	bool ModuleLogicCompiler::writeTuningInfoFile() const
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

		quint64 uniqueID = m_tuningData->fotipTuningDataUID();

		file.append(QString("FOTIP tuning data UID: %1 (0x%2)").arg(uniqueID).arg(QString::number(uniqueID, 16).toUpper(), 16, Latin1Char::ZERO));

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

		bool result = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
											  getInfoFileName("tun"), file);
		return result;
	}

	bool ModuleLogicCompiler::writeOptoModulesReport() const
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_optoModuleStorage, m_log);

		QVector<Hardware::OptoModuleShared> modules;

		modules = m_optoModuleStorage->getLmAssociatedOptoModules(lmEquipmentID());

		qsizetype count = modules.count();

		if (count == 0)
		{
			return true;
		}

		std::vector<Hardware::OptoModuleShared> optoModules;

		for(auto& m : modules)
		{
			optoModules.push_back(m);
		}

		std::sort(optoModules.begin(), optoModules.end(),
					[] (Hardware::OptoModuleShared a, Hardware::OptoModuleShared b)
					{
						return a->place() < b->place();
					});

		QStringList file;

		QString delim = "--------------------------------------------------------------------";

		QString str;

		for(qsizetype i = 0; i < count; i++)
		{
			Hardware::OptoModuleShared module = modules[i];

			if (module == nullptr)
			{
				assert(false);
				continue;
			}

			file.append(delim);

			if (module->isLmOrBvb())
			{
				str = QString(tr("Opto module LM (or BVB) %1")).arg(module->equipmentID());
			}
			else
			{
				if (module->isOcm())
				{
					str = QString(tr("Opto module OCM %1")).arg(module->equipmentID());
				}
				else
				{
					assert(false);
				}
			}

			file.append(str);
			file.append(delim);
			file.append("");

			// write module's opto ports information
			//
			for(const auto& [equipmentID, port] : module->ports())
			{
				port->writeInfo(file);
			}
		}

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
												getInfoFileName("opto"), file);
		return (buildFile != nullptr);
	}

	bool ModuleLogicCompiler::writeLoopbacksReport()
	{
		QStringList file;

		m_loopbacks.writeReport(&file);

		BuildFile* bf = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
												getInfoFileName("loopbacks"), file);

		return bf != nullptr;
	}

	bool ModuleLogicCompiler::writeHeapsLog()
	{
		if (m_context->generateExtraDebugInfo() == false ||
			noCodeGenRequired() == true)
		{
			return true;
		}

		QStringList file;

		m_ualSignals.getHeapsLog(&file);

		BuildFile* bf = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
												getInfoFileName("heaps"), file);
		return bf != nullptr;
	}

	bool ModuleLogicCompiler::writeNonPlatformRegInfoFile() const
	{
		TEST_PTR_RETURN_FALSE(m_lm);

		if (m_lm->isNonPlatformAppDataSourceModule() == false)
		{
			return true;			// Its Ok
		}

		int appDataSizeW = 0;

		bool res = DeviceHelper::getIntProperty(m_lm, EquipmentPropNames::APP_LAN_DATA_SIZE, &appDataSizeW, m_log);

		int rupFramesQuantity = (appDataSizeW * 2 + Rup::FRAME_DATA_SIZE - 1) / Rup::FRAME_DATA_SIZE;

		quint32 appDataUID = 0;

		res &= DeviceHelper::getUIntProperty(m_lm, EquipmentPropNames::APP_LAN_DATA_UID, &appDataUID, m_log);

		RETURN_IF_FALSE(res);

		QString line;

		line.fill(QChar('-'), 90);

		QStringList file;

		file.append(QString("%1 %2 registration info file\n").arg(m_lm->caption()).arg(lmEquipmentID()));
		file.append(QString("App data size:       %1 words (%2 bytes)").arg(appDataSizeW).arg(appDataSizeW * 2));
		file.append(QString("RUP frames quantity: %1").arg(rupFramesQuantity));
		file.append(QString("App data UID:        0x%1 (%2)\n").
						arg(QString::number(appDataUID, 16).toUpper().rightJustified(8, QChar('0'), false)).
						arg(appDataUID));

		file.append(line);
		file.append("  Module  | Place |  Value   | Validity |            AppSignalID");
		file.append(line);

		int prevPlace = -1;

		for(const auto& [regValueAddr, buimAppSignals] : m_nonPlatformRegSignals)
		{
			for(const NonPlatformAppSignal& bas : buimAppSignals)
			{
				const AppSignal* appSignal = bas.appSignal;

				TEST_PTR_CONTINUE(appSignal);

				Q_ASSERT(regValueAddr == appSignal->regValueAddr());

				QString placeStr(QStringLiteral("          |       "));

				if (bas.modulePlace != prevPlace)
				{
					if (prevPlace != -1)
					{
						file.append(line);
					}

					placeStr = QString(" %1 |  %2   ").arg(bas.moduleCaption, -8).arg(bas.modulePlace, -2);

					prevPlace = bas.modulePlace;
				}

				file.append(QString("%1| %2 | %3 | %4").
									arg(placeStr).
									arg(appSignal->regValueAddr().toString(true)).
									arg(appSignal->regValidityAddr().isValid() ?
											appSignal->regValidityAddr().toString(true) : "   No   ").
									arg(appSignal->appSignalID()));
			}
		}

		file.append(line);

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
														getInfoFileName("reg"), file);

		return buildFile != nullptr;
	}

	bool ModuleLogicCompiler::writeResult()
	{
		bool result = true;

		QByteArray binCode;

		m_optiAppLogicCode.getBinCode(&binCode);

		if (m_lmDescription->flashMemory().m_appLogicWriteBitstream == true)
		{
			result &= writeBinCodeForLm(binCode);

			RETURN_IF_FALSE(result);
		}

		//

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

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID),
														QString("%1-%2.mif").arg(m_lm->caption()).arg(m_lmNumber), mifCode);

		if (buildFile == nullptr)
		{
			result = false;
		}*/

		result &= writeOcmRsSignalsXml();

		//

		return result;
	}

	bool ModuleLogicCompiler::writeBinCodeForLm(const QByteArray& binCode)
	{
		bool result = true;

		int metadataFieldsVersion = 0;

		QStringList metadataFields;

		m_optiAppLogicCode.getAsmMetadataFields(&metadataFields, &metadataFieldsVersion);

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

		std::vector<QVariantList> metadata;

		m_optiAppLogicCode.getAsmMetadata(m_lmDescription, &metadata);

		result &= firmwareWriter->setChannelData(m_lmSubsystemID,
												 appLogicUartID,
												 lmEquipmentID(),
												 m_lmNumber,
												 m_lmAppLogicFramePayload,
												 m_lmAppLogicFrameCount,
												 m_rupAppDataUID,
												 binCode,
												 metadata,
												 log());
		return result;
	}

	bool ModuleLogicCompiler::calcAppDataUID()
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

		//

		Crc64 crc;

		crc.add(m_lm->equipmentIdTemplate());

		QByteArray appLogicBinCode;

		m_optiAppLogicCode.getBinCode(&appLogicBinCode);

		crc.add(appLogicBinCode);

		// add signals to UID
		//
		for(UalSignal* ualSignal: acquiredSignals)
		{
			TEST_PTR_CONTINUE(ualSignal);

			crc.add(ualSignal->appSignalID());

			if (ualSignal->regValueAddr().isValid() == true)
			{
				crc.add(ualSignal->regValueAddr().bitAddress());
			}
			else
			{
				assert(false);
			}
		}

		m_rupAppDataUID = crc.result32();

		return DeviceHelper::setUIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											EquipmentPropNames::APP_LAN_DATA_UID,
											m_rupAppDataUID,
											m_log);
	}

	bool ModuleLogicCompiler::calcDiagDataUID()
	{
		Crc64 crc;

		crc.add(m_lm->equipmentIdTemplate());

		//

		m_rupDiagDataUID = crc.result32();

		return DeviceHelper::setUIntProperty(const_cast<Hardware::DeviceModule*>(m_lm),
											EquipmentPropNames::DIAG_LAN_DATA_UID,
											m_rupDiagDataUID,
											m_log);
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

	void ModuleLogicCompiler::printCodeStatistics(const AppLogicCode& code,
												QStringList& file,
												bool exludeNotUsedCommands) const
	{
		std::vector<CommandStatistics> stat;

		code.getCommandsStatistics(m_lmDescription, &stat);

		QString phaseStr;
		int phaseClocks = 0;
		double phaseTime = 0;

		switch(code.codeType())
		{
		case AppLogicCode::Type::IDR_Code:
			phaseStr = QString("IDR phase");
			phaseClocks = m_lmDescription->logicUnit().idrPhaseClocks();
			phaseTime = m_lmDescription->logicUnit().m_idrPhaseTime;
			break;

		case AppLogicCode::Type::ALP_Code:
			phaseStr = QString("ALP phase");
			phaseClocks = m_lmDescription->logicUnit().alpPhaseClocks();
			phaseTime = m_lmDescription->logicUnit().m_alpPhaseTime;
			break;

		case AppLogicCode::Type::AllCode:
			phaseStr = QString("All");
			phaseClocks = m_lmDescription->logicUnit().idrPhaseClocks() +
						  m_lmDescription->logicUnit().alpPhaseClocks();
			phaseTime = m_lmDescription->logicUnit().m_idrPhaseTime +
						m_lmDescription->logicUnit().m_alpPhaseTime;
			break;

		default:
			Q_ASSERT(false);
			return;
		}

		file << Separator::EMPTY_STR;
		file << Separator::EMPTY_STR;
		file << QString("%1 code statistics ordered by commands execution time descending").arg(phaseStr);
		file << Separator::EMPTY_STR;
		file << QString("%1 code execution time used %2 of %3 clocks, %4 of %5 mcs (%6%).").
					arg(phaseStr).
					arg(code.clockCount()).
					arg(phaseClocks).
					arg(code.execTimeMcs(), 0, 'f', 1).
					arg(phaseTime, 0, 'f', 1).
					arg(code.lmCycleTimeUsage(), 0, 'f', 2);
		file << Separator::EMPTY_STR;

		std::sort(stat.begin(), stat.end(), [] (const CommandStatistics& a,
												const CommandStatistics& b) -> bool
												{ return a.execTime > b.execTime; });

		printCodeStatisticsTable(code, stat, file, exludeNotUsedCommands);

		//

		file << Separator::EMPTY_STR << Separator::EMPTY_STR;
		file << QString("%1 code statistics ordered by commands code size descending.").arg(phaseStr);
		file << Separator::EMPTY_STR;
		file << QString("%1 code memory used %2 of %3 words (%4%).").
					arg(phaseStr).
					arg(code.codeSizeW()).
					arg(m_lmDescription->memory().m_codeMemorySize).
					arg(code.lmCodeMemoryUsage(), 0, 'f', 2);
		file << Separator::EMPTY_STR;

		std::sort(stat.begin(), stat.end(), [] (const CommandStatistics& a,
												const CommandStatistics& b) -> bool
												{ return a.codeSizeW > b.codeSizeW; });

		printCodeStatisticsTable(code, stat, file, exludeNotUsedCommands);
	}

	void ModuleLogicCompiler::printCodeStatisticsTable(const AppLogicCode& code,
												const std::vector<CommandStatistics>& stat,
												QStringList& file,
												bool exludeNotUsedCommands) const
	{
		QString headerStr;

		switch(code.codeType())
		{
		case AppLogicCode::Type::IDR_Code:
			headerStr = QString("            | Used in IDR code |  IDR code size   |  IDR exec time   ");
			break;

		case AppLogicCode::Type::ALP_Code:
			headerStr = QString("            | Used in ALP code |  ALP code size   |  ALP exec time   ");
			break;

		case AppLogicCode::Type::AllCode:
			headerStr = QString("            |       Used       |     Code size    |    Exec time     ");
			break;

		default:
			Q_ASSERT(false);
			return;
		}

		file << headerStr;
		file << QString("  Command   |------------------+------------------+------------------");
		file << QString("            | Count  | Percent | Words  | Percent | Clocks | Percent ");
		file << QString("------------+--------+---------+--------+---------+--------+---------");

		float usedPercentTotal = 0;
		float sizePercentTotal = 0;
		float execPercentTotal = 0;

		int usedCountTotal = 0;
		int codeSizeWTotal = 0;
		int execTimeTotal = 0;

		for(const CommandStatistics& cs : stat)
		{
			if (exludeNotUsedCommands == true && cs.usedCount == 0)
			{
				continue;
			}

			//

			usedCountTotal += cs.usedCount;
			float usedPercent = static_cast<float>(cs.usedCount * 100) /
										static_cast<float>(code.commandsCount());
			usedPercentTotal += usedPercent;

			//

			codeSizeWTotal += cs.codeSizeW;
			float sizePercent = static_cast<float>(cs.codeSizeW * 100) /
										static_cast<float>(code.codeSizeW());
			sizePercentTotal += sizePercent;

			//

			execTimeTotal += cs.execTime;

			float execPercent = static_cast<float>(cs.execTime * 100) /
										static_cast<float>(code.clockCount());
			execPercentTotal += execPercent;

			//

			const LmCommand* lmCmd = m_lmDescription->commandPtr(cs.code);

			if (lmCmd != nullptr)
			{
				file << getStatStr(lmCmd->caption.toUpper(),
								   cs.usedCount, usedPercent,
								   cs.codeSizeW, sizePercent,
								   cs.execTime, execPercent, false);
			}
			else
			{
				Q_ASSERT(false);
			}
		}

		Q_ASSERT(code.commandsCount() == usedCountTotal);
		Q_ASSERT(code.codeSizeW() == codeSizeWTotal);
		Q_ASSERT(code.clockCount() == execTimeTotal);

		file << QString("------------+--------+---------+--------+---------+--------+---------");

		file << getStatStr(QString("Total"),
						 usedCountTotal, usedPercentTotal,
						 codeSizeWTotal, sizePercentTotal,
						 execTimeTotal, execPercentTotal, true);
	}

	QString ModuleLogicCompiler::getStatStr(const QString& mnemo,
											int used, float usedPercent,
											int sizeW, float sizePercent,
											int execTime, float execPercent,
											bool isTotal) const
	{
		const int CSTR_SIZE = 16;
		char cstr[CSTR_SIZE];

		QString str;

		str += " " + mnemo.leftJustified(11) + "|";

		//

		str += " " + QString::number(used).rightJustified(6) + " |";

		if (isTotal == true)
		{
			cstr[0] = 0;
		}
		else
		{
			snprintf(cstr, CSTR_SIZE, "%5.2f", usedPercent);
		}

		str += " " +(QString("%1").arg(cstr)).rightJustified(7) + " |";

		//

		str += " " + QString::number(sizeW).rightJustified(6) + " |";

		if (isTotal == true)
		{
			cstr[0] = 0;
		}
		else
		{
			snprintf(cstr, CSTR_SIZE, "%5.2f", sizePercent);
		}

		str += " " +(QString("%1").arg(cstr)).rightJustified(7) + " |";

		//

		str += " " + QString::number(execTime).rightJustified(6) + " |";

		if (isTotal == true)
		{
			cstr[0] = 0;
		}
		else
		{
			snprintf(cstr, CSTR_SIZE, "%5.2f", execPercent);
		}

		str += " " +(QString("%1").arg(cstr)).rightJustified(7);

		return str;
	}

	bool ModuleLogicCompiler::displayResourcesUsageInfo()
	{
		if (noCodeGenRequired() == true)
		{
			return true;
		}

		QString str;

		double percentOfUsedCodeMemory = (m_optiAppLogicCode.codeSizeW() * 100.0) / m_lmCodeMemorySize;

		bool result = true;

		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("Used resources of %1:")).arg(lmEquipmentID()));

		str.setNum(percentOfUsedCodeMemory, 'f', 2);

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

		str.setNum(percentOfUsedBitMemory, 'f', 2);

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

		str.setNum(percentOfUsedWordMemory, 'f', 2);

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
		double idrPhaseTime = (1.0/m_lmClockFrequency) * m_optiIdrCode.clockCount();
		double idrPhaseTimeUsed = 0;

		assert(m_lmIDRPhaseTime != 0);

		if (m_lmIDRPhaseTime != 0)
		{
			idrPhaseTimeUsed = (idrPhaseTime * 100) / (static_cast<double>(m_lmIDRPhaseTime) / 1000000.0);
		}

		str_percent.setNum(static_cast<float>(idrPhaseTimeUsed), 'f', 2);
		str.setNum(static_cast<float>(idrPhaseTime * 1000000), 'f', 2);

		LOG_MESSAGE(m_log, QString(tr("Input Data Receive phase time - %1% (%2 clocks or %3 &micro;s of %4 &micro;s)")).
					arg(str_percent).arg(m_optiIdrCode.clockCount()).arg(str).arg(m_lmIDRPhaseTime));

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
		double alpPhaseTime = (1.0/m_lmClockFrequency) * m_optiAlpCode.clockCount();
		double alpPhaseTimeUsed = 0;

		assert(m_lmALPPhaseTime != 0);

		if (m_lmALPPhaseTime != 0)
		{
			alpPhaseTimeUsed = (alpPhaseTime * 100) / (static_cast<double>(m_lmALPPhaseTime) / 1000000.0);
		}

		str_percent.setNum(static_cast<float>(alpPhaseTimeUsed), 'f', 2);
		str.setNum(static_cast<float>(alpPhaseTime * 1000000), 'f', 2);

		LOG_MESSAGE(m_log, QString(tr("Application Logic Processing phase time - %1% (%2 clocks or %3 &micro;s of %4 &micro;s)")).
					arg(str_percent).arg(m_optiAlpCode.clockCount()).arg(str).arg(m_lmALPPhaseTime));

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

			aui.usedInstances = m_afbComponents.getUsedInstancesCount(componentOpCode);

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

	bool ModuleLogicCompiler::checkLoopbackTargetSignalsCompatibility(const AppSignal& srcSignal, QUuid srcSignalUuid, const UalAfb& fb, const AfbSignal& afbSignal)
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

		return EMPTY_STR;
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

		m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID + Separator::DIR + lmEquipmentID()),
									QString("sl_%1.csv").arg(listName), "", "", strList);
		return result;
	}

	bool ModuleLogicCompiler::writeUalSignalsList() const
	{
		QStringList report;

		m_ualSignals.getReport(report);

		BuildFile* buildFile = m_resultWriter->addFile(m_resultWriter->subsystemDirectory(m_lmSubsystemID + Separator::DIR + lmEquipmentID()),
															"ualSignals.csv", "", "", report, false);

		return buildFile != nullptr;
	}

	bool ModuleLogicCompiler::runProcs(const std::vector<ProcToCall>& procArray)
	{
		bool result = true;

		for(const ProcToCall& p : procArray)
		{
			std::function<bool(ModuleLogicCompiler*)> proc = p.first;

			result &= std::invoke(proc, this);

			if (result == false)
			{
				// %1 has been finished with errors.
				//
				const QString& procName = p.second;
				m_log->errALC5999(procName);
				break;
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::runCodeGenProcs(const std::vector<CodeGenProcToCall>& procArray, CodeSnippet* code)
	{
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		bool result = true;

		for(const CodeGenProcToCall& p : procArray)
		{
			std::function<bool(ModuleLogicCompiler*, CodeSnippet*)> proc = p.first;

			result &= std::invoke (proc, this, code);

			if (result == false)
			{
				// %1 has been finished with errors.
				//
				const QString& procName = p.second;
				m_log->errALC5999(procName);
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

	bool ModuleLogicCompiler::codeCopyBits(CodeSnippet* code, int destAddrOffset, const CopyBitsMap& copyBitsMap)
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(code, m_log);

		if (copyBitsMap.empty() == true)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		int const0Count = 0;
		int const1Count = 0;
		int nonConstCount = 0;
		int nonConstInvertedBitsCount = 0;

		std::set<Address16> uniqueSrcBitAddrs;

		for(const auto& [destBitAddr, copyBitInfo] : copyBitsMap)
		{
			if (destBitAddr.isValid() == false ||
				destBitAddr.offset() != destAddrOffset ||
				copyBitInfo.ualSignal == nullptr ||
				(copyBitInfo.ualSignal->isConstDiscrete() == false && copyBitInfo.srcBitAddr.isValid() == false))
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (copyBitInfo.ualSignal->isConstDiscrete() == true )
			{
				copyBitInfo.constValue = copyBitInfo.ualSignal->constDiscreteValue();

				if (copyBitInfo.invertBit)
				{
					copyBitInfo.constValue ^= 1;
				}

				const1Count += copyBitInfo.constValue;
			}
			else
			{
				if (copyBitInfo.ualSignal->isDiscrete() == false)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				nonConstCount++;

				if (copyBitInfo.invertBit == true)
				{
					nonConstInvertedBitsCount++;
				}

				uniqueSrcBitAddrs.insert(copyBitInfo.srcBitAddr);
			}
		}

		const0Count = SIZE_16BIT - (nonConstCount + const1Count);

		RETURN_IF_FALSE(result);

		auto getComment = [](const CopyBitInfo& cbi) -> QString
		{
			if (cbi.comment.isEmpty() == false)
			{
				return cbi.comment;
			}

			return QString("copy %1").arg(cbi.ualSignal->refSignalIDsJoined());
		};

		CodeItem cmd;

		//

		if (const1Count == SIZE_16BIT)
		{
			const CopyBitInfo& first = copyBitsMap.begin()->second;

			*code << cmd.movConst(destAddrOffset, 0xFFFF, getComment(first));

			return true;
		}

		if (const0Count == SIZE_16BIT)
		{
			const CopyBitInfo& first = copyBitsMap.begin()->second;

			*code << cmd.movConst(destAddrOffset, 0x0000, getComment(first));

			return true;
		}

		if (nonConstCount == SIZE_16BIT && uniqueSrcBitAddrs.size() == 1)
		{
			// whole word fills by one non constant bit
			//
			const CopyBitInfo& cbi = copyBitsMap.begin()->second;

			Address16 srcBitAddrAddr = *uniqueSrcBitAddrs.begin();

			if (nonConstInvertedBitsCount == 0)
			{
				*code << cmd.fillb(Address16(destAddrOffset, 0),
										  srcBitAddrAddr, getComment(cbi));
				return true;
			}

			if (nonConstInvertedBitsCount == SIZE_16BIT)
			{
				*code << cmd.fillb(wordAccumulatorAddress16(),
									srcBitAddrAddr, getComment(cbi));

				result &= codeNotWord(code, wordAccumulatorAddress16(), EMPTY_STR,
									  Address16(destAddrOffset, 0), EMPTY_STR);
				return result;
			}

			// else, i.e. invertedBitsCount > 0 && < SIZE_16BIT, continue below
		}

		int initializedBy = -1;

		if (m_bitAccAvailable == false || nonConstInvertedBitsCount > 0)
		{
			int destAccAddr = destAddrOffset;

			if (addressInBitMemory(destAddrOffset) == false)
			{
				destAccAddr = bitAccumulatorAddress();
			}

			if (nonConstCount < SIZE_16BIT)
			{
				if (const1Count > const0Count)
				{
					// destAcc <= 0xFFFF
					//
					*code << cmd.movConst(destAccAddr, 0xFFFF);
					initializedBy = 1;
				}
				else
				{
					// destAcc <= 0x0000
					//
					*code << cmd.movConst(destAccAddr, 0);
					initializedBy = 0;
				}
			}

			for(const auto& [destBitAddr, copyBitInfo] : copyBitsMap)
			{
				switch(copyBitInfo.constValue)
				{
				case CopyBitInfo::CONST_0:
					if (initializedBy != CopyBitInfo::CONST_0)
					{
						*code << cmd.movBitConst(destAccAddr, destBitAddr.bit(), 0, getComment(copyBitInfo));
					}
					break;

				case CopyBitInfo::CONST_1:
					if (initializedBy != CopyBitInfo::CONST_1)
					{
						*code << cmd.movBitConst(destAccAddr, destBitAddr.bit(), 1, getComment(copyBitInfo));
					}
					break;

				case CopyBitInfo::NON_CONST:
					if (copyBitInfo.invertBit == true)
					{
						result &= codeNotBit(code, copyBitInfo.srcBitAddr, getComment(copyBitInfo),
											 Address16(destAccAddr, destBitAddr.bit()), EMPTY_STR);
					}
					else
					{
						*code << cmd.movBit(Address16(destAccAddr, destBitAddr.bit()),
											copyBitInfo.srcBitAddr, getComment(copyBitInfo));
					}
					break;

				default:
					Q_ASSERT(false);
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}
			}

			if (destAccAddr != destAddrOffset)
			{
				*code << cmd.mov(destAddrOffset, destAccAddr);
			}
		}
		else
		{
			auto it = copyBitsMap.rbegin();

			// find highest non-zero bitNo
			//
			int bitNo = SIZE_16BIT - 1;

			while(it != copyBitsMap.rend())
			{
				const auto& [destBitAddr, copyBitInfo] = *it;

				bitNo = destBitAddr.bit();

				if (copyBitInfo.constValue != CopyBitInfo::CONST_0)
				{
					break;
				}

				it++;
			}

			if (bitNo < SIZE_16BIT - 1)
			{
				*code << cmd.resetAcc();
			}

			while(bitNo >= 0)
			{
				bool bitWritten = false;

				if (it != copyBitsMap.rend())
				{
					const auto& [destBitAddr, copyBitInfo] = *it;

					if (bitNo == destBitAddr.bit())
					{
						switch(copyBitInfo.constValue)
						{
						case CopyBitInfo::NON_CONST:
							*code << cmd.movBitAccAddr(copyBitInfo.srcBitAddr, getComment(copyBitInfo));
							break;

						case CopyBitInfo::CONST_0:
								*code << cmd.lshift0Acc(getComment(copyBitInfo));
							break;

						case CopyBitInfo::CONST_1:
								*code << cmd.lshift1Acc(getComment(copyBitInfo));
							break;

						default:
							Q_ASSERT(false);
							LOG_INTERNAL_ERROR(m_log);
							return false;
						}

						it++;
						bitWritten = true;
					}
				}

				if (bitWritten == false)
				{
					*code << cmd.lshift0Acc();
				}

				bitNo--;
			}

			*code << cmd.movAddrAcc(destAddrOffset);
		}

		return result;
	}

	bool ModuleLogicCompiler::codeNotWord(CodeSnippet* code,
										  const Address16& srcAddr,
										  const QString& srcComment,
										  const Address16& destAddr,
										  const QString& destComment) const
	{
		CodeItem cmd;

		if (m_bitAccAvailable == true)
		{
			// code used bit acc
			//
			*code << cmd.movAccAddr(srcAddr, srcComment);
			*code << cmd.notAcc();
			*code << cmd.movAddrAcc(destAddr, destComment);

			return true;
		}

		// code used AFB bus_not
		//
		auto it = m_fbConv.find(Afb::AFB_BUS_NOT);

		if (it == m_fbConv.end())
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const FbConv& fbBusNot = it->second;

		if (fbBusNot.ualAfbs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const UalAfb* afbBusNot = *fbBusNot.ualAfbs.begin();

		*code << cmd.writeFuncBlock(afbBusNot->opcode(),
								   afbBusNot->instance(),
								   fbBusNot.inputSignalIndex,
								   srcAddr,
								   afbBusNot->caption(),
								   srcComment);

		*code << cmd.startafb(afbBusNot->opcode(),
							 afbBusNot->instance(),
							 afbBusNot->caption(),
							 afbBusNot->runTime());

		*code << cmd.readFuncBlock(destAddr,
								  afbBusNot->opcode(),
								  afbBusNot->instance(),
								  fbBusNot.outputSignalIndex,
								  afbBusNot->caption(),
								  destComment);
		return true;
	}

	bool ModuleLogicCompiler::codeNotBit(CodeSnippet* code,
										  const Address16& srcAddr,
										  const QString& srcComment,
										  const Address16& destAddr,
										  const QString& destComment) const
	{
		CodeItem cmd;

		if (m_bitAccAvailable == true)
		{
			// code used bit acc
			//
			*code << cmd.movBitAccAddr(srcAddr, srcComment);
			*code << cmd.notAcc();
			*code << cmd.movBitAddrAcc(destAddr, destComment);

			return true;
		}

		// code used AFB bus_not
		//
		auto it = m_fbConv.find(Afb::AFB_NOT);

		if (it == m_fbConv.end())
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const FbConv& fbNot = it->second;

		if (fbNot.ualAfbs.size() != 1)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const UalAfb* afbNot = *fbNot.ualAfbs.begin();

		*code << cmd.writeFuncBlockBit(afbNot->opcode(),
									   afbNot->instance(),
									   fbNot.inputSignalIndex,
									   srcAddr,
									   afbNot->caption(),
									   srcComment);

		*code << cmd.startafb(afbNot->opcode(),
							 afbNot->instance(),
							 afbNot->caption(),
							 afbNot->runTime());

		*code << cmd.readFuncBlockBit(destAddr,
									  afbNot->opcode(),
									  afbNot->instance(),
									  fbNot.outputSignalIndex,
									  afbNot->caption(),
									  destComment);
		return true;
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

	void ModuleLogicCompiler::findLogicAfbsForBitAccReplacing(const QString& afbCaption,
															  int logicConfValue, std::set<QUuid>* guidsMap)
	{
		TEST_PTR_RETURN(guidsMap);

		if (m_lmDescription->isBitAccAvailable() == false)
		{
			return;
		}

		for(const UalAfb* ualAfb : m_ualAfbs)
		{
			TEST_PTR_CONTINUE(ualAfb);

			if (ualAfb->caption() != afbCaption)
			{
				continue;
			}

			if (ualAfb->opcode() != TO_INT(Afb::AfbType::LOGIC))
			{
				Q_ASSERT(false);
				continue;
			}

			bool ok = false;

			int iConfValue = ualAfb->getParamIntValueByOpName(Afb::PARAM_I_CONF, &ok);

			if (ok == false )
			{
				Q_ASSERT(false);
				continue;
			}

			if (iConfValue != logicConfValue)
			{
				continue;
			}

			guidsMap->insert(ualAfb->guid());
		}
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

	bool ModuleLogicCompiler::Module::isOptoModule() const
	{
		if (device == nullptr)
		{
			assert(false);
			return false;
		}

		return device->isOptoModule();
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

	QString ModuleLogicCompiler::Module::equipmentID() const
	{
		if (device == nullptr)
		{
			Q_ASSERT(false);
			return EMPTY_STR;
		}

		return device->equipmentIdTemplate();
	}
}
