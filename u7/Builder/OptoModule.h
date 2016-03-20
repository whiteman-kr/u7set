#pragma once

#include <QObject>

#include "../include/OrderedHash.h"
#include "../include/DeviceObject.h"
#include "../include/OutputLog.h"
#include "../include/OrderedHash.h"


namespace Hardware
{
    class DeviceModule;

    class OptoPort : public QObject
    {
        Q_OBJECT

    public:
        enum class Mode
        {
            Optical,
            Serial
        };
        Q_ENUM(Mode)

        enum class SerialMode
        {
            RS232,
            RS485
        };
        Q_ENUM(SerialMode)

    private:
        QString m_strID;
        DeviceController* m_deviceController = nullptr;

        int m_port = 0;

        Mode m_mode = Mode::Optical;
        SerialMode m_serialMode = SerialMode::RS232;

        int m_rxStartAddress = 0;
        int m_rxSizeW = 0;

        int m_txStartAddress = 0;
        int m_txSizeW = 0;

        bool m_enable = true;             // serial mode only
        bool m_enableDuplex = false;      // serial mode and OCMN only

    public:
        OptoPort(DeviceController* optoPortController, int port);

        QString strID() const { return m_strID; }
        void setStrID(const QString& strID) { m_strID = strID; }

        int port() const { return m_port; }
        void setPort(int port) { m_port = port; }

        Mode mode() const { return m_mode; }
        void setMode(Mode mode) { m_mode = mode; }

        SerialMode serialMode() const { return m_serialMode; }
        void setSerialMode(SerialMode serialMode) { m_serialMode = serialMode; }

        int rxStartAddress() const { return m_rxStartAddress; }
        void setRxStartAddress(int address) { m_rxStartAddress = address; }

        int txStartAddress() const { return m_txStartAddress; }
        void setTxStartAddress(int address) { m_txStartAddress = address; }

        bool enable() const { return m_enable; }
        void setEnable(bool enable) { m_enable = enable; }

        bool enableDuplex() const { return m_enableDuplex; }
        void setEnableDuplex(bool enable) { m_enableDuplex = enable; }

        const DeviceController* deviceController() const { return m_deviceController; }
    };


    // Class represent modules with opto-ports - LM or OCM
    //
    class OptoModule : public QObject
    {
        Q_OBJECT

    private:
        QString m_strID;
        DeviceModule* m_deviceModule = nullptr;

        HashedVector<QString, OptoPort*> m_ports;
        OutputLog* m_log = nullptr;

        bool m_valid = false;

    public:
        OptoModule(DeviceModule* module, OutputLog* log);
        ~OptoModule();

        bool isValid() const { return m_valid; }

        const DeviceModule* deviceModule() const { return m_deviceModule; }

        friend class OptoModuleStorage;
    };


    class OptoModuleStorage : public QObject
    {
        Q_OBJECT

    private:

        EquipmentSet* m_equipmentSet = nullptr;
        OutputLog* m_log = nullptr;

        HashedVector<QString, OptoModule*> m_modules;
        HashedVector<QString, OptoPort*> m_ports;

        bool addModule(DeviceModule* module);

        void clear();

    public:
        OptoModuleStorage(EquipmentSet* equipmentSet, OutputLog* log);
        ~OptoModuleStorage();

        bool build();

        OptoModule* getOptoModule(const QString& optoModuleStrID);
        OptoPort* getOptoPort(const QString& optoPortStrID);
    };
}
