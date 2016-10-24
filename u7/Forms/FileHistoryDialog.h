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
	FileHistoryDialog(QString title, const std::vector<DbChangesetInfo>& fileHistory, QWidget* parent);
	~FileHistoryDialog();

	static void showHistory(QString fileName, const std::vector<DbChangesetInfo>& fileHistory, QWidget* parent);
	
private slots:
	void on_changesetList_doubleClicked(const QModelIndex &index);

private:
	Ui::FileHistoryDialog *ui;
	std::vector<DbChangesetInfo> m_fileHistory;
};

