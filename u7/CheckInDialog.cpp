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

	ui->fileListEditBox->setReadOnly(true);

	QString fileListStr;
	for (size_t i = 0; i < m_files.size(); i++)
	{
		QString fileName = m_files[i].fileName();

		if (fileName.isEmpty() == true)
		{
			fileName = tr("Not defined yet.");
		}

		fileListStr += fileName + "\n";
	}

	ui->fileListEditBox->setText(fileListStr);

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
	resize(screen.width() * 0.30, screen.height() * 0.30);
	move(screen.center() - rect().center());

	return;
}

void CheckInDialog::on_checkInButton_clicked()
{
	QString comment = ui->commentEdit->toPlainText();

	if (comment.isEmpty() == true)
	{
		QMessageBox mb(this);

		mb.setText(tr("Check In comment connot be empty!"));
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
