#ifndef DIALOGPROJECTDIFF_H
#define DIALOGPROJECTDIFF_H

#include <QDialog>

namespace Ui {
	class DialogProjectDiff;
}

class DbController;

class DialogProjectDiff : public QDialog
{
	Q_OBJECT

private:
	explicit DialogProjectDiff(DbController* db, QWidget *parent = nullptr);

public:
	~DialogProjectDiff();
	static void showProjectDiff(DbController* db, QWidget* parent);

protected:
	virtual void showEvent(QShowEvent* event) override;

private slots:
	void versionTypeChanged();
	void on_sourceChangesetButton_clicked();
	void on_targetChangesetButton_clicked();

	virtual void done(int r) override;

private:
	DbController* m_db = nullptr;


private:
	Ui::DialogProjectDiff *ui;
};

#endif // DIALOGPROJECTDIFF_H
