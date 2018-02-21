#include "SimulatorSchemaPage.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QMenu>

SimulatorSchemaPage::SimulatorSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
										 std::shared_ptr<SimIdeSimulator> simulator,
										 QWidget* parent)
	: SimulatorBasePage(simulator, parent),
	m_schema(schema)
{
	assert(m_schema);
	assert(m_simulator);

	// --
	//

	return;
}

QString SimulatorSchemaPage::schemaId() const
{
	return m_schema->schemaId();
}
