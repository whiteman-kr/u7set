#ifndef CALIBRATORMANAGERDIALOG_H
#define CALIBRATORMANAGERDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include "Calibrator.h"

// ==============================================================================================

namespace Ui
{
    class CalibratorManagerDialog;
}

// ==============================================================================================

class CalibratorManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibratorManagerDialog(Calibrator* pCalibrator, QWidget *parent = 0);
    ~CalibratorManagerDialog();

private:

    Calibrator* m_pCalibrator = nullptr;

    QTextEdit*  m_pErrorList = nullptr;

    void        enableItemCtrl(bool enable);

private:

    Ui::CalibratorManagerDialog *ui;

signals:

    void		calibratorSetUnit(int mode, int unit);
    void        calibratorSetValue(double value);
    void        calibratorGetValue();
    void        calibratorReset(int resetType);
    void        calibratorRemoveControl(bool enable);

private slots:

    void        onCalibratorError(QString err);


    void        onConnect();
    void        onDisconnect();

    void        unitIsChanged();
    void        changingValue();
    void        updateValue();

    void        modeList(int mode);
    void        unitList(int unit);

    void        setValue();
    void        reset();

    void        removeControl();

    void        showErrorList();
};

// ==============================================================================================

#endif // CALIBRATORMANAGERDIALOG_H
