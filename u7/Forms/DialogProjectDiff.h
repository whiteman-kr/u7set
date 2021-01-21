#ifndef DIALOGPROJECTDIFF_H
#define DIALOGPROJECTDIFF_H

#include <QDialog>
#include "GlobalMessanger.h"
#include "Reports/ProjectDiffGenerator.h"

namespace Ui {
	class DialogProjectDiff;
}

class DbController;

class DialogProjectDiffSections : public QDialog
{
	Q_OBJECT

public:
	explicit DialogProjectDiffSections(const ProjectDiffReportParams& reportParams, DbController* db, QWidget *parent);

	ProjectDiffReportParams reportParams() const;

private slots:
	void pageSetup();
	void setToDefault();

private:
	void fillTree();

private:
	QTreeWidget* m_treeWidget = nullptr;
	ProjectDiffReportParams m_reportParams;
	DbController* m_db = nullptr;
};

class DialogProjectDiff : public QDialog
{
	Q_OBJECT

public:
	explicit DialogProjectDiff(DbController* db, QWidget *parent = nullptr);
	~DialogProjectDiff();

	QString fileName() const;
	ProjectDiffReportParams reportParams() const;

protected:
	virtual void showEvent(QShowEvent* event) override;

private slots:
	void versionTypeChanged();

	virtual void done(int r) override;

	void on_sourceChangesetButton_clicked();
	void on_targetChangesetButton_clicked();
	void on_buttonSelectAll_clicked();
	void on_buttonSelectNone_clicked();
	void on_categoriesList_itemPressed(QListWidgetItem *item);
	void on_fileBrowseButton_clicked();
	void on_pageSetupButton_clicked();
	void on_pageSetDefault_clicked();
	void on_multiFilepageSetupButton_clicked();

private:
	void updatePageSizeInfo();

private:
	DbController* m_db = nullptr;
	Ui::DialogProjectDiff *ui;
	QString m_fileName;
	static ProjectDiffReportParams m_reportParams;

};

#endif // DIALOGPROJECTDIFF_H
