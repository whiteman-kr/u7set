#pragma once

#include <QMainWindow>
#include <QWidget>

class RvduWidget;
class RvduSimThreadController;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QString buildFolder, int subsystemKey, QWidget* parent = nullptr);
	~MainWindow();


private slots:
	void onTimer();
	void onSignals();
	void onShowAbout();

private:
	bool parseCommandLine();
	void setupUi();
	
	void createMenu();


private:
	QString m_buildFolder;
	int m_subsystemKey = 0;
	int m_width = 1280;
	int m_height = 720;

	QTimer* m_timer = nullptr;

	unsigned int* m_pHdmiBuffer = nullptr;

	RvduWidget* m_rvduWidget = nullptr;
	RvduSimThreadController* m_controller = nullptr;
};
