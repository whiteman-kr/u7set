#ifndef APPLICATIONTABPAGE_H
#define APPLICATIONTABPAGE_H

#include <QWidget>
#include "ui_applicationtabpage.h"
#include "../lib/ModuleFirmware.h"

using namespace Hardware;

class ApplicationTabPage : public QWidget
{
	Q_OBJECT

public:
	ApplicationTabPage(QWidget* parent = 0);
	~ApplicationTabPage();

	bool isFileLoaded() const;

	Hardware::ModuleFirmwareStorage* configuration();
	QString subsystemId();

signals:
	void loadBinaryFile(const QString& fileName, ModuleFirmwareStorage* storage);

private slots:
	void openFileClicked();
	void on_resetCountersButton_clicked();

public slots:
	void loadBinaryFileHeaderComplete();
	void uploadComplete(int uartID);

private:
	void clearUartData();
	void fillUartData();

	const int columnSubsysId = 0;
	const int columnUartId = 1;
	const int columnUartType = 2;
	const int columnUploadCount = 3;

private:
	Ui::ApplicationTabPage ui;
    //ConfigDataReader m_reader;
	ModuleFirmwareStorage m_confFirmware;
};

#endif // APPLICATIONTABPAGE_H
