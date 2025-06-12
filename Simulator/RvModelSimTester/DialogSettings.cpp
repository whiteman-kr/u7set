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
	setWindowTitle("Settings");

	ipEdit = new QLineEdit;
	portLocalEdit = new QLineEdit;
	portRemoteEdit = new QLineEdit;
	statusLabel = new QLabel;

	QFormLayout* layout = new QFormLayout;
	layout->addRow("IP Address:", ipEdit);
	layout->addRow("Remote Port:", portRemoteEdit);
	layout->addRow("Local Port:", portLocalEdit);
	layout->addWidget(statusLabel);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	QPushButton* okButton = new QPushButton("OK");
	QPushButton* cancelButton = new QPushButton("Cancel");

	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	connect(okButton, &QPushButton::clicked, this, &DialogSettings::accept);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	layout->addRow(buttonsLayout);

	setLayout(layout);

	setFixedSize(300, 180);

	// if extept OK :
	loadSettings();
	// else if extept Cancel : close
}

void DialogSettings::loadSettings()
{
	AppSettings asp = AppSettings::load();

	ipEdit->setText(asp.ip);
	portLocalEdit->setText(QString::number(asp.portLocal));
	portRemoteEdit->setText(QString::number(asp.portRemote));
}

void DialogSettings::saveSettings()
{
	AppSettings asp;

	asp.ip = ipEdit->text();
	asp.portLocal = portLocalEdit->text().toInt();
	asp.portRemote = portRemoteEdit->text().toInt();

	asp.save();
}

void DialogSettings::accept()
{
	saveSettings();
	QDialog::accept();
}