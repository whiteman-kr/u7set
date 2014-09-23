#include "CheckInDialog.h"
#include "ui_CheckInDialog.h"

CheckInDialog::CheckInDialog(const std::vector<DbFileInfo>& files, DbController* dbcontroller, QWidget* parent) :
	QDialog(parent),
	HasDbController(dbcontroller),
	ui(new Ui::CheckInDialog),
	m_files(files)
{
	ui->setupUi(this);

	// Fill the file list
	//
	assert(m_files.empty() == false);

	ui->fileListEditBox->setReadOnly(true);

	QString fileListStr;
	for (unsigned int i = 0; i < m_files.size(); i++)
	{
		fileListStr += m_files[i].fileName() + "\n";
	}

	ui->fileListEditBox->setText(fileListStr);

	return;
}

CheckInDialog::~CheckInDialog()
{
	delete ui;
}

bool CheckInDialog::checkIn(std::vector<DbFileInfo>& files, DbController* dbcontroller, QWidget* parent)
{
	CheckInDialog d(files, dbcontroller, parent);
	d.exec();

	if (d.result() == Accepted)
	{
		files.swap(d.m_files);
	}

	return d.result();
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

	bool result = dbcontroller()->checkIn(m_files, comment, this);
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
