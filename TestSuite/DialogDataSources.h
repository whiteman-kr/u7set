#pragma once

#include "../lib/Tuning/ITuningAuthorization.h"

#include <ClientLib/TuningSignalManager.h>
#include <ClientLib/AdsSourceStateConnection.h>
#include <ClientLib/TuningConnection.h>

namespace TestSuite
{
	class TestSuiteConfigController;
}

namespace SchemaClientLib
{
	class AppDataSourcesWidget;
	class TuningSourcesWidget;
}

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

	SchemaClientLib::AppDataSourcesWidget* m_appDataSourcesWidget = nullptr;

	QLabel* m_tuningSourcesLabel = nullptr;
	SchemaClientLib::TuningSourcesWidget* m_tuningSourcesWidget = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;

	// --
	//
	const TestSuite::TestSuiteConfigController& m_configController;
	ILogFile* m_logFile = nullptr;

	// --
	//
	ClientLib::TuningSignalManager m_emptySignals{QString(), m_logFile};
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
