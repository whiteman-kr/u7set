#ifndef DELEGATE_H
#define DELEGATE_H

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QPainter>

// ==============================================================================================

class IntDelegate : public QItemDelegate
{
	Q_OBJECT

public:

	IntDelegate(QObject* parent);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const override;
};

// ==============================================================================================

class DoubleDelegate : public QItemDelegate
{
	Q_OBJECT

public:

	DoubleDelegate(QObject* parent);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const override;
};

// ==============================================================================================

class ColorDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	ColorDelegate(QObject* parent);

	void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// ==============================================================================================

class FindTextDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	FindTextDelegate(QObject* parent);

	void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// ==============================================================================================

#endif // DELEGATE_H
