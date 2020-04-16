#ifndef DIALOGTRENDSIGNALPROPERTIES_H
#define DIALOGTRENDSIGNALPROPERTIES_H

#include <QDialog>
#include "TrendSignal.h"
#include "TrendParam.h"

namespace Ui {
	class DialogTrendSignalProperties;
}

class DialogTrendSignalProperties : public QDialog
{
	Q_OBJECT

public:
	DialogTrendSignalProperties(const TrendLib::TrendSignalParam& trendSignal,
								TrendLib::TrendSignalSet* trendSignalSet,
								E::TimeType timeType,
								TrendLib::TrendScaleType scaleType,
								E::TrendMode trendMode,
								QWidget* parent);
	virtual ~DialogTrendSignalProperties();

	const TrendLib::TrendSignalParam& trendSignal() const;		// result

signals:
	void signalPropertiesChanged();	// emitted when user presses apply or OK button

public slots:
	virtual void accept() override;

private slots:
	void on_buttonPoints_clicked();
	void on_buttonApply_clicked();
	void on_viewFormatCombo_currentIndexChanged(const QString &text);

private:
	void fillProperties();
	bool applyProperties();

private:
	Ui::DialogTrendSignalProperties *ui;

	TrendLib::TrendSignalParam m_trendSignal;

	// Parameters needed for points dialog
	//
	TrendLib::TrendSignalSet* m_trendSignalSet = nullptr;
	QString m_appSignalId;
	E::TimeType m_timeType = E::TimeType::Plant;
	TrendLib::TrendScaleType m_scaleType = TrendLib::TrendScaleType::Linear;
	E::TrendMode m_trendMode = E::TrendMode::Archive;

	//
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
