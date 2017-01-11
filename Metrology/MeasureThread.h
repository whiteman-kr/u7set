#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMessageBox>

#include "Measure.h"
#include "CalibratorBase.h"
#include "SignalBase.h"

// ==============================================================================================

const int                   MEASURE_THREAD_TIMEOUT_STEP = 100; // 100 milliseconds

// ==============================================================================================

class MeasureThread : public QThread
{
    Q_OBJECT

public:

    explicit                MeasureThread(QObject *parent = 0);

    void                    init(QWidget* parent = 0);

    void                    setMeasureType(int type)    { m_measureType = type; }

    void                    stop()                      { m_cmdStopMeasure = true; }

private:

    QWidget*                m_parentWidget = nullptr;

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;
    bool                    m_cmdStopMeasure = true;

    MeasureMultiSignal      m_activeSignal;

    void                    waitMeasureTimeout();

    bool                    calibratorIsValid(CalibratorManager* pManager);

    bool                    prepareCalibrator(CalibratorManager* pManager, const int& calibratorMode, const int& signalInputUnit, const double& highInputLimit);

    void                    measureLinearity();
    void                    measureComprators();
    void                    measureComplexComprators();

protected:

    void                    run();

signals:

    void                    showMsgBox(QString);

    // measure thread signals
    //
    void                    measureInfo(QString);
    void                    measureInfo(int);
    void                    measureComplite(MeasureItem*);

private slots:

    void                    msgBox(QString text)        { QMessageBox::information(m_parentWidget, tr("Measurement process"), text); }

    void                    calibratorDisconnected();

    void                    finish();
};

// ==============================================================================================

#endif // MEASURETHREAD_H
