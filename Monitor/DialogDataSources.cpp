#include "DialogDataSources.h"

DialogDataSources::DialogDataSources(TcpAppSourcesState* tcpAppSourceState, bool showTuningWidget, TuningTcpClient* tcpTuningClient, bool hasActivationControls, QWidget* parent)
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

	setTuningTcpClient(showTuningWidget, tcpTuningClient, hasActivationControls);

	//

	setLayout(m_mainLayout);

}

DialogDataSources::~DialogDataSources()
{
}

void DialogDataSources::setTuningTcpClient(bool showTuningWidget, TuningTcpClient* tcpTuningClient, bool hasActivationControls)
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
			if (tcpTuningClient == nullptr)
			{
				Q_ASSERT(tcpTuningClient);
				return;
			}

			m_tuningSourcesWidget = new TuningSourcesWidget(tcpTuningClient, hasActivationControls, true, this);

			connect(m_tuningSourcesWidget, &TuningSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

			m_mainLayout->addWidget(m_tuningSourcesWidget);
		}
		else
		{
			m_tuningSourcesWidget->setTuningTcpClient(tcpTuningClient);
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
