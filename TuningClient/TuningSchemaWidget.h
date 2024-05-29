#ifndef TUNINGSCHEMAWIDGET_H
#define TUNINGSCHEMAWIDGET_H

#include <VFrame30/ClientSchemaWidget.h>
#include "TuningSchemaManager.h"

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
	void signalContextMenu(QStringList appSignals,
						   QStringList impactSignals,
						   QStringList loopbacks,
						   const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

private:
	TuningConfigController& m_configController;
};

#endif // TUNINGSCHEMAWIDGET_H
