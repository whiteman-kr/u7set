#pragma once

#include "MonitorConfigController.h"
#include "ArchiveModelView.h"

class MonitorSignalManager;

class MonitorArchiveWidget : public QMainWindow
{
public:
	MonitorArchiveWidget(MonitorSignalManager* signalManager,
						 MonitorConfigController* configController,
						 QWidget* parent);
	virtual ~MonitorArchiveWidget();

public:
	void ensureVisible();

	bool setSignals(const std::vector<AppSignalParam>& appSignals);
	bool setTime(QDateTime startTime, QDateTime endTime, E::TimeType timeType);
	void requestDataOnConnection();

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
	void timeTypeCurrentIndexChanged(int index);

	void exportButton();
	void printButton();
	void updateOrCancelButton();
	void signalsButton();

	void showSignalInfo(QString appSignalId);	// Slot to ArchiveView::requestToShowSignalInfo
	void removeSignal(QString appSignalId);		// Slot to ArchiveView::requestToRemoveSignal

	void slot_configurationArrived(ConfigSettings configuration);

	void tcpConnectionEstablished();
	void dataReceived(std::shared_ptr<ArchiveChunk> chunk);
	void tcpClientError(QString errorMessage);
	void tcpStatus(QString status, int statesReceived, int requestCount, int repliesCount);
	void tcpRequestFinished();

	// Data
	//
private:
	MonitorSignalManager* m_signalManager = nullptr;

	std::vector<MonitorSettings::ArchiveService> m_archiveServices;
	QString m_projectName;
	QString m_softwareId;

//	ArchiveTcpClient* m_tcpClient = nullptr;
//	SimpleThread* m_tcpClientThread = nullptr;


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

	bool m_requestDataOnConnection = false;
};


