#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include <DbLib/DbStruct.h>

SimSchemaManager::SimSchemaManager(SimIdeSimulator* simulator, QObject* parent) :
	VFrame30::SchemaManager(parent),
	m_simulator(simulator)
{
	Q_ASSERT(m_simulator);
	return;
}

std::shared_ptr<VFrame30::Schema> SimSchemaManager::loadSchema(const QString& schemaId)
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
	std::array folders = {
		Db::File::AlFileExtension,
		Db::File::MvsFileExtension,
		Db::File::TvsFileExtension,
		Db::File::UfbFileExtension,
		Db::File::DvsFileExtension,
		Db::File::VduFileExtension,
	};

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

bool SimSchemaManager::trendData(QUuid /*trendUuid*/,
								 const TrendLib::TrendSignalParam& /*trendSignal*/,
								 QDateTime /*from*/,
								 QDateTime /*to*/,
								 E::TimeType /*timeType*/,
								 E::TrendMode /*mode*/,
								 std::list<std::shared_ptr<TrendLib::OneHourData>>* /*outData*/) const
{
	// If you want Monitor schemas in simulator ide and want SchemaItemIndicator in mode trend
	// then you have to add here some code
	//
	Q_ASSERT(false);
	return false;
}

SimIdeSimulator* SimSchemaManager::simulator()
{
	return m_simulator;
}

const SimIdeSimulator* SimSchemaManager::simulator() const
{
	return m_simulator;
}

