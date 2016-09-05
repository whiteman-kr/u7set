#include "SerialDataParser.h"
#include <assert.h>
#include <QDebug>

SerialDataParser::SerialDataParser()
{
	m_buffer = new char[SerialParserBufferSize];
}

SerialDataParser::~SerialDataParser()
{
	delete m_buffer;
}

void SerialDataParser::parse(const QByteArray& receivedData)
{
	memcpy(m_buffer, receivedData.constData(), receivedData.size()); // Make pointer on the packet beginning

	m_readPtr = m_buffer;		// Beginning of the packet. It will show: where we are currently in the packet
	m_dataSize = receivedData.size();	// Size of the packet

	switch (m_state)
	{
	case ScanningSignature: scanningSignaure(); break;
	case ReadingHeader: readingHeader(); break;
	case ReadingData: readingData(); break;
	default: assert(false);
	}
}

void SerialDataParser::scanningSignaure()
{	
	int bytesToCopy = 0;					// How many bytes we need to copy
	int avaiableDataSize = m_dataSize;		// How many data received

	do
	{
		if (sizeof(m_signature.bytes) == 4)
			m_bytesCount=0;

		bytesToCopy = 4 - m_bytesCount;		// How many data recorded to signature (in case, when packet was not received sucsessfuly - m_bytesCount will be not 0)

		avaiableDataSize = m_dataSize - (m_readPtr - m_buffer); // How much data currently avaiable

		if (avaiableDataSize < bytesToCopy)  // When our packet have amount of bytes, which is smaller than we need - record all of them into signature value
		{
			bytesToCopy = avaiableDataSize;
		}

		memcpy(m_signature.bytes + m_bytesCount, m_readPtr, bytesToCopy); // Record setted amount of bytes
		m_bytesCount += bytesToCopy; // Set signature bytes amount to our value (Now, signature has its own amount of byes + recordered amount of bytes)


		if (m_bytesCount == 4)  // If signature consist of 4 bytes, lets have a look
		{

			if (m_signature.uint32 == baseSignature) // If our signature is a beginning of the packet header
			{
				m_state = ReadingHeader; // Change current state
				m_readPtr += bytesToCopy; // Move our packet pointer forward by recordered amount of bytes (WARNNG: it mmust be bytesToCopy value, in case of the packet beginning)
				m_bytesCount = 0; // Reset signature bytes amount value
				readingHeader(); // Lets start packet reading
			}
			else
			{
				m_bytesCount = 0; // In case of wrong signature - just reset it is own bytes amount
				m_readPtr++; // Move pointer by one byte
			}
		}
		else
		{
			break; // If amount of bytes to record is smaller than 4 - it means that we are at the end of the packet
		}
	}
	while(m_readPtr < m_buffer + m_dataSize);
}

void SerialDataParser::readingHeader()
{
	int bytesToCopy = 8 - m_bytesCount;		// How many data recorded to signature (in case, when packet was not received sucsessfuly - m_bytesCount will be not 0)

	int avaiableDataSize = m_dataSize - (m_readPtr - m_buffer); // How much data currently avaiable

	if (avaiableDataSize < bytesToCopy)  // When our packet have amount of bytes, which is smaller than we need - record all of them into signature value
	{
		bytesToCopy = avaiableDataSize;
	}

	memcpy(m_header.bytes + m_bytesCount, m_readPtr, bytesToCopy);
	m_bytesCount += bytesToCopy;

	if (m_bytesCount == 8)
	{
		m_state = ReadingData;
		m_readPtr += bytesToCopy;
		m_bytesCount = 0;
		m_packetData = new char[m_header.header.dataAmount];
		readingData();
	}
	else
	{
		m_state = ScanningSignature;
		m_readPtr++;
		m_bytesCount = 0;
		scanningSignaure();
	}
}

void SerialDataParser::readingData()
{
	qint16 dataAmount = m_header.header.dataAmount+12; // 8 - crc bytes and 4 - dataUniqueIdBytes

	int bytesToCopy = (dataAmount) - m_bytesCount;
	int avaiableDataSize = m_dataSize - (m_readPtr - m_buffer);

	if (avaiableDataSize < bytesToCopy)
	{
		bytesToCopy = avaiableDataSize;
	}

	memcpy(m_packetData + m_bytesCount, m_readPtr, bytesToCopy);

	m_bytesCount += bytesToCopy;

	if (m_bytesCount == dataAmount)
	{
		m_state = ScanningSignature;
		m_bytesCount = 0;

		QByteArray dataUniqueId;

		dataUniqueId.insert(0, m_packetData, 4); // Read dataUniqueId
		m_packetData+=4;

		QByteArray dataToSend;

		dataToSend.insert(0, m_packetData, m_header.header.dataAmount); // Read packet data
		m_packetData += m_header.header.dataAmount;

		CrcRepresentation crc;

		QByteArray dataForCrc;
		dataForCrc.append(m_header.bytes, 8);
		dataForCrc.append(dataUniqueId, 4);
		dataForCrc.append(dataToSend, dataToSend.size());

		crc.uint64 = Crc::crc64(dataForCrc, dataForCrc.size());

		CrcRepresentation crcFromPacket;

		memcpy(crcFromPacket.bytes, m_packetData, 8);

		m_readPtr += bytesToCopy;

		QString version = QString::number(m_header.header.version);
		QString trId = QString::number(m_header.header.txid);
		QString numerator = QString::number(m_header.header.numerator);

		if (crc.uint64 != crcFromPacket.uint64)
		{
			qDebug() << "CRC error: " << crc.uint64 << ":::" << crcFromPacket.uint64;
			emit crcError(version, trId, numerator, dataUniqueId);
		}
		else
		{
			emit packetProcessed(version, trId, numerator, dataUniqueId, dataToSend);
		}

		if (m_dataSize - (m_readPtr - m_buffer) > 0)
		{
			m_state = ScanningSignature;
			m_bytesCount = 0;
			scanningSignaure();
		}
	}
	else
	{
		m_state = ScanningSignature;
		m_readPtr++;
		m_bytesCount = 0;
		scanningSignaure();
	}
}
