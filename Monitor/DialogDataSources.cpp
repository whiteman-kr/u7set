#include "DialogDataSources.h"

DialogDataSources::DialogDataSources(TcpAppSourcesState* tcpAppSourceState, bool showTuningWidget, TuningTcpClient* tcpTuningClient, bool hasActivationControls, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Data Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	QVBoxLayout* layout = new QVBoxLayout();

	// AppData

	if (tcpAppSourceState == nullptr)
	{
		assert(tcpAppSourceState);
		return;
	}

	QLabel* l = new QLabel(tr("Application Data Sources"));
	layout->addWidget(l);

	m_appDataSourcesWidget = new AppDataSourcesWidget(tcpAppSourceState, showTuningWidget == false /*closeButton*/, this);

	connect(m_appDataSourcesWidget, &AppDataSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

	layout->addWidget(m_appDataSourcesWidget);

	// Tuning

	if (showTuningWidget == true)
	{
		if (tcpTuningClient == nullptr)
		{
			assert(tcpTuningClient);
			return;
		}

		l = new QLabel(tr("Tuning Data Sources"));
		layout->addWidget(l);

		m_tuningSourcesWidget = new TuningSourcesWidget(tcpTuningClient, hasActivationControls, true, this);

		connect(m_tuningSourcesWidget, &TuningSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

		layout->addWidget(m_tuningSourcesWidget);

		setMinimumSize(1024, 500);
	}
	else
	{
		setMinimumSize(1024, 300);
	}

	//

	setLayout(layout);

}

DialogDataSources::~DialogDataSources()
{
}

void DialogDataSources::reject()
{
	emit dialogClosed();
	QDialog::reject();
}

DialogDataSources* theDialogDataSources = nullptr;

