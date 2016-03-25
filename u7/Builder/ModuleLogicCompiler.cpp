#include "../Builder/ModuleLogicCompiler.h"
#include "../Builder/ApplicationLogicCompiler.h"

namespace Builder
{

    // ---------------------------------------------------------------------------------
    //
    //	ModuleLogicCompiler class implementation
    //
    // ---------------------------------------------------------------------------------

    ModuleLogicCompiler::ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm) :
        m_appLogicCompiler(appLogicCompiler),
        m_memoryMap(appLogicCompiler.m_log),
        m_appSignals(*this)
    {
        m_equipmentSet = appLogicCompiler.m_equipmentSet;
        m_deviceRoot = m_equipmentSet->root();
        m_signals = appLogicCompiler.m_signals;
        m_afbl = appLogicCompiler.m_afbl;
        m_appLogicData = appLogicCompiler.m_appLogicData;
        m_resultWriter = appLogicCompiler.m_resultWriter;
        m_log = appLogicCompiler.m_log;
        m_lm = lm;
        m_connections = appLogicCompiler.m_connections;
        m_optoModuleStorage = appLogicCompiler.m_optoModuleStorage;

        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::OTHER, "OTHER");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::LM, "LM");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AIM, "AIM");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AOM, "AOM");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::DIM, "DIM");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::DOM, "DOM");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::AIFM, "AIFM");
        m_moduleFamilyTypeStr.insert(Hardware::DeviceModule::OCM, "OCM");
    }


    Signal* ModuleLogicCompiler::getSignal(const QString& strID)
    {
        if (m_signalsStrID.contains(strID))
        {
            return m_signalsStrID.value(strID);
        }

        return nullptr;
    }


    bool ModuleLogicCompiler::firstPass()
    {

        LOG_EMPTY_LINE(m_log)

        msg = QString(tr("Compilation pass #1 for LM %1 was started...")).arg(m_lm->strId());

        LOG_MESSAGE(m_log, msg);

        bool result = false;

        do
        {
            if (!getLMChassis()) break;

            if (!loadLMSettings()) break;

            if (!loadModulesSettings()) break;

            if (!prepareAppLogicGeneration()) break;

            if (!buildRS232SignalLists()) break;

            result = true;
        }

        while(false);

        if (result == true)
        {
            msg = QString(tr("Compilation pass #1 for LM %1 was successfully finished.")).
                    arg(m_lm->strId());

            LOG_SUCCESS(m_log, msg);
        }
        else
        {
            msg = QString(tr("Compilation pass #1 for LM %1 was finished with errors")).arg(m_lm->strId());
            LOG_MESSAGE(m_log, msg);
        }

        return result;
    }

    bool ModuleLogicCompiler::secondPass()
    {

        LOG_EMPTY_LINE(m_log)

        msg = QString(tr("Compilation pass #2 for LM %1 was started...")).arg(m_lm->strId());

        LOG_MESSAGE(m_log, msg);

        bool result = false;

        do
        {
            if (!generateAppStartCommand()) break;

            if (!generateFbTestCode()) break;

    //			if (!initAfbs()) break;

            if (!finishTestCode()) break;

            if (!startAppLogicCode()) break;

            if (!initAfbs()) break;

            if (!copyLMDataToRegBuf()) break;

            if (!copyInModulesAppLogicDataToRegBuf()) break;

            if (!initOutModulesAppLogicDataInRegBuf()) break;

            if (!generateAppLogicCode()) break;

            if (!copyDiscreteSignalsToRegBuf()) break;

            if (!copyLmOutSignalsToModuleMemory()) break;

            if (!copyOutModulesAppLogicDataToModulesMemory()) break;

            if (!generateRS232ConectionCode()) break;

            if (!finishAppLogicCode()) break;

            if (!calculateCodeRunTime()) break;

            if (!writeOcmRsSignalsXml()) break;

            if (!writeResult()) break;

            result = true;
        }
        while(false);

        if (result == true)
        {
            msg = QString(tr("Compilation pass #2 for LM %1 was successfully finished.")).
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

            displayTimingInfo();
        }
        else
        {
            msg = QString(tr("Compilation pass #2 for LM %1 was finished with errors")).arg(m_lm->strId());
            LOG_MESSAGE(m_log, msg);
        }

        cleanup();

        return result;
    }


    void ModuleLogicCompiler::displayTimingInfo()
    {
        QString str_percent;
        QString str;

        // display IDR phase timing
        //
        double idrPhaseTime = (1.0/m_lmClockFrequency) * m_idrPhaseClockCount;
        double idrPhaseTimeUsed = 0;

        if (idrPhaseTime != 0)
        {
            idrPhaseTimeUsed = (idrPhaseTime * 100) / (static_cast<double>(m_lmIDRPhaseTime) / 1000000.0);
        }

        str_percent.sprintf("%.3f", static_cast<float>(idrPhaseTimeUsed));
        str.sprintf("%.3f", static_cast<float>(idrPhaseTime * 1000000));

        if (idrPhaseTimeUsed < 90)
        {
            LOG_MESSAGE(m_log, QString(tr("Input Data Receive phase time used - %1% (%2 clocks or %3 μs of %4 μs)")).
                        arg(str_percent).arg(m_idrPhaseClockCount).arg(str).arg(m_lmIDRPhaseTime));
        }
        else
        {
            if (idrPhaseTimeUsed < 100)
            {
                LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                            QString(tr("Input Data Receive phase time used - %1% (%2 clocks or %3 μs of %4 μs)")).
                                arg(str_percent).arg(m_idrPhaseClockCount).arg(str).arg(m_lmIDRPhaseTime));
            }
            else
            {
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("Input Data Receive phase time used - %1% (%2 clocks or %3 μs of %4 μs)")).
                            arg(str_percent).arg(m_idrPhaseClockCount).arg(str).arg(m_lmIDRPhaseTime));
            }
        }

        // display ALP phase timing
        //
        double alpPhaseTime = (1.0/m_lmClockFrequency) * m_alpPhaseClockCount;
        double alpPhaseTimeUsed = 0;

        if (alpPhaseTime != 0)
        {
            alpPhaseTimeUsed = (alpPhaseTime * 100) / (static_cast<double>(m_lmALPPhaseTime) / 1000000.0);
        }

        str_percent.sprintf("%.3f", static_cast<float>(alpPhaseTimeUsed));
        str.sprintf("%.3f", static_cast<float>(alpPhaseTime * 1000000));

        if (alpPhaseTimeUsed < 90)
        {
            LOG_MESSAGE(m_log, QString(tr("Application Logic Processing phase time used - %1% (%2 clocks or %3 μs of %4 μs)")).
                        arg(str_percent).arg(m_alpPhaseClockCount).arg(str).arg(m_lmALPPhaseTime));
        }
        else
        {
            if (alpPhaseTimeUsed < 100)
            {
                LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                            QString(tr("Application Logic Processing phase time used - %1% (%2 clocks or %3 μs of %4 μs)")).
                            arg(str_percent).arg(m_alpPhaseClockCount).arg(str).arg(m_lmALPPhaseTime));
            }
            else
            {
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("Application Logic Processing phase time used - %1% (%2 clocks or %3 μs of %4 μs)")).
                            arg(str_percent).arg(m_alpPhaseClockCount).arg(str).arg(m_lmALPPhaseTime));
            }
        }
    }


    bool ModuleLogicCompiler::getLMChassis()
    {
        m_chassis = m_lm->getParentChassis();

        if (m_chassis == nullptr)
        {
            msg = QString(tr("LM %1 must be installed in the chassis!")).arg(m_lm->strId());
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
            return false;
        }

        return true;
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

            m_code.initCommandMemoryRanges(m_appLogicBitData.startAddress(),
                                           m_appLogicBitData.sizeW(),
                                           m_appLogicWordData.startAddress(),
                                           m_appLogicWordData.sizeW());
        }

        result &= getLMIntProperty("ClockFrequency", &m_lmClockFrequency);
        result &= getLMIntProperty("ALPPhaseTime", &m_lmALPPhaseTime);
        result &= getLMIntProperty("IDRPhaseTime", &m_lmIDRPhaseTime);

        result &= getLMIntProperty("AppLogicFrameSize", &m_lmAppLogicFrameSize);
        result &= getLMIntProperty("AppLogicFrameCount", &m_lmAppLogicFrameCount);

        result &= getLMIntProperty("CycleDuration", &m_lmCycleDuration);

        if (result)
        {
            LOG_MESSAGE(m_log, QString(tr("Loading LMs settings... Ok")));
        }
        else
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("LM settings are not loaded")));
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
                result &= ApplicationLogicCompiler::getDeviceIntProperty(device, moduleSetting.name, moduleSetting.var);
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Modules settings are not loaded")));
        }

        return result;
    }


    bool ModuleLogicCompiler::prepareAppLogicGeneration()
    {
        bool result = false;

        std::shared_ptr<AppLogicModule> appLogicModule = m_appLogicData->getModuleLogicData(m_lm->strId());

        m_moduleLogic = appLogicModule.get();

        if (m_moduleLogic == nullptr)
        {
            msg = QString(tr("Application logic not found for module %1")).arg(m_lm->strId());
            LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
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


    bool ModuleLogicCompiler::buildOptoModulesStorage()
    {
        return true;
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
            if (module.device == nullptr)
            {
                LOG_INTERNAL_ERROR(m_log);

                result = false;

                continue;
            }

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

            case Hardware::DeviceModule::FamilyType::AIFM:
                result &= copyAifmDataToRegBuf(module);
                break;

            default:
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Unknown input module %1 family type")).arg(module.device->strId()));

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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("AIM module must have 128 input signals")));
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
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
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
                    LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                              QString(tr("Signal %1 must have 32-bit data size")).arg(signal->strID()));
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

                cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
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


    bool ModuleLogicCompiler::copyAifmDataToRegBuf(const Module& module)
    {
        LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                    QString(tr("Copying AIFM data to RegBuf is not implemented (module %1)")).arg(module.device->strId()));

        return true;
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signal is not found, GUID: %1")).arg(appItem->guid().toString()));
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signal value undefined: %1")).arg(appSignal->strID()));
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
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("Floating point constant connected to discrete signal: ")).arg(appSignal.strID()));

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
                        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                  QString(tr("Constant of type 'Float' (value %1) connected to signal %2 of type 'Signed Int'")).
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
                        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                  QString(tr("Constant of type 'Signed Int' (value %1) connected to signal %2 of type 'Float'")).
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

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

                return false;
            }

            if (appSignal.dataFormat() != srcSignal.dataFormat())
            {
                msg = QString(tr("Signals %1 and %2 data formats are not compatible")).
                        arg(appSignal.strID()).arg(srcSignal.strID());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

                return false;
            }

            if (appSignal.dataSize() != srcSignal.dataSize())
            {
                msg = QString(tr("Signals %1 and %2 have different data sizes")).
                        arg(appSignal.strID()).arg(srcSignal.strID());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

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

        if (appSignal.dataFormat() != srcSignal.dataFormat())
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signals %1 and  %2 is not compatible by dataFormat")).
                      arg(srcSignal.strID()).arg(appSignal.strID()));

            return false;
        }

        if (appSignal.dataSize() != srcSignal.dataSize())
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signals %1 and  %2 is not compatible by dataSize")).
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signal %1 RAM addreess is not calculated")).
                      arg(srcSignal.strID()));
            return false;
        }

        if (destRamAddrOffset == -1 || destRamAddrBit == -1)
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signal %1 RAM addreess is not calculated")).
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
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("Unknown data size of signal %1 - %2 bit")).
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
            cmd.setComment(QString(tr("compute %1")).arg(appFb->caption()));
        }
        else
        {
            cmd.nstart(appFb->opcode(), appFb->instance(), startCount, appFb->caption(), appFb->runTime());
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
                    LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Signal value undefined: %1")).arg(appSignal->strID()));

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
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Floating point constant connected to discrete input")));
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

        LogicAfbSignal afbSignal;

        if (appFb.getAfbSignalByIndex(fbParamNo, &afbSignal) == false)
        {
            return false;
        }

        if (afbSignal.isAnalog())
        {
            if (!appSignal.isAnalog())
            {
                msg = QString(tr("Discrete signal %1 connected to analog input '%2' of %3")).
                        arg(appSignal.strID()).arg(afbSignal.caption()).arg(appFb.caption());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

                return false;
            }

            if (appSignal.isCompatibleDataFormat(afbSignal.dataFormat()) == false)
            {
                msg = QString(tr("Signal %1 data format is not compatible with input '%2' data format of %3 ")).
                        arg(appSignal.strID()).arg(afbSignal.caption()).arg(appFb.caption());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

                return false;
            }

            if (appSignal.dataSize() != afbSignal.size())
            {
                msg = QString(tr("Signal %1 data size (%2) is not compatible with input '%3' data size (%4) of %4")).
                        arg(appSignal.strID()).arg(appSignal.dataSize()).
                        arg(afbSignal.caption()).arg(afbSignal.size()).arg(appFb.caption());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

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

        Command cmd;

        int ramAddrOffset = appSignal.ramAddr().offset();
        int ramAddrBit = appSignal.ramAddr().bit();

        if (ramAddrOffset == -1 || ramAddrBit == -1)
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Address of signal '%1' is not calculated")).
                      arg(appSignal.strID()));
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Signal is not found, GUID: %1")).arg(signalGuid.toString()));
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

        LogicAfbSignal afbSignal;

        if (appFb.getAfbSignalByIndex(fbParamNo, &afbSignal) == false)
        {
            return false;
        }

        if (afbSignal.isAnalog())
        {
            if (!appSignal->isAnalog())
            {
                msg = QString(tr("Analog output %1.%2 connected to discrete signal %3")).
                        arg(appFb.strID()).arg(afbSignal.caption()).arg(appSignal->strID());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

                return false;
            }

            if (appSignal->isCompatibleDataFormat(afbSignal.dataFormat()) == false)
            {
                msg = QString(tr("Signal %1 data format is not compatible with output '%2' data format of %3 ")).
                        arg(appSignal->strID()).arg(afbSignal.caption()).arg(appFb.caption());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

                return false;
            }

            if (appSignal->dataSize() != afbSignal.size())
            {
                msg = QString(tr("Signal %1 data size (%2) is not compatible with output '%3' data size (%4) of %4")).
                        arg(appSignal->strID()).arg(appSignal->dataSize()).
                        arg(afbSignal.caption()).arg(afbSignal.size()).arg(appFb.caption());

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

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

        Command cmd;

        int ramAddrOffset = appSignal->ramAddr().offset();
        int ramAddrBit = appSignal->ramAddr().bit();

        if (ramAddrOffset == -1 || ramAddrBit == -1)
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("RAM-address of signal %1 is not calculated")).arg(appSignal->strID()));
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

                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Unknown output module family type"));

                result = false;
            }
        }

        return result;
    }


    bool ModuleLogicCompiler::buildRS232SignalLists()
    {
        if (m_optoModuleStorage == nullptr)
        {
            assert(false);
            return false;
        }

        QList<Hardware::OptoModule*> optoModules = m_optoModuleStorage->getLmAssociatedOptoModules(m_lm->strId());

        if (optoModules.isEmpty())
        {
            return true;
        }

        bool result = true;

        for(Hardware::OptoModule* optoModule : optoModules)
        {
            if (optoModule == nullptr)
            {
                assert(false);
                LOG_INTERNAL_ERROR(m_log);
                result = false;
                continue;
            }

            QList<Hardware::OptoPort*> rs232Ports = optoModule->getRS232Ports();

            if (rs232Ports.isEmpty())
            {
                continue;
            }

            for(Hardware::OptoPort* port : rs232Ports)
            {
                QStringList txSignalsStrIDList = port->getTxSignalsStrID();

                for(const QString& txSignalStrID : txSignalsStrIDList)
                {
                    if (m_signalsStrID.contains(txSignalStrID) == false)
                    {
                        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                  QString(tr("Signal '%1' is not found (RS232/485 connection '%2')")).
                                  arg(txSignalStrID).arg(port->connectionCaption()));

                        result = false;
                        continue;
                    }

                    Signal* txSignal = m_signalsStrID[txSignalStrID];

                    if (txSignal == nullptr)
                    {
                        assert(false);
                        continue;
                    }

                    port->addTxSignal(txSignal);
                }

                port->calculateTxSignalsAddresses();

                QString idStr;

                idStr.sprintf("0x%X", port->txDataID());

                LOG_MESSAGE(m_log, QString(tr("RS232/485 connection '%1': analog signals %2, discrete signals %3, data size %4 bytes, dataID %5")).
                            arg(port->connectionCaption()).
                            arg(port->txAnalogSignalsCount()).
                            arg(port->txDiscreteSignalsCount()).
                            arg(port->txDataSizeW() * 2).
                            arg(idStr)  );
            }
        }

        return result;
    }

    bool ModuleLogicCompiler::generateRS232ConectionCode()
    {
        if (m_lm == nullptr || m_optoModuleStorage == nullptr)
        {
            LOG_INTERNAL_ERROR(m_log);
            assert(false);
            return false;
        }

        int connectionCount = m_connections->count();

        bool result = true;

        for(int i = 0; i < connectionCount; i++)
        {
            std::shared_ptr<Hardware::Connection> connection = m_connections->get(i);

            if (connection == nullptr)
            {
                LOG_INTERNAL_ERROR(m_log);
                assert(false);
                continue;
            }

            if (connection->mode() != Hardware::OptoPort::Mode::Serial)
            {
                continue;
            }

            Hardware::OptoPort* optoPort = m_optoModuleStorage->getOptoPort(connection->port1StrID());

            if (optoPort == nullptr)
            {
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("OCM port '%1' is not found (connection '%2')")).
                          arg(connection->port1StrID().
                          arg(connection->caption())));
                return false;
            }

            Hardware::OptoModule* optoModule = m_optoModuleStorage->getOptoModule(optoPort);

            if (optoModule == nullptr)
            {
                assert(false);
                continue;
            }

            if (optoModule->lmStrID() == m_lm->strId())
            {
                // this serial connection must be processed in this LM
                //
                result &= generateRS232ConectionCode(connection, optoModule, optoPort);
            }
        }

        return result;
    }


    bool ModuleLogicCompiler::generateRS232ConectionCode(std::shared_ptr<Hardware::Connection> connection,
                                                         Hardware::OptoModule* optoModule,
                                                         Hardware::OptoPort* optoPort)
    {
        if (optoModule == nullptr ||
            optoPort == nullptr)
        {
            assert(false);
            return false;
        }

        // build analog and discrete signals list
        //
        QStringList& signslList = connection->signalList();

        bool result = true;

        HashedVector<QString, Signal*> analogSignals;
        HashedVector<QString, Signal*> discreteSignals;

        int analogSgnalsSizeW = 0;
        int discreteSignalsSizeBit = 0;

        for(QString signalStrID : signslList)
        {
            if (m_signalsStrID.contains(signalStrID) == false)
            {
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("Signal '%1' is not found (RS232/485 connection '%2')")).
                          arg(signalStrID).arg(connection->caption()));
                result &= false;
                continue;
            }

            Signal* signal = m_signalsStrID[signalStrID];

            if (signal == nullptr)
            {
                LOG_INTERNAL_ERROR(m_log);
                result &= false;
                continue;
            }

            if (signal->isAnalog())
            {
                analogSignals.insert(signalStrID, signal);

                analogSgnalsSizeW += signal->dataSize() / WORD_SIZE;
            }
            else
            {
                discreteSignals.insert(signalStrID, signal);

                discreteSignalsSizeBit++;
            }
        }

        if (result == false)
        {
            return result;
        }

        int discreteSignalsSizeW = discreteSignalsSizeBit / WORD_SIZE + (discreteSignalsSizeBit % WORD_SIZE ? 1 : 0);

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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("AOM module must have 32 output signals")));
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
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
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
                    LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Signal %1 must have 32-bit data size")).arg(signal->strID()));
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

                cmd.start(appFb->opcode(), appFb->instance(), appFb->caption(), appFb->runTime());
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


    bool ModuleLogicCompiler::calculateCodeRunTime()
    {
        bool result = m_code.getRunTimes(&m_idrPhaseClockCount, &m_alpPhaseClockCount);

        if (result == false)
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("Code runtime calculcation error!")))
        }

        return result;
    }


    bool ModuleLogicCompiler::writeResult()
    {
        if (m_lm->propertyExists("SubsysID") == false)
        {
            assert(false);
            return false;
        }

        if (m_lm->propertyExists("Channel") == false)
        {
            assert(false);
            return false;
        }

        QString subsysId = m_lm->propertyValue("SubsysID").toString();

        int channel = m_lm->propertyValue("Channel").toInt();

        bool result = true;

        m_code.generateBinCode();

        QByteArray binCode;

        m_code.getBinCode(binCode);

        m_appLogicCompiler.writeBinCodeForLm(subsysId, m_lm->caption(), channel,
                                                          m_lmAppLogicFrameSize, m_lmAppLogicFrameCount, binCode);
        QStringList mifCode;

        m_code.getMifCode(mifCode);

        result &= m_resultWriter->addFile(subsysId, QString("%1-%2ch.mif").
                                          arg(m_lm->caption()).arg(channel), mifCode);

        QStringList asmCode;

        m_code.getAsmCode(asmCode);

        result = m_resultWriter->addFile(subsysId, QString("%1-%2ch.asm").
                                         arg(m_lm->caption()).arg(channel), asmCode);

        QStringList memFile;

        m_memoryMap.getFile(memFile);

        result = m_resultWriter->addFile(subsysId, QString("%1-%2ch.mem").
                                         arg(m_lm->caption()).arg(channel), memFile);
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


    bool ModuleLogicCompiler::writeOcmRsSignalsXml()
    {
        if (!m_signals || m_signals->isEmpty())
        {
            LOG_MESSAGE(m_log, tr("Signals not found!"));
            return true;
        }

        if (!m_connections)
        {
            LOG_MESSAGE(m_log, tr("Connections not found!"));
            return true;
        }

        if (m_signalsStrID.isEmpty())
        {
            createDeviceBoundSignalsMap();
        }
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

                m_resultWriter->addFile(m_lm->propertyValue("SubsysID").toString(), QString("rs-%1-ocm.xml").arg(connection->port1StrID()), data);
            }
        });*/

        return true;
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
                    LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
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

                    switch(signal->inOutType())
                    {
                    case E::SignalInOutType::Input:
                        appItem = createFbForAnalogInputSignalConversion(*signal);
                        break;

                    case E::SignalInOutType::Output:
                        appItem = createFbForAnalogOutputSignalConversion(*signal);
                        break;

                    case E::SignalInOutType::Internal:
                        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                  QString(tr("Internal signal %1 can not be connected to device signal. Connect this signal to appropriate LM module or change its type to Input or Output")).
                                  arg(signal->strID()));
                        break;

                    default:
                        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                  QString(tr("Unknown inOutType of signal %1.")).
                                  arg(signal->strID()));
                    }

                    if (appItem != nullptr)
                    {
                        AppFb* appFb = createAppFb(*appItem);

                        m_inOutSignalsToScalAppFbMap.insert(signal->strID(), appFb);

                        qDebug() << signal->strID();

                        delete appItem;
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Low and High ADC values of signal %1 are equal (= %2)")).arg(signal.strID()).arg(x1));
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Unknown conversion for signal %1, dataFormat %2")).
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Low and High Limit values of signal %1 are equal (= %2)")).arg(signal.strID()).arg(x1));
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
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Unknown conversion for signal %1, dataFormat %2")).
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
                LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);
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
                    LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("More than one application signal is bound to device signal %1")).arg(deviceSignal->strId()));
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

                    ApplicationLogicCompiler::getDeviceIntProperty(deviceSignal.get(), QString("ValueOffset"), &signalOffset);
                    ApplicationLogicCompiler::getDeviceIntProperty(deviceSignal.get(), QString("ValueBit"), &bit);

                    if (signalOffset != ERR_VALUE && bit != ERR_VALUE)
                    {
                        if (signalOffset >= module.appLogicDataSize)
                        {
                            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                      QString(tr("Signal %1 offset out of module application data size")).arg(signal->strID()));

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
                        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                                  QString(tr("Can't calculate RAM address of application signal %1")).arg(signal->strID()));

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
                Address16 ramAddr = m_memoryMap.addRegAnalogSignal(appSignal->constSignal());

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
                Address16 ramAddr = m_memoryMap.addRegDiscreteSignal(appSignal->constSignal());

                appSignal->ramAddr() = ramAddr;

                Address16 regAddr = m_memoryMap.addRegDiscreteSignalToRegBuffer(appSignal->constSignal());

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
                Address16 ramAddr = m_memoryMap.addNonRegAnalogSignal(appSignal->constSignal());

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
                Address16 ramAddr = m_memoryMap.addNonRegDiscreteSignal(appSignal->constSignal());

                appSignal->ramAddr() = ramAddr;
            }
        }

        m_memoryMap.recalculateAddresses();

        return result;
    }


    bool ModuleLogicCompiler::getLMIntProperty(const QString& name, int *value)
    {
        return ApplicationLogicCompiler::getDeviceIntProperty(m_lm, name, value);
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
            if (param.operandIndex() == ModuleLogicCompiler::FOR_USER_ONLY_PARAM_INDEX)
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
        }
    }


    bool AppFb::getAfbParamByIndex(int index, LogicAfbParam* afbParam) const
    {
        for(LogicAfbParam param : afb().params())
        {
            if (param.operandIndex() == index)
            {
                *afbParam = param;
                return true;
            }
        }

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Not found parameter with opIndex = %1 int FB %2")).arg(index).arg(caption()));

        return false;
    }


    bool AppFb::getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const
    {
        if (afbSignal == nullptr)
        {
            return false;
        }

        for(LogicAfbSignal input : afb().inputSignals())
        {
            if (input.operandIndex() == index)
            {
                *afbSignal = input;
                return true;
            }
        }

        for(LogicAfbSignal output : afb().outputSignals())
        {
            if (output.operandIndex() == index)
            {
                *afbSignal = output;
                return true;
            }
        }

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Not found signal with opIndex = %1 int FB %2")).arg(index).arg(caption()));

        return false;
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
                LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                          QString(tr("Required parameter '%1' of FB %2 (%3) is missing")).
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

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Parameter '%1' of FB '%2' must have type UnsignedInt")).
                  arg(paramValue.opName()).arg(afb().caption()));

        return false;
    }


    bool AppFb::checkUnsignedInt16(const AppFbParamValue& paramValue)
    {
        if (paramValue.isUnsignedInt16())
        {
            return true;
        }

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Parameter '%1' of FB '%2' must have type UnsignedInt16")).
                  arg(paramValue.opName()).arg(afb().caption()));

        return false;
    }


    bool AppFb::checkUnsignedInt32(const AppFbParamValue& paramValue)
    {
        if (paramValue.isUnsignedInt32())
        {
            return true;
        }

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Parameter '%1' of FB '%2' must have type UnsignedInt32")).
                  arg(paramValue.opName()).arg(afb().caption()));

        return false;
    }


    bool AppFb::checkSignedInt32(const AppFbParamValue& paramValue)
    {
        if (paramValue.isSignedInt32())
        {
            return true;
        }

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Parameter '%1' of FB '%2' must have type SignedInt32")).
                  arg(paramValue.opName()).arg(afb().caption()));

        return false;
    }


    bool AppFb::checkFloat32(const AppFbParamValue& paramValue)
    {
        if (paramValue.isFloat32())
        {
            return true;
        }

        LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                  QString(tr("Parameter '%1' of FB '%2' must have type Float32")).
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


    AppSignal::AppSignal(Signal *signal, const AppItem *appItem) :
        m_signal(signal),
        m_appItem(appItem)
    {
        m_isShadowSignal = false;

        // construct AppSignal based on real signal
        //
        if (m_signal == nullptr)
        {
            assert(false);
            return;
        }

        //*dynamic_cast<Signal*>(this) = *signal;

        // believe that all input signals have already been computed
        //
        if (m_signal->isInput())
        {
            setComputed();
        }
    }


    AppSignal::AppSignal(const QUuid& guid, E::SignalType signalType, E::DataFormat dataFormat, int dataSize, const AppItem *appItem, const QString& strID) :
        m_appItem(appItem),
        m_guid(guid)
    {
        m_isShadowSignal = true;

        m_signal = new Signal;

        // construct shadow AppSignal based on OutputPin
        //
        m_signal->setStrID(strID);
        m_signal->setType(signalType);
        m_signal->setDataFormat(dataFormat);
        m_signal->setDataSize(dataSize);
        m_signal->setInOutType(E::SignalInOutType::Internal);
        m_signal->setAcquire(false);								// non-registered signal !

    }


    AppSignal::~AppSignal()
    {
        if (m_isShadowSignal == true)
        {
            delete m_signal;
        }
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

        Signal* s = m_compiler.getSignal(strID);

        if (s == nullptr)
        {
            QString msg = QString(tr("Signal identifier is not found: %1")).arg(strID);

            LOG_ERROR_OBSOLETE(m_compiler.log(), Builder::IssueType::NotDefined, msg);

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

        switch(s.type())			// Afb::AfbSignalType
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

        E::DataFormat dataFormat = E::DataFormat::SignedInt;

        switch(s.dataFormat())		// Afb::AfbDataFormat
        {
        case Afb::AfbDataFormat::Float:
            dataFormat = E::DataFormat::Float;
            break;

        case Afb::AfbDataFormat::UnsignedInt:
            dataFormat = E::DataFormat::UnsignedInt;
            break;

        case Afb::AfbDataFormat::SignedInt:
            dataFormat = E::DataFormat::SignedInt;
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
            assert(false);							// duplicate StrID

            appSignal = m_signalStrIdMap[strID];

            qDebug() << "Bind appSignal = " << strID;
        }
        else
        {
            appSignal = new AppSignal(outPinGuid, signalType, dataFormat, s.size(), appFb, strID);

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

        QString strID = QString("%1_I%2_N%3_P%4").arg(appFb->afbStrID()).arg(appFb->instance()).arg(appFb->number()).arg(outputPin.afbOperandIndex());

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
