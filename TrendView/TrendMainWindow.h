#ifndef TRENDSMAINWINDOW_H
#define TRENDSMAINWINDOW_H

#include <QMainWindow>
#include "TrendSlider.h"
#include "../TrendView/TrendSignal.h"
#include "../TrendView/TrendDrawParam.h"

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

	protected:
		void createToolBar();

		void saveWindowState();
		void restoreWindowState();

		// Events
		//
	protected:
		virtual void closeEvent(QCloseEvent*e) override;
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void showEvent(QShowEvent*) override;

	private slots:
		void actionOpenTriggered();
		void actionSaveTriggered();
		void actionPrintTriggered();
		void actionExitTriggered();
		void actionAboutTriggered();

		void timeComboCurrentIndexChanged(int index);
		void viewComboCurrentIndexChanged(int index);
		void laneCountComboCurrentIndexChanged(int index);

		void sliderValueChanged(TimeStamp value);

	private:
		Ui::TrendsMainWindow *ui;

		QToolBar* m_toolBar = nullptr;
		QComboBox* m_timeCombo = nullptr;
		QComboBox* m_viewCombo = nullptr;
		QComboBox* m_lanesCombo = nullptr;

		TrendLib::TrendSignalSet m_signalSet;
		TrendLib::TrendWidget* m_trendWidget = nullptr;
		TrendSlider* m_trendSlider = nullptr;

		static const int singleStepSliderDivider = 50;
	};

}

#endif // TRENDSMAINWINDOW_H
