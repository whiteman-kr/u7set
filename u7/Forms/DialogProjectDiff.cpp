#include "DialogProjectDiff.h"
#include "ui_DialogProjectDiff.h"
#include "SelectChangesetDialog.h"
#include "Reports/DialogSchemasReport.h"
#include "Reports/DialogReportPageSetup.h"

#include <QPageSetupDialog>
#include <QPrinter>

//
// DialogProjectDiff
//

ProjectDiffReportParams DialogProjectDiff::m_reportParams;

DialogProjectDiff::DialogProjectDiff(DbController* db, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogProjectDiff),
	m_db(db)
{
	assert(m_db);

	ui->setupUi(this);

	// --
	//
	QSettings s;

	QString defaultPath = QDir::toNativeSeparators(tr("%1%2%3").arg(qApp->applicationDirPath()).arg(QDir::separator()).arg("DiffReport.pdf"));
	m_fileName = s.value("ProjectDiffGenerator/fileName", defaultPath).toString();

	ui->reportFileEdit->setText(fileName());

	QStringList versionTypes;

	versionTypes << tr("Changeset");			// VersionType::Changeset
	versionTypes << tr("Date");					// VersionType::Date
	versionTypes << tr("Latest Version");		// VersionType::LatestVersion

	ui->sourceTypeComboBox->addItems(versionTypes);
	ui->targetTypeComboBox->addItems(versionTypes);

	int sourceTypeIndex = s.value("ProjectDiffGenerator/sourceType", static_cast<int>(CompareVersionType::Changeset)).toInt();
	if (sourceTypeIndex < 0 || sourceTypeIndex > static_cast<int>(CompareVersionType::Changeset))
	{
		sourceTypeIndex = static_cast<int>(CompareVersionType::Changeset);
	}
	ui->sourceTypeComboBox->setCurrentIndex(sourceTypeIndex);

	int targetTypeIndex = s.value("ProjectDiffGenerator/targetType", static_cast<int>(CompareVersionType::LatestVersion)).toInt();
	if (targetTypeIndex < 0 || targetTypeIndex > static_cast<int>(CompareVersionType::Changeset))
	{
		targetTypeIndex = static_cast<int>(CompareVersionType::LatestVersion);
	}
	ui->targetTypeComboBox->setCurrentIndex(targetTypeIndex);

	int sourceChangeset = s.value("ProjectDiffGenerator/sourceChangeset", 1).toInt();
	ui->sourceChangesetLineEdit->setText(QString::number(sourceChangeset));

	int targetChangeset = s.value("ProjectDiffGenerator/targetChangeset", 1).toInt();
	ui->targetChangesetLineEdit->setText(QString::number(targetChangeset));

	ui->sourceDateEdit->setDateTime(s.value("ProjectDiffGenerator/sourceDateTime", QDateTime::currentDateTime()).toDateTime());
	ui->targetDateEdit->setDateTime(s.value("ProjectDiffGenerator/targetDateTime", QDateTime::currentDateTime()).toDateTime());

	versionTypeChanged();

	// --
	//
	connect(ui->sourceTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogProjectDiff::versionTypeChanged);
	connect(ui->targetTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogProjectDiff::versionTypeChanged);

	// Select default file types if they are not selected
	//
	if (m_reportParams.schemaTypesParams.empty() == true)
	{
		m_reportParams.schemaTypesParams = ProjectDiffGenerator::defaultFileTypeParams(db);
	}

	// Fill file types list
	//
	for (const Builder::SchemaTypesParams& ft : m_reportParams.schemaTypesParams)
	{
		if (ft.hasFileId() == false)
		{
			continue;
		}

		QListWidgetItem* item = new QListWidgetItem(tr("%1").arg(ft.caption()));
		item->setCheckState(ft.selected() ? Qt::Checked : Qt::Unchecked);
		item->setData(Qt::UserRole, ft.fileId());
		ui->categoriesList->addItem(item);
	}

	m_reportParams.singleFile = s.value("ProjectDiffGenerator/singleFile", true).toBool();
	ui->singleFileReportsCheck->setChecked(m_reportParams.singleFile == true ? Qt::Checked : Qt::Unchecked);

	m_reportParams.expertProperties = s.value("ProjectDiffGenerator/expertProperties", false).toBool();
	ui->expertPropertiesCheck->setChecked(m_reportParams.expertProperties == true ? Qt::Checked : Qt::Unchecked);

	return;

}

DialogProjectDiff::~DialogProjectDiff()
{
	QSettings s;

	s.setValue("ProjectDiffGenerator/fileName", fileName());

	int sourceTypeIndex = ui->sourceTypeComboBox->currentIndex();
	if (sourceTypeIndex < 0 || sourceTypeIndex > static_cast<int>(CompareVersionType::Changeset))
	{
		sourceTypeIndex = static_cast<int>(CompareVersionType::Changeset);
	}
	s.setValue("ProjectDiffGenerator/sourceType", sourceTypeIndex);

	int targetTypeIndex = ui->targetTypeComboBox->currentIndex();
	if (targetTypeIndex < 0 || targetTypeIndex > static_cast<int>(CompareVersionType::Changeset))
	{
		targetTypeIndex = static_cast<int>(CompareVersionType::LatestVersion);
	}
	s.setValue("ProjectDiffGenerator/targetType", targetTypeIndex);

	int sourceChangeset = ui->sourceChangesetLineEdit->text().toInt();
	s.setValue("ProjectDiffGenerator/sourceChangeset", sourceChangeset);

	int targetChangeset = ui->targetChangesetLineEdit->text().toInt();
	s.setValue("ProjectDiffGenerator/targetChangeset", targetChangeset);

	QDateTime sourceDateTime = ui->sourceDateEdit->dateTime();
	s.setValue("ProjectDiffGenerator/sourceDateTime", sourceDateTime);

	QDateTime targetDateTime = ui->targetDateEdit->dateTime();
	s.setValue("ProjectDiffGenerator/targetDateTime", targetDateTime);

	s.setValue("ProjectDiffGenerator/singleFile", m_reportParams.singleFile);

	s.setValue("ProjectDiffGenerator/expertProperties", m_reportParams.expertProperties);

	delete ui;
}

QString DialogProjectDiff::fileName() const
{
	return m_fileName;
}

ProjectDiffReportParams DialogProjectDiff::reportParams() const
{
	return m_reportParams;
}

void DialogProjectDiff::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = parentWidget()->screen()->availableGeometry();

	resize(static_cast<int>(screen.width() * 0.25),
		   rect().height());

	move(screen.center() - rect().center());

	return;
}

void DialogProjectDiff::done(int r)
{
	if (r == QDialog::Rejected)
	{
		QDialog::done(r);
		return;
	}

	// Save Filename

	if (ui->reportFileEdit->text().isEmpty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Please enter report file name!"));
		ui->reportFileEdit->setFocus();
		return;
	}

	m_fileName = ui->reportFileEdit->text();

	// Save Compare Data

	CompareData compareData;

	// Source
	//
	compareData.sourceVersionType = static_cast<CompareVersionType>(ui->sourceTypeComboBox->currentIndex());

	bool sourceChangesetConversionOk = false;
	compareData.sourceChangeset = ui->sourceChangesetLineEdit->text().toInt(&sourceChangesetConversionOk);

	compareData.sourceDate = ui->sourceDateEdit->dateTime();

	// Target
	//
	compareData.targetVersionType = static_cast<CompareVersionType>(ui->targetTypeComboBox->currentIndex());

	bool targetChangesetConversionOk = false;
	compareData.targetChangeset = ui->targetChangesetLineEdit->text().toInt(&targetChangesetConversionOk);

	compareData.targetDate = ui->targetDateEdit->dateTime();

	// Checks
	//
	if (compareData.sourceVersionType == CompareVersionType::Changeset &&
		sourceChangesetConversionOk == false)
	{
		ui->sourceChangesetLineEdit->setFocus();
		ui->sourceChangesetLineEdit->selectAll();
		return;
	}

	if (compareData.sourceVersionType == CompareVersionType::Date &&
		compareData.sourceDate.isValid() == false)
	{
		ui->sourceDateEdit->setFocus();
		ui->sourceDateEdit->selectAll();
		return;
	}

	if (compareData.targetVersionType == CompareVersionType::Changeset &&
		targetChangesetConversionOk == false)
	{
		ui->targetChangesetLineEdit->setFocus();
		ui->targetChangesetLineEdit->selectAll();
		return;
	}

	if (compareData.targetVersionType == CompareVersionType::Date &&
		compareData.targetDate.isValid() == false)
	{
		ui->targetDateEdit->setFocus();
		ui->targetDateEdit->selectAll();
		return;
	}

	if (compareData.sourceVersionType == compareData.targetVersionType &&
		compareData.sourceDate == compareData.targetDate &&
		compareData.sourceChangeset == compareData.targetChangeset)
	{
		QMessageBox::critical(this, qAppName(), tr("Please select different changesets!"));
		return;
	}

	m_reportParams.compareData = compareData;

	// Save File Types

	int selectedCount = 0;

	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);
		if (item->checkState() == Qt::Checked)
		{
			selectedCount++;
		}

		int fileId = item->data(Qt::UserRole).toInt();

		for (auto& stp : m_reportParams.schemaTypesParams)
		{
			if (fileId == stp.fileId())
			{
				stp.setSelected(item->checkState() == Qt::Checked);
			}
		}
	}

	if (selectedCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Please select at least one file category!"));
		return;
	}

	// Save options

	m_reportParams.singleFile = ui->singleFileReportsCheck->isChecked() == true;
	m_reportParams.expertProperties = ui->expertPropertiesCheck->isChecked() == true;

	QDialog::done(r);
}

void DialogProjectDiff::versionTypeChanged()
{
	// Source
	//
	CompareVersionType sourceType = static_cast<CompareVersionType>(ui->sourceTypeComboBox->currentIndex());

	switch (sourceType)
	{
	case CompareVersionType::Changeset:
		ui->sourceChangesetLabel->setEnabled(true);
		ui->sourceChangesetLineEdit->setEnabled(true);
		ui->sourceChangesetButton->setEnabled(true);
		ui->sourceDateLabel->setEnabled(false);
		ui->sourceDateEdit->setEnabled(false);
		break;
	case CompareVersionType::Date:
		ui->sourceChangesetLabel->setEnabled(false);
		ui->sourceChangesetLineEdit->setEnabled(false);
		ui->sourceChangesetButton->setEnabled(false);
		ui->sourceDateLabel->setEnabled(true);
		ui->sourceDateEdit->setEnabled(true);
		break;
	case CompareVersionType::LatestVersion:
		ui->sourceChangesetLabel->setEnabled(false);
		ui->sourceChangesetLineEdit->setEnabled(false);
		ui->sourceChangesetButton->setEnabled(false);
		ui->sourceDateLabel->setEnabled(false);
		ui->sourceDateEdit->setEnabled(false);
		break;
	default:
		assert(false);
	}

	// Target
	//
	CompareVersionType targetType = static_cast<CompareVersionType>(ui->targetTypeComboBox->currentIndex());

	switch (targetType)
	{
	case CompareVersionType::Changeset:
		ui->targetChangesetLabel->setEnabled(true);
		ui->targetChangesetLineEdit->setEnabled(true);
		ui->targetChangesetButton->setEnabled(true);
		ui->targetDateLabel->setEnabled(false);
		ui->targetDateEdit->setEnabled(false);
		break;
	case CompareVersionType::Date:
		ui->targetChangesetLabel->setEnabled(false);
		ui->targetChangesetLineEdit->setEnabled(false);
		ui->targetChangesetButton->setEnabled(false);
		ui->targetDateLabel->setEnabled(true);
		ui->targetDateEdit->setEnabled(true);
		break;
	case CompareVersionType::LatestVersion:
		ui->targetChangesetLabel->setEnabled(false);
		ui->targetChangesetLineEdit->setEnabled(false);
		ui->targetChangesetButton->setEnabled(false);
		ui->targetDateLabel->setEnabled(false);
		ui->targetDateEdit->setEnabled(false);
		break;
	default:
		assert(false);
	}

	return;
}

void DialogProjectDiff::on_sourceChangesetButton_clicked()
{
	int changeset = SelectChangesetDialog::getProjectChangeset(m_db, this);

	if (changeset != -1)
	{
		ui->sourceChangesetLineEdit->setText(QString::number(changeset));
	}
}

void DialogProjectDiff::on_targetChangesetButton_clicked()
{
	int changeset = SelectChangesetDialog::getProjectChangeset(m_db, this);

	if (changeset != -1)
	{
		ui->targetChangesetLineEdit->setText(QString::number(changeset));
	}
}

void DialogProjectDiff::on_buttonSelectAll_clicked()
{
	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);
		item->setCheckState(Qt::Checked);
	}
}

void DialogProjectDiff::on_buttonSelectNone_clicked()
{
	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);
		item->setCheckState(Qt::Unchecked);
	}
}

void DialogProjectDiff::on_categoriesList_itemPressed(QListWidgetItem *item)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	if (item->checkState() == Qt::Checked)
	{
		item->setCheckState(Qt::Unchecked);
	}
	else
	{
		item->setCheckState(Qt::Checked);
	}

	return;
}

void DialogProjectDiff::on_fileBrowseButton_clicked()
{
	// Get filename
	//
	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Diff Report"),
													path + QDir::separator(),
													QObject::tr("PDF documents (*.pdf)"));
	if (fileName.isNull() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	fileName = QDir::toNativeSeparators(fileName);

	m_fileName = fileName;
	ui->reportFileEdit->setText(fileName);

	return;
}

void DialogProjectDiff::on_multiFilepageSetupButton_clicked()
{
	std::vector<Builder::SchemaTypesParams> editSchemaTypesParams;

	bool singleFile = ui->singleFileReportsCheck->isChecked();

	for (const auto& params : m_reportParams.schemaTypesParams)
	{
		// Show single file param if single file option is set, otherwise show all others except single file
		//
		bool singleFileParam = params.hasFileId() == false;
		if (singleFileParam == singleFile)
		{
			editSchemaTypesParams.push_back(params);
		}
	}

	DialogReportPageSetup d(editSchemaTypesParams, ProjectDiffGenerator::defaultFileTypeParams(m_db), this);
	if (d.exec() == QDialog::Accepted)
	{
		editSchemaTypesParams = d.schemaTypesParams();

		for (auto& params : m_reportParams.schemaTypesParams)
		{
			for (const auto& editParams : editSchemaTypesParams)
			{
				if (params.fileId() == editParams.fileId())
				{
					params = editParams;
				}
			}
		}
	}
}

