#pragma once

#include "MonitorConfigController.h"
#include "ArchiveModelView.h"
#include "ArchiveConnection.h"


namespace ClientLib
{
	class AppSignalManager;
}

class ArchiveWidget : public QMainWindow
{
	Q_OBJECT
public:
	explicit ArchiveWidget(ClientLib::AppSignalManager& signalManager,
						   MonitorConfigController* configController,
						   const AppSignalLists::AppSignalListSet& appSignalListSet,
						   QWidget* parent);
	virtual ~ArchiveWidget();

public:
	void ensureVisible();

	// Call these functions only from the main thread
	//
	bool setSignals(const std::vector<AppSignalParam>& appSignals);
	bool setSignals(std::vector<ArchiveSignal> archiveSignals);

	bool setTime(QDateTime startTime, QDateTime endTime, E::TimeType timeType);

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

	void updateUiState();		// Update buttons state

	// Slots
	//
protected slots:
	void timeTypeCurrentIndexChanged(int index);

	void exportButton();
	void printButton();
	void updateOrCancelButton();
	void signalsButton();

	void showSignalInfo(QString appSignalId);							// Slot to ArchiveView::requestToShowSignalInfo
	void removeSignal(QString appSignalId, QString archiveServiceId);	// Slot to ArchiveView::requestToRemoveSignal

	void slot_configurationArrived(MonitorConfigSettings configuration);

	void dataReceived(std::shared_ptr<ArchiveRequestResult> chunk);
	void requestError(QString errorMessage);
	void requestStatus(QString serverStatus, int requests, int replies, int states);
	void requestFinished();

private:
	void exportData(bool exportSelected);
	void printData(bool printSelected);

	// Data
	//
private:
	ClientLib::AppSignalManager& m_signalManager;

	// These three updated on slot_configurationArrived
	//
	std::vector<SoftwareEndpoint::ArchiveService> m_archiveServices;
	QString m_projectName;
	QString m_softwareId;

	// Communication
	//
	ArchiveConnection m_archiveConnection;

	// --
	//
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

	ArchiveSource m_source;
	const AppSignalLists::AppSignalListSet& m_appSignalListSet;
};


