#pragma once

#include "../../AppSignalLib/AppSignal.h"

#include "BuildOption.h"

// ==============================================================================================

namespace PS
{
	class Signal : public ::AppSignal
	{

	public:

		Signal();
		Signal(const PS::Signal& from);
		virtual ~Signal() override;

	public:

		void clear();

		void calcOffset();

		int offset() const { return m_offset; }
		int frameIndex() const { return m_frameIndex; }
		int frameOffset() const { return m_frameOffset; }

		QString signalTypeStr() const;
		QString signalInOutTypeStr() const;
		QString engineeringRangeStr() const;
		QString signalFormatStr() const;
		QString stateOffsetStr() const;
		QString stateBitStr() const;

		QString stateStr() const;
		double state() const;
		bool setState(double state);

		quint8* valueData() { return m_pValueData; }
		void setValueData(quint8* pData) { m_pValueData = pData; }

		Signal& operator=(const Signal& from);

	private:

		mutable QMutex m_signalMutex;

		int m_offset = -1;
		int m_frameIndex = -1;
		int m_frameOffset = -1;

		quint8* m_pValueData = nullptr;
	};
}

// ==============================================================================================

class SignalBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalBase(QObject *parent = nullptr);
	virtual ~SignalBase() override;

public:

	void clear();
	int count() const;

	int append(const PS::Signal& signal);

	PS::Signal* signalPtr(const QString& appSignalID) const;
	PS::Signal* signalPtr(const Hash& hash) const;
	PS::Signal* signalPtr(int index) const;

	PS::Signal signal(const Hash& hash) const;
	PS::Signal signal(int index) const;

	void setSignal(int index, const PS::Signal& signal);

	//
	//
	void saveSignalState(PS::Signal* pSignal);
	void restoreSignalsState();

	SignalBase& operator=(const SignalBase& from);

private:

	mutable QMutex m_signalMutex;

	std::vector<PS::Signal> m_signalList;
	QMap<Hash, int> m_signalHashMap;

	// for save and restore state after reload bases, maybe reconnect
	//
	struct SignalState
	{
		QString appSignalID;
		double state = 0;
	};

	std::vector<SignalState> m_signalStateList;
};

// ==============================================================================================
