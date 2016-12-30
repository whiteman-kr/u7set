#include "CompareDialog.h"
#include "ui_CompareDialog.h"
#include "../../lib/DbController.h"
#include "SelectChangesetDialog.h"
#include "GlobalMessanger.h"

CompareDialog::CompareDialog(DbController* db, const DbChangesetObject& object, int changeset, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CompareDialog),
	m_db(db),
	m_object(object)
{
	assert(m_db);

	ui->setupUi(this);

	// --
	//
	QString objectName;

	if (object.isFile() == true)
	{
		objectName = QString("File: %1, FileID: %2, Parent: %3")
					 .arg(object.name())
					 .arg(object.id())
					 .arg(object.parent());
	}

	if (object.isSignal() == true)
	{
		objectName = QString("Signal: %1, DBSignalID: %2")
					 .arg(object.caption())
					 .arg(object.id());
	}

	ui->objectEdit->setText(objectName);

	QStringList versionTypes;

	versionTypes << tr("Changeset");			// VersionType::Changeset
	versionTypes << tr("Date");					// VersionType::Date
	versionTypes << tr("Latest Version");		// VersionType::LatestVersion

	ui->sourceTypeComboBox->addItems(versionTypes);
	ui->targetTypeComboBox->addItems(versionTypes);

	if (changeset != -1)
	{
		ui->sourceTypeComboBox->setCurrentIndex(static_cast<int>(CompareVersionType::Changeset));
		ui->sourceChangesetLineEdit->setText(QString::number(changeset));
	}

	ui->targetTypeComboBox->setCurrentIndex(static_cast<int>(CompareVersionType::LatestVersion));

	if (changeset != -1)
	{
		ui->targetChangesetLineEdit->setText(QString::number(changeset));
	}

	ui->sourceDateEdit->setDateTime(QDateTime::currentDateTime());
	ui->targetDateEdit->setDateTime(QDateTime::currentDateTime());

	versionTypeChanged();

	// --
	//
	connect(ui->sourceTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CompareDialog::versionTypeChanged);
	connect(ui->targetTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CompareDialog::versionTypeChanged);

	return;
}

CompareDialog::~CompareDialog()
{
	delete ui;
}

void CompareDialog::showCompare(DbController* db, const DbChangesetObject& object, int changeset, QWidget* parent)
{
	if (db == nullptr)
	{
		assert(db);
		return;
	}

	CompareDialog* dialog = new CompareDialog(db, object, changeset, parent);

	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();

	return;
}

void CompareDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(screen.width() * 0.25, rect().height());

	move(screen.center() - rect().center());

	return;
}

void CompareDialog::versionTypeChanged()
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

void CompareDialog::on_sourceChangesetButton_clicked()
{
	if (m_object.isFile() == true)
	{
		DbFileInfo file;
		file.setFileId(m_object.id());

		int changeset = SelectChangesetDialog::getFileChangeset(m_db, file, this);

		if (changeset != -1)
		{
			ui->sourceChangesetLineEdit->setText(QString::number(changeset));
		}

		return;
	}

	if (m_object.isSignal() == true)
	{
		DbFileInfo file;
		file.setFileId(m_object.id());

		int changeset = SelectChangesetDialog::getSignalChangeset(m_db, m_object, this);

		if (changeset != -1)
		{
			ui->sourceChangesetLineEdit->setText(QString::number(changeset));
		}

		return;
	}

	assert(false);
}

void CompareDialog::on_targetChangesetButton_clicked()
{
	if (m_object.isFile() == true)
	{
		DbFileInfo file;
		file.setFileId(m_object.id());

		int changeset = SelectChangesetDialog::getFileChangeset(m_db, file, this);

		if (changeset != -1)
		{
			ui->targetChangesetLineEdit->setText(QString::number(changeset));
		}
	}
}

void CompareDialog::done(int r)
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

	// Emit signal
	//
	GlobalMessanger::instance()->fireCompareObject(m_object, compareData);

	QDialog::done(r);
}
