#ifndef SERIALDATATESTER_H
#define SERIALDATATESTER_H

#include "SettingsDialog.h"
#include "PortReceiver.h"
#include "SerialDataParser.h"
#include <QMainWindow>
#include <QXmlReader>
#include <QPixmap>
#include <QTimer>
#include <QBitArray>

namespace Ui {
	class SerialDataTester;
}

class SerialDataTester : public QMainWindow
{
	Q_OBJECT

public:
	explicit SerialDataTester(QWidget* parent = 0);
	virtual ~SerialDataTester();

private slots:
	void parseFile();
	void reloadConfig();
	void selectNewSignalsFile();
	void applicationExit();
	void setPort(QAction* newPort);
	void setBaud(QAction* newBaud);
	void setBits(QAction* newBits);
	void setStopBits(QAction* newStopBits);
	void portError(QString error);
	void dataReceived (QString version, QString trId, QString numerator, QByteArray dataId, QByteArray receivedValues);
	void crcError (QString version, QString trId, QString numerator, QByteArray dataId);
	void signalTimeout();
	void erasePacketData();
	void loadLastUsedSettings();
	void startReceiver();
	void stopReceiver();

signals:
	void portChanged(QString newPortName);
	void baudChanged(int newPortBaud);
	void bitsChanged(QSerialPort::DataBits newBits);
	void stopBitsChanged(QSerialPort::StopBits newStopBits);

private:
	QBitArray bytesToBits(QByteArray bytes);
	QByteArray bitsToBytes(QBitArray bits);

	Ui::SerialDataTester *ui = nullptr;

	QTimer* receiveTimeout;

	SettingsDialog* m_applicationSettingsDialog = nullptr;
	PortReceiver* m_portReceiver = nullptr;
	SerialDataParser* m_parser = nullptr;

	QMenu* m_file = nullptr;
	QAction* m_reloadCfg = nullptr;
	QAction* m_changeSignalSettingsFile = nullptr;
	QAction* m_startReceiving = nullptr;
	QAction* m_stopReceiving = nullptr;
	QAction* m_exit = nullptr;

	QMenu* m_settings = nullptr;
	QMenu* m_setPort = nullptr;
	QMenu* m_setBaud = nullptr;
	QMenu* m_setDataBits = nullptr;
	QMenu* m_setStopBits = nullptr;
	QAction* m_erasePacketData = nullptr;
	QAction* m_loadDefaultSettings = nullptr;
	QAction* m_readXmlFile = nullptr;

	QString m_pathToSignalsXml;

	QThread* m_PortThread;
    QThread* m_ParserThread;

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

	enum Columns
	{
		strId,
		name,
		offset,
		bit,
		type,
		value
	};

	QVector<SignalData> m_signalsFromXml;
	const quint32 m_signature = 0x424D4C47;
	int m_dataSize = 0;
};

#endif // SERIALDATATESTER_H
