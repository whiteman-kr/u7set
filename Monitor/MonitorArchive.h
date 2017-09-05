#ifndef MONITORARCHIVE_H
#define MONITORARCHIVE_H

#include "MonitorConfigController.h"

class MonitorArchiveWidget;

class MonitorArchive
{
public:
	static std::vector<QString> getArchiveList();
	static bool activateWindow(QString archiveName);
	static bool startNewWidget(MonitorConfigController* configController, const std::vector<AppSignalParam>& appSignals, QWidget* parent);

	static void registerWindow(QString name, MonitorArchiveWidget* window);
	static void unregisterWindow(QString name);

private:
	static std::map<QString, MonitorArchiveWidget*> m_archiveList;
};


class MonitorArchiveWidget : public QMainWindow
{
public:
	MonitorArchiveWidget(MonitorConfigController* configController, QWidget* parent);
	virtual ~MonitorArchiveWidget();

public:
	void ensureVisible();

	bool setSignals(const std::vector<AppSignalParam>& appSignals);
	bool addSignal(const AppSignalParam& appSignal);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*e) override;
//	virtual void timerEvent(QTimerEvent* event) override;
//	virtual void showEvent(QShowEvent*) override;

//	virtual void dragEnterEvent(QDragEnterEvent* event) override;
//	virtual void dropEvent(QDropEvent* event) override;

protected:
	void saveWindowState();
	void restoreWindowState();

//	void signalsButton();


//	// Slots
//	//
//protected slots:
//	//void slot_dataReceived(QString appSignalId, TimeStamp requestedHour, TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);

	// Data
	//
private:
	ConfigConnection m_archiveService1;
	ConfigConnection m_archiveService2;

	std::vector<AppSignalParam> m_appSignals;

//	ArchiveTcpClient* m_tcpClient = nullptr;
//	std::unique_ptr<SimpleThread> m_tcpClientThread;

	enum  StatusBarColumns
	{
		SB_Text,
		SB_QueueSize,
		SB_NetworkRequests,
		SB_NetworkRellies,
	};

	QToolBar* m_toolBar = nullptr;
	QPushButton* m_exportButton = nullptr;
	QPushButton* m_printButton = nullptr;
	QPushButton* m_signalsButton = nullptr;

	QStatusBar* m_statusBar = nullptr;
	QLabel* m_statusBarTextLabel = nullptr;
	QLabel* m_statusBarQueueSizeLabel = nullptr;
	QLabel* m_statusBarNetworkRequestsLabel = nullptr;
	QLabel* m_statusBarServerLabel = nullptr;
	QLabel* m_statusBarConnectionStateLabel = nullptr;
};


#endif // MONITORARCHIVE_H
