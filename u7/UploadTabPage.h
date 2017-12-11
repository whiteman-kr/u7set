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
	void findProjectBuilds();

protected slots:
	void configurationTypeChanged(const QString& s);
	void buildChanged(int index);
	void subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo, bool verify);

	void readConfiguration(int);
	void readFirmware(QString fileName);

	//void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc);
	void showConfDataFileInfo(const QString& fileName);
	void writeConfDataFile(const QString& fileName, const QString& subsystemId);
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
	QString subsystemId();

private slots:

	void clearSubsystemsUartData();
	void resetUartData();
	void loadHeaderComplete(std::map<QString, std::vector<UartPair> > subsystemsUartsInfo);
	void uploadSuccessful(int uartID);

	// Data
	//
private:

	QSplitter* m_vsplitter = nullptr;
	QListWidget* m_pBuildList = nullptr;

	QComboBox* m_pConfigurationCombo = nullptr;
	QTreeWidget* m_pSubsystemsListWidget = nullptr;
	QTreeWidget* m_pUartListWidget = nullptr;

	QTextEdit* m_pLog = nullptr;

	QPushButton* m_pReadButton = nullptr;
	QPushButton* m_pConfigureButton = nullptr;
	QPushButton* m_pEraseButton = nullptr;

	QPushButton* m_pSettingsButton = nullptr;
	QPushButton* m_pClearLogButton = nullptr;
	QPushButton* m_pCancelButton = nullptr;

	int m_logTimerId = -1;

	Configurator* m_pConfigurator = nullptr;
	QThread* m_pConfigurationThread = nullptr;

	Builder::IssueLogger m_outputLog;

	QString m_buildSearchPath;

	QString m_currentBuild;
	QString m_currentSubsystem;
	QString m_currentFileName;

	int m_currentBuildIndex = -1;
	int m_currentSubsystemIndex = -1;

	bool m_uploading = false;

	const int columnSubsysId = 0;
	const int columnUartId = 0;
	const int columnUartType = 1;
	const int columnUploadCount = 2;

	std::map<QString, std::vector<UartPair>> m_subsystemsUartsInfo;
};



