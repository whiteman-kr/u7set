#pragma once
#include "MainTabPage.h"
#include "../lib/OutputLog.h"
#include "../lib/ModuleFirmware.h"
#include "../lib/Configurator.h"
#include "../lib/OutputLog.h"
#include "../lib/BuildInfo.h"

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
	void buildChanged();
	void subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo, bool verify);

	void readConfiguration(int);
	void readFirmware(QString fileName, std::optional<std::vector<int>> selectedUarts);

	void loadBinaryFile(const QString& fileName, ModuleFirmwareStorage* storage);
	void uploadFirmware(ModuleFirmwareStorage* storage, const QString& selectedSubsystem, std::optional<std::vector<int>> selectedUarts);
	void detectSubsystem();

	void eraseFlashMemory(int, std::optional<std::vector<int>> selectedUarts);
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
	void mconf();

	void disableControls();
	void enableControls();
	void communicationReadFinished();

private:
	void writeLog(const OutputLogItem& logItem);
	QString selectedSubsystem();

	void selectBuild(const QString& id);
	void selectSubsystem(const QString& id);

	void refreshBinaryFile();

	bool readBuildInfo(const QString& buildPath, Builder::BuildInfo* buildInfo, bool* success);

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

	enum class BuildColumns
	{
		Name,
		Date,
		Result
	};

	// Interface members

	QSplitter* m_pLeftSplitter = nullptr;
	QSplitter* m_pRightSplitter = nullptr;

	QTreeWidget* m_pBuildTree = nullptr;

	QTreeWidget* m_pSubsystemsListWidget = nullptr;
	QTreeWidget* m_pUartListWidget = nullptr;

	QTextEdit* m_pLog = nullptr;

	QPushButton* m_pDetectSubsystemButton = nullptr;
	QPushButton* m_pReadToFileButton = nullptr;
	QPushButton* m_pConfigureButton = nullptr;
	QPushButton* m_pEraseButton = nullptr;
	QPushButton* m_pMconfButton = nullptr;

	QPushButton* m_pSettingsButton = nullptr;
	QPushButton* m_pClearLogButton = nullptr;
	QPushButton* m_pCancelButton = nullptr;

	Configurator* m_pConfigurator = nullptr;
	QThread* m_pConfigurationThread = nullptr;

	// Firmware and processing

	Hardware::ModuleFirmwareStorage m_firmware;

	int m_logTimerId = -1;

	bool m_uploading = false;

	OutputLog m_outputLog;

	QString m_buildSearchPath;

	// Builds list

	std::vector<std::pair<QString, QDateTime>> m_builds;

	// Currently selected build and file info

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
