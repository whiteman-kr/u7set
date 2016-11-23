#pragma once

#include <QDialog>
#include "../lib/DbController.h"
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
	SelectChangesetDialog(QString title, DbController* db, const std::vector<DbChangeset>& history, bool onlySelectChanget, QWidget* parent);
	~SelectChangesetDialog();

	int changeset() const;

	void setFile(const DbFileInfo& file);
	DbFileInfo file() const;

	static int getChangeset(DbController* db, const DbFileInfo& file, bool onlySelectChangeset, QWidget* parent);
	
private slots:
	void on_buttonBox_accepted();
	void on_changesetList_doubleClicked(const QModelIndex &index);

	void on_changesetList_customContextMenuRequested(const QPoint &pos);

	void changesetDetails(int changeset);
	void compareFile(DbFileInfo& file, int changeset);

	void on_buttonBox_rejected();

private:
	Ui::SelectChangesetDialog* ui;
	DbController* m_db = nullptr;
	std::vector<DbChangeset> m_history;
	int m_changeset = -1;

	DbFileInfo m_file;

	bool m_onlySelectChanget = false;
};

