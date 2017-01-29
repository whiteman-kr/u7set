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
                            ~MeasureThread();

    void                    init(QWidget* parent = 0);

    void                    setMeasureType(const int measureType) { m_measureType = measureType; }

    void                    stop() { m_cmdStopMeasure = true; }

private:

    QWidget*                m_parent = nullptr;

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;
    MeasureSignalParam      m_activeSignalParam[MAX_CHANNEL_COUNT];

    bool                    m_cmdStopMeasure = true;

    void                    waitMeasureTimeout();

    bool                    calibratorIsValid(CalibratorManager* pManager);

    bool                    prepareCalibrator(CalibratorManager* pManager, const int calibratorMode, const E::InputUnit signalInputUnit, const double inputElectricHighLimit);

    void                    measureLinearity();
    void                    measureComprators();
    void                    measureComplexComprators();

protected:

    void                    run();

signals:

    void                    showMsgBox(QString);

    void                    measureInfo(QString);
    void                    measureInfo(int);

    void                    measureComplite(Measurement*);

private slots:

    void                    msgBox(QString text) const { QMessageBox::information(m_parent, tr("Measurement process"), text); }

    void                    updateSignalParam(const Hash& signalHash);

    void                    stopMeasure();
};

// ==============================================================================================

#endif // MEASURETHREAD_H
