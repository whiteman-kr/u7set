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

	Hash						m_signalHash[Metrology::ChannelCount];

	Metrology::SignalLocation	m_location;

	QString						m_strID;

public:

	bool						isEmpty() const;
	void						clear();

	Hash						hash(int channel) const;
	bool						setSignal(int channel, int measureKind, const Metrology::SignalParam& param);

	Metrology::SignalLocation&	location() { return m_location; }
	void						setLocation(const Metrology::SignalLocation& location) { m_location = location; }

	QString&					strID() { return m_strID; }

	MetrologyMultiSignal&		operator=(const MetrologyMultiSignal& from);
};

// ==============================================================================================

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

	bool					m_equalPhysicalRange = false;

public:

	void					clear();
	bool					isValid() const;

	Metrology::SignalParam	param(int type) const;
	bool					setParam(int type, const Metrology::SignalParam& param);

	int						outputSignalType() const { return m_outputSignalType; }
	void					setOutputSignalType(int type) { m_outputSignalType = type; }

	bool					equalPhysicalRange() const { return m_equalPhysicalRange; }
	bool					testPhysicalRange();

	QString					rackCaption() const;
	QString					signalID(bool showCustomID, const QString& divider) const;
	QString					chassisStr() const;
	QString					moduleStr() const;
	QString					placeStr() const;
	QString					caption(const QString &divider) const;
	QString					physicalRangeStr(const QString& divider) const;
	QString					electricRangeStr(const QString& divider) const;
	QString					electricSensorStr(const QString& divider) const;

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
	Hash					signalHash(int type, int channel) const;

	bool					setSignal(int type, const MetrologyMultiSignal& signal);
	bool					setSignal(int channel, int measureKind, int outputSignalType, const Metrology::SignalParam& param);

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

	// racks that received form CgfSrv
	//
	RackBase				m_rackBase;

	// all signals that received form CgfSrv
	//
	mutable QMutex			m_signalMutex;
	QMap<Hash, int>			m_signalHashMap;
	QVector<Metrology::Signal> m_signalList;

	// units that received form CgfSrv
	//
	UnitList				m_unitList;

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
	QVector<MeasureSignal>	m_signalMesaureList;

	// main signal that are measuring at the current moment
	//
	mutable QMutex			m_activeSignalMutex;
	MeasureSignal			m_activeSignal;

	// output signals
	//
	OutputSignalBase		m_outputSignalBase;

	// signals of tuning
	//
	TuningSignalBase		m_tuningSignalBase;

public:

	void					clear();
	void					sortByPosition();

	// Signals
	//
	int						signalCount() const;
	void					clearSignalList();

	int						appendSignal(const Metrology::SignalParam& param);

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

	int						createMeasureRackList(int outputSignalType);
	void					clearMeasureRackList();

	int						measureRackCount() const;
	Metrology::RackParam	measureRack(int index);

	// signals for measure
	//
	void					initSignals();
	void					updateRackParam();

	int						createMeasureSignalList(int measureKind, int outputSignalType, int rackIndex);
	void					clearMeasureSignalList();

	int						measureSignalCount() const;
	MeasureSignal			measureSignal(int index);

	// main signal for measure
	//
	MeasureSignal			activeSignal() const;
	void					setActiveSignal(const MeasureSignal& signal);
	void					clearActiveSignal();

	// units
	//
	UnitList&				units() { return m_unitList; }

	// output signals
	//
	OutputSignalBase&		outputSignals() { return m_outputSignalBase; }

	// signals of tuning
	//
	TuningSignalBase&		tuningSignals() { return m_tuningSignalBase; }

signals:

	void					updatedSignalParam(Hash signalHash);

	void					activeSignalChanged(const MeasureSignal& signal);

public slots:

};

// ==============================================================================================

extern SignalBase theSignalBase;

// ==============================================================================================

#endif // SIGNALBASE_H
