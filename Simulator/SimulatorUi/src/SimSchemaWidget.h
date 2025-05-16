#pragma once

#include <VFrame30/ClientSchemaWidget.h>


namespace VFrame30
{
	class AppSignalController;
	class SchemaItem;
	class Schema;
} // namespace VFrame30

namespace SimUi
{
	class SimSchemaManager;
	class SimSchemaView;
	class SimIdeSimulator;
	class SimWidgetPrivate;

	//
	//
	// SimSchemaWidget
	//
	//
	class SimSchemaWidget : public VFrame30::ClientSchemaWidget
	{
		Q_OBJECT

	private:
		SimSchemaWidget() = delete;

	public:
		SimSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
						SimSchemaManager* schemaManager,
						VFrame30::AppSignalController* appSignalController,
						SimIdeSimulator* simulator,
						QWidget* parent);
		virtual ~SimSchemaWidget();

	protected:
		void createActions();

		// Methods
		//
	protected:
		SimWidgetPrivate* simWidget();

		// Signals
		//
	signals:

		// Slots
		//
	public slots:
		void contextMenuRequested(const QPoint& pos);

		void signalContextMenu(QStringList appSignals, QStringList impactSignals, QString loopbackId, const QList<QMenu*> customMenu);

	protected slots:
		void updateProject();

		// Properties
		//
	public:
		SimSchemaView* simSchemaView();
		const SimSchemaView* simSchemaView() const;

		// Data
		//
	private:
		SimIdeSimulator* m_simulator = nullptr;
		VFrame30::LogController m_logController;

		// Actions
		//
		QAction* m_zoomInAction = nullptr;
		QAction* m_zoomOutAction = nullptr;
		QAction* m_zoom100Action = nullptr;
		QAction* m_zoomToFitAction = nullptr;

		QAction* m_newTabAction = nullptr;
		QAction* m_closeTabAction = nullptr;
	};
} // namespace SimUi