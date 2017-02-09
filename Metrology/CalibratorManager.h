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

// ==============================================================================================

class CalibratorManager : public QDialog
{
    Q_OBJECT

public:
    explicit        CalibratorManager(Calibrator* pCalibrator, QWidget* parent = 0);
                    ~CalibratorManager();

private:

    Calibrator*     m_pCalibrator = nullptr;
    bool            m_readyForManage = false;

public:

    Calibrator*     calibrator() const { return m_pCalibrator; }
    void            setCalibrator(Calibrator* pCalibrator) { m_pCalibrator = pCalibrator;  }

    int             index() const;
    QString         portName() const;

    bool            isReadyForManage() const { return m_readyForManage; }

public:

    // elements of interface - Menu
    //
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

    void            createManageDialog();
    void            initDialog();
    void            enableInterface(bool enable);

public:

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

    void            loadCalibratorSettings(Calibrator* pCalibrator);
    void            saveCalibratorSettings(Calibrator* pCalibrator);

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

#endif // CALIBRATORMANAGER_H
