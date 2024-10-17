#include "SimulatorTabPage.h"
#include "./Simulator/SimSelectBuildDialog.h"
#include <Simulator/SimConsoleLogFile.h>


// SimPropertyStorage - implementation of ISimPropertyStorage that uses DbController to store properties
//
SimPropertyStorage::SimPropertyStorage(DbController& dbc, QWidget* m_parentWidget) :
	m_dbc{dbc},
	m_parentWidget{m_parentWidget}
{
}

QStringList SimPropertyStorage::getPropertyNames() const
{
	QStringList result;

	if (m_dbc.isProjectOpened() == true)
	{
		m_dbc.getUserPropertyList("%", &result, m_parentWidget);
	}

	return result;
}

bool SimPropertyStorage::removeProperty(QStringView propertyName) const
{
	if (m_dbc.isProjectOpened() == true)
	{
		return m_dbc.removeUserProperty(propertyName.toString(), m_parentWidget);
	}
	else
	{
		return false;
	}
}

void SimPropertyStorage::saveProperty(QStringView propertyName, QStringView value)
{
	if (m_dbc.isProjectOpened() == true)
	{
		m_dbc.setUserProperty(propertyName.toString(), value.toString(), m_parentWidget);
	}

	return;
}

QString SimPropertyStorage::loadProperty(QStringView propertyName, QStringView defaultValue, bool* ok)
{
	QString result;
	bool wasOk = false;

	if (m_dbc.isProjectOpened() == true)
	{
		wasOk = m_dbc.getUserProperty(propertyName.toString(), &result, defaultValue.toString(), m_parentWidget);
	}

	if (ok != nullptr)
	{
		*ok = wasOk;
	}

	return result;
}


// DbProjectStateNotifier - notifies about project state changes (opened/closed)
//
DbProjectStateNotifier::DbProjectStateNotifier(DbController& dbc, QObject* parent) :
	SimUi::DbProjectStateNotifier{parent},
	m_dbc{dbc}
{
	connect(&m_dbc, &DbController::projectOpened, this, &DbProjectStateNotifier::projectOpened);
	connect(&m_dbc, &DbController::projectClosed, this, &DbProjectStateNotifier::projectClosed);

	return;
}


//
//
// SimulatorTabPage
//
//
SimulatorTabPage::SimulatorTabPage(DbController* dbc, QWidget* parent) :
	MainTabPage{dbc, parent},
	m_propertyStorage{*dbc, this},
	m_dbProjectStateNotifier{*dbc}
{
	assert(dbc != nullptr);

	// Controls
	//
	std::function<QString(void)> getPathFunc = [this]()
	{
		return getProjectPathFunc();
	};

	m_simulatorWidget = new SimUi::SimWidget{std::make_shared<Sim::ConsoleLogFile>(),
											 {},
											 getPathFunc,
											 m_propertyStorage,
											 &m_dbProjectStateNotifier,
											 nullptr,
											 Qt::Widget,
											 false,
											 nullptr};

	QVBoxLayout* layout = new QVBoxLayout;

	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_simulatorWidget);

	setLayout(layout);

	return;
}

QString SimulatorTabPage::getProjectPathFunc()
{
	QSettings settings;

	QString project = db()->currentProject().projectName().toLower();
	QString lastPath = settings.value("SimulatorWidget/ProjectLastPath/" + project).toString();

	SimSelectBuildDialog d{project, lastPath, this};

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		lastPath = d.resultBuildPath();
		settings.setValue("SimulatorWidget/ProjectLastPath/" + project, lastPath);
	}
	else
	{
		lastPath.clear();
	}

	return lastPath;
}

void SimulatorTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}
