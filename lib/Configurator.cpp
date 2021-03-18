#include "Stable.h"
#include "../lib/Configurator.h"
#include "../lib/Crc.h"
#include <QtEndian>
#include <QHostInfo>
#include <cstring>

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
void CONF_IDENTIFICATION_DATA_V1::dump(QStringList& out) const
{
	out << QString("Identification struct version: %1").arg(structVersion());

	out << "Module Id: " + moduleUuid.toQUuid().toString();
	out << "Configuration counter: " + QString().setNum(count);

	out << "First time configured: ";
	out << "__Date: " + QDateTime().fromTime_t(static_cast<uint>(firstConfiguration.date)).toString();
	out << "__Host: " + QString(firstConfiguration.host);
	out << "__ConfigurationId: " + firstConfiguration.configurationId.toQUuid().toString();
	out << "__Configurator factory no: " + QString().setNum(firstConfiguration.configuratorFactoryNo);

	out << "Last time configured: ";
	out << "__Date: " + QDateTime().fromTime_t(static_cast<uint>(lastConfiguration.date)).toString();
	out << "__Host: " + QString(lastConfiguration.host);
	out << "__ConfigurationId: " + lastConfiguration.configurationId.toQUuid().toString();
	out << "__Configurator factory no: " + QString().setNum(lastConfiguration.configuratorFactoryNo);

	return;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // warning for std::strncpy
#endif

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
	std::strncpy(firstConfiguration.host, hostName.toStdString().data(), sizeof(firstConfiguration.host));

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
	std::strncpy(lastConfiguration.host, hostName.toStdString().data(), sizeof(lastConfiguration.host));

	lastConfiguration = lastConfiguration;
}

//
// CONF_IDENTIFICATION_DATA_V2
//
void CONF_IDENTIFICATION_DATA_V2::dump(QStringList& out) const
{
	out << QString("Identification struct version: %1").arg(structVersion());

	out << "Module Id: " + moduleUuid.toQUuid().toString();
	out << "Configuration counter: " + QString().setNum(count);

	out << "First time configured: ";
	out << "__Date: " + QDateTime().fromTime_t(static_cast<uint>(firstConfiguration.date)).toString();
	out << "__Host: " + QString(firstConfiguration.host);
	out << "__User: " + QString(firstConfiguration.userName);
	out << "__Build No: " + QString::number(firstConfiguration.buildNo).rightJustified(6, '0');
	out << "__Build Config: " + QString();	// Obsolete
	out << "__ConfigurationId: " + firstConfiguration.configurationId.toQUuid().toString();

	out << "Last time configured: ";
	out << "__Date: " + QDateTime().fromTime_t(static_cast<uint>(lastConfiguration.date)).toString();
	out << "__Host: " + QString(lastConfiguration.host);
	out << "__User: " + QString(lastConfiguration.userName);
	out << "__Build No: " + QString::number(lastConfiguration.buildNo).rightJustified(6, '0');
	out << "__Build Config: " + QString();	// Obsolete
	out << "__ConfigurationId: " + lastConfiguration.configurationId.toQUuid().toString();

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
	std::strncpy(firstConfiguration.host, hostName.toStdString().data(), sizeof(firstConfiguration.host));

	firstConfiguration.buildNo = storage->buildNumber();

	QString buildConfig;	// Obsolete
	std::strncpy(firstConfiguration.buildConfig, buildConfig.toStdString().data(), sizeof(firstConfiguration.buildConfig));

	QString userName = storage->userName().right(sizeof(firstConfiguration.userName) - 1);
	std::strncpy(firstConfiguration.userName, userName.toStdString().data(), sizeof(firstConfiguration.userName));

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
	std::strncpy(lastConfiguration.host, hostName.toStdString().data(), sizeof(lastConfiguration.host));

	lastConfiguration.buildNo = storage->buildNumber();

	QString buildConfig;	// Obsolete
	std::strncpy(lastConfiguration.buildConfig, buildConfig.toStdString().data(), sizeof(lastConfiguration.buildConfig));

	QString userName = storage->userName().right(sizeof(lastConfiguration.userName) - 1);
	std::strncpy(lastConfiguration.userName, userName.toStdString().data(), sizeof(lastConfiguration.userName));
}

#ifdef _MSC_VER
#pragma warning(pop)		//#pragma warning(disable: 3996) // warning for std::strncpy
#endif

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

	// !! Cannot create children for a parent that is in a different thread. !!

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

	case Nop2:
		{
			CONF_HEADER nop2Header = CONF_HEADER();

			nop2Header.version = static_cast<uint16_t>(ProtocolMaxVersion);
			//readHeader.moduleUartId = static_cast<uint16_t>(moduleUartId);
			nop2Header.opcode = static_cast<uint16_t>(opcode);
			//readHeader.frameIndex = static_cast<uint16_t>(frameIndex);
			nop2Header.setCrc();			// Calculate and set CRC for formed header

			buffer.resize(sizeof(nop2Header), 0);
			memcpy(buffer.data(), &nop2Header, sizeof(nop2Header));

			expecetedDataBytes = blockSize;
			headerSize = sizeof(nop2Header);

			if (showDebugInfo() == true)
			{
				m_Log->writeMessage("");
				m_Log->writeMessage(tr("Sending header, opcode Nop2:"));
				nop2Header.dump(m_Log);
			}
		}
		break;

	default:
		assert(false);
		m_Log->writeError(__FUNCTION__ + tr(" Unknown command %1.").arg(opcode));
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

	if (writtenBytes != static_cast<qint64>(buffer.size()))
	{
		m_Log->writeError(tr("Written bytes number %1, expected %2").arg(writtenBytes).arg(buffer.size()));
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

bool Configurator::requestUartInfo(CONF_HEADER* pingReceivedHeader, std::vector<int>* moduleUarts)
{
	if (pingReceivedHeader == nullptr || moduleUarts == nullptr)
	{
		Q_ASSERT(pingReceivedHeader);
		Q_ASSERT(moduleUarts);
		return false;
	}

	//
	// PING command
	//
	std::vector<quint8> nopReply;

	if (send(0, Nop2, 0, Nop2BlockSize, std::vector<quint8>(), pingReceivedHeader, &nopReply) == false)
	{
		return false;
	}

	int protocolVersion = pingReceivedHeader->version;

	switch (protocolVersion)
	{
	case 1:
		{
			CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(pingReceivedHeader);

			if ((pingReplyVersioned.flags & OpDeniedInvalidOpcode) == 0)
			{
				// Nop2 command is supported, fill lmUarts

				if (nopReply.size() != pingReplyVersioned.blockSize)
				{
					m_Log->writeError(tr("NOP2 command returned wrong number bytes of data: %1, expected: %2.").arg(nopReply.size()).arg(pingReplyVersioned.blockSize));
					return false;
				}

				quint16* wData = reinterpret_cast<quint16*>(nopReply.data());
				int wDataIndex = 0;

				int uartsCount = qFromBigEndian<quint16>(wData[wDataIndex++]);
				m_Log->writeMessage(tr("UART count: %1").arg(uartsCount));

				if ((uartsCount < 1) || (uartsCount > 16))
				{
					m_Log->writeError(tr("Invalid Uarts count, expected 1..16."));
					throw tr("Communication error.");
				}

				for (int i = 0; i < uartsCount; i++)
				{
					moduleUarts->push_back(qFromBigEndian<quint16>(wData[wDataIndex++]));
				}

				QStringList uartsList;
				for (int uart : *moduleUarts)
				{
					uartsList.push_back(tr("0x%1").arg(QString::number(uart, 16)));
				}
				m_Log->writeMessage(tr("Supported UARTs: ") + uartsList.join(' '));

				return true;
			}
		}
		break;
	default:
		m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
		throw tr("Communication error.");
	}

	if (moduleUarts->empty() == true)
	{
		// Nop2 command is not supported, send nop

		if (send(0, Nop, 0, 0, std::vector<quint8>(), pingReceivedHeader, &nopReply) == false)
		{
			return false;
		}

		protocolVersion = pingReceivedHeader->version;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(pingReceivedHeader);

				if ((pingReplyVersioned.flags & OpDeniedInvalidOpcode) == 0)
				{
					moduleUarts->push_back(pingReplyVersioned.moduleUartId);
				}
				else
				{
					m_Log->writeError("NOP command returned an error: Invalid opcode.");
					return false;
				}
			}
			break;
		default:
			m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
			return false;
		}
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

void Configurator::readServiceInformation(int param)
{
	emit operationStarted();

	readServiceInformationWorker(param);

	emit operationFinished();

	return;
}


bool Configurator::loadBinaryFileWorker(const QString& fileName, ModuleFirmwareStorage* storage, bool loadBinaryData)
{
	if (storage == nullptr)
	{
		assert(storage);
		return false;
	}


	QString errorCode;
	bool result = false;

	if (loadBinaryData == true)
	{
		m_Log->writeMessage(tr("Loading binary data..."));
		result = storage->load(fileName, &errorCode);
	}
	else
	{
		m_Log->writeMessage(tr("//----------------------"));
		m_Log->writeMessage(tr("File: %1").arg(fileName));
		result = storage->loadHeader(fileName, &errorCode);
	}

	if (result == false)
	{
		QString str = tr("File %1 wasn't loaded!").arg(fileName);
		if (errorCode.isEmpty() == false)
		{
			str += "\r\n\r\n" + errorCode;
		}

		m_Log->writeError(str);
		return result;
	}

	if (loadBinaryData == false)
	{
		m_Log->writeMessage(tr("File Version: %1").arg(storage->fileVersion()));
		m_Log->writeMessage(tr("ChangesetID: %1").arg(storage->changesetId()));
		m_Log->writeMessage(tr("Build User: %1").arg(storage->userName()));
		m_Log->writeMessage(tr("Build No: %1").arg(QString::number(storage->buildNumber())));
		m_Log->writeMessage(tr("Subsystems: %1").arg(storage->subsystemsString()));

		emit loadBinaryFileHeaderComplete();
	}

	return result;
}

void Configurator::uploadFirmwareWorker(ModuleFirmwareStorage *storage, const QString& subsystemId, std::optional<std::vector<int>> selectedUarts)
{
	m_Log->writeMessage(tr("Uploading binary data for subsystem %1").arg(subsystemId));

	//emit uploadFirmwareComplete(0x101);
	//return;

	m_cancelFlag = false;

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		return;
	}

	bool ok = false;
	Hardware::ModuleFirmware& conf = storage->firmware(subsystemId, &ok);

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
		CONF_HEADER pingReceivedHeader = CONF_HEADER();
		std::vector<int> moduleUarts;

		if (requestUartInfo(&pingReceivedHeader, &moduleUarts) == false)
		{
			throw tr("Communication error.");
		}

		if (moduleUarts.size() == 1 && moduleUarts[0] == ConfigurationUartId)
		{
			throw tr("Wrong UART, use Bitstream Configuration port.");
		}

		//
		// Parse PING Header Info
		//
		int protocolVersion = pingReceivedHeader.version;
		uint16_t payloadSize = 0;
		uint16_t blockSize = 0;
		int romSize = 0;
		int blockCount = 0;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				protocolVersion = pingReplyVersioned.version;
				payloadSize = pingReplyVersioned.frameSize;
				blockSize = pingReplyVersioned.blockSize;
				romSize = pingReplyVersioned.romSize;
				if (payloadSize != 0)
				{
					blockCount = romSize / blockSize;
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
		assert(blockSize != 0);

		m_Log->writeMessage("PING Reply:");
		m_Log->writeMessage(tr("Protocol Version: %1").arg(protocolVersion));
		m_Log->writeMessage(tr("Frame Payload Size: %1").arg(QString::number(payloadSize)));
		m_Log->writeMessage(tr("Frame Size: %1").arg(blockSize));
		m_Log->writeMessage(tr("Frame Count: %1").arg(QString::number(blockCount)));

		//
		// Loop for all uarts
		//
		for (int moduleUartId : moduleUarts)
		{
			// Skip configuration UART
			//
			if (moduleUartId == ConfigurationUartId)
			{
				continue;
			}

			// Skip UART if it is not checked
			//
			if (selectedUarts.has_value() == true)
			{
				const std::vector<int>& selectedUartsValue = selectedUarts.value();
				if (std::find(selectedUartsValue.begin(), selectedUartsValue.end(), moduleUartId) == selectedUartsValue.end())
				{
					m_Log->writeWarning0(tr("Uart ID = 0x%1 is skipped.").arg(QString::number(moduleUartId, 16)));
					continue;
				}
			}

			if (conf.uartExists(moduleUartId) == false)
			{
				throw tr("No firmware data exists for current UART ID = %1h.").arg(QString::number(moduleUartId, 16));
			}

			emit uartOperationStart(moduleUartId, "Uploading");

			m_Log->writeEmptyLine();
			m_Log->writeMessage(QString("UartId: %1 (%2h)").arg(moduleUartId).arg(moduleUartId, 4, 16, QLatin1Char('0')));

			int confFrameDataSize = conf.eepromFramePayloadSize(moduleUartId);
			if (payloadSize < confFrameDataSize)
			{
				throw tr("EEPROM Frame size is to small, requeried at least %1, but current frame size is %2.").arg(confFrameDataSize).arg(payloadSize);
			}

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

					if (bool sendOk = send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<uint8_t>(), &readReceivedHeader, &identificationData);
						sendOk == false)
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
			std::vector<uint8_t> replyData;
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

					for (uint16_t i = 0; i < conf.eepromFrameCount(moduleUartId); i++)
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

						const std::vector<quint8> frameData = conf.frame(moduleUartId, i);

						if (frameData.size() != blockSize)
						{
							throw tr("Frame %1 size (%2) does not match module's BlockSize (%3).").arg(i).arg(frameData.size()).arg(blockSize);
						}

						// Write data
						//
						CONF_HEADER_V1 replyHeader = CONF_HEADER_V1();
						std::vector<uint8_t> replyBuffer;
						bool sendResult = send(moduleUartId, Write, frameIndex, blockSize, frameData, &replyHeader, &replyBuffer);

						if (sendResult == false)
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

			emit uploadFirmwareComplete(moduleUartId);
		}
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

void Configurator::readServiceInformationWorker(int /*param*/)
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
		uint16_t blockSize = 0;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				protocolVersion = pingReplyVersioned.version;
				moduleUartId = pingReplyVersioned.moduleUartId;
				blockSize = pingReplyVersioned.blockSize;

				// Check if the connector in the configuartion UART
				//
				if (moduleUartId  != ConfigurationUartId)
				{
					throw tr("Wrong UART, use Service Configuration port.");
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
		// READ indentification block
		//
		m_Log->writeMessage(tr("Read identification block."));

		bool identificationError = false;
		std::vector<quint8> identificationData;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 readReceivedHeader = CONF_HEADER_V1();

				if (bool sendOk = send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &identificationData);
					sendOk == false)
				{
					throw tr("Communication error.");
				}

				assert(protocolVersion == readReceivedHeader.version);

				// Ignoring all flags, CRC, etc
				//

				QStringList dumpLog;
				dumpIdentificationData(identificationData, blockSize, dumpLog, &identificationError);

				for (const auto& s : dumpLog)
				{
					m_Log->writeMessage(s);
				}
			}
			break;
		default:
			assert(false);
		}

		if (identificationError == true)
		{
			throw tr("Identification error.");
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

bool Configurator::readFirmwareWorker(std::vector<ModuleFirmwareData>* firmwareDataArray, int maxFrameCount, std::optional<std::vector<int>> selectedUarts)
{
	if (firmwareDataArray == nullptr)
	{
		assert(firmwareDataArray);
		return false;
	}

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		emit operationFinished();
		return false;
	}

	bool result = true;

	try
	{
		//
		// PING command
		//
		CONF_HEADER pingReceivedHeader = CONF_HEADER();
		std::vector<int> moduleUarts;

		if (requestUartInfo(&pingReceivedHeader, &moduleUarts) == false)
		{
			throw tr("Communication error.");
		}

		//
		// Parse PING Header Info
		//
		int protocolVersion = pingReceivedHeader.version;
		uint16_t payloadSize = 0;
		uint16_t blockSize = 0;
		int romSize = 0;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				protocolVersion = pingReplyVersioned.version;
				payloadSize = pingReplyVersioned.frameSize;
				blockSize = pingReplyVersioned.blockSize;
				romSize = pingReplyVersioned.romSize;

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
		assert(payloadSize != 0);
		assert(romSize != 0);

		int blockCount = romSize / blockSize;

		m_Log->writeMessage("PING Reply:");
		m_Log->writeMessage(tr("Protocol Version: %1").arg(protocolVersion));
		m_Log->writeMessage(tr("Frame Payload Size: %1").arg(QString::number(payloadSize)));
		m_Log->writeMessage(tr("Frame Size: %1").arg(blockSize));
		m_Log->writeMessage(tr("Frame Count: %1").arg(QString::number(blockCount)));

		//
		// Loop for all uarts
		//
		for (int moduleUartId : moduleUarts)
		{
			if (moduleUartId == ConfigurationUartId)
			{
				// Skip service UART
				continue;
			}

			// Skip UART if it is not checked
			//
			if (selectedUarts.has_value() == true)
			{
				const std::vector<int>& selectedUartsValue = selectedUarts.value();
				if (std::find(selectedUartsValue.begin(), selectedUartsValue.end(), moduleUartId) == selectedUartsValue.end())
				{
					m_Log->writeWarning0(tr("Uart ID = 0x%1 is skipped.").arg(QString::number(moduleUartId, 16)));
					continue;
				}
			}

			ModuleFirmwareData firmwareData;

			emit uartOperationStart(moduleUartId, "Reading");

			// Write log and output file
			//

			m_Log->writeEmptyLine();
			m_Log->writeMessage(QString("UartId: %1 (%2h)").arg(moduleUartId).arg(moduleUartId, 4, 16, QLatin1Char('0')));

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

			firmwareData.uartId = moduleUartId;
			firmwareData.eepromFrameSize = blockSize;
			firmwareData.frames.resize(blockCount);

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

						if (maxFrameCount != -1 && i >= maxFrameCount)
						{
							break;
						}

						m_Log->writeMessage(tr("Reading block ") + QString().setNum(i));

						std::vector<quint8>& readData = firmwareData.frames[i];
						CONF_HEADER readReceivedHeader = CONF_HEADER();

						if (bool sendOk = send(moduleUartId, Read, i, blockSize, std::vector<quint8>(), &readReceivedHeader, &readData);
							sendOk == false)
						{
							throw tr("Communication error.");
						}

						assert(protocolVersion == readReceivedHeader.version);

						if (i == 0)
						{
							QStringList dumpLog;
							bool identificationError = false;

							dumpIdentificationData(readData, blockSize, dumpLog, &identificationError);

							for (const auto& s : dumpLog)
							{
								m_Log->writeMessage(s);
							}
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
							}
							break;
						default:
							m_Log->writeError(tr("Unsupported protocol version, module protocol version: ") + QString().setNum(protocolVersion) + tr(", the maximum supported version: ") + QString().setNum(ProtocolMaxVersion) + ".");
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
			firmwareDataArray->push_back(firmwareData);

		} // moduleUarts
	}// try

	catch (QString str)
	{
		m_Log->writeError(str);
		result = false;
	}

	// Close connection
	//
	if (closeConnection() == false)
	{
		m_Log->writeError(tr("CloseConnection failed."));
		result = false;
	}

	return result;
}

void Configurator::dumpIdentificationData(const std::vector<quint8>& identificationData, int blockSize, QStringList& out, bool* error)
{
	if (error == nullptr)
	{
		Q_ASSERT(error);
		return;
	}

	*error = false;

	if (identificationData.size() != blockSize)
	{
		out << tr("Identification block is empty.");
	}

	const CONF_IDENTIFICATION_DATA* pReadIdentificationStruct = reinterpret_cast<const CONF_IDENTIFICATION_DATA*>(identificationData.data());
	if (pReadIdentificationStruct->marker == IdentificationStructMarker)
	{
		switch (pReadIdentificationStruct->version)
		{
		case 1:
			{
				const CONF_IDENTIFICATION_DATA_V1* pReadIdentificationStruct_v1 = reinterpret_cast<const CONF_IDENTIFICATION_DATA_V1*>(identificationData.data());
				pReadIdentificationStruct_v1->dump(out);
			}
			break;
		case 2:
			{

				const CONF_IDENTIFICATION_DATA_V2* pReadIdentificationStruct_v2 = reinterpret_cast<const CONF_IDENTIFICATION_DATA_V2*>(identificationData.data());
				pReadIdentificationStruct_v2->dump(out);
			}
			break;
		default:
			out << tr("Unknown identification block version: ") + QString().setNum(pReadIdentificationStruct->version);
			*error = true;
		}
	}
	else
	{
		out << tr("Wrong identification block, marker: ") + QString().setNum(pReadIdentificationStruct->marker, 16);
		*error = true;
	}

	return;
}


void Configurator::uploadServiceInformation(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc)
{
	emit operationStarted();

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		emit operationFinished();
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
		uint16_t blockSize = 0;
		int frameSize = 0;
		
		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				protocolVersion = pingReplyVersioned.version;
				moduleUartId = pingReplyVersioned.moduleUartId;
				blockSize = pingReplyVersioned.blockSize;
				frameSize = pingReplyVersioned.frameSize;

				// Check if the connector in the configuartion UART
				//
				if (moduleUartId  != ConfigurationUartId)
				{
					throw tr("Wrong UART, use Service Configuration port.");
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
		
		std::vector<quint8> identificationData;
		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 readReceivedHeader = CONF_HEADER_V1();

				if (bool sendOk = send(moduleUartId, Read, IdentificationFrameIndex, blockSize, std::vector<quint8>(), &readReceivedHeader, &identificationData);
					sendOk == false)
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
		storage.setProjectInfo("projectName", userName, 0, 0);

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
		std::vector<uint8_t> replyDatBuffera;
		if (send(moduleUartId, Write, IdentificationFrameIndex, blockSize, identificationData, &replyIdentificationHeader, &replyDatBuffera) == false)
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
				writeServiceStruct.setManufactureYear(static_cast<uint16_t>(manufactureDate.year()));
				writeServiceStruct.setManufactureMonth(static_cast<uint16_t>(manufactureDate.month()));
				writeServiceStruct.setManufactureDay(static_cast<uint16_t>(manufactureDate.day()));
				writeServiceStruct.setConfigureYear(static_cast<uint16_t>(now.year()));
				writeServiceStruct.setConfigureMonth(static_cast<uint16_t>(now.month()));
				writeServiceStruct.setConfigureDay(static_cast<uint16_t>(now.day()));
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
				std::vector<uint8_t> replyDataV1;
				if (send(moduleUartId, Write, ConfiguartionFrameIndex, blockSize, writeData, &replyHeader, &replyDataV1) == false)
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

	emit operationFinished();
	return;
}

void Configurator::loadBinaryFile(const QString& fileName, ModuleFirmwareStorage* storage)
{
	m_fileName = fileName;

	emit operationStarted();

	loadBinaryFileWorker(fileName, storage, false);

	emit operationFinished();
}

void Configurator::uploadFirmware(ModuleFirmwareStorage *storage, const QString& subsystemId, std::optional<std::vector<int>> selectedUarts)
{
	m_cancelFlag = false;

	emit operationStarted();

	// If no binary data was loaded, load it
	//
	if (storage->hasBinaryData() == false)
	{
		if (loadBinaryFileWorker(m_fileName, storage, true) == false)
		{
			emit operationFinished();
			return;
		}
	}

	uploadFirmwareWorker(storage, subsystemId, selectedUarts);

	emit operationFinished();
}

void Configurator::readFirmware(const QString& fileName, std::optional<std::vector<int>> selectedUarts)
{
	m_cancelFlag = false;

	emit operationStarted();

	// Open outputFile
	//
	QFile file(fileName);

	if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
	{
		m_Log->writeError(tr("Cannot open output file %1, %2").arg(fileName).arg(file.error()));
		emit operationFinished();
		return;
	}

	QTextStream out(&file);

	std::vector<ModuleFirmwareData> fdArray;

	if (readFirmwareWorker(&fdArray, -1, selectedUarts) == false)
	{
		emit operationFinished();
		return;
	}

	for (const ModuleFirmwareData& fd : fdArray)
	{
		out << QString("UartId: %1 (%2h)\n").arg(fd.uartId).arg(fd.uartId, 4, 16, QLatin1Char('0'));
		out << QString("EEPROM frame size: %1 (%2h)\n").arg(fd.eepromFrameSize).arg(fd.eepromFrameSize, 4, 16, QLatin1Char('0'));
		out << QString("EEPROM frames count: %1\n").arg(fd.frames.size());

		for (int f = 0; f < fd.frames.size(); f++)
		{
			out << "FrameIndex " << f << "\n";

			const std::vector<quint8>& frame = fd.frames[f];

			if (f == 0)
			{
				QStringList dumpLog;
				bool identificationError = false;

				dumpIdentificationData(frame, static_cast<int>(frame.size()), dumpLog, &identificationError);

				for (const auto& s : dumpLog)
				{
					out << s << "\n";
				}
			}

			// Write frame dump to outut file
			//
			QString dataString;

			for (size_t i = 0; i < frame.size(); i++)
			{
				if ((i % 32) == 0 && i != 0)
				{
					QString s = QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString;
					out << s << "\n";
					dataString.clear();
				}

				dataString += ((i % 16) ? " " : " ' ")  + QString().setNum(frame[i], 16).rightJustified(2, '0');

				if (i == frame.size() - 1 && i % 32 > 0)	// last iteration
				{
					QString s = QString().setNum(i - 32, 16).rightJustified(4, '0') + ":" + dataString;
					out << s << "\n";
					dataString.clear();
				}
			}
		}

		out << "\n";
	}

	m_Log->writeSuccess(tr("Successful."));

	emit operationFinished();

	return;
}

void Configurator::eraseFlashMemory(int, std::optional<std::vector<int>> selectedUarts)
{
	m_cancelFlag = false;

	emit operationStarted();

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		emit operationFinished();
		return;
	}

	try
	{
		//
		// PING command
		//
		CONF_HEADER pingReceivedHeader = CONF_HEADER();
		std::vector<int> moduleUarts;

		if (requestUartInfo(&pingReceivedHeader, &moduleUarts) == false)
		{
			throw tr("Communication error.");
		}

		//
		// Parse PING Header Info
		//
		int protocolVersion = pingReceivedHeader.version;
		uint16_t blockSize = 0;
		int romSize = 0;

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_HEADER_V1 pingReplyVersioned = *reinterpret_cast<CONF_HEADER_V1*>(&pingReceivedHeader);

				protocolVersion = pingReplyVersioned.version;
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
		assert(blockSize != 0);

		int blockCount = romSize / blockSize;

		m_Log->writeMessage("PING Reply:");
		m_Log->writeMessage(tr("Protocol Version: %1").arg(protocolVersion));
		m_Log->writeMessage(tr("Frame Size: %1").arg(blockSize));
		m_Log->writeMessage(tr("Frame Count: %1").arg(QString::number(blockCount)));
		//
		// Loop for all uarts
		//
		for (int moduleUartId : moduleUarts)
		{
			// Skip UART if it is not checked
			//
			if (selectedUarts.has_value() == true)
			{
				const std::vector<int>& selectedUartsValue = selectedUarts.value();
				if (std::find(selectedUartsValue.begin(), selectedUartsValue.end(), moduleUartId) == selectedUartsValue.end())
				{
					m_Log->writeWarning0(tr("Uart ID = 0x%1 is skipped.").arg(QString::number(moduleUartId, 16)));
					continue;
				}
			}

			m_Log->writeMessage(tr("Processing Uart ID = 0x%1").arg(QString::number(moduleUartId, 16)));

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

						if (bool sendOk = send(moduleUartId, Write, i, blockSize, writeData, &replyHeader, &replyData);
							sendOk == false)
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

		} // moduleUarts
	}// try
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

	emit operationFinished();
	return;
}

void Configurator::detectSubsystem_v1()
{
	m_cancelFlag = false;

	emit operationStarted();

	std::vector<ModuleFirmwareData> fdArray;

	const int readFramesCount = 2;

	if (readFirmwareWorker(&fdArray, readFramesCount, {}) == false)
	{
		emit operationFinished();
		return;
	}

	if (fdArray.empty() == true)
	{
		return;
	}

	bool ssKeyFirst = true;
	quint16 ssKeyGlobal = 0;

	for (const ModuleFirmwareData& fd : fdArray)
	{
		if (fd.frames.size() < readFramesCount)
		{
			m_Log->writeError(QString("EEPROM frames is less than expected: %1").arg(readFramesCount));
			emit operationFinished();
			return;
		}

		const int formatFrameIndex = 1;

		const std::vector<quint8>& formatFrame = fd.frames[formatFrameIndex];

		const quint16* dataPtr = (quint16*)formatFrame.data();

		quint16 marker = qFromBigEndian(*dataPtr++);
		quint16 version = qFromBigEndian(*dataPtr++);

		if (marker != 0xCA70 && version != 1)
		{
			m_Log->writeError(QString("Wrong storage format marker (0x%1) or version (%2), expected 0xca70, version 1.")
							  .arg(QString::number(marker, 16))
							  .arg(version));
			emit operationFinished();
			return;
		}

		quint16 ssKey = qFromBigEndian(*dataPtr++) >> 6;

		if (ssKeyFirst == true)
		{
			ssKeyGlobal = ssKey;
			ssKeyFirst = false;
		}
		else
		{
			if (ssKey != ssKeyGlobal)
			{
				m_Log->writeError(QString("Subsystem key is not the same in EEPROM areas."));
				emit operationFinished();
				return;
			}
		}
	}

	emit detectSubsystemComplete(ssKeyGlobal);
	emit operationFinished();

	return;
}

void Configurator::detectUarts()
{
	m_Log->writeMessage(tr("Detecting UARTs supported by module..."));

	m_cancelFlag = false;

	emit operationStarted();

	// Open port
	//
	if (openConnection() == false)
	{
		m_Log->writeError(tr("Cannot open ") + device() + ".");
		emit operationFinished();
		return;
	}

	try
	{
		CONF_HEADER pingReceivedHeader = CONF_HEADER();

		std::vector<int> uartIds;

		if (requestUartInfo(&pingReceivedHeader, &uartIds) == false)
		{
			throw tr("Communication error.");
		}

		emit detectUartsComplete(uartIds);

		m_Log->writeSuccess(tr("Successful."));
	}

	catch (QString str)
	{
		m_Log->writeError(str);
	}

	emit operationFinished();

	// Close connection
	//
	if (closeConnection() == false)
	{
		m_Log->writeError(tr("CloseConnection failed."));
	}

	return;
}

void Configurator::cancelOperation()
{
	m_cancelFlag = true;
}
