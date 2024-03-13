#ifndef DIALOGDATASOURCES_H
#define DIALOGDATASOURCES_H

#include "MonitorConfigController.h"

#include "../lib/Ui/AppDataSourcesWidget.h"
#include "../lib/Ui/TuningSourcesWidget.h"

#include <ClientLib/AdsSourceStateConnection.h>


class DialogDataSources : public QDialog
{
	Q_OBJECT

public:
	static void create(const MonitorConfigController& configController,
					   ClientLib::TuningConnection& tuningConnection,
					   ILogFile* logFile,
					   QWidget* parent);

private:
	explicit DialogDataSources(const MonitorConfigController& configController,
							   ClientLib::TuningConnection& tuningConnection,
							   ILogFile* logFile,
							   QWidget* parent);
	virtual ~DialogDataSources();

private slots:
	void slot_configurationArrived(MonitorConfigSettings configuration);
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
	ClientLib::AdsSourceStateConnection m_tcpSignalClientCtrl{m_logFile};
	ClientLib::TuningConnection& m_tuningConnection;
};

#endif // DIALOGDATASOURCES_H
