#include "Stable.h"
#include "CentralWidget.h"
#include "MainTabPage.h"

CentralWidget::CentralWidget(QWidget* parent) :
	QTabWidget(parent)
{
}

int CentralWidget::addTabPage(MainTabPage* tabPage, const QString& label)
{
	assert(tabPage != nullptr);
	return addTab(tabPage, label);
}
