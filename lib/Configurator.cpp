#include "Stable.h"
#include "../lib/Configurator.h"
#include "../lib/Crc.h"
#include <QtEndian>
#include <QHostInfo>

//#ifdef Q_OS_WIN32
//#include "./ftdi/ftd2xx.h"
//#endif


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
void CONF_HEADER_V1::dump(OutputLog *log)
{
	if (log == nullptr)
	{
		assert(log);
		return;
	}

	log->writeMessage("version: " + QString().setNum(version, 16).rightJustified(4, '0'));
	log->writeMessage("moduleUartId: " + QString().setNum(moduleUartId, 16).rightJustified(4, '0'));
	log->writeMessage("opcode: " + QString().setNum(opcode, 16).rightJustified(4, '0'));
	log->writeMessage("flags: " + QString().setNum(flags, 16).rightJustified(4, '0'));
	log->writeMessage("frameIndex: " + QString().setNum(frameIndex, 16).rightJustified(4, '0'));
	log->writeMessage("frameSize: " + QString().setNum(frameSize, 16).rightJustified(4, '0'));
	log->writeMessage("blockSize: " + QString().setNum(blockSize, 16).rightJustified(4, '0'));
	log->writeMessage("romSize: " + QString().setNum(romSize, 16).rightJustified(8, '0'));
	log->writeMessage("crc64: " + QString().setNum(crc64, 16).rightJustified(16, '0'));

	return;
}

void CONF_HEADER_V1::dumpFlagsState(OutputLog* log)
{
	if (log == nullptr)
	{
		assert(log);
		return;
	}

	if (flags & OpDeniedCalibrationIsActive)
	{
		log->writeError("Flags: Operation denied, calibration is active.");
	}

	if (flags & OpDeniedInvalidModuleUartId)
	{
		log->writeError("Flags: Operation denied, invalid ModeleID.");
	}

	if (flags & OpDeniedInvalidOpcode)
	{
		log->writeError("Flags: Invalid opcode.");
	}
	
	if (flags & OpDeniedInvalidFrameIndex)
	{
		log->writeError("Flags: Operation denied, invalid frame index.");
	}

	if (flags & OpDeniedInvalidCrc)
	{
		log->writeError("Flags: Operation denied, invalid header CRC.");
	}

	if (flags & OpDeniedEepromHwError)
	{
		log->writeError("Flags: EEPROM HW error.");
	}

	if (flags & OpDeniedInvalidEepromCrc)
	{
		log->writeError("Flags: Operation denied, invalid data CRC.");
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

uint32_t CONF_SERVICE_DATA_V1::firmwareCrc() const
{
    return qFromBigEndian(m_firmwareCrc);
}
void CONF_SERVICE_DATA_V1::setFirmwareCrc(uint32_t value)
{
    m_firmwareCrc = qToBigEndian(value);
}

//
// CONF_IDENTIFICATION_DATA_V1
//
void CONF_IDENTIFICATION_DATA_V1::dump(OutputLog* log) const
{
	if (log == nullptr)
	{
		assert(log);
		return;
	}

	log->writeMessage(QString("Identification struct version: %1").arg(structVersion()));

	log->writeMessage("BlockId: " + moduleUuid.toQUuid().toString());
	log->writeMessage("Configuration counter: " + QString().setNum(count));
			
	log->writeMessage("First time configured: ");
	log->writeMessage("__Date: " + QDateTime().fromTime_t(firstConfiguration.date).toString());
	log->writeMessage("__Host: " + QString(firstConfiguration.host));
	log->writeMessage("__ConfigurationId: " + firstConfiguration.configurationId.toQUuid().toString());
	log->writeMessage("__Configurator factory no: " + QString().setNum(firstConfiguration.configuratorFactoryNo));

	log->writeMessage("Last time configured: ");
	log->writeMessage("__Date: " + QDateTime().fromTime_t(lastConfiguration.date).toString());
	log->writeMessage("__Host: " + QString(lastConfiguration.host));
	log->writeMessage("__ConfigurationId: " + lastConfiguration.configurationId.toQUuid().toString());
	log->writeMessage("__Configurator factory no: " + QString().setNum(lastConfiguration.configuratorFactoryNo));

	return;
}

void CONF_IDENTIFICATION_DATA_V1::createFirstConfiguration()
{
	marker = IdentificationStructMarker;
	version = CONF_IDENTIFICATION_DATA_V1::structVersion();
	moduleUuid = QUuid::createUuid();	// Add this record to database Uniquie MODULE identifier
	count = 1;

	//
	firstConfiguration.configurationId = QUuid::createUuid();	// Add this record to database
	firstConfiguration.date = QDateTime::currentDateTime().toTime_t();

//#pragma message(__FUNCTION__ "When we will ha configurator factory, enter here it")
	firstConfiguration.configuratorFactoryNo = 0;

	QString hostName = QHostInfo::localHostName().right(sizeof(firstConfiguration.host) - 1);
	strncpy(firstConfiguration.host, hostName.toStdString().data(), sizeof(firstConfiguration.host));

	lastConfiguration = firstConfiguration;
}

void CONF_IDENTIFICATION_DATA_V1::createNextConfiguration()
{
	// last configuration record
	//
	count ++;			// Incerement configartion counter

	CONF_IDENTIFICATION_DATA_V1::CONF_IDENTIFICATION_RECORD lastConfiguration = CONF_IDENTIFICATION_DATA_V1::CONF_IDENTIFICATION_RECORD();
	lastConfiguration.configurationId = QUuid::createUuid();				// Add this record to database
	lastConfiguration.date = QDateTime().currentDateTime().toTime_t();

//#pragma message(__FUNCTION__ "When we will ha configurator factory, enter here it")
	lastConfiguration.configuratorFactoryNo = 0;

	QString hostName = QHostInfo::localHostName().right(sizeof(lastConfiguration.host) - 1);
	strncpy(lastConfiguration.host, hostName.toStdString().data(), sizeof(lastConfiguration.host));

	lastConfiguration = lastConfiguration;
}

//
// CONF_IDENTIFICATION_DATA_V2
//
void CONF_IDENTIFICATION_DATA_V2::dump(OutputLog* log) const
{
	if (log == nullptr)
	{
		assert(log);
		return;
	}

	log->writeMessage(QString("Identification struct version: %1").arg(structVersion()));

	log->writeMessage("BlockId: " + moduleUuid.toQUuid().toString());
	log->writeMessage("Configuration counter: " + QString().setNum(count));

	log->writeMessage("First time configured: ");
	log->writeMessage("__Date: " + QDateTime().fromTime_t(firstConfiguration.date).toString());
	log->writeMessage("__Host: " + QString(firstConfiguration.host));
	log->writeMessage("__User: " + QString(firstConfiguration.userName));
	log->writeMessage("__Build No: " + QString::number(firstConfiguration.buildNo).rightJustified(6, '0'));
	log->writeMessage("__Build Config: " + QString(firstConfiguration.buildConfig));
	log->writeMessage("__ConfigurationId: " + firstConfiguration.configurationId.toQUuid().toString());

	log->writeMessage("Last time configured: ");
	log->writeMessage("__Date: " + QDateTime().fromTime_t(lastConfiguration.date).toString());
	log->writeMessage("__Host: " + QString(lastConfiguration.host));
	log->writeMessage("__User: " + QString(lastConfiguration.userName));
	log->writeMessage("__Build No: " + QString::number(lastConfiguration.buildNo).rightJustified(6, '0'));
	log->writeMessage("__Build Config: " + QString(lastConfiguration.buildConfig));
	log->writeMessage("__ConfigurationId: " + lastConfiguration.configurationId.toQUuid().toString());

	return;
}

void CONF_IDENTIFICATION_DATA_V2::createFirstConfiguration(Hardware::ModuleFirmwareStorage* storage)
{

	marker = IdentificationStructMarker;
	version = CONF_IDENTIFICATION_DATA_V2::structVersion();
	moduleUuid = QUuid::createUuid();	// Add this record to database Uniquie MODULE identifier
	count = 1;

	//
	firstConfiguration.configurationId = QUuid::createUuid();	// Add this record to database
	firstConfiguration.date = QDateTime::currentDateTime().toTime_t();

	QString hostName = QHostInfo::localHostName().right(sizeof(firstConfiguration.host) - 1);
	strncpy(firstConfiguration.host, hostName.toStdString().data(), sizeof(firstConfiguration.host));

	firstConfiguration.buildNo = storage->buildNumber();

	QString buildConfig = storage->buildConfig().right(sizeof(firstConfiguration) - 1);
	strncpy(firstConfiguration.buildConfig, buildConfig.toStdString().data(), sizeof(firstConfiguration.buildConfig));

	QString userName = storage->userName().right(sizeof(firstConfiguration.userName) - 1);
	strncpy(firstConfiguration.userName, userName.toStdString().data(), sizeof(firstConfiguration.userName));

	lastConfiguration = firstConfiguration;
}

void CONF_IDENTIFICATION_DATA_V2::createNextConfiguration(Hardware::ModuleFirmwareStorage* storage)
{
	// last configuration record
	//
	count ++;			// Incerement configartion counter

	lastConfiguration.configurationId = QUuid::createUuid();				// Add this record to database
	lastConfiguration.date = QDateTime().currentDateTime().toTime_t();

	QString hostName = QHostInfo::localHostName().right(sizeof(lastConfiguration.host) - 1);
	strncpy(lastConfiguration.host, hostName.toStdString().data(), sizeof(lastConfiguration.host));

	lastConfiguration.buildNo = storage->buildNumber();

	QString buildConfig = storage->buildConfig().right(sizeof(lastConfiguration.buildConfig) - 1);
	strncpy(lastConfiguration.buildConfig, buildConfig.toStdString().data(), sizeof(lastConfiguration.buildConfig));

	QString userName = storage->userName().right(sizeof(lastConfiguration.userName) - 1);
	strncpy(lastConfiguration.userName, userName.toStdString().data(), sizeof(lastConfiguration.userName));
}

//
// Configurator
//
Configurator::Configurator(QString serialDevice, OutputLog *log, QObject* parent)
	: QObject(parent),
	m_device(serialDevice),
	m_serialPort(nullptr),
	m_Log(log)
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

bool Configurator::verify() const
{
	return m_verify;
}

void Configurator::setVerify(bool value)
{
	m_verify = value;
}

bool Configurator::openConnection()
{
	m_configuratorfactoryNo = 0;

	// Check configurator serial no
	//
#ifdef Q_OS_UNIX
	m_Log->writeMessage("The Linux version does not support configurator Factory No (FTDI programmer factory number).");
	m_Log->writeMessage("You can still continue using configurator, but a Factory No will not be written to the configured module.");
#endif

	// Read programmers factory no
	//
/*#ifdef Q_OS_WIN32
    DWORD DeviceCount = 0;
	FT_STATUS Result = FT_CreateDeviceInfoList(&DeviceCount);
	if (Result != FT_OK)
	{
		m_Log->writeError(__FUNCTION__ + tr(" FT_CreateDeviceInfoList error."));
        return false;
	}

	if (DeviceCount == 0)
	{
		m_Log->writeError(__FUNCTION__ + tr(" Can't find any configurator."));
        return false;
	}

	if (DeviceCount != 1)
	{
		m_Log->writeError(__FUNCTION__ + tr(" There are more than one configurator, please leave only one."));
        return false;
	}

	FT_HANDLE ftHandle;

	if (FT_Open(0, &ftHandle) != FT_OK)
	{
		m_Log->writeError(__FUNCTION__ + tr(" FT_Open error."));
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
			m_Log->writeError(__FUNCTION__ + tr(" Wrong configuration factory no(") + SerialNumberBuf + ")");
            FT_Close(ftHandle);
            return false;
		}

		m_configuratorfactoryNo = sn;
		m_Log->writeMessage(tr("Configurator factory no:") + QString().setNum(m_configuratorfactoryNo));
	}
	else
	{
		m_Log->writeError(__FUNCTION__ + tr(" FT_Read error."));
        return false;
	}

    FT_Close(ftHandle);
#endif*/

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
		m_Log->writeError(__FUNCTION__ + tr(" %1").arg(m_serialPort->errorString()));

#ifdef Q_OS_LINUX
		if (m_serialPort->error() == QSerialPort::PermissionError)
		{
			m_Log->writeMessage(tr("Add user to the dialout group by the following command:"));
			m_Log->writeMessage(tr("sudo usermod -a -G dialout USER"));
			m_Log->writeMessage(tr("Then user has to logout and log in back."));
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
		m_Log->writeError("CloseConnection error: The port was already closed.");
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
		m_Log->writeError(tr("Port object is not created: %1").arg(device()));
        return false;
    }

    if (m_serialPort->isOpen() == false)
	{
		m_Log->writeError(tr("Port is not opened: %1").arg(device()));
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
				m_Log->writeMessage("");
				m_Log->writeMessage(tr("Sending header, opcode Read:"));
				readHeader.dump(m_Log);
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
				m_Log->writeMessage("");
				m_Log->writeMessage(tr("Sending header, opcode Write:"));

				writeHeader.dump(m_Log);
				m_Log->writeDump(requestData);
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
				m_Log->writeMessage("");
				m_Log->writeMessage(tr("Sending header, opcode Nop"));
				nopHeader.dump(m_Log);
			}
		}
		break;

	default:
		assert(false);
		m_Log->writeError(__FUNCTION__ + tr(" Unknown command ") + opcode + ".");
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
		m_Log->writeError(tr("Written bytes number is ") + writtenBytes + ", expected is " + sizeof(buffer));
		m_Log->writeError(tr("Operation terminated."));
		return false;
	}

	// Read reply
	//
    std::vector<quint8> recBuffer;
    int recSize = headerSize + expecetedDataBytes;

	for (int tc = 0; tc < 100; tc++)
	{
		QThread::msleep(10);

		QApplication::processEvents();

		if (m_serialPort->bytesAvailable() == 0)
		{
			continue;
		}

		QByteArray arr = m_serialPort->readAll();

		for (int i = 0; i < arr.size(); i++)
		{
			recBuffer.push_back(arr[i]);
		}

		if (recBuffer.size() >= recSize)
		{
			break;
		}
	}

    if (recBuffer.size() != recSize)
	{
		m_Log->writeError(tr("Received ") + QString().setNum(recBuffer.size()) + " bytes, expected " + QString().setNum(recSize) + ".");
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
		m_Log->writeMessage(tr("Received header:"));
		pReceivedHeader->dump(m_Log);
	}

	// Check received header checksum
	//
	if (pReceivedHeader->checkCrc() == false)
	{
		m_Log->writeError(tr("Wrong CRC, received value is: ") +
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
		m_Log->writeDump(*replyData);
	}

	return true;
}

void Configurator::setSettings(QString device, bool showDebugInfo, bool verify)
{
	this->setDevice(device);
	this->setShowDebugInfo(showDebugInfo);
	this->setVerify(verify);

	if (showDebugInfo == true)
	{
		this->m_Log->writeMessage(tr("Configurator serial port is %1.").arg(device));
	}
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
					pingReplyVersioned.dumpFlagsState(m_Log);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }

        assert(protocolVersion != 0);
        assert(moduleUartId != 0);
        assert(blockSize != 0);

        //
        // READ indentification block
        //
		m_Log->writeMessage(tr("Read identification block."));
		
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

				dumpIdentificationData(identificationData, blockSize);
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
					readReceivedHeader.dumpFlagsState(m_Log);
                    throw tr("Communication error.");
                }

                // Send factoryNo, Crc's and other to interface
                //
                emit communicationReadFinished(readReply.version, readData);
            }
            break;
        default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }
				
        // --
        //
		m_Log->writeSuccess(tr("Successful."));
	}
	catch (QString str)
	{
		m_Log->writeError(str);
	}

	// Close connection
	//
	if (closeConnection() == false)
	{
		m_Log->writeError(tr("CloseConnection failed with error "));
	}

	return;
}

void Configurator::writeConfigurationWorker(ModuleFirmwareStorage *storage, const QString& subsystemId)
{
	m_cancelFlag = false;

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		return;
	}

	bool ok = false;
	Hardware::ModuleFirmware& conf = storage->moduleFirmware(subsystemId, &ok);

	if (ok == false)
	{
		assert(false);
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

				m_currentUartId = moduleUartId;

				m_Log->writeMessage(tr("UART ID is %1h").arg(QString::number(m_currentUartId, 16)));

				// Check if firmware exists for current uart

				if (conf.uartExists(m_currentUartId) == false)
				{
					throw tr("No firmware data exists for current UART ID.");
				}

				m_Log->writeMessage(tr("FrameSize: %1").arg(QString::number(conf.eepromFramePayloadSize(m_currentUartId))));
				m_Log->writeMessage(tr("FrameSize with CRC: %1").arg(QString::number(conf.eepromFrameSize(m_currentUartId))));
				m_Log->writeMessage(tr("FrameCount: %1").arg(QString::number(conf.eepromFrameCount(m_currentUartId))));

				int confFrameDataSize = conf.eepromFramePayloadSize(m_currentUartId);

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
					pingReplyVersioned.dumpFlagsState(m_Log);
					throw tr("Communication error.");
				}
			}
			break;
		default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
			throw tr("Communication error.");
		}

		assert(protocolVersion != 0);
		assert(moduleUartId != 0);
		assert(blockSize != 0);

		//
		// READ IDENTIFICATION BLOCK
		//
		m_Log->writeMessage(tr("Read identification block."));

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

		CONF_IDENTIFICATION_DATA* pReadIdentificationStruct = reinterpret_cast<CONF_IDENTIFICATION_DATA*>(identificationData.data());
		if (pReadIdentificationStruct->marker != IdentificationStructMarker ||
				pReadIdentificationStruct->version != CONF_IDENTIFICATION_DATA::structVersion())
		{

			if (pReadIdentificationStruct->marker == IdentificationStructMarker)
			{
				m_Log->writeMessage(tr("Upgrading CONF_IDENTIFICATION_DATA struct version: %1 -> %2").
									arg(pReadIdentificationStruct->version).arg(CONF_IDENTIFICATION_DATA::structVersion()));
			}

			if (sizeof(CONF_IDENTIFICATION_DATA) > blockSize)
			{
				throw tr("CONF_IDENTIFICATION_DATA struct size (%1) is bigger than blockSize (%2).").arg(sizeof(CONF_IDENTIFICATION_DATA)).arg(blockSize);
			}

			memset(identificationData.data(), 0, identificationData.size());

			// This is the first configuration
			//
			pReadIdentificationStruct->createFirstConfiguration(storage);

		}
		else
		{
			pReadIdentificationStruct->createNextConfiguration(storage);
			//pReadIdentificationStruct->dump(m_Log);

		}

		// Set Crc to identificationData
		//
		Crc::setDataBlockCrc(IdentificationFrameIndex, identificationData.data(), (int)identificationData.size());

		CONF_HEADER_V1 replyIdentificationHeader = CONF_HEADER_V1();
		auto replyData = std::vector<uint8_t>();
		if (send(moduleUartId, Write, IdentificationFrameIndex, blockSize, identificationData, &replyIdentificationHeader, &replyData) == false)
		{
			throw tr("Communication error.");
		}

		if (replyIdentificationHeader.version != 1)
		{
			throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyIdentificationHeader.version) + ".";
		}

		if (replyIdentificationHeader.flagStateSuccess() != true)
		{
			replyIdentificationHeader.dumpFlagsState(m_Log);
			throw tr("Communication error.");
		}

		if (verify() == true)
		{
			// Verify the written identificationData
			//

			m_Log->writeMessage(tr("Verifying block %1").arg(IdentificationFrameIndex));

			std::vector<quint8> readData;
			CONF_HEADER readReceivedHeader = CONF_HEADER();

			if (send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &readData) == false)
			{
				throw tr("Communication error.");
			}

			if (identificationData.size() != readData.size())
			{
				throw(tr("Send identificationData size does not match received data size in frame %1.").arg(IdentificationFrameIndex));
			}
			for (int v = 0; v < identificationData.size(); v++)
			{
				if (readData[v] != identificationData[v])
				{
					throw(tr("Sent identificationData does not match received data size in frame %1, offset %2.").arg(IdentificationFrameIndex).arg(v));
				}
			}
		}

		//
		// WRITE CONFIGURATION command
		//
		switch (protocolVersion)
		{
		case 1:
			{
				m_Log->writeMessage("Write configuration...");

				for (int i = 0; i < conf.eepromFrameCount(m_currentUartId); i++)
				{
					if (m_cancelFlag == true)
					{
						m_Log->writeMessage("Write configuration cancelled.");
						break;
					}

					uint16_t frameIndex = i;

					if (frameIndex == IdentificationFrameIndex)
					{
						// Skip identifiaction frame
						//
						continue;
					}

					m_Log->writeMessage(tr("Writing block %1").arg(frameIndex));

					if (frameIndex >= blockCount)
					{
						throw tr("Wrong FrameIndex %1").arg(frameIndex);
					}

					const std::vector<quint8> frameData = conf.frame(m_currentUartId, i);

					if (frameData.size() != blockSize)
					{
						throw tr("Frame %1 size (%2) does not match module's BlockSize (%3).").arg(i).arg(frameData.size()).arg(blockSize);
					}

					// Write data
					//
					CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
					auto replyData = std::vector<uint8_t>();
					bool ok = send(moduleUartId, Write, frameIndex, blockSize, frameData, &replyHeader, &replyData);

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
						replyHeader.dumpFlagsState(m_Log);
						throw tr("Communication error.");
					}

					if (verify() == true)
					{
						// Verify the written data
						//

						m_Log->writeMessage(tr("Verifying block %1").arg(frameIndex));

						std::vector<quint8> readData;
						CONF_HEADER readReceivedHeader = CONF_HEADER();

						if (send(moduleUartId, Read, frameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &readData) == false)
						{
							throw tr("Communication error.");
						}

						if (frameData.size() != readData.size())
						{
							throw(tr("Send data size does not match received data size in frame %1.").arg(frameIndex));
						}
						for (int v = 0; v < frameData.size(); v++)
						{
							if (readData[v] != frameData[v])
							{
								throw(tr("Send data does not match received data size in frame %1, offset %2.").arg(frameIndex).arg(v));
							}
						}
					}
				}
			}
			break;
		default:
			assert(false);
		}

		// --
		//
		m_Log->writeSuccess(tr("Successful."));

		emit uploadSuccessful(m_currentUartId);
	}
	catch (QString str)
	{
		m_Log->writeError(str);
	}

	// Close connection
	//
	if (closeConnection() == false)
	{
		m_Log->writeError(tr("CloseConnection failed."));
	}

	return;
}

void Configurator::dumpIdentificationData(const std::vector<quint8>& identificationData, int blockSize)
{
	if (identificationData.size() != blockSize)
	{
		m_Log->writeMessage(tr("Identification block is empty."));
	}

	const CONF_IDENTIFICATION_DATA* pReadIdentificationStruct = reinterpret_cast<const CONF_IDENTIFICATION_DATA*>(identificationData.data());
	if (pReadIdentificationStruct->marker == IdentificationStructMarker)
	{
		switch (pReadIdentificationStruct->version)
		{
		case 1:
			{
				const CONF_IDENTIFICATION_DATA_V1* pReadIdentificationStruct_v1 = reinterpret_cast<const CONF_IDENTIFICATION_DATA_V1*>(identificationData.data());
				pReadIdentificationStruct_v1->dump(m_Log);
			}
			break;
		case 2:
			{

				const CONF_IDENTIFICATION_DATA_V2* pReadIdentificationStruct_v2 = reinterpret_cast<const CONF_IDENTIFICATION_DATA_V2*>(identificationData.data());
				pReadIdentificationStruct_v2->dump(m_Log);
			}
			break;
		default:
			m_Log->writeMessage(tr("Unknown identification block version: ") + QString().setNum(pReadIdentificationStruct->version));
		}
	}
	else
	{
		m_Log->writeMessage(tr("Wrong identification block, marker: ") + QString().setNum(pReadIdentificationStruct->marker, 16));
	}
}


void Configurator::writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc)
{
	emit communicationStarted();

    // Open port
    //
    if (openConnection() == false)
    {
		m_Log->writeError(tr("Cannot open ") + device() + ".");
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
					pingReplyVersioned.dumpFlagsState(m_Log);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
            throw tr("Communication error.");
        }

        assert(protocolVersion != 0);
        assert(moduleUartId != 0);
        assert(blockSize != 0);

        //
        // READ IDENTIFICATION BLOCK
        //
		m_Log->writeMessage(tr("Read identification block."));
		
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
		
		// An empty firmware
		//

		QString userName = QDir::home().dirName();

		Hardware::ModuleFirmwareStorage storage;
		storage.setProjectInfo("projectName", userName, 0, false, 0);

		CONF_IDENTIFICATION_DATA* pReadIdentificationStruct = reinterpret_cast<CONF_IDENTIFICATION_DATA*>(identificationData.data());
		if (pReadIdentificationStruct->marker != IdentificationStructMarker ||
				pReadIdentificationStruct->version != CONF_IDENTIFICATION_DATA::structVersion())
		{

			if (pReadIdentificationStruct->marker == IdentificationStructMarker)
			{
				m_Log->writeMessage(tr("Upgrading CONF_IDENTIFICATION_DATA struct version: %1 -> %2").
									arg(pReadIdentificationStruct->version).arg(CONF_IDENTIFICATION_DATA::structVersion()));
			}

			if (sizeof(CONF_IDENTIFICATION_DATA) > blockSize)
			{
				throw tr("CONF_IDENTIFICATION_DATA struct size (%1) is bigger than blockSize (%2).").arg(sizeof(CONF_IDENTIFICATION_DATA)).arg(blockSize);
			}

			memset(identificationData.data(), 0, identificationData.size());

			// This is the first configuration
			//
			pReadIdentificationStruct->createFirstConfiguration(&storage);

		}
		else
		{
			pReadIdentificationStruct->createNextConfiguration(&storage);
			//pReadIdentificationStruct->dump(m_Log);

		}
        // Set Crc to identificationData
        //
        Crc::setDataBlockCrc(IdentificationFrameIndex, identificationData.data(), (int)identificationData.size());
				
        CONF_HEADER_V1 replyIdentificationHeader = CONF_HEADER_V1();
		auto replyData = std::vector<uint8_t>();
		if (send(moduleUartId, Write, IdentificationFrameIndex, blockSize, identificationData, &replyIdentificationHeader, &replyData) == false)
        {
            throw tr("Communication error.");
        }

        if (replyIdentificationHeader.version != 1)
        {
            throw tr("Command Write reply error. Different header version, expected 1, received ") + QString().setNum(replyIdentificationHeader.version) + ".";
        }

        if (replyIdentificationHeader.flagStateSuccess() != true)
        {
			replyIdentificationHeader.dumpFlagsState(m_Log);
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
                writeServiceStruct.setFirmwareCrc(firmwareCrc);

                std::vector<quint8> writeData;
                writeData.resize(blockSize, 0);
                memcpy(writeData.data(), &writeServiceStruct, sizeof(writeServiceStruct));

                // Set Crc to databuffer
                //
                Crc::setDataBlockCrc(ConfiguartionFrameIndex, writeData.data(), (int)writeData.size());
				
                // --
                //
                CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
				auto replyData = std::vector<uint8_t>();
				if (send(moduleUartId, Write, ConfiguartionFrameIndex, blockSize, writeData, &replyHeader, &replyData) == false)
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
					replyHeader.dumpFlagsState(m_Log);
                    throw tr("Communication error.");
                }
            }
            break;
        default:
            assert(false);
        }
				
        // --
        //
		m_Log->writeSuccess(tr("Successful."));
    }
    catch (QString str)
    {
		m_Log->writeError(str);
    }

    // Close connection
    //
    if (closeConnection() == false)
    {
		m_Log->writeError(tr("CloseConnection failed."));
    }

	emit communicationFinished();
	return;
}

void Configurator::showBinaryFileInfo(const QString& fileName)
{
	processConfDataFile(fileName, QString(), false);
}

void Configurator::uploadBinaryFile(const QString& fileName, const QString& subsystemId)
{
	processConfDataFile(fileName, subsystemId, true);
}

void Configurator::processConfDataFile(const QString& fileName, const QString& subsystemId, bool writeToFlash)
{
	emit communicationStarted();

	Hardware::ModuleFirmwareStorage confFirmware;

	m_Log->writeMessage(tr("//----------------------"));
	m_Log->writeMessage(tr("File: %1").arg(fileName));

	QString errorCode;
	bool result = false;

	if (writeToFlash == true)
	{
		result = confFirmware.load(fileName, &errorCode);
	}
	else
	{
		result = confFirmware.loadHeader(fileName, &errorCode);
	}

	if (result == false)
	{
		QString str = tr("File %1 wasn't loaded!").arg(fileName);
		if (errorCode.isEmpty() == false)
		{
			str += "\r\n\r\n" + errorCode;
		}

		m_Log->writeError(str);
		emit communicationFinished();
		return;
	}

	m_Log->writeMessage(tr("File Version: %1").arg(confFirmware.fileVersion()));
	m_Log->writeMessage(tr("ChangesetID: %1").arg(confFirmware.changesetId()));
	m_Log->writeMessage(tr("Build User: %1").arg(confFirmware.userName()));
	m_Log->writeMessage(tr("Build No: %1").arg(QString::number(confFirmware.buildNumber())));
	m_Log->writeMessage(tr("Build Config: %1").arg(confFirmware.buildConfig()));

	QStringList subsystemList = confFirmware.subsystemsList();
	QString subsystems;
	for (const QString& s : subsystemList)
	{
		subsystems.push_back(s + " ");
	}

	m_Log->writeMessage(tr("Subsystems: %1").arg(subsystems));

	if (writeToFlash == true)
	{
		writeConfigurationWorker(&confFirmware, subsystemId);
	}
	/*else
	{
		std::vector<UartPair> uartList = confFirmware.uartList();
		emit loadHeaderComplete(uartList);
	}*/

	emit communicationFinished();

	return;

}

void Configurator::uploadConfData(ModuleFirmwareStorage *storage, const QString& subsystemId)
{
	emit communicationStarted();

	writeConfigurationWorker(storage, subsystemId);

	emit communicationFinished();

}



void Configurator::readFirmware(const QString& fileName)
{
	m_cancelFlag = false;

	emit communicationStarted();

	// Open outputFile
	//
	QFile file(fileName);

	if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
	{
		m_Log->writeError(tr("Cannot open output file %1, %2").arg(fileName).arg(file.error()));
		emit communicationFinished();
		return;
	}

	QTextStream out(&file);
	//out << "Result: " << qSetFieldWidth(10) << left << 3.14 << 2.7;

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		emit communicationFinished();
		return;
	}

	try
	{
		//
		// PING command
		//
		std::vector<uint8_t> nopReply;
		CONF_HEADER pingReceivedHeader = CONF_HEADER();

		if (send(0, Nop, 0, 0, std::vector<uint8_t>(), &pingReceivedHeader, &nopReply) == false)
		{
			throw tr("Communication error.");
		}

		int protocolVersion = pingReceivedHeader.version;
		int moduleUartId = 0;
		int blockSize = 0;
		int romSize = 0;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				// Check if the connector in the configuartion UART
				//
				if ((pingReplyVersioned.moduleUartId & ConfigurationUartMask) != ConfigurationUartValue)
				{
					// Read any memomy type!!!
					//
					//throw tr("Wrong UART, use configuration port.");
				}

				protocolVersion = pingReplyVersioned.version;
				moduleUartId = pingReplyVersioned.moduleUartId;
				blockSize = pingReplyVersioned.blockSize;
				romSize = pingReplyVersioned.romSize;

				// Write log and output file
				//
				m_Log->writeEmptyLine();
				m_Log->writeMessage("PING Reply:");
				m_Log->writeMessage(QString("ProtocolVersion: %1").arg(protocolVersion));
				m_Log->writeMessage(QString("UartId: %1 (%2h)").arg(moduleUartId).arg(moduleUartId, 4, 16, QLatin1Char('0')));
				m_Log->writeMessage(QString("BlockSize: %1 (%2h)").arg(blockSize).arg(blockSize, 4, 16, QLatin1Char('0')));
				m_Log->writeMessage(QString("RomSize: %1 (%2h)").arg(romSize).arg(romSize, 4, 16, QLatin1Char('0')));

				out << "PING Reply:\n";
				out << QString("ProtocolVersion: %1\n").arg(protocolVersion);
				out << QString("UartId: %1 (%2h)\n").arg(moduleUartId).arg(moduleUartId, 4, 16, QLatin1Char('0'));
				out << QString("BlockSize: %1 (%2h)\n").arg(blockSize).arg(blockSize, 4, 16, QLatin1Char('0'));
				out << QString("RomSize: %1 (%2h)\n").arg(romSize).arg(romSize, 4, 16, QLatin1Char('0'));

				// --
				//
				if (romSize % blockSize != 0)
				{
					throw tr("Flash memory block count is not an intergral multiple.");
				}

				// Ignore Wrong moduleUartId flag
				//
				pingReplyVersioned.flags &= ~OpDeniedInvalidModuleUartId;						// Ping was required to deremine moduleUartId

				// Check flags
				//
				if (pingReplyVersioned.flagStateSuccess() != true)
				{
					pingReplyVersioned.dumpFlagsState(m_Log);
					throw tr("Communication error.");
				}
			}
			break;
		default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
			throw tr("Communication error.");
		}

		assert(protocolVersion != 0);
		assert(moduleUartId != 0);
		assert(blockSize != 0);

		int blockCount = romSize / blockSize;

		//
		// READ command
		//

		// Disabel showing DebugInfo (too much information)
		bool oldStateShowDebugInfo = showDebugInfo();

		// restore ShwoDebugInfo state throw shared_ptr and deleter
		//
		auto deleter = [this, oldStateShowDebugInfo](bool*)
			{
				this->setShowDebugInfo(oldStateShowDebugInfo);
			};
		std::shared_ptr<bool> scopedRestoreShowDebugInfo(nullptr, deleter);

		setShowDebugInfo(false);

		switch (protocolVersion)
		{
		case 1:
			{
				for (decltype(CONF_HEADER_V1().frameIndex) i = 0; i < blockCount; i++)
				{
					if (m_cancelFlag == true)
					{
						m_Log->writeMessage(tr("Firmware reading cancelled."));
						break;
					}

					m_Log->writeMessage(tr("Reading block ") + QString().setNum(i));
					out << "FrameIndex " << i << "\n";

					std::vector<quint8> readData;
					CONF_HEADER readReceivedHeader = CONF_HEADER();

					if (send(moduleUartId, Read, i, blockSize, std::vector<quint8>(), &readReceivedHeader, &readData) == false)
					{
						throw tr("Communication error.");
					}

					assert(protocolVersion == readReceivedHeader.version);

					if (i == 0)
					{
						dumpIdentificationData(readData, blockSize);
					}

					switch (protocolVersion)
					{
					case 1:
						{
							CONF_HEADER_V1 readReply = *reinterpret_cast<CONF_HEADER_V1*>(&readReceivedHeader);

							// Check flags
							//
							if (readReply.flagStateSuccess() != true)
							{
								readReceivedHeader.dumpFlagsState(m_Log);
								throw tr("Communication error.");
							}

							// Write frame dump to outut file
							//
							QString dataString;

							for (size_t i = 0 ; i < readData.size(); i++)
							{
								if (i % 32 == 0 && i != 0)
								{
									QString s = QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString;
									out << s << "\n";
									dataString.clear();
								}

								dataString += (i %16 ? " " : " ' ")  + QString().setNum(readData[i], 16).rightJustified(2, '0');

								if (i == readData.size() - 1 && i % 32 > 0)	// last iteration
								{
									QString s = QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString;
									out << s << "\n";
									dataString.clear();
								}
							}

							//m_Log->writeDump(readData);
						}
						break;
					default:
						m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
						throw tr("Communication error.");
					}

//					std::vector<uint8_t> writeData;
//					writeData.resize(blockSize, 0);

//					// Set Crc to databuffer
//					//
//					Crc::setDataBlockCrc(i, writeData.data(), static_cast<int>(writeData.size()));

//					// --
//					//
//					CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
//					if (send(moduleUartId, Write, i, blockSize, writeData, &replyHeader, &std::vector<uint8_t>()) == false)
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
//						replyHeader.dumpFlagsState(m_Log);
//						throw tr("Communication error.");
//					}
				}
			}
			break;
		default:
			assert(false);
		}

		// --
		//
		m_Log->writeSuccess(tr("Successful."));
	}
	catch (QString str)
	{
		m_Log->writeError(str);
	}


	// Close connection
	//
	if (closeConnection() == false)
	{
		m_Log->writeError(tr("CloseConnection failed."));
	}

	emit communicationFinished();
	return;
}

void Configurator::eraseFlashMemory(int)
{
	m_cancelFlag = false;

	emit communicationStarted();

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		emit communicationFinished();
		return;
	}

	try
	{
		//
		// PING command
		//
		std::vector<uint8_t> nopReply;
		CONF_HEADER pingReceivedHeader = CONF_HEADER();

		if (send(0, Nop, 0, 0, std::vector<uint8_t>(), &pingReceivedHeader, &nopReply) == false)
		{
			throw tr("Communication error.");
		}

		int protocolVersion = pingReceivedHeader.version;
		int moduleUartId = 0;
		int blockSize = 0;
		int romSize = 0;
		
		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				// Check if the connector in the configuartion UART
				//
				if ((pingReplyVersioned.moduleUartId & ConfigurationUartMask) != ConfigurationUartValue)
				{
					// Erase any memomy type
					//
					//throw tr("Wrong UART, use configuration port.");
				}

				protocolVersion = pingReplyVersioned.version;
				moduleUartId = pingReplyVersioned.moduleUartId;
				blockSize = pingReplyVersioned.blockSize;
				romSize = pingReplyVersioned.romSize;

				if (romSize % blockSize != 0)
				{
					throw tr("Flash memory block count is not an intergral multiple.");
				}

				// Ignore Wrong moduleUartId flag
				//
				pingReplyVersioned.flags &= ~OpDeniedInvalidModuleUartId;						// Ping was required to deremine moduleUartId

				// Check flags
				//
				if (pingReplyVersioned.flagStateSuccess() != true)
				{
					pingReplyVersioned.dumpFlagsState(m_Log);
					throw tr("Communication error.");
				}
			}
			break;
		default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
			throw tr("Communication error.");
		}

		assert(protocolVersion != 0);
		assert(moduleUartId != 0);
		assert(blockSize != 0);
		
		//
		// ERASE command (Write zeros)
		//

		// Disabel showing DebugInfo (too much information)
		bool oldStateShowDebugInfo = showDebugInfo();

		// restore ShwoDebugInfo state throw shared_ptr and deleter
		//
		auto deleter = [this, oldStateShowDebugInfo](bool*)
			{
				this->setShowDebugInfo(oldStateShowDebugInfo);
			};
		std::shared_ptr<bool> scopedRestoreShowDebugInfo(nullptr, deleter);

		setShowDebugInfo(false);

		switch (protocolVersion)
		{
		case 1:
			{
				m_Log->writeMessage("");

				int blockCount = romSize / blockSize;

				for (decltype(CONF_HEADER_V1().frameIndex) i = 0; i < blockCount; i++)
				{

					if (m_cancelFlag == true)
					{
						m_Log->writeMessage("Memory erasing cancelled.");
						break;
					}

					if (i == IdentificationFrameIndex)
					{
						m_Log->writeMessage(tr("Erasing block ") + QString().setNum(i) + " - skip identification block.");
						continue;
					}

					m_Log->writeMessage(tr("Erasing block ") + QString().setNum(i));

					std::vector<uint8_t> writeData;
					writeData.resize(blockSize, 0);

					// Set Crc to databuffer
					//
					Crc::setDataBlockCrc(i, writeData.data(), static_cast<int>(writeData.size()));

					// --
					//
					CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
					auto replyData = std::vector<uint8_t>();
					if (send(moduleUartId, Write, i, blockSize, writeData, &replyHeader, &replyData) == false)
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
						replyHeader.dumpFlagsState(m_Log);
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
		m_Log->writeSuccess(tr("Successful."));
	}
	catch (QString str)
	{
		m_Log->writeError(str);
	}

	// Close connection
	//
	if (closeConnection() == false)
	{
		m_Log->writeError(tr("CloseConnection failed."));
	}

	emit communicationFinished();
	return;
}

void Configurator::cancelOperation()
{
	m_cancelFlag = true;
}
