#include "MainTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"

MainTabPage::MainTabPage(DbController* dbcontroller, QWidget* parent) :
	QWidget(parent),
	m_dbController(dbcontroller)
{
	assert(m_dbController != nullptr);
}

DbController* MainTabPage::dbController()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

DbController* MainTabPage::db()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

GlobalMessanger* MainTabPage::globalMesssanger()
{
	return GlobalMessanger::instance();
}
