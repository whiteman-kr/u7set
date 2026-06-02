#include <SchemaClientLib/SchemaListWidget.h>
#include "SchemaListWidgetPrivate.h"

namespace SchemaClientLib
{
	//
	//
	//		SchemaListWidget
	//
	//
	SchemaListWidget::SchemaListWidget(std::vector<SchemaListTreeColumns> columns, bool showTags, QWidget* parent) :
		QWidget(parent),
		m_widget{new SchemaListWidgetPrivate{std::move(columns), showTags, this}}
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(m_widget);

		setMinimumHeight(100);

		connect(m_widget, &SchemaListWidgetPrivate::openSchemaRequest, this, &SchemaListWidget::openSchemaRequest);

		return;
	}
	
	SchemaListWidget::~SchemaListWidget() = default;

	void SchemaListWidget::setDetails(VFrame30::SchemaDetailsSet details)
	{
		return m_widget->setDetails(std::move(details));
	}
} // namespace SchemaClientLib