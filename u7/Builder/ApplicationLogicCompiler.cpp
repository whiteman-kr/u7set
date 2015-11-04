#include "ApplicationLogicCompiler.h"
#include "../VFrame30/Afb.h"

namespace Builder
{

#define ASSERT_RESULT_FALSE_BREAK	assert(false); \
									result = false; \
									break;

#define RESULT_FALSE_BREAK			result = false; \
									break;

#define ASSERT_RETURN_FALSE			assert(false); \
									return false;

#define TEST_PTR_RETURN_FALSE(ptr)	if (ptr == nullptr) \
									{	\
										assert(false);	\
										return false; \
									}

	// ---------------------------------------------------------------------------------
	//
	//	MemoryArea class implementation
	//

	void MemoryArea::setStartAddress(int startAddress)
	{
		assert(m_locked == false);
		m_startAddress = startAddress;
		m_nextSignalAddress.set(startAddress, 0);
	}


	MemoryArea& MemoryArea::operator = (const MemoryArea& ma)
	{
		assert(m_locked == false);

		m_startAddress = ma.m_startAddress;
		m_sizeW = ma.m_sizeW;

		return *this;
	}


	Address16 MemoryArea::appendSignal(const Signal& signal)
	{
		Address16 signalAddress;

		if (signal.isAnalog())
		{
			// do word-align
			//
			m_nextSignalAddress.wordAlign();
		}

		signalAddress = m_nextSignalAddress;

		if (signal.isAnalog())
		{
			m_nextSignalAddress.addWord(signal.sizeW());
		}
		else
		{
			m_nextSignalAddress.add1Bit();
		}

		m_signals.append(SignalAddress16(signal.strID(), signalAddress, signal.sizeW(), signal.isDiscrete()));

		m_sizeW = m_nextSignalAddress.offset() - m_startAddress;

		if (m_nextSignalAddress.bit() > 0)
		{
			m_sizeW += 1;
		}

		return signalAddress;
	}

	// ---------------------------------------------------------------------------------
	//
	//	LmMemoryMap class implementation
	//

	LmMemoryMap::LmMemoryMap(OutputLog *log) :
		m_log(log)
	{
		assert(m_log != nullptr);
	}


	bool LmMemoryMap::init(	const MemoryArea& moduleData,
							const MemoryArea& optoInterfaceData,
							const MemoryArea& appLogicBitData,
							const MemoryArea& tuningData,
							const MemoryArea& appLogicWordData,
							const MemoryArea& lmDiagData,
							const MemoryArea& lmIntOutData)
	{

		// init modules memory mapping
		//
		m_modules.memory.setStartAddress(moduleData.startAddress());
		m_modules.memory.setSizeW(moduleData.sizeW() * MODULES_COUNT);
		m_modules.memory.lock();

		for(int i = 0; i < MODULES_COUNT; i++)
		{
			m_modules.module[i].setStartAddress(m_modules.memory.startAddress() + i * moduleData.sizeW());
			m_modules.module[i].setSizeW(moduleData.sizeW());
		}

		// init opto interface memory mapping
		//
		m_optoInterface.memory.setStartAddress(optoInterfaceData.startAddress());
		m_optoInterface.memory.setSizeW(optoInterfaceData.sizeW() * (OPTO_INTERFACE_COUNT + 1));
		m_optoInterface.memory.lock();

		for(int i = 0; i < OPTO_INTERFACE_COUNT; i++)
		{
			if (i == 0)
			{
				m_optoInterface.channel[i].setStartAddress(m_optoInterface.memory.startAddress());
			}
			else
			{
				m_optoInterface.channel[i].setStartAddress(m_optoInterface.channel[i-1].nextAddress());
			}

			m_optoInterface.channel[i].setSizeW(optoInterfaceData.sizeW());
		}

		m_optoInterface.result.setStartAddress(m_optoInterface.channel[OPTO_INTERFACE_COUNT -1].nextAddress());
		m_optoInterface.result.setSizeW(optoInterfaceData.sizeW());

		// init application bit-addressed memory mapping
		//
		m_appBitAdressed.memory = appLogicBitData;
		m_appBitAdressed.memory.lock();

		m_appBitAdressed.regDiscretSignals.setStartAddress(appLogicBitData.startAddress());
		m_appBitAdressed.nonRegDiscretSignals.setStartAddress(appLogicBitData.startAddress());

		// init tuning interface memory mapping
		//
		m_tuningInterface.memory = tuningData;
		m_tuningInterface.memory.lock();

		// init application word-addressed memory mapping
		//
		m_appWordAdressed.memory = appLogicWordData;
		m_appWordAdressed.memory.lock();

		m_appWordAdressed.lmDiagnostics.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.lmInputs.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.lmOutputs.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.regAnalogSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.regDiscreteSignals.setStartAddress(appLogicWordData.startAddress());
		m_appWordAdressed.nonRegAnalogSignals.setStartAddress(appLogicWordData.startAddress());

		// init LM diagnostics memory mapping
		//
		m_lmDiagnostics.memory = lmDiagData;
		m_lmDiagnostics.memory.lock();

		// init LM in/out controller memory mapping
		//
		m_lmInOuts.memory = lmIntOutData;
		m_lmInOuts.memory.lock();

		return recalculateAddresses();
	}


	bool LmMemoryMap::recalculateAddresses()
	{
		// recalc application bit-addressed memory mapping
		//

		m_appBitAdressed.nonRegDiscretSignals.setStartAddress(m_appBitAdressed.regDiscretSignals.nextAddress());

		if (m_appBitAdressed.nonRegDiscretSignals.nextAddress() > m_appBitAdressed.memory.nextAddress())
		{
			LOG_ERROR(m_log, tr("Out of bit-addressed memory range!"));

			return false;
		}

		// recalc application word-addressed memory mapping
		//

		// LM diagnostics

		m_appWordAdressed.lmDiagnostics.setStartAddress(m_appWordAdressed.memory.startAddress());
		m_appWordAdressed.lmDiagnostics.setSizeW(m_lmDiagnostics.memory.sizeW());

		// LM input discrete signals

		m_appWordAdressed.lmInputs.setStartAddress(m_appWordAdressed.lmDiagnostics.nextAddress());
		m_appWordAdressed.lmInputs.setSizeW(m_lmInOuts.memory.sizeW());

		// LM output discrete signals

		m_appWordAdressed.lmOutputs.setStartAddress(m_appWordAdressed.lmInputs.nextAddress());
		m_appWordAdressed.lmOutputs.setSizeW(m_lmInOuts.memory.sizeW());

		// modules data

		for(int i = 0; i < MODULES_COUNT; i++)
		{
			if (i == 0)
			{
				m_appWordAdressed.module[0].setStartAddress(m_appWordAdressed.lmOutputs.nextAddress());
			}
			else
			{
				m_appWordAdressed.module[i].setStartAddress(m_appWordAdressed.module[i-1].nextAddress());
			}
		}

		// registered analog signals

		m_appWordAdressed.regAnalogSignals.setStartAddress(m_appWordAdressed.module[MODULES_COUNT - 1].nextAddress());

		// registered discrete signals

		m_appWordAdressed.regDiscreteSignals.setStartAddress(m_appWordAdressed.regAnalogSignals.nextAddress());
		m_appWordAdressed.regDiscreteSignals.setSizeW(m_appBitAdressed.regDiscretSignals.sizeW());

		// non registered analog signals

		m_appWordAdressed.nonRegAnalogSignals.setStartAddress(m_appWordAdressed.regDiscreteSignals.nextAddress());

		if (m_appWordAdressed.nonRegAnalogSignals.nextAddress() > m_appWordAdressed.memory.nextAddress())
		{
			LOG_ERROR(m_log, tr("Out of word-addressed memory range!"));

			return false;
		}

		return true;
	}


	int LmMemoryMap::getModuleDataOffset(int place)
	{
		assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

		return m_modules.module[place - 1].startAddress();;
	}


	int LmMemoryMap::getModuleRegDataOffset(int place)
	{
		assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

		return m_appWordAdressed.module[place - 1].startAddress();;
	}


	int LmMemoryMap::addModule(int place, int moduleAppRegDataSize)
	{
		assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

		m_appWordAdressed.module[place - 1].setSizeW(moduleAppRegDataSize);

		recalculateAddresses();

		return getModuleRegDataOffset(place);
	}


	void LmMemoryMap::getFile(QStringList& memFile)
	{
		memFile.append(QString(" LM's memory map"));
		memFile.append("");

		//

		addSection(memFile, m_modules.memory, "I/O modules controller memory");

		for(int i = 0; i < MODULES_COUNT; i++)
		{
			addRecord(memFile, m_modules.module[i], QString().sprintf("I/O module %02d", i + 1));
		}

		memFile.append("");

		addSection(memFile, m_optoInterface.memory, "Opto interfaces memory");

		//

		for(int i = 0; i < OPTO_INTERFACE_COUNT; i++)
		{
			addRecord(memFile, m_optoInterface.channel[i], QString().sprintf("opto interface %02d", i + 1));
		}

		memFile.append("");

		addRecord(memFile, m_optoInterface.result, "opto interfaces data processing result");

		memFile.append("");

		//

		addSection(memFile, m_appBitAdressed.memory, "Application logic bit-addressed memory");

		addRecord(memFile, m_appBitAdressed.regDiscretSignals, "registrated discrete signals");

		memFile.append("");

		addSignals(memFile, m_appBitAdressed.regDiscretSignals);

		addRecord(memFile, m_appBitAdressed.nonRegDiscretSignals, "non-registrated discrete signals");

		memFile.append("");

		addSignals(memFile, m_appBitAdressed.nonRegDiscretSignals);

		//

		addSection(memFile, m_tuningInterface.memory, "Tuning interface memory");

		memFile.append("");

		//

		addSection(memFile, m_appWordAdressed.memory, "Application logic word-addressed memory");

		addRecord(memFile, m_appWordAdressed.lmDiagnostics, "LM's diagnostics data");
		addRecord(memFile, m_appWordAdressed.lmInputs, "LM's inputs state");
		addRecord(memFile, m_appWordAdressed.lmOutputs, "LM's outputs state");

		memFile.append("");

		for(int i = 0; i < MODULES_COUNT; i++)
		{
			addRecord(memFile, m_appWordAdressed.module[i], QString().sprintf("I/O module %02d data", i + 1));
		}

		memFile.append("");

		addRecord(memFile, m_appWordAdressed.regAnalogSignals, "registrated analogs signals");

		memFile.append("");

		addSignals(memFile, m_appWordAdressed.regAnalogSignals);

		addRecord(memFile, m_appWordAdressed.regDiscreteSignals, "registrated discrete signals (from bit-addressed memory)");

		memFile.append("");

		addSignals(memFile, m_appWordAdressed.regDiscreteSignals);

		addRecord(memFile, m_appWordAdressed.nonRegAnalogSignals, "non-registrated analogs signals");

		memFile.append("");

		addSignals(memFile, m_appWordAdressed.nonRegAnalogSignals);

		//

		addSection(memFile, m_lmDiagnostics.memory, "LM's diagnostics memory");
		addSection(memFile, m_lmInOuts.memory, "LM's inputs/outputs memory");
	}


	void LmMemoryMap::addSection(QStringList& memFile, MemoryArea& memArea, const QString& title)
	{
		memFile.append(QString().rightJustified(80, '-'));
		memFile.append(QString(" Address    Size      Description"));
		memFile.append(QString().rightJustified(80, '-'));

		QString str;
		str.sprintf(" %05d      %05d     %s", memArea.startAddress(), memArea.sizeW(), C_STR(title));

		memFile.append(str);
		memFile.append(QString().rightJustified(80, '-'));
		memFile.append("");
	}


	void LmMemoryMap::addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title)
	{
		QString str;

		str.sprintf(" %05d      %05d     %s", memArea.startAddress(), memArea.sizeW(), C_STR(title));

		memFile.append(str);
	}


	void LmMemoryMap::addSignals(QStringList& memFile, MemoryArea& memArea)
	{
		if (memArea.hasSignals() == false)
		{
			return;
		}

		QVector<MemoryArea::SignalAddress16>& signalsArray = memArea.getSignals();

		for(MemoryArea::SignalAddress16& signal : signalsArray)
		{
			QString str;

			if (signal.isDiscrete())
			{
				str.sprintf(" %05d.%02d   00000.01  %s",
							signal.address().offset(), signal.address().bit(),
							C_STR(signal.signalStrID()));

			}
			else
			{
				str.sprintf(" %05d      %05d     %s",
							signal.address().offset(),
							signal.sizeW(),
							C_STR(signal.signalStrID()));
			}

			memFile.append(str);
		}

		memFile.append("");
	}


	Address16 LmMemoryMap::addRegDiscreteSignal(const Signal& signal)
	{
		assert(signal.isInternal() && signal.isRegistered() && signal.isDiscrete());

		return m_appBitAdressed.regDiscretSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::addRegDiscreteSignalToRegBuffer(const Signal& signal)
	{
		assert(signal.isInternal() && signal.isRegistered() && signal.isDiscrete());

		return m_appWordAdressed.regDiscreteSignals.appendSignal(signal);
	}

	Address16 LmMemoryMap::addNonRegDiscreteSignal(const Signal& signal)
	{
		assert(signal.isInternal() && !signal.isRegistered() && signal.isDiscrete());

		return m_appBitAdressed.nonRegDiscretSignals.appendSignal(signal);
	}


	Address16 LmMemoryMap::addRegAnalogSignal(const Signal& signal)
	{
		assert(signal.isInternal() && signal.isRegistered() && signal.isAnalog());

		return m_appWordAdressed.regAnalogSignals.appendSignal(signal);
	}


	Address16 LmMemoryMap::addNonRegAnalogSignal(const Signal& signal)
	{
		assert(signal.isInternal() && !signal.isRegistered() && signal.isAnalog());

		return m_appWordAdressed.nonRegAnalogSignals.appendSignal(signal);
	}

	double LmMemoryMap::bitAddressedMemoryUsed()
	{
		return double((m_appBitAdressed.regDiscretSignals.sizeW() + m_appBitAdressed.nonRegDiscretSignals.sizeW()) * 100) /
				double(m_appBitAdressed.memory.sizeW());
	}

	double LmMemoryMap::wordAddressedMemoryUsed()
	{
		return double((m_appWordAdressed.nonRegAnalogSignals.nextAddress() - m_appWordAdressed.memory.startAddress()) * 100) /
				double(m_appWordAdressed.memory.sizeW());
	}


	// ---------------------------------------------------------------------------------
	//
	//	ApplicationLogicCompiler class implementation
	//

	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems, Hardware::DeviceObject* equipment,
													   SignalSet* signalSet, Afb::AfbElementCollection *afblSet,
													   ApplicationLogicData* appLogicData, BuildResultWriter* buildResultWriter,
													   OutputLog *log) :
		m_subsystems(subsystems),
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

		if (m_subsystems == nullptr ||
			m_equipment == nullptr ||
			m_signals == nullptr ||
			m_afbl == nullptr ||
			m_appLogicData == nullptr ||
			m_resultWriter == nullptr)
		{
			msg = tr("%1: Invalid params. Compilation aborted.").arg(__FUNCTION__);

			LOG_ERROR(m_log, msg);

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
			LOG_MESSAGE(m_log, tr("Logic modules (LMs) not found!"));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Found logic modules (LMs): %1")).arg(m_lm.count()));
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

			LOG_ERROR(m_log, msg);

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

						LOG_WARNING(m_log, msg);

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


	bool ApplicationLogicCompiler::writeBinCodeForLm(QString subsysStrID, QString lmCaption, int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode)
	{
		if (m_resultWriter == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		int susbsysID = m_subsystems->ssKey(subsysStrID);

		if (susbsysID == -1)
		{
			LOG_ERROR(m_log, QString(tr("Undefined subsystem strID %1 assigned in LM %2")).arg(subsysStrID).arg(lmCaption));

			return false;
		}

		bool result = true;

		Hardware::ModuleFirmware moduleFirmware;

		moduleFirmware.init(lmCaption, subsysStrID, susbsysID, 0x0101, frameSize, frameCount,
						 m_resultWriter->projectName(), m_resultWriter->userName(), m_resultWriter->changesetID());

		QString errorMsg;

		if (!moduleFirmware.setChannelData(channel, frameSize, frameCount, appLogicBinCode, &errorMsg))
		{
			LOG_ERROR(m_log, errorMsg);

			result = false;
		}

		QByteArray moduleFirmwareFileData;

		if (!moduleFirmware.save(moduleFirmwareFileData, &errorMsg))
		{
			LOG_ERROR(m_log, errorMsg);
			result = false;
		}

		result &= m_resultWriter->addFile(moduleFirmware.subsysId(), moduleFirmware.caption() + ".alb", moduleFirmwareFileData);

		return result;
	}


	// ---------------------------------------------------------------------------------
	//
	//	ModuleLogicCompiler class implementation
	//

	ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm) :
		m_appLogicCompiler(appLogicCompiler),
		m_memoryMap(appLogicCompiler.m_log),
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
			LOG_ERROR(m_log, msg);
			return false;
		}

		m_chassis = dynamic_cast<Hardware::DeviceChassis*>(parent);

		if (m_chassis == nullptr)
		{
			assert(false);
			return false;
		}

		LOG_EMPTY_LINE(m_log)

		msg = QString(tr("Compilation for LM %1 was started...")).arg(m_lm->strId());

		LOG_MESSAGE(m_log, msg);

		bool result = false;

		do
		{
			if (!loadLMSettings()) break;

			if (!loadModulesSettings()) break;

			if (!prepareAppLogicGeneration()) break;

			if (!generateAppStartCommand()) break;

			if (!generateFbTestCode()) break;

			if (!initAfbs()) break;

			if (!finishTestCode()) break;

			if (!startAppLogicCode()) break;

			if (!copyLMDataToRegBuf()) break;

			if (!copyInModulesAppLogicDataToRegBuf()) break;

			if (!initOutModulesAppLogicDataInRegBuf()) break;

			if (!generateAppLogicCode()) break;

			if (!copyDiscreteSignalsToRegBuf()) break;

			if (!copyLmOutSignalsToModuleMemory()) break;

			if (!copyOutModulesAppLogicDataToModulesMemory()) break;

			if (!finishAppLogicCode()) break;

			if (!writeResult()) break;

			result = true;
		}
		while(false);

		if (result == true)
		{
			msg = QString(tr("Compilation for LM %1 was successfully finished.")).
					arg(m_lm->strId());

			LOG_SUCCESS(m_log, msg);

			QString str;

			if (m_code.commandAddress() != 0)
			{
				str.sprintf("%.2f", (m_code.commandAddress() * 100.0) / 65536.0);
			}
			else
			{
				str = "0.00";
			}

			LOG_MESSAGE(m_log, QString(tr("Code memory used - %1%")).arg(str));

			str.sprintf("%.2f", m_memoryMap.bitAddressedMemoryUsed());
			LOG_MESSAGE(m_log, QString(tr("Bit-addressed memory used - %1%")).arg(str));

			str.sprintf("%.2f", m_memoryMap.wordAddressedMemoryUsed());
			LOG_MESSAGE(m_log, QString(tr("Word-addressed memory used - %1%")).arg(str));
		}
		else
		{
			msg = QString(tr("Compilation for LM %1 was finished with errors")).arg(m_lm->strId());
			LOG_MESSAGE(m_log, msg);
		}

		cleanup();

		return result;
	}


	bool ModuleLogicCompiler::loadLMSettings()
	{
		bool result = true;

		MemoryArea m_moduleData;
		MemoryArea m_optoInterfaceData;
		MemoryArea m_appLogicBitData;
		MemoryArea m_tuningData;
		MemoryArea m_appLogicWordData;
		MemoryArea m_lmDiagData;
		MemoryArea m_lmIntOutData;

		const PropertyNameVar memSettings[] =
		{
			{	"ModuleDataOffset", m_moduleData.ptrStartAddress() },
			{	"ModuleDataSize", m_moduleData.ptrSizeW() },

			{	"OptoInterfaceDataOffset", m_optoInterfaceData.ptrStartAddress() },
			{	"OptoInterfaceDataSize", m_optoInterfaceData.ptrSizeW() },

			{	"AppLogicBitDataOffset", m_appLogicBitData.ptrStartAddress() },
			{	"AppLogicBitDataSize", m_appLogicBitData.ptrSizeW() },

			{	"TuningDataOffset", m_tuningData.ptrStartAddress() },
			{	"TuningDataSize", m_tuningData.ptrSizeW() },

			{	"AppLogicWordDataOffset", m_appLogicWordData.ptrStartAddress() },
			{	"AppLogicWordDataSize", m_appLogicWordData.ptrSizeW() },

			{	"LMDiagDataOffset", m_lmDiagData.ptrStartAddress() },
			{	"LMDiagDataSize", m_lmDiagData.ptrSizeW() },

			{	"LMInOutDataOffset", m_lmIntOutData.ptrStartAddress() },
			{	"LMInOutDataSize", m_lmIntOutData.ptrSizeW() }
		};

		for(PropertyNameVar memSetting : memSettings)
		{
			result &= getLMIntProperty(memSetting.name, memSetting.var);
		}

		if (result == true)
		{
			m_memoryMap.init(m_moduleData,
							 m_optoInterfaceData,
							 m_appLogicBitData,
							 m_tuningData,
							 m_appLogicWordData,
							 m_lmDiagData,
							 m_lmIntOutData);
		}

		result &= getLMIntProperty("AppLogicFrameSize", &m_lmAppLogicFrameSize);
		result &= getLMIntProperty("AppLogicFrameCount", &m_lmAppLogicFrameCount);

		result &= getLMIntProperty("CycleDuration", &m_lmCycleDuration);

		if (result)
		{
			LOG_MESSAGE(m_log, QString(tr("Loading LMs settings... Ok")));
		}
		else
		{
			LOG_ERROR(m_log, QString(tr("LM settings are not loaded")));
		}

		return result;
	}


	bool ModuleLogicCompiler::loadModulesSettings()
	{
		bool result = true;

		m_modules.clear();

		// build Module structures array
		//
		for(int place = FIRST_MODULE_PLACE; place <= LAST_MODULE_PLACE; place++)
		{
			Module m;

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
				{	"AppLogicDataSizeWithReserve", &m.appLogicDataSizeWithReserve },

				{	"AppLogicRegDataSize", &m.appLogicRegDataSize },
			};

			for(PropertyNameVar moduleSetting : moduleSettings)
			{
				result &= getDeviceIntProperty(device, moduleSetting.name, moduleSetting.var);
			}

			m.rxTxDataOffset = m_memoryMap.getModuleDataOffset(place);
			m.moduleAppDataOffset = m.rxTxDataOffset + m.appLogicDataOffset;
			m.appLogicRegDataOffset = m_memoryMap.addModule(place, m.appLogicRegDataSize);

			m_modules.append(m);
		}

		if (result)
		{
			LOG_MESSAGE(m_log, QString(tr("Loading modules settings... Ok")));
		}
		else
		{
			LOG_ERROR(m_log, QString(tr("Modules settings are not loaded")));
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
			LOG_WARNING(m_log, msg);
		}

		do
		{
			if (!buildServiceMaps()) break;

			if (!createDeviceBoundSignalsMap()) break;

			if (!appendFbsForAnalogInOutSignalsConversion()) break;

			if (!createAppFbsMap()) break;

			if (!createAppSignalsMap()) break;

			if (!calculateLmMemoryMap()) break;

			if (!calculateInOutSignalsAddresses()) break;

			if (!calculateInternalSignalsAddresses()) break;

			result = true;
		}
		while(false);

		return result;
	}


	bool ModuleLogicCompiler::calculateLmMemoryMap()
	{
		return true;
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


	bool ModuleLogicCompiler::generateFbTestCode()
	{
		Comment comment;

		comment.setComment("Start of FB's testing code");

		m_code.append(comment);
		m_code.newLine();

		// implement testing code generation

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

				if (appFb->hasRam())
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

		m_code.newLine();

		bool commandAdded = false;

		for(const AppFbParamValue& paramValue : appFbParamValues)
		{
			int operandIndex = paramValue.operandIndex();

			if (operandIndex == NOT_FB_OPERAND_INDEX)
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
					LOG_ERROR(m_log, QString(tr("Afb parameter '%1' with Float data format must have dataSize == 32")).arg(opName));
					result = false;
					break;

				default:
					assert(false);

					LOG_ERROR(m_log, tr("Unknown Afb parameter data format"));
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


	bool ModuleLogicCompiler::finishTestCode()
	{
		Command cmd;

		cmd.stop();

		m_code.comment(tr("End of FB's testing and initialization code section"));
		m_code.newLine();
		m_code.append(cmd);
		m_code.newLine();

		return true;
	}


	bool ModuleLogicCompiler::copyLMDataToRegBuf()
	{
		Command cmd;

		cmd.movMem(m_memoryMap.rb_lmDiagnosticsAddress(),
				   m_memoryMap.lmDiagnosticsAddress(),
				   m_memoryMap.lmDiagnosticsSizeW());

		cmd.setComment("copy LM diagnostics data to RegBuf");

		m_code.append(cmd);

		//

		cmd.movMem(m_memoryMap.rb_lmInputsAddress(),
				   m_memoryMap.lmInOutsAddress(),
				   m_memoryMap.lmInOutsSizeW());

		cmd.setComment("copy LM's' input signals to RegBuf");

		m_code.append(cmd);

		//

		cmd.setMem(m_memoryMap.rb_lmOutputsAddress(), m_memoryMap.lmInOutsSizeW(), 0);

		cmd.setComment("init to 0 LM's output signals");

		m_code.append(cmd);
		m_code.newLine();


		return true;
	}

	bool ModuleLogicCompiler::copyLmOutSignalsToModuleMemory()
	{
		m_code.comment("Copy LM's output signals from RegBuf to LM's in/out memory");
		m_code.newLine();

		Command cmd;

		cmd.movMem(	m_memoryMap.lmInOutsAddress(),
					m_memoryMap.rb_lmOutputsAddress(),
					m_memoryMap.lmInOutsSizeW());

		m_code.append(cmd);
		m_code.newLine();

		return true;
	}


	bool ModuleLogicCompiler::copyInModulesAppLogicDataToRegBuf()
	{
		bool firstInputModle = true;

		bool result = true;

		for(Module module : m_modules)
		{
			if (!module.isInputModule())
			{
				continue;
			}

			if (firstInputModle)
			{
				m_code.comment("Copy input modules application logic data to RegBuf");
				m_code.newLine();

				firstInputModle = false;
			}

			switch(module.familyType())
			{
			case Hardware::DeviceModule::FamilyType::DIM:
				result &= copyDimDataToRegBuf(module);
				break;

			case Hardware::DeviceModule::FamilyType::AIM:
				result &= copyAimDataToRegBuf(module);
				break;

			default:
				assert(false);

				LOG_ERROR(m_log, tr("Unknown input module family type"));

				result = false;
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::copyDimDataToRegBuf(const Module& module)
	{
		m_code.comment(QString(tr("Copying DIM data place %2 to RegBuf")).arg(module.place));
		m_code.newLine();

		Command cmd;

		cmd.movMem(module.appLogicRegDataOffset, module.moduleAppDataOffset, module.appLogicDataSize);
		m_code.append(cmd);

		m_code.newLine();

		return true;
	}


	bool ModuleLogicCompiler::copyAimDataToRegBuf(const Module& module)
	{
		if (module.device == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		msg = QString(tr("Copying AIM data place %1 to RegBuf")).arg(module.place);

		if (m_convertUsedInOutAnalogSignalsOnly == true)
		{
			msg += QString(tr(" (validities & used signals only)"));
		}
		else
		{
			msg += QString(tr(" (all signals)"));
		}

		m_code.comment(msg);
		m_code.newLine();

		Command cmd;

		if (m_convertUsedInOutAnalogSignalsOnly == true)
		{
			// initialize module signals memory to 0
			//
			cmd.setMem(module.appLogicRegDataOffset, module.appLogicRegDataSize, 0);
			cmd.setComment(tr("initialize module memory to 0"));
			m_code.append(cmd);
			m_code.newLine();
		}

		bool result = true;

		std::vector<std::shared_ptr<Hardware::DeviceSignal>> moduleSignals = module.device->getAllSignals();

		// sort signals by place ascending
		//

		int moduleSignalsCount = static_cast<int>(moduleSignals.size());

		if (moduleSignalsCount != 128)
		{
			LOG_ERROR(m_log, QString(tr("AIM module must have 128 input signals")));
			return false;
		}

		for(int i = 0; i < moduleSignalsCount - 1; i++)
		{
			for(int j = i + 1; j < moduleSignalsCount; j++)
			{
				if (moduleSignals[i]->place() > moduleSignals[j]->place())
				{
					std::shared_ptr<Hardware::DeviceSignal> tmp = moduleSignals[i];
					moduleSignals[i] = moduleSignals[j];
					moduleSignals[j] = tmp;
				}
			}

		}

		// copy validity words
		//
		const int dataBlockSize = 17;
		const int regDataBlockSize = 33;

		for(int dataBlock = 0; dataBlock < 4; dataBlock++)
		{
			cmd.mov(module.appLogicRegDataOffset + regDataBlockSize * dataBlock,
					module.moduleAppDataOffset + dataBlockSize * dataBlock);
			cmd.setComment(QString(tr("validity of %1 ... %2 inputs")).arg(dataBlock * 16 + 16).arg(dataBlock * 16 + 1));
			m_code.append(cmd);
		}

		m_code.newLine();

		for(std::shared_ptr<Hardware::DeviceSignal>& deviceSignal : moduleSignals)
		{
			if (deviceSignal->isAnalogSignal() == false)
			{
				continue;
			}

			if (!m_deviceBoundSignals.contains(deviceSignal->strId()))
			{
				continue;
			}

			QList<Signal*> boundSignals = m_deviceBoundSignals.values(deviceSignal->strId());

			if (boundSignals.count() > 1)
			{
				LOG_ERROR(m_log, QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
				result = false;
				break;
			}

			bool commandWritten = false;

			for(Signal* signal : boundSignals)
			{
				if (signal == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				if (signal->isDiscrete())
				{
					continue;
				}

				if (signal->dataSize() != SIZE_32BIT)
				{
					LOG_ERROR(m_log, QString(tr("Signal %1 must have 32-bit data size")).arg(signal->strID()));
					RESULT_FALSE_BREAK
				}

				if (m_convertUsedInOutAnalogSignalsOnly == true &&
					m_appSignals.getByStrID(signal->strID()) == nullptr)
				{
					continue;
				}

				if (m_inOutSignalsToScalAppFbMap.contains(signal->strID()) == false)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				AppFb* appFb = m_inOutSignalsToScalAppFbMap[signal->strID()];

				if (appFb == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				FbScal& fbScal = m_fbScal[FB_SCALE_16UI_FP_INDEX];

				if (signal->dataFormat() == E::DataFormat::Float)
				{
					;	// already assigned
				}
				else
				{
					if (signal->dataFormat() == E::DataFormat::SignedInt)
					{
						fbScal = m_fbScal[FB_SCALE_16UI_SI_INDEX];
					}
					else
					{
						assert(false);
					}
				}

				cmd.writeFuncBlock(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
								   module.moduleAppDataOffset + deviceSignal->valueOffset(), appFb->caption());
				cmd.setComment(QString(tr("input %1 %2")).arg(deviceSignal->place()).arg(signal->strID()));
				m_code.append(cmd);

				cmd.start(appFb->opcode(), appFb->instance(), appFb->caption());
				cmd.setComment("");
				m_code.append(cmd);

				cmd.readFuncBlock32(signal->ramAddr().offset(), appFb->opcode(), appFb->instance(),
									fbScal.outputSignalIndex, appFb->caption());
				m_code.append(cmd);

				commandWritten = true;
			}

			if (commandWritten)
			{
				m_code.newLine();
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::initOutModulesAppLogicDataInRegBuf()
	{
		m_code.comment("Init output modules application logic data in RegBuf");
		m_code.newLine();

		for(Module module : m_modules)
		{
			if (!module.isOutputModule())
			{
				continue;
			}

			Command cmd;

			cmd.setMem(module.appLogicRegDataOffset, module.appLogicRegDataSize, 0);

			cmd.setComment(QString(tr("init %1 data (place %2) in RegBuf")).arg(getModuleFamilyTypeStr(module.familyType())).arg(module.place));

			m_code.append(cmd);
		}

		m_code.newLine();

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
				// appItem is signal
				//
				result &= generateAppSignalCode(appItem);
				continue;
			}

			if (appItem->isFb())
			{
				// appItem is FB
				//
				result &= generateFbCode(appItem);
				continue;
			}

			if (appItem->isConst())
			{
				// appItem is Const, no code generation needed
				//
				continue;
			}

			// unknown type of appItem
			//
			assert(false);

			result = false;

			break;
		}

		return result;
	}


	bool ModuleLogicCompiler::generateAppSignalCode(const AppItem* appItem)
	{
		if (!m_appSignals.contains(appItem->guid()))
		{
			LOG_ERROR(m_log, QString(tr("Signal is not found, GUID: %1")).arg(appItem->guid().toString()));
			return false;
		}

		AppSignal* appSignal = m_appSignals[appItem->guid()];

		TEST_PTR_RETURN_FALSE(appSignal)

		bool result = true;

		if (appSignal->isComputed())
		{
			return true;				// signal already computed
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
					LOG_ERROR(m_log, QString(tr("More than one pin is connected to the input")));

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
						LOG_ERROR(m_log, QString(tr("Output pin is not found, GUID: %1")).arg(connectedPinGuid.toString()));

						ASSERT_RESULT_FALSE_BREAK
					}

					srcSignalGuid = m_outPinSignal[connectedPinGuid];
				}

				if (!m_appSignals.contains(srcSignalGuid))
				{
					LOG_ERROR(m_log, QString(tr("Signal is not found, GUID: %1")).arg(srcSignalGuid.toString()));

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
			LOG_ERROR(m_log, QString(tr("Signal value undefined: %1")).arg(appSignal->strID()));
		}

		return result;
	}


	bool ModuleLogicCompiler::generateWriteConstToSignalCode(AppSignal &appSignal, const LogicConst& constItem)
	{
		quint16 ramAddrOffset = appSignal.ramAddr().offset();
		quint16 ramAddrBit = appSignal.ramAddr().bit();

		Command cmd;

		switch(appSignal.type())
		{
		case E::SignalType::Discrete:

			if (!constItem.isIntegral())
			{
				LOG_ERROR(m_log, QString(tr("Floating point constant connected to discrete signal: ")).arg(appSignal.strID()));

				return false;
			}
			else
			{
				quint16 constValue = constItem.intValue() > 0 ? 1 : 0;

				cmd.movBitConst(ramAddrOffset, ramAddrBit, constValue);
				cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
							   arg(appSignal.strID()).arg(appSignal.regAddr().toString()).arg(constValue));
			}
			break;

		case E::SignalType::Analog:
			switch(appSignal.dataSize())
			{
			case SIZE_16BIT:
				cmd.movConst(ramAddrOffset, constItem.intValue());
				cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
							   arg(appSignal.strID()).arg(appSignal.regAddr().toString()).arg(constItem.intValue()));
				break;

			case SIZE_32BIT:
				switch(appSignal.dataFormat())
				{
				case E::DataFormat::SignedInt:
					if (constItem.isIntegral())
					{
						cmd.movConstInt32(ramAddrOffset, constItem.intValue());
						cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
									   arg(appSignal.strID()).arg(appSignal.regAddr().toString()).arg(constItem.intValue()));
					}
					else
					{
						LOG_ERROR(m_log, QString(tr("Constant of type 'Float' (value %1) connected to signal %2 of type 'Signed Int'")).
								  arg(constItem.floatValue()).arg(appSignal.strID()));
					}
					break;

				case E::DataFormat::Float:
					if (constItem.isFloat())
					{
						cmd.movConstFloat(ramAddrOffset, constItem.floatValue());
						cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
									   arg(appSignal.strID()).arg(appSignal.regAddr().toString()).arg(constItem.floatValue()));
					}
					else
					{
						LOG_ERROR(m_log, QString(tr("Constant of type 'Signed Int' (value %1) connected to signal %2 of type 'Float'")).
								  arg(constItem.intValue()).arg(appSignal.strID()));
					}
					break;

				default:
					assert(false);
				}

				break;

			default:
				assert(false);
			}
			break;

		default:
			assert(false);
			return false;
		}

		if (cmd.isValidCommand())
		{
			m_code.newLine();
			m_code.append(cmd);
		}

		appSignal.setComputed();

		return true;
	}


	bool ModuleLogicCompiler::generateWriteSignalToSignalCode(AppSignal& appSignal, const AppSignal& srcSignal)
	{
		if (appSignal.isAnalog())
		{
			if (!srcSignal.isAnalog())
			{
				msg = QString(tr("Discrete signal %1 connected to analog signal %2")).
						arg(srcSignal.strID()).arg(appSignal.strID());

				LOG_ERROR(m_log, msg);

				return false;
			}

			if (appSignal.dataFormat() != srcSignal.dataFormat())
			{
				msg = QString(tr("Signals %1 and %2 data formats are not compatible")).
						arg(appSignal.strID()).arg(srcSignal.strID());

				LOG_ERROR(m_log, msg);

				return false;
			}

			if (appSignal.dataSize() != srcSignal.dataSize())
			{
				msg = QString(tr("Signals %1 and %2 have different data sizes")).
						arg(appSignal.strID()).arg(srcSignal.strID());

				LOG_ERROR(m_log, msg);

				return false;
			}
		}
		else
		{
			if (appSignal.isDiscrete())
			{
				if (!srcSignal.isDiscrete())
				{
					msg = QString(tr("Analog signal %1 connected to discrete signal %2")).
							arg(srcSignal.strID()).arg(appSignal.strID());

					LOG_ERROR(m_log, msg);

					return false;
				}
			}
			else
			{
				assert(false);		// unknown afb signal type
				return false;
			}
		}

		if (appSignal.dataFormat() != srcSignal.dataFormat())
		{
			LOG_ERROR(m_log, QString(tr("Signals %1 and  %2 is not compatible by dataFormat")).
							  arg(srcSignal.strID()).arg(appSignal.strID()));

			return false;
		}

		if (appSignal.dataSize() != srcSignal.dataSize())
		{
			LOG_ERROR(m_log, QString(tr("Signals %1 and  %2 is not compatible by dataSize")).
							  arg(srcSignal.strID()).arg(appSignal.strID()));

			return false;
		}

		Command cmd;

		int srcRamAddrOffset = srcSignal.ramAddr().offset();
		int srcRamAddrBit = srcSignal.ramAddr().bit();

		int destRamAddrOffset = appSignal.ramAddr().offset();
		int destRamAddrBit = appSignal.ramAddr().bit();

		if (srcRamAddrOffset == -1 || srcRamAddrBit == -1)
		{
			LOG_ERROR(m_log, QString(tr("Signal %1 RAM addreess is not calculated")).
							  arg(srcSignal.strID()));
			return false;
		}

		if (destRamAddrOffset == -1 || destRamAddrBit == -1)
		{
			LOG_ERROR(m_log, QString(tr("Signal %1 RAM addreess is not calculated")).
							  arg(appSignal.strID()));
			return false;
		}

		if (appSignal.isAnalog())
		{
			// move value of analog signal
			//
			switch(appSignal.dataSize())
			{
			case SIZE_16BIT:
				cmd.mov(destRamAddrOffset, srcRamAddrOffset);
				break;

			case SIZE_32BIT:
				cmd.mov32(destRamAddrOffset, srcRamAddrOffset);
				break;

			default:
				LOG_ERROR(m_log, QString(tr("Unknown data size of signal %1 - %2 bit")).
								  arg(appSignal.strID()).arg(appSignal.dataSize()));
				return false;
			}
		}
		else
		{
			// move value of discrete signal
			//
			cmd.moveBit(destRamAddrOffset, destRamAddrBit, srcRamAddrOffset, srcRamAddrBit);
		}

		cmd.setComment(QString(tr("%1 (reg %2) <= %3 (reg %4)")).
					   arg(appSignal.strID()).arg(appSignal.regAddr().toString()).
					   arg(srcSignal.strID()).arg(srcSignal.regAddr().toString()));
		m_code.append(cmd);
		m_code.newLine();

		appSignal.setComputed();

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
			if (!writeFbInputSignals(appFb)) break;

			if (!startFb(appFb)) break;

			if (!readFbOutputSignals(appFb)) break;

			result = true;
		}
		while(false);

		m_code.newLine();

		return result;
	}


	bool ModuleLogicCompiler::startFb(const AppFb* appFb)
	{
		int startCount = 1;

		for(LogicAfbParam param : appFb->afb().params())
		{
			if (param.opName() == PARAM_TEST_START_COUNT)
			{
				startCount = param.value().toInt();
				break;
			}
		}
		Command cmd;

		if (startCount == 1)
		{
			cmd.start(appFb->opcode(), appFb->instance(), appFb->caption());
			cmd.setComment(QString(tr("compute %1")).arg(appFb->caption()));
		}
		else
		{
			cmd.nstart(appFb->opcode(), appFb->instance(), startCount, appFb->caption());
			cmd.setComment(QString(tr("compute %1 %2 times")).arg(appFb->afbStrID()).arg(startCount));
		}

		m_code.append(cmd);

		return true;
	}


	bool ModuleLogicCompiler::writeFbInputSignals(const AppFb* appFb)
	{
		bool result = true;

		for(LogicPin inPin : appFb->inputs())
		{
			if (inPin.dirrection() != VFrame30::ConnectionDirrection::Input)
			{
				LOG_ERROR(m_log, QString(tr("Input pin %1 of %2 has wrong direction")).arg(inPin.caption()).arg(appFb->strID()));
				RESULT_FALSE_BREAK
			}

			int connectedPinsCount = 1;

			for(QUuid connectedPinGuid : inPin.associatedIOs())
			{
				if (connectedPinsCount > 1)
				{
					LOG_ERROR(m_log, QString(tr("More than one pin is connected to the input")));

					RESULT_FALSE_BREAK
				}

				connectedPinsCount++;

				if (!m_pinParent.contains(connectedPinGuid))
				{
					LOG_ERROR(m_log, QString(tr("Pin is not found, GUID %1")).arg(connectedPinGuid.toString()));

					RESULT_FALSE_BREAK
				}

				AppItem* connectedPinParent = m_pinParent[connectedPinGuid];

				if (connectedPinParent == nullptr)
				{
					LOG_ERROR(m_log, QString(tr("Pin parent is NULL, pin GUID ")).arg(connectedPinGuid.toString()));
					RESULT_FALSE_BREAK
				}

				if (connectedPinParent->isConst())
				{
					result &= generateWriteConstToFbCode(*appFb, inPin, connectedPinParent->logicConst());
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
						LOG_ERROR(m_log, QString(tr("Output pin is not found, GUID: %1")).arg(connectedPinGuid.toString()));

						RESULT_FALSE_BREAK
					}

					signalGuid = m_outPinSignal[connectedPinGuid];
				}

				if (!m_appSignals.contains(signalGuid))
				{
					LOG_ERROR(m_log, QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()));

					RESULT_FALSE_BREAK
				}

				AppSignal* appSignal = m_appSignals[signalGuid];

				if (appSignal == nullptr)
				{
					LOG_ERROR(m_log, QString(tr("Signal pointer is NULL, signal GUID: %1")).arg(signalGuid.toString()));

					RESULT_FALSE_BREAK
				}

				if (!appSignal->isComputed())
				{
					LOG_ERROR(m_log, QString(tr("Signal value undefined: %1")).arg(appSignal->strID()));

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
		case Afb::AfbSignalType::Discrete:
			// input connected to discrete input
			//
			if (!constItem.isIntegral())
			{
				LOG_ERROR(m_log, QString(tr("Floating point constant connected to discrete input")));
			}
			else
			{
				quint16 constValue = constItem.intValue() > 0 ? 1 : 0;

				cmd.writeFuncBlockConst(fbType, fbInstance, fbParamNo, constValue, appFb.caption());
				cmd.setComment(QString(tr("%1 <= %2")).arg(inPin.caption()).arg(constValue));
			}
			break;

		case Afb::AfbSignalType::Analog:
			// const connected to analog input
			//

			switch(fbInput.size())
			{
			case SIZE_16BIT:
				cmd.writeFuncBlockConst(fbType, fbInstance, fbParamNo, constItem.intValue(), appFb.caption());
				cmd.setComment(QString(tr("%1 <= %2")).arg(inPin.caption()).arg(constItem.intValue()));
				break;

			case SIZE_32BIT:
				switch(fbInput.dataFormat())
				{
				case Afb::AfbDataFormat::SignedInt:
					cmd.writeFuncBlockConstInt32(fbType, fbInstance, fbParamNo, constItem.intValue(), appFb.caption());
					cmd.setComment(QString(tr("%1 <= %2")).arg(fbInput.opName()).arg(constItem.intValue()));
					break;

				case Afb::AfbDataFormat::Float:
					cmd.writeFuncBlockConstFloat(fbType, fbInstance, fbParamNo, constItem.floatValue(), appFb.caption());
					cmd.setComment(QString(tr("%1 <= %2")).arg(fbInput.opName()).arg(constItem.floatValue()));
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


	bool ModuleLogicCompiler::generateWriteSignalToFbCode(const AppFb& appFb, const LogicPin& inPin, const AppSignal& appSignal)
	{
		quint16 fbType = appFb.opcode();
		quint16 fbInstance = appFb.instance();
		quint16 fbParamNo = inPin.afbOperandIndex();

		LogicAfbSignal afbSignal = appFb.getAfbSignalByIndex(fbParamNo);

		if (afbSignal.isAnalog())
		{
			if (!appSignal.isAnalog())
			{
				msg = QString(tr("Discrete signal %1 connected to analog input '%2' of %3")).
						arg(appSignal.strID()).arg(afbSignal.caption()).arg(appFb.caption());

				LOG_ERROR(m_log, msg);

				return false;
			}

			if (appSignal.isCompatibleDataFormat(afbSignal.dataFormat()) == false)
			{
				msg = QString(tr("Signal %1 data format is not compatible with input '%2' data format of %3 ")).
						arg(appSignal.strID()).arg(afbSignal.caption()).arg(appFb.caption());

				LOG_ERROR(m_log, msg);

				return false;
			}

			if (appSignal.dataSize() != afbSignal.size())
			{
				msg = QString(tr("Signal %1 data size (%2) is not compatible with input '%3' data size (%4) of %4")).
						arg(appSignal.strID()).arg(appSignal.dataSize()).
						arg(afbSignal.caption()).arg(afbSignal.size()).arg(appFb.caption());

				LOG_ERROR(m_log, msg);

				return false;
			}
		}
		else
		{
			if (afbSignal.isDiscrete())
			{
				if (!appSignal.isDiscrete())
				{
					msg = QString(tr("Analog signal %1 connected to discrete input '%2' of %3")).
							arg(appSignal.strID()).arg(afbSignal.caption()).arg(appFb.caption());

					LOG_ERROR(m_log, msg);

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

		int ramAddrOffset = appSignal.ramAddr().offset();
		int ramAddrBit = appSignal.ramAddr().bit();

		if (ramAddrOffset == -1 || ramAddrBit == -1)
		{
			assert(false);		// signal ramAddr is not calculated!!!
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
						   arg(inPin.caption()).arg(appSignal.strID()).arg(appSignal.regAddr().toString()));

			m_code.append(cmd);
		}

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

				result &= generateReadFuncBlockToSignalCode(*appFb, outPin, signalGuid);
			}

			if (connectedSignals == 0)
			{
				// output pin is not connected to signal
				// save FB output value to shadow signal with GUID == outPin.guid()
				//
				result &= generateReadFuncBlockToSignalCode(*appFb, outPin, outPin.guid());
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
			LOG_ERROR(m_log, QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()));
			return false;
		}

		AppSignal* appSignal = m_appSignals[signalGuid];

		if (appSignal == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		quint16 fbType = appFb.opcode();
		quint16 fbInstance = appFb.instance();
		quint16 fbParamNo = outPin.afbOperandIndex();

		LogicAfbSignal afbSignal = appFb.getAfbSignalByIndex(fbParamNo);

		if (afbSignal.isAnalog())
		{
			if (!appSignal->isAnalog())
			{
				msg = QString(tr("Analog output %1.%2 connected to discrete signal %3")).
						arg(appFb.strID()).arg(afbSignal.caption()).arg(appSignal->strID());

				LOG_ERROR(m_log, msg);

				return false;
			}

			if (appSignal->isCompatibleDataFormat(afbSignal.dataFormat()) == false)
			{
				msg = QString(tr("Signal %1 data format is not compatible with output '%2' data format of %3 ")).
						arg(appSignal->strID()).arg(afbSignal.caption()).arg(appFb.caption());

				LOG_ERROR(m_log, msg);

				return false;
			}

			if (appSignal->dataSize() != afbSignal.size())
			{
				msg = QString(tr("Signal %1 data size (%2) is not compatible with output '%3' data size (%4) of %4")).
						arg(appSignal->strID()).arg(appSignal->dataSize()).
						arg(afbSignal.caption()).arg(afbSignal.size()).arg(appFb.caption());

				LOG_ERROR(m_log, msg);

				return false;
			}
		}
		else
		{
			if (afbSignal.isDiscrete())
			{
				if (!appSignal->isDiscrete())
				{
					msg = QString(tr("Discrete output %1.%2 connected to analog signal %3")).
							arg(appFb.strID()).arg(afbSignal.caption()).arg(appSignal->strID());

					LOG_ERROR(m_log, msg);

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

		int ramAddrOffset = appSignal->ramAddr().offset();
		int ramAddrBit = appSignal->ramAddr().bit();

		if (ramAddrOffset == -1 || ramAddrBit == -1)
		{
			LOG_ERROR(m_log, QString(tr("RAM-address of signal %1 is not calculated")).arg(appSignal->strID()));
			return false;
		}
		else
		{
			if (appSignal->isAnalog())
			{
				// output connected to analog signal
				//
				switch(appSignal->dataSize())
				{
				case SIZE_16BIT:
					cmd.readFuncBlock(ramAddrOffset, fbType, fbInstance, fbParamNo, appFb.caption());
					break;

				case SIZE_32BIT:
					cmd.readFuncBlock32(ramAddrOffset, fbType, fbInstance, fbParamNo, appFb.caption());
					break;

				default:
					assert(false);
					return false;
				}
			}
			else
			{
				// output connected to discrete signal
				//
				cmd.readFuncBlockBit(ramAddrOffset, ramAddrBit, fbType, fbInstance, fbParamNo, appFb.caption());
			}

			cmd.setComment(QString(tr("%1 (reg %2) <= %3")).
						   arg(appSignal->strID()).arg(appSignal->regAddr().toString()).arg(outPin.caption()));

			m_code.append(cmd);
		}

		appSignal->setComputed();

		return true;
	}


	bool ModuleLogicCompiler::copyDiscreteSignalsToRegBuf()
	{
		if (m_memoryMap.regDiscreteSignalsSizeW() == 0)
		{
			return true;
		}

		m_code.comment("Copy internal discrete signals from bit-addressed memory to RegBuf");
		m_code.newLine();

		Command cmd;

		cmd.movMem(m_memoryMap.rb_regDiscreteSignalsAddress(),
				   m_memoryMap.regDiscreteSignalsAddress(),
				   m_memoryMap.regDiscreteSignalsSizeW());

		m_code.append(cmd);

		m_code.newLine();

		return true;
	}


	bool ModuleLogicCompiler::copyOutModulesAppLogicDataToModulesMemory()
	{
		bool firstOutputModule = true;

		bool result = true;

		for(Module module : m_modules)
		{
			if (!module.isOutputModule())
			{
				continue;
			}

			if (firstOutputModule)
			{
				m_code.comment("Copy output modules application logic data to modules memory");
				m_code.newLine();

				firstOutputModule = false;
			}

			switch(module.familyType())
			{
			case Hardware::DeviceModule::FamilyType::AOM:
				result &= copyAomDataToModuleMemory(module);
				break;

			case Hardware::DeviceModule::FamilyType::DOM:
				result &= copyDomDataToModuleMemory(module);
				break;

			default:
				// unknown output module family type
				//
				assert(false);

				LOG_ERROR(m_log, tr("Unknown output module family type"));

				result = false;
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::copyDomDataToModuleMemory(const Module& module)
	{
		m_code.comment(QString(tr("Copying DOM data place %1 to modules memory")).arg(module.place));
		m_code.newLine();

		Command cmd;

		assert(module.appLogicDataSize == module.appLogicRegDataSize);

		cmd.movMem(module.moduleAppDataOffset, module.appLogicRegDataOffset, module.appLogicDataSize);
		m_code.append(cmd);

		if (module.appLogicDataSize < module.appLogicDataSizeWithReserve)
		{
			cmd.setMem(module.moduleAppDataOffset + module.appLogicDataSize, module.appLogicDataSizeWithReserve - module.appLogicDataSize, 0);
			cmd.setComment(QString(tr("set reserv data to 0")));

			m_code.append(cmd);
		}

		m_code.newLine();

		return true;
	}


	bool ModuleLogicCompiler::copyAomDataToModuleMemory(const Module& module)
	{
		if (module.device == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		msg = QString(tr("Copying AOM data place %1 to modules memory")).arg(module.place);

		if (m_convertUsedInOutAnalogSignalsOnly == true)
		{
			msg += QString(tr(" (used signals only)"));
		}
		else
		{
			msg += QString(tr(" (all signals)"));
		}

		m_code.comment(msg);
		m_code.newLine();

		Command cmd;

		if (m_convertUsedInOutAnalogSignalsOnly == true)
		{
			cmd.setMem(module.moduleAppDataOffset, module.appLogicDataSize, 0);
			m_code.append(cmd);
			m_code.newLine();
		}

		bool result = true;

		std::vector<std::shared_ptr<Hardware::DeviceSignal>> moduleSignals = module.device->getAllSignals();

		// sort signals by place ascending
		//

		int moduleSignalsCount = static_cast<int>(moduleSignals.size());

		if (moduleSignalsCount != 32)
		{
			LOG_ERROR(m_log, QString(tr("AOM module must have 32 output signals")));
			return false;
		}

		for(int i = 0; i < moduleSignalsCount - 1; i++)
		{
			for(int j = i + 1; j < moduleSignalsCount; j++)
			{
				if (moduleSignals[i]->place() > moduleSignals[j]->place())
				{
					std::shared_ptr<Hardware::DeviceSignal> tmp = moduleSignals[i];
					moduleSignals[i] = moduleSignals[j];
					moduleSignals[j] = tmp;
				}
			}

		}

		for(std::shared_ptr<Hardware::DeviceSignal>& deviceSignal : moduleSignals)
		{
			if (deviceSignal->isAnalogSignal() == false)
			{
				continue;
			}

			if (!m_deviceBoundSignals.contains(deviceSignal->strId()))
			{
				continue;
			}

			QList<Signal*> boundSignals = m_deviceBoundSignals.values(deviceSignal->strId());

			if (boundSignals.count() > 1)
			{
				LOG_ERROR(m_log, QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
				result = false;
				break;
			}

			bool codeWritten = false;

			for(Signal* signal : boundSignals)
			{
				if (signal == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				if (signal->isDiscrete())
				{
					continue;
				}

				if (signal->dataSize() != SIZE_32BIT)
				{
					LOG_ERROR(m_log, QString(tr("Signal %1 must have 32-bit data size")).arg(signal->strID()));
					RESULT_FALSE_BREAK
				}

				if (m_convertUsedInOutAnalogSignalsOnly == true &&
					m_appSignals.getByStrID(signal->strID()) == nullptr)
				{
					continue;
				}

				if (m_inOutSignalsToScalAppFbMap.contains(signal->strID()) == false)
				{
					ASSERT_RESULT_FALSE_BREAK
				}

				AppFb* appFb = m_inOutSignalsToScalAppFbMap[signal->strID()];

				if (appFb == nullptr)
				{
					ASSERT_RESULT_FALSE_BREAK
				}
				FbScal& fbScal = m_fbScal[FB_SCALE_FP_16UI_INDEX];

				if (signal->dataFormat() == E::DataFormat::Float)
				{
					;	// already assigned
				}
				else
				{
					if (signal->dataFormat() == E::DataFormat::SignedInt)
					{
						fbScal = m_fbScal[FB_SCALE_SI_16UI_INDEX];
					}
					else
					{
						assert(false);
					}
				}

				cmd.writeFuncBlock32(appFb->opcode(), appFb->instance(), fbScal.inputSignalIndex,
								   signal->ramAddr().offset(), appFb->caption());
				cmd.setComment(QString(tr("output %1 %2")).arg(deviceSignal->place()).arg(signal->strID()));
				m_code.append(cmd);

				cmd.start(appFb->opcode(), appFb->instance(), appFb->caption());
				cmd.setComment("");
				m_code.append(cmd);

				cmd.readFuncBlock(module.moduleAppDataOffset + deviceSignal->valueOffset(), appFb->opcode(), appFb->instance(),
								fbScal.outputSignalIndex, appFb->caption());
				m_code.append(cmd);

				codeWritten = true;
			}

			if (codeWritten == true)
			{
				m_code.newLine();
			}
		}

		if (module.appLogicDataSize < module.appLogicDataSizeWithReserve)
		{
			cmd.setMem(module.moduleAppDataOffset + module.appLogicDataSize, module.appLogicDataSizeWithReserve - module.appLogicDataSize, 0);
			cmd.setComment(QString(tr("set reserv data to 0")));

			m_code.append(cmd);
			m_code.newLine();
		}

		return result;
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


	bool ModuleLogicCompiler::writeResult()
	{
		bool result = true;

		m_code.generateBinCode();

		QByteArray binCode;

		m_code.getBinCode(binCode);

		m_appLogicCompiler.writeBinCodeForLm(m_lm->subSysID(), m_lm->caption(), m_lm->channel(),
														  m_lmAppLogicFrameSize, m_lmAppLogicFrameCount, binCode);
		QStringList mifCode;

		m_code.getMifCode(mifCode);

		result &= m_resultWriter->addFile(m_lm->subSysID(), QString("%1.mif").arg(m_lm->caption()), mifCode);

		QStringList asmCode;

		m_code.getAsmCode(asmCode);

		result = m_resultWriter->addFile(m_lm->subSysID(), QString("%1.asm").arg(m_lm->caption()), asmCode);

		QStringList memFile;

		m_memoryMap.getFile(memFile);

		result = m_resultWriter->addFile(m_lm->subSysID(), QString("%1.mem").arg(m_lm->caption()), memFile);

		//

		writeLMCodeTestFile();

		//

		return result;
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

		const char* const FB_SCALE_X1_OPNAME = "input_low";
		const char* const FB_SCALE_X2_OPNAME = "input_high";
		const char* const FB_SCALE_Y1_OPNAME = "output_low";
		const char* const FB_SCALE_Y2_OPNAME = "output_high";

		const char* const FB_SCALE_INPUT_SIGNAL_CAPTION = "i_data";
		const char* const FB_SCALE_OUTPUT_SIGNAL_CAPTION = "o_result";

		for(const char* const fbCaption : fbScalCaption)
		{
			bool fbFound = false;

			for(std::shared_ptr<Afb::AfbElement> afbElement : m_afbl->elements())
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
					LOG_ERROR(m_log, QString(tr("Required parameter 'InputLow' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (fb.x2ParamIndex == -1)
				{
					LOG_ERROR(m_log, QString(tr("Required parameter 'InputHigh' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (fb.y1ParamIndex == -1)
				{
					LOG_ERROR(m_log, QString(tr("Required parameter 'OutputLow' of AFB %1 is not found")).arg(fb.caption))
					result = false;
				}

				if (fb.y2ParamIndex == -1)
				{
					LOG_ERROR(m_log, QString(tr("Required parameter 'OutputHigh' of AFB %1 is not found")).arg(fb.caption))
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
					LOG_ERROR(m_log, QString(tr("Required input signal %1 of AFB %2 is not found")).
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
					LOG_ERROR(m_log, QString(tr("Required output signal %1 of AFB %2 is not found")).
							  arg(FB_SCALE_OUTPUT_SIGNAL_CAPTION).arg(fb.caption))
					result = false;
					break;
				}

				m_fbScal.append(fb);
			}

			if (fbFound == false)
			{
				LOG_ERROR(m_log, QString(tr("Required AFB %1 is not found")).arg(fbCaption));
				result = false;
				break;
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::appendFbsForAnalogInOutSignalsConversion()
	{
		LOG_MESSAGE(m_log, QString(tr("Prepare FBs for input/output signals conversion...")));

		if (findFbsForAnalogInOutSignalsConversion() == false)
		{
			return false;
		}

		bool result = true;

		for(const Module& module : m_modules)
		{
			if (module.device == nullptr)
			{
				assert(false);
				return false;
			}

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
					LOG_WARNING(m_log, QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
				}

				for(Signal* signal : boundSignals)
				{
					if (signal == nullptr)
					{
						assert(false);
						continue;
					}

					if (signal->isDiscrete())
					{
						continue;
					}

					AppItem* appItem = nullptr;

					if (signal->isInput())
					{
						appItem = createFbForAnalogInputSignalConversion(*signal);
					}
					else
					{
						if (signal->isOutput())
						{
							appItem = createFbForAnalogOutputSignalConversion(*signal);
						}
						else
						{
							assert(false);
						}
					}

					if (appItem != nullptr)
					{
						AppFb* appFb = createAppFb(*appItem);

						m_inOutSignalsToScalAppFbMap.insert(signal->strID(), appFb);

						qDebug() << signal->strID();

						delete appItem;
					}
					else
					{
						assert(false);
					}
				}
			}
		}

		return result;
	}


	AppItem* ModuleLogicCompiler::createFbForAnalogInputSignalConversion(const Signal& signal)
	{
		assert(signal.isAnalog());
		assert(signal.isInput());
		assert(signal.deviceStrID().isEmpty() == false);

		int x1 = signal.lowADC();
		int x2 = signal.highADC();

		if (x2 - x1 == 0)
		{
			LOG_ERROR(m_log, QString(tr("Low and High ADC values of signal %1 are equal (= %2)")).arg(signal.strID()).arg(x1));
			return nullptr;
		}

		double y1 = signal.lowLimit();
		double y2 = signal.highLimit();

		AppItem* appItem = nullptr;

		switch(signal.dataFormat())
		{
		case E::DataFormat::Float:
			{
				FbScal fb = m_fbScal[FB_SCALE_16UI_FP_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1));
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2));

				appItem = new AppItem(fb.pointer);
			}

			break;

		case E::DataFormat::SignedInt:
			{
				FbScal& fb = m_fbScal[FB_SCALE_16UI_SI_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

				appItem = new AppItem(fb.pointer);
			}

			break;

		default:
			LOG_ERROR(m_log, QString(tr("Unknown conversion for signal %1, dataFormat %2")).
					  arg(signal.strID()).arg(static_cast<int>(signal.dataFormat())));
		}

		return appItem;
	}


	AppItem* ModuleLogicCompiler::createFbForAnalogOutputSignalConversion(const Signal& signal)
	{
		assert(signal.isAnalog());
		assert(signal.isOutput());
		assert(signal.deviceStrID().isEmpty() == false);

		double x1 = signal.lowLimit();
		double x2 = signal.highLimit();

		if (x2 - x1 == 0.0)
		{
			LOG_ERROR(m_log, QString(tr("Low and High Limit values of signal %1 are equal (= %2)")).arg(signal.strID()).arg(x1));
			return nullptr;
		}

		int y1 = signal.lowADC();
		int y2 = signal.highADC();

		AppItem* appItem = nullptr;

		switch(signal.dataFormat())
		{
		case E::DataFormat::Float:
			{
				FbScal& fb = m_fbScal[FB_SCALE_FP_16UI_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1));
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2));

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

				appItem = new AppItem(fb.pointer);
			}

			break;

		case E::DataFormat::SignedInt:
			{
				FbScal& fb = m_fbScal[FB_SCALE_SI_16UI_INDEX];

				fb.pointer->params()[fb.x1ParamIndex].setValue(QVariant(x1).toInt());
				fb.pointer->params()[fb.x2ParamIndex].setValue(QVariant(x2).toInt());

				fb.pointer->params()[fb.y1ParamIndex].setValue(QVariant(y1).toInt());
				fb.pointer->params()[fb.y2ParamIndex].setValue(QVariant(y2).toInt());

				appItem = new AppItem(fb.pointer);
			}

			break;

		default:
			LOG_ERROR(m_log, QString(tr("Unknown conversion for signal %1, dataFormat %2")).
					  arg(signal.strID()).arg(static_cast<int>(signal.dataFormat())));
		}

		return appItem;
	}


	bool ModuleLogicCompiler::buildServiceMaps()
	{
		m_afbs.clear();

		for(std::shared_ptr<Afb::AfbElement> afbl : m_afbl->elements())
		{
			m_afbs.insert(afbl);
		}

		m_appItems.clear();
		m_pinParent.clear();

		bool result = true;

		if (m_moduleLogic != nullptr)
		{
			for(const AppLogicItem& logicItem : m_moduleLogic->items())
			{
				// build QHash<QUuid, AppItem*> m_appItems
				// item GUID -> item ptr
				//
				if (m_appItems.contains(logicItem.m_fblItem->guid()))
				{
					AppItem* firstItem = m_appItems[logicItem.m_fblItem->guid()];

					msg = QString(tr("Duplicate GUID %1 of %2 and %3 elements")).
							arg(logicItem.m_fblItem->guid().toString()).arg(firstItem->strID()).arg(getAppLogicItemStrID(logicItem));

					LOG_ERROR(m_log, msg);

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

						LOG_ERROR(m_log, msg);

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

						LOG_ERROR(m_log, msg);

						result = false;

						continue;
					}

					m_pinParent.insert(output.guid(), appItem);
				}
			}
		}

		return result;
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
				assert(false);
				return false;
			}
		}

		return true;
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
			return nullptr;
		}

		// get Functional Block instance
		//
		bool result = m_afbs.addInstance(appFb);

		if (result == false)
		{
			delete appFb;
			return nullptr;
		}

		m_appFbs.insert(appFb);

		return appFb;
	}


	bool ModuleLogicCompiler::createDeviceBoundSignalsMap()
	{
		int count = m_signals->count();

		for(int i = 0; i < count; i++)
		{
			Signal* s = &(*m_signals)[i];

			if (m_signalsStrID.contains(s->strID()))
			{
				msg = QString(tr("Duplicate signal identifier: %1")).arg(s->strID());
				LOG_WARNING(m_log, msg);
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
			if (!item->isSignal())
			{
				continue;
			}

			result &= m_appSignals.insert(item);
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
					if (m_appFbs.contains(item->guid()))
					{
						const AppFb* appFb = m_appFbs[item->guid()];

						if (appFb != nullptr)
						{
							result &= m_appSignals.insert(appFb, output);
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

		return result;
	}


	bool ModuleLogicCompiler::calculateInOutSignalsAddresses()
	{
		LOG_MESSAGE(m_log, QString(tr("Input & Output signals addresses calculation...")));

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
					LOG_WARNING(m_log, QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
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
						if (signalOffset >= module.appLogicDataSize)
						{
							LOG_ERROR(m_log, QString(tr("Signal %1 offset out of module application data size")).arg(signal->strID()));

							result = false;
						}
						else
						{
							switch(module.familyType())
							{
							case Hardware::DeviceModule::FamilyType::AIM:
								{
									int signalGroup = signalOffset / 17;
									int signalNo = signalOffset % 17;

									if (signalNo == 0)
									{
										// this is discrete validity signal
										//
										signalOffset = signalGroup * 33;
									}
									else
									{
										// this is analog input signal
										//
										signalOffset = signalGroup * 33 + 1 + 2 * (signalNo - 1);
									}
								}
								break;

							case Hardware::DeviceModule::FamilyType::AOM:
								signalOffset *= 2;
								break;

							case Hardware::DeviceModule::FamilyType::DIM:
							case Hardware::DeviceModule::FamilyType::DOM:
								break;

							default:
								assert(false);
								break;
							}

							// !!! signal - pointer to Signal objects in build-time SignalSet (ModuleLogicCompiler::m_signals member) !!!
							//
							Address16 ramAddr(module.appLogicRegDataOffset + signalOffset, bit);
							Address16 regAddr(ramAddr.offset() - m_memoryMap.wordAddressedMemoryAddress(), bit);

							signal->ramAddr() = ramAddr;
							signal->regAddr() = regAddr;

							// set same ramAddr & regAddr for corresponding signals in m_appSignals map
							//
							AppSignal* appSignal = m_appSignals.getByStrID(signal->strID());

							if (appSignal != nullptr)
							{
								// not all device-bound signals must be in m_appSignals map
								//
								appSignal->ramAddr() = ramAddr;
								appSignal->regAddr() = regAddr;
							}
						}
					}
					else
					{
						LOG_ERROR(m_log, QString(tr("Can't calculate RAM address of application signal %1")).arg(signal->strID()));

						result = false;
					}
				}
			}
		}

		return result;
	}


	bool ModuleLogicCompiler::calculateInternalSignalsAddresses()
	{
		LOG_MESSAGE(m_log, QString(tr("Internal signals addresses calculation...")));

		bool result = true;

		// internal analog registered
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			if (appSignal == nullptr)
			{
				ASSERT_RESULT_FALSE_BREAK
			}

			if (appSignal->isInternal() && appSignal->isRegistered() && appSignal->isAnalog())
			{
				Address16 ramAddr = m_memoryMap.addRegAnalogSignal(*appSignal);

				appSignal->ramAddr() = ramAddr;
				appSignal->regAddr() = Address16(ramAddr.offset() - m_memoryMap.wordAddressedMemoryAddress(), 0);
			}
		}

		m_memoryMap.recalculateAddresses();

		// internal discrete registered
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			if (appSignal->isInternal() && appSignal->isRegistered() && appSignal->isDiscrete())
			{
				Address16 ramAddr = m_memoryMap.addRegDiscreteSignal(*appSignal);

				appSignal->ramAddr() = ramAddr;

				Address16 regAddr = m_memoryMap.addRegDiscreteSignalToRegBuffer(*appSignal);

				appSignal->regAddr() = Address16(regAddr.offset() - m_memoryMap.wordAddressedMemoryAddress(), ramAddr.bit());
			}
		}

		m_memoryMap.recalculateAddresses();

		// internal analog non-registered
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			if (appSignal->isInternal() && !appSignal->isRegistered() && appSignal->isAnalog())
			{
				Address16 ramAddr = m_memoryMap.addNonRegAnalogSignal(*appSignal);

				appSignal->ramAddr() = ramAddr;
			}
		}

		m_memoryMap.recalculateAddresses();

		// internal discrete non-registered
		//
		for(AppSignal* appSignal : m_appSignals)
		{
			if (appSignal->isInternal() && !appSignal->isRegistered() && appSignal->isDiscrete())
			{
				Address16 ramAddr = m_memoryMap.addNonRegDiscreteSignal(*appSignal);

				appSignal->ramAddr() = ramAddr;
			}
		}

		m_memoryMap.recalculateAddresses();

		return result;
	}


	bool ModuleLogicCompiler::getLMIntProperty(const QString& name, int *value)
	{
		return getDeviceIntProperty(m_lm, name, value);
	}


	bool ModuleLogicCompiler::getDeviceIntProperty(Hardware::DeviceObject* device, const QString& name, int *value)
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

		QVariant val = device->propertyValue(name);

		if (val.isValid() == false)
		{
			LOG_ERROR(m_log, QString(tr("Property %1 is not found in device %2")).arg(name).arg(device->strId()));
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

		for(AppItem* scalAppItem : m_scalAppItems)
		{
			delete scalAppItem;
		}

		m_scalAppItems.clear();
	}


	// ---------------------------------------------------------------------------------------
	//
	// Fbl class implementation
	//

	LogicAfb::LogicAfb(std::shared_ptr<Afb::AfbElement> afb) :
		m_afb(afb)
	{
		if (m_afb == nullptr)
		{
			assert(false);
			return;
		}
	}

	LogicAfb::~LogicAfb()
	{
	}


	// ---------------------------------------------------------------------------------------
	//
	// FblsMap class implementation
	//

	void AfbMap::insert(std::shared_ptr<Afb::AfbElement> logicAfb)
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

		LogicAfb* afb = new LogicAfb(logicAfb);

		HashedVector<QString, LogicAfb*>::insert(afb->strID(), afb);

		// initialize map Fbl opCode -> current instance
		//
		if (!m_fblInstance.contains(logicAfb->type().toOpCode()))
		{
			m_fblInstance.insert(logicAfb->type().toOpCode(), 0);
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
			if (param.operandIndex() == FOR_USER_ONLY_PARAM_INDEX)
			{
				continue;
			}

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


	bool AfbMap::addInstance(AppFb* appFb)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return false;
		}

		QString afbStrID = appFb->strID();

		if (!contains(afbStrID))
		{
			assert(false);			// unknown FBL strID
			return false;
		}

		LogicAfb* fbl = (*this)[afbStrID];

		if (fbl == nullptr)
		{
			assert(false);
			return 0;
		}

		int instance = 0;

		QString instantiatorID = appFb->instantiatorID();

		if (fbl->hasRam())
		{
			Afb::AfbType afbType = fbl->type();

			if (m_fblInstance.contains(afbType.toOpCode()))
			{
				instance = m_fblInstance[afbType.toOpCode()];

				instance++;

				m_fblInstance[afbType.toOpCode()] = instance;

				m_nonRamFblInstance.insert(instantiatorID, instance);
			}
			else
			{
				assert(false);		// unknown opcode
			}
		}
		else
		{
			// Calculate non-RAM Fbl instance
			//
			if (m_nonRamFblInstance.contains(instantiatorID))
			{
				instance = m_nonRamFblInstance.value(instantiatorID);
			}
			else
			{
				Afb::AfbType afbType = fbl->type();

				if (m_fblInstance.contains(afbType.toOpCode()))
				{
					instance = m_fblInstance[afbType.toOpCode()];

					instance++;

					m_fblInstance[afbType.toOpCode()] = instance;

					m_nonRamFblInstance.insert(instantiatorID, instance);
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
			return false;
		}

		if (instance > MAX_FB_INSTANCE)
		{
			assert(false);				// reached the max instance number
			return false;
		}

		appFb->setInstance(instance);

		return true;
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
		for(LogicAfb* fbl : *this)
		{
			delete fbl;
		}

		HashedVector<QString, LogicAfb*>::clear();
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppItem class implementation
	//

	AppItem::AppItem(const Builder::AppLogicItem &appLogicItem) :
		m_appLogicItem(appLogicItem)
	{
	}


	AppItem::AppItem(const AppItem& appItem) :
		QObject()
	{
		m_appLogicItem = appItem.m_appLogicItem;
	}


	AppItem::AppItem(std::shared_ptr<Afb::AfbElement> afbElement)
	{
		m_appLogicItem.m_afbElement = *afbElement.get();
		m_appLogicItem.m_fblItem = std::shared_ptr<VFrame30::FblItemRect>(
					new VFrame30::SchemeItemAfb(VFrame30::SchemeUnit::Display, *afbElement.get()));

		// copy parameters
		//
		for(Afb::AfbParam& param : afbElement->params())
		{
			m_appLogicItem.m_fblItem->toFblElement()->setAfbParamByOpName(param.opName(), param.value());
		}
	}


	QString AppItem::strID() const
	{
		if (m_appLogicItem.m_fblItem->isSignalElement())
		{
			VFrame30::SchemeItemSignal* itemSignal= m_appLogicItem.m_fblItem->toSignalElement();

			if (itemSignal == nullptr)
			{
				assert(false);
				return "";
			}

			return itemSignal->signalStrIds();
		}

		if (m_appLogicItem.m_fblItem->isFblElement())
		{
			VFrame30::SchemeItemAfb* itemFb= m_appLogicItem.m_fblItem->toFblElement();

			if (itemFb == nullptr)
			{
				assert(false);
				return "";
			}

			return itemFb->afbStrID();
		}

		if (m_appLogicItem.m_fblItem->isConstElement())
		{
			VFrame30::SchemeItemConst* itemConst= m_appLogicItem.m_fblItem->toSchemeItemConst();

			if (itemConst == nullptr)
			{
				assert(false);
				return "";
			}


			return QString("Const(%1)").arg(itemConst->valueToString());
		}

		assert(false);		// unknown type of item
		return "";
	}


	// ---------------------------------------------------------------------------------------
	//
	// AppFb class implementation
	//

	AppFb::AppFb(const AppItem& appItem) :
		AppItem(appItem)
	{
		// initialize m_paramValuesArray
		//
		for(const Afb::AfbParam& afbParam : appItem.params())
		{
			AppFbParamValue value(afbParam);

			m_paramValuesArray.insert(afbParam.opName(), value);

			qDebug() << afbParam.opName() << " =  " << value.toString();
		}

		qDebug() << "";
	}


	LogicAfbParam AppFb::getAfbParamByIndex(int index) const
	{
		for(LogicAfbParam param : afb().params())
		{
			if (param.operandIndex() == index)
			{
				return param;
			}
		}

		assert(false);			// bad param index

		return LogicAfbParam();
	}


	LogicAfbSignal AppFb::getAfbSignalByIndex(int index) const
	{
		for(LogicAfbSignal input : afb().inputSignals())
		{
			if (input.operandIndex() == index)
			{
				return input;
			}
		}

		for(LogicAfbSignal output : afb().outputSignals())
		{
			if (output.operandIndex() == index)
			{
				return output;
			}
		}

		assert(false);			// bad signal index

		return LogicAfbSignal();
	}


	QString AppFb::instantiatorID()
	{
		if (m_instantiatorID.isEmpty() == false)
		{
			return m_instantiatorID;
		}

		m_instantiatorID = afb().strID();

		// append instantiator param's values to instantiatorID
		//
		for(const AppFbParamValue& paramValue : m_paramValuesArray)
		{
			if (paramValue.instantiator() == false)
			{
				continue;
			}

			switch(paramValue.dataFormat())
			{
			case E::DataFormat::Float:
				m_instantiatorID += QString(":%1").arg(paramValue.floatValue());
				break;

			case E::DataFormat::SignedInt:
				m_instantiatorID += QString(":%1").arg(paramValue.signedIntValue());
				break;

			case E::DataFormat::UnsignedInt:
				m_instantiatorID += QString(":%1").arg(paramValue.unsignedIntValue());
				break;

			default:
				assert(false);
			}
		}

		return m_instantiatorID;
	}


	bool AppFb::checkRequiredParameters(const QStringList& requiredParams)
	{
		bool result = true;

		for(const QString& opName : requiredParams)
		{
			if (m_paramValuesArray.contains(opName) == false)
			{
				LOG_ERROR(m_log, QString(tr("Required parameter '%1' of FB %2 (%3) is missing")).
						  arg(opName).arg(caption()).arg(typeCaption()));
				result = false;
			}
		}

		return result;
	}


	bool AppFb::checkUnsignedInt(const AppFbParamValue& paramValue)
	{
		if (paramValue.isUnsignedInt())
		{
			return true;
		}

		LOG_ERROR(m_log, QString(tr("Parameter '%1' of FB '%2' must have type UnsignedInt")).
				  arg(paramValue.opName()).arg(afb().caption()));

		return false;
	}


	bool AppFb::checkUnsignedInt16(const AppFbParamValue& paramValue)
	{
		if (paramValue.isUnsignedInt16())
		{
			return true;
		}

		LOG_ERROR(m_log, QString(tr("Parameter '%1' of FB '%2' must have type UnsignedInt16")).
				  arg(paramValue.opName()).arg(afb().caption()));

		return false;
	}


	bool AppFb::checkUnsignedInt32(const AppFbParamValue& paramValue)
	{
		if (paramValue.isUnsignedInt32())
		{
			return true;
		}

		LOG_ERROR(m_log, QString(tr("Parameter '%1' of FB '%2' must have type UnsignedInt32")).
				  arg(paramValue.opName()).arg(afb().caption()));

		return false;
	}


	bool AppFb::checkSignedInt32(const AppFbParamValue& paramValue)
	{
		if (paramValue.isSignedInt32())
		{
			return true;
		}

		LOG_ERROR(m_log, QString(tr("Parameter '%1' of FB '%2' must have type SignedInt32")).
				  arg(paramValue.opName()).arg(afb().caption()));

		return false;
	}


	bool AppFb::checkFloat32(const AppFbParamValue& paramValue)
	{
		if (paramValue.isFloat32())
		{
			return true;
		}

		LOG_ERROR(m_log, QString(tr("Parameter '%1' of FB '%2' must have type Float32")).
				  arg(paramValue.opName()).arg(afb().caption()));

		return false;
	}



	// ---------------------------------------------------------------------------------------
	//
	// AppFbsMap class implementation
	//

	AppFb* AppFbMap::insert(AppFb *appFb)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return nullptr;
		}

		appFb->setNumber(m_fbNumber);

		m_fbNumber++;

		HashedVector<QUuid, AppFb*>::insert(appFb->guid(), appFb);

		return appFb;
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


	AppSignal::AppSignal(const QUuid& guid, E::SignalType signalType, int dataSize, const AppItem *appItem, const QString& strID) :
		m_appItem(appItem),
		m_guid(guid)
	{
		// construct shadow AppSignal based on OutputPin
		//
		setStrID(strID);
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
	bool AppSignalMap::insert(const AppItem* appItem)
	{
		if (!appItem->isSignal())
		{
			ASSERT_RETURN_FALSE
		}

		QString strID = appItem->strID();

		if (strID[0] != '#')
		{
			strID = "#" + strID;
		}

		const Signal* s = m_compiler.getSignal(strID);

		if (s == nullptr)
		{
			QString msg = QString(tr("Signal identifier is not found: %1")).arg(strID);

			LOG_ERROR(m_compiler.log(), msg);

			return false;
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

		return true;
	}

	// insert "shadow" signal bound to FB output pin
	//
	bool AppSignalMap::insert(const AppFb* appFb, const LogicPin& outputPin)
	{
		if (appFb == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		const LogicAfbSignal s = m_compiler.getAfbSignal(appFb->afb().strID(), outputPin.afbOperandIndex());

		E::SignalType signalType = E::SignalType::Discrete;

		Afb::AfbSignalType st = s.type();

		switch(st)
		{
		case Afb::AfbSignalType::Analog:
			signalType = E::SignalType::Analog;
			break;

		case Afb::AfbSignalType::Discrete:
			signalType = E::SignalType::Discrete;
			break;

		default:
			assert(false);
			return false;
		}

		QUuid outPinGuid = outputPin.guid();

		QString strID = getShadowSignalStrID(appFb, outputPin);

		AppSignal* appSignal = nullptr;

		if (m_signalStrIdMap.contains(strID))
		{
			assert(false);			// duplicate pin GUID

			appSignal = m_signalStrIdMap[strID];

			qDebug() << "Bind appSignal = " << strID;
		}
		else
		{
			appSignal = new AppSignal(outPinGuid, signalType, s.size(), appFb, strID);

			appSignal->setAcquire(false);			// non-registered signal

			m_signalStrIdMap.insert(strID, appSignal);

			qDebug() << "Create appSignal = " << strID;

			incCounters(appSignal);
		}

		assert(appSignal != nullptr);

		HashedVector<QUuid, AppSignal*>::insert(outPinGuid, appSignal);

		return true;
	}


	QString AppSignalMap::getShadowSignalStrID(const AppFb* appFb, const LogicPin& outputPin)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return "";
		}

		QString strID = QString("%1_%2$кукпук_N%3_P%4").arg(appFb->afbStrID()).arg(appFb->instance()).arg(appFb->number()).arg(outputPin.afbOperandIndex());

		strID = strID.toUpper();

		strID = "#" + strID.remove(QRegularExpression("[^A-Z0-9_]"));

		return strID;
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


	// ---------------------------------------------------------------------------------------
	//
	// AppFbParamValue class implementation
	//
	// ---------------------------------------------------------------------------------------


	AppFbParamValue::AppFbParamValue(const Afb::AfbParam& afbParam)
	{
		QVariant qv = afbParam.value();

		m_opName = afbParam.opName();
		m_operandIndex = afbParam.operandIndex();
		m_instantiator = afbParam.instantiator();

		if (afbParam.isDiscrete())
		{
			m_type = E::SignalType::Discrete;
			m_dataFormat = E::DataFormat::UnsignedInt;
			m_dataSize = 1;

			m_unsignedIntValue = qv.toUInt();
		}
		else
		{
			m_type = E::SignalType::Analog;
			m_dataSize = afbParam.size();

			switch(afbParam.dataFormat())
			{
			case Afb::SignedInt:
				m_dataFormat = E::DataFormat::SignedInt;
				m_signedIntValue = qv.toInt();
				break;

			case Afb::UnsignedInt:
				m_dataFormat = E::DataFormat::UnsignedInt;
				m_unsignedIntValue = qv.toUInt();
				break;

			case Afb::Float:
				assert(m_dataSize == SIZE_32BIT);
				m_dataFormat = E::DataFormat::Float;
				m_floatValue = qv.toFloat();
				break;

			default:
				assert(false);
			}
		}
	}


	QString AppFbParamValue::toString() const
	{
		QString str;

		switch(m_dataFormat)
		{
		case E::DataFormat::UnsignedInt:
			str = QString("%1").arg(m_unsignedIntValue);
			break;

		case E::DataFormat::SignedInt:
			str = QString("%1").arg(m_signedIntValue);
			break;

		case E::DataFormat::Float:
			str = QString("%1").arg(m_floatValue);
			break;

		default:
			assert(false);
		}

		return str;
	}

}



