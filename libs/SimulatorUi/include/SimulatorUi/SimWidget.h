#pragma once

#include <ILogFile.h>
#include <SimulatorUi/ISimPropertyStorage.h>
#include <SimulatorUi/SimIdeSimulator.h>

#include <functional>
#include <memory>

#include <QMainWindow>


namespace SimUi
{
	class SimWidgetPrivate;

	// Derive from this class to notify simulator when the project state changes, DbProject was opened or closed.
	//
	class DbProjectStateNotifier : public QObject
	{
		Q_OBJECT

	public:
		DbProjectStateNotifier(QObject* parent = nullptr) :
			QObject{parent}
		{
		}

	signals:
		void projectOpened(); // Emit these signals when the project state changes
		void projectClosed();
	};

	// Main simulator widget
	//
	class SimWidget : public QMainWindow
	{
		Q_OBJECT

	public:
		SimWidget(std::shared_ptr<ILogFile> ideLogFile,
				  std::shared_ptr<SimIdeSimulator> simulator,
				  std::function<QString(void)> getProjectPathFunc,
				  ISimPropertyStorage& propertyStorage,
				  DbProjectStateNotifier* dbProjectStateNotifier,
				  QWidget* parent = nullptr,
				  Qt::WindowType windowType = Qt::Window);

		SimWidget(std::shared_ptr<ILogFile> ideLogFile,
				  std::shared_ptr<SimIdeSimulator> simulator,
				  std::function<QString(void)> getProjectPathFunc,
				  ISimPropertyStorage& propertyStorage,
				  DbProjectStateNotifier* dbProjectStateNotifier,
				  QWidget* parent = nullptr,
				  Qt::WindowType windowType = Qt::Window,
				  bool slaveWindow = false, // Cannot have output pane, do not stores its state
				  SimWidgetPrivate* masterWindow = nullptr);

		virtual ~SimWidget();

	private:
		SimWidgetPrivate* m_impl = nullptr;
	};
} // namespace SimUi