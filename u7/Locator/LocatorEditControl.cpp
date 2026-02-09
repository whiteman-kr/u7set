#include "LocatorEditControl.h"
#include "../MainWindow.h"

namespace Locator
{
	LocatorEditControl::LocatorEditControl(Locator& locator, LocatorListWidget& listWidget, MainWindow* appMainWindow) :
		QLineEdit{nullptr},
		m_locator{locator},
		m_listWidget{listWidget},
		m_appMainWindow{appMainWindow}
	{
		// --
		//
		setPlaceholderText(tr("Type to locate (Ctrl + K)"));

		m_findIcon.setIcon(QIcon{":/Images/Images/LocatorFind.svg"});
		addAction(&m_findIcon, QLineEdit::LeadingPosition);

		setClearButtonEnabled(true);

		connect(this, &QLineEdit::textChanged, this, &LocatorEditControl::slot_textChanged);

		connect(&listWidget, &LocatorListWidget::clearFocusFromInput, this, &LocatorEditControl::slot_clearFocusFromInput);

		// Start time for updating last widget in focus
		//
		m_focusTimerId = startTimer(100);
		return;
	}

	LocatorEditControl::~LocatorEditControl()
	{
		killTimer(m_focusTimerId);
	}

	void LocatorEditControl::timerEvent(QTimerEvent* event)
	{
		QObject::timerEvent(event);

		QWidget* currentWidget = QApplication::focusWidget();

		if (currentWidget == this || currentWidget == m_focusCameFrom)
		{
			return;
		}

		if (m_focusCameFrom != nullptr)
		{
			// We do not need to track live of object m_focusCameFrom as we about to change it to the diffeternt one.
			//
			disconnect(m_focusCameFrom, nullptr, this, nullptr);
		}

		m_focusCameFrom = currentWidget;

		if (m_focusCameFrom != nullptr)
		{
			// Subscribe for destroy, to set m_focusCameFrom if the object was destroyed, so we will not focus in on ESC
			// for already destroyed object.
			//
			connect(m_focusCameFrom,
					&QObject::destroyed,
					this,
					[this]()
					{
						m_focusCameFrom = nullptr;
					});
		}
	}

	void LocatorEditControl::keyPressEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key_Escape)
		{
			clearFocus();

			if (m_focusCameFrom != nullptr)
			{
				m_focusCameFrom->setFocus();
			}
		}

		return QLineEdit::keyPressEvent(event);
	}

	void LocatorEditControl::focusInEvent(QFocusEvent* event)
	{
		QLineEdit::focusInEvent(event);

		QPoint listControlPos{0,
							  m_appMainWindow->height() - m_listWidget.height() - m_appMainWindow->statusBar()->frameGeometry().height()};

		slot_textChanged(text());

		m_listWidget.setParent(nullptr);
		m_listWidget.setParent(m_appMainWindow); // Repareting widget will rise it to the top in the drawing chain.

		m_listWidget.move(listControlPos);
		m_listWidget.showList();

		return;
	}

	void LocatorEditControl::focusOutEvent(QFocusEvent* event)
	{
		QLineEdit::focusOutEvent(event);
		m_listWidget.hideList();

		m_locator.stopSearching();
		return;
	}

	void LocatorEditControl::slot_textChanged(const QString& text)
	{
		m_locator.setText(text.trimmed());
	}

	void LocatorEditControl::slot_clearFocusFromInput()
	{
		clearFocus();
	}
} // namespace Locator
