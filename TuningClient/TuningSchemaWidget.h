#pragma once

#include "TuningSchemaManager.h"

#include <VFrame30/ClientSchemaWidget.h>

class TuningSchemaView;


class TuningSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

public:
	TuningSchemaWidget() = delete;
	TuningSchemaWidget(TuningConfigController& configController,
					   VFrame30::LogController* logController,
					   std::shared_ptr<VFrame30::Schema> schema,
					   TuningSchemaManager& schemaManager,
					   QWidget* parent);
	virtual ~TuningSchemaWidget() = default;

	// Slots
	//
public slots:
	void contextMenuRequested(const QPoint& pos);
	void signalContextMenu(QStringList appSignals, QStringList impactSignals, QStringList loopbacks, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

public:
	TuningSchemaView* tuningSchemaView();
	const TuningSchemaView* tuningSchemaView() const;

private:
	TuningConfigController& m_configController;
};
