#include "SelectSchemaWidget.h"

SelectSchemaWidget::SelectSchemaWidget(QWidget *parent) : QWidget(parent)
{
	m_schemas.reserve(32);
}

void SelectSchemaWidget::clear()
{
	m_currentIndex = -1;
	m_schemas.clear();

	return;
}

void SelectSchemaWidget::addSchema(QString schemaId, QString caption)
{
	m_schemas.push_back(SchemaItem{schemaId, caption});
}

bool SelectSchemaWidget::setCurrentSchema(QString schemaId)
{
	for (size_t index = 0; index < m_schemas.size(); index++)
	{
		if (m_schemas[index].schemaId == schemaId)
		{
			m_currentIndex = static_cast<int>(index);
			return true;
		}
	}

	return false;
}

bool SelectSchemaWidget::setCurrentSchema(int index)
{
	if (index < 0)
	{
		m_currentIndex = -1;
		return true;
	}

	if (static_cast<size_t>(index) >= m_schemas.size())
	{
		assert(static_cast<size_t>(index) < m_schemas.size());
		return false;
	}

	m_currentIndex = index;

	return true;
}

int SelectSchemaWidget::currentIndex() const
{
	return m_currentIndex;
}

QString SelectSchemaWidget::currentSchemaId() const
{
	if (m_currentIndex < 0 ||
		static_cast<size_t>(m_currentIndex) >= m_schemas.size())
	{
		return QString();
	}

	return m_schemas[m_currentIndex].schemaId;
}
