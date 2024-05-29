#include "DialogSettings.h"
#include "ui_DialogSettings.h"
#include "Settings.h"
#include "MainWindow.h"

DialogSettings::DialogSettings(const ClientLib::ClientTranslator& translator, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);

	// Fill Languages List
	//
	createLanguagesList(translator);

	auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	if (okButton != nullptr)
	{
		okButton->setEnabled(TuningClientAppSettings::instance().wasLoadedFromFile() == false);
	}
	else
	{
		Q_ASSERT(okButton);
	}
}

DialogSettings::~DialogSettings()
{
	delete ui;
}

const TuningClientAppSettings::SystemData& DialogSettings::settings() const
{
	return m_settings;
}

void DialogSettings::setSettings(const TuningClientAppSettings::SystemData& value)
{
	m_settings = value;

	QString instanceHistoryString = QSettings().value("DialogSettings/instanceHistory", QString()).toString();
	QStringList instanceHistory = instanceHistoryString.split(';', Qt::SkipEmptyParts);

	ui->m_instanceCombo->addItems(instanceHistory);
	ui->m_instanceCombo->setCurrentText(value.m_instanceStrId.toUpper());

	ui->m_IP1->setText(value.m_configuratorIpAddress1);
	ui->m_port1->setText(QString::number(value.m_configuratorPort1));

	ui->m_IP2->setText(value.m_configuratorIpAddress2);
	ui->m_port2->setText(QString::number(value.m_configuratorPort2));

	ui->m_useCustomFilters->blockSignals(true);
	ui->m_useCustomFilters->setChecked(value.m_useFiltersCustomFile == true);
	ui->m_useCustomFilters->blockSignals(false);

	ui->m_customFiltersEdit->setText(value.m_filtersCustomFile);
	ui->m_customFiltersEdit->setEnabled(value.m_useFiltersCustomFile == true);

	for (int i = 0; i < ui->m_languageCombo->count(); i++)
	{
		if (m_settings.m_language == ui->m_languageCombo->itemData(i).toString())
		{
			ui->m_languageCombo->setCurrentIndex(i);
			break;
		}
	}

	return;
}

void DialogSettings::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = this->screen()->availableGeometry();

	resize(static_cast<int>(screen.width() * 0.23), height());
	move(screen.center() - rect().center());

	return;
}

void DialogSettings::accept()
{
	auto d = parseData();

	if (d.has_value() == true)
	{
		if (d.value().m_configuratorIpAddress1 != m_settings.m_configuratorIpAddress1 || d.value().m_configuratorIpAddress2 != m_settings.m_configuratorIpAddress2 
			|| d.value().m_configuratorPort1 != m_settings.m_configuratorPort1 || d.value().m_configuratorPort2 != m_settings.m_configuratorPort2)
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

void DialogSettings::createLanguagesList(const ClientLib::ClientTranslator& translator)
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

std::optional<TuningClientAppSettings::SystemData> DialogSettings::parseData()
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

	QSettings().setValue("DialogSettings/instanceHistory", instanceHistory.join(';'));

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

	// --
	//
	TuningClientAppSettings::SystemData data;

	data.m_instanceStrId = instanceStrId;

	data.m_configuratorIpAddress1 = configuratorIpAddress1;
	data.m_configuratorPort1 = serverPort1;

	data.m_configuratorIpAddress2 = configuratorIpAddress2;
	data.m_configuratorPort2 = serverPort2;

	data.m_useFiltersCustomFile = ui->m_useCustomFilters->isChecked() == true;
	data.m_filtersCustomFile = ui->m_customFiltersEdit->text();

	data.m_language = language;

	return {data};
}

void DialogSettings::on_m_useCustomFilters_stateChanged(int arg1)
{
	Q_UNUSED(arg1);

	ui->m_customFiltersEdit->setEnabled(ui->m_useCustomFilters->isChecked() == true);
}

void DialogSettings::on_m_filtersBrowse_clicked()
{
	static QString path{"."};
	QString fileName = QFileDialog::getOpenFileName(this, tr("Filters File"),
													path,
													tr("Filter Files (*.xml)"));
	if (fileName.isNull() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	ui->m_customFiltersEdit->setText(QDir::toNativeSeparators(fileName));
}

void DialogSettings::on_saveAsButton_clicked()
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

	TuningClientAppSettings ts;
	ts.setSystem(d.value());

	if (bool ok = ts.saveToFile(fileName);
		ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("File %1 saving error.").arg(fileName));
	}

	return;
}


