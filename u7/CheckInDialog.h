#ifndef CHECKINDIALOG_H
#define CHECKINDIALOG_H

#include <QDialog>
#include "../include/DbStore.h"

namespace Ui {
	class CheckInDialog;
}

class CheckInDialog : protected QDialog, protected HasDbStore
{
	Q_OBJECT
private:
	CheckInDialog();				// deleted
	CheckInDialog(const std::vector<DbFileInfo>& files, DbStore* dbstore, QWidget* parent);
	~CheckInDialog();

public:
	static bool checkIn(const std::vector<DbFileInfo>& files, DbStore* dbstore, QWidget* parent);
	
private slots:
	void on_checkInButton_clicked();

	void on_cancelButton_clicked();

private:
	Ui::CheckInDialog* ui;
	std::vector<DbFileInfo> m_files;
};

#endif // CHECKINDIALOG_H
