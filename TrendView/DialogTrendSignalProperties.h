#ifndef DIALOGTRENDSIGNALPROPERTIES_H
#define DIALOGTRENDSIGNALPROPERTIES_H

#include <QDialog>
#include "TrendSignal.h"

namespace Ui {
	class DialogTrendSignalProperties;
}

class DialogTrendSignalProperties : public QDialog
{
	Q_OBJECT

public:
	DialogTrendSignalProperties(const TrendLib::TrendSignalParam& trendSignal, QWidget* parent);
	~DialogTrendSignalProperties();

	const TrendLib::TrendSignalParam& trendSignal() const;		// result

public slots:
	virtual void accept() override;

private:
	Ui::DialogTrendSignalProperties *ui;

	TrendLib::TrendSignalParam m_trendSignal;
};

class ChooseColorWidget : public QLabel
{
public:
	explicit ChooseColorWidget(QWidget* parent = nullptr);

	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;

	QColor color() const;
	void setColor(QColor value);

private:
	QColor m_color = Qt::black;
};

#endif // DIALOGTRENDSIGNALPROPERTIES_H
