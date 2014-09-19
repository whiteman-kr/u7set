#ifndef CALIBRATORBASE_H
#define CALIBRATORBASE_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QDialog>
#include <QMenuBar>
#include <QAction>
#include <QTimer>
#include <QTableWidget>
#include <QProgressBar>

#include "Calibrator.h"
#include "CalibratorManagerDialog.h"

// ==============================================================================================

const int MAX_CALIBRATOR_COUNT = 6;

// ==============================================================================================

class CalibratorBase : public QObject
{
    Q_OBJECT


public:

    explicit CalibratorBase(QObject *parent = 0);

    void                showInitializationWnd();
    Calibrator*         getMainCalibrator();

private:

    QList<Calibrator*>              m_calibratorList;
    QList<CalibratorManagerDialog*> m_calibratorManagerList;


private:

    // Elements of interface
    //
    QDialog*            m_pInitializationWnd = nullptr;

    QMenu*              m_pCalibratorMenu = nullptr;
    QMenuBar*           m_pMenuBar  = nullptr;
    QAction*            m_pInitAction = nullptr;
    QAction*            m_pManageAction = nullptr;

    QTableWidget*       m_pCalibratorView = nullptr;
    QProgressBar*       m_pCalibratorProgress = nullptr;

    QTimer              m_timer;
    int                 m_timeout = 0;

    void                init();
    void                createCalibratorObjects();
    void                createInitializationWnd();

    void                updateList();

signals:

    void                openAllCalibrator();
    void                closeAllCalibrator();

public slots:

    void                timeoutInitialization();            // Slot of timer

    void                onInitialization();                 // Slot of calibrator menu - Initialization
    void                onManage();                         // Slot of calibrator menu - Manage

    void                onFinishedInitializationWnd();      // Slot blocks close the windows, until the initialization process finished

    void                editSettings(int row,int column);   // Slot for edit serial port and type of calibrator


    void                onConnected();                      // Slots events from calibrator
};

// ==============================================================================================

#endif // CALIBRATORBASE_H
