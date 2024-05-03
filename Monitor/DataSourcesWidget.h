#pragma once

#include "MonitorConfigController.h"
#include <ClientLib/AdsSourceStateConnection.h>


namespace ClientLib
{
	class TuningConnection;
}

namespace SchemaClientLib
{
	class AppDataSourcesWidget;
	class TuningSourcesWidget;
}

class DataSourcesWidget : public QWidget
{
	Q_OBJECT

public:
	DataSourcesWidget(const MonitorConfigController& configController,
					  ClientLib::TuningConnection& tuningConnection,
					  ILogFile* logFile,
					  QWidget* parent);

private slots:
	void slot_configurationArrived(MonitorConfigSettings configuration);
	void detailsClicked();

private:
	SchemaClientLib::AppDataSourcesWidget* m_appDataSourcesWidget = nullptr;

	QLabel* m_tuningSourcesLabel = nullptr;
	SchemaClientLib::TuningSourcesWidget* m_tuningSourcesWidget = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	// --
	//
	const MonitorConfigController& m_configController;
	ILogFile* m_logFile = nullptr;

	ClientLib::AdsSourceStateConnection m_tcpSignalClientCtrl{m_logFile};
	ClientLib::TuningConnection& m_tuningConnection;
};
