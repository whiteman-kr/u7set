#ifndef DIALOGPROJECTDIFF_H
#define DIALOGPROJECTDIFF_H

#include <QDialog>
#include "GlobalMessanger.h"

namespace Ui {
	class DialogProjectDiff;
}

class DbController;

class DialogProjectDiff : public QDialog
{
	Q_OBJECT

public:
	explicit DialogProjectDiff(DbController* db, QWidget *parent = nullptr);
	~DialogProjectDiff();

	CompareData compareDataResult() const;
	std::map<int, QString> fileTypesMap() const;

protected:
	virtual void showEvent(QShowEvent* event) override;

private slots:
	void versionTypeChanged();
	void on_sourceChangesetButton_clicked();
	void on_targetChangesetButton_clicked();

	virtual void done(int r) override;

	void on_buttonSelectAll_clicked();

	void on_buttonSelectNone_clicked();

private:
	DbController* m_db = nullptr;


private:
	Ui::DialogProjectDiff *ui;

	CompareData m_compareDataResult;
	std::map<int, QString> m_fileTypesMapResult;
};

#endif // DIALOGPROJECTDIFF_H
