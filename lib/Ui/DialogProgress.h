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
	void setProgressSingle(int progress, int progressMin, int progressMax, const QString& status);
	void setProgressMultiple(int progress, int progressMin, int progressMax, const QStringList& status);

protected:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void onTimer();
	void on_cancelButton_clicked();

private:

	int m_progressMin = 0;
	int m_progressMax = 100;

	std::vector<QLabel*> m_labelsStatus;
	QProgressBar* m_progressBar = nullptr;

};

