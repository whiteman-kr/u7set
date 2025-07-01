#include "SimWidgetPrivate.h"
#include <SimulatorUi/SimWidget.h>


namespace SimUi
{
	SimWidget::SimWidget(std::shared_ptr<ILogFile> ideLogFile,
						 std::shared_ptr<SimIdeSimulator> simulator,
						 std::function<QString(QWidget*)> getProjectPathFunc,
						 ISimPropertyStorage& propertyStorage,
						 DbProjectStateNotifier* dbProjectStateNotifier,
						 QWidget* parent,
						 Qt::WindowType windowType,
						 bool slaveWindow,
						 SimWidgetPrivate* masterWindow) :
		QMainWindow{parent, windowType}
	{
		m_impl = new SimWidgetPrivate{ideLogFile,
									  simulator,
									  getProjectPathFunc,
									  propertyStorage,
									  dbProjectStateNotifier,
									  parent,
									  windowType,
									  slaveWindow,
									  masterWindow};
		setCentralWidget(m_impl);

		return;
	}

	SimWidget::~SimWidget() = default;
} // namespace SimUi