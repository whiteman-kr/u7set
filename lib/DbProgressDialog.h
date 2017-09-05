#pragma once

#include <QDialog>
#include <QLabel>
#include <QProgressBar>

class DbProgress;

//
//
// ProgressDialog
//
//

class ProgressDialog : public QDialog
{
public:
    ProgressDialog(QWidget* parent, const QString& description, DbProgress* progress);

	static void showProgressDialog(QWidget* parent, const QString& description, DbProgress* progress);

protected:
    virtual void timerEvent(QTimerEvent*) override;

protected slots:
	virtual void reject() override;

private:
	int m_timerId = 0;
    QString m_description;
	QLabel* m_label = nullptr;
	QProgressBar* m_progressBar = nullptr;
	QPushButton* m_cancelButton = nullptr;
	DbProgress* m_progress = nullptr;
};
