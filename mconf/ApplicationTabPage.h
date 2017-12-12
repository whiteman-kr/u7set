#ifndef APPLICATIONTABPAGE_H
#define APPLICATIONTABPAGE_H

#include <QWidget>
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
	QString selectedSubsystem();
	void selectSubsystem(const QString& id);

signals:
	void loadBinaryFile(const QString& fileName, ModuleFirmwareStorage* storage);

private slots:
	void subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2);
	void openFileClicked();
	void on_resetCountersButton_clicked();

public slots:
	void loadBinaryFileHeaderComplete();
	void uploadStart(int uartID);
	void uploadComplete(int uartID);
	void detectSubsystemComplete(int selectedSubsystem);

	void enableControls();

private:
	void clearSubsystemsUartData();

	const int columnSubsysId = 0;

	const int columnUartId = 0;
	const int columnUartType = 1;
	const int columnUploadCount = 2;
	const int columnUartStatus = 3;

private:

	QLineEdit* m_pFileNameEdit = nullptr;

	QTreeWidget* m_pSubsystemsListWidget = nullptr;

	QTreeWidget* m_pUartListWidget = nullptr;

	ModuleFirmwareStorage m_firmware;
};

#endif // APPLICATIONTABPAGE_H
