#include "../lib/Ui/UiTools.h"

#include <QDialog>
#include <QDesktopWidget>
#include <QDateTime>
#include <QDesktopServices>

QIcon UiTools::drawColorBox(QColor color)
{
	const QStyle* style = QApplication::style();
	// Figure out size of an indicator and make sure it is not scaled down in a list view item
	// by making the pixmap as big as a list view icon and centering the indicator in it.
	// (if it is smaller, it can't be helped)
	const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth);
	const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight);
	const int listViewIconSize = indicatorWidth;
	const int pixmapWidth = indicatorWidth;
	const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

	QRect rect = QRect(0, 0, indicatorWidth - 1, indicatorHeight - 1);
	QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
	pixmap.fill(Qt::transparent);
	{
		// Center?
		const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
		const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
		QPainter painter(&pixmap);
		painter.translate(xoff, yoff);

		painter.setPen(QColor(Qt::black));
		painter.setBrush(QBrush(color));
		painter.drawRect(rect);
	}
	return QIcon(pixmap);
}

void UiTools::adjustDialogPlacement(QDialog* dialog)
{
	if (dialog == nullptr)
	{
		assert(dialog);
		return;
	}

	QRect windowRect = QRect(dialog->pos(), dialog->frameSize());

	QRect desktopRect = QApplication::desktop()->availableGeometry(dialog);

	if (windowRect.left() < desktopRect.left())
	{
		windowRect.moveLeft(desktopRect.left());
		dialog->move(windowRect.topLeft());
	}

	if (windowRect.top() < desktopRect.top())
	{
		windowRect.moveTop(desktopRect.top());
		dialog->move(windowRect.topLeft());
	}

	if (windowRect.right() > desktopRect.right())
	{
		windowRect.moveRight(desktopRect.right());
		dialog->move(windowRect.topLeft());
	}

	if (windowRect.bottom() > desktopRect.bottom())
	{
		windowRect.moveBottom(desktopRect.bottom());
		dialog->move(windowRect.topLeft());
	}
}

void UiTools::openHelp(const QString& file, QWidget* parent)
{
	QFile f(file);
	if (f.exists() == true)
	{
		QUrl url = QUrl::fromLocalFile(file);
		QDesktopServices::openUrl(url);
	}
	else
	{
		QMessageBox::critical(parent, qAppName(), QObject::tr("Help file '%1' does not exist!").arg(file));
	}
}
