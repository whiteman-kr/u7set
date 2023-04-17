#pragma once

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel 
{ 
	Q_OBJECT

public:
	explicit ClickableLabel(QString text, QWidget* parent = nullptr) :
		QLabel(text, parent)
	{
	}

protected:
	void mousePressEvent(QMouseEvent* /*event*/)
	{
		emit clicked();
	}

signals:
	void clicked();
};

