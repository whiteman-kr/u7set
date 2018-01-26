#include "Stable.h"
#include "CentralWidget.h"
#include "MainTabPage.h"
#include "UploadTabPage.h"

CentralWidget::CentralWidget(QWidget* parent) :
	QTabWidget(parent)
{
	QSize sz = fontMetrics().size(Qt::TextSingleLine, "APPLICATION LOGIC");
	sz.setHeight(sz.height() * 1.75);

	QString ss = QString("QTabBar::tab { min-width: %1px; min-height: %2px;}").arg(sz.width()).arg(sz.height());
	setStyleSheet(ss);

	connect(this, &QTabWidget::currentChanged, this, &CentralWidget::currentChanged);
}

int CentralWidget::addTabPage(MainTabPage* tabPage, const QString& label)
{
	assert(tabPage != nullptr);
	return addTab(tabPage, label);
}

void CentralWidget::currentChanged(int index)
{
	Q_UNUSED (index);

	QWidget* w = currentWidget();
	if (w == nullptr)
	{
		return;
	}

	// refresh builds list for uploading page
	//
	UploadTabPage* uploadPage = dynamic_cast<UploadTabPage*>(w);
	if (uploadPage != nullptr)
	{
		if (uploadPage->isUploading() == false)
		{
			uploadPage->refreshProjectBuilds();
		}
	}

}

void CentralWidget::switchToTabPage(QWidget* w)
{
	int index = indexOf(w);
	if (index == -1)
	{
		assert(index != -1);
		return;
	}

	setCurrentIndex(index);

	return;
}
