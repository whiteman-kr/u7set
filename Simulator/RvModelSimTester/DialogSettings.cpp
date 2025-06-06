#include "DialogSettings.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QString>
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include <QPushButton>

DialogSettings::DialogSettings(QDialog* parent) :
	QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("Settings");

    ipEdit = new QLineEdit;
    portInEdit = new QLineEdit;
    portOutEdit = new QLineEdit;
    statusLabel = new QLabel;

    QFormLayout* layout = new QFormLayout;
    layout->addRow("IP Address:", ipEdit);
	layout->addRow("Port In:", portInEdit);
	layout->addRow("Port Out:", portOutEdit);
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

    //if extept OK :
    loadSettings();
	// else if extept Cancel : close
}

void DialogSettings::loadSettings()
{
    QSettings settings(settingsFile, QSettings::IniFormat);
    settings.beginGroup("Network");
    ipEdit->setText(settings.value("IP", "").toString());
    portInEdit->setText(settings.value("PortIn", "").toString());
    portOutEdit->setText(settings.value("PortOut", "").toString());
    settings.endGroup();
}

void DialogSettings::saveSettings()
{
    QSettings settings(settingsFile, QSettings::IniFormat);
    settings.beginGroup("Network");
    settings.setValue("IP", ipEdit->text());
    settings.setValue("PortIn", portInEdit->text());
    settings.setValue("PortOut", portOutEdit->text());
    settings.endGroup();
}

void DialogSettings::accept()
{
    saveSettings();
    QDialog::accept();
}