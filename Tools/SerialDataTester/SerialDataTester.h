#ifndef SERIALDATATESTER_H
#define SERIALDATATESTER_H

#include "SettingsDialog.h"
#include "PortReceiver.h"
#include <QMainWindow>
#include <QXmlReader>
#include <QPixmap>
#include <QTimer>

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
	void portError(QString error);
	void dataReceived(QByteArray data);
	void signalTimeout();

signals:
	void portChanged(QString);
	void baudChanged(int);

private:
	Ui::SerialDataTester *ui = nullptr;

	QTimer* receiveTimeout;

	SettingsDialog* m_applicationSettingsDialog = nullptr;
	PortReceiver* m_portReceiver = nullptr;

	QMenu* m_file = nullptr;
	QAction* m_reloadCfg = nullptr;
	QAction* m_changeSignalSettingsFile = nullptr;
	QAction* m_exit = nullptr;

	QMenu* m_settings = nullptr;
	QMenu* m_setPort = nullptr;
	QMenu* m_setBaud = nullptr;

	QString m_pathToSignalsXml;

	QThread *m_PortThread;

	struct SignalData
	{
		QString strId;
		QString caption;
		int offset = 0;
		int bit = 0;
		QString type;
	};

	enum Columns
	{
		strId,
		caption,
		offset,
		bit,
		type,
		value
	};

	QVector<SignalData> signalsFromXml;
};

#endif // SERIALDATATESTER_H
