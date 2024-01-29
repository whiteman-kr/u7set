#include "TuningSchemaManager.h"

TuningSchemaManager::TuningSchemaManager(TuningConfigController& configController, QObject* parent) :
	SchemaClientLib::ClientSchemaManager(configController, parent)
{
	return;
}

TuningConfigController& TuningSchemaManager::configController()
{
	return static_cast<TuningConfigController&>(SchemaClientLib::ClientSchemaManager::configController());
}

const TuningConfigController& TuningSchemaManager::configController() const
{
	return static_cast<const TuningConfigController&>(SchemaClientLib::ClientSchemaManager::configController());
}
