#pragma once

#include <QObject>

namespace Hardware
{
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
        int m_rxSizeW = 0;
        int m_txSizeW = 0;
        bool enable = true;             // serial mode only
        bool enableDuplex = false;      // serial mode and OCMN only
    };


    // Class represent modules with opto-ports - LM or OCM
    //
    class OptoModule
    {
    public:
    };


    class OptoModuleStorage
    {
    private:

    public:
        //void AddOptoModule(const QString& optoModuleStrID);
    };
}
