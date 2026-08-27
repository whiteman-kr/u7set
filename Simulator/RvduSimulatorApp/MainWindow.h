#pragma once

#include "../../UtilsLib/LogFile.h"
#include <QMainWindow>
#include <QWidget>

class RvduWidget;
class RvduSimThreadController;
class SimCommThreadController;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QString buildFolder, int subsystemKey, QString sharedMemoryKey, QWidget* parent = nullptr);
	~MainWindow();

signals:
	void aboutToClose();

private slots:
	void onTimer();
	void onSignals();
	void onShowSharedMemory();
	void onShowAppLog();
	void onShowAbout();

private:
	void setupUi();
	void setupRvduController();
	void setupSharedMemController();
	void createMenu();
	void createStatusBar();

	void updateStatusBar();

	bool eventFilter(QObject* object, QEvent* event) override;

	static void writeLogFileMessage(int code, const char* message);

private:
	QString m_buildFolder;
	int m_subsystemKey = 0;
	QString m_sharedMemoryKey;
	int m_width = 1280;
	int m_height = 720;

	Log::LogFile m_logFile;


	QTimer* m_timer = nullptr;

	unsigned int* m_pHdmiBuffer = nullptr;

	RvduWidget* m_rvduWidget = nullptr;
	RvduSimThreadController* m_rvduController = nullptr;
	SimCommThreadController* m_sharedMemController = NULL;

	QLabel* m_statusBarProjectInfo = nullptr;
	QLabel* m_statusBarSharedDataInfo = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;
};
