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
	SelectChangesetDialog(QString title, const std::vector<DbChangeset>& history, QWidget* parent);
	~SelectChangesetDialog();

	int changeset() const;

	void setFile(const DbFileInfo& file);
	DbFileInfo file() const;

	static int getChangeset(const DbFileInfo& file, const std::vector<DbChangeset>& history, QWidget* parent);
	
private slots:
	void on_buttonBox_accepted();
	void on_changesetList_doubleClicked(const QModelIndex &index);

	void on_changesetList_customContextMenuRequested(const QPoint &pos);

	static void changesetDetails(int changeset);
	static void compareFile(DbFileInfo& file, int changeset);

private:
	Ui::SelectChangesetDialog* ui;
	std::vector<DbChangeset> m_history;
	int m_changeset;

	DbFileInfo m_file;
};

