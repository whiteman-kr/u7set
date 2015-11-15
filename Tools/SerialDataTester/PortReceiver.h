#ifndef PORTRECEIVER_H
#define PORTRECEIVER_H

#include <QSerialPort>

class PortReceiver : public QObject
{
	Q_OBJECT
public:
	explicit PortReceiver(QObject *parent = 0);

signals:
	void portError(QString);

private slots:
	void dataReceived();

public slots:
	void setNewPort(const QString&);
	void setBaud(const int&);
	void openPort();

private:
	QSerialPort* m_serialPort = nullptr;
};

#endif // PORTRECEIVER_H
