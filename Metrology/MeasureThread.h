#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMutex>
#include "Measure.h"
#include "CalibratorBase.h"

// ==============================================================================================

const int MT_TIMEOUT_STEP       = 100; // 100 milliseconds
const int MT_VALUE_IS_READY     = 0xFFFF;

// ==============================================================================================

class MeasureThread : public QThread
{
    Q_OBJECT

public:

    explicit MeasureThread(QObject *parent = 0);

    void        setMeasureType(int type)    { m_measureType = type; }
    int         getMeasureType()            { return m_measureType; }

    bool        stop();

private:

    QMutex      m_mutex;

    int         m_measureType = MEASURE_TYPE_UNKNOWN;

    bool        m_cmdStopMeasure = true;

    void        waitMeasureTimeout();

    bool        prepareCalibrator(CalibratorManager* manager, int mode, int unit);

    void        measureLinearity();
    void        measureComprators();
    void        measureComplexComprators();

protected:

    void        run();

signals:

    // measure thread signals
    //
    void        measureInfo(QString);
    void        measureInfo(int);
    void        measureComplite();

public slots:

};

// ==============================================================================================

#endif // MEASURETHREAD_H
