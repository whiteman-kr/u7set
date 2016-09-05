#ifndef SERIALDATATESTERSERVER_H
#define SERIALDATATESTERSERVER_H

#include <QDialog>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QBitArray>

#include "../../lib/Crc.h"

namespace Ui {
	class SerialDataTesterServer;
}

class SerialDataTesterServer : public QDialog
{
	Q_OBJECT

public:
	explicit SerialDataTesterServer(QWidget *parent = 0);
	virtual ~SerialDataTesterServer();

private slots:
	void setFile();
	void startServer();
	void stopServer();
	void parseFile();
	void sendPacket();

private:
	Ui::SerialDataTesterServer *ui = nullptr;

	const quint32 m_signature = 0x424D4C47;
	const quint16 m_version = 0x0001;
	quint16 m_Id = 0x0002;
	quint16 m_numerator = 0x0003;
	quint16 m_dataAmount = 0x0004;
	quint16 m_dataUniqueId = 0x0005;
	int m_dataBits = 0;
	quint8 m_dataType = 0x00;
	int m_dataSize = 0;

	struct SignalData
	{
		QString strId;
		QString exStrId;
		QString name;
		QString type;
		QString unit;
		int dataSize = 0;
		QString dataFormat;
		QString byteOrder;
		int offset = 0;
		int bit = 0;
	};

    union Signature
    {
        char bytes[4];
        quint32 uint32;
    };

	struct Header
    {
        quint16 version;
        quint16 id;
        quint16 num;
        quint16 amount;
    };

    union HeaderUnion
    {
		char bytes[8];
        Header hdr;
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

	int m_numberOfPacket = 0;
	int m_amountOfSignals = 0;

	QTimer *m_timerForPackets = nullptr;

	QSerialPort *m_serialPort = nullptr;

	QVector<SignalData> m_signalsFromXml;

	QByteArray  m_packet;
	QBitArray m_data;
	quint64 crc_table[256];
};

#endif // SERIALDATATESTERSERVER_H
