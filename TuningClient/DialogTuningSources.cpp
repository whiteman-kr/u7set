#include "DialogTuningSources.h"
#include "MainWindow.h"

#include "TuningSourcesHelper.h"

ClientTuningSourcesWidget::ClientTuningSourcesWidget(ClientLib::TuningConnection& connection,
													 ClientLib::TuningUserManager& userManager,
													 bool hasActivationControls,
													 QWidget* parent):
	TuningSourcesWidget(connection, hasActivationControls, parent),
	m_userManager(userManager)
{

}

ClientTuningSourcesWidget::~ClientTuningSourcesWidget()
{

}

bool ClientTuningSourcesWidget::login()
{
	if (m_userManager.login(this) == false)
	{
		return false;
	}

	return true;
}

//
// ---
//

DialogTuningSources::DialogTuningSources(ClientLib::TuningConnection& tuningConnection, ClientLib::TuningUserManager& userManager, bool hasActivationControls, QWidget* parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningConnection(tuningConnection)
{
	setWindowTitle(tr("Tuning Data Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	m_tuningSourcesWidget = new ClientTuningSourcesWidget(tuningConnection, userManager, hasActivationControls, this);

	QHBoxLayout* bottomLayout = new QHBoxLayout();

	if (hasActivationControls == true)
	{
		m_btnEnableControl = new QPushButton(tr("Activate Control..."));
		m_btnEnableControl->setEnabled(false);
		connect(m_btnEnableControl, &QPushButton::clicked, m_tuningSourcesWidget, &TuningSourcesWidget::enableControlClicked);
		bottomLayout->addWidget(m_btnEnableControl);

		m_btnDisableControl = new QPushButton(tr("Deactivate Control..."));
		m_btnDisableControl->setEnabled(false);
		connect(m_btnDisableControl, &QPushButton::clicked, m_tuningSourcesWidget, &TuningSourcesWidget::disableControlClicked);
		bottomLayout->addWidget(m_btnDisableControl);

		connect(m_tuningSourcesWidget, &ClientTuningSourcesWidget::activationControlsAccessChanged,
				this, [this](bool activateEnabled, bool deactivateEnabled){
			m_btnEnableControl->setEnabled(activateEnabled);
			m_btnDisableControl->setEnabled(deactivateEnabled);
		});

		connect(m_tuningSourcesWidget, &ClientTuningSourcesWidget::activateSourceControl,
				this, [this](const QString& sourceEquipmentId, bool activate){
			ClientLib::TuningSourcesHelper::activateTuningSource(m_tuningConnection, sourceEquipmentId, activate, this);
		});

	}

	QPushButton* detailsButton = new QPushButton(tr("Details..."));
	bottomLayout->addWidget(detailsButton);
	detailsButton->setFocusPolicy(Qt::NoFocus);
	connect(detailsButton, &QPushButton::clicked, m_tuningSourcesWidget, &ClientTuningSourcesWidget::detailsClicked);

	bottomLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"));
	bottomLayout->addWidget(closeButton);
	closeButton->setFocusPolicy(Qt::NoFocus);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

	QVBoxLayout* l = new QVBoxLayout();
	l->addWidget(m_tuningSourcesWidget);
	l->addLayout(bottomLayout);
	setLayout(l);

	setMinimumSize(1150, 300);
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
