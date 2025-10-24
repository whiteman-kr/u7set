#pragma once

#include "AppSignalPropertyManager.h"

class AppSignalSetProvider : public QObject
{
	Q_OBJECT

public:
	AppSignalSetProvider(DbController* dbController, QWidget* parentWidget);
	virtual ~AppSignalSetProvider();

	static AppSignalSetProvider* getInstance();

	DbController* dbController();

	const AppSignalSet& signalSet() const;
	int signalCount() const;

	AppSignalPropertyManager& signalPropertyManager();

	void onProjectOpened();
	void onProjectClosed();

	bool projectProperty_uppercaseAppSignalID() const;

	int currentUserID() const;
	bool currentUserIsAdmin() const;
	QString getUserName(int userId);
	QString signalCheckedOutByUser(const AppSignal& s);

	void reloadAllSignals();
	void reloadSignals(const std::vector<int>& signalIds, bool updateViews);
	void enforceAllSignalsLoading();
	const AppSignal* loadSignal(int signalId, bool updateViews);

	void setMiddleVisibleSignalIndex(int signalIndex);

	AppSignal* getSignal(const QString& appSignalID);
	const AppSignal* getSignal(const QString& appSignalID) const;

	AppSignal* getSignalByHash(Hash appSignalIDHash);
	const AppSignal* getSignalByHash(Hash appSignalIDHash) const;

	AppSignal* getSignalByID(int signalID);

	AppSignal* getSignalByIndex(int index);
	const AppSignal* getSignalByIndex(int index) const;

	bool signalExists(const QString& appSignalID) const;

	const std::vector<AppSignal*>& signalsVector() const;

	int signalIndex(int signalID) const;
	int signalID(int index) const;

	void getSameChannelSignalsIndexes(int signalIndex, std::vector<int>* sameChannelIndexes);

	AppSignal* getLoadedSignal(AppSignal* s, bool updateViews);
	AppSignal* getLoadedSignalByID(int signalID, bool updateViews);
	AppSignal* getLoadedSignalByIndex(int index, bool updateViews);

	bool isEditableSignal(int index) const;
	bool isEditableSignal(const AppSignal* signal) const;

	bool isCheckinableSignalForMe(int index) const;
	bool isCheckinableSignalForMe(const AppSignal* signal) const;
	bool isCheckinableSignalForMe(const ObjectState& objState) const;

	// Signal set modifications DbController calls
	//
	bool createNewSignals(const AppSignal& signalTemplate, int channelsCount, int signalsCount, std::vector<int>* addedSignalIDs);

	bool addSignals(E::SignalType signalType, std::vector<AppSignal>* newSignals, QWidget* parentWidget = nullptr);

	bool autoAddSignals(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignals,
						std::vector<AppSignal>* addedSignals,
						QWidget* parentWidget = nullptr);

	std::vector<int> cloneSignals(const std::vector<int>& signalIDsToClone);

	bool saveSignal(AppSignal* signal, QWidget* parentWidget);
	bool saveSignals(const std::vector<AppSignal*>& signalsVector, QWidget* parentWidget);

	bool checkoutSignalByIndex(int index, QString* message);
	bool checkoutSignal(const AppSignal* s, QString* message, std::vector<int>* checkedOutIDs = nullptr);
	bool checkoutSignals(const std::vector<AppSignal*>& appSignals, QString* message, std::vector<int>* checkedOutIDs = nullptr);
	bool checkoutSignals(const std::vector<int>& appSignalIDs, QString* message, std::vector<int>* checkedOutIDs = nullptr);

	bool checkinSignals(const std::vector<int>& signalIDs, QString comment);

	bool undoSignalsChanges(const std::vector<int>& signalIDs, QWidget* parentWidget = nullptr);
	bool undoSignal(int id);
	bool undoSignal(const AppSignal& s);

	bool updateSignalsSpecProps(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignalsToUpdate, QString* errMsg);

	void deleteSignals(const std::vector<int>& signalIDs);

	bool getProjectProperties(DbProjectProperties* projectProps) const;
	bool isSafetyProject() const;

signals:
	void error(const QString& message); // for throwing message boxes

	// for reloading entire signal model content or any signal count changes
	//
	void signalsCountChanged();

	// for updating row in signal view (throwing models DataChanged signal)
	// also update known properties in AppSignalPropertyManager
	//
	void signalsUpdated(const std::vector<int>& indexes);

	// only update known properties in AppSignalPropertyManager
	//
	void detectNewProperties(const std::vector<int>& indexes);

private:
	void loadUsers();
	void reloadPropertiesBehaviour();

	void startSignalsLoading();
	void terminateSignalsLoading();

	void loadIdAppSignalId();

	void appendSignalsAndUpdateViews(const std::vector<AppSignal>& newSignals);

	void updateSignalSet(const std::vector<ObjectState>& states);

	void onSignalsLoadTimer();

	void emitSignalsUpdated(const std::vector<int>& indexes);

	//

	QString errorMessage(const ObjectState& state); // Converts ObjectState to human readable text

	// if no errors returns TRUE
	// returns FALSE if errors presents
	//
	bool showError(const ObjectState& state);
	bool showErrors(const std::vector<ObjectState>& states);

private:
	static AppSignalSetProvider* m_instance;
	static QThread* m_thread;

	static const int BAD_INDEX = -1;

	DbController* m_db = nullptr;
	QWidget* m_parentWidget = nullptr;

	int m_currentUserID = -1;
	bool m_currentUserIsAdmin = false;

	std::map<int, QString> m_users; // userID => userName

	AppSignalSet m_signalSet;

	AppSignalPropertyManager m_propertyManager;

	//

	QTimer m_signalsLoadTimer;
	int m_middleVisibleSignalIndex = 0;
	bool m_signalsLoading = false; // true - signals loading in progress
};
