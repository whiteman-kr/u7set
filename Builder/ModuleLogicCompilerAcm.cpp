#include <HardwareLib/DeviceAppSignal.h>
#include <HardwareLib/DeviceController.h>
#include <HardwareLib/DeviceModule.h>

#include "ModuleLogicCompiler.h"
#include "DeviceHelper.h"

namespace Builder
{
	bool ModuleLogicCompiler::acmPass1()
	{
		LOG_EMPTY_LINE(m_log)

		LOG_MESSAGE(m_log, QString(tr("Compilation pass #1 for Actuator type %1 was started...")).arg(m_actuatorTypeID));

		const BuildActuatorType& actuatorType = getBuildActuatorType(m_actuatorTypeID);

		if (actuatorType.isValid() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Actuator type %1 is not found in the project").arg(m_actuatorTypeID));
			return false;
		}

		m_moduleLogic = actuatorType.parseResult;

		if (m_moduleLogic == nullptr)
		{
			//	Application logic for actuator type '%1' is not found.
			//
			m_log->wrnALC5208(m_actuatorTypeID);
		}

		std::vector<ProcToCall> procs = {
			PROC_TO_CALL(ModuleLogicCompiler::loadLMSettings),
			// PROC_TO_CALL(ModuleLogicCompiler::loadModulesSettings),
			// PROC_TO_CALL(ModuleLogicCompiler::createModuleSignalsMap),
			PROC_TO_CALL(ModuleLogicCompiler::createUalItemsMaps),
			PROC_TO_CALL(ModuleLogicCompiler::createUalAfbsMap),
			PROC_TO_CALL(ModuleLogicCompiler::acmCreateSignalSet),
			PROC_TO_CALL(ModuleLogicCompiler::acmCreateUalSignals),
			// PROC_TO_CALL(ModuleLogicCompiler::processSignalsWithFlags),
			PROC_TO_CALL(ModuleLogicCompiler::sortUalSignals),
			// PROC_TO_CALL(ModuleLogicCompiler::processTxSignals),
			// PROC_TO_CALL(ModuleLogicCompiler::processSinglePortRxSignals),
			//PROC_TO_CALL(ModuleLogicCompiler::acmDisposeSwInOuts),
			PROC_TO_CALL(ModuleLogicCompiler::disposeSignalsInHeap),
			PROC_TO_CALL(ModuleLogicCompiler::acmCreateSignalLists),
			PROC_TO_CALL(ModuleLogicCompiler::acmDisposeSignalsInMemory),
			//PROC_TO_CALL(ModuleLogicCompiler::appendAfbsForInOutSignalsConversion),
			PROC_TO_CALL(ModuleLogicCompiler::setOutputSignalsAsComputed),
			// PROC_TO_CALL(ModuleLogicCompiler::setOptoRawInSignalsAsComputed),
			// PROC_TO_CALL(ModuleLogicCompiler::fillComparatorSet),
			// PROC_TO_CALL(ModuleLogicCompiler::findEndpointSignals),
		};

		bool result = runProcs(procs);

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString(tr("Compilation pass #1 for Actuator type %1 was successfully finished.")).arg(m_actuatorTypeID));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Compilation pass #1 for Actuator type %1 was finished with errors")).arg(m_actuatorTypeID));
		}

		return result;
	}

	bool ModuleLogicCompiler::acmPass2()
	{
		LOG_EMPTY_LINE(m_log)

		LOG_MESSAGE(m_log, QString(tr("Compilation pass #2 for Actuator %1 was started...")).arg(m_actuatorTypeID));

		std::vector<ProcToCall> procs = {PROC_TO_CALL(Builder::ModuleLogicCompiler::initComparatorSignals),

										 // PROC_TO_CALL(ModuleLogicCompiler::writeSignalLists),			// extra debug info signal lists

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

										 //PROC_TO_CALL(ModuleLogicCompiler::setLmAppLanDataSize),
										 //PROC_TO_CALL(ModuleLogicCompiler::setLmDiagLanDataSize),

										 PROC_TO_CALL(ModuleLogicCompiler::detectUnusedSignals),
										 PROC_TO_CALL(ModuleLogicCompiler::detectUsedReservedSignals),
										 PROC_TO_CALL(ModuleLogicCompiler::fillAnalogSignalsOnSchemas),

										 //PROC_TO_CALL(ModuleLogicCompiler::calcAppDataUID),
										 //PROC_TO_CALL(ModuleLogicCompiler::calcDiagDataUID),

										 PROC_TO_CALL(ModuleLogicCompiler::writeResult),
										 //PROC_TO_CALL(ModuleLogicCompiler::writeNonPlatformRegInfoFile)
										 };

		bool result = runProcs(procs);

		if (result == true)
		{
			result &= displayResourcesUsageInfo();
		}

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString(tr("Compilation pass #2 for Actuator %1 was successfully finished.")).arg(m_actuatorTypeID));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Compilation pass #2 for Actuator %1 was finished with errors")).arg(m_actuatorTypeID));
		}

		calcOptoDiscretesStatistics();

		cleanup();

		return result;
	}

	bool ModuleLogicCompiler::acmCreateSignalSet()
	{
		m_actuatorSignals->prepareBusses();

		const BuildActuatorType& actuatorType = getBuildActuatorType(m_actuatorTypeID);

		if (actuatorType.isValid() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		bool result = true;

		m_actuatorSignals->enableIdGeneration();

		for (const auto& [signalID, deviceAppSignal] : actuatorType.acmInputs)
		{
			result &= acmCreateHardwareInOutSignal(signalID, deviceAppSignal);
		}

		for (const auto& [signalID, deviceAppSignal] : actuatorType.acmOutputs)
		{
			result &= acmCreateHardwareInOutSignal(signalID, deviceAppSignal);
		}

		PropertyVector<VFrame30::ActuatorSignal> swInputs = actuatorType.actuatorHeader.inputs();

		for (const auto& swInput : swInputs)
		{
			result &= acmCreateSoftwareInOutSignal(*swInput, E::SignalInOutType::Input);
		}

		PropertyVector<VFrame30::ActuatorSignal> swOutputs = actuatorType.actuatorHeader.outputs();

		for (const auto& swOutput : swOutputs)
		{
			result &= acmCreateSoftwareInOutSignal(*swOutput, E::SignalInOutType::Output);
		}

		result &= acmCreateInternalSignals();

		for (AppSignal* appSignal : *m_signals)
		{
			m_moduleSignals.emplace(calcHash(appSignal->appSignalID()), appSignal);
			appSignal->setAcquire(false);
		}

		return result;
	}

	bool ModuleLogicCompiler::acmCreateUalSignals()
	{
		m_ualSignals.clear();

		bool result = true;

		result &= writeUalItemsFile();

		result &= createUalItemSignalsList();

		result &= loopbacksPreprocessing();

		// primarily created signals
		//
//		result &= createUalSignalsFromInputAndTuningAcquiredSignals();
	    result &= acmCreateUalSignalsFromInputs();
//		result &= createUalSignalsForNonPlatformModules();
		result &= createUalSignalsFromBusComposers();
//		result &= createUalSignalsFromOptoValidity();
//		result &= createUalSignalsFromReceivers();

		RETURN_IF_FALSE(result);

		// secondary created signals
		//
		for (UalItem* ualItem : m_ualItems)
		{
			if (ualItem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			switch (ualItem->type())
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
				break;

			// UAL items that doesn't generate signals
			//
			case E::UalItemType::Terminator:
			case E::UalItemType::LoopbackSource:
			case E::UalItemType::LoopbackTarget:
				break;

			// Not supported UAL items in ACM
			//
			case E::UalItemType::Receiver: 
			case E::UalItemType::Transmitter:
				LOG_INTERNAL_ERROR(m_log);
				Q_ASSERT(false);
				result = false;
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
		for (UalItem* ualItem : m_ualItems)
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

	bool ModuleLogicCompiler::acmCreateUalSignalsFromInputs()
	{
		for (AppSignal* ioSignal : m_ioSignals)
		{
			TEST_PTR_CONTINUE(ioSignal);
			
			if (ioSignal->isInput() == false)
			{
				continue;
			}

			m_ualSignals.createSignal(ioSignal);
		}

		return true;
	}

	bool ModuleLogicCompiler::acmCreateHardwareInOutSignal(const QString& signalID, const std::shared_ptr<Hardware::DeviceAppSignal>& deviceAppSignal)
	{
		TEST_PTR_RETURN_FALSE(deviceAppSignal);

		AppSignal* appSignal = new AppSignal;

		QString errMsg = appSignal->initFromDeviceSignal(deviceAppSignal->equipmentIdTemplate(),
														 deviceAppSignal->signalType(),
														 deviceAppSignal->function(),
														 signalID,
														 signalID,
														 QString("Signal %1").arg(signalID),
														 deviceAppSignal->appSignalBusTypeId(),
														 deviceAppSignal->appSignalDataFormat(),
														 deviceAppSignal->signalSpecPropsStruct(),
														 false,
														 0,
														 0,
														 0);

		if (errMsg.isEmpty() == false)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Error createActuatorInOutSignal: %1").arg(errMsg));
			delete appSignal;
			return false;
		}

		m_actuatorSignals->append(appSignal, m_lm);
		m_ioSignals.push_back(appSignal);
		return true;
	}

	bool ModuleLogicCompiler::acmCreateSoftwareInOutSignal(const VFrame30::ActuatorSignal& as, E::SignalInOutType inOut)
	{
		if (inOut != E::SignalInOutType::Input && inOut != E::SignalInOutType::Output)
		{
			Q_ASSERT(false);
			return false;
		}

		for (int ch = 1; ch <= 2; ch++)
		{
			AppSignal* appSignal = new AppSignal;

			QString signalID = as.signalIdChannel1();

			if (ch == 2)
			{
				signalID = as.signalIdChannel2();
			}

			int chIndex = ch - 1;

			Q_ASSERT(chIndex == 0 || chIndex == 1);

			appSignal->setInOutType(inOut);
			appSignal->setAppSignalID(signalID);
			appSignal->setCustomAppSignalID(signalID);
			appSignal->setEquipmentID(m_lm->equipmentIdTemplate());
			appSignal->setCaption(QString("Signal %1").arg(signalID));

			appSignal->setSignalType(as.signalType());
			appSignal->setAnalogSignalFormat(as.analogFormat());
			appSignal->setBusTypeID(as.busTypeId());
			appSignal->setByteOrder(E::ByteOrder::BigEndian);

			int dataSize = 0;

			switch (as.signalType())
			{
			case E::SignalType::Discrete:
				dataSize = DISCRETE_SIZE;

				if (inOut == E::SignalInOutType::Input)
				{
					m_acmSwInDiscretes[chIndex].append(signalID);
				}
				else
				{
					m_acmSwOutDiscretes[chIndex].append(signalID);
				}

				break;

			case E::SignalType::Analog:
				{
					switch (as.analogFormat())
					{
					case E::AnalogAppSignalFormat::Float32:
						dataSize = FLOAT32_SIZE;
						break;

					case E::AnalogAppSignalFormat::SignedInt32:
						dataSize = SIGNED_INT32_SIZE;
						break;

					default:
						Q_ASSERT(false);
						dataSize = 0;
					}

					if (inOut == E::SignalInOutType::Input)
					{
						m_acmSwInAnalogs[chIndex].append(signalID);
					}
					else
					{
						m_acmSwOutAnalogs[chIndex].append(signalID);
					}
				}

				break;

			case E::SignalType::Bus:
				{
					BusShared bus = m_actuatorSignals->getBus(as.busTypeId());

					if (bus == nullptr)
					{
						// Bus type ID %1 of signal %2 is undefined.
						//
						m_log->errALC5092(as.busTypeId(), signalID);
						Q_ASSERT(false);
						return false;
					}

					dataSize = bus->sizeBit();

					if (inOut == E::SignalInOutType::Input)
					{
						m_acmSwInBusses[chIndex].append(signalID);
					}
					else
					{
						m_acmSwOutBusses[chIndex].append(signalID);
					}
				}
				break;

			default:
				Q_ASSERT(false);
			}

			Q_ASSERT(dataSize != 0);

			appSignal->setDataSize(dataSize);

			m_actuatorSignals->append(appSignal, m_lm);
			m_ioSignals.push_back(appSignal);
		}

		return true;
	}

	bool ModuleLogicCompiler::acmCreateInternalSignals()
	{
		for (const UalItem* item : m_ualItems)
		{
			TEST_PTR_CONTINUE(item);

			if (item->isSignal() == false)
			{
				continue;
			}

			QString appSignalID = item->strID();

			if (m_actuatorSignals->contains(appSignalID))
			{
				continue;
			}

			AppSignal* appSignal = new AppSignal;

			appSignal->setInOutType(E::SignalInOutType::Internal);
			appSignal->setAppSignalID(appSignalID);
			appSignal->setCustomAppSignalID(appSignalID);
			appSignal->setEquipmentID(m_lm->equipmentIdTemplate());
			appSignal->setCaption(QString("Signal %1").arg(appSignalID));
			appSignal->setAcquire(false);

			PinSignalType pinSignalType;

			bool res = acmDetectInternalSignalType(item, &pinSignalType);

			if (res == false)
			{
				Q_ASSERT(false);
				delete appSignal;
				continue;
			}

			appSignal->setSignalType(pinSignalType.signalType);
			appSignal->setAnalogSignalFormat(pinSignalType.analogFormat);
			appSignal->setBusTypeID(pinSignalType.busType);
			appSignal->setByteOrder(pinSignalType.byteOrder);
			appSignal->setDataSize(pinSignalType.dataSize);

			m_actuatorSignals->append(appSignal, m_lm);
		}

		return true;
	}

	bool ModuleLogicCompiler::acmCalculateIoSignalsAddresses()
	{
		bool result = true;

		int appDataOffset = m_lmDescription->memory().m_appDataOffset;
		int diagDataOffset = m_lmDescription->memory().m_txDiagDataOffset;

		std::vector<Hardware::DeviceController*> controllers = DeviceHelper::getChildControllers(m_lm.get());

		for (AppSignal* ioSignal : m_ioSignals)
		{
			TEST_PTR_CONTINUE(ioSignal);

			if (ioSignal->appSignalID().endsWith(SW_INOUT_SUFFIX1) || ioSignal->appSignalID().endsWith(SW_INOUT_SUFFIX2))
			{
				continue;			// this is Software InOut signal!
			}

			int pos = ioSignal->appSignalID().lastIndexOf('_');

			if (pos == -1)
			{
				Q_ASSERT(false);
				continue;
			}

			QString suffix = ioSignal->appSignalID().mid(pos);

			Hardware::DeviceAppSignal* deviceAppSignal = nullptr;

			for (const Hardware::DeviceController* ctrl : controllers)
			{
				deviceAppSignal = DeviceHelper::getChildDeviceAppSignalBySuffix(ctrl, suffix, nullptr);

				if (deviceAppSignal != nullptr)
				{
					break;
				}
			}

			if (deviceAppSignal == nullptr)
			{
				Q_ASSERT(false);
				continue;		
			}

			Address16 ioBufAddr(deviceAppSignal->valueOffset(), deviceAppSignal->valueBit());

			switch (deviceAppSignal->memoryArea())
			{
			case E::MemoryArea::ApplicationData:

				switch (ioSignal->inOutType())
				{
				case E::SignalInOutType::Input:
				case E::SignalInOutType::Output:
					ioBufAddr.addWord(appDataOffset);
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

				switch (ioSignal->inOutType())
				{
				case E::SignalInOutType::Input:
				case E::SignalInOutType::Output:
					ioBufAddr.addWord(diagDataOffset);
					ioSignal->setIoBufAddr(ioBufAddr);
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

			if (ioSignal->isInput())
			{
				UalSignal* ualSignal = m_ualSignals.get(ioSignal->appSignalID());

				if (ualSignal == nullptr)
				{
					Q_ASSERT(false);
					result = false;
				}
				else
				{
					ualSignal->setUalAddr(ioSignal->ioBufAddr());
				}
			}
		}

		return result;
	}

	bool ModuleLogicCompiler::acmSetInOutSignalsUalAddresses()
	{
		bool result = true;

		// set ualAddress of InOut signals to ioBufAddr of input and output signal
		//
		for (const AppSignal* ioSignal : m_ioSignals)
		{
			if (ioSignal == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			if (ioSignal->isInput() == false && ioSignal->isOutput() == false)
			{
				Q_ASSERT(false);
				continue;
			}

			UalSignal* ualSignal = m_ualSignals.get(ioSignal->appSignalID());

			if (ualSignal == nullptr)
			{
				continue; // is not an error
			}

			// if (ualSignal->isInput() == false || (ualSignal->isDiscrete() == false && ioSignal->isBus() == false))
			//{
			//	Q_ASSERT(false); // ualSignal must be Discrete or Bus Input if reffered by ioSignal
			//	LOG_INTERNAL_ERROR(m_log);
			//	result = false;
			//	continue;
			// }

			if (ualSignal->isDiscrete() && ualSignal->invertSignal())
			{
				continue; // UalAddr for inverted signals will be set later
			}

			ualSignal->setUalAddr(ioSignal->ioBufAddr());
		}

		return result;
	}

	bool ModuleLogicCompiler::acmDisposeSignalsInMemory()
	{
		if (isActuatorCompiler() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		bool result = false;

		do
		{
			BREAK_IF_FALSE(acmDisposeSwInOuts());
			BREAK_IF_FALSE(acmCalculateIoSignalsAddresses());
			BREAK_IF_FALSE(acmWriteIoSignalsAddrsFile());

			// if (setInOutSignalsUalAddresses() == false)	break;

			if (disposeDiscreteSignalsInBitMemory() == false)
				break;

			if (disposeDiscreteSignalsHeap() == false)
				break;

			if (disposeNonAcquiredAnalogSignals() == false)
				break;

			if (disposeNonAcquiredBuses() == false)
				break;

			if (disposeNonAcquiredDiscreteInvertedInputSignals() == false)
				break;

			if (disposeAnalogAndBusSignalsHeap() == false)
				break;

			result = true;
			break;
		} 
		while (false);

		return result;
	}

	bool ModuleLogicCompiler::acmDisposeSwInOuts()
	{
		bool result = true;

		QStringList allIDs;

		for (int ch = ACM_CHANNEL_1_INDEX; ch <= ACM_CHANNEL_2_INDEX; ch++)
		{
			m_acmSwInAnalogs[ch].sort();
			m_acmSwInBusses[ch].sort();
			m_acmSwInDiscretes[ch].sort();

			allIDs.append(m_acmSwInAnalogs[ch]);
			allIDs.append(m_acmSwInBusses[ch]);
			allIDs.append(m_acmSwInDiscretes[ch]);

			result &= acmDisposeSwInOutsChannel(ch, m_acmSwInAnalogs[ch], m_acmSwInBusses[ch], m_acmSwInDiscretes[ch]);

			//

			m_acmSwOutAnalogs[ch].sort();
			m_acmSwOutBusses[ch].sort();
			m_acmSwOutDiscretes[ch].sort();

			allIDs.append(m_acmSwOutAnalogs[ch]);
			allIDs.append(m_acmSwOutBusses[ch]);
			allIDs.append(m_acmSwOutDiscretes[ch]);

			result &= acmDisposeSwInOutsChannel(ch, m_acmSwOutAnalogs[ch], m_acmSwOutBusses[ch], m_acmSwOutDiscretes[ch]);
		}

		m_acmSwInOutID = 0;

		for (const QString& id : allIDs)
		{
			m_acmSwInOutID = calcHash(id, m_acmSwInOutID);

			const AppSignal* s = m_actuatorSignals->getSignal(id);

			if (s == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			quint32 v = static_cast<quint32>(s->inOutType());

			m_acmSwInOutID = calcHash(&v, sizeof(v), m_acmSwInOutID);

			v = static_cast<quint32>(s->signalType());

			m_acmSwInOutID = calcHash(&v, sizeof(v), m_acmSwInOutID);

			switch (s->signalType())
			{
			case E::SignalType::Analog:
				v = static_cast<quint32>(s->analogSignalFormat());
				m_acmSwInOutID = calcHash(&v, sizeof(v), m_acmSwInOutID);
				break;

			case E::SignalType::Bus:
				m_acmSwInOutID = calcHash(s->busTypeID(), m_acmSwInOutID);
				break;

			case E::SignalType::Discrete:
				break;

			default:
				Q_ASSERT(false);
			}
		}

		return true;
	}

	bool ModuleLogicCompiler::acmDisposeSwInOutsChannel(int chIndex, QStringList& analogs, QStringList& busses, QStringList& discretes)
	{
		bool result = true;

		Address16 addr(m_lmDescription->memory().m_moduleDataOffset + ACM_SW_INOUT_ID_SIZE, 0);

		if (chIndex == ACM_CHANNEL_2_INDEX)
		{
			addr.setOffset(m_lmDescription->memory().m_moduleDataOffset + ACM_SW_INOUT_ID_SIZE + m_lmDescription->memory().m_moduleDataSize);
		}

		QStringList analogsAndBusses;

		analogsAndBusses.append(analogs);
		analogsAndBusses.append(busses);

		for (const QString& id : analogsAndBusses)
		{
			AppSignal* appSignal = m_actuatorSignals->getSignal(id);

			if (appSignal == nullptr)
			{
				Q_ASSERT(false);
				result = false;
				break;
			}

			appSignal->setIoBufAddr(addr);

			if ((appSignal->dataSize() % SIZE_16BIT) != 0)
			{
				Q_ASSERT(false);
				result = false;
				break;
			}

			addr.addBit(appSignal->dataSize());

			if (addr.bit() != 0)
			{
				Q_ASSERT(false);
				result = false;
				break;
			}

			//

			UalSignal* ualSignal = m_ualSignals.get(id);

			if (ualSignal == nullptr)
			{
				//				Q_ASSERT(false);
				// result = false;
				//				break;

				qDebug() << C_STR(QString("UalSignal not found for %1").arg(appSignal->appSignalID()));
				continue;
			}

			if (ualSignal->ualAddr().isValid())
			{
				continue;
			}

			ualSignal->setUalAddr(addr);
		}

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
		}

		for (const QString& id : discretes)
		{
			AppSignal* appSignal = m_actuatorSignals->getSignal(id);

			if (appSignal == nullptr)
			{
				Q_ASSERT(false);
				result = false;
				break;
			}
			
			if (appSignal->dataSize() != DISCRETE_SIZE)
			{
				Q_ASSERT(false);
				result = false;
				break;
			}

			appSignal->setIoBufAddr(addr);
			addr.addBit(appSignal->dataSize());

			//

			UalSignal* ualSignal = m_ualSignals.get(id);

			if (ualSignal == nullptr)
			{
//				Q_ASSERT(false);
				//result = false;
//				break;

				qDebug() << C_STR(QString("UalSignal not found for %1").arg(appSignal->appSignalID()));
				continue;
			}

			if (ualSignal->ualAddr().isValid())
			{
				continue;
			}

			ualSignal->setUalAddr(addr);
		}

		if (result == false)
		{
			LOG_INTERNAL_ERROR(m_log);
		}

		return result;
	}

	bool ModuleLogicCompiler::acmCreateSignalLists()
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_lm, m_log);

		bool result = true;

		// result &= createAcquiredDiscreteInputSignalsList();
		// result &= createAcquiredDiscreteStrictOutputSignalsList();
		// result &= createAcquiredDiscreteInternalSignalsList();
		// result &= createAcquiredDiscreteOptoSignalsList();
		// result &= createAcquiredDiscreteBusChildSignalsList();
		// result &= createAcquiredDiscreteTuningSignalsList();
		// result &= createAcquiredDiscreteConstSignalsList();

		result &= createNonAcquiredDiscreteInputSignalsList();
		result &= createNonAcquiredDiscreteStrictOutputSignalsList();
		result &= createNonAcquiredDiscreteInternalSignalsList();

		// result &= createAcquiredAnalogInputSignalsList();
		// result &= createAcquiredAnalogStrictOutputSignalsList();
		// result &= createAcquiredAnalogInternalSignalsList();
		// result &= createAcquiredAnalogOptoSignalsList();
		// result &= createAcquiredAnalogBusChildSignalsList();
		// result &= createAcquiredAnalogTuninglSignalsList();
		// result &= createAcquiredAnalogConstSignalsList();

		// result &= createNonAcquiredAnalogInputSignalsList();			// !!! maybe NO
		result &= createNonAcquiredAnalogStrictOutputSignalsList();
		result &= createNonAcquiredAnalogInternalSignalsList();

		result &= createAnalogOutputSignalsToConversionList();

		// result &= createAcquiredInputBusesList();
		// result &= createAcquiredOutputBusesList();
		// result &= createAcquiredInternalBusesList();
		// result &= createAcquiredBusBusChildSignalsList();
		// result &= createAcquiredOptoBusesList();

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
		// sortSignalList(m_acquiredDiscreteInputSignals);
		// sortSignalList(m_acquiredDiscreteInvertedInputSignals);
		// sortSignalList(m_acquiredDiscreteStrictOutputSignals);
		// sortSignalList(m_acquiredDiscreteInternalSignals);
		// sortSignalList(m_acquiredDiscreteOptoSignals);
		// sortSignalList(m_acquiredDiscreteBusChildSignals);
		// sortSignalList(m_acquiredDiscreteConstSignals);
		//	m_acquiredDiscreteTuningSignals;		sorting not required

		// Analogs
		//
		sortSignalList(m_nonAcquiredDiscreteInputSignals);
		sortSignalList(m_nonAcquiredDiscreteInvertedInputSignals);
		sortSignalList(m_nonAcquiredDiscreteStrictOutputSignals);
		sortSignalList(m_nonAcquiredDiscreteInternalSignals);

		// sortSignalList(m_acquiredAnalogInputSignals);
		// sortSignalList(m_acquiredAnalogStrictOutputSignals);
		// sortSignalList(m_acquiredAnalogInternalSignals);
		// sortSignalList(m_acquiredAnalogOptoSignals);
		// sortSignalList(m_acquiredAnalogBusChildSignals);
		//	m_acquiredAnalogTuningSignals;			sorting not required

		sortSignalList(m_nonAcquiredAnalogInputSignals);
		sortSignalList(m_nonAcquiredAnalogStrictOutputSignals);
		sortSignalList(m_nonAcquiredAnalogInternalSignals);

		// Buses
		//
		// sortSignalList(m_acquiredInputBuses);
		// sortSignalList(m_acquiredOutputBuses);
		// sortSignalList(m_acquiredInternalBuses);

		// sortSignalList(m_acquiredOptoBuses); // To DO sorting - group by OptoPortID, and next by addr in port buf for sequential move optimization

		// sortSignalList(m_acquiredBusChildBuses);

		sortSignalList(m_nonAcquiredOutputBuses);
		sortSignalList(m_nonAcquiredInternalBuses);

		sortSignalList(m_discreteInvertedOutputSignals);

		return result;
	}

	bool ModuleLogicCompiler::acmDetectInternalSignalType(const UalItem* itemSignal, PinSignalType* pinSignalType)
	{
		TEST_PTR_RETURN_FALSE(itemSignal);
		TEST_PTR_RETURN_FALSE(pinSignalType);

		if (itemSignal->isSignal() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		QString id = itemSignal->strID();
		QString label = itemSignal->label();

		const std::vector<SchemaPin>& inputs = itemSignal->inputs();

		if (inputs.size() == 0 || inputs.size() > 1)
		{
			//			Q_ASSERT(false);
			return true; // TO DO check outputs
		}

		QUuid outPinUuid;

		UalItem* outPinParent = getAssociatedOutputPinParent(inputs[0], &outPinUuid);

		if (outPinParent == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		bool result = false;

		switch (outPinParent->type())
		{
		case E::UalItemType::Unknown:
		case E::UalItemType::Transmitter:
		case E::UalItemType::Receiver:
		case E::UalItemType::Terminator:
		case E::UalItemType::Const:
		case E::UalItemType::BusComposer:
		case E::UalItemType::BusExtractor:
		case E::UalItemType::LoopbackSource:
		case E::UalItemType::LoopbackTarget:
			Q_ASSERT(false); // TO DO
			LOG_INTERNAL_ERROR(m_log);
			return false;

		case E::UalItemType::Signal:
			{
				QString srcSignalID = outPinParent->strID();

				const AppSignal* as = m_actuatorSignals->getSignal(srcSignalID);

				if (as == nullptr)
				{
					Q_ASSERT(false);
					result = false;
				}
				else
				{
					pinSignalType->signalType = as->signalType();
					pinSignalType->analogFormat = as->analogSignalFormat();
					pinSignalType->busType = as->busTypeID();
					pinSignalType->byteOrder = as->byteOrder();
					pinSignalType->dataSize = as->dataSize();
					result = true;
				}
			}
			break;

		case E::UalItemType::Afb:
			result = acmDetectAfbOutSignalType(outPinParent, outPinUuid, pinSignalType);
			break;

		default:
			Q_ASSERT(false);
			result = false;
		}

		return result;
	}

	bool ModuleLogicCompiler::acmDetectAfbOutSignalType(const UalItem* item, const QUuid& pinUuid, PinSignalType* pinSignalType)
	{
		TEST_PTR_RETURN_FALSE(item);
		TEST_PTR_RETURN_FALSE(pinSignalType);

		if (item->isAfb() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		int afbOperandIndex = -1;

		for (const SchemaPin& out : item->outputs())
		{
			if (out.guid() == pinUuid)
			{
				afbOperandIndex = out.afbOperandIndex();
				break;
			}
		}

		if (afbOperandIndex == -1)
		{
			Q_ASSERT(false);
			return false;
		}

		const Afb::AfbElement& afbElement = item->afb();

		for (const AfbSignal& afbSignal : afbElement.outputSignals())
		{
			if (afbSignal.operandIndex() == afbOperandIndex)
			{
				pinSignalType->signalType = afbSignal.type();

				switch (afbSignal.type())
				{
				case E::SignalType::Discrete:
					Q_ASSERT(afbSignal.size() == 1);
					pinSignalType->dataSize = 1;
					break;

				case E::SignalType::Analog:
					{
						E::DataFormat dataFormat = afbSignal.dataFormat();
						int dataSize = afbSignal.size();

						Q_ASSERT(dataSize == 32);

						switch (dataFormat)
						{
						case E::DataFormat::Float:
							pinSignalType->analogFormat = E::AnalogAppSignalFormat::Float32;
							break;

						case E::DataFormat::SignedInt:
							pinSignalType->analogFormat = E::AnalogAppSignalFormat::SignedInt32;
							break;

						case E::DataFormat::UnsignedInt:
						default:
							Q_ASSERT(false);
						}
					}
					break;

				case E::SignalType::Bus:
				default:
					Q_ASSERT(false);
				}

				pinSignalType->byteOrder = afbSignal.byteOrder();
				pinSignalType->busType.clear();

				return true;
			}
		}

		return false;
	}

	bool ModuleLogicCompiler::acmWriteIoSignalsAddrsFile()
	{
		TEST_PTR_RETURN_FALSE(m_resultWriter);

		static const QString line("--------------------------------------------------------------------------------");
		static const QString line2("================================================================================");
		static const QString addrLine(" IO buf Address | AppSignalID");

		QStringList file;

		auto printSignals = [&](const QStringList& ids)
		{
			for (const QString& id : ids)
			{
				AppSignal* appSignal = m_actuatorSignals->getSignal(id);

				TEST_PTR_CONTINUE(appSignal);

				if (appSignal->ioBufAddr().isValid() == false)
				{
					Q_ASSERT(false);
					continue;
				}

				QString str = QString(" %1       | %2").arg(appSignal->ioBufAddr().toString(true)).arg(appSignal->appSignalID());

				file.append(str);
			}
		};

		for (int inOut = 0; inOut < 2; inOut++)
		{
			for (int ch = ACM_CHANNEL_1_INDEX; ch <= ACM_CHANNEL_2_INDEX; ch++)
			{
				QString inOutStr = "inputs";

				QStringList& analogs = m_acmSwInAnalogs[ch];
				QStringList& busses = m_acmSwInBusses[ch];
				QStringList& discretes = m_acmSwInDiscretes[ch];

				if (inOut == 1)
				{
					inOutStr = "outputs";

					analogs = m_acmSwOutAnalogs[ch];
					busses = m_acmSwOutBusses[ch];
					discretes = m_acmSwOutDiscretes[ch];
				}

				file.append(Separator::EMPTY_STR);
				file.append(line2);
				file.append(QString(" Channel %1 %2").arg(ch + 1).arg(inOutStr));
				file.append(line2);
				file.append(" Analogs");
				file.append(line);
				file.append(addrLine);
				file.append(line);
				printSignals(analogs);

				file.append(line);
				file.append(" Busses");
				file.append(line);
				file.append(addrLine);
				file.append(line);
				printSignals(busses);

				file.append(line);
				file.append(" Discretes");
				file.append(line);
				file.append(addrLine);
				file.append(line);
				printSignals(discretes);
				file.append(line);
			}
		}

		BuildFile* bf = m_resultWriter->addFile(QString("Subsystems/%1").arg(m_lmSubsystemID), QString("%1-%2-sw-inouts.txt").arg(m_lmSubsystemID).arg(m_lmNumber), file, false);

		return (bf != nullptr);
	}
}