#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include "Measure.h"
#include "CalibratorBase.h"

// ==============================================================================================

const int MEASURE_THREAD_TIMEOUT_STEP = 100; // 100 milliseconds

// ==============================================================================================

class MeasureThread : public QThread
{
    Q_OBJECT

public:

    explicit                MeasureThread(QWidget *parent = 0);

    void                    setMeasureType(int type)    { m_measureType = type; }

    bool                    stop();

private:

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;

    bool                    m_cmdStopMeasure = true;

    CalibratorManagerList   m_calibratorManagerList;

    void                    waitMeasureTimeout();

    bool                    prepareCalibrator(CalibratorManager* manager, int mode, int unit);

    void                    measureLinearity();
    void                    measureComprators();
    void                    measureComplexComprators();

protected:

    void                     run();

signals:

    // measure thread signals
    //
    void                    measureInfo(QString);
    void                    measureInfo(int);
    void                    measureComplite();
};

// ==============================================================================================

#endif // MEASURETHREAD_H
