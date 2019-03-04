#ifndef SERIALPORTWORKER_H
#define SERIALPORTWORKER_H

#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "Options.h"

// ==============================================================================================

const int REQUEST_SERIAL_PORT_TIMEOUT = 50; // 50 ms

const int TIMEOUT_COUNT = 10;

// ==============================================================================================

class SerialPortWorker : public QObject
{
	Q_OBJECT

public:

	explicit SerialPortWorker(SerialPortOption* option);
	virtual ~SerialPortWorker();

private:

	QSerialPort*		m_port= nullptr;
	SerialPortOption*	m_option = nullptr;

	bool				m_finishThread = false;

	bool				m_disconnectSerialPort = false;

    int                 m_timeout = 0;

public:

	bool				openSerialPort();
	bool				closeSerialPort();

	bool				runTest();

signals:

	void				finished();

private slots:

	void				serialPortError(QSerialPort::SerialPortError error);	// on serial port disconnected

public slots:

	void				process();
	void				finish() { m_finishThread = true; }
	void				reopenSerialPort() { m_disconnectSerialPort = true; }
};

// ==============================================================================================

#endif // SERIALPORTWORKER_H
