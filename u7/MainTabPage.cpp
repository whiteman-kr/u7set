#include "MainTabPage.h"
#include "../include/DbStore.h"

MainTabPage::MainTabPage(DbStore* dbstore, QWidget* parent) :
	QWidget(parent),
	m_pDbStore(dbstore)
{
	assert(m_pDbStore != nullptr);
}

DbStore* MainTabPage::dbStore()
{
	assert(m_pDbStore != nullptr);
	return m_pDbStore;
}
