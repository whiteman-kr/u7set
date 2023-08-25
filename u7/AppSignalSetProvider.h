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

	void projectOpened();
	void projectClosed();

	const AppSignalSet& signalSet() const;					// may be delete!
	AppSignalPropertyManager& signalPropertyManager();

	int signalCount() { return m_signalSet.count(); }

	void reloadAllSignals();

	void loadSignals(const std::vector<int>& signalIds, bool withoutProgress = true);
	void reloadSignals(const std::vector<int>& signalIds);

	void enforceAllSignalsLoading();
	const AppSignal* loadSignal(int signalId, bool updateViews);

	void setMiddleVisibleSignalIndex(int signalIndex);

	QString getUserName(int userId);

	AppSignal* getSignal(const QString& appSignalID);
	AppSignal* getSignalByID(int signalID);
	AppSignal* getSignal(int index);

	bool getChannelSignalsID(const AppSignal& signal, std::vector<int>* channelSignalIDs) const;
	bool getChannelSignalsID(int signalID, int groupID, std::vector<int>* channelSignalIDs) const;

	int signalIndex(int signalID) const;
	int signalID(int index) const;

	QVector<int> getSameChannelSignals(int index);

	AppSignal* getLoadedSignal(AppSignal* s, bool updateViews);
	AppSignal* getLoadedSignalByID(int signalID, bool updateViews);
	AppSignal* getLoadedSignal(int index, bool updateViews);

	AppSignalParam getAppSignalParam(int index);
	AppSignalParam getAppSignalParam(const QString& appSignalId);

	bool isEditableSignal(int index) const;
	bool isEditableSignal(const AppSignal* signal) const;

	bool isCheckinableSignalForMe(int index) const;
	bool isCheckinableSignalForMe(const AppSignal* signal) const;

	bool checkoutSignal(int index, QString* message);
	bool undoSignal(int id);

	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const std::vector<int>& signalIDs);
	void deleteSignal(int signalID);

	void addSignal(AppSignal& signal);
	void saveSignal(AppSignal& signal);
	void saveSignals(const std::vector<AppSignal*>& signalVector);
	std::vector<int> cloneSignals(const std::vector<int>& signalIDsToClone);

	void showError(const ObjectState& state);
	void showErrors(const std::vector<ObjectState>& states);

	static void trimSignalTextFields(AppSignal& signal);

	// DbController calls

	bool checkinSignals(const std::vector<int>& signalIDs,
						QString comment,
						std::vector<ObjectState>* objectStates);

	bool undoSignalChanges(int signalID, ObjectState* objectStates);

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

	void onSignalsLoadTimer();

	QString errorMessage(const ObjectState& state);	// Converts ObjectState to human readable text

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

