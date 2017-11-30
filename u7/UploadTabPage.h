#pragma once
#include "MainTabPage.h"
#include "../lib/OutputLog.h"
#include "../lib/ModuleConfiguration.h"
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
	void findSubsystemsInBuild(int index);
	void subsystemChanged(int index);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo, bool verify);

	void readConfiguration(int);
	void readFirmware(QString fileName);

	//void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc);
	void showConfDataFileInfo(const QString& fileName);
	void writeConfDataFile(const QString& fileName);
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

private slots:

	void removeFirmwareLabels();
	void resetFirmwareLabels();
	void loadHeaderComplete(std::vector<int> uartIDList, QStringList uartTypeList);
	void uploadSuccessful(int uartID);

	// Data
	//
private:

	QSplitter* m_vsplitter = nullptr;

	QListWidget* m_pBuildList = nullptr;

	QListWidget* m_pSubsystemList = nullptr;

	QComboBox* m_pConfigurationCombo = nullptr;

	QTreeWidget* m_pFirmwareListWidget = nullptr;

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

};



