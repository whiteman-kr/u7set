#include "TestSuiteDialogSettings.h"
#include "ui_TestSuiteDialogSettings.h"

TestSuiteDialogSettings::TestSuiteDialogSettings(const ClientLib::ClientTranslator& translator, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TestSuiteDialogSettings)
{
	ui->setupUi(this);
	createLanguagesList(translator);

	auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	if (okButton != nullptr)
	{
		okButton->setEnabled(AppConfigSettings::instance().wasLoadedFromFile() == false);
	}
	else
	{
		Q_ASSERT(okButton);
	}
}

TestSuiteDialogSettings::~TestSuiteDialogSettings()
{
	delete ui;
}

const AppConfigSettings::Data& TestSuiteDialogSettings::settings() const
{
	return m_settings;
}

void TestSuiteDialogSettings::setSettings(const AppConfigSettings::Data& settings)
{
	m_settings = settings;

	QString instanceHistoryString = QSettings().value("TestSuiteDialogSettings/instanceHistory", QString()).toString();
	QStringList instanceHistory = instanceHistoryString.split(';', Qt::SkipEmptyParts);

	ui->m_instanceCombo->addItems(instanceHistory);
	ui->m_instanceCombo->setCurrentText(m_settings.m_librarySettings.instanceStrId().toUpper());

	ui->m_IP1->setText(m_settings.m_librarySettings.configuratorAddress1().addressStr());
	ui->m_port1->setText(QString::number(m_settings.m_librarySettings.configuratorAddress1().port()));

	ui->m_IP2->setText(m_settings.m_librarySettings.configuratorAddress2().addressStr());
	ui->m_port2->setText(QString::number(m_settings.m_librarySettings.configuratorAddress2().port()));

	ui->loadSciptsPathCheck->setChecked(m_settings.m_useLocalScriptsPath);
	ui->loadSciptsPath->setEnabled(m_settings.m_useLocalScriptsPath);
	ui->loadSciptsPath->setText(m_settings.m_localScriptsPath);

	for (int i = 0; i < ui->m_languageCombo->count(); i++)
	{
		if (m_settings.m_language == ui->m_languageCombo->itemData(i).toString())
		{
			ui->m_languageCombo->setCurrentIndex(i);
			break;
		}
	}
}

void TestSuiteDialogSettings::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = this->screen()->availableGeometry();

	resize(static_cast<int>(screen.width() * 0.23), height());
	move(screen.center() - rect().center());

	return;
}

void TestSuiteDialogSettings::accept()
{
	auto d = parseData();

	if (d.has_value() == true)
	{
		if (d.value().m_localScriptsPath.isEmpty() == true && d.value().m_useLocalScriptsPath == true)
		{
			QMessageBox::warning(this, qAppName(), tr("Scripts are chosen to be loaded from a directoty. Please specify a directory with scripts files."));
			ui->tabWidget->setCurrentIndex(1);
			ui->loadSciptsPath->setFocus();
			return;
		}

		if (d.value().m_librarySettings.configuratorAddress1() != m_settings.m_librarySettings.configuratorAddress1() ||
			d.value().m_librarySettings.configuratorAddress2() != m_settings.m_librarySettings.configuratorAddress2())
		{
			QMessageBox::warning(this, tr("TuningClient"), tr("Configurator address has been changed, please restart the application."));
		}

		if (d.value().m_language != m_settings.m_language)
		{
			QMessageBox::warning(this, qAppName(), tr("Language has been changed, please restart the application."));
		}

		m_settings = d.value();
		QDialog::accept();
	}

	return;
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
	}
}

std::optional<AppConfigSettings::Data> TestSuiteDialogSettings::parseData()
{
	// ID
	//
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
	if (instanceStrId.isEmpty() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("Instance StrID cannot be empty"));
		mb.exec();

		ui->m_instanceCombo->setFocus();
		return {};
	}

	if (instanceHistory.contains(instanceStrId) == false)
	{
		instanceHistory.push_front(instanceStrId);
	}

	QSettings().setValue("TestSuiteDialogSettings/instanceHistory", instanceHistory.join(';'));

	// IP Configuration

	// Check ip address 1
	//
	QString configuratorIpAddress1 = ui->m_IP1->text();
	QHostAddress ha;
	if (ha.setAddress(configuratorIpAddress1) == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect format of the configurator IP Address."));
		mb.exec();

		ui->m_IP1->setFocus();
		ui->m_IP1->selectAll();
		return {};
	}

	// Check port num 1
	//
	bool convResult = false;
	int serverPort1 = ui->m_port1->text().toInt(&convResult);

	if (convResult == false || serverPort1 < 0 || serverPort1 > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->m_port1->setFocus();
		ui->m_port1->selectAll();
		return {};
	}

	// Check ip address 2
	//
	QString configuratorIpAddress2 = ui->m_IP2->text();
	if (ha.setAddress(configuratorIpAddress2) == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect format of the configurator IP Address."));
		mb.exec();

		ui->m_IP2->setFocus();
		ui->m_IP2->selectAll();
		return {};
	}

	// Check port num 2
	//
	int serverPort2 = ui->m_port1->text().toInt(&convResult);

	if (convResult == false || serverPort2 < 0 || serverPort2 > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->m_port2->setFocus();
		ui->m_port2->selectAll();
		return {};
	}

	// Language

	QString language = ui->m_languageCombo->currentData().toString();
	//

	// --
	//
	AppConfigSettings::Data data;

	data.m_librarySettings.setInstanceStrId(instanceStrId);

	data.m_librarySettings.setConfiguratorAddress1({configuratorIpAddress1, serverPort1});
	data.m_librarySettings.setConfiguratorAddress2({configuratorIpAddress2, serverPort2});

	data.m_useLocalScriptsPath = ui->loadSciptsPathCheck->isChecked() == true;
	data.m_localScriptsPath = ui->loadSciptsPath->text();

	data.m_language = language;

	return {data};
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

void TestSuiteDialogSettings::on_saveAsButton_clicked()
{
	auto d = parseData();

	if (d.has_value() == false)
	{
		return;
	}

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													path + QDir::separator(),
													tr("ini File (*.ini);;All Files (*.*)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	AppConfigSettings ts;
	ts.setData(d.value());

	if (bool ok = ts.saveToFile(fileName);
		ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("File %1 saving error.").arg(fileName));
	}

	return;
}

