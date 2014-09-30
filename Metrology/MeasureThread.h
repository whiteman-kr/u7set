#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMutex>
#include "Measure.h"

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

    bool        prepareCalibrator();

    void        measureLinearity() {};
    void        measureComprators() {};
    void        measureComplexComprators() {};



protected:

    void        run();
    bool        eventFilter(QObject *object, QEvent *event);

public:

    bool        stop();

signals:

    void        measureState(QString);

public slots:

};

// ==============================================================================================

#endif // MEASURETHREAD_H
