#ifndef COMPAREDIALOG_H
#define COMPAREDIALOG_H

#include <QDialog>
#include "../../lib/DbStruct.h"

namespace Ui {
	class CompareDialog;
}

class DbController;



class CompareDialog : public QDialog
{
	Q_OBJECT

private:
	CompareDialog(DbController* db, const DbChangesetObject& object, int changeset, QWidget* parent);

public:
	~CompareDialog();

	static void showCompare(DbController* db, const DbChangesetObject& object, int changeset, QWidget* parent);

private slots:
	void versionTypeChanged();
	void on_sourceChangesetButton_clicked();
	void on_targetChangesetButton_clicked();

	virtual void done(int r) override;

private:
	Ui::CompareDialog* ui = nullptr;

	DbController* m_db = nullptr;
	DbChangesetObject m_object;
};

#endif // COMPAREDIALOG_H
