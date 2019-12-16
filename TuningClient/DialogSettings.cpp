#include "DialogSettings.h"
#include "ui_DialogSettings.h"
#include "Settings.h"
#include "MainWindow.h"

#include <QFileDialog>

DialogSettings::DialogSettings(QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);

	ui->m_instanceCombo->addItems(theSettings.instanceHistory());
	ui->m_instanceCombo->setCurrentText(theSettings.instanceStrId());

	ui->m_IP1->setText(theSettings.configuratorAddress1().addressStr());
	ui->m_port1->setText(QString::number(theSettings.configuratorAddress1().port()));

	ui->m_IP2->setText(theSettings.configuratorAddress2().addressStr());
	ui->m_port2->setText(QString::number(theSettings.configuratorAddress2().port()));

	ui->m_useCustomFilters->blockSignals(true);
	ui->m_useCustomFilters->setChecked(theSettings.useCustomFiltersFile() == true);
	ui->m_useCustomFilters->blockSignals(false);

	ui->m_customFiltersEdit->setText(theSettings.customFiltersFile());
	ui->m_customFiltersEdit->setEnabled(theSettings.useCustomFiltersFile());

#ifdef USE_ADMIN_REGISTRY_AREA
	if (theSettings.admin() == false)
	{
		ui->m_instanceCombo->setEnabled(false);
		ui->m_IP1->setEnabled(false);
		ui->m_port1->setEnabled(false);
		ui->m_IP2->setEnabled(false);
		ui->m_port2->setEnabled(false);

		ui->m_useCustomFilters->setEnabled(false);
		ui->m_customFiltersEdit->setEnabled(false);

	}
#endif

	createLanguagesList();
}

void DialogSettings::createLanguagesList()
{
	ui->m_languageCombo->addItem("English", "en");
	ui->m_languageCombo->setCurrentIndex(0);

	QDirIterator it(":/languages", QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		QString locale = it.next();
		locale.truncate(locale.lastIndexOf('.')); // "TuningClient_"
		locale.remove(0, locale.indexOf('_') + 1); // "de"

		QString lang = QLocale::languageToString(QLocale(locale).language());

		ui->m_languageCombo->addItem(lang, locale);

		if (theSettings.language() == locale)
		{
			ui->m_languageCombo->setCurrentIndex(ui->m_languageCombo->count() - 1);
		}
	}
}


DialogSettings::~DialogSettings()
{
	delete ui;
}

void DialogSettings::on_DialogSettings_accepted()
{
	// ID

	QStringList instanceHistory;
	for (int i = 0; i < ui->m_instanceCombo->count(); i++)
	{
		instanceHistory.push_back(ui->m_instanceCombo->itemText(i));

		if (instanceHistory.size() >= 10)
		{
			break;
		}
	}

	if (instanceHistory.contains(ui->m_instanceCombo->currentText()) == false)
	{
		instanceHistory.push_front(ui->m_instanceCombo->currentText());
	}

	theSettings.setInstanceHistory(instanceHistory);
	theSettings.setInstanceStrId(ui->m_instanceCombo->currentText());

	// IP Configuration

	QString configIP1 = ui->m_IP1->text();
	int configPort1 = ui->m_port1->text().toInt();

	QString configIP2 = ui->m_IP2->text();
	int configPort2 = ui->m_port2->text().toInt();

	if (configIP1 != theSettings.configuratorAddress1().addressStr() || configIP2 != theSettings.configuratorAddress2().addressStr()
			|| configPort1 != theSettings.configuratorAddress1().port() || configPort2 != theSettings.configuratorAddress2().port())
	{

		theSettings.setConfiguratorAddress1(configIP1, configPort1);
		theSettings.setConfiguratorAddress2(configIP2, configPort2);

		QMessageBox::warning(this, tr("TuningClient"), tr("Configurator address has been changed, please restart the application."));
	}

	// Language

	QVariant data = ui->m_languageCombo->currentData();

	QString lang = data.toString();

	if (lang != theSettings.language())
	{
		theSettings.setLanguage(lang);

		QMessageBox::warning(this, tr("TuningClient"), tr("Language has been changed, please restart the application."));
	}

	theSettings.setUseCustomFiltersFile(ui->m_useCustomFilters->isChecked() == true);
	theSettings.setCustomFiltersFile(ui->m_customFiltersEdit->text());

	//

#ifdef USE_ADMIN_REGISTRY_AREA
	if (theSettings.admin() == true)
	{
		theSettings.StoreSystem();
	}
#else
	theSettings.StoreSystem();
#endif

}

void DialogSettings::on_m_useCustomFilters_stateChanged(int arg1)
{
	Q_UNUSED(arg1);

	ui->m_customFiltersEdit->setEnabled(ui->m_useCustomFilters->isChecked() == true);
}

void DialogSettings::on_m_filtersBrowse_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Filters File"),
													QString(),
													tr("Filter Files (*.xml)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	ui->m_customFiltersEdit->setText(QDir::toNativeSeparators(fileName));

}
