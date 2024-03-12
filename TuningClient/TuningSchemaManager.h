#pragma once

#include <SchemaClientLib/ClientSchemaManager.h>
#include "TuningConfigController.h"

class TuningSchemaManager : public SchemaClientLib::ClientSchemaManager
{
public:
	explicit TuningSchemaManager(TuningConfigController& configController, QObject* parent = nullptr);

public:
	[[nodiscard]] TuningConfigController& configController();
	[[nodiscard]] const TuningConfigController& configController() const;

private:
};
