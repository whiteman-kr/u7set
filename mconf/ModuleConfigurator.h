#ifndef MODULECONFIGURATOR_H
#define MODULECONFIGURATOR_H

#include <QtWidgets/QMainWindow>
#include "ui_moduleconfigurator.h"
#include "../lib/ModuleFirmware.h"
#include "../lib/OutputLog.h"
#include "Settings.h"

#include <optional>

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
	void cancelClicked();
	void settingsClicked();
	void clearLogClicked();

	void disableControls();
	void enableControls();

    void communicationReadFinished(int protocolVersion, std::vector<quint8> data);

signals:
	void setCommunicationSettings(QString device, bool showDebugInfo, bool verify);
	
	void readServiceInformation(int);
	void readFirmware(QString fileName, std::optional<std::vector<int>> selectedUarts);

    void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc);
	void writeConfData(ModuleFirmwareStorage *storage, const QString& subsystemId, std::optional<std::vector<int>> selectedUarts);
	void eraseFlashMemory(int, std::optional<std::vector<int>> selectedUarts);
	
private:
	Ui::ModuleConfiguratorClass ui;
	Settings m_settings;

	QSplitter* m_pSplitter = nullptr;

	QTabWidget* m_tabWidget = nullptr;

	QTextEdit* m_pLog = nullptr;

	QPushButton* m_pReadButton = nullptr;
	QPushButton* m_pConfigureButton = nullptr;
	QPushButton* m_pEraseButton = nullptr;
	QPushButton* m_pCancelButton = nullptr;

	QPushButton* m_pSettingsButton = nullptr;
	QPushButton* m_pClearLogButton = nullptr;

	int m_logTimerId;

	Configurator* m_pConfigurator = nullptr;
	QThread* m_pConfigurationThread = nullptr;
};

#endif // MODULECONFIGURATOR_H
