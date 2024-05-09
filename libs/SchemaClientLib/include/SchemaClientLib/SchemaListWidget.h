#pragma once

#include "../../VFrame30/SchemaDetails.h"
#include <QStringList>
#include <QWidget>

namespace SchemaClientLib
{
	class SchemaListWidgetPrivate;

	enum class SchemaListTreeColumns
	{
		SchemaID,
		Caption,
		Tags,
		Modules
	};

	//
	//
	//		SchemaListWidget
	//
	//
	class SchemaListWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit SchemaListWidget(std::vector<SchemaClientLib::SchemaListTreeColumns> columns, bool showTags, QWidget* parent);
		virtual ~SchemaListWidget();

	public:
		void setDetails(VFrame30::SchemaDetailsSet details);

	signals:
		void openSchemaRequest(QString schemaId, QStringList highlightIds);

	private:
		SchemaListWidgetPrivate* m_widget = nullptr;
	};
} // namespace SchemaClientLib	