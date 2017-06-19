#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

static void setWindowPosition(QWidget* window, QString savedGeometryKey)
{
	if (window == nullptr)
	{
		return;
	}
	QRect screenRect = QApplication::desktop()->screenGeometry(window);
	QPoint center = screenRect.center();

	QRect baseWindowRect = screenRect;
	baseWindowRect.setSize(QSize(screenRect.width() * 2 / 3, screenRect.height() * 2 / 3));
	baseWindowRect.moveCenter(center);

	QSettings settings;
	QRect windowRect = settings.value(savedGeometryKey, baseWindowRect).toRect();

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
	if (windowRect.left() + windowRect.width() > screenRect.width())
	{
		windowRect.moveLeft(screenRect.width() - windowRect.width());
	}

	if (windowRect.top() < 0)
	{
		windowRect.moveTop(0);
	}
	if (windowRect.top() + windowRect.height() > screenRect.height())
	{
		windowRect.moveTop(screenRect.height() - windowRect.height());
	}

	window->setGeometry(windowRect);
}

#endif // WIDGETUTILS_H
