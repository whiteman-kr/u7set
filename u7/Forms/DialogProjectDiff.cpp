#include "DialogProjectDiff.h"
#include "ui_DialogProjectDiff.h"
#include "../../lib/DbController.h"
#include "SelectChangesetDialog.h"

ProjectDiffParams DialogProjectDiff::m_diffParams;

DialogProjectDiff::DialogProjectDiff(DbController* db, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogProjectDiff),
	m_db(db)
{
	assert(m_db);

	ui->setupUi(this);

	// --
	//

	QStringList versionTypes;

	versionTypes << tr("Changeset");			// VersionType::Changeset
	versionTypes << tr("Date");					// VersionType::Date
	versionTypes << tr("Latest Version");		// VersionType::LatestVersion

	ui->sourceTypeComboBox->addItems(versionTypes);
	ui->targetTypeComboBox->addItems(versionTypes);

	ui->targetTypeComboBox->setCurrentIndex(static_cast<int>(CompareVersionType::LatestVersion));

	ui->sourceDateEdit->setDateTime(QDateTime::currentDateTime());
	ui->targetDateEdit->setDateTime(QDateTime::currentDateTime());

	versionTypeChanged();

	// --
	//
	connect(ui->sourceTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogProjectDiff::versionTypeChanged);
	connect(ui->targetTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogProjectDiff::versionTypeChanged);

	// Select default file types if they are not selected
	//
	if (m_diffParams.projectFileTypes.empty() == true)
	{
		m_diffParams.projectFileTypes = ProjectDiffGenerator::defaultProjectFileTypes(db);
	}

	// Fill file types list
	//
	for (const ProjectDiffFileType& ft : m_diffParams.projectFileTypes)
	{
		QListWidgetItem* item = new QListWidgetItem(tr("%1").arg(ft.fileName));

		if (ft.selected == true)
		{
			item->setCheckState(Qt::Checked);
		}
		else
		{
			item->setCheckState(Qt::Unchecked);
		}

		ui->categoriesList->addItem(item);
	}

	ui->expertPropertiesCheck->setChecked(m_diffParams.expertProperties == true);

	ui->multipleFilesCheck->setChecked(m_diffParams.multipleFiles == true);

	return;

}

DialogProjectDiff::~DialogProjectDiff()
{
	delete ui;
}

ProjectDiffParams DialogProjectDiff::diffParams() const
{
	return m_diffParams;
}

void DialogProjectDiff::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.25),
		   rect().height());

	move(screen.center() - rect().center());

	return;
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

void DialogProjectDiff::done(int r)
{
	if (r == QDialog::Rejected)
	{
		QDialog::done(r);
		return;
	}

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

	m_diffParams.compareData = compareData;

	int selectedCount = 0;

	if (ui->categoriesList->count() != m_diffParams.projectFileTypes.size())
	{
		Q_ASSERT(false);
		return;
	}

	for (int i = 0; i < ui->categoriesList->count(); i++)
	{
		QListWidgetItem* item = ui->categoriesList->item(i);

		m_diffParams.projectFileTypes[i].selected = item->checkState() == Qt::Checked;
		if (m_diffParams.projectFileTypes[i].selected == true)
		{
			selectedCount++;
		}
	}

	if (selectedCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Please select at least one file category!"));
		return;
	}

	m_diffParams.expertProperties = ui->expertPropertiesCheck->isChecked() == true;

	m_diffParams.multipleFiles = ui->multipleFilesCheck->isChecked() == true;

	QDialog::done(r);
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
