#pragma once

#include <QDialog>
#include "../lib/DbController.h"
#include "../lib/DbStruct.h"

namespace Ui {
	class FileHistoryDialog;
}

class FileHistoryDialog : public QDialog
{
	Q_OBJECT
	
private:
	FileHistoryDialog();
public:
	FileHistoryDialog(QString title, DbController* db, const std::vector<DbChangeset>& fileHistory, QWidget* parent);
	~FileHistoryDialog();

	static void showHistory(DbController* db, QString objectName, const std::vector<DbChangeset>& fileHistory, QWidget* parent);
	
private slots:
	void on_changesetList_doubleClicked(const QModelIndex &index);
	void on_changesetList_customContextMenuRequested(const QPoint &pos);

	void changesetDetails(int changeset);

private:
	Ui::FileHistoryDialog *ui;
	std::vector<DbChangeset> m_fileHistory;
	DbController* m_db = nullptr;
};

