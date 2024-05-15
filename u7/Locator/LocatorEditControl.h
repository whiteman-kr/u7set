#pragma once

#include "Locator.h"
#include "LocatorListWidget.h"

class MainWindow;

namespace Locator
{
	class LocatorEditControl : public QLineEdit
	{
		Q_OBJECT

	public:
		explicit LocatorEditControl(Locator& locator, LocatorListWidget& listWidget, MainWindow* appMainWindow);

	protected:
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void focusInEvent(QFocusEvent* event) override;
		virtual void focusOutEvent(QFocusEvent* event) override;

	private slots:
		void slot_textChanged(const QString& text);
		void slot_clearFocusFromInput();

	private:
		Locator& m_locator;
		LocatorListWidget& m_listWidget;
		MainWindow* m_appMainWindow = nullptr;

		QWidget* m_focusCameFrom = nullptr;	// There is no way to get previos focus from focusInEvent, so this variable will be updated by timer.

		QAction m_findIcon{this};
	};
}
