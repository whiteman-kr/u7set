#pragma once

#include <QWidget>
#include "../../lib/Ui/SchemaListWidget.h"
#include "SimIdeSimulator.h"
#include "SimBasePage.h"

class SimSelectSchemaPage : public SimBasePage
{
	Q_OBJECT

public:
	explicit SimSelectSchemaPage(SimIdeSimulator* simulator, QWidget* parent);

public slots:
	void updateSchemaList();

signals:
	void openSchemaTabPage(QString schemaId, QStringList highlightIds);

private:
	SchemaListWidget* m_schemaListWidget = nullptr;
};
