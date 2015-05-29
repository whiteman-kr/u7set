#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class MainWindow;
}

class MonitorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MonitorMainWindow(QWidget *parent = 0);
	~MonitorMainWindow();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
