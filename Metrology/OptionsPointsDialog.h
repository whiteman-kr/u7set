#ifndef OPTIONSPOINTSDIALOG_H
#define OPTIONSPOINTSDIALOG_H

#include <QDialog>
#include <QMenu>
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

class OptionsPointsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsPointsDialog(const LinearityOption& linearity, QWidget *parent = 0);
    ~OptionsPointsDialog();

    LinearityOption     m_linearity;

private:

    QAction*            m_pAction[POINT_SENSOR_COUNT];
    QMenu*              m_headerContextMenu = nullptr;

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

    bool                m_updatingList = false;

    void                SetHeaderList();
    void                updateRangeType();
    void                updateList();
    void                clearList();

    void                hideColumn(int column, bool hide);

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

    void                onHeaderContextMenu(QPoint);
    void                onAction(QAction* action);
};

// ==============================================================================================

#endif // OPTIONSPOINTSDIALOG_H
