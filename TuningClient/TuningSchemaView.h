#ifndef TUNINGSCHEMAVIEW_H
#define TUNINGSCHEMAVIEW_H

#include "ScriptTuningClientApplication.h"
#include "TuningSchemaManager.h"
#include "../VFrame30/ClientSchemaView.h"


class TuningSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	TuningSchemaView(TuningConfigController& configController, TuningSchemaManager& schemaManager, QWidget* parent = nullptr);
	virtual ~TuningSchemaView() = default;

public:
	virtual VFrame30::DrawMode drawMode() const override;

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void updateScriptGlobalVars(QJSEngine& engine) override;

private slots:
	void configurationArrived(TuningClientConfigSettings configuration);

	// Data
	//
private:
	int m_configurationId = -1;		// Last set configuration
	TuningConfigController& m_configController;
	ScriptTuningClientApplication m_app;
};

#endif // TUNINGSCHEMAVIEW_H
