#ifndef CALIBRATORBASE_H
#define CALIBRATORBASE_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QEvent>
#include <QDialog>
#include <QMenuBar>
#include <QAction>
#include <QTimer>
#include <QTableWidget>
#include <QProgressBar>

#include "Calibrator.h"
#include "CalibratorManager.h"

// ==============================================================================================


const int                   CALIBRATOR_0            = 0,
                            CALIBRATOR_1            = 1,
                            CALIBRATOR_2            = 2,
                            CALIBRATOR_3            = 3,
                            CALIBRATOR_4            = 4,
                            CALIBRATOR_5            = 5;

const int                   MAX_CALIBRATOR_COUNT    = 6;         // max amount calibrators in calibrator base

// ==============================================================================================

const int                   CALIBRATOR_COUNT_FOR_CC = 2;      // amount calibrators for measure complex cpmporator

// ==============================================================================================

const char* const CalibratorsColumn[] =
{
                            QT_TRANSLATE_NOOP("CalibratorBase.h", "Serial port"),
                            QT_TRANSLATE_NOOP("CalibratorBase.h", "Type"),
                            QT_TRANSLATE_NOOP("CalibratorBase.h", "Connected"),
                            QT_TRANSLATE_NOOP("CalibratorBase.h", "Serial number"),
};

const int                   CalibratorsColumnCount  = sizeof(CalibratorsColumn)/sizeof(char*);

const int                   CalibratorColumnPort    = 0,
                            CalibratorColumnType    = 1,
                            CalibratorColumnConnect = 2,
                            CalibratorColumnSN      = 3;

// ==============================================================================================

class CalibratorBase : public QObject
{
    Q_OBJECT

public:

    explicit                CalibratorBase(QObject *parent = 0);
                            ~CalibratorBase();

    void                    init(QWidget* parent = 0);
    void                    show();

    int                     count() { return m_calibratorManagerList.count(); }

    CalibratorManager*      at(int index);
    CalibratorManager*      firstConnectedCalibrator();

    CalibratorManager*      сalibratorForMeasure(int index);

    int                     connectedCalibratorsCount() { return m_connectedCalibratorsCount; }

    void                    clear();

private:

    QTimer                  m_timer;
    int                     m_timeout = 0;

    CalibratorManagerList   m_calibratorManagerList;

    void                    createCalibrators();
    void                    removeCalibrators();

    int                     m_connectedCalibratorsCount = 0;

    void                    updateConnectedCalibrators();

private:

    // Elements of interface
    //
    QWidget*                m_parentWidget = nullptr;
    QDialog*                m_pDialog = nullptr;

    QMenuBar*               m_pMenuBar = nullptr;
    QMenu*                  m_pCalibratorMenu = nullptr;
    QAction*                m_pInitAction = nullptr;
    QAction*                m_pManageAction = nullptr;
    QAction*                m_pSettingsAction = nullptr;
    QAction*                m_pCopyAction = nullptr;

    QTableWidget*           m_pCalibratorView = nullptr;
    QProgressBar*           m_pCalibratorProgress = nullptr;

    void                    createDialog();

    void                    setHeaderList();
    void                    updateList();

protected:

    bool                    eventFilter(QObject *object, QEvent *event);

signals:

    void                    calibratorOpen();
    void                    calibratorClose();

    void                    calibratorConnectedChanged(int);

public slots:

    void                    timeoutInitialization();            // Slot of timer

    void                    onInitialization();                 // Slot of calibrator menu - Initialization
    void                    onManage();                         // Slot of calibrator menu - Manage
    void                    onSettings();                       // Slot of calibrator menu - Edit setting
    void                    onSettings(int row,int);            // Slot for edit serial port and type of calibrator
    void                    onCopy();                           // Slot of calibrator menu - Copy serail number
    void                    onContextMenu(QPoint);              // Slot of context menu

    void                    onCalibratorConnected();            // Slots events from calibrator
    void                    onCalibratorDisconnected();         // Slots events from calibrator
};

// ==============================================================================================

extern CalibratorBase       theCalibratorBase;

// ==============================================================================================

#endif // CALIBRATORBASE_H
