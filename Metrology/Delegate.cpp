#include "Delegate.h"

#include "FindMeasurePanel.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

IntDelegate::IntDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

QWidget * IntDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);

    editor->setValidator(new QIntValidator(editor));

    return editor;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DoubleDelegate::DoubleDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

QWidget * DoubleDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);

    editor->setValidator(new QRegExpValidator(QRegExp("^[-]{0,1}[0-9]*[.]{0,1}[0-9]*$"),editor));

    return editor;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ColorDelegate::ColorDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void ColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect colorRect = option.rect;
    colorRect.adjust(2,2,-2,-2);
    colorRect.setWidth(option.rect.height() - 4);

    QRect textRect = option.rect;
    textRect.adjust(option.rect.height() + 4,1,0,-1);

    if (index.data().canConvert<QColor>())
        {
            QColor color = qvariant_cast<QColor>(index.data(Qt::UserRole));

            if ((option.state & QStyle::State_Selected) != 0)
            {
                if ((option.state & QStyle::State_HasFocus) != 0)
                {
                    painter->fillRect(option.rect, option.palette.highlight());
                }
                else
                {
                    painter->fillRect(option.rect, option.palette.window());
                }
            }

            painter->fillRect(colorRect, color);

            painter->setPen( color == Qt::white ? Qt::lightGray : color);
            painter->drawRect(colorRect);

            painter->setRenderHint(QPainter::TextAntialiasing );
            painter->setPen(option.palette.text().color());
            painter->drawText(textRect, Qt::AlignLeft, QString("[%1, %2, %3]").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindTextDelegate::FindTextDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void FindTextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect textRect = option.rect;
    QRect selectTextRect;
    textRect.adjust(6, 1, 0, -1);

    FindItem item = qvariant_cast<FindItem>(index.data(Qt::UserRole));

    if ((option.state & QStyle::State_Selected) != 0)
    {
//        if ((option.state & QStyle::State_HasFocus) != 0)
//        {
            painter->fillRect(option.rect, option.palette.highlight());
//        }
//        else
//        {
//            painter->fillRect(option.rect, option.palette.window());
//        }
    }

    QString offerText = item.text().left(item.beginPos());
    QString selectText = item.text().mid(item.beginPos(), item.endPos() - item.beginPos());

    QSize offerTextSize = option.fontMetrics.size(Qt::TextSingleLine, offerText);
    QSize selectTextSize = option.fontMetrics.size(Qt::TextSingleLine, selectText);

    selectTextRect.setRect(option.rect.left() + offerTextSize.width() + 6, option.rect.top() + 2, selectTextSize.width() + 1, selectTextSize.height() - 2);
    painter->fillRect(selectTextRect, QColor(0xFF, 0xF0, 0x0F));

    painter->setRenderHint(QPainter::TextAntialiasing );
    painter->setPen(option.palette.text().color());
    painter->drawText(textRect, Qt::AlignLeft, item.text());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
