#include "CentralWidget.h"
#include "UploadTabPage.h"


CentralWidget::CentralWidget(QWidget* parent) :
	UiLib::TabWidgetEx(parent)
{
	QSize sz = fontMetrics().size(Qt::TextSingleLine, "APPLICATION LOGIC");
	tabBar()->setStyleSheet(QString("QTabBar::tab{ min-width: %1px;}").arg(sz.width()));

	tabBarEx()->setTopStyle(UiLib::TabBarEx::Style::TopLineRoundedAlways);

	setTabsClosable(false);
	setMovable(false);

	QRgb colorBlue = qRgba(0, 108, 190, 220);
	QRgb colorOrange = qRgba(226, 67, 41, 220);
	QRgb colorGreen = qRgba(107, 160, 43, 220);

	std::map<QString, QRgb> tabColors = {
		{"Equipment", colorBlue},
		{"Application Signals", colorBlue},
		{"Files", colorBlue},
		{"Schemas", colorBlue},
		{"Build", colorOrange},
		{"Simulator", colorGreen},
		{"Tests", colorGreen},
		{"Upload", colorGreen},
	};

	tabBarEx()->setTabColors(std::move(tabColors));

	connect(this, &QTabWidget::currentChanged, this, &CentralWidget::currentChanged);
}

int CentralWidget::addTabPage(MainTabPage* tabPage, const QString& label)
{
	assert(tabPage != nullptr);
	return addTab(tabPage, label);
}

void CentralWidget::currentChanged(int index)
{
	Q_UNUSED(index);

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

	return;
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
