#ifndef CALIBRATORMANAGERDIALOG_H
#define CALIBRATORMANAGERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>

#include "Calibrator.h"

// ==============================================================================================

class CalibratorManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibratorManagerDialog(Calibrator* pCalibrator, QWidget *parent = 0);
    ~CalibratorManagerDialog();

private:

    Calibrator*     m_pCalibrator = nullptr;

    // Elements of interface - Menu
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

    void            createInterfaceItems();
    void            initInterfaceItems();
    void            enableInterfaceItems(bool enable);

signals:

    void            calibratorSetUnit(int mode, int unit);
    void            calibratorSetValue(double value);
    void            calibratorGetValue();
    void            calibratorRemoveControl(bool enable);

private slots:

    void            onCalibratorError(QString err);

    void            onConnect();
    void            onDisconnect();

    void            onUnitChanged();
    void            onValueChanging();
    void            onValueChanged();

    void            onSetValue();

    void            onModeList(int mode);
    void            onUnitList(int unit);

    void            onErrorList();

    void            onRemoveControl();
};

// ==============================================================================================

#endif // CALIBRATORMANAGERDIALOG_H
