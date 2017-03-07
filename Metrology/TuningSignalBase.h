#ifndef TUNINGSIGNALBASE_H
#define TUNINGSIGNALBASE_H

// This is class was designed to save and distribute tuning signals for measure
//
// Algorithm:
//
//

#include <QObject>

#include "../lib/Hash.h"
#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

class TuningSourceState
{
public:
                            TuningSourceState() {}
                            ~TuningSourceState() {}
private:

    bool                    m_isReply = false;
    quint64                 m_requestCount = 0;
    quint64                 m_replyCount = 0;
    int                     m_commandQueueSize = 0;

public:

    bool                    isReply() const { return m_isReply; }
    void                    setIsReply(bool isReply) { m_isReply = isReply; }

    quint64                 requestCount() const { return m_requestCount; }
    void                    setRequestCount(quint64 count) { m_requestCount = count; }

    quint64                 replyCount() const { return m_replyCount; }
    void                    setReplyCount(quint64 count) { m_replyCount = count; }

    int                     commandQueueSize() const { return m_commandQueueSize; }
    void                    setCommandQueueSize(int size) { m_commandQueueSize = size; }
};

// ==============================================================================================

class TuningSource
{
public:
                                TuningSource();
   explicit                     TuningSource(const Network::DataSourceInfo& info);
                                ~TuningSource();
private:

    quint64                     m_sourceID = -1;

    QString                     m_equipmentID;
    QString                     m_caption;

    QString                     m_serverIP;
    int                         m_serverPort = 0;

    int                         m_channel = -1;

    QString                     m_subSystem;

    int                         m_lmNumber = -1;

    TuningSourceState           m_state;

public:

    quint64                     sourceID() const { return m_sourceID; }
    void                        setSourceID(quint64 id) { m_sourceID = id; }

    QString                     equipmentID() const { return m_equipmentID; }
    void                        setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

    QString                     caption() const { return m_caption; }
    void                        setCaption(const QString& caption) { m_caption = caption; }

    QString                     serverIP() const { return m_serverIP; }
    void                        setServerIP(const QString& ip) { m_serverIP = ip; }

    int                         serverPort() const { return m_serverPort; }
    void                        setServerPort(int port) { m_serverPort = port; }

    int                         channel() const { return m_channel; }
    void                        setChannel(int channel) { m_channel = channel; }

    QString                     subSystem() const { return m_subSystem; }
    void                        setSubSystem(const QString& subSystem) { m_subSystem = subSystem; }

    int                         lmNumber() const { return m_lmNumber; }
    void                        setLmNumber(int lmNumber) { m_lmNumber = lmNumber; }

    TuningSourceState&          state() { return m_state; }
    void                        setState(TuningSourceState state) { m_state = state; }
};

// ==============================================================================================

class TuningSignalState
{
public:
                            TuningSignalState() {}
                            ~TuningSignalState() {}
private:

    bool                    m_valid = false;

    double                  m_value = 0;

    double                  m_lowLimit = 0;
    double                  m_highLimit = 0;

public:

    bool                    valid() const { return m_valid; }
    void                    setValid(bool valid) { m_valid = valid; }

    double                  value() const { return m_value; }
    void                    setValue(double value) { m_value = value; }

    double                  lowLimit() const { return m_lowLimit; }
    void                    setLowLimit(double lowLimit) { m_lowLimit = lowLimit; }

    double                  highLimit() const { return m_highLimit; }
    void                    setHighLimit(double highLimit) { m_highLimit = highLimit; }

    void                    setLimits(double lowLimit, double highLimit) { m_lowLimit = lowLimit; m_highLimit = highLimit; }
};

// ==============================================================================================

class TuningSignal
{
public:

                            TuningSignal();
                            TuningSignal(const TuningSignal& from);
	explicit                TuningSignal(const Metrology::SignalParam& param);
                            ~TuningSignal();
private:

    Hash                    m_hash = 0;

	QString                 m_rack;
    QString                 m_appSignalID;
    QString                 m_customAppSignalID;
    QString                 m_equipmentID;
    QString                 m_caption;

    E::SignalType           m_signalType = E::SignalType::Analog;

    TuningSignalState       m_state;

    double                  m_defaultValue = 0;

    int                     m_precision = 2;

public:

    Hash                    hash() const { return m_hash; }
    void                    setHash(Hash hash) { m_hash = hash; }

	QString                 caseStr() const { return m_rack; }
	void                    setCase(const QString& caseStr) { m_rack = caseStr; }

    QString                 appSignalID() const { return m_appSignalID; }
    void                    setAppSignalID(const QString& appSignalID);

    QString                 customAppSignalID() const { return m_customAppSignalID; }
    void                    setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

    QString                 equipmentID() const { return m_equipmentID; }
    void                    setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

    QString                 caption() const { return m_caption; }
    void                    setCaption(const QString& caption) { m_caption = caption; }

    E::SignalType           signalType() const { return m_signalType; }
    void                    setSignalType(const E::SignalType& type) { m_signalType = type; }

    TuningSignalState&      state() { return m_state; }
    void                    setState(TuningSignalState state) { m_state = state; }

    QString                 valueStr() const;
    QString                 rangeStr() const;

    double                  defaultValue() const { return m_defaultValue; }
    QString                 defaultValueStr() const;
    void                    setDefaultValue(double value) { m_defaultValue = value; }

    int                     precision() const { return m_precision; }
    void                    setPrecision(int precision) { m_precision = precision; }
};

// ==============================================================================================

                            Q_DECLARE_METATYPE(TuningSignal)
                            Q_DECLARE_METATYPE(TuningSignal*)

// ==============================================================================================

class TuningWriteCmd
{
public:
                            TuningWriteCmd();
                            TuningWriteCmd(const Hash &signalHash, float value);
                            ~TuningWriteCmd();
private:

    Hash                    m_signalHash;
    float                   m_value;

public:

    Hash                    signalHash() const { return m_signalHash; }
    void                    setSignalHash(Hash hash) { m_signalHash = hash; }

    float                   value() const { return m_value; }
    void                    setValue(float value) { m_value = value; }
};

// ==============================================================================================

class TuningSignalBase : public QObject
{
    Q_OBJECT

public:
    explicit                TuningSignalBase(QObject *parent = 0);
                            ~TuningSignalBase();

public:

    void                    clear();

    // Source
    //

    int                     sourceCount() const;

    int                     appendSource(const TuningSource& source);

    TuningSource            source(int index) const;

    TuningSourceState       sourceState(quint64 sourceID);
    void                    setSourceState(quint64 sourceID, const Network::TuningSourceState& state);

    void                    clearSourceList();

    // Signal for read
    //

    void                    createSignalList();

    int                     signalCount() const;

    int                     appendSignal(const TuningSignal& signal);

    TuningSignal            signalForRead(const Hash& hash) const;
    TuningSignal            signalForRead(int index) const;

    void                    setSignalForRead(const TuningSignal& signal);

    TuningSignalState       signalState(const Hash& hash);
    void                    setSignalState(const Hash& hash, const Network::TuningSignalState& state);

    void                    singalsSetNovalid();

    void                    clearSignalLlst();

    // Commands for write
    //

    int                     cmdFowWriteCount() const;

    void                    appendCmdFowWrite(const TuningWriteCmd& cmd);
    void                    appendCmdFowWrite(const Hash& signalHash, float value);

    TuningWriteCmd          cmdFowWrite(int index);

private:

    mutable QMutex          m_sourceMutex;
    QVector<TuningSource>   m_sourceList;
    QMap<quint64, int>      m_sourceIdMap;

    mutable QMutex          m_signalMutex;
    QVector<TuningSignal>   m_signalList;
    QMap<Hash, int>         m_signalHashMap;

    mutable QMutex          m_cmdFowWriteMutex;
    QVector<TuningWriteCmd> m_cmdFowWriteList;

signals:

	void					signalsLoaded();

public slots:

    void                    updateSignalParam(const Hash& signalHash);

};

// ==============================================================================================

extern TuningSignalBase theTuningSignalBase;

// ==============================================================================================

#endif // SIGNALBASE_H
