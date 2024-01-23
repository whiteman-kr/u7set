#include "MainTabPage.h"
#include "GlobalMessanger.h"

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

const DbController* MainTabPage::dbController() const
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

DbController* MainTabPage::db()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

const DbController* MainTabPage::db() const
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

