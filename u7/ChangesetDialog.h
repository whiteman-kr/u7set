#pragma once

#include <QDialog>
#include "../include/DbStruct.h"

namespace Ui {
	class ChangesetDialog;
}

class ChangesetDialog : public QDialog
{
	Q_OBJECT
	
private:
	ChangesetDialog();
public:
	ChangesetDialog(const std::vector<DbChangesetInfo>& fileHistory, QWidget* parent);
	~ChangesetDialog();

	int changeset() const;			// Result

	static int getChangeset(const std::vector<DbChangesetInfo>& fileHistory, QWidget* parent);
	
private slots:
	void on_buttonBox_accepted();

	void on_changesetList_doubleClicked(const QModelIndex &index);

private:
	Ui::ChangesetDialog *ui;
	std::vector<DbChangesetInfo> m_fileHistory;
	int m_changeset;
};

