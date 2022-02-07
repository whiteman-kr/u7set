#include "DialogDataSources.h"

DialogDataSources::DialogDataSources(TcpAppSourcesState* tcpAppSourceState, bool showTuningWidget, std::vector<TuningTcpClient*> tcpTuningClients, bool hasActivationControls, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Data Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	m_mainLayout = new QVBoxLayout();

	// AppData

	if (tcpAppSourceState == nullptr)
	{
		Q_ASSERT(tcpAppSourceState);
		return;
	}

	QLabel* l = new QLabel(tr("Application Data Sources"));
	m_mainLayout->addWidget(l);

	m_appDataSourcesWidget = new AppDataSourcesWidget(tcpAppSourceState, showTuningWidget == false /*closeButton*/, this);

	connect(m_appDataSourcesWidget, &AppDataSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

	m_mainLayout->addWidget(m_appDataSourcesWidget);

	// Tuning
	//
	setTuningTcpClients(showTuningWidget, std::move(tcpTuningClients), hasActivationControls);

	// --
	//
	setLayout(m_mainLayout);

	return;
}

void DialogDataSources::setTuningTcpClients(bool showTuningWidget, std::vector<TuningTcpClient*> tcpTuningClients, bool hasActivationControls)
{
	if (showTuningWidget == true)
	{
		// Show tuning widget

		if (m_tuningSourcesLabel == nullptr)
		{
			m_tuningSourcesLabel = new QLabel(tr("Tuning Data Sources"));
			m_mainLayout->addWidget(m_tuningSourcesLabel);
		}

		if (m_tuningSourcesWidget == nullptr)
		{
			m_tuningSourcesWidget = new TuningSourcesWidget(std::move(tcpTuningClients), hasActivationControls, true, this);

			connect(m_tuningSourcesWidget, &TuningSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

			m_mainLayout->addWidget(m_tuningSourcesWidget);
		}
		else
		{
			m_tuningSourcesWidget->setTuningTcpClients(std::move(tcpTuningClients));
		}
	}
	else
	{
		// Delete tuning widget

		if (m_tuningSourcesLabel != nullptr)
		{
			delete m_tuningSourcesLabel;
			m_tuningSourcesLabel = nullptr;
		}

		if (m_tuningSourcesWidget != nullptr)
		{
			delete m_tuningSourcesWidget;
			m_tuningSourcesWidget = nullptr;
		}
	}

	m_appDataSourcesWidget->showCloseButton(showTuningWidget == false);

	if (showTuningWidget == true)
	{
		setMinimumSize(1024, 500);
	}
	else
	{
		setMinimumSize(1024, 300);
	}
}

void DialogDataSources::reject()
{
	emit dialogClosed();
	QDialog::reject();
}
