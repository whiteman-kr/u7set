#ifndef PORTRECEIVER_H
#define PORTRECEIVER_H

#include <QSerialPort>

class PortReceiver : public QObject
{
	Q_OBJECT
public:
	explicit PortReceiver(QObject* parent = 0);
	virtual ~PortReceiver();

signals:
	void portError(QString error);
	void dataFromPort(QByteArray data);

private slots:
	void dataReceived();

public slots:
	void setPort(const QString& portName);
	void setBaud(const int& baud);
	void setDataBits(const QSerialPort::DataBits& dataBits);
	void setStopBits(const QSerialPort::StopBits& stopBits);

	void openPort();
	void closePort();

private:
	QSerialPort* m_serialPort = nullptr;
};

#endif // PORTRECEIVER_H
