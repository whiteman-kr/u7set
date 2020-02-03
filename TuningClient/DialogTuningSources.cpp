#include "DialogTuningSources.h"
#include "MainWindow.h"

ClientTuningSourcesWidget::ClientTuningSourcesWidget(TuningTcpClient* tcpClient, bool hasActivationControls, bool hasCloseButton, QWidget* parent):
	TuningSourcesWidget(tcpClient, hasActivationControls, hasCloseButton, parent)
{

}

ClientTuningSourcesWidget::~ClientTuningSourcesWidget()
{

}

bool ClientTuningSourcesWidget::login()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return false;
	}

	return true;
}

//
// ---
//

DialogTuningSources::DialogTuningSources(TuningTcpClient* tcpClient, bool hasActivationControls, QWidget* parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Tuning Data Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	m_tuningSourcesWidget = new ClientTuningSourcesWidget(tcpClient, hasActivationControls, true, this);

	connect(m_tuningSourcesWidget, &TuningSourcesWidget::closeButtonPressed, this, &DialogTuningSources::reject);

	QHBoxLayout* l = new QHBoxLayout();
	l->addWidget(m_tuningSourcesWidget);
	setLayout(l);

	setMinimumSize(1024, 300);
}

DialogTuningSources::~DialogTuningSources()
{
}

void DialogTuningSources::reject()
{
	emit dialogClosed();
	QDialog::reject();
}

DialogTuningSources* theDialogTuningSources = nullptr;
