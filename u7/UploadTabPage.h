#pragma once
#include "MainTabPage.h"
#include "../lib/OutputLog.h"
#include "../lib/ModuleFirmware.h"
#include "../lib/Configurator.h"
#include "Builder/IssueLogger.h"

class DbController;
class QCheckBox;
class QComboBox;

//
//
// UploadTabPage
//
//
class UploadTabPage : public MainTabPage
{
	Q_OBJECT

public:
	UploadTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~UploadTabPage();

	// Public methods
	//
public:

	bool isUploading();
	void refreshProjectBuilds();

protected slots:
	void configurationTypeChanged(const QString& s);
	void buildChanged(int index);
	void subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo, bool verify);

	void readConfiguration(int);
	void readFirmware(QString fileName);

	void loadBinaryFile(const QString& fileName, ModuleFirmwareStorage* storage);
	void uploadFirmware(ModuleFirmwareStorage* storage, const QString& selectedSubsystem);
	void detectSubsystem();

	void eraseFlashMemory(int);
	void cancelOperation();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void timerEvent(QTimerEvent* pTimerEvent) override;

public slots:
	void projectOpened();
	void projectClosed();

	void read();
	void upload();
	void erase();
	void cancel();
	void clearLog();
	void settings();

	void disableControls();
	void enableControls();
	void communicationReadFinished();

private:
	void writeLog(const OutputLogItem& logItem);
	QString selectedSubsystem();

	void selectBuild(const QString& id);
	void selectSubsystem(const QString& id);

	void refreshBinaryFile();

private slots:

	void clearSubsystemsUartData();
	void resetUartData();

	void loadBinaryFileHeaderComplete();
	void uartOperationStart(int uartID, QString operation);
	void uploadComplete(int uartID);
	void detectSubsystemComplete(int selectedSubsystem);

	// Data
	//
private:

	// Interface members

	QSplitter* m_vsplitter = nullptr;

	QComboBox* m_pConfigurationCombo = nullptr;

	QListWidget* m_pBuildList = nullptr;

	QTreeWidget* m_pSubsystemsListWidget = nullptr;
	QTreeWidget* m_pUartListWidget = nullptr;

	QTextEdit* m_pLog = nullptr;

	QPushButton* m_pDetectSubsystemButton = nullptr;
	QPushButton* m_pReadToFileButton = nullptr;
	QPushButton* m_pConfigureButton = nullptr;
	QPushButton* m_pEraseButton = nullptr;

	QPushButton* m_pSettingsButton = nullptr;
	QPushButton* m_pClearLogButton = nullptr;
	QPushButton* m_pCancelButton = nullptr;

	Configurator* m_pConfigurator = nullptr;
	QThread* m_pConfigurationThread = nullptr;

	// Firmware and processing

	Hardware::ModuleFirmwareStorage m_firmware;

	int m_logTimerId = -1;

	bool m_uploading = false;

	Builder::IssueLogger m_outputLog;

	QString m_buildSearchPath;

	// Currently selected build and file info

	QStringList m_currentBuilds;
	QString m_currentBuild;

	QString m_currentFilePath;
	QDateTime m_currentFileModifiedTime;

	// Uart Columns indexes

	const int columnSubsysId = 0;
	const int columnUartId = 0;
	const int columnUartType = 1;
	const int columnUploadCount = 2;
	const int columnUartStatus = 3;
};
