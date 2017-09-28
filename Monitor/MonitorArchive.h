#ifndef MONITORARCHIVE_H
#define MONITORARCHIVE_H

#include "MonitorConfigController.h"
#include "DialogChooseArchiveSignals.h"
#include "ArchiveTcpClient.h"
#include "ArchiveModelView.h"

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

protected:
	void requestData();
	void cancelRequest();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent *e) override;
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

protected:
	void saveWindowState();
	void restoreWindowState();

	// Slots
	//
protected slots:
	void signalsButton();

	void dataReceived(std::shared_ptr<ArchiveChunk> chunk);
	void tcpClientError(QString errorMessage);
	void tcpStatus(QString status, int statesReceived, int requestCount, int repliesCount);

	// Data
	//
private:
	ConfigConnection m_archiveService1;
	ConfigConnection m_archiveService2;

	std::vector<VFrame30::SchemaDetails> m_schemasDetais;

	ArchiveTcpClient* m_tcpClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;

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

	ArchiveModel* m_model = new ArchiveModel(this);
	ArchiveView* m_view = new ArchiveView(this);

	QStatusBar* m_statusBar = nullptr;
	QLabel* m_statusBarTextLabel = nullptr;
	QLabel* m_statusBarStatesReceivedLabel = nullptr;
	QLabel* m_statusBarNetworkRequestsLabel = nullptr;
	QLabel* m_statusBarServerLabel = nullptr;
	QLabel* m_statusBarConnectionStateLabel = nullptr;

	ArchiveSource m_source;
};


#endif // MONITORARCHIVE_H
