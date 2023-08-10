#pragma once

#include "../AppSignalLib/AppSignalParam.h"
#include "../Builder/SignalSet.h"
#include "AppSignalPropertyManager.h"

class AppSignalSetProvider : public QObject
{
	Q_OBJECT

public:
	AppSignalSetProvider(DbController* dbController, QWidget* parentWidget);
	virtual ~AppSignalSetProvider();

	static AppSignalSetProvider* getInstance();
	AppSignalPropertyManager& signalPropertyManager() { return m_propertyManager; }

	void setMiddleVisibleSignalIndex(int signalIndex);

	void clearSignals();

	const AppSignalSet& signalSet() const	{ return m_signalSet; }
	static void trimSignalTextFields(AppSignal& signal);

	int signalCount() { return static_cast<int>(m_signalSet.count()); }
	AppSignal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	AppSignal* getSignalByStrID(const QString& signalStrID);
	QVector<int> getChannelSignalsID(int signalGroupID) { return m_signalSet.getChannelSignalsID(signalGroupID); }
	int key(int index) const { return m_signalSet.key(index); }
	int keyIndex(int key) { return static_cast<int>(m_signalSet.keyIndex(key)); }
	QVector<int> getSameChannelSignals(int index);

	const AppSignal& getLoadedSignal(int index);

	AppSignalParam getAppSignalParam(int index);
	AppSignalParam getAppSignalParam(const QString& appSignalId);

	bool isEditableSignal(int index) const { return isEditableSignal(m_signalSet[index]); }
	bool isEditableSignal(const AppSignal& signal) const;
	bool isCheckinableSignalForMe(int index) const{ return isCheckinableSignalForMe(m_signalSet[index]); }
	bool isCheckinableSignalForMe(const AppSignal& signal) const;

	QString getUserStr(int userId) const;

	DbController* dbController() { return m_dbController; }
	const DbController* dbController() const { return m_dbController; }

	bool checkoutSignal(int index);
	bool checkoutSignal(int index, QString& message);
	bool undoSignal(int id);

	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const QSet<int>& signalIDs);
	void deleteSignal(int signalID);

	void addSignal(AppSignal& signal);
	void saveSignal(AppSignal& signal);
	void saveSignals(QVector<AppSignal*> signalVector);
	QVector<int> cloneSignals(const QSet<int>& signalIDs);

	void showError(const ObjectState& state);
	void showErrors(const QVector<ObjectState>& states);

signals:
	void error(const QString& message);						// for throwing message boxes
	void signalCountChanged();								// for reloading entire signal model content
	void signalUpdated(int signalIndex);					// for updating row in signal view (throwing models DataChanged signal)
	void signalPropertiesChanged(const AppSignal& signal);	// for updating property list if new properties exist in signal

public slots:
	void initLazyLoadSignals();
	void finishLoadingSignals();
	void stopLoadingSignals();
	void loadNextSignalsPortion();
	void loadUsers();
	void loadSignals();
	void loadSignalSet(QVector<int> keys);
	const AppSignal* loadSignal(int signalId);

private:
	QString errorMessage(const ObjectState& state);	// Converts ObjectState to human readable text

	static AppSignalSetProvider* m_instance;

	DbController* m_dbController = nullptr;
	AppSignalPropertyManager m_propertyManager;
	QTimer* m_lazyLoadSignalsTimer = nullptr;
	int m_middleVisibleSignalIndex = 0;
	AppSignalSet m_signalSet;
	QMap<int, QString> m_usernameMap;
	bool m_partialLoading = false;
};
