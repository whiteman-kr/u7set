#ifndef APPLICATIONTABPAGE_H
#define APPLICATIONTABPAGE_H

#include <QWidget>
#include "../HardwareLib/ModuleFirmware.h"

#include <optional>
using namespace Hardware;

class ApplicationTabPage : public QWidget
{
	Q_OBJECT

public:
	ApplicationTabPage(bool expertMode, QWidget* parent = 0);
	~ApplicationTabPage();

	bool isFileLoaded() const;

	Hardware::ModuleFirmwareStorage* configuration();

	QString selectedSubsystem() const;
	void selectSubsystem(const QString& id);

	std::optional<std::vector<int>> selectedUarts() const;

signals:
	void loadBinaryFile(const QString& fileName, ModuleFirmwareStorage* storage);
	void detectSubsystem();
	void detectUarts();

private slots:
	void subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2);
	void openFileClicked();
	void resetCountersClicked();
	void detectSubsystemsClicked();
	void detectUartsClicked();

public slots:
	void loadBinaryFileHeaderComplete();
	void uartOperationStart(int uartID, QString operation);
	void uploadComplete(int uartID);
	void detectSubsystemComplete(int selectedSubsystem);
	void detectUartsComplete(std::vector<int> uartIds);

	void enableControls();

private:
	void clearSubsystemsUartData();
	void fillUartsList();

	const int columnSubsysId = 0;

	const int columnUartId = 0;
	const int columnUartType = 1;
	const int columnUploadCount = 2;
	const int columnUartStatus = 3;

private:
	bool m_expertMode = false;

	enum class UartListColumn
	{
		Id,
		Type,
		Process
	};

	QLineEdit* m_fileNameEdit = nullptr;

	QTreeWidget* m_subsystemsListTree = nullptr;

	QTreeWidget* m_bitstreamUartListTree = nullptr;

	QTreeWidget* m_pUartsListTree = nullptr;

	ModuleFirmwareStorage m_firmware;

	std::vector<int> m_uartIds;

	std::map<int, QString> m_uartIdTypes;
};

#endif // APPLICATIONTABPAGE_H
