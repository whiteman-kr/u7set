#include "SimBasePage.h"
#include "SimModulePage.h"

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
	qDebug() << "SimulatorBasePage::~SimulatorBasePage()";
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

SimModulePage* SimBasePage::modulePage(QString lmEquipmnetId, QWidget* parent)
{
	QVariant parentValue =  QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent));

	for (SimBasePage* page : m_pages)
	{
		SimModulePage* cp = dynamic_cast<SimModulePage*>(page);

		if (cp != nullptr &&
			cp->equipmnetId() == lmEquipmnetId &&
			cp->property("SimParentObject") == parentValue)
		{
			return cp;
		}
	}

	return nullptr;
}
