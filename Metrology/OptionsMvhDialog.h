#ifndef OPTIONSMEASUREVIEWHEADERDIALOG_H
#define OPTIONSMEASUREVIEWHEADERDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>

#include "Options.h"

// ==============================================================================================

const char* const       MvhColumn[] =
{
                        QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Title"),
                        QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Visible"),
                        QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Width"),
                        QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Color"),
                        QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Bold font"),
};

const int               MVH_COLUMN_COUNT    = sizeof(MvhColumn)/sizeof(char*);

const int               MVH_COLUMN_TITLE    = 0,
                        MVH_COLUMN_VISIBLE  = 1,
                        MVH_COLUMN_WIDTH    = 2,
                        MVH_COLUMN_COLOR    = 3,
                        MVH_COLUMN_BOLD     = 4;

// ----------------------------------------------------------------------------------------------

const int               MvhColumnWidth[MVH_COLUMN_COUNT] =
{
                        200,
                        90,
                        90,
                        110,
                        90,
};

// ==============================================================================================

class OptionsMeasureViewHeaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit            OptionsMeasureViewHeaderDialog(const MeasureViewOption& header, QWidget *parent = 0);
                        ~OptionsMeasureViewHeaderDialog();

    int                 m_measureType = MEASURE_TYPE_LINEARITY;
    MeasureViewOption   m_header;

private:

    // elements of interface
    //
    QLabel*             m_measureTypeLabel = nullptr;
    QComboBox*          m_measureTypeList = nullptr;

    QTableWidget*       m_columnList = nullptr;

    bool                m_updatingList = false;

    void                setHeaderList();
    void                updateList();
    void                clearList();

    void                onMeasureType(int type);

protected:

    void                keyPressEvent(QKeyEvent* e);
    void                showEvent(QShowEvent* e);

signals:

    void                updateMeasureViewPage(bool isDialog);

private slots:

    void                cellChanged(int,int);
    void                currentCellChanged(int,int,int,int);

    void                onEdit(int row, int column);
};

// ==============================================================================================

#endif // OPTIONSMEASUREVIEWHEADERDIALOG_H
