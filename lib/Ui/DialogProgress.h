#pragma once

#include <QDialog>

class DialogProgress : public QDialog
{
	Q_OBJECT

public:
	explicit DialogProgress(const QString& caption, int statusLinesCount, QWidget *parent);
	~DialogProgress();

signals:
	void getProgress();
	void cancelClicked();

public slots:
	//These slots are thread-safe
	//
	void setProgressSingle(int progress, int progressMin, int progressMax, const QString& status);
	void setProgressMultiple(int progress, int progressMin, int progressMax, const QStringList& status);
	void setErrorMessage(const QString& message);	// Displays an error message and exits the dialog

protected:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void onTimer();
	void onCancelClicked();

private:
	std::vector<QLabel*> m_labelsStatus;
	QProgressBar* m_progressBar = nullptr;

	QTimer* m_timer = nullptr;

	QMutex m_mutex;

	// Data protected by m_mutex
	//
	int m_progressMin = 0;
	int m_progressMax = 100;
	int m_progressValue = 0;
	QStringList m_status;
	QString m_errorMessage;

};

