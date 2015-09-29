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
	CheckInDialog(const std::vector<DbFileInfo>& files, bool treeCheckIn, DbController* db, QWidget* parent);
	~CheckInDialog();

public:
	static bool checkIn(std::vector<DbFileInfo>& files, bool treeCheckIn, std::vector<DbFileInfo>* checkedInFiles, DbController* db, QWidget* parent);
	
private slots:
	void on_checkInButton_clicked();

	void on_cancelButton_clicked();

private:
	bool m_treeCheckIn = false;
	Ui::CheckInDialog* ui;
	std::vector<DbFileInfo> m_files;
};

