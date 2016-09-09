#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Stable.h"

#include "TuningWorkspace.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:

	TuningWorkspace* m_tuningWorkspace = nullptr;


};

#endif // MAINWINDOW_H
