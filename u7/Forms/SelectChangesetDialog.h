#pragma once

#include <QDialog>
#include "../lib/DbStruct.h"

namespace Ui {
	class SelectChangesetDialog;
}

class SelectChangesetDialog : public QDialog
{
	Q_OBJECT
	
private:
	SelectChangesetDialog();
public:
	SelectChangesetDialog(QString title, const std::vector<DbChangeset>& fileHistory, QWidget* parent);
	~SelectChangesetDialog();

	int changeset() const;			// Result

	static int getChangeset(QString fileName, const std::vector<DbChangeset>& fileHistory, QWidget* parent);
	
private slots:
	void on_buttonBox_accepted();
	void on_changesetList_doubleClicked(const QModelIndex &index);

private:
	Ui::SelectChangesetDialog *ui;
	std::vector<DbChangeset> m_fileHistory;
	int m_changeset;
};

