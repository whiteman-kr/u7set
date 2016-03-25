#pragma once

#include <QObject>

#include "../include/OrderedHash.h"
#include "../include/DeviceObject.h"
#include "../include/OutputLog.h"
#include "../include/OrderedHash.h"
#include "../include/Address16.h"
#include "../include/Signal.h"


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

        struct TxSignal
        {
            QString strID;
            Address16 address;        // offset from beginning of txBuffer
            int sizeBit = 0;
        };

    private:
        QString m_strID;
        DeviceController* m_deviceController = nullptr;

        QString m_optoModuleStrID;

        QString m_linkedPortStrID;

        QString m_connectionCaption;

        QStringList m_txSignalsStrIDList;

        QList<TxSignal> m_txAnalogSignalList;
        QList<TxSignal> m_txDiscreteSignalList;

        int m_txAnalogSignalsSizeW = 0;     // variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
        int m_txDiscreteSignalsSizeW = 0;   //
        quint32 m_txDataID = 0;             // range 0..0xFFFFFFFF
        int m_txDataSizeW = 0;                  //

        int m_port = 0;

        Mode m_mode = Mode::Optical;
        SerialMode m_serialMode = SerialMode::RS232;

        int m_rxStartAddress = 0;
        int m_rxDataSizeW = 0;

        int m_txStartAddress = 0;

        quint16 m_portID = 0;           // range 0..999, 0 - not linked port ID, 1..999 - linked port ID

        bool m_enable = true;           // serial mode only
        bool m_enableDuplex = false;    // serial mode and OCMN only

    public:
        OptoPort(const QString& optoModuleStrID, DeviceController* optoPortController, int port);

        quint16 portID() const { return m_portID; }
        void setPortID(quint16 portID) { m_portID = portID; }

        quint32 txDataID() const { return m_txDataID; }

        QString strID() const { return m_strID; }

        QString linkedPortStrID() const { return m_linkedPortStrID; }
        void setLinkedPortStrID(const QString& linkedPortStrID) { m_linkedPortStrID = linkedPortStrID; }

        QString connectionCaption() const { return m_connectionCaption; }
        void setConnectionCaption(const QString& connectionCaption) { m_connectionCaption = connectionCaption; }

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

        QString optoModuleStrID() const { return m_optoModuleStrID; }

        void addTxSignalStrID(const QString& signalStrID);
        void addTxSignalsStrID(const QStringList& signalStrIDList);

        QStringList getTxSignalsStrID() const { return m_txSignalsStrIDList; }

        void addTxSignal(Signal* txSignal);
        void calculateTxSignalsAddresses();

        QList<TxSignal> txAnalogSignals() const { return m_txAnalogSignalList; }
        QList<TxSignal> txDiscreteSignals() const { return m_txDiscreteSignalList; }

        int txAnalogSignalsSizeW() const { return m_txAnalogSignalsSizeW; }
        int txAnalogSignalsCount() const { return m_txAnalogSignalList.count(); }

        int txDiscreteSignalsSizeW() const { return m_txDiscreteSignalsSizeW; }
        int txDiscreteSignalsCount() const { return m_txDiscreteSignalList.count(); }

        int txDataSizeW() const { return m_txDataSizeW; }

        int rxDataSizeW() const { return m_rxDataSizeW; }
        void setRxDataSizeW(int rxDataSizeW) { m_rxDataSizeW = rxDataSizeW; }
    };


    // Class represent modules with opto-ports - LM or OCM
    //
    class OptoModule : public QObject
    {
        Q_OBJECT

    private:
        QString m_strID;
        DeviceModule* m_deviceModule = nullptr;

        QString m_lmStrID;
        DeviceModule* m_lmDeviceModule = nullptr;

        HashedVector<QString, OptoPort*> m_ports;
        OutputLog* m_log = nullptr;

        bool m_valid = false;

        // device properties
        //
        int m_optoInterfaceDataOffset = 0;
        int m_optoPortDataSize = 0;
        int m_optoPortAppDataOffset = 0;
        int m_optoPortAppDataSize = 0;
        int m_optoPortCount = 0;

    public:
        OptoModule(DeviceModule* module, OutputLog* log);
        ~OptoModule();

        bool isValid() const { return m_valid; }

        bool isLM();
        bool isOCM();

        QString strID() const { return m_strID; }
        const DeviceModule* deviceModule() const { return m_deviceModule; }

        QString lmStrID() const { return m_lmStrID; }
        const DeviceModule* lmDeviceModule() const { return m_lmDeviceModule; }

        QList<OptoPort*> getRS232Ports();

        QList<OptoPort*> ports();

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

        QHash<QString, OptoModule*> m_lmAssociatedModules;

        QList<OptoModule*> modules();
        QList<OptoPort*> ports();

        bool addModule(DeviceModule* module);

        void clear();

    public:
        OptoModuleStorage(EquipmentSet* equipmentSet, OutputLog* log);
        ~OptoModuleStorage();

        bool build();

        OptoModule* getOptoModule(const QString& optoModuleStrID);
        OptoModule* getOptoModule(const OptoPort* optoPort);
        OptoPort* getOptoPort(const QString& optoPortStrID);

        bool isCompatiblePorts(const OptoPort* optoPort1, const OptoPort* optoPort2);

        QList<OptoModule*> getLmAssociatedOptoModules(const QString& lmStrID);

        bool setPortsRxDataSizes();
        bool calculatePortsTxRxStartAddresses();
    };
}
