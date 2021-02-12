#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/MetrologySignal.h"
#include "../lib/MetrologyConnection.h"

#include "CalibratorManager.h"
#include "RackBase.h"
#include "TuningSignalBase.h"
#include "StatisticsBase.h"

// ==============================================================================================
// IoSignalParam for :
//						Conversion
//						SignalInfoPanel
//						ComparatorInfoPanel
//						MeasureThread
//						MeasureBase
//
// ----------------------------------------------------------------------------------------------

#define						MULTI_TEXT_DEVIDER	"\n"

// ----------------------------------------------------------------------------------------------

class IoSignalParam
{
public:

	IoSignalParam();
	IoSignalParam(const IoSignalParam& from);
	virtual~IoSignalParam() {}

public:

	bool isValid() const;
	void clear();

	Metrology::SignalParam param(int ioType) const;
	bool setParam(int ioType, const Metrology::SignalParam& param);

	Metrology::ConnectionType connectionType() const { return m_connectionType; }
	void setConnectionType(Metrology::ConnectionType type) { m_connectionType = type; }

	QString appSignalID() const;
	QString customSignalID() const;
	QString equipmentID() const;
	QString rackCaption() const;
	QString chassisStr() const;
	QString moduleStr() const;
	QString placeStr() const;
	QString caption() const;
	QString electricRangeStr() const;
	QString electricSensorStr() const;
	QString physicalRangeStr() const;
	QString engineeringRangeStr() const;

	CalibratorManager* calibratorManager() const { return m_pCalibratorManager; }
	QString calibratorStr() const;
	void setCalibratorManager(CalibratorManager* pCalibratorManager) { m_pCalibratorManager = pCalibratorManager; }

	double percent() const { return m_percent; }
	void setPercent(double percent) { m_percent = percent; }

	int comparatorIndex() const { return m_comparatorIndex; }
	void setComparatorIndex(int index) { m_comparatorIndex = index; }

	int comparatorValueType() const { return m_comparatorValueType; }
	void setComparatorValueType(int type) { m_comparatorValueType = type; }

	bool isNegativeRange() const { return m_negativeRange; }
	void setNegativeRange(bool negativeRange) { m_negativeRange = negativeRange; }

	double tunStateForRestore() const { return m_tunStateForRestore; }
	void setTunStateForRestore(double state) { m_tunStateForRestore = state; }

	IoSignalParam& operator=(const IoSignalParam& from);

private:

	mutable QMutex m_mutex;

	Metrology::SignalParam	m_param[Metrology::ConnectionIoTypeCount];
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::Unsed;

	CalibratorManager* m_pCalibratorManager = nullptr;

	double m_percent = 0;					// for measuring of linearity
	int m_comparatorIndex = -1;				// for measuring of comparators - current cmp index
	int m_comparatorValueType = -1;			// for measuring of comparators - cmp or hst

	bool m_negativeRange = false;
	double m_tunStateForRestore = 0;		// for restore tun value after measuring
};

// ==============================================================================================
// class MultiChannelSignal consists list of Metrology::Signal
//
class MultiChannelSignal
{
public:

	explicit MultiChannelSignal();
	MultiChannelSignal(const MultiChannelSignal& from);
	virtual ~MultiChannelSignal() {}

public:

	void clear();
	bool isEmpty() const;

	int channelCount() const { return m_channelCount; }
	void setChannelCount(int count);

	Metrology::Signal* metrologySignal(int channel) const;
	bool setMetrologySignal(int measureKind, int channel, Metrology::Signal* pSignal);

	Metrology::Signal* firstMetrologySignal() const;

	Metrology::SignalLocation location() const { return m_location; }

	QString signalID() const { return m_signalID; }
	QString caption() const { return m_caption; }

	MultiChannelSignal& operator=(const MultiChannelSignal& from);

private:

	mutable QMutex m_mutex;
	int m_channelCount = 0;
	QVector<Metrology::Signal*> m_pSignalList;

	Metrology::SignalLocation m_location;

	QString m_signalID;		// depend from SignalLocation and measureKind
	QString m_caption;		// depend from SignalLocation and measureKind
};

// ==============================================================================================
// class MeasureSignal consists array of two classes MultiChannelSignal: input and output
//
// MeasureSignal --- MultiChannelSignal[ConnectionIoType::ioCount] --- Metrology::Signal[Metrology::ChannelCount]
//
class MeasureSignal
{
public:

	MeasureSignal();
	MeasureSignal(const MeasureSignal& from);
	virtual ~MeasureSignal() {}

public:

	void clear();
	bool isEmpty() const;

	Metrology::ConnectionType connectionType() const { return m_connectionType; }

	int channelCount() const { return m_channelCount; }
	void setChannelCount(int count);

	MultiChannelSignal multiChannelSignal(int ioType) const;
	bool setMultiSignal(int ioType, const MultiChannelSignal& signal);

	Metrology::Signal* metrologySignal(int ioType, int channel) const;
	bool setMetrologySignal(int measureKind, const Metrology::ConnectionBase& сonnectionBase, Metrology::ConnectionType connectionType, int channel, Metrology::Signal* pSignal);

	bool contains(Metrology::Signal* pSignal) const;

	MeasureSignal& operator=(const MeasureSignal& from);

private:

	mutable QMutex m_mutex;

	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::Unsed;

	int m_channelCount = 0;
	MultiChannelSignal m_signal[Metrology::ConnectionIoTypeCount];
};

// ==============================================================================================

Q_DECLARE_METATYPE(MeasureSignal)

// ==============================================================================================

class SignalBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalBase(QObject* parent = nullptr);
	virtual ~SignalBase() {}

public:

	void					clear();

	// Signals
	//
	int						signalCount() const;
	void					clearSignalList();

	int						appendSignal(const Metrology::SignalParam& param);

	Metrology::Signal*		signalPtr(const QString& appSignalID);
	Metrology::Signal*		signalPtr(const Hash& hash);
	Metrology::Signal*		signalPtr(int index);

	Metrology::Signal		signal(const QString& appSignalID) const;
	Metrology::Signal		signal(const Hash& hash) const;
	Metrology::Signal		signal(int index) const;

	Metrology::SignalParam	signalParam(const QString& appSignalID) const;
	Metrology::SignalParam	signalParam(const Hash& hash) const;
	Metrology::SignalParam	signalParam(int index) const;

	void					setSignalParam(const QString& appSignalID, const Metrology::SignalParam& param);
	void					setSignalParam(const Hash& hash, const Metrology::SignalParam& param);
	void					setSignalParam(int index, const Metrology::SignalParam& param);

	Metrology::SignalState	signalState(const QString& appSignalID) const;
	Metrology::SignalState	signalState(const Hash& hash) const;
	Metrology::SignalState	signalState(int index) const;

	void					setSignalState(const QString& appSignalID, const Metrology::SignalState& state);
	void					setSignalState(const Hash& hash, const Metrology::SignalState& state);
	void					setSignalState(int index, const Metrology::SignalState& state);

	bool					enableForMeasure(Metrology::ConnectionType connectionType, Metrology::Signal* pSignal);

	// hashs for update signal state
	//
	int						hashForRequestStateCount() const;
	Hash					hashForRequestState(int index);

	// racks for measure
	//
	RackBase&				racks() { return m_rackBase; }

	int						createRackListForMeasure(int measureKind, Metrology::ConnectionType connectionType);
	void					clearRackListForMeasure();

	int						rackForMeasureCount() const;
	Metrology::RackParam	rackForMeasure(int index) const;

	// module
	//
	QString					findAppSignalIDforSerialNo(const QString& moduleID);

	// signals for measure
	//
	void					initSignals();
	void					updateRackParam();
	void					initMetrologyConnections();

	int						createSignalListForMeasure(int measureKind, Metrology::ConnectionType connectionType, int rackIndex);
	void					clearSignalListForMeasure();

	int						signalForMeasureCount() const;
	MeasureSignal			signalForMeasure(int index) const;
	bool					setSignalForMeasure(int index, const MeasureSignal& signal);

	// main signal for measure
	//
	MeasureSignal			activeSignal() const;
	void					setActiveSignal(const MeasureSignal& signal);
	void					clearActiveSignal();

	// other bases
	//
	TuningBase&				tuning() { return m_tuningBase; }								// sources and signals of tuning
	Metrology::ConnectionBase& connections() { return m_connectionBase; }					// metrology connections
	StatisticsBase&			statistics() { return m_statisticsBase; }						// statistics of measured signals

	// comparators
	//
	bool					loadComparatorsInSignal(const ComparatorSet& comparatorSet);
	bool					initComparatorSignals(Metrology::ComparatorEx* pComparatorEx);

private:

	// all racks that received form CgfSrv
	//
	RackBase				m_rackBase;

	// all signals that received form CgfSrv
	//
	mutable QMutex			m_signalMutex;
	QHash<Hash, int>		m_signalHashList;
	QVector<Metrology::Signal> m_signalList;

	// list of hashes in order to receive signal state form AppDataSrv
	//
	mutable QMutex			m_stateMutex;
	QVector<Hash>			m_requestStateList;

	// list of racks form CgfSrv in order to select signal for measure
	//
	mutable QMutex			m_rackMutex;
	QVector<Metrology::RackParam> m_rackList;

	// list of signals for measure
	//
	mutable QMutex			m_signalMesaureMutex;
	QVector<MeasureSignal>	m_signalMeasureList;

	// main signal that are measuring at the current moment
	//
	mutable QMutex			m_activeSignalMutex;
	MeasureSignal			m_activeSignal;

	TuningBase				m_tuningBase;					// sources and signals of tuning
	Metrology::ConnectionBase m_connectionBase;				// metrology connections
	StatisticsBase			m_statisticsBase;				// statistics of measured signals

signals:

	void					activeSignalChanged(const MeasureSignal& signal);
	void					signalParamChanged(const QString& appSignalID);
};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================

#endif // SIGNALBASE_H
