#include "Calibrator.h"

#include <assert.h>
#include <QSettings>

// -------------------------------------------------------------------------------------------------------------------

Calibrator::Calibrator(QObject *parent) :
    QObject(parent),
    m_port(this)
{
    empty();
}

// -------------------------------------------------------------------------------------------------------------------

Calibrator::~Calibrator()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::empty()
{
    if (m_port.isOpen() == true)
    {
        m_port.clear();
        m_port.close();
    }

    m_connected = false;

    m_name = "";
    m_serialNo = "";

    m_timeout = 0;

    m_mode = CALIBRATOR_MODE_UNKNOWN;
    m_measureUnit = CALIBRATOR_UNIT_UNKNOWN;
    m_sourceUnit = CALIBRATOR_UNIT_UNKNOWN;

    m_measureValue = 0;
    m_sourceValue = 0;

    m_lastResponse = "";
    m_lastError = "";
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::setConnected(bool connect)
{
    m_connected = connect;

    if (connect == true)
    {
        emit connected();
    }
    else
    {
        emit disconnected();
    }

}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::open()
{
    empty();

    if (m_portName.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Error description: Don't defined port name").arg(__FUNCTION__);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (openPort() == false)
    {
        return false;
    }

    if (getIDN() == false)
    {
        return false;
    }

    setConnected(true);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::getIDN()
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    setRemoteControl(true);

    send(CALIBRATOR_IDN);
    if (recv() == false)
    {
        return false;
    }

    // Extracts from the string of the last response from the calibrator name and serial number
    //
    int begPos = 0, endPos = 0;

    begPos = m_lastResponse.indexOf(',', 0);
    endPos = m_lastResponse.indexOf(',', begPos+1);

    // Calibrator name
    //
    m_name = m_lastResponse.left(endPos);
    m_name = m_name.right(endPos - begPos - 1);

    if (m_name.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration name").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    begPos = endPos;
    endPos = m_lastResponse.indexOf(',', begPos+1);

    // Calibrator serial number
    //
    m_serialNo = m_lastResponse.left(endPos);
    m_serialNo = m_serialNo.right(endPos - begPos -1);

    if (m_serialNo.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration serial number").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::openPort()
{
    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    m_port.setPortName(m_portName);

    m_port.setBaudRate( QSerialPort::Baud9600 );

    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);

    if (m_port.open(QIODevice::ReadWrite) == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: %3 (%4)").arg(__FUNCTION__).arg(m_portName).arg(m_port.errorString()).arg(m_port.error());
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    setWaitResponse(true);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::send(QString cmd)
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (cmd.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty command").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    // add ending
    //
    switch(m_type)
    {
        case CALIBRATOR_TYPE_TRXII:     cmd.append("\n\r"); break;
        case CALIBRATOR_TYPE_CALYS75:	cmd.append("\r\r"); break;
        default:                        assert(false);      break;
    }

    QByteArray cmdData = cmd.toLocal8Bit();;

    // send
    //
    qint64 writtenBytes = m_port.write(cmdData);
    if (writtenBytes != cmdData.count())
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Command is sent is not fully").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    emit commandIsSent(cmd);

    while(m_port.waitForBytesWritten((CALIBRATOR_TIMEOUT_STEP)));

    //qDebug("Serial Port: " + m_portName.toLocal8Bit() + " send: "+ cmd.toLocal8Bit());

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::recv()
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    m_lastResponse = "";

    QByteArray requestData;

    m_timeout = 0;
    while(m_timeout < CALIBRATOR_TIMEOUT)
    {
        if (m_enableWaitResponse == false)
        {
            m_timeout = CALIBRATOR_TIMEOUT;
            break;
        }

        while (m_port.waitForReadyRead(CALIBRATOR_TIMEOUT_STEP))
        {
            requestData += m_port.readAll();
        }

        if (requestData.isEmpty() == false)
        {
            if (requestData[requestData.count() - 1] == '\r')
            {
                break;
            }
        }

        m_timeout += CALIBRATOR_TIMEOUT_STEP;
    }

    setWaitResponse(true);

    if (m_timeout == CALIBRATOR_TIMEOUT && requestData.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Calibrator don't sent a response").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);

        close();

        return false;
    }

    m_lastResponse = requestData;
    m_lastResponse.remove("\n\r");

    qDebug("Function: %s, Serial Port: " + m_portName.toLocal8Bit() + ", response: " + m_lastResponse.toLocal8Bit(), __FUNCTION__);

    emit responseIsReceived(m_lastResponse);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::setUnit(int mode, int unit)
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (mode < 0 || mode >= CALIBRATOR_MODE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined mode").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit()); emit error( m_lastError);
        return false;
    }

    m_mode = mode;

    if (unit < 0 || unit >= CALIBRATOR_UNIT_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined unit").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    QString cmdUnit, cmdRange;

    switch(m_type)
    {
        case CALIBRATOR_TYPE_TRXII:

            switch(m_mode)
            {
                case CALIBRATOR_MODE_MEASURE:

                    switch(unit)
                    {
                        case CALIBRATOR_UNIT_MV:		cmdUnit = TRXII_MEASURE_UNIT_MV;		break;
                        case CALIBRATOR_UNIT_MA:		cmdUnit = TRXII_MEASURE_UNIT_MA;		break;
                        case CALIBRATOR_UNIT_V:         cmdUnit = TRXII_MEASURE_UNIT_V;			break;
                        case CALIBRATOR_UNIT_KHZ:		cmdUnit = TRXII_MEASURE_UNIT_KHZ;		break;
                        case CALIBRATOR_UNIT_LOW_OHM:	cmdUnit = TRXII_MEASURE_UNIT_OHM;		break;
                        case CALIBRATOR_UNIT_HIGH_OHM:	cmdUnit = TRXII_MEASURE_UNIT_OHM;		break;
                        default:                        assert(false);                          break;
                    }

                    break;

                case CALIBRATOR_MODE_SOURCE:

                    switch(unit)
                    {
                        case CALIBRATOR_UNIT_MV:		cmdUnit = TRXII_SOURCE_UNIT_MV;			break;
                        case CALIBRATOR_UNIT_MA:		cmdUnit = TRXII_SOURCE_UNIT_MA;			break;
                        case CALIBRATOR_UNIT_V:         cmdUnit = TRXII_SOURCE_UNIT_V;			break;
                        case CALIBRATOR_UNIT_KHZ:		cmdUnit = TRXII_SOURCE_UNIT_KHZ;		break;
                        case CALIBRATOR_UNIT_LOW_OHM:	cmdUnit = TRXII_SOURCE_UNIT_LOW_OHM;	break;
                        case CALIBRATOR_UNIT_HIGH_OHM:	cmdUnit = TRXII_SOURCE_UNIT_HIGH_OHM;	break;
                        default:                        assert(false);                          break;
                    }

                    break;

                default:
                    assert(false);
                    break;
            }

            send(cmdUnit);

            break;

        case CALIBRATOR_TYPE_CALYS75:

            switch(m_mode)
            {
                case CALIBRATOR_MODE_MEASURE:

                    switch(unit)
                    {
                        case CALIBRATOR_UNIT_MV:		cmdUnit = CALYS75_MEASURE_UNIT_MV;	cmdRange = CALYS75_MEASURE_RANG_MV;			break;
                        case CALIBRATOR_UNIT_MA:		cmdUnit = CALYS75_MEASURE_UNIT_MA;	cmdRange = CALYS75_MEASURE_RANG_MA;			break;
                        case CALIBRATOR_UNIT_V:         cmdUnit = CALYS75_MEASURE_UNIT_V;	cmdRange = CALYS75_MEASURE_RANG_V;			break;
                        case CALIBRATOR_UNIT_KHZ:		cmdUnit = CALYS75_MEASURE_UNIT_KHZ;	cmdRange = CALYS75_MEASURE_RANG_KHZ;		break;
                        case CALIBRATOR_UNIT_LOW_OHM:	cmdUnit = CALYS75_MEASURE_UNIT_OHM;	cmdRange = CALYS75_MEASURE_LOW_RANG_OHM;	break;
                        case CALIBRATOR_UNIT_HIGH_OHM:	cmdUnit = CALYS75_MEASURE_UNIT_OHM;	cmdRange = CALYS75_MEASURE_HIGH_RANG_OHM;	break;
                        default:                        assert(false);                                                                  break;
                    }

                    break;

                case CALIBRATOR_MODE_SOURCE:

                    switch(unit)
                    {
                        case CALIBRATOR_UNIT_MV:		cmdUnit = CALYS75_SOURCE_UNIT_MV;	cmdRange = CALYS75_SOURCE_RANG_MV;			break;
                        case CALIBRATOR_UNIT_MA:		cmdUnit = CALYS75_SOURCE_UNIT_MA;	cmdRange = CALYS75_SOURCE_RANG_MA;			break;
                        case CALIBRATOR_UNIT_V:         cmdUnit = CALYS75_SOURCE_UNIT_V;	cmdRange = CALYS75_SOURCE_RANG_V;			break;
                        case CALIBRATOR_UNIT_KHZ:		cmdUnit = CALYS75_SOURCE_UNIT_KHZ;	cmdRange = CALYS75_SOURCE_RANG_KHZ;			break;
                        case CALIBRATOR_UNIT_LOW_OHM:	cmdUnit = CALYS75_SOURCE_UNIT_OM;	cmdRange = CALYS75_SOURCE_LOW_RANG_OHM;		break;
                        case CALIBRATOR_UNIT_HIGH_OHM:	cmdUnit = CALYS75_SOURCE_UNIT_OM;	cmdRange = CALYS75_SOURCE_HIGH_RANG_OHM;	break;
                        default:                        assert(false);                                                                  break;
                    }

                    break;

                default:
                    assert(false);
                    break;
            }

            send(cmdUnit);
            send(cmdRange);

            break;

        default:
            assert(false);
            break;
    }

    switch(m_mode)
    {
        case CALIBRATOR_MODE_MEASURE:	m_measureUnit = unit;	break;
        case CALIBRATOR_MODE_SOURCE:	m_sourceUnit = unit;	break;
        default:                        assert(false);          break;
    }

    qDebug("Function: %s, Serial port: " + m_portName.toLocal8Bit() + ", Mode: %s, Unit: %s", __FUNCTION__, CalibratorMode[mode], CalibratorUnit[unit]);

    emit unitIsChanged();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::setValue(double value)
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_mode < 0 || m_mode >= CALIBRATOR_MODE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined mode").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined unit").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }


    convert(value, CALIBRATOR_MODE_SOURCE, CALIBRATOR_CONVERT_KHZ_TO_HZ);

    QString cmdSetValue;

    switch(m_type)
    {
        case CALIBRATOR_TYPE_TRXII:     cmdSetValue = QString("%1%2").arg(TRXII_SET_VALUE, QString::number(value, 10, 3));      break;
        case CALIBRATOR_TYPE_CALYS75:   cmdSetValue = QString("%1%2").arg(CALYS75_SET_VALUE, QString::number(value, 10, 5));    break;
        default:                        assert(false);                                                                          break;
    }

    if (cmdSetValue.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty command").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    send(cmdSetValue);

    getValue();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::stepDown()
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_mode < 0 || m_mode >= CALIBRATOR_MODE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined mode").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined unit").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    switch(m_sourceUnit)
    {
        case CALIBRATOR_UNIT_LOW_OHM:	m_sourceValue -= 0.01;	break;
        case CALIBRATOR_UNIT_HIGH_OHM:	m_sourceValue -= 0.1;	break;
        default:						m_sourceValue -= 0.001;	break;
    }

    convert(m_sourceValue, CALIBRATOR_MODE_SOURCE, CALIBRATOR_CONVERT_KHZ_TO_HZ);

    QString cmdKeyDown;

    switch(m_type)
    {
        case CALIBRATOR_TYPE_TRXII:     cmdKeyDown = QString("%1%2").arg(TRXII_SET_VALUE,QString::number(m_sourceValue, 10, 3));    break;
        case CALIBRATOR_TYPE_CALYS75:   cmdKeyDown = QString("%1%2").arg(CALYS75_SET_VALUE,QString::number(m_sourceValue, 10, 5));  break;
        default:                        assert(false);                                                                              break;
    }

    if (cmdKeyDown.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty command").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    send(cmdKeyDown);

    getValue();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::stepUp()
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_mode < 0 || m_mode >= CALIBRATOR_MODE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined mode").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_sourceUnit < 0 || m_sourceUnit >= CALIBRATOR_UNIT_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined unit").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    switch(m_sourceUnit)
    {
        case CALIBRATOR_UNIT_LOW_OHM:	m_sourceValue += 0.01;	break;
        case CALIBRATOR_UNIT_HIGH_OHM:	m_sourceValue += 0.1;	break;
        default:						m_sourceValue += 0.001;	break;
    }

    convert(m_sourceValue, CALIBRATOR_MODE_SOURCE, CALIBRATOR_CONVERT_KHZ_TO_HZ);

    QString cmdKeyUp;

    switch(m_type)
    {
        case CALIBRATOR_TYPE_TRXII:     cmdKeyUp = QString("%1%2").arg(TRXII_SET_VALUE,QString::number(m_sourceValue, 10, 3));      break;
        case CALIBRATOR_TYPE_CALYS75:   cmdKeyUp = QString("%1%2").arg(CALYS75_SET_VALUE,QString::number(m_sourceValue, 10, 5));    break;
        default:                        assert(false);                                                                              break;
    }

    if (cmdKeyUp.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty command").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    send(cmdKeyUp);

    getValue();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::step(int stepType)
{
    if (stepType < 0 || stepType >= CALIBRATOR_STEP_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined step type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    bool stepResult = false;

    switch(stepType)
    {
        case CALIBRATOR_STEP_DOWN:  stepResult = stepDown();    break;
        case CALIBRATOR_STEP_UP:    stepResult = stepUp();      break;
        default:                    assert(false);              break;
    }

    return stepResult;
}


// -------------------------------------------------------------------------------------------------------------------

double Calibrator::getValue()
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return 0;
    }

    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return 0;
    }

    QString cmdGetValue;

    switch(m_type)
    {
        case CALIBRATOR_TYPE_TRXII:     cmdGetValue = TRXII_GET_VALUE;		break;
        case CALIBRATOR_TYPE_CALYS75:   cmdGetValue = CALYS75_GET_VALUE;    break;
        default:                        assert(0);                          break;
    }

    if (cmdGetValue.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty command").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return 0;
    }

    setBusy(true);

        send(cmdGetValue);

        recv();
        parseResponse();

    setBusy(false);

    return m_sourceValue;
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::setBusy(bool busy)
{
    m_busy = busy;

    if (busy == true)
    {
        emit valueIsRequested();
    }
    else
    {
        emit valueIsReceived();
    }
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::beep()
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    send(CALIBRATOR_BEEP);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::reset(int resetType)
{
    if (m_port.isOpen() == false)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port is not open").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (resetType < 0 || resetType >= CALIBRATOR_RESET_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined reset type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }


    QString cmd;

    switch(resetType)
    {
        case CALIBRATOR_RESET_HARD:

            cmd = CALIBRATOR_RESET;

            break;

        case CALIBRATOR_RESET_SOFT:

            if (m_type == CALIBRATOR_TYPE_TRXII)
            {
                cmd = TRXII_RESET_SOFT;
            }

            break;

        default:
            assert(false);
            break;
    }

    if (cmd.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty command").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    send(cmd);
    recv();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Calibrator::setRemoteControl(bool enable)
{
    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return false;
    }

    if (m_type == CALIBRATOR_TYPE_CALYS75)
    {
        if (enable == true)
        {
            send(CALYS75_REMOTE_CONTROL);
        }
        else
        {
            send(CALYS75_MANUAL_CONTROL);
        }
    }

    return true;
}


// -------------------------------------------------------------------------------------------------------------------

void Calibrator::parseResponse()
{
    if (m_type < 0 || m_type >= CALIBRATOR_TYPE_COUNT)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Don't defined calibration type").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);
        return;
    }

    if (m_lastResponse.isEmpty() == true)
    {
        m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Empty response").arg(__FUNCTION__).arg(m_portName);
        qDebug(m_lastError.toLocal8Bit());
        emit error( m_lastError);

        m_measureValue = 0;
        m_sourceValue = 0;

        return;
    }


    QString value, valueMeasure;
    int begPos, endPos;

    // Extracts from the string of the last response from the calibrator current electrical values
    //
    switch(m_type)
    {
    case CALIBRATOR_TYPE_TRXII:

        value = m_lastResponse;
        value.remove(' ');

        begPos = value.indexOf(',');
        if (begPos == -1)
        {
            break;
        }

        valueMeasure = value.left(begPos);
        value.remove(0, begPos + 1);

        m_measureValue = valueMeasure.toDouble();
        m_sourceValue = value.toDouble();

        break;

    case CALIBRATOR_TYPE_CALYS75:

        value = m_lastResponse;

        begPos = value.indexOf("<MEAS>", 0);
        if (begPos == -1)
        {
            break;
        }

        value.remove(0, begPos + 6);

        endPos = value.indexOf("</MEAS>", 0);
        if (endPos == -1)
        {
            break;
        }

        value.remove(endPos, value.count());

        m_measureValue = value.toDouble();

        value = m_lastResponse;

        begPos = value.indexOf("<MEAS>", begPos + 1);
        if (begPos == -1)
        {
            break;
        }

        value.remove(0, begPos + 6);

        endPos = value.indexOf("</MEAS>", 0);
        if (endPos == -1)
        {
            break;
        }

        value.remove(endPos, value.count());

        m_sourceValue = value.toDouble();

        break;

    default:
        assert(0);
        break;
    }

    convert(m_measureValue,	CALIBRATOR_MODE_MEASURE,	CALIBRATOR_CONVERT_HZ_TO_KHZ);
    convert(m_sourceValue,	CALIBRATOR_MODE_SOURCE	,	CALIBRATOR_CONVERT_HZ_TO_KHZ);
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::convert(double& val, int mode, int order)
{
    bool enableCorrect = false;

    switch(mode)
    {
        case CALIBRATOR_MODE_MEASURE:

            if (m_measureUnit	== CALIBRATOR_UNIT_KHZ)
            {
                enableCorrect = true;
            }

            break;

        case CALIBRATOR_MODE_SOURCE:

            if (m_type == CALIBRATOR_TYPE_TRXII)
            {
                if (m_sourceUnit == CALIBRATOR_UNIT_KHZ)
                {
                    enableCorrect = true;
                }
            }

            break;

        default:
            assert(0);
            break;
;
    }

    if (enableCorrect == false)
    {
        return;
    }

    switch(order)
    {
        case CALIBRATOR_CONVERT_HZ_TO_KHZ:	val /=  1000;	break;
        case CALIBRATOR_CONVERT_KHZ_TO_HZ:	val *=  1000;	break;
        default:                            assert(0);      break;
;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void Calibrator::close()
{
    setWaitResponse(false);

    if (m_port.isOpen() == true)
    {
        setRemoteControl(false);
    }

    empty();

    setConnected(false);
}

// -------------------------------------------------------------------------------------------------------------------


