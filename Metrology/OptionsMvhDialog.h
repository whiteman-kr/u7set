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
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QPainter>

#include "Options.h"

// ==============================================================================================

const char* const   MvhColumn[] =
{
                    QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Title"),
                    QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Visible"),
                    QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Width"),
                    QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Color"),
                    QT_TRANSLATE_NOOP("OptionsMvhDialog.h", "Bold font"),
};

const int           MVH_COLUMN_COUNT    = sizeof(MvhColumn)/sizeof(char*);

const int           MVH_COLUMN_TITLE    = 0,
                    MVH_COLUMN_VISIBLE  = 1,
                    MVH_COLUMN_WIDTH    = 2,
                    MVH_COLUMN_COLOR    = 3,
                    MVH_COLUMN_BOLD     = 4;

// ----------------------------------------------------------------------------------------------

const int           MvhColumnWidth[MVH_COLUMN_COUNT] =
{
                    200,
                    90,
                    90,
                    110,
                    90,
};

// ==============================================================================================

class IntDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    IntDelegate(QObject *parent) : QItemDelegate(parent) {}

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
    {
        QLineEdit *editor = new QLineEdit(parent);

        editor->setValidator(new QIntValidator(editor));
        return editor;
    }
};

// ==============================================================================================

class ColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ColorDelegate(QObject *parent) : QStyledItemDelegate(parent) { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QRect colorRect = option.rect;
        colorRect.adjust(2,2,-2,-2);
        colorRect.setWidth(option.rect.height() - 4);

        QRect textRect = option.rect;
        textRect.adjust(option.rect.height() + 4,1,0,-1);

//        if (index.data().canConvert<MeasureViewColumn>())
        {
              MeasureViewColumn column = qvariant_cast<MeasureViewColumn>(index.data(Qt::UserRole));
              QColor color = column.color();

              if (option.state & QStyle::State_Selected)
              {
                  painter->fillRect(option.rect, option.palette.highlight());
              }

              painter->fillRect(colorRect, color);

              painter->setPen( color == Qt::white ? Qt::lightGray : color);
              painter->drawRect(colorRect);

              painter->setRenderHint(QPainter::TextAntialiasing );
              painter->setPen(column.enableVisible() == false ? Qt::lightGray : option.palette.text().color());
              painter->drawText(textRect, Qt::AlignLeft, QString("[%1, %2, %3]").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
//        else
//        {
//           QStyledItemDelegate::paint(painter, option, index);
//        }


    }
};

// ==============================================================================================

class OptionsMeasureViewHeaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsMeasureViewHeaderDialog(const MeasureViewOption& header, QWidget *parent = 0);
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
