#include "SimBasePage.h"
#include "SimLogicModulePage.h"
#include "SimConnectionPage.h"
#include "SimSelectSchemaPage.h"

std::list<SimBasePage*> SimBasePage::m_pages;


SimBasePage::SimBasePage(SimIdeSimulator* simulator, QWidget* parent)
	: QWidget(parent),
	  m_simulator(simulator)
{
	assert(m_simulator);

	setAttribute(Qt::WA_DeleteOnClose);

	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Window);

	m_closeAction = new QAction(tr("Close"), this);
	m_closeAction->setShortcut(Qt::CTRL + Qt::Key_W);	// QKeySequence::Close leads to CTRL+F4 somehow (((
	connect(m_closeAction, &QAction::triggered, this, &SimBasePage::deleteLater);
	addAction(m_closeAction);

	m_pages.push_back(this);
	this->setProperty("SimParentObject", QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent)));

	return;
}

SimBasePage::~SimBasePage()
{
	m_pages.remove(this);
}

void SimBasePage::deleteAllPages()
{
	auto listCopy = m_pages;		// m_pages is changed in destruct, so we need a copy
	for (SimBasePage* page : listCopy)
	{
		delete page;
	}

	assert(m_pages.empty() == true);
	return;
}

SimLogicModulePage* SimBasePage::logicModulePage(QString lmEquipmnetId, QWidget* parent)
{
	QVariant parentValue =  QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent));

	for (SimBasePage* page : m_pages)
	{
		SimLogicModulePage* cp = dynamic_cast<SimLogicModulePage*>(page);

		if (cp != nullptr &&
			cp->equipmentId() == lmEquipmnetId &&
			cp->property("SimParentObject") == parentValue)
		{
			return cp;
		}
	}

	return nullptr;
}

SimConnectionPage* SimBasePage::connectionPage(QString connectionId, QWidget* parent)
{
	QVariant parentValue =  QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent));

	for (SimBasePage* page : m_pages)
	{
		SimConnectionPage* cp = dynamic_cast<SimConnectionPage*>(page);

		if (cp != nullptr &&
			cp->connectionId() == connectionId &&
			cp->property("SimParentObject") == parentValue)
		{
			return cp;
		}
	}

	return nullptr;
}


SimSelectSchemaPage* SimBasePage::selectSchemaPage(QWidget* parent)
{
	QVariant parentValue =  QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent));

	for (SimBasePage* page : m_pages)
	{
		SimSelectSchemaPage* cp = dynamic_cast<SimSelectSchemaPage*>(page);

		if (cp != nullptr &&
			cp->property("SimParentObject") == parentValue)
		{
			return cp;
		}
	}

	return nullptr;
}
