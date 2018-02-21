#include "SimulatorBasePage.h"
#include "SimulatorControlPage.h"

std::list<SimulatorBasePage*> SimulatorBasePage::m_pages;

SimulatorBasePage::SimulatorBasePage(std::shared_ptr<SimIdeSimulator> simulator, QWidget* parent)
	: QWidget(parent),
	  m_simulator(simulator)
{
	assert(m_simulator);

	setAttribute(Qt::WA_DeleteOnClose);

	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Window);

	m_closeAction = new QAction(tr("Close"), this);
	m_closeAction->setShortcut(Qt::CTRL + Qt::Key_W);	// QKeySequence::Close leads to CTRL+F4 somehow (((
	connect(m_closeAction, &QAction::triggered, this, &SimulatorBasePage::deleteLater);
	addAction(m_closeAction);

	m_pages.push_back(this);
	this->setProperty("SimParentObject", QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent)));

	return;
}

SimulatorBasePage::~SimulatorBasePage()
{
	qDebug() << "SimulatorBasePage::~SimulatorBasePage()";
	m_pages.remove(this);
}

void SimulatorBasePage::deleteAllPages()
{
	auto listCopy = m_pages;		// m_pages is changed in destruct, so we need a copy
	for (SimulatorBasePage* page : listCopy)
	{
		delete page;
	}

	assert(m_pages.empty() == true);
	return;
}

SimulatorControlPage* SimulatorBasePage::controlPage(QString lmEquipmnetId, QWidget* parent)
{
	QVariant parentValue =  QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(parent));

	for (SimulatorBasePage* page : m_pages)
	{
		SimulatorControlPage* cp = dynamic_cast<SimulatorControlPage*>(page);

		if (cp != nullptr &&
			cp->equipmnetId() == lmEquipmnetId &&
			cp->property("SimParentObject") == parentValue)
		{
			return cp;
		}
	}

	return nullptr;
}
