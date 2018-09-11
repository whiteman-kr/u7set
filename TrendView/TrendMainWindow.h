#ifndef TRENDSMAINWINDOW_H
#define TRENDSMAINWINDOW_H

#include <QMainWindow>
#include "TrendSlider.h"
#include "../TrendView/Trend.h"

class QComboBox;

namespace Ui
{
	class TrendsMainWindow;
}

namespace TrendLib
{
	class TrendWidget;

	class TrendMainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit TrendMainWindow(QWidget* parent = 0);
		virtual ~TrendMainWindow();

		// Methods
		//
	public:
		void ensureVisible();

		bool addSignals(const std::vector<TrendSignalParam>& trendSignals, bool redraw);
		bool addSignal(const TrendSignalParam& trendSignal, bool redraw);

	protected:
		void createToolBar();

		QStatusBar* statusBar();

		void saveWindowState();
		void restoreWindowState();

		// Events
		//
	protected:
		virtual void closeEvent(QCloseEvent*e) override;
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void showEvent(QShowEvent*) override;

		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;

	protected slots:
		virtual void signalsButton();
		void updateWidget();

	private slots:
		void signalProperties(QString appSignalId);

		void actionOpenTriggered();
		void actionSaveTriggered();
		void actionPrintTriggered();
		void actionExitTriggered();
		void actionAboutTriggered();
		void actionRefreshTriggered();
		void actionAutoSclaeTriggered();

		void actionAddRuler(QPoint mousePos);
		void actionDeleteRuler(int rulerIndex);
		void actionRulerProperties(int rulerIndex);

		void timeComboCurrentIndexChanged(int index);
		void viewComboCurrentIndexChanged(int index);
		void laneCountComboCurrentIndexChanged(int index);
		void timeTypeComboCurrentIndexChanged(int index);
		void realtimeModeToggled(bool state);

		void sliderValueChanged(TimeStamp value);

		void startTimeChanged(TimeStamp value);
		void durationChanged(qint64 value);

		void contextMenuRequested(const QPoint& pos);

		// Proprties
		//
	public:
		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		E::TrendMode trendMode() const;
		void setTrendMode(E::TrendMode value);

	private:
		Ui::TrendsMainWindow *ui;

		QToolBar* m_toolBar = nullptr;
		QComboBox* m_timeCombo = nullptr;
		QComboBox* m_viewCombo = nullptr;
		QComboBox* m_lanesCombo = nullptr;
		QComboBox* m_timeTypeCombo = nullptr;
		QPushButton* m_realtimeModeButton = nullptr;
		QPushButton* m_refreshButton = nullptr;
		QPushButton* m_signalsButton = nullptr;

		QAction* m_refreshAction = nullptr;

	protected:
		TrendLib::TrendWidget* m_trendWidget = nullptr;
		TrendSlider* m_trendSlider = nullptr;

		static const int singleStepSliderDivider = 50;
	};

}

#endif // TRENDSMAINWINDOW_H
