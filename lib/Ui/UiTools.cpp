#include "../lib/Ui/UiTools.h"

#include <QDialog>
#include <QDesktopWidget>
#include <QDateTime>
#include <QDesktopServices>

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

void UiTools::openPdf(const QString& file, QWidget* parent)
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
