#pragma once
#include <QSerialPort>
#include <QByteArray>

#include "../../lib/Crc.h"

const int SerialParserBufferSize = 1024;

class SerialDataParser : public QObject
{
	Q_OBJECT
public :
	SerialDataParser();
	virtual ~SerialDataParser();

signals:
	void packetProcessed(QString version, QString trId, QString numerator, QByteArray dataId, QByteArray data);
	void crcError(QString version, QString trId, QString numerator, QByteArray dataId);

public slots:
	void parse(const QByteArray& receivedData);

private slots:
	void scanningSignaure();
	void readingHeader();
	void readingData();

private:

	char* m_buffer = nullptr; // Pointer on the beginning of the received data
	char* m_readPtr = nullptr;// Pointer, that point on place, where reading was stopped
	int m_dataSize = 0; // Size of the received data
	quint64 m_crc_table[256];

#pragma pack(push, 1)

	union Signature
	{
		char bytes[4];
		quint32 uint32;
	};

	struct Header
	{
		quint16 version;
		quint16 txid;
		quint16 numerator;
		quint16 dataAmount;
	};

	union HeaderUnion
	{
		char bytes[sizeof(Header)];
		Header header;
	};

	union DataUniqueId
	{
		char bytes[4];
		quint32 uint32;
	};

	union CrcRepresentation
	{
		char bytes[8];
		quint64 uint64;
	};

#pragma pack(pop)

	Signature m_signature; // Value to store received bytes (Signature bytes)
	HeaderUnion m_header;
	char* m_packetData;
	const quint32 baseSignature = 0x424D4C47; // Signature for compare
	int m_bytesCount = 0; // Stores amount of writed down bytes in m_signature

	enum State
	{
		ScanningSignature,
		ReadingHeader,
		ReadingData
	};

	State m_state = ScanningSignature;
};
