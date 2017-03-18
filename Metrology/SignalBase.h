#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/MetrologySignal.h"

#include "CalibratorManager.h"
#include "RackBase.h"
#include "OutputSignalBase.h"
#include "TuningSignalBase.h"

// ==============================================================================================

class MetrologyMultiSignal
{
public:

	MetrologyMultiSignal();
	MetrologyMultiSignal(const MetrologyMultiSignal& from);
	virtual ~MetrologyMultiSignal() {}

private:

	mutable QMutex				m_mutex;
	Metrology::Signal*			m_pSignal[Metrology::ChannelCount];

	Metrology::SignalLocation	m_location;
	QString						m_strID;		// depend from SignalLocation and measureKind

public:

	bool						isEmpty() const;
	void						clear();

	Metrology::Signal*			metrologySignal(int channel) const;
	bool						setMetrologySignal(int measureKind, int channel, Metrology::Signal* pSignal);

	Metrology::SignalLocation	location() const { return m_location; }
	QString						strID() const { return m_strID; }

	MetrologyMultiSignal&		operator=(const MetrologyMultiSignal& from);
};

// ==============================================================================================

#define						MultiTextDivider	"\n"

// ----------------------------------------------------------------------------------------------

class MeasureMultiParam
{
public:

	MeasureMultiParam();
	MeasureMultiParam(const MeasureMultiParam& from);
	virtual~MeasureMultiParam() {}

private:

	mutable QMutex			m_mutex;

	Metrology::SignalParam	m_param[MEASURE_IO_SIGNAL_TYPE_COUNT];
	int						m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

	CalibratorManager*		m_pCalibratorManager = nullptr;

public:

	bool					isValid() const;
	void					clear();

	Metrology::SignalParam	param(int type) const;
	bool					setParam(int type, const Metrology::SignalParam& param);

	int						outputSignalType() const { return m_outputSignalType; }
	void					setOutputSignalType(int type) { m_outputSignalType = type; }

	QString					rackCaption() const;
	QString					signalID(bool showCustomID) const;
	QString					equipmentID() const;
	QString					chassisStr() const;
	QString					moduleStr() const;
	QString					placeStr() const;
	QString					caption() const;
	QString					physicalRangeStr() const;
	QString					electricRangeStr() const;
	QString					electricSensorStr() const;

	CalibratorManager*		calibratorManager() const { return m_pCalibratorManager; }
	QString					calibratorStr() const;
	void					setCalibratorManager(CalibratorManager* pCalibratorManager) { m_pCalibratorManager = pCalibratorManager; }

	MeasureMultiParam&		operator=(const MeasureMultiParam& from);
};

// ==============================================================================================

class MeasureSignal
{
public:

	MeasureSignal();
	MeasureSignal(const MeasureSignal& from);
	virtual ~MeasureSignal() {}

private:

	mutable QMutex			m_mutex;

	int						m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

	MetrologyMultiSignal	m_signal[MEASURE_IO_SIGNAL_TYPE_COUNT];

public:

	void					clear();
	bool					isEmpty() const;

	int						outputSignalType() const { return m_outputSignalType; }

	MetrologyMultiSignal	signal(int type) const;
	bool					setSignal(int type, const MetrologyMultiSignal& signal);

	Metrology::Signal*		metrologySignal(int type, int channel) const;
	bool					setMetrologySignal(int measureKind, int outputSignalType, int channel, Metrology::Signal* pSignal);

	bool					contains(Metrology::Signal* pSignal);

	MeasureSignal&			operator=(const MeasureSignal& from);
};

// ==============================================================================================

Q_DECLARE_METATYPE(MeasureSignal)

// ==============================================================================================

class SignalBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalBase(QObject *parent = 0);
	virtual ~SignalBase() {}

private:

	// all racks that received form CgfSrv
	//
	RackBase				m_rackBase;

	// all units that received form CgfSrv
	//
	UnitList				m_unitList;

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

	// output signals
	//
	OutputSignalBase		m_outputSignalBase;

	// sources and signals of tuning
	//
	TuningBase				m_tuningBase;

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

	Metrology::Signal		signal(const QString& appSignalID);
	Metrology::Signal		signal(const Hash& hash);
	Metrology::Signal		signal(int index);

	Metrology::SignalParam	signalParam(const QString& appSignalID);
	Metrology::SignalParam	signalParam(const Hash& hash);
	Metrology::SignalParam	signalParam(int index);

	void					setSignalParam(const QString& appSignalID, const Metrology::SignalParam& param);
	void					setSignalParam(const Hash& hash, const Metrology::SignalParam& param);
	void					setSignalParam(int index, const Metrology::SignalParam& param);

	Metrology::SignalState	signalState(const QString& appSignalID);
	Metrology::SignalState	signalState(const Hash& hash);
	Metrology::SignalState	signalState(int index);

	void					setSignalState(const QString& appSignalID, const Metrology::SignalState& state);
	void					setSignalState(const Hash& hash, const Metrology::SignalState& state);
	void					setSignalState(int index, const Metrology::SignalState& state);

	// hashs for update signal state
	//
	int						hashForRequestStateCount() const;
	Hash					hashForRequestState(int index);

	// racks for measure
	//
	RackBase&				racks() { return m_rackBase; }

	int						createRackListForMeasure(int outputSignalType);
	void					clearRackListForMeasure();

	int						rackCountForMeasure() const;
	Metrology::RackParam	rackForMeasure(int index);

	// signals for measure
	//
	void					initSignals();
	void					updateRackParam();

	int						createSignalListForMeasure(int measureKind, int outputSignalType, int rackIndex);
	void					clearSignalListForMeasure();

	int						signalForMeasureCount() const;
	MeasureSignal			signalForMeasure(int index);

	// main signal for measure
	//
	MeasureSignal			activeSignal() const;
	void					setActiveSignal(const MeasureSignal& signal);
	void					clearActiveSignal();

	// other bases
	//
	UnitList&				units() { return m_unitList; }						// units that received form CgfSrv
	OutputSignalBase&		outputSignals() { return m_outputSignalBase; }		// output signals
	TuningBase&				tuning() { return m_tuningBase; }					// sources and signals of tuning

signals:

	void					updatedSignalParam(Hash signalHash);

	void					activeSignalChanged(const MeasureSignal& signal);

public slots:

};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================

#endif // SIGNALBASE_H
