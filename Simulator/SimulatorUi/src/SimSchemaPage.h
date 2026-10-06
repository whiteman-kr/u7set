#pragma once

#include "SimBasePage.h"
#include "SimSchemaWidget.h"
#include <SimulatorLib/SimControlStatus.h>
#include <SimulatorLib/SimLogicModule.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/TuningController.h>


namespace VFrame30
{
	class Schema;
	class AppSignalController;
} // namespace VFrame30

namespace SimUi
{
	class SimSchemaManager;
	class SimIdeSimulator;


	class SimSchemaPage : public SimBasePage
	{
		Q_OBJECT

	public:
		SimSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
					  SimIdeSimulator* simulator,
					  SimSchemaManager* schemaManager,
					  VFrame30::AppSignalController* appSignalController,
					  QWidget* parent = nullptr);

	public:
		QString schemaId() const;

		const QStringList& highlightIds() const;
		void setHighlightIds(const QStringList& value);

	protected slots:
		void controlStateChanged(Sim::SimControlState state);

	signals:
		// Request to create new trend for the specified signals
		//
		void signal_newTrend(QStringList appSignalIds);

	public:
		SimSchemaWidget* simSchemaWidget();
		const SimSchemaWidget* simSchemaWidget() const;

	private:
		SimSchemaWidget* m_schemaWidget = nullptr;
	};
} // namespace SimUi