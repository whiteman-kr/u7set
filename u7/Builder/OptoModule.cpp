#include "OptoModule.h"
#include "./Builder/Builder.h"

namespace Hardware
{

    OptoPort::OptoPort(DeviceController* optoPortController, int port) :
        m_deviceController(optoPortController)
    {
        if (optoPortController == nullptr)
        {
            assert(false);
            return;
        }

        m_strID = optoPortController->strId();
        m_port = port;
    }


    OptoModule::OptoModule(DeviceModule* module, OutputLog* log) :
        m_deviceModule(module),
        m_log(log)
    {
        if (module == nullptr || m_log == nullptr)
        {
            return;
        }

        m_strID = module->strId();

        QVariant val = module->propertyValue("OptoPortCount");

        if (val.isValid() == false)
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                      QString(tr("Property '%1' is not found in device '%2'")).arg("OptoPortCount").arg(module->strId()));
            return;
        }

        int portCount = val.toInt();

        int findPortCount = 0;

        for(int i = 0; i < portCount; i++)
        {
            QString portStrID = QString("%1_PORT0%2").arg(module->strId()).arg(i + 1);

            int childrenCount = module->childrenCount();

            for(int c = 0; c < childrenCount; c++)
            {
                DeviceObject* child = module->child(c);

                if (child == nullptr)
                {
                    assert(false);
                    continue;
                }

                if (child->isController() && child->strId() == portStrID)
                {
                    DeviceController* optoPortController = child->toController();

                    if (optoPortController == nullptr)
                    {
                        assert(false);
                        LOG_INTERNAL_ERROR(m_log);
                        return;
                    }

                    OptoPort* optoPort = new OptoPort(optoPortController, i + 1);

                    m_ports.insert(optoPortController->strId(), optoPort);

                    findPortCount++;

                    break;
                }
            }
        }

        if (findPortCount != portCount)
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
                               QString(tr("Not all opto-port controllers found in module '%1'")).arg(module->strId()));
            return;
        }

        m_valid = true;
    }


    OptoModule::~OptoModule()
    {
        for(OptoPort* optoPort : m_ports)
        {
            delete optoPort;
        }

        m_ports.clear();
    }


    OptoModuleStorage::OptoModuleStorage(EquipmentSet* equipmentSet, OutputLog *log) :
        m_equipmentSet(equipmentSet),
        m_log(log)
    {
    }


    OptoModuleStorage::~OptoModuleStorage()
    {
        clear();
    }


    void OptoModuleStorage::clear()
    {
        for(OptoModule* optoModule : m_modules)
        {
            delete optoModule;
        }

        m_modules.clear();
        m_ports.clear();
    }


    bool OptoModuleStorage::build()
    {
        LOG_EMPTY_LINE(m_log);

        LOG_MESSAGE(m_log, QString(tr("Searching opto-modules")));

        clear();

        bool result = true;

        equipmentWalker(m_equipmentSet->root(), [this, &result](DeviceObject* currentDevice)
            {
                if (currentDevice == nullptr)
                {
                    assert(false);
                    result = false;
                    return;
                }

                if (currentDevice->isModule() == false)
                {
                    return;
                }

                Hardware::DeviceModule* module = currentDevice->toModule();

                result &= addModule(module);
            }
        );


        for(OptoModule* optoModule : m_modules)
        {
            for(OptoPort* optoPort : optoModule->m_ports)
            {
                m_ports.insert(optoPort->strID(), optoPort);
            }
        }

        if (result == true)
        {
            LOG_MESSAGE(m_log, QString(tr("Opto-modules found: %1")).arg(m_modules.count()))
            LOG_SUCCESS(m_log, QString(tr("Ok")));
        }

        return result;
    }




    bool OptoModuleStorage::addModule(DeviceModule* module)
    {
        if (module == nullptr)
        {
            assert(false);
            LOG_INTERNAL_ERROR(m_log);
            return false;
        }

        if (module->moduleFamily() != DeviceModule::FamilyType::LM &&
            module->moduleFamily() != DeviceModule::FamilyType::OCM)
        {
            // this is not opto-module
            //
            return true;
        }

        OptoModule* optoModule = new OptoModule(module, m_log);

        if (optoModule->isValid() == false)
        {
            delete optoModule;
            return false;
        }

        m_modules.insert(module->strId(), optoModule);

        return true;
    }


    OptoModule* OptoModuleStorage::getOptoModule(const QString& optoModuleStrID)
    {
        if (m_modules.contains(optoModuleStrID))
        {
            return m_modules[optoModuleStrID];
        }

        return nullptr;
    }


    OptoPort* OptoModuleStorage::getOptoPort(const QString& optoPortStrID)
    {
        if (m_ports.contains(optoPortStrID))
        {
            return m_ports[optoPortStrID];
        }

        return nullptr;
    }
}
