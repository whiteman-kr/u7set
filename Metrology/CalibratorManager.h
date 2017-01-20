#ifndef CALIBRATORMANAGER_H
#define CALIBRATORMANAGER_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QMutex>

#include "Calibrator.h"
#include "ObjectVector.h"

// ==============================================================================================

class CalibratorManager : public QObject
{
    Q_OBJECT

public:
    explicit        CalibratorManager();
                    ~CalibratorManager();

private:

    int             m_index = -1;                    // index calibrator in a common base calibrators CalibratorBase
    Calibrator*     m_pCalibrator = nullptr;
    bool            m_ready = false;

public:

    int             index()                         { return m_index;   }
    void            setIndex(int index)             { m_index = index;  }

    void            setCalibrator(Calibrator* pCalibrator) { m_pCalibrator = pCalibrator;  }

    Calibrator*     calibrator()                    { return m_pCalibrator; }
    QString         portName()                      { if (m_pCalibrator == nullptr) return ""; return m_pCalibrator->portName(); }

    bool            isReady()                       { return m_ready; }

public:

    // elements of interface - Menu
    //
    QDialog*        m_pManageDialog = nullptr;

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

    void            createDialog(QWidget* parent);
    void            initDialog();
    void            enableInterface(bool enable);


public:

    void            showManageDialog();

    bool            calibratorIsConnected();

    // function for manage
    //

    bool            setUnit(int mode, int unit);

    void            updateValue();
    void            value();
    void            setValue(double value);

    void            stepDown();
    void            stepUp();

    // options
    //

    void            loadSettings();
    void            saveSettings();

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
    void            onModeUnitList(int);

    void            onValueChanging();
    void            onValueChanged();

    void            onSetValue();
    void            onStepDown();
    void            onStepUp();

    void            onErrorList();

    void            onRemoveControl();
};

// ==============================================================================================

typedef PtrObjectVector<CalibratorManager> CalibratorManagerList;

// ==============================================================================================

#endif // CALIBRATORMANAGER_H
