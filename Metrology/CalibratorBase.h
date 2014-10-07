#ifndef CALIBRATORBASE_H
#define CALIBRATORBASE_H

#include <QObject>
#include <QMutex>
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

const int MAX_CALIBRATOR_COUNT = 6;

// ==============================================================================================

const char* const CalibratorsColumn[] =
{
    QT_TRANSLATE_NOOP("CalibratorBase.h", "Serial port"),
    QT_TRANSLATE_NOOP("CalibratorBase.h", "Type"),
    QT_TRANSLATE_NOOP("CalibratorBase.h", "Connected"),
    QT_TRANSLATE_NOOP("CalibratorBase.h", "Serial number"),
    // QT_TRANSLATE_NOOP("CalibratorBase.h", "Kind"),
};

const int   CalibratorsColumnCount  = sizeof(CalibratorsColumn)/sizeof(char*);

const int   CalibratorColumnPort    = 0,
            CalibratorColumnType    = 1,
            CalibratorColumnConnect = 2,
            CalibratorColumnSN      = 3;

// ==============================================================================================

class CalibratorBase : public QObject
{
    Q_OBJECT

public:

    explicit CalibratorBase(QObject *parent = 0);
            ~CalibratorBase();

    void                        init(QWidget* parent = 0);
    void                        clear();

    void                        show();

    int                         calibratorCount();

    CalibratorManager*          getCalibratorManager(int index);

    int                         getConnectedCalibratorsCount() { return m_connectedCalibratorsCount; }

private:

    QMutex                      m_mutex;

    QTimer                      m_timer;
    int                         m_timeout = 0;

    QList<CalibratorManager*>   m_calibratorManagerList;

    int                         m_connectedCalibratorsCount = 0;
    int                         m_connectedCalibratorsIndex[MAX_CALIBRATOR_COUNT];


    void                        updateConnectedCalibrators();

private:

    // Elements of interface
    //
    QWidget*            m_parentWidget = nullptr;
    QDialog*            m_pMainWnd = nullptr;

    QMenu*              m_pCalibratorMenu = nullptr;
    QMenuBar*           m_pMenuBar = nullptr;
    QAction*            m_pInitAction = nullptr;
    QAction*            m_pManageAction = nullptr;
    QAction*            m_pEditAction = nullptr;

    QTableWidget*       m_pCalibratorView = nullptr;
    QProgressBar*       m_pCalibratorProgress = nullptr;


    void                createCalibrators();
    void                removeCalibrators();

    void                createMainWnd();

    void                setHeaderList();
    void                updateList();

    void                appendCalibratorManger(CalibratorManager* pManager);
    void                removeCalibratorManger(int index);

signals:

    void                calibratorOpen();
    void                calibratorClose();

    void                calibratorConnectedChanged(int);

public slots:

    void                timeoutInitialization();            // Slot of timer

    void                onInitialization();                 // Slot of calibrator menu - Initialization
    void                onManage();                         // Slot of calibrator menu - Manage
    void                onEdit();                           // Slot of calibrator menu - Edit setting
    void                onContextMenu(QPoint);              // Slot of context menu - Manage

    void                onClose();                          // Slot blocks close the windows, until the initialization process finishing

    void                editSettings(int row,int);          // Slot for edit serial port and type of calibrator

    void                onCalibratorConnected();            // Slots events from calibrator
    void                onCalibratorDisconnected();         // Slots events from calibrator
};

// ==============================================================================================

extern CalibratorBase theCalibratorBase;

// ==============================================================================================

#endif // CALIBRATORBASE_H
