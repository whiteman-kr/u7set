#pragma once

#include "../AppSignalLib/AppSignalParam.h"
#include "../Builder/SignalSet.h"
#include "../DbLib/DbController.h"
#include "../UtilsLib/SimpleThread.h"

#include "AppSignalPropertyManager.h"

class AppSignalSetProvider : public QObject
{
	Q_OBJECT

public:
	AppSignalSetProvider(DbController* dbController, QWidget* parentWidget);
	virtual ~AppSignalSetProvider();

	static AppSignalSetProvider* getInstance();

	DbController* dbController();

	void projectOpened();
	void projectClosed();

	bool projectProperty_uppercaseAppSignalID() const;

	int currentUserID() const;
	bool currentUserIsAdmin() const;

	const AppSignalSet& signalSet() const;					// may be delete!
	AppSignalPropertyManager& signalPropertyManager();

	int signalCount() { return m_signalSet.count(); }

	void reloadAllSignals();
	void reloadSignals(const std::vector<int>& signalIds);

	void enforceAllSignalsLoading();
	const AppSignal* loadSignal(int signalId, bool updateViews);

	void setMiddleVisibleSignalIndex(int signalIndex);

	QString getUserName(int userId);

	AppSignal* getSignal(const QString& appSignalID);
	AppSignal* getSignalByID(int signalID);
	AppSignal* getSignalByIndex(int index);

	bool getChannelSignalsID(const AppSignal& signal, std::vector<int>* channelSignalIDs) const;
	bool getChannelSignalsID(int signalID, int groupID, std::vector<int>* channelSignalIDs) const;

	int signalIndex(int signalID) const;
	int signalID(int index) const;

	QVector<int> getSameChannelSignals(int index);

	AppSignal* getLoadedSignal(AppSignal* s, bool updateViews);
	AppSignal* getLoadedSignalByID(int signalID, bool updateViews);
	AppSignal* getLoadedSignalByIndex(int index, bool updateViews);

	AppSignalParam getAppSignalParam(int index);
	AppSignalParam getAppSignalParam(const QString& appSignalId);

	bool isEditableSignal(int index) const;
	bool isEditableSignal(const AppSignal* signal) const;

	bool isCheckinableSignalForMe(int index) const;
	bool isCheckinableSignalForMe(const AppSignal* signal) const;

	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const std::vector<int>& signalIDs);
	void deleteSignal(int signalID);

	void saveSignal(AppSignal& signal);
	void saveSignals(const std::vector<AppSignal*>& signalVector);
	std::vector<int> cloneSignals(const std::vector<int>& signalIDsToClone);

	QString errorMessage(const ObjectState& state);	// Converts ObjectState to human readable text

	// if no errors returns TRUE
	// returns FALSE if errors presents
	//
	bool showError(const ObjectState& state);
	bool showErrors(const std::vector<ObjectState>& states);

	// DbController calls

	bool checkoutSignalByIndex(int index, QString* message);
	bool checkoutSignal(const AppSignal* s, QString* message);

	bool checkinSignals(const std::vector<int>& signalIDs,
						QString comment);

	bool undoSignalsChanges(const std::vector<int>& signalIDs);
	bool undoSignal(int id);
	bool undoSignal(const AppSignal& s);

	bool getSignalHistory(int signalID, std::vector<DbChangeset>* changesets);

	bool getSpecificSignals(const std::vector<int>& signalIDs,
							int changesetId,
							std::vector<AppSignal>* signalsInstances);

	int getNextSignalCounter();

	bool updateSignalsSpecProps(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignalsToUpdate,
								QString* errMsg);

	bool createNewSignals(const AppSignal& signalTemplate,
						  int channelsCount, int signalsCount,
						  std::vector<int>* addedSignalIDs);


signals:
	void error(const QString& message);						// for throwing message boxes

	// for reloading entire signal model content or any signal count changes
	//
	void signalsCountChanged();

	// for updating row in signal view (throwing models DataChanged signal)
	//
	void signalsUpdated(const std::vector<int>& indexes);

	// for updating property list if new properties exist in signal
	//
	void signalsPropertiesChanged(const std::vector<const AppSignal*>& signalsArray);

private:
	void loadUsers();

	void startSignalsLoading();
	void terminateSignalsLoading();

	void loadIdAppSignalId();
	void loadSignals(const std::vector<int>& signalIds);

	void onSignalsLoadTimer();

private:
	static AppSignalSetProvider* m_instance;
	static QThread* m_thread;

	static const int BAD_INDEX = -1;

	DbController* m_db = nullptr;
	QWidget* m_parentWidget = nullptr;

	int m_currentUserID = -1;
	bool m_currentUserIsAdmin = false;

	std::map<int, QString> m_users;				// userID => userName

	AppSignalSet m_signalSet;

	AppSignalPropertyManager m_propertyManager;

	//

	QTimer m_signalsLoadTimer;
	int m_middleVisibleSignalIndex = 0;
	bool m_signalsLoading = false;				// true - signals loading in progress
};

