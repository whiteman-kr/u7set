#pragma once
#include "SignalSet.h"

class DbController;

class SignalSetProvider : public QObject
{
	Q_OBJECT

public:
	SignalSetProvider(DbController* dbController, QWidget* parentWidget);
	static SignalSetProvider* getInstance();

	void setMiddleVisibleSignalIndex(int signalIndex);

	void clearSignals();

	const SignalSet& signalSet() const	{ return m_signalSet; }
	static void trimSignalTextFields(Signal& signal);

	int signalCount() { return m_signalSet.count(); }
	Signal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	Signal* getSignalByStrID(const QString signalStrID);
	QVector<int> getChannelSignalsID(int signalGroupID) { return m_signalSet.getChannelSignalsID(signalGroupID); }
	int key(int row) const { return m_signalSet.key(row); }
	int keyIndex(int key) { return m_signalSet.keyIndex(key); }
	const Signal& signal(int row) const { return m_signalSet[row]; }
	QVector<int> getSameChannelSignals(int row);
	bool isEditableSignal(int index) const { return isEditableSignal(m_signalSet[index]); }
	bool isEditableSignal(const Signal& signal) const;
	QString getUserStr(int userId) const;

	DbController* dbController() { return m_dbController; }
	const DbController* dbController() const { return m_dbController; }

	bool checkoutSignal(int index);
	bool checkoutSignal(int index, QString& message);
	bool undoSignal(int id);

	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const QSet<int>& signalIDs);
	void deleteSignal(int signalID);

	void addSignal(Signal& signal);
	void saveSignal(Signal& signal);
	QVector<int> cloneSignals(const QSet<int>& signalIDs);

	void showError(const ObjectState& state);
	void showErrors(const QVector<ObjectState>& states);

signals:
	void error(const QString& message) const;	// for throwing message boxes
	void usersLoaded() const;
	void signalCountChanged() const;	// for reloading entire signal model content
	void signalUpdated(int signalIndex) const;	// for updating row in signal view (throwing models DataChanged signal)
	void signalPropertiesChanged(const Signal& signal) const; // for updating property list if new properties exist in signal

public slots:
	void initLazyLoadSignals();
	void finishLoadSignals();
	void loadNextSignalsPortion();
	void loadUsers();
	void loadSignals();
	void loadSignalSet(QVector<int> keys);
	void loadSignal(int signalId);

private:
	QString errorMessage(const ObjectState& state);	// Converts ObjectState to human readable text

	static SignalSetProvider* m_instance;

	DbController* m_dbController;
	QTimer* m_lazyLoadSignalsTimer;
	int m_middleVisibleSignalIndex = 0;
	QWidget* m_parentWidget;	//used by DbController
	SignalSet m_signalSet;
	QMap<int, QString> m_usernameMap;
	bool m_partialLoading;
};
