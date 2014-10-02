#ifndef OPTIONSPOINTSDIALOG_H
#define OPTIONSPOINTSDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>

#include "Options.h"

// ==============================================================================================


const char* const PointsColumn[] =
{
            QT_TRANSLATE_NOOP("OptionsPointsDialog.h", "%"),
            LinearityPointSensor[POINT_SENSOR_I_0_5_MA],
            LinearityPointSensor[POINT_SENSOR_I_4_20_MA],
};

const int   PointsColumnCount       = sizeof(PointsColumn)/sizeof(char*);

const int   PointsColumn_Percent    = 0,
            PointsColumn_0_5mA      = 1,
            PointsColumn_4_20mA     = 2;

// ==============================================================================================

class OptionsPointsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsPointsDialog(const LinearityOption& linearity, QWidget *parent = 0);
    ~OptionsPointsDialog();

    LinearityOption     m_linearity;

private:

    // elements of interface
    //
    QLabel*             m_rangeTypeLabel = nullptr;
    QComboBox*          m_rangeTypeList = nullptr;

    QLabel*             m_pointCountLabel = nullptr;
    QLineEdit*          m_pointCountEdit = nullptr;
    QLabel*             m_lowRangeLabel = nullptr;
    QLineEdit*          m_lowRangeEdit = nullptr;
    QLabel*             m_highRangeLabel = nullptr;
    QLineEdit*          m_highRangeEdit = nullptr;


    QPushButton*        m_addButton = nullptr;
    QPushButton*        m_editButton = nullptr;
    QPushButton*        m_removeButton = nullptr;
    QPushButton*        m_upButton = nullptr;
    QPushButton*        m_downButton = nullptr;


    QTableWidget*       m_pointList = nullptr;

    bool                m_updatingList;

    void                SetHeaderList();
    void                updateRangeType();
    void                updateList();
    void                clearList();


protected:

    void                keyPressEvent(QKeyEvent* e);
    void                showEvent(QShowEvent* e);

signals:

    void                updateLinearityPage(bool isDialog);

private slots:

    void                onAddPoint();
    void                onEditPoint();
    void                onRemovePoint();
    void                onUpPoint();
    void                onDownPoint();
    void                onRangeType(int type);
    void                onAutomaticCalculatePoints();

    void                cellChanged(int,int);
    void                currentCellChanged(int,int,int,int);
};

// ==============================================================================================

#endif // OPTIONSPOINTSDIALOG_H
