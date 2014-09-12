#pragma once

#include <QDialog>
#include "../include/DbController.h"

namespace Ui {
	class CheckInDialog;
}

class CheckInDialog : protected QDialog, protected HasDbController
{
	Q_OBJECT
private:
	CheckInDialog();				// deleted
	CheckInDialog(const std::vector<DbFileInfo>& files, DbController* dbcontroller, QWidget* parent);
	~CheckInDialog();

public:
	static bool checkIn(const std::vector<DbFileInfo>& files, DbController* dbcontroller, QWidget* parent);
	
private slots:
	void on_checkInButton_clicked();

	void on_cancelButton_clicked();

private:
	Ui::CheckInDialog* ui;
	std::vector<DbFileInfo> m_files;
};

