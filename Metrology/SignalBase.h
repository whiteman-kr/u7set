#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/MetrologySignal.h"
#include "../lib/MetrologyConnectionBase.h"

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

private:

	mutable QMutex			m_mutex;

	Metrology::SignalParam	m_param[Metrology::IO_SIGNAL_CONNECTION_TYPE_COUNT];
	int						m_signalConnectionType = Metrology::CONNECTION_TYPE_UNUSED;

	CalibratorManager*		m_pCalibratorManager = nullptr;

	double					m_percent = 0;					// for measuring of linearity
	int						m_comparatorIndex = -1;			// for measuring of comparators - current cmp index
	int						m_comparatorValueType = -1;		// for measuring of comparators - cmp or hst

	bool					m_negativeRange = false;
	double					m_tunStateForRestore = 0;		// for restore tun value after measuring

public:

	bool					isValid() const;
	void					clear();

	Metrology::SignalParam	param(int type) const;
	bool					setParam(int type, const Metrology::SignalParam& param);

	int						signalConnectionType() const { return m_signalConnectionType; }
	void					setSignalConnectionType(int type) { m_signalConnectionType = type; }

	QString					appSignalID() const;
	QString					customSignalID() const;
	QString					equipmentID() const;
	QString					rackCaption() const;
	QString					chassisStr() const;
	QString					moduleStr() const;
	QString					placeStr() const;
	QString					caption() const;
	QString					electricRangeStr() const;
	QString					electricSensorStr() const;
	QString					physicalRangeStr() const;
	QString					engineeringRangeStr() const;

	CalibratorManager*		calibratorManager() const { return m_pCalibratorManager; }
	QString					calibratorStr() const;
	void					setCalibratorManager(CalibratorManager* pCalibratorManager) { m_pCalibratorManager = pCalibratorManager; }

	double					percent() const { return m_percent; }
	void					setPercent(double percent) { m_percent = percent; }

	int						comparatorIndex() const { return m_comparatorIndex; }
	void					setComparatorIndex(int index) { m_comparatorIndex = index; }

	int						comparatorValueType() const { return m_comparatorValueType; }
	void					setComparatorValueType(int type) { m_comparatorValueType = type; }

	bool					isNegativeRange() const { return m_negativeRange; }
	void					setNegativeRange(bool negativeRange) { m_negativeRange = negativeRange; }

	double					tunStateForRestore() const { return m_tunStateForRestore; }
	void					setTunStateForRestore(double state) { m_tunStateForRestore = state; }

	IoSignalParam&			operator=(const IoSignalParam& from);
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

private:

	mutable QMutex				m_mutex;
	int							m_channelCount = 0;
	QVector<Metrology::Signal*>	m_pSignalList;

	Metrology::SignalLocation	m_location;

	QString						m_signalID;		// depend from SignalLocation and measureKind
	QString						m_caption;		// depend from SignalLocation and measureKind

public:

	void						clear();
	bool						isEmpty() const;

	int							channelCount() const { return m_channelCount; }
	void						setChannelCount(int count);

	Metrology::Signal*			metrologySignal(int channel) const;
	bool						setMetrologySignal(int measureKind, int channel, Metrology::Signal* pSignal);

	Metrology::Signal*			firstMetrologySignal() const;

	Metrology::SignalLocation	location() const { return m_location; }

	QString						signalID() const { return m_signalID; }
	QString						caption() const { return m_caption; }

	MultiChannelSignal&			operator=(const MultiChannelSignal& from);
};

// ==============================================================================================
// class MeasureSignal consists array of two classes MultiChannelSignal: input and output
//
// MeasureSignal --- MultiChannelSignal[IO_SIGNAL_CONNECTION_TYPE_COUNT] --- Metrology::Signal[Metrology::ChannelCount]
//
class MeasureSignal
{
public:

	MeasureSignal();
	MeasureSignal(const MeasureSignal& from);
	virtual ~MeasureSignal() {}

private:

	mutable QMutex			m_mutex;

	int						m_signalConnectionType = Metrology::CONNECTION_TYPE_UNUSED;

	int						m_channelCount = 0;
	MultiChannelSignal		m_signal[Metrology::IO_SIGNAL_CONNECTION_TYPE_COUNT];

public:

	void					clear();
	bool					isEmpty() const;

	int						signalConnectionType() const { return m_signalConnectionType; }

	int						channelCount() const { return m_channelCount; }
	void					setChannelCount(int count);

	MultiChannelSignal		multiChannelSignal(int type) const;
	bool					setMultiSignal(int type, const MultiChannelSignal& signal);

	Metrology::Signal*		metrologySignal(int type, int channel) const;
	bool					setMetrologySignal(int measureKind, const Metrology::ConnectionBase& сonnectionBase, int signalConnectionType, int channel, Metrology::Signal* pSignal);

	bool					contains(Metrology::Signal* pSignal) const;

	MeasureSignal&			operator=(const MeasureSignal& from);
};

// ==============================================================================================

Q_DECLARE_METATYPE(MeasureSignal)

// ==============================================================================================

class SignalBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalBase(QObject *parent = nullptr);
	virtual ~SignalBase() {}

private:

	// all racks that received form CgfSrv
	//
	RackBase				m_rackBase;

	// all signals that received form CgfSrv
	//
	mutable QMutex			m_signalMutex;
	QMap<Hash, int>			m_signalHashMap;
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

	TuningBase				m_tuningBase;				// sources and signals of tuning
	Metrology::ConnectionBase m_signalConnectionBase;	// signal connections
	StatisticsBase			m_statisticsBase;			// statistics of measured signals

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

	bool					enableForMeasure(int signalConnectionType, Metrology::Signal* pSignal);

	// hashs for update signal state
	//
	int						hashForRequestStateCount() const;
	Hash					hashForRequestState(int index);

	// racks for measure
	//
	RackBase&				racks() { return m_rackBase; }

	int						createRackListForMeasure(int measureKind, int signalConnectionType);
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
	void					initSignalConnections();

	int						createSignalListForMeasure(int measureKind, int signalConnectionType, int rackIndex);
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
	TuningBase&				tuning() { return m_tuningBase; }							// sources and signals of tuning
	Metrology::ConnectionBase& signalConnections() { return m_signalConnectionBase; }	// signal connections
	StatisticsBase&			statistics() { return m_statisticsBase; }					// statistics of measured signals
	
	// comparators
	//
	bool					loadComparatorsInSignal(const ComparatorSet& comparatorSet);
	bool					initComparatorSignals(Metrology::ComparatorEx* pComparatorEx);

signals:

	void					activeSignalChanged(const MeasureSignal& signal);
	void					signalParamChanged(const QString& appSignalID);
};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================

#endif // SIGNALBASE_H
