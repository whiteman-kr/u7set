#include "DialogSettings.h"
#include "AppSettings.h"

#include <QCloseEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QString>

DialogSettings::DialogSettings(QDialog* parent) :
	QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("Settings"));

	m_ipEdit = new QLineEdit;
	m_portLocalEdit = new QLineEdit;
	m_portRemoteEdit = new QLineEdit;
	m_statusLabel = new QLabel;

	QFormLayout* layout = new QFormLayout;
	layout->addRow(tr("IP Address:"), m_ipEdit);
	layout->addRow(tr("Remote Port:"), m_portRemoteEdit);
	layout->addRow(tr("Local Port:"), m_portLocalEdit);
	layout->addWidget(m_statusLabel);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	QPushButton* okButton = new QPushButton(tr("OK"));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	connect(okButton, &QPushButton::clicked, this, &DialogSettings::accept);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	layout->addRow(buttonsLayout);

	setLayout(layout);

	setFixedSize(300, 180);

	// if OK :
	loadSettings();
	// else if Cancel : close
}

void DialogSettings::loadSettings()
{
	AppSettings asp = AppSettings::load();

	m_ipEdit->setText(asp.ip);
	m_portLocalEdit->setText(QString::number(asp.portLocal));
	m_portRemoteEdit->setText(QString::number(asp.portRemote));
}

void DialogSettings::saveSettings()
{
	AppSettings asp;

	asp.ip = m_ipEdit->text();
	asp.portLocal = m_portLocalEdit->text().toInt();
	asp.portRemote = m_portRemoteEdit->text().toInt();

	asp.save();
}

void DialogSettings::accept()
{
	saveSettings();
	QDialog::accept();
}