#ifndef TRENDSMAINWINDOW_H
#define TRENDSMAINWINDOW_H

#include <vector>

#include <QMainWindow>

#include "TrendSignal.h"

class QComboBox;

namespace Ui
{
	class TrendsMainWindow;
}

namespace TrendLib
{
	class Trend;
	class TrendWidget;
	class TrendSlider;
	class TrendSignalSet;

	class TrendMainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit TrendMainWindow(QWidget* parent = nullptr);
		virtual ~TrendMainWindow();

		// Methods
		//
	public:
		void ensureVisible();

		bool addSignals(const std::vector<TrendSignalParam>& trendSignals, bool redraw);
		bool addSignal(const TrendSignalParam& trendSignal, bool redraw);

		/// Add, remove or reorder signals.
		///
		void updateSignals(const std::vector<TrendSignalParam>& trendsignals);

		// Slider methods
		//
		bool isTimeInRange(const TimeStamp& value) const;

	protected:
		void createToolBar();

		QStatusBar* statusBar();

		void saveWindowState();
		void restoreWindowState();

		void setRealtimeAutoShift(const TimeStamp& ts);

		void autoSelectScaleType(const std::vector<TrendLib::TrendSignalParam>& acceptedSignals);

		// Events
		//
	protected:
		virtual void closeEvent(QCloseEvent*e) override;
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void showEvent(QShowEvent*) override;

	protected slots:
		virtual void signalsButton();
		void updateWidget();

		void signalProperties(QString appSignalId, QString archiveServerId);

		void actionOpenTriggered();
		void actionSaveTriggered();
		void actionPrintTriggered();
		void actionExitTriggered();
		void actionAboutTriggered();
		void actionRefreshTriggered();
		void actionAutoScaleTriggered();

		void actionAddRuler(QPoint mousePos);
		void actionDeleteRuler(int rulerIndex);
		void actionRulerProperties(int rulerIndex);

		void timeComboCurrentIndexChanged(int index);
		void viewComboCurrentIndexChanged(int index);
		void scaleTypeComboCurrentIndexChanged(int index);
		void laneCountComboCurrentIndexChanged(int index);
		void timeTypeComboCurrentIndexChanged(int index);
		void realtimeModeToggled(bool state);
		void realtimeAutoShiftClicked(bool state);

		void sliderValueChanged(TimeStamp value);

		void startTimeChanged(TimeStamp value);
		void durationChanged(qint64 value);

		void contextMenuRequested(const QPoint& pos);

	signals:
		void trendModeChanged();

		// Properties
		//
	protected:
		[[nodiscard]] TrendLib::Trend& trend();
		[[nodiscard]] const TrendLib::Trend& trend() const;

	public:
		[[nodiscard]] TrendLib::TrendSignalSet& signalSet();
		[[nodiscard]] const TrendLib::TrendSignalSet& signalSet() const;

		E::TrendViewMode viewMode() const;
		void setViewMode(E::TrendViewMode value);

		E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		int laneCount() const;
		void setLaneCount(int value);

		E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		TimeStamp startTime() const;
		void setStartTime(const TimeStamp& startTime);

		TimeStamp finishTime() const;

		qint64 duration() const;
		void setLaneDuration(qint64 interval);

		[[nodiscard]] E::TrendMode trendMode() const;
		void setTrendMode(E::TrendMode value);

		[[nodiscard]] bool isRealtimeAutoShift() const;

		quint64 rulerStep() const;
		void setRulerStep(quint64 value);

	protected:
		Ui::TrendsMainWindow *ui;

		QToolBar* m_toolBar = nullptr;
		QComboBox* m_timeCombo = nullptr;
		QComboBox* m_viewCombo = nullptr;
		QComboBox* m_scaleTypeCombo = nullptr;
		QComboBox* m_lanesCombo = nullptr;
		QComboBox* m_timeTypeCombo = nullptr;

		QPushButton* m_realtimeModeButton = nullptr;
		QPushButton* m_realtimeAutoShiftButton = nullptr;
		QAction* m_realtimeActionForButton = nullptr;	// Watch the following comment for m_refreshActionForButton!

		QPushButton* m_refreshButton = nullptr;
		QAction* m_refreshActionForButton = nullptr;	// When m_refreshButton is added to ToolBar m_refreshActionForButton is returned value.
														// Action is used for hiding Button from ToolBar, as hiding QWidget does not work
														// Qt Help for QToolBar::addWidget says:
														// Note: You should use QAction::setVisible() to change the visibility of the widget.
														// Using QWidget::setVisible(), QWidget::show() and QWidget::hide() does not work.

		QPushButton* m_signalsButton = nullptr;

		QAction* m_refreshAction = nullptr;

		TimeStamp m_lastRealtimeMaxValue;

		TrendSlider* m_trendSlider = nullptr;

	protected:
		TrendLib::TrendWidget* m_trendWidget = nullptr;

		static const int singleStepSliderDivider = 50;

	private:
		bool m_autoSelectedScaleType = false;
	};

}

#endif // TRENDSMAINWINDOW_H
