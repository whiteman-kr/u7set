#include "DialogSettings.h"
#include "ui_DialogSettings.h"
#include "Settings.h"
#include "MainWindow.h"

DialogSettings::DialogSettings(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);



	ui->m_instanceID->setText(theSettings.instanceStrId());

	ui->m_IP1->setText(theSettings.configuratorAddress1().addressStr());
	ui->m_port1->setText(QString::number(theSettings.configuratorAddress1().port()));

	ui->m_IP2->setText(theSettings.configuratorAddress2().addressStr());
	ui->m_port2->setText(QString::number(theSettings.configuratorAddress2().port()));

	if (theSettings.admin() == false)
	{
		ui->m_instanceID->setEnabled(false);
		ui->m_IP1->setEnabled(false);
		ui->m_port1->setEnabled(false);
		ui->m_IP2->setEnabled(false);
		ui->m_port2->setEnabled(false);

	}

	ui->m_filterByEquipment->setChecked(theSettings.filterByEquipment());
	ui->m_filterBySchema->setChecked(theSettings.filterBySchema());

}

DialogSettings::~DialogSettings()
{
	delete ui;
}

void DialogSettings::on_DialogSettings_accepted()
{
	theSettings.setInstanceId(ui->m_instanceID->text());

    QString configIP1 = ui->m_IP1->text();
    int configPort1 = ui->m_port1->text().toInt();

    QString configIP2 = ui->m_IP2->text();
    int configPort2 = ui->m_port2->text().toInt();

    if (configIP1 != theSettings.configuratorAddress1().addressStr() || configIP2 != theSettings.configuratorAddress2().addressStr()
            || configPort1 != theSettings.configuratorAddress1().port() || configPort2 != theSettings.configuratorAddress2().port())
    {
        theSettings.setConfiguratorAddress1(configIP1, configPort1);
        theSettings.setConfiguratorAddress2(configIP2, configPort2);

        QMessageBox::warning(this, "TuningClient", tr("Configurator address has been changed, please restart the application."));
    }

	theSettings.setFilterByEquipment(ui->m_filterByEquipment->checkState() == Qt::Checked);
	theSettings.setFilterBySchema(ui->m_filterBySchema->checkState() == Qt::Checked);

	if (theSettings.admin() == true)
	{
		theSettings.StoreSystem();
	}
}
