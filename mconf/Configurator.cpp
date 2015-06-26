#include "Stable.h"
#include "Configurator.h"
#include "../include/Crc.h"

#ifdef Q_OS_WIN32
#include "./ftdi/ftd2xx.h"
#endif


//
// Uuid
//

QUuid Uuid::toQUuid() const
{
	QUuid u;
	u.data1 = data1;
	u.data2 = data2;
	u.data3 = data3;
	u.data4[0] = data4[0];
	u.data4[1] = data4[1];
	u.data4[2] = data4[2];
	u.data4[3] = data4[3];
	u.data4[4] = data4[4];
	u.data4[5] = data4[5];
	u.data4[6] = data4[6];
	u.data4[7] = data4[7];
	return u;
}

//
// CONF_HEADER_V1
//
void CONF_HEADER_V1::dump(OutputLog &log)
{
    log.writeMessage("version: " + QString().setNum(version, 16).rightJustified(4, '0'));
    log.writeMessage("moduleUartId: " + QString().setNum(moduleUartId, 16).rightJustified(4, '0'));
    log.writeMessage("opcode: " + QString().setNum(opcode, 16).rightJustified(4, '0'));
    log.writeMessage("flags: " + QString().setNum(flags, 16).rightJustified(4, '0'));
    log.writeMessage("frameIndex: " + QString().setNum(frameIndex, 16).rightJustified(4, '0'));
    log.writeMessage("frameSize: " + QString().setNum(frameSize, 16).rightJustified(4, '0'));
    log.writeMessage("blockSize: " + QString().setNum(blockSize, 16).rightJustified(4, '0'));
    log.writeMessage("romSize: " + QString().setNum(romSize, 16).rightJustified(8, '0'));
    log.writeMessage("crc64: " + QString().setNum(crc64, 16).rightJustified(16, '0'));

	return;
}

void CONF_HEADER_V1::dumpFlagsState(OutputLog& log)
{
	if (flags & OpDeniedCalibrationIsActive)
	{
        log.writeError("Flags: Operation denied, calibration is active.");
	}

	if (flags & OpDeniedInvalidModuleUartId)
	{
        log.writeError("Flags: Operation denied, invalid ModeleID.");
	}

	if (flags & OpDeniedInvalidOpcode)
	{
        log.writeError("Flags: Invalid opcode.");
	}
	
	if (flags & OpDeniedInvalidFrameIndex)
	{
        log.writeError("Flags: Operation denied, invalid frame index.");
	}

	if (flags & OpDeniedInvalidCrc)
	{
        log.writeError("Flags: Operation denied, invalid header CRC.");
	}

	if (flags & OpDeniedEepromHwError)
	{
        log.writeError("Flags: EEPROM HW error.");
	}

	if (flags & OpDeniedInvalidEepromCrc)
	{
        log.writeError("Flags: Operation denied, invalid data CRC.");
	}

	return;
}

void CONF_HEADER_V1::setCrc()
{
	// !!! ATTENTION !!!
	// HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN
	//
	quint64 le_crc64 = Crc::crc64(this, sizeof(CONF_HEADER) - sizeof(uint64_t));
	crc64 = qToBigEndian(le_crc64);

	return;
}

bool CONF_HEADER_V1::checkCrc()
{
	// !!! ATTENTION !!!
	// HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN
	//
	quint64 le_crc64 = Crc::crc64(this, sizeof(CONF_HEADER) - sizeof(uint64_t));
	return crc64 == qToBigEndian(le_crc64);
}

bool CONF_HEADER_V1::flagStateSuccess()
{
	return flags == 0;
}

//
// CONF_SERVICE_DATA_V1
//
uint16_t CONF_SERVICE_DATA_V1::diagVersion() const
{
	return m_diagVersion;
}
void CONF_SERVICE_DATA_V1::setDiagVersion(uint16_t value)
{
	m_diagVersion = value;
}

uint32_t CONF_SERVICE_DATA_V1::factoryNo() const
{
	return qFromBigEndian(m_factoryNo);
}
void CONF_SERVICE_DATA_V1::setFactoryNo(uint32_t value)
{
	m_factoryNo = qToBigEndian(value);
}

uint16_t CONF_SERVICE_DATA_V1::manufactureYear() const
{
	return qFromBigEndian(m_manufactureYear);
}
void CONF_SERVICE_DATA_V1::setManufactureYear(uint16_t value)
{
	m_manufactureYear = qToBigEndian(value);
}

uint16_t CONF_SERVICE_DATA_V1::manufactureMonth() const
{
	return qFromBigEndian(m_manufactureMonth);
}
void CONF_SERVICE_DATA_V1::setManufactureMonth(uint16_t value)
{
	m_manufactureMonth = qToBigEndian(value);
}

uint16_t CONF_SERVICE_DATA_V1::manufactureDay() const
{
	return qFromBigEndian(m_manufactureDay);
}
void CONF_SERVICE_DATA_V1::setManufactureDay(uint16_t value)
{
	m_manufactureDay = qToBigEndian(value);
}

uint16_t CONF_SERVICE_DATA_V1::configureYear() const
{
	return qFromBigEndian(m_configureYear);
}
void CONF_SERVICE_DATA_V1::setConfigureYear(uint16_t value)
{
	m_configureYear = qToBigEndian(value);
}

uint16_t CONF_SERVICE_DATA_V1::configureMonth() const
{
	return qFromBigEndian(m_configureMonth);
}
void CONF_SERVICE_DATA_V1::setConfigureMonth(uint16_t value)
{
	m_configureMonth = qToBigEndian(value);
}

uint16_t CONF_SERVICE_DATA_V1::configureDay() const
{
	return qFromBigEndian(m_configureDay);
}
void CONF_SERVICE_DATA_V1::setConfigureDay(uint16_t value)
{
	m_configureDay = qToBigEndian(value);
}

uint32_t CONF_SERVICE_DATA_V1::firmwareCrc1() const
{
	return qFromBigEndian(m_firmwareCrc1);
}
void CONF_SERVICE_DATA_V1::setFirmwareCrc1(uint32_t value)
{
	m_firmwareCrc1 = qToBigEndian(value);
}

uint32_t CONF_SERVICE_DATA_V1::firmwareCrc2() const
{
	return qFromBigEndian(m_firmwareCrc2);
}
void CONF_SERVICE_DATA_V1::setFirmwareCrc2(uint32_t value)
{
	m_firmwareCrc2 = qToBigEndian(value);
}

//
// CONF_IDENTIFICATION_DATA_V1
//
void CONF_IDENTIFICATION_DATA_V1::dump(OutputLog& log)
{
    log.writeMessage("BlockId: " + moduleUuid.toQUuid().toString());
    log.writeMessage("Configuration counter: " + QString().setNum(count));
			
    log.writeMessage("First time configured: ");
    log.writeMessage("__Date: " + QDateTime().fromTime_t(firstConfiguration.date).toString());
    log.writeMessage("__Host: " + QString(firstConfiguration.host));
    log.writeMessage("__ConfigurationId: " + firstConfiguration.configurationId.toQUuid().toString());
    log.writeMessage("__Configurator factory no: " + QString().setNum(firstConfiguration.configuratorFactoryNo));

    log.writeMessage("Last time configured: ");
    log.writeMessage("__Date: " + QDateTime().fromTime_t(lastConfiguration.date).toString());
    log.writeMessage("__Host: " + QString(lastConfiguration.host));
    log.writeMessage("__ConfigurationId: " + lastConfiguration.configurationId.toQUuid().toString());
    log.writeMessage("__Configurator factory no: " + QString().setNum(lastConfiguration.configuratorFactoryNo));

	return;
}

//
// Configurator
//
Configurator::Configurator(QString serialDevide, QObject* parent)
	: QObject(parent),
    m_device(serialDevide),
    m_serialPort(nullptr)
{
}

Configurator::~Configurator()
{
}

QString Configurator::device() const
{
	QMutexLocker ml(&mutex);
	return m_device;
}

void Configurator::setDevice(const QString& device)
{
	QMutexLocker ml(&mutex);
	m_device = device;
	return;
}

bool Configurator::showDebugInfo() const
{
	return m_showDebugInfo;
}

void Configurator::setShowDebugInfo(bool showDebugInfo)
{
	m_showDebugInfo = showDebugInfo;
}

bool Configurator::openConnection()
{
	m_configuratorfactoryNo = 0;

	// Check configurator serial no
	//
#ifdef Q_OS_UNIX
	theLog.writeMessage("The Linux version does not support configurator Factory No (FTDI programmer factory number).");
	theLog.writeMessage("You can still continue using configurator, but a Factory No will not be written to the configured module.");
#endif

	// Read programmers factory no
	//
#ifdef Q_OS_WIN32
    DWORD DeviceCount = 0;
	FT_STATUS Result = FT_CreateDeviceInfoList(&DeviceCount);
	if (Result != FT_OK)
	{
        theLog.writeError(__FUNCTION__ + tr(" FT_CreateDeviceInfoList error."));
        return false;
	}

	if (DeviceCount == 0)
	{
        theLog.writeError(__FUNCTION__ + tr(" Can't find any configurator."));
        return false;
	}

	if (DeviceCount != 1)
	{
        theLog.writeError(__FUNCTION__ + tr(" There are more than one configurator, please leave only one."));
        return false;
	}

	FT_HANDLE ftHandle;

	if (FT_Open(0, &ftHandle) != FT_OK)
	{
        theLog.writeError(__FUNCTION__ + tr(" FT_Open error."));
        return false;
	}


	FT_PROGRAM_DATA ftData = FT_PROGRAM_DATA();
	char ManufacturerBuf[32];
	char ManufacturerIdBuf[16];
	char DescriptionBuf[64];
	char SerialNumberBuf[16];

	ftData.Signature1 = 0x00000000;
	ftData.Signature2 = 0xffffffff;
	ftData.Version = 0x00000005;					// EEPROM structure with FT232H extensions
	ftData.Manufacturer = ManufacturerBuf;
	ftData.ManufacturerId = ManufacturerIdBuf;
	ftData.Description = DescriptionBuf;
	ftData.SerialNumber = SerialNumberBuf;

	if (FT_EE_Read(ftHandle, &ftData) == FT_OK)
	{
		QString serialNo = SerialNumberBuf;

		bool converted = false;
		int sn = serialNo.toUInt(&converted);

		if (converted == false || sn == 0)
		{
            theLog.writeError(__FUNCTION__ + tr(" Wrong configuration factory no(") + SerialNumberBuf + ")");
            FT_Close(ftHandle);
            return false;
		}

		m_configuratorfactoryNo = sn;
        theLog.writeMessage(tr("Configurator factory no:") + QString().setNum(m_configuratorfactoryNo));
	}
	else
	{
        theLog.writeError(__FUNCTION__ + tr(" FT_Read error."));
        return false;
	}

    FT_Close(ftHandle);
#endif

    if (m_serialPort != nullptr)
    {
        Q_ASSERT(false);
        delete m_serialPort;
        m_serialPort = nullptr;
    }

	// Open port
	//
    m_serialPort = new QSerialPort();
    m_serialPort->setPortName(device());

    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);

	//!!!!!!!!!!Cannot create children for a parent that is in a different thread.

    bool ok = m_serialPort->open(QIODevice::ReadWrite);


	if (ok == false)
	{
        theLog.writeError(__FUNCTION__ + tr(" %1").arg(m_serialPort->errorString()));

#ifdef Q_OS_LINUX
        if (serialPort->error() == QSerialPort::PermissionError)
		{
			theLog.writeMessage(tr("Add user to the dialout group by the following command:"));
			theLog.writeMessage(tr("sudo usermod -a -G dialout USER"));
			theLog.writeMessage(tr("Then user has to logout and log in back."));
		}
#endif

        delete m_serialPort;
        m_serialPort = nullptr;

		return false;
	}

    return true;
}

bool Configurator::closeConnection()
{
	m_configuratorfactoryNo = 0;

    if (m_serialPort == nullptr)
    {
        Q_ASSERT(m_serialPort);
        return false;
    }

    if (m_serialPort->isOpen() == false)
	{
        theLog.writeError("CloseConnection error: The port was already closed.");
		return false;
	}

    m_serialPort->close();

    delete m_serialPort;
    m_serialPort = nullptr;

	return true;
}

bool Configurator::send(int moduleUartId,
	ConfigureCommand opcode,
	uint16_t frameIndex,
	uint16_t blockSize,
    const std::vector<quint8>& requestData,
	CONF_HEADER* pReceivedHeader,
    std::vector<quint8>* replyData)
{
    if (m_serialPort == nullptr)
    {
        Q_ASSERT(false);
        theLog.writeError(tr("Port object is not created: %1").arg(device()));
        return false;
    }

    if (m_serialPort->isOpen() == false)
	{
        theLog.writeError(tr("Port is not opened: %1").arg(device()));
		return false;
	}

	if (replyData == nullptr || pReceivedHeader == nullptr)
	{
		assert(replyData != nullptr);
		assert(pReceivedHeader != nullptr);
		return false;
	}

	// Generate packet
	//
    std::vector<quint8> buffer;				//
	int expecetedDataBytes = 0;				// Expecting reply in expecetedDataBytes to read from device + sizeof(CONF_HEADER)
	int headerSize = 0;

	switch (opcode)
	{
    case Read:
        {
            CONF_HEADER readHeader = CONF_HEADER();

            readHeader.version = static_cast<uint16_t>(ProtocolMaxVersion);
            readHeader.moduleUartId = static_cast<uint16_t>(moduleUartId);
            readHeader.opcode = static_cast<uint16_t>(opcode);
            readHeader.frameIndex = static_cast<uint16_t>(frameIndex);
            readHeader.setCrc();			// Calculate and set CRC for formed header

            buffer.resize(sizeof(readHeader), 0);
            memcpy(buffer.data(), &readHeader, sizeof(readHeader));

            expecetedDataBytes = blockSize;
            headerSize = sizeof(readHeader);

            if (showDebugInfo() == true)
            {
                theLog.writeMessage("");
                theLog.writeMessage(tr("Sending header, opcode Read:"));
                readHeader.dump(theLog);
            }
        }
        break;

    case Write:
        {
            assert(requestData.size() == blockSize);

            CONF_HEADER writeHeader = CONF_HEADER();

            writeHeader.version = static_cast<uint16_t>(ProtocolMaxVersion);
            writeHeader.moduleUartId = static_cast<uint16_t>(moduleUartId);
            writeHeader.opcode = static_cast<uint16_t>(opcode);
            writeHeader.frameIndex = static_cast<uint16_t>(frameIndex);
            writeHeader.frameSize = blockSize - sizeof(writeHeader.crc64);
            writeHeader.blockSize = blockSize;
            writeHeader.setCrc();

            buffer.resize(sizeof(writeHeader) + requestData.size(), 0);

            memcpy(buffer.data(), &writeHeader, sizeof(writeHeader));
            memcpy(static_cast<quint8*>(buffer.data()) + sizeof(writeHeader), requestData.data(), requestData.size());

            expecetedDataBytes = 0;
            headerSize = sizeof(writeHeader);

            if (showDebugInfo() == true)
            {
                theLog.writeMessage("");
                theLog.writeMessage(tr("Sending header, opcode Write:"));

                writeHeader.dump(theLog);
                theLog.writeDump(requestData);
            }
        }
        break;

	case Nop:
		{
			CONF_HEADER nopHeader = CONF_HEADER();

			nopHeader.version = static_cast<uint16_t>(ProtocolMaxVersion);
			nopHeader.opcode = static_cast<uint16_t>(opcode);
			nopHeader.setCrc();

			buffer.resize(sizeof(nopHeader), 0);
			memcpy(buffer.data(), &nopHeader, sizeof(nopHeader));

			headerSize = sizeof(nopHeader);
			expecetedDataBytes = 0;							// Not expecting any data, just header

			if (showDebugInfo() == true)
			{
                theLog.writeMessage("");
                theLog.writeMessage(tr("Sending header, opcode Nop"));
				nopHeader.dump(theLog);
			}
		}
		break;

	default:
		assert(false);
        theLog.writeError(__FUNCTION__ + tr(" Unknown command ") + opcode + ".");
		return false;
	}

	// Send Request
	//

	// Clear all buffers
	//
    m_serialPort->clear();

	// Send Data
	//

    qint64 writtenBytes = m_serialPort->write((char*)buffer.data(), buffer.size());
    m_serialPort->flush();

	if (writtenBytes != buffer.size())
	{
        theLog.writeError(tr("Written bytes number is ") + writtenBytes + ", expected is " + sizeof(buffer));
        theLog.writeError(tr("Operation terminated."));
		return false;
	}

	// Read reply
	//
    std::vector<quint8> recBuffer;
    int recSize = headerSize + expecetedDataBytes;

    int recMaxTime = (1000 * recSize / 11520) + 50; // time needed to receive all packet + 50 ms

    while (m_serialPort->waitForReadyRead(recMaxTime))
    {
        QByteArray arr = m_serialPort->read(recSize);

        for (int i = 0; i < arr.size(); i++)
        {
            recBuffer.push_back(arr[i]);
        }
    }

    qDebug()<<"Read "<<recBuffer.size();

    if (recBuffer.size() != recSize)
	{
        theLog.writeError(tr("Received ") + QString().setNum(recBuffer.size()) + " bytes, expected " + QString().setNum(recSize) + ".");
		return false;
	}

    std::vector<quint8> recHeaderBuffer;
    for (int i = 0; i < headerSize; i++)
    {
        recHeaderBuffer.push_back(recBuffer[i]);
    }

    // fill output header
	//
	*pReceivedHeader = *reinterpret_cast<decltype(pReceivedHeader)>(recHeaderBuffer.data());

	if (showDebugInfo() == true)
	{
		// Show Received Header as debug info
		//
        theLog.writeMessage(tr("Received header:"));
		pReceivedHeader->dump(theLog);
	}

	// Check received header checksum
	//
	if (pReceivedHeader->checkCrc() == false)
	{
		theLog.writeError(tr("Wrong CRC, received value is: ") +
            QString().setNum(pReceivedHeader->crc64, 16).rightJustified(16, '0') + ".");

		return false;
	}

	if (expecetedDataBytes == 0)
	{
		// We are not expecting any data
		//
		return true;
	}

	// Read Data
	//
    std::vector<quint8> recDataBuffer;
    for (int i = 0; i < expecetedDataBytes; i++)
    {
        recDataBuffer.push_back(recBuffer[headerSize + i]);
    }

	*replyData = recDataBuffer;

	if (showDebugInfo() == true)
	{
		// Show received data
		//
		theLog.writeDump(*replyData);
	}

	return true;
}

void Configurator::setSettings(QString device, bool showDebugInfo)
{
	this->setDevice(device);
	this->setShowDebugInfo(showDebugInfo);
	return;
}

void Configurator::readConfiguration(int param)
{
	emit communicationStarted();

	readConfigurationWorker(param);

	emit communicationFinished();

	return;
}

void Configurator::readConfigurationWorker(int /*param*/)
{
	// Open port
	//
	bool ok = openConnection();
	if (ok == false)
	{
		return;
	}

	try
	{
		//
		// PING command
		//
        std::vector<quint8> nopReply;
		CONF_HEADER pingReceivedHeader = CONF_HEADER();

        if (send(0, Nop, 0, 0, std::vector<quint8>(), &pingReceivedHeader, &nopReply) == false)
		{
			throw tr("Communication error (ping send error).");
		}

		int protocolVersion = pingReceivedHeader.version;
		int moduleUartId = 0;
		int blockSize = 0;
		
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

                // Check if the connector in the configuartion UART
                //
                if ((pingReplyVersioned.moduleUartId & ConfigurationUartMask) != ConfigurationUartValue)
                {
                    throw tr("Wrong UART, use configuration port.");
                }

                protocolVersion = pingReplyVersioned.version;
                moduleUartId = pingReplyVersioned.moduleUartId;
                blockSize = pingReplyVersioned.blockSize;

                // Ignore Wrong moduleUartId flag
                //
                pingReplyVersioned.flags &= ~OpDeniedInvalidModuleUartId;						// Ping was required to deremine moduleUartId

                // Check flags
                //
                if (pingReplyVersioned.flagStateSuccess() != true)
                {
                    pingReplyVersioned.dumpFlagsState(theLog);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
            theLog.writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }

        assert(protocolVersion != 0);
        assert(moduleUartId != 0);
        assert(blockSize != 0);

        //
        // READ indentification block
        //
        theLog.writeMessage(tr("Read identification block."));
		
        std::vector<quint8> identificationData;
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 readReceivedHeader = CONF_HEADER_V1();

                if (send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &identificationData) == false)
                {
                    throw tr("Communication error.");
                }

                assert(protocolVersion == readReceivedHeader.version);

                // Ignoring all flags, CRC, etc
                //

                if (identificationData.size() != blockSize)
                {
                    theLog.writeMessage(tr("Identification block is empty."));
                }
                else
                {
                    CONF_IDENTIFICATION_DATA_V1* pReadIdentificationStruct = reinterpret_cast<CONF_IDENTIFICATION_DATA_V1*>(identificationData.data());
                    if (pReadIdentificationStruct->marker == IdentificationStructMarker)
                    {
                        pReadIdentificationStruct->dump(theLog);
                    }
                    else
                    {
                        theLog.writeMessage(tr("Wrong identification block, marker: ") + QString().setNum(pReadIdentificationStruct->marker, 16));
                    }
                }
            }
            break;
        default:
            assert(false);
        }
	

        //
        // READ command
        //
        std::vector<quint8> readData;
        CONF_HEADER readReceivedHeader = CONF_HEADER();
		
        if (send(moduleUartId, Read, ConfiguartionFrameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &readData) == false)
        {
            throw tr("Communication error.");
        }

        assert(protocolVersion == readReceivedHeader.version);
		
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 readReply = *reinterpret_cast<CONF_HEADER_V1*>(&readReceivedHeader);

                // Check flags
                //
                if (readReply.flagStateSuccess() != true)
                {
                    readReceivedHeader.dumpFlagsState(theLog);
                    throw tr("Communication error.");
                }

                // Send factoryNo, Crc's and other to interface
                //
                emit communicationReadFinished(readReply.version, readData);
            }
            break;
        default:
            theLog.writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }
				
        // --
        //
        theLog.writeSuccess(tr("Successful."));
	}
	catch (QString str)
	{
        theLog.writeError(str);
	}

	// Close connection
	//
	if (closeConnection() == false)
	{
        theLog.writeError(tr("CloseConnection failed with error "));
	}

	emit communicationFinished();
	return;
}

void Configurator::writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc1, quint32 firmwareCrc2)
{
	emit communicationStarted();

    // Open port
    //
    if (openConnection() == false)
    {
        theLog.writeError(tr("Cannot open ") + device() + ".");
        emit communicationFinished();
        return;
    }

    try
    {
        //
        // PING command
        //
        std::vector<quint8> nopReply;
        CONF_HEADER pingReceivedHeader = CONF_HEADER();

        if (send(0, Nop, 0, 0, std::vector<quint8>(), &pingReceivedHeader, &nopReply) == false)
        {
            throw tr("Communication error.");
        }

        int protocolVersion = pingReceivedHeader.version;
        int moduleUartId = 0;
        int blockSize = 0;
        int frameSize = 0;
		
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

                // Check if the connector in the configuartion UART
                //
                if ((pingReplyVersioned.moduleUartId & ConfigurationUartMask) != ConfigurationUartValue)
                {
                    throw tr("Wrong UART, use configuration port.");
                }

                protocolVersion = pingReplyVersioned.version;
                moduleUartId = pingReplyVersioned.moduleUartId;
                blockSize = pingReplyVersioned.blockSize;
                frameSize = pingReplyVersioned.frameSize;

                // Ignore Wrong moduleUartId flag
                //
                pingReplyVersioned.flags &= ~OpDeniedInvalidModuleUartId;						// Ping was required to deremine moduleUartId

                // Check flags
                //
                if (pingReplyVersioned.flagStateSuccess() != true)
                {
                    pingReplyVersioned.dumpFlagsState(theLog);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
            theLog.writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }

        assert(protocolVersion != 0);
        assert(moduleUartId != 0);
        assert(blockSize != 0);

        //
        // READ IDENTIFICATION BLOCK
        //
        theLog.writeMessage(tr("Read identification block."));
		
        std::vector<quint8> identificationData;
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 readReceivedHeader = CONF_HEADER_V1();

                if (send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &identificationData) == false)
                {
                    throw tr("Communication error.");
                }

                assert(protocolVersion == readReceivedHeader.version);

                // Ignoring all flags, CRC, etc
                //
            }
            break;
        default:
            assert(false);
        }
		
        //
        // WRITE IDENTIFICATION BLOCK
        //
        if (identificationData.size() != blockSize)
        {
            identificationData.resize(blockSize);
        }
		
        CONF_IDENTIFICATION_DATA_V1* pReadIdentificationStruct = reinterpret_cast<CONF_IDENTIFICATION_DATA_V1*>(identificationData.data());
        if (pReadIdentificationStruct->marker != IdentificationStructMarker)
        {
            // This is the first configuration
            //
            memset(identificationData.data(), 0, identificationData.size());

            pReadIdentificationStruct->marker = IdentificationStructMarker;
            pReadIdentificationStruct->version = 1;
            pReadIdentificationStruct->moduleUuid = QUuid::createUuid();	// Add this record to database Uniquie MODULE identifier
            pReadIdentificationStruct->count = 1;

            //
            pReadIdentificationStruct->firstConfiguration.configurationId = QUuid::createUuid();	// Add this record to database
            pReadIdentificationStruct->firstConfiguration.date = QDateTime::currentDateTime().toTime_t();

#pragma message(__FUNCTION__ "When we will ha configurator factory, enter here it")
            pReadIdentificationStruct->firstConfiguration.configuratorFactoryNo = m_configuratorfactoryNo;

            QString hostName = QHostInfo::localHostName().right(sizeof(pReadIdentificationStruct->firstConfiguration.host) - 1);
            strcpy_s(pReadIdentificationStruct->firstConfiguration.host, sizeof(pReadIdentificationStruct->firstConfiguration.host), hostName.toStdString().data());

            pReadIdentificationStruct->lastConfiguration = pReadIdentificationStruct->firstConfiguration;
        }
        else
        {
            //pReadIdentificationStruct->dump(theLog);

            // last configuration record
            //
            pReadIdentificationStruct->count ++;			// Incerement configartion counter

            CONF_IDENTIFICATION_DATA_V1::CONF_IDENTIFICATION_RECORD lastConfiguration = CONF_IDENTIFICATION_DATA_V1::CONF_IDENTIFICATION_RECORD();
            lastConfiguration.configurationId = QUuid::createUuid();				// Add this record to database
            lastConfiguration.date = QDateTime().currentDateTime().toTime_t();

#pragma message(__FUNCTION__ "When we will ha configurator factory, enter here it")
            lastConfiguration.configuratorFactoryNo = m_configuratorfactoryNo;

            QString hostName = QHostInfo::localHostName().right(sizeof(lastConfiguration.host) - 1);
            strcpy_s(lastConfiguration.host, sizeof(lastConfiguration.host), hostName.toStdString().data());

            pReadIdentificationStruct->lastConfiguration = lastConfiguration;
        }

        // Set Crc to identificationData
        //
        Crc::setDataBlockCrc(IdentificationFrameIndex, identificationData.data(), (int)identificationData.size());
				
        CONF_HEADER_V1 replyIdentificationHeader = CONF_HEADER_V1();
        if (send(moduleUartId, Write, IdentificationFrameIndex, blockSize, identificationData, &replyIdentificationHeader, &std::vector<quint8>()) == false)
        {
            throw tr("Communication error.");
        }

        if (replyIdentificationHeader.version != 1)
        {
            throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyIdentificationHeader.version) + ".";
        }

        if (replyIdentificationHeader.flagStateSuccess() != true)
        {
            replyIdentificationHeader.dumpFlagsState(theLog);
            throw tr("Communication error.");
        }
		
        //
        // WRITE CONFIGURATION command
        //
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_SERVICE_DATA_V1 writeServiceStruct = CONF_SERVICE_DATA_V1();
                QDate now = QDate::currentDate();

                writeServiceStruct.setFactoryNo(factoryNo);
                writeServiceStruct.setManufactureYear(manufactureDate.year());
                writeServiceStruct.setManufactureMonth(manufactureDate.month());
                writeServiceStruct.setManufactureDay(manufactureDate.day());
                writeServiceStruct.setConfigureYear(now.year());
                writeServiceStruct.setConfigureMonth(now.month());
                writeServiceStruct.setConfigureDay(now.day());
                writeServiceStruct.setFirmwareCrc1(firmwareCrc1);
                writeServiceStruct.setFirmwareCrc2(firmwareCrc2);

                std::vector<quint8> writeData;
                writeData.resize(blockSize, 0);
                memcpy(writeData.data(), &writeServiceStruct, sizeof(writeServiceStruct));

                // Set Crc to databuffer
                //
                Crc::setDataBlockCrc(ConfiguartionFrameIndex, writeData.data(), (int)writeData.size());
				
                // --
                //
                CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
                if (send(moduleUartId, Write, ConfiguartionFrameIndex, blockSize, writeData, &replyHeader, &std::vector<quint8>()) == false)
                {
                    throw tr("Communication error.");
                }

                if (replyHeader.version != 1)
                {
                    throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyHeader.version) + ".";
                }

                //
                if (replyHeader.flagStateSuccess() != true)
                {
                    replyHeader.dumpFlagsState(theLog);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
            assert(false);
        }
				
        // --
        //
        theLog.writeSuccess(tr("Successful."));
    }
    catch (QString str)
    {
        theLog.writeError(str);
    }

    // Close connection
    //
    if (closeConnection() == false)
    {
        theLog.writeError(tr("CloseConnection failed with error ") + QString().setNum(::GetLastError()) + ".");
    }

	emit communicationFinished();
	return;
}

void Configurator::writeConfData(ModuleFirmware *conf)
{
    emit communicationStarted();

    // Open port
    //
    if (openConnection() == false)
    {
        theLog.writeError(tr("Cannot open ") + device() + ".");
        emit communicationFinished();
        return;
    }

    try
    {
        //
        // PING command
        //
        std::vector<quint8> nopReply;
        CONF_HEADER pingReceivedHeader = CONF_HEADER();

        if (send(0, Nop, 0, 0, std::vector<quint8>(), &pingReceivedHeader, &nopReply) == false)
        {
            throw tr("Communication error.");
        }

        int protocolVersion = pingReceivedHeader.version;
        int moduleUartId = 0;
        int blockSize = 0;
        int frameSize = 0;
        int blockCount = 0;
		
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

                protocolVersion = pingReplyVersioned.version;
                moduleUartId = pingReplyVersioned.moduleUartId;
                blockSize = pingReplyVersioned.blockSize;
                frameSize = pingReplyVersioned.frameSize;
                blockCount = pingReplyVersioned.romSize / pingReplyVersioned.blockSize;

                // Check if the connector in correct Uart
                //
#pragma message(Q_FUNC_INFO " DEBUG!!!!!!!!!!!!!!!!!!!!!!! Uncomment it!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ")
                //if (moduleUartId != conf.uartID())
                //{
                //	throw tr("Wrong UART, use %1h port.").arg(QString::number(conf.uartID(), 16));
                //}

				int confFrameDataSize = conf->frameSize() - sizeof(pingReplyVersioned.crc64);

				if (pingReplyVersioned.frameSize < confFrameDataSize)
                {
					throw tr("EEPROM Frame size is to small, requeried at least %1, but current frame size is %2.").arg(confFrameDataSize).arg(frameSize);
                }

                // Ignore Wrong moduleUartId flag
                //
                pingReplyVersioned.flags &= ~OpDeniedInvalidModuleUartId;						// Ping was required to deremine moduleUartId

                // Check flags
                //
                if (pingReplyVersioned.flagStateSuccess() != true)
                {
                    pingReplyVersioned.dumpFlagsState(theLog);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
            theLog.writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }

        assert(protocolVersion != 0);
        assert(moduleUartId != 0);
        assert(blockSize != 0);

        //
        // READ IDENTIFICATION BLOCK
        //
        theLog.writeMessage(tr("Read identification block."));
		
        std::vector<uint8_t> identificationData;
        switch (protocolVersion)
        {
        case 1:
            {
                CONF_HEADER_V1 readReceivedHeader = CONF_HEADER_V1();

                if (send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<uint8_t>(), &readReceivedHeader, &identificationData) == false)
                {
                    throw tr("Communication error.");
                }

                assert(protocolVersion == readReceivedHeader.version);

                // Ignoring all flags, CRC, etc
                //
            }
            break;
        default:
            assert(false);
        }
		
        //
        // WRITE IDENTIFICATION BLOCK
        //
        if (identificationData.size() != blockSize)
        {
            identificationData.resize(blockSize);
        }
		
        CONF_IDENTIFICATION_DATA_V1* pReadIdentificationStruct = reinterpret_cast<CONF_IDENTIFICATION_DATA_V1*>(identificationData.data());
        if (pReadIdentificationStruct->marker != IdentificationStructMarker)
        {
            // This is the first configuration
            //
            memset(identificationData.data(), 0, identificationData.size());

            pReadIdentificationStruct->marker = IdentificationStructMarker;
            pReadIdentificationStruct->version = 1;
            pReadIdentificationStruct->moduleUuid = QUuid::createUuid();	// Add this record to database Uniquie MODULE identifier
            pReadIdentificationStruct->count = 1;

            //
            pReadIdentificationStruct->firstConfiguration.configurationId = QUuid::createUuid();	// Add this record to database
            pReadIdentificationStruct->firstConfiguration.date = QDateTime::currentDateTime().toTime_t();

#pragma message(__FUNCTION__ "When we will ha configurator factory, enter here it")
            pReadIdentificationStruct->firstConfiguration.configuratorFactoryNo = m_configuratorfactoryNo;

            QString hostName = QHostInfo::localHostName().right(sizeof(pReadIdentificationStruct->firstConfiguration.host) - 1);
            strcpy_s(pReadIdentificationStruct->firstConfiguration.host, sizeof(pReadIdentificationStruct->firstConfiguration.host), hostName.toStdString().data());

            pReadIdentificationStruct->lastConfiguration = pReadIdentificationStruct->firstConfiguration;
        }
        else
        {
            //pReadIdentificationStruct->dump(theLog);

            // last configuration record
            //
            pReadIdentificationStruct->count ++;			// Incerement configartion counter

            CONF_IDENTIFICATION_DATA_V1::CONF_IDENTIFICATION_RECORD lastConfiguration = CONF_IDENTIFICATION_DATA_V1::CONF_IDENTIFICATION_RECORD();
            lastConfiguration.configurationId = QUuid::createUuid();				// Add this record to database
            lastConfiguration.date = QDateTime().currentDateTime().toTime_t();

#pragma message(__FUNCTION__ "When we will ha configurator factory, enter here it")
            lastConfiguration.configuratorFactoryNo = m_configuratorfactoryNo;

            QString hostName = QHostInfo::localHostName().right(sizeof(lastConfiguration.host) - 1);
            strcpy_s(lastConfiguration.host, sizeof(lastConfiguration.host), hostName.toStdString().data());

            pReadIdentificationStruct->lastConfiguration = lastConfiguration;
        }

        // Set Crc to identificationData
        //
        Crc::setDataBlockCrc(IdentificationFrameIndex, identificationData.data(), (int)identificationData.size());
				
        CONF_HEADER_V1 replyIdentificationHeader = CONF_HEADER_V1();
        if (send(moduleUartId, Write, IdentificationFrameIndex, blockSize, identificationData, &replyIdentificationHeader, &std::vector<uint8_t>()) == false)
        {
            throw tr("Communication error.");
        }

        if (replyIdentificationHeader.version != 1)
        {
            throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyIdentificationHeader.version) + ".";
        }

        if (replyIdentificationHeader.flagStateSuccess() != true)
        {
            replyIdentificationHeader.dumpFlagsState(theLog);
            throw tr("Communication error.");
        }
		
        //
        // WRITE CONFIGURATION command
        //
        switch (protocolVersion)
        {
        case 1:
            {
                theLog.writeMessage("Write configuration...");

                //std::vector<int> frames = conf->frameCount();
                for (int i = 0; i < conf->frameCount(); i++)
                {
                    //uint16_t frameIndex = static_cast<uint16_t>(frames[i]);
                    uint16_t frameIndex = i;

					if (frameIndex == IdentificationFrameIndex)
					{
						// Skip identifiaction frame
						//
						continue;
					}

                    theLog.writeMessage(tr("Writing block %1").arg(frameIndex));

					if (frameIndex >= blockCount)
                    {
                        throw tr("Wrong FrameIndex %1").arg(frameIndex);
                    }

                    std::vector<quint8> frameData = conf->frame(i);
					frameData.resize(blockSize, 0);

                    // Set Crc to databuffer
                    //
                    Crc::setDataBlockCrc(frameIndex, frameData.data(), (int)frameData.size());

                    // Write data
                    //
                    CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
                    bool ok = send(moduleUartId, Write, frameIndex, blockSize, frameData, &replyHeader, &std::vector<quint8>());

                    if (ok == false)
                    {
                        throw tr("Communication error.");
                    }
					
                    if (replyHeader.version != 1)
                    {
                        throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyHeader.version) + ".";
                    }

                    if (replyHeader.flagStateSuccess() != true)
                    {
                        replyHeader.dumpFlagsState(theLog);
                        throw tr("Communication error.");
                    }
                }
            }
            break;
        default:
            assert(false);
        }
				
        // --
        //
        theLog.writeSuccess(tr("Successful."));
    }
    catch (QString str)
    {
        theLog.writeError(str);
    }

    // Close connection
    //
    if (closeConnection() == false)
    {
        theLog.writeError(tr("CloseConnection failed with error ") + QString().setNum(::GetLastError()) + ".");
    }

	emit communicationFinished();
	return;
}

void Configurator::eraseFlashMemory(int)
{
	emit communicationStarted();

//	// Open port
//	//
//	HANDLE hDevice = openConnection();
//	if (hDevice == INVALID_HANDLE_VALUE)
//	{
//		theLog.writeError(tr("Cannot open ") + device() + ".", true);
//		emit communicationFinished();
//		return;
//	}

//	try
//	{
//		//
//		// PING command
//		//
//		std::vector<uint8_t> nopReply;
//		CONF_HEADER pingReceivedHeader = CONF_HEADER();

//		if (send(hDevice, 0, Nop, 0, 0, std::vector<uint8_t>(), &pingReceivedHeader, &nopReply) == false)
//		{
//			throw tr("Communication error.");
//		}

//		int protocolVersion = pingReceivedHeader.version;
//		int moduleUartId = 0;
//		int blockSize = 0;
//		int romSize = 0;
		
//		switch (protocolVersion)
//		{
//		case 1:
//			{
//				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

//				// Check if the connector in the configuartion UART
//				//
//				if ((pingReplyVersioned.moduleUartId & ConfigurationUartMask) != ConfigurationUartValue)
//				{
//					// Erase any memomy type
//					//
//					//throw tr("Wrong UART, use configuration port.");
//				}

//				protocolVersion = pingReplyVersioned.version;
//				moduleUartId = pingReplyVersioned.moduleUartId;
//				blockSize = pingReplyVersioned.blockSize;
//				romSize = pingReplyVersioned.romSize;

//				if (romSize % blockSize != 0)
//				{
//					throw tr("Flash memory block count is not an intergral multiple.");
//				}

//				// Ignore Wrong moduleUartId flag
//				//
//				pingReplyVersioned.flags &= ~OpDeniedInvalidModuleUartId;						// Ping was required to deremine moduleUartId

//				// Check flags
//				//
//				if (pingReplyVersioned.flagStateSuccess() != true)
//				{
//					pingReplyVersioned.dumpFlagsState(theLog);
//					throw tr("Communication error.");
//				}
//			}
//			break;
//		default:
//			theLog.writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
//			throw tr("Communication error.");
//		}

//		assert(protocolVersion != 0);
//		assert(moduleUartId != 0);
//		assert(blockSize != 0);
		
//		//
//		// ERASE command (Write zeros)
//		//

//		// Disabel showing DebugInfo (too much information)
//		bool oldStateShowDebugInfo = showDebugInfo();

//		// restore ShwoDebugInfo state throw shared_ptr and deleter
//		//
//		auto deleter = [this, oldStateShowDebugInfo](bool*)
//			{
//				this->setShowDebugInfo(oldStateShowDebugInfo);
//			};
//		std::shared_ptr<bool> scopedRestoreShowDebugInfo(nullptr, deleter);

//		setShowDebugInfo(false);

//		switch (protocolVersion)
//		{
//		case 1:
//			{
//				theLog.writeMessage("");

//				int blockCount = romSize / blockSize;

//				for (decltype(CONF_HEADER_V1().frameIndex) i = 0; i < blockCount; i++)
//				{
//					if (i == IdentificationFrameIndex)
//					{
//						theLog.writeMessage(tr("Erasing block ") + QString().setNum(i) + " - skip identification block.");
//						continue;
//					}

//					theLog.writeMessage(tr("Erasing block ") + QString().setNum(i));

//					std::vector<uint8_t> writeData;
//					writeData.resize(blockSize, 0);

//					// Set Crc to databuffer
//					//
//					Crc::setDataBlockCrc(i, writeData.data(), writeData.size());

//					// --
//					//
//					CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
//					if (send(hDevice, moduleUartId, Write, i, blockSize, writeData, &replyHeader, &std::vector<uint8_t>()) == false)
//					{
//						throw tr("Communication error.");
//					}

//					if (replyHeader.version != 1)
//					{
//						throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyHeader.version) + ".";
//					}

//					//
//					if (replyHeader.flagStateSuccess() != true)
//					{
//						replyHeader.dumpFlagsState(theLog);
//						throw tr("Communication error.");
//					}
//				}
//			}
//			break;
//		default:
//			assert(false);
//		}
				
//		// --
//		//
//		theLog.writeSuccess(tr("Successful."), true);
//	}
//	catch (QString str)
//	{
//		theLog.writeError(str, true);
//	}

//	// Close connection
//	//
//	if (closeConnection(hDevice) == false)
//	{
//		theLog.writeError(tr("CloseConnection failed with error ") + QString().setNum(::GetLastError()) + ".");
//	}

	emit communicationFinished();
	return;
}
