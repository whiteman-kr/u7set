#ifndef TRENDSMAINWINDOW_H
#define TRENDSMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class TrendsMainWindow;
}

class TrendsMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TrendsMainWindow(QWidget *parent = 0);
	~TrendsMainWindow();

	// Methods
	//
protected:
	void saveWindowState();
	void restoreWindowState();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*e) override;
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void showEvent(QShowEvent*) override;

private slots:
	void actionOpenTriggered();
	void actionSaveTriggered();
	void actionExitTriggered();
	void actionAboutTriggered();

private:
	Ui::TrendsMainWindow *ui;
};

#endif // TRENDSMAINWINDOW_H
