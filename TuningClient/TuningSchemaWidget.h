#ifndef TUNINGSCHEMAWIDGET_H
#define TUNINGSCHEMAWIDGET_H

#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/SchemaView.h"
#include "TuningSchemaView.h"
#include "TuningSchemaManager.h"
#include "../ClientLib/TuningUserManager.h"
#include "../ClientLib/TuningConnection.h"



class TuningClientTuningController : public VFrame30::TuningController
{
	Q_OBJECT

public:
	TuningClientTuningController(ITuningSignalManager* signalManager, ClientLib::TuningConnection* tuningConnection, ClientLib::TuningUserManager& userManager, QWidget* parent = nullptr);

protected:
	[[nodiscard]] virtual bool checkTuningAccess() const override;

private:
	ClientLib::TuningUserManager& m_userManager;
};


class TuningSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

public:
	TuningSchemaWidget() = delete;
	TuningSchemaWidget(TuningConfigController& configController,
					   TuningSignalManager& tuningSignalManager,
					   ClientLib::TuningConnection& tuningConnection,
					   TuningClientTuningController* tuningController,
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
	TuningSignalManager& m_tuningSignalManager;
	ClientLib::TuningConnection& m_tuningConnection;
};

#endif // TUNINGSCHEMAWIDGET_H
