#ifndef PORTRECEIVER_H
#define PORTRECEIVER_H

#include <QSerialPort>

class PortReceiver : public QObject
{
	Q_OBJECT
public:
	explicit PortReceiver(QObject *parent = 0);
	virtual ~PortReceiver();

signals:
	void portError(QString error);
	void dataFromPort(QByteArray data);

private slots:
	void dataReceived();

public slots:
	void setNewPort(const QString& portName);
	void setBaud(const int& baud);
	void openPort();

private:
	QSerialPort* m_serialPort = nullptr;
};

#endif // PORTRECEIVER_H
