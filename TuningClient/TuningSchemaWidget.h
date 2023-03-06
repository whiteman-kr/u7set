#ifndef TUNINGSCHEMAWIDGET_H
#define TUNINGSCHEMAWIDGET_H

#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/SchemaView.h"
#include "TuningSchemaView.h"
#include "TuningSchemaManager.h"
#include "../lib/Tuning/TuningUserManager.h"


class TuningClientTuningController : public VFrame30::TuningController
{
	Q_OBJECT

public:
	TuningClientTuningController(ITuningSignalManager* signalManager, TuningUserManager& userManager, std::vector<ITuningTcpClient*> tcpClients, QWidget* parent = nullptr);

protected:
	[[nodiscard]] virtual bool checkTuningAccess() const override;

private:
	TuningUserManager& m_userManager;
};


class TuningSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

public:
	TuningSchemaWidget() = delete;
	TuningSchemaWidget(TuningConfigController& configController,
					   TuningSignalManager& tuningSignalManager,
					   TuningClientTuningController* tuningController,
					   VFrame30::LogController* logController,
					   std::shared_ptr<VFrame30::Schema> schema,
					   TuningSchemaManager& schemaManager,
					   QWidget* parent);
	virtual ~TuningSchemaWidget() = default;

	//TuningSchemaView* tuningSchemaView();

private:
};

#endif // TUNINGSCHEMAWIDGET_H
