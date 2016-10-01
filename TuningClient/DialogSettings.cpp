#include "DialogSettings.h"
#include "ui_DialogSettings.h"
#include "Settings.h"

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

	theSettings.setConfiguratorAddress1(ui->m_IP1->text(), ui->m_port1->text().toInt());
	theSettings.setConfiguratorAddress2(ui->m_IP2->text(), ui->m_port2->text().toInt());

	theSettings.setFilterByEquipment(ui->m_filterByEquipment->checkState() == Qt::Checked);
	theSettings.setFilterBySchema(ui->m_filterBySchema->checkState() == Qt::Checked);

	theSettings.StoreSystem();
}
