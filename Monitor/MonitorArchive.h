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

	bool saveArchiveWithDocWriter(QString fileName, QString format);
	bool saveArchiveToCsv(QString fileName);

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
	void timeTypeCurrentIndexChanged(int index);

	void exportButton();
	void printButton();
	void updateOrCancelButton();
	void signalsButton();

	void slot_configurationArrived(ConfigSettings configuration);

	void dataReceived(std::shared_ptr<ArchiveChunk> chunk);
	void tcpClientError(QString errorMessage);
	void tcpStatus(QString status, int statesReceived, int requestCount, int repliesCount);
	void tcpRequestFinished();

	// Data
	//
private:
	ConfigConnection m_archiveService1;
	ConfigConnection m_archiveService2;

	std::vector<VFrame30::SchemaDetails> m_schemasDetais;

	ConfigSettings m_configuration;

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
	QPushButton* m_updateButton = nullptr;
	QPushButton* m_signalsButton = nullptr;

	QDateTimeEdit* m_startDateTimeEdit = nullptr;
	QDateTimeEdit* m_endDateTimeEdit = nullptr;
	QComboBox* m_timeType = nullptr;

	ArchiveModel* m_model = new ArchiveModel(this);
	ArchiveView* m_view = new ArchiveView(this);

	QStatusBar* m_statusBar = nullptr;
	QLabel* m_statusBarTextLabel = nullptr;
	QLabel* m_statusBarStatesReceivedLabel = nullptr;
	QLabel* m_statusBarNetworkRequestsLabel = nullptr;
	QLabel* m_statusBarServerLabel = nullptr;
	QLabel* m_statusBarConnectionStateLabel = nullptr;

	ArchiveSource m_source;

	const int m_maxReportStates = 10000;
	const int m_maxReportStatesForCsv = 5000000;
};


#endif // MONITORARCHIVE_H
