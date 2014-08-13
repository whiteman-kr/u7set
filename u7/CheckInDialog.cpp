#include "CheckInDialog.h"
#include "ui_CheckInDialog.h"

CheckInDialog::CheckInDialog(const std::vector<DbFileInfo>& files, DbStore* dbstore, QWidget* parent) :
	QDialog(parent),
	HasDbStore(dbstore),
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

bool CheckInDialog::checkIn(const std::vector<DbFileInfo>& files, DbStore* dbstore, QWidget* parent)
{
	CheckInDialog d(files, dbstore, parent);
	d.exec();

	return d.result() == Accepted;
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

	bool result = dbstore()->checkInFiles(m_files, comment, this);
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
