#pragma once

#include "SimBasePage.h"

namespace SchemaClientLib
{
	class SchemaListWidget;
}

namespace SimUi
{
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
		SchemaClientLib::SchemaListWidget* m_schemaListWidget = nullptr;
	};
} // namespace SimUi