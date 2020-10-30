#ifndef DIALOGPROJECTDIFFPROGRESS_H
#define DIALOGPROJECTDIFFPROGRESS_H

#include <QDialog>

namespace Ui {
	class DialogProjectDiffProgress;
}

class ProjectDiffWorker;

class DialogProjectDiffProgress : public QDialog
{
	Q_OBJECT

public:
	explicit DialogProjectDiffProgress(std::shared_ptr<ProjectDiffWorker> worker, QWidget *parent = nullptr);
	~DialogProjectDiffProgress();

protected:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void onTimer();

	void on_cancelButton_clicked();

private:
	Ui::DialogProjectDiffProgress *ui;

	std::shared_ptr<ProjectDiffWorker> m_worker = nullptr;
};

#endif // DIALOGPROJECTDIFFPROGRESS_H
