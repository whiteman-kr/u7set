#ifndef SERIALDATATESTERSERVER_H
#define SERIALDATATESTERSERVER_H

#include <QDialog>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>

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
	quint8 m_dataOffset = 0x00;
	quint8 m_dataBits = 0x03;
	qint16 m_dataValue = 1;

	struct SignalData
	{
		QString strId;
		QString caption;
		int offset = 0;
		int bit = 0;
		QString type;
	};

	int m_numberOfPacket = 0;
	int m_amountOfSignals = 0;

	QTimer *m_timerForPackets = nullptr;

	QSerialPort *m_serialPort = nullptr;

	QVector<SignalData> m_signalsFromXml;

	QByteArray  m_bytes;
	quint64 crc_table[256];
};

#endif // SERIALDATATESTERSERVER_H
