#ifndef SCHEMASTORAGE_H
#define SCHEMASTORAGE_H

#include <QObject>
#include "TuningConfigController.h"
#include "../VFrame30/SchemaManager.h"

class TuningSchemaManager : public VFrame30::SchemaManager
{
public:
	explicit TuningSchemaManager(TuningConfigController* configController, QObject* parent = nullptr);

public:
	virtual int schemaCount() const override;
	virtual std::shared_ptr<VFrame30::Schema> schemaByIndex(int schemaIndex,
															std::shared_ptr<VFrame30::Context> context) override;

	virtual QString schemaCaptionById(const QString& schemaId) const override;
	virtual QString schemaCaptionByIndex(int schemaIndex) const override;
	virtual QString schemaIdByIndex(int schemaIndex) const override;

public:
	[[nodiscard]] TuningConfigController* configController();
	[[nodiscard]] const TuningConfigController* configController() const;

protected:
	virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

private:
	TuningConfigController* m_configController = nullptr;
};

#endif // SCHEMASTORAGE_H
