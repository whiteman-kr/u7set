#ifndef SERIALDATATESTER_H
#define SERIALDATATESTER_H

#include <QMainWindow>
#include <QXmlReader>
#include <QPixmap>

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
		//void reloadConfig();
		void parseFile();

private:
	Ui::SerialDataTester *ui;

	/*QMenu* m_file = nullptr;
	QAction* m_reloadCfg = nullptr;*/

	QMenu* m_settingsMenu = nullptr;
	QAction* m_setComPort = nullptr;

	QMenu* m_setPortBaud = nullptr;
	QAction* m_baud115200 = nullptr;
	QAction* m_baud57600 = nullptr;
	QAction* m_baud38400 = nullptr;
	QAction* m_baud19200 = nullptr;

	struct SignalData
	{
		QString strId;
		QString caption;
		int offset;
		int bit;
		QString type;
	};

	struct Columns
	{
		int strId = 0;
		int caption = 1;
		int offset = 2;
		int bit = 3;
		int type = 4;
	};

	Columns tableColumns;

	QVector<SignalData> signalsFromXml;
};

#endif // SERIALDATATESTER_H
