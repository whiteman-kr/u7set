#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>

static void saveWindowPosition(QWidget* window, QString widgetKey)
{
	QSettings settings;
	int screenNumber = QApplication::desktop()->screenNumber(window);
	settings.setValue(widgetKey + "/screenNumber", screenNumber);
	settings.setValue(widgetKey + "/geometry", window->geometry());
}

static void setWindowPosition(QWidget* window, QString widgetKey)
{
	if (window == nullptr)
	{
		return;
	}
	QSettings settings;
	int screenNumber = settings.value(widgetKey + "/screenNumber", QApplication::desktop()->screenNumber(window)).toInt();

	QRect screenRect = QApplication::desktop()->screenGeometry(screenNumber);
	QPoint center = screenRect.center();

	QRect baseWindowRect = screenRect;
	baseWindowRect.setSize(QSize(screenRect.width() * 2 / 3, screenRect.height() * 2 / 3));
	baseWindowRect.moveCenter(center);

	QRect windowRect = settings.value(widgetKey + "/geometry", baseWindowRect).toRect();

	if (windowRect.height() > screenRect.height())
	{
		windowRect.setHeight(screenRect.height());
	}
	if (windowRect.width() > screenRect.width())
	{
		windowRect.setWidth(screenRect.width());
	}

	if (windowRect.left() < 0)
	{
		windowRect.moveLeft(0);
	}
	if (windowRect.left() + windowRect.width() > screenRect.right())
	{
		windowRect.moveLeft(screenRect.right() - windowRect.width());
	}

	if (windowRect.top() < 0)
	{
		windowRect.moveTop(0);
	}
	if (windowRect.top() + windowRect.height() > screenRect.bottom())
	{
		windowRect.moveTop(screenRect.bottom() - windowRect.height());
	}

	window->setGeometry(windowRect);
}

#endif // WIDGETUTILS_H
