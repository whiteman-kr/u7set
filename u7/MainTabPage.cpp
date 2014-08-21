#include "MainTabPage.h"
#include "../include/DbController.h"

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
