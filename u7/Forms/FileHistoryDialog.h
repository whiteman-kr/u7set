#pragma once

#include <QDialog>
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
	FileHistoryDialog(QString title, const std::vector<DbChangeset>& fileHistory, QWidget* parent);
	~FileHistoryDialog();

	static void showHistory(QString fileName, const std::vector<DbChangeset>& fileHistory, QWidget* parent);
	
private slots:
	void on_changesetList_doubleClicked(const QModelIndex &index);
	void on_changesetList_customContextMenuRequested(const QPoint &pos);

	static void changesetDetails(int changeset);

private:
	Ui::FileHistoryDialog *ui;
	std::vector<DbChangeset> m_fileHistory;
};

