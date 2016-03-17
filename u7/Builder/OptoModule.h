#pragma once

#include <QObject>
#include "../include/OrderedHash.h"

namespace Hardware
{
    class DeviceModule;

    class OptoPort : public QObject
    {
        Q_OBJECT

    public:
        enum class SerialMode
        {
            RS232,
            RS485
        };
        Q_ENUM(SerialMode)

        enum class Mode
        {
            Optical,
            Serial
        };
        Q_ENUM(Mode)

    private:
        int m_rxStartAddress = 0;
        int m_rxSizeW = 0;

        int m_txStartAddress = 0;
        int m_txSizeW = 0;

        bool enable = true;             // serial mode only
        bool enableDuplex = false;      // serial mode and OCMN only
    };


    // Class represent modules with opto-ports - LM or OCM
    //
    class OptoModule
    {
    private:
        HashedVector<QString, OptoPort> m_ports;

    public:
    };


    class OptoModuleStorage
    {
    private:
        HashedVector<QString, OptoModule> m_modules;

    public:
        void AddModule(const DeviceModule& module);
    };
}
