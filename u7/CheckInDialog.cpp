#include "CheckInDialog.h"
#include "ui_CheckInDialog.h"

CheckInDialog::CheckInDialog(const std::vector<DbFileInfo>& files, bool treeCheckIn, DbController* dbcontroller, QWidget* parent) :
	QDialog(parent),
	HasDbController(dbcontroller),
	m_treeCheckIn(treeCheckIn),
	ui(new Ui::CheckInDialog),
	m_files(files)
{
	ui->setupUi(this);

	// Fill the file list
	//
	assert(m_files.empty() == false);

	std::sort(m_files.begin(), m_files.end(), [](const DbFileInfo& f1, const DbFileInfo& f2) {	return f1.fileId() < f2.fileId();});

	ui->fileListEditBox->setReadOnly(true);

	QStringList fileList;
	fileList.reserve(static_cast<int>(files.size()));

	for (const DbFileInfo& file : m_files)
	{
		QString fileNameRecord = file.fileName();
		if (fileNameRecord.isEmpty() == true)
		{
			fileNameRecord = tr("Not defined yet.");
		}

		QJsonDocument jsdoc = QJsonDocument::fromJson(file.details().toUtf8());
		QJsonObject jsobject = jsdoc.object();

		if (jsobject.isEmpty() == false && jsobject.contains("SchemaID"))
		{
			fileNameRecord.append(QString("  |  SchemaID: %1").arg(jsobject.value("SchemaID").toString()));
		}

		if (jsobject.isEmpty() == false && jsobject.contains("EquipmentID"))
		{
			fileNameRecord.append(QString("  |  EquipmentID: %1").arg(jsobject.value("EquipmentID").toString()));
		}

		if (jsobject.isEmpty() == false && jsobject.contains("Place"))
		{
			fileNameRecord.append(QString("  |  Place: %1").arg(jsobject.value("Place").toInt()));
		}

		fileList.push_back(fileNameRecord);
	}

	ui->fileListEditBox->setText(fileList.join('\n'));

	return;
}

CheckInDialog::~CheckInDialog()
{
	delete ui;
}

bool CheckInDialog::checkIn(std::vector<DbFileInfo>& files, bool treeCheckIn, std::vector<DbFileInfo>* checkedInFiles, DbController* dbcontroller, QWidget* parent)
{
	if (files.empty() == true)
	{
		return false;
	}

	if (checkedInFiles == nullptr)
	{
		assert(checkedInFiles != nullptr);
		return false;
	}

	CheckInDialog d(files, treeCheckIn, dbcontroller, parent);
	d.exec();

	checkedInFiles->clear();

	if (d.result() == Accepted)
	{
		checkedInFiles->swap(d.m_files);
	}

	return d.result();
}

void CheckInDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.30),
		   static_cast<int>(screen.height() * 0.30));

	move(screen.center() - rect().center());

	return;
}

void CheckInDialog::on_checkInButton_clicked()
{
	QString comment = ui->commentEdit->toPlainText();

	if (comment.isEmpty() == true)
	{
		QMessageBox mb(this);

		mb.setText(tr("Check In comment cannot be empty!"));
		mb.exec();

		ui->commentEdit->setFocus();

		return;
	}

	bool result = false;

	if (m_treeCheckIn == true)
	{
		std::vector<DbFileInfo> checkedInFiles;

		result = db()->checkInTree(m_files, &checkedInFiles, comment, this);

		checkedInFiles.swap(m_files);
	}
	else
	{
		result = db()->checkIn(m_files, comment, this);
	}

	if (result == true)
	{
		accept();
	}
	else
	{
		reject();
	}

	return;
}

void CheckInDialog::on_cancelButton_clicked()
{
	reject();
}
