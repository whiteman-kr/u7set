#ifndef SCHEMASTORAGE_H
#define SCHEMASTORAGE_H

#include <QObject>
#include "ConfigController.h"
#include "../VFrame30/SchemaManager.h"

class TuningSchemaManager : public VFrame30::SchemaManager
{
public:
	explicit TuningSchemaManager(ConfigController* configController, QObject* parent = nullptr);

protected:
	virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

public slots:
	void globalScriptArrived(QByteArray data);

private:
	ConfigController* m_configController = nullptr;
};

#endif // SCHEMASTORAGE_H
