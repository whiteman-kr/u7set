#ifndef CALIBRATORMANAGER_H
#define CALIBRATORMANAGER_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>

#include "Calibrator.h"

// ==============================================================================================

class CalibratorManager : public QObject
{
    Q_OBJECT

public:
    explicit CalibratorManager(Calibrator* pCalibrator, QWidget* parent);
    ~CalibratorManager();

    void            show()                          { if (m_pDialog != nullptr) m_pDialog->show(); }

    int             getIndex()                      { return m_index;   }
    void            setIndex(int index)             { m_index = index;  }

    Calibrator*     getCalibrator()                 { return m_pCalibrator; }
    QString         getPortName()                   { if (m_pCalibrator == nullptr) return ""; return m_pCalibrator->getPortName(); }

    bool            calibratorIsConnected();

    bool            setUnit(int mode, int unit);
    void            setValue(double value);
    void            stepDown();
    void            stepUp();

    void            getValue();

    bool            isReady()                       { return m_ready; }

    void            loadSettings();
    void            saveSettings();

private:

    Calibrator*     m_pCalibrator = nullptr;
    int             m_index = -1;                                                           // index calibrator in a common base calibrators CalibratorBase

    bool            m_ready = false;

    // Elements of interface - Menu
    //
    QWidget*        m_parentWidget = nullptr;
    QDialog*        m_pDialog = nullptr;

    QFont*          m_pFont;
    QLabel*         m_pMeasureLabel = nullptr;
    QLineEdit*      m_pMeasureEdit = nullptr;
    QLabel*         m_pSourceLabel = nullptr;
    QLineEdit*      m_pSourceEdit = nullptr;
    QLineEdit*      m_pValueEdit = nullptr;
    QPushButton*    m_pSetValueButton = nullptr;
    QPushButton*    m_pStepDownButton = nullptr;
    QPushButton*    m_pStepUpButton = nullptr;
    QComboBox*      m_pModeList = nullptr;
    QComboBox*      m_pUnitList = nullptr;
    QPushButton*    m_pErrorsButton = nullptr;
    QCheckBox*      m_pRemoteControlCheck = nullptr;

    QDialog*        m_pErrorDialog = nullptr;
    QTextEdit*      m_pErrorList = nullptr;

    void            createDialog();
    void            initDialog();
    void            enableInterface(bool enable);
    void            updateValue();

signals:

    void            calibratorSetUnit(int mode, int unit);
    void            calibratorSetValue(double value);
    void            calibratorStepDown();
    void            calibratorStepUp();
    void            calibratorGetValue();
    void            calibratorRemoveControl(bool enable);

private slots:

    void            onCalibratorError(QString text);

    void            onCalibratorConnect();
    void            onCalibratorDisconnect();

    void            onUnitChanged();
    void            onValueChanging();
    void            onValueChanged();

    void            onSetValue();
    void            onStepDown();
    void            onStepUp();

    void            onModeUnitList(int);

    void            onErrorList();

    void            onRemoveControl();
};

// ==============================================================================================

class CalibratorManagerList
{
private:

    QMutex                      m_mutex;

    QList<CalibratorManager*>   m_list;

public:

    int                         count();

    int                         append(CalibratorManager* pManager);
    CalibratorManager*          at(int index);
    bool                        removeAt(int index);

    void                        clear();
};

// ==============================================================================================

#endif // CALIBRATORMANAGER_H
