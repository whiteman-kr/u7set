#ifndef SERIALDATATESTER_H
#define SERIALDATATESTER_H

#include "SettingsDialog.h"
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
		void reloadConfig();
		void parseFile();

private:
	Ui::SerialDataTester *ui;

	SettingsDialog* m_applicationSettingsDialog;

	QMenu* m_file;
	QAction* m_reloadCfg;

	struct SignalData
	{
		QString strId;
		QString caption;
		int offset;
		int bit;
		QString type;
	};

	enum Columns
	{
		strId,
		caption,
		offset,
		bit,
		type
	};

	QVector<SignalData> signalsFromXml;
};

#endif // SERIALDATATESTER_H
