#ifndef DIALOGDATASOURCES_H
#define DIALOGDATASOURCES_H

#include "../lib/Ui/AppDataSourcesWidget.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"
#include "MonitorConfigController.h"
#include "TcpAppSourcesState.h"

class DialogDataSources : public QDialog
{
	Q_OBJECT
public:
	explicit DialogDataSources(MonitorConfigController* configController, std::vector<TuningTcpClient*> tcpTuningClients, ILogFile* logFile, QWidget* parent);
	virtual ~DialogDataSources();

	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpTuningClients);

protected:
	virtual void reject() override;

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void dialogClosed();

private:
	void createAppSourceStateClients();
	void deleteAppSourceStateClients();

private:
	AppDataSourcesWidget* m_appDataSourcesWidget = nullptr;

	QLabel* m_tuningSourcesLabel = nullptr;
	TuningSourcesWidget* m_tuningSourcesWidget = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	std::vector<TcpAppSourcesState*> m_tcpSourcesStateClients;
	std::vector<SimpleThread*> m_sourcesStateClientThreads;

	MonitorConfigController* m_configController = nullptr;
	ILogFile* m_logFile = nullptr;
};

#endif // DIALOGDATASOURCES_H
