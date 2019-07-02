#include "Options.h"

#include <QSettings>

#include "../../lib/Crc.h"

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TestResultOption::TestResultOption(QObject *parent) :
	QObject(parent),
	m_maxPacketCount(MAX_PACKET_COUNT_FOR_TEST),
	m_testFinisedCount(0)
{
	m_fileName.clear();

	m_moduleID.clear();
	m_operatorName.clear();
}

// -------------------------------------------------------------------------------------------------------------------

TestResultOption::TestResultOption(const TestResultOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

TestResultOption::~TestResultOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultOption::clear()
{
	m_fileName.clear();
	m_maxPacketCount = MAX_PACKET_COUNT_FOR_TEST;

	m_moduleID.clear();
	m_operatorName.clear();

	m_testFinisedCount = 0;
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultOption::updateTestFinisedCount()
{
	m_testFinisedCount++;

	if (m_testFinisedCount == SERIAL_PORT_COUNT)
	{
		emit testFinished();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultOption::load()
{
	QSettings s;

	m_fileName = s.value(QString("%1FileName").arg(TESTRESULT_OPTIONS_REG_KEY), RESULT_FILE_NAME).toString();
	m_maxPacketCount = s.value(QString("%1PacketCount").arg(TESTRESULT_OPTIONS_REG_KEY), MAX_PACKET_COUNT_FOR_TEST).toInt();

	m_moduleID = s.value(QString("%1ModuleID").arg(TESTRESULT_OPTIONS_REG_KEY), "0001").toString();
	m_operatorName = s.value(QString("%1OperatorName").arg(TESTRESULT_OPTIONS_REG_KEY), "Operator name").toString();
}

// -------------------------------------------------------------------------------------------------------------------

void TestResultOption::save()
{
	QSettings s;

	s.setValue(QString("%1FileName").arg(TESTRESULT_OPTIONS_REG_KEY), m_fileName);
	s.setValue(QString("%1PacketCount").arg(TESTRESULT_OPTIONS_REG_KEY), m_maxPacketCount);

	s.setValue(QString("%1ModuleID").arg(TESTRESULT_OPTIONS_REG_KEY), m_moduleID);
	s.setValue(QString("%1OperatorName").arg(TESTRESULT_OPTIONS_REG_KEY), m_operatorName);
}

// -------------------------------------------------------------------------------------------------------------------

TestResultOption& TestResultOption::operator=(const TestResultOption& from)
{
	m_fileName = from.m_fileName;
	m_maxPacketCount = from.m_maxPacketCount;

	m_moduleID = from.m_moduleID;
	m_operatorName = from.m_operatorName;

	m_testFinisedCount = from.m_testFinisedCount;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SerialPortOption::SerialPortOption(QObject *parent) :
	QObject(parent),
	m_connected(false),
	m_noReply(true),
	m_type( RS_TYPE_UNKNOWN),
	m_baudRate(0),
	m_receivedBytes(0),
	m_skippedBytes(0),
    m_queueBytes(0),
    m_packetCount(0)
{
	m_portName.clear();
	setDataSize(SERIAL_PORT_HEADER_SIZE+MIN_DATA_SIZE);
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortOption::SerialPortOption(const SerialPortOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortOption::~SerialPortOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::clear()
{
	m_connected = false;
	m_noReply = true;

	m_portName.clear();
	m_type = RS_TYPE_UNKNOWN;
	m_baudRate = 0;

	setDataSize(0);

	m_receivedBytes = 0;
	m_skippedBytes = 0;
	m_queueBytes = 0;
    m_packetCount = 0;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::setNoReply(bool noReply)
{
	m_noReply = noReply;

	// for test
	//
	if (m_testResult.isRunning() == true)
	{
		if (m_noReply == true)
		{
            saveTestResult();
		}
		else
		{
            if (m_testResult.resultIsClear() == false)
            {
                clearTestResult();
            }
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::setConnected(bool connected)
{
	if (m_connected != connected)
	{
		emit connectChanged();
	}

	m_connected = connected;
}

// -------------------------------------------------------------------------------------------------------------------

QString SerialPortOption::typeStr() const
{
	if (m_type < 0 || m_type >= RS_TYPE_COUNT)
	{
		return QString ();
	}

	return RsType[m_type];
}

// -------------------------------------------------------------------------------------------------------------------

QString SerialPortOption::dataSizeStr() const
{
	QString str;

	int size = m_dataSize - SERIAL_PORT_HEADER_SIZE;

	if (theOptions.view().showInWord() == true)
	{
		str = QString::number(size/2) + tr(" words");
	}
	else
	{
		str = QString::number(size) + tr(" bytes");
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::setDataSize(int size)
{
	m_dataMutex.lock();

		if (size > MAX_DATA_SIZE)
		{
			size = MAX_DATA_SIZE;
		}

		m_dataSize = size;

		m_data.resize(m_dataSize);
		m_data.fill(0);

	m_dataMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortDataHeader* SerialPortOption::dataHeader()
{
	SerialPortDataHeader* pHeader = nullptr;

	m_dataMutex.lock();

		pHeader = (SerialPortDataHeader*) m_data.data();

	m_dataMutex.unlock();

	return pHeader;
}

// -------------------------------------------------------------------------------------------------------------------

quint16 SerialPortOption::data(int index)
{
	int dataSize = theOptions.view().showInWord() == true ? m_dataSize/2 : m_dataSize;

	if (theOptions.view().showHeader() == false)
	{
		index += theOptions.view().showInWord() == true ? SERIAL_PORT_HEADER_SIZE/2 : SERIAL_PORT_HEADER_SIZE;
	}

	if (index < 0 || index >= dataSize)
	{
		return 0;
	}

	quint16 aData = 0;

	m_dataMutex.lock();

		if (theOptions.view().showInWord() == true)
		{
			aData = *((quint16*) m_data.data() + index);
		}
		else
		{
			aData = *((quint8*) m_data.data() + index);
		}

	m_dataMutex.unlock();

	return aData;
}

// -------------------------------------------------------------------------------------------------------------------

quint64 SerialPortOption::dataCRC()
{
    if (m_dataSize < (SERIAL_PORT_HEADER_SIZE+MIN_DATA_SIZE))
	{
		return 0;
	}

	quint64 crc = 0;

	m_dataMutex.lock();

		crc = *(quint64*) (m_data.data() + (m_dataSize - sizeof(quint64)));

	m_dataMutex.unlock();

	return crc;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::setData(const QByteArray& arr)
{
	m_dataMutex.lock();

		m_data = arr;

	m_dataMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

QString SerialPortOption::receivedBytesStr()
{
    QString str;

	if (m_noReply == false)
    {
        str = QString::number((double) m_receivedBytes/1024, 10, 2) + tr(" Kb");
    }
	else
	{
		str = tr("No data");
	}

    return str;
}

// -------------------------------------------------------------------------------------------------------------------

QString SerialPortOption::skippedBytesStr()
{
    QString str;

	if (m_noReply == false)
    {
		str = QString::number((double) m_skippedBytes/1024, 10, 2) + tr(" Kb");
    }

    return str;
}

// -------------------------------------------------------------------------------------------------------------------

QString SerialPortOption::queueBytesStr()
{
    QString str;

	if (m_noReply == false)
    {
        str = QString::number((double) m_queueBytes/1024, 10, 2) + tr(" Kb");
    }

    return str;
}

// -------------------------------------------------------------------------------------------------------------------

QString SerialPortOption::packetCountStr()
{
	QString str;

	if (m_noReply == false)
	{
		str = QString::number(m_packetCount);
	}

	return str;
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortOption::isDataUidOk() const
{
	bool duidOk = false;

	m_dataMutex.lock();

		quint32 dataUID = *reinterpret_cast<const quint32*>(m_data.constData() + SERIAL_PORT_HEADER_SIZE);

		const SerialPortDataHeader* pHeader = reinterpret_cast<const SerialPortDataHeader*>(m_data.constData());

		if (pHeader->DataUID == SWAP_4_BYTES(dataUID))
		{
			duidOk = true;
		}

	m_dataMutex.unlock();

	return duidOk;
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortOption::isHeaderCrcOk() const
{
    bool crcOk = false;

    m_dataMutex.lock();

		const SerialPortDataHeader* pHeader = reinterpret_cast<const SerialPortDataHeader*>(m_data.constData());

		if (pHeader->CRC64 == SWAP_8_BYTES(Crc::crc64(m_data.data(), SERIAL_PORT_HEADER_SIZE - sizeof(quint64))))
        {
            crcOk = true;
        }

    m_dataMutex.unlock();

    return crcOk;
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortOption::isDataCrcOk() const
{
    bool crcOk = false;

	m_dataMutex.lock();

		const quint64 dataCRC64 = *reinterpret_cast<const quint64*>(m_data.constData() + (static_cast<const quint32>(m_dataSize) - sizeof(quint64)));

		if (dataCRC64 == SWAP_8_BYTES(Crc::crc64((m_data.data() + SERIAL_PORT_HEADER_SIZE), static_cast<const quint32>(m_dataSize) - SERIAL_PORT_HEADER_SIZE - sizeof(quint64))))
        {
            crcOk = true;
        }

	m_dataMutex.unlock();

	return crcOk;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::runTest()
{
	m_testResult.setIsRunning(true);
	m_testResult.setResultIsClear(false);
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::clearTestResult()
{
	m_receivedBytes = 0;
	m_skippedBytes = 0;
	m_packetCount = 0;

	m_testResult.clear();
	m_testResult.setStartTime();

    m_testResult.setResultIsClear(true);
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::saveTestResult()
{
	m_testResult.setReceivedBytes(m_receivedBytes);
	m_testResult.setSkippedBytes(m_skippedBytes);
	m_testResult.setPacketCount(m_packetCount);

	m_testResult.setStopTime();

	m_testResult.setIsRunning(false);
	m_testResult.setResultIsClear(false);

	bool result = m_skippedBytes * 100.0 / m_receivedBytes < MAX_SKIPPED_BYTES_IN_PERCENTAGES &&
				  100 - (m_packetCount * 100 / theOptions.testOption().maxPacketCount()) < MAX_SKIPPED_BYTES_IN_PERCENTAGES;

	m_testResult.setIsOk(result);

	theOptions.testOption().updateTestFinisedCount();
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::load(int index)
{
	if (index < 0 || index >= SERIAL_PORT_COUNT)
	{
		return;
	}

	QSettings s;
	QString portKey = tr("SerialPort%1/").arg(index);
	QString portName = tr("COM%1").arg(index+1);

	m_portName = s.value(QString("%1%2Name").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), portName).toString();
	m_type = s.value(QString("%1%2Type").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), RS_TYPE_232).toInt();
	m_baudRate = s.value(QString("%1%2BaudRate").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), QSerialPort::Baud115200).toInt();
	m_dataSize = s.value(QString("%1%2DataSize").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), SERIAL_PORT_HEADER_SIZE+MIN_DATA_SIZE).toInt();

	setDataSize(m_dataSize);
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortOption::save(int index)
{
	if (index < 0 || index >= SERIAL_PORT_COUNT)
	{
		return;
	}

	QSettings s;
	QString portKey = tr("SerialPort%1/").arg(index);

	s.setValue(QString("%1%2Name").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), m_portName);
	s.setValue(QString("%1%2Type").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), m_type);
	s.setValue(QString("%1%2BaudRate").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), m_baudRate);
	s.setValue(QString("%1%2DataSize").arg(SERIALPORTS_OPTIONS_REG_KEY).arg(portKey), m_dataSize);
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortOption& SerialPortOption::operator=(const SerialPortOption& from)
{
	m_connected = from.m_connected;
	m_noReply = from.m_noReply;

	m_portName = from.m_portName;
	m_type = from.m_type;
	m_baudRate = from.m_baudRate;

	setDataSize(from.m_dataSize);
	setData(from.m_data);

	m_receivedBytes = from.m_receivedBytes;
	m_skippedBytes = from.m_skippedBytes;
	m_queueBytes = from.m_queueBytes;
    m_packetCount = from.m_packetCount;

	m_testResult = from.m_testResult;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SerialPortsOption::SerialPortsOption(QObject *parent) :
	QObject(parent),
	m_dataSize(0)
{
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortsOption::SerialPortsOption(const SerialPortsOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortsOption::~SerialPortsOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortsOption::clear()
{
	m_mutex.lock();

		for(int i = 0; i < SERIAL_PORT_COUNT; i++)
		{
			m_serialPort[i].clear();
		}

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortOption* SerialPortsOption::port(int index)
{
	if (index < 0 || index >= SERIAL_PORT_COUNT)
	{
		return nullptr;
	}

	return &m_serialPort[index];
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortsOption::setPort(int index, const SerialPortOption& portOption)
{
	if (index < 0 || index >= SERIAL_PORT_COUNT)
	{
		return;
	}

	m_mutex.lock();

		m_serialPort[index] = portOption;

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SerialPortsOption::recalcDataSize()
{
	m_mutex.lock();

		m_dataSize = 0;

		for(int i = 0; i < SERIAL_PORT_COUNT; i++)
		{
			if ( m_dataSize < m_serialPort[i].dataSize())
			{
				m_dataSize = m_serialPort[i].dataSize();
			}
		}

	m_mutex.unlock();

	return m_dataSize;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortsOption::load()
{
	for(int i = 0; i < SERIAL_PORT_COUNT; i++)
	{
		m_serialPort[i].load(i);
	}

	recalcDataSize();
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortsOption::save()
{
	for(int i = 0; i < SERIAL_PORT_COUNT; i++)
	{
		m_serialPort[i].save(i);
	}
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortsOption& SerialPortsOption::operator=(const SerialPortsOption& from)
{
	m_mutex.lock();

		for(int i = 0; i < SERIAL_PORT_COUNT; i++)
		{
			m_serialPort[i] = from.m_serialPort[i];
		}

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ViewOption::ViewOption(QObject *parent) :
	QObject(parent),
	m_showHeader(false),
	m_showInWord(false),
	m_showInHex(false),
	m_showInFloat(false)
{
}

// -------------------------------------------------------------------------------------------------------------------

ViewOption::ViewOption(const ViewOption& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

ViewOption::~ViewOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ViewOption::load()
{
	QSettings s;

	m_showHeader = s.value(QString("%1ShowHeader").arg(VIEW_OPTIONS_REG_KEY), false).toBool();
	m_showInWord = s.value(QString("%1ShowInWord").arg(VIEW_OPTIONS_REG_KEY), false).toBool();
	m_showInHex = s.value(QString("%1ShowInHex").arg(VIEW_OPTIONS_REG_KEY), false).toBool();
	m_showInFloat = s.value(QString("%1ShowInFloat").arg(VIEW_OPTIONS_REG_KEY), false).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void ViewOption::save()
{
	QSettings s;

	s.setValue(QString("%1ShowHeader").arg(VIEW_OPTIONS_REG_KEY), m_showHeader);
	s.setValue(QString("%1ShowInWord").arg(VIEW_OPTIONS_REG_KEY), m_showInWord);
	s.setValue(QString("%1ShowInHex").arg(VIEW_OPTIONS_REG_KEY), m_showInHex);
	s.setValue(QString("%1ShowInFloat").arg(VIEW_OPTIONS_REG_KEY), m_showInFloat);
}

// -------------------------------------------------------------------------------------------------------------------

ViewOption& ViewOption::operator=(const ViewOption& from)
{
	m_showHeader = from.m_showHeader;
	m_showInWord = from.m_showInWord;
	m_showInHex = from.m_showInHex;
	m_showInFloat = from.m_showInFloat;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject *parent) :
	QObject(parent)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

Options::~Options()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Options::load()
{
	m_testResult.load();
	m_serialPorts.load();
	m_view.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
	m_testResult.save();
	m_serialPorts.save();
	m_view.save();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::unload()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool Options::readFromXml()
{
	bool result = false;

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
	m_mutex.lock();

		m_testResult = from.m_testResult;
		m_serialPorts = from.m_serialPorts;
		m_view = from.m_view;

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
