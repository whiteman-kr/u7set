#include "TestSuiteDialogSettings.h"
#include "ui_TestSuiteDialogSettings.h"

TestSuiteDialogSettings::TestSuiteDialogSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TestSuiteDialogSettings)
{
	ui->setupUi(this);
}

TestSuiteDialogSettings::~TestSuiteDialogSettings()
{
	delete ui;
}

void TestSuiteDialogSettings::setSettings(const AppConfigSettings& settings)
{
	m_settings = settings;

	ui->m_instanceCombo->addItems(m_settings.instanceHistory());
	ui->m_instanceCombo->setCurrentText(m_settings.librarySettings().instanceStrId().toUpper());

	ui->m_IP1->setText(m_settings.librarySettings().configuratorAddress1().addressStr());
	ui->m_port1->setText(QString::number(m_settings.librarySettings().configuratorAddress1().port()));

	ui->m_IP2->setText(m_settings.librarySettings().configuratorAddress2().addressStr());
	ui->m_port2->setText(QString::number(m_settings.librarySettings().configuratorAddress2().port()));

	ui->loadSciptsPathCheck->setChecked(m_settings.librarySettings().loadScriptsFromPath());
	ui->loadSciptsPath->setEnabled(m_settings.librarySettings().loadScriptsFromPath());
	ui->loadSciptsPath->setText(m_settings.librarySettings().scriptsPath());
}

const AppConfigSettings& TestSuiteDialogSettings::settings() const
{
	return m_settings;
}

void TestSuiteDialogSettings::on_TestSuiteDialogSettings_accepted()
{
	// ID

	QStringList instanceHistory;
	for (int i = 0; i < ui->m_instanceCombo->count(); i++)
	{
		instanceHistory.push_back(ui->m_instanceCombo->itemText(i).toUpper());

		if (instanceHistory.size() >= 10)
		{
			break;
		}
	}

	QString instanceStrId = ui->m_instanceCombo->currentText().toUpper();

	if (instanceHistory.contains(instanceStrId) == false)
	{
		instanceHistory.push_front(instanceStrId);
	}

	m_settings.setInstanceHistory(instanceHistory);

	m_settings.librarySettings().setInstanceStrId(instanceStrId);

	m_settings.librarySettings().setLoadScriptsFromPath(ui->loadSciptsPathCheck->isChecked() == true);
	m_settings.librarySettings().setScriptsPath(ui->loadSciptsPath->text());

	// IP Configuration

	QString configIP1 = ui->m_IP1->text();
	int configPort1 = ui->m_port1->text().toInt();

	QString configIP2 = ui->m_IP2->text();
	int configPort2 = ui->m_port2->text().toInt();

	if (configIP1 != m_settings.librarySettings().configuratorAddress1().addressStr() || configIP2 != m_settings.librarySettings().configuratorAddress2().addressStr()
			|| configPort1 != m_settings.librarySettings().configuratorAddress1().port() || configPort2 != m_settings.librarySettings().configuratorAddress2().port())
	{

		m_settings.librarySettings().setConfiguratorAddress1(configIP1, configPort1);
		m_settings.librarySettings().setConfiguratorAddress2(configIP2, configPort2);

		QMessageBox::warning(this, qAppName(), tr("Configurator address has been changed, please restart the application."));
	}

	// Language
	/*

	QVariant data = ui->m_languageCombo->currentData();

	QString lang = data.toString();

	if (lang != m_settings.language())
	{
		m_settings.setLanguage(lang);

		QMessageBox::warning(this, qAppName(), tr("Language has been changed, please restart the application."));
	}
*/
	//
}


void TestSuiteDialogSettings::on_loadSciptsPathBrowse_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Scripts Directory"),
												 ui->loadSciptsPath->text(),
												 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (dir.isEmpty() == false)
	{
		ui->loadSciptsPath->setText(QDir::toNativeSeparators(dir));
	}
}


void TestSuiteDialogSettings::on_loadSciptsPathCheck_stateChanged(int /*arg1*/)
{
	ui->loadSciptsPath->setEnabled(ui->loadSciptsPathCheck->isChecked() == true);

}

