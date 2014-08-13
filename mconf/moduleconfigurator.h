#ifndef MODULECONFIGURATOR_H
#define MODULECONFIGURATOR_H

#include <QtWidgets/QMainWindow>
#include "ui_moduleconfigurator.h"
#include "../include/configdata.h"
#include "Log.h"
#include "settings.h"

class QLineEdit;
class QPushButton;
class QTextEdit;
class Configurator;

class ModuleConfigurator : public QMainWindow
{
	Q_OBJECT

public:
	ModuleConfigurator(QWidget *parent = 0);
	~ModuleConfigurator();

public:

protected:
	virtual void timerEvent(QTimerEvent* pTimerEvent) override;
	void writeLog(const LogItem& logItem);

private slots:
	void configureClicked();
	void readClicked();
	void eraseClicked();
	void settingsClicked();
	void clearLogClicked();

	void disableControls();
	void enableControls();

	void communicationReadFinished(int protocolVersion, std::vector<uint8_t> data);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo);
	
	void readConfiguration(int);
	void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc1, quint32 firmwareCrc2);
	void writeConfData(ConfigDataReader conf);
	void eraseFlashMemory(int);
	
private:
	Ui::ModuleConfiguratorClass ui;
	Settings m_settings;

	QTabWidget* m_tabWidget;

	QTextEdit* m_pLog;

	QPushButton* m_pReadButton;
	QPushButton* m_pConfigureButton;
	QPushButton* m_pEraseButton;

	QPushButton* m_pSettingsButton;
	QPushButton* m_pClearLogButton;

	int m_logTimerId;

	Configurator* m_pConfigurator;
	QThread* m_pConfigurationThread;
};

#endif // MODULECONFIGURATOR_H
