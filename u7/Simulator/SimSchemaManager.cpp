#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include "../../lib/DbStruct.h"

SimSchemaManager::SimSchemaManager(SimIdeSimulator* simulator, QObject* parent) :
	VFrame30::SchemaManager(parent),
	m_simulator(simulator)
{
	Q_ASSERT(m_simulator);

	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimSchemaManager::slot_projectUpdated);

	return;
}

std::shared_ptr<VFrame30::Schema> SimSchemaManager::loadSchema(QString schemaId)
{
	QString buildPath = QDir::fromNativeSeparators(m_simulator->buildPath());
	if (buildPath.isEmpty() == true)
	{
		return {};
	}

	if (buildPath.endsWith('/') == false)
	{
		buildPath += "/";
	}

	// Load schema from one of folowing folder.
	// Unfortunatelly, schema folder cannot be get from SchemaID, so just try every possible folder.
	//
	std::array folders = {Db::File::AlFileExtension,
						  Db::File::MvsFileExtension,
						  Db::File::TvsFileExtension,
						  Db::File::UfbFileExtension,
						  Db::File::DvsFileExtension};

	for (auto ext : folders)
	{
		QString fileName = buildPath + QString("Schemas.%1/").arg(ext) + schemaId + "." + QString(ext);
		std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(fileName);

		if (schema != nullptr)
		{
			return schema;
		}
	}

	return {};
}

void SimSchemaManager::slot_projectUpdated()
{
	return;
}

SimIdeSimulator* SimSchemaManager::simulator()
{
	return m_simulator;
}

const SimIdeSimulator* SimSchemaManager::simulator() const
{
	return m_simulator;
}

