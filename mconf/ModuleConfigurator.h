#ifndef MODULECONFIGURATOR_H
#define MODULECONFIGURATOR_H

#include <QtWidgets/QMainWindow>
#include "ui_moduleconfigurator.h"
#include "../lib/ModuleFirmware.h"
#include "../lib/OutputLog.h"
#include "Settings.h"

class QLineEdit;
class QPushButton;
class QTextEdit;
class Configurator;

using namespace Hardware;

class ModuleConfigurator : public QMainWindow
{
	Q_OBJECT

public:
	ModuleConfigurator(QWidget *parent = 0);
	~ModuleConfigurator();

public:

protected:
	virtual void timerEvent(QTimerEvent* pTimerEvent) override;
	void writeLog(const OutputLogItem& logItem);

private slots:
	void configureClicked();
	void readClicked();
	void eraseClicked();
	void settingsClicked();
	void clearLogClicked();

	void disableControls();
	void enableControls();

    void communicationReadFinished(int protocolVersion, std::vector<quint8> data);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo, bool verify);
	
	void readConfiguration(int);
	void readFirmware(QString fileName);

    void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc);
	void writeConfData(ModuleFirmware* conf);
	void eraseFlashMemory(int);
	
private:
	Ui::ModuleConfiguratorClass ui;
	Settings m_settings;

	QTabWidget* m_tabWidget = nullptr;

	QTextEdit* m_pLog = nullptr;

	QPushButton* m_pReadButton = nullptr;
	QPushButton* m_pConfigureButton = nullptr;
	QPushButton* m_pEraseButton = nullptr;

	QPushButton* m_pSettingsButton = nullptr;
	QPushButton* m_pClearLogButton = nullptr;

	int m_logTimerId;

	Configurator* m_pConfigurator = nullptr;
	QThread* m_pConfigurationThread = nullptr;
};

#endif // MODULECONFIGURATOR_H
