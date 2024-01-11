#pragma once

#include "../lib/Ui/AppDataSourcesWidget.h"
#include "../lib/Ui/TuningSourcesWidget.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../ClientLib/TuningTcpClient.h"
#include "../ClientLib/AdsSourceStateConnection.h"
#include "../ClientLib/TuningConnection.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"
#include "../ClientLib/IRecentAppSignals.h"
#include "../lib/Tuning/ITuningAuthorization.h"

class DialogDataSources : public QDialog
{
	Q_OBJECT

public:
	static void create(const TestSuite::TestSuiteConfigController& configController,
					   ILogFile* logFile,
					   QWidget* parent);

private:
	explicit DialogDataSources(const TestSuite::TestSuiteConfigController& configController,
							   ILogFile* logFile,
							   QWidget* parent);
	virtual ~DialogDataSources();

private slots:
	void slot_configurationArrived();
	void detailsClicked();

private:
	static inline DialogDataSources* s_dialogDataSources = nullptr;

	AppDataSourcesWidget* m_appDataSourcesWidget = nullptr;

	QLabel* m_tuningSourcesLabel = nullptr;
	TuningSourcesWidget* m_tuningSourcesWidget = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	// --
	//
	const TestSuite::TestSuiteConfigController& m_configController;
	ILogFile* m_logFile = nullptr;

	// --
	//
	TuningSignalManager m_emptySignals{QString(), m_logFile};
	TuningAuthorizationStub m_authorization;
	ClientLib::TuningLogStub m_tuningLogStub;

	ClientLib::AdsSourceStateConnection m_tcpSignalClientCtrl{m_logFile};
	ClientLib::TuningConnection m_tcpTuningCtrl{m_emptySignals,
												m_emptySignals,
												m_emptySignals,
												m_authorization,
												m_logFile,
												&m_tuningLogStub};
};
