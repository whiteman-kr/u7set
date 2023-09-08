#include "TestSuiteDialogSettings.h"
#include "ui_TestSuiteDialogSettings.h"

TestSuiteDialogSettings::TestSuiteDialogSettings(const ClientLib::ClientTranslator& translator, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TestSuiteDialogSettings)
{
	ui->setupUi(this);
	createLanguagesList(translator);
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

	ui->loadSciptsPathCheck->setChecked(m_settings.useLocalScriptsPath());
	ui->loadSciptsPath->setEnabled(m_settings.useLocalScriptsPath());
	ui->loadSciptsPath->setText(m_settings.localScriptsPath());
}

const AppConfigSettings& TestSuiteDialogSettings::settings() const
{
	return m_settings;
}

void TestSuiteDialogSettings::createLanguagesList(const ClientLib::ClientTranslator& translator)
{
	QStringList languages = translator.languagesList();
	if (languages.isEmpty() == true)
	{
		ui->m_languageCombo->addItem("English", "en");
		ui->m_languageCombo->setCurrentIndex(0);
		ui->m_languageCombo->setEnabled(false);
		return;
	}

	for (const QString& code : languages)
	{
		QString name = translator.languageName(code);

		ui->m_languageCombo->addItem(name, code);

		if (theSettings.language() == code)
		{
			ui->m_languageCombo->setCurrentIndex(ui->m_languageCombo->count() - 1);
		}
	}
}

void TestSuiteDialogSettings::accept()
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

	QString instanceStrId = ui->m_instanceCombo->currentText().toUpper().trimmed();

	if (instanceHistory.contains(instanceStrId) == false)
	{
		instanceHistory.push_front(instanceStrId);
	}

	m_settings.setInstanceHistory(instanceHistory);

	m_settings.librarySettings().setInstanceStrId(instanceStrId);

	m_settings.setUseLocalScriptsPath(ui->loadSciptsPathCheck->isChecked() == true);
	m_settings.setLocalScriptsPath(ui->loadSciptsPath->text());

	if (m_settings.localScriptsPath().isEmpty() == true && m_settings.useLocalScriptsPath() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Scripts are chosen to be loaded from a directoty. Please specify a directory with scripts files."));
		ui->tabWidget->setCurrentIndex(1);
		ui->loadSciptsPath->setFocus();
		return;
	}

	// IP Configuration
	HostAddressPort address1{ui->m_IP1->text(), ui->m_port1->text().toInt()};
	HostAddressPort address2{ui->m_IP2->text(), ui->m_port2->text().toInt()};

	if (address1 != m_settings.librarySettings().configuratorAddress1() ||
		address2 != m_settings.librarySettings().configuratorAddress2())
	{

		m_settings.librarySettings().setConfiguratorAddress1(address1);
		m_settings.librarySettings().setConfiguratorAddress2(address2);

		QMessageBox::warning(this, qAppName(), tr("Configurator address has been changed, please restart the application."));
	}

	// Language


	QVariant data = ui->m_languageCombo->currentData();

	QString lang = data.toString();

	if (lang != m_settings.language())
	{
		m_settings.setLanguage(lang);
		QMessageBox::warning(this, qAppName(), tr("Language has been changed, please restart the application."));
	}

	//

	QDialog::accept();
}

void TestSuiteDialogSettings::on_TestSuiteDialogSettings_accepted()
{
	
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

