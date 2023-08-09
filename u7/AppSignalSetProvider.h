#pragma once

#include "../AppSignalLib/AppSignalParam.h"
#include "../Builder/SignalSet.h"
#include "../DbLib/DbController.h"
#include "AppSignalPropertyManager.h"

class AppSignalSetProvider : public QObject
{
	Q_OBJECT

public:
	AppSignalSetProvider(DbController* dbController, QWidget* parentWidget);
	virtual ~AppSignalSetProvider();

	static AppSignalSetProvider* getInstance();

	const AppSignalSet& signalSet() const;
	AppSignalPropertyManager& signalPropertyManager();

	void setMiddleVisibleSignalIndex(int signalIndex);

	void clearSignals();

	static void trimSignalTextFields(AppSignal& signal);

	int signalCount() { return m_signalSet.count(); }

	AppSignal* getSignal(const QString& appSignalID);
	AppSignal* getSignal(int signalID);

	bool getChannelSignalsID(const AppSignal& signal, SignalIDsSet* channelSignalIDs) const;
	bool getChannelSignalsID(int signalID, int groupID, SignalIDsSet* channelSignalIDs) const;

	int signalIndex(int signalID) const;
	int signalID(int index) const;

	QVector<int> getSameChannelSignals(int index);

	AppSignal* getLoadedSignal(int index);

	AppSignalParam getAppSignalParam(int index);
	AppSignalParam getAppSignalParam(const QString& appSignalId);

	bool isEditableSignal(int index) const { return isEditableSignal(m_signalSet.at(index)); }
	bool isEditableSignal(const AppSignal* signal) const;
	bool isCheckinableSignalForMe(int index) const{ return isCheckinableSignalForMe(m_signalSet.at(index)); }
	bool isCheckinableSignalForMe(const AppSignal* signal) const;

	QString getUserName(int userId) const;

	DbController* dbController() { return m_dbController; }
	const DbController* dbController() const { return m_dbController; }

	bool checkoutSignal(int index, QString* message);
	bool undoSignal(int id);

	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const SignalIDsSet& signalIDs);
	void deleteSignal(int signalID);

	void addSignal(AppSignal& signal);
	void saveSignal(AppSignal& signal);
	void saveSignals(QVector<AppSignal*> signalVector);
	QVector<int> cloneSignals(const SignalIDsSet& signalIDsToClone);

	void initLazyLoadSignals();
	void finishLoadingSignals();
	void stopLoadingSignals();
	void loadNextSignalsPortion();
	void loadUsers();
	void loadSignals();
	void loadSignalSet(QVector<int> keys);
	const AppSignal* loadSignal(int signalId);

	void showError(const ObjectState& state);
	void showErrors(const QVector<ObjectState>& states);

signals:
	void error(const QString& message);						// for throwing message boxes
	void signalCountChanged();								// for reloading entire signal model content
	void signalUpdated(int signalIndex);					// for updating row in signal view (throwing models DataChanged signal)
	void signalPropertiesChanged(const AppSignal& signal);	// for updating property list if new properties exist in signal

private:
	AppSignal* privateGetLoadedSignal(AppSignal* signal);

	QString errorMessage(const ObjectState& state);	// Converts ObjectState to human readable text

private:
	static AppSignalSetProvider* m_instance;
	AppSignalSet m_signalSet;

	DbController* m_dbController = nullptr;
	AppSignalPropertyManager m_propertyManager;
	QTimer* m_lazyLoadSignalsTimer = nullptr;
	int m_middleVisibleSignalIndex = 0;
	std::map<int, QString> m_usernameMap;		// userID => userName
	bool m_partialLoading = false;
};
