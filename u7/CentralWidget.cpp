#include "Stable.h"
#include "CentralWidget.h"
#include "MainTabPage.h"

CentralWidget::CentralWidget(QWidget* parent) :
	QTabWidget(parent)
{
}

void CentralWidget::addTabPage(MainTabPage* tabPage, const QString& label)
{
	assert(tabPage != nullptr);

	addTab(tabPage, label);
    return;
}
