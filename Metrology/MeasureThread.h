#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMutex>
#include "Measure.h"

// ==============================================================================================

const int MEASURE_THREAD_TIMEOUT_STEP = 100; // 100 milliseconds

// ==============================================================================================

class MeasureThread : public QThread
{
    Q_OBJECT

public:

    explicit MeasureThread(QObject *parent = 0);

    void        setMeasureType(int type)    { m_measureType = type; }
    int         getMeasureType()            { return m_measureType; }

private:

    QMutex      m_mutex;

    int         m_measureType = MEASURE_TYPE_UNKNOWN;

    bool        m_cmdStopMeasure = true;

    void        waitMeasureTimeout();

    bool        prepareCalibrator();

    void        measureLinearity();
    void        measureComprators();
    void        measureComplexComprators();

protected:

    void        run();

public:

    bool        stop();

signals:

    void        measureInfo(QString);
    void        measureInfo(int);

public slots:

};

// ==============================================================================================

#endif // MEASURETHREAD_H
