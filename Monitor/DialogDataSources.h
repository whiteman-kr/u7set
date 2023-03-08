#ifndef DIALOGDATASOURCES_H
#define DIALOGDATASOURCES_H

#include "../lib/Ui/AppDataSourcesWidget.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Ui/TuningSourcesWidget.h"
#include "../ClientLib/AdsSourceStateConnection.h"
#include "MonitorConfigController.h"


class DialogDataSources : public QDialog
{
	Q_OBJECT

public:
	static void create(const MonitorConfigController& configController, std::vector<TuningTcpClient*> tcpTuningClients, ILogFile* logFile, QWidget* parent);
	static void updateTuningTcpClients(std::vector<TuningTcpClient*> tcpTuningClients);

private:
	explicit DialogDataSources(const MonitorConfigController& configController, std::vector<TuningTcpClient*> tcpTuningClients, ILogFile* logFile, QWidget* parent);
	virtual ~DialogDataSources();

private:
	void setTuningTcpClients(std::vector<TuningTcpClient*> tcpTuningClients);

private slots:
	void slot_configurationArrived(ConfigSettings configuration);
	void detailsClicked();

private:
	static inline DialogDataSources* s_dialogDataSources = nullptr;

	AppDataSourcesWidget* m_appDataSourcesWidget = nullptr;

	QLabel* m_tuningSourcesLabel = nullptr;
	TuningSourcesWidget* m_tuningSourcesWidget = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	// --
	//
	const MonitorConfigController& m_configController;
	ILogFile* m_logFile = nullptr;

	// --
	//
	Client::AdsSourceStateConnection m_tcpSignalClientCtrl{m_logFile};
};

#endif // DIALOGDATASOURCES_H
