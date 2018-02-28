#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <memory>
#include <QObject>
#include "Schema.h"
#include "VFrame30Lib_global.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaManager : public QObject
	{
		Q_OBJECT

	public:
		explicit SchemaManager(QObject* parent = nullptr);
		virtual ~SchemaManager();

		// Methods to use
		//
	public:
		void clear();

		// Get loaded schema
		//
		virtual std::shared_ptr<VFrame30::Schema> schema(QString schemaId);

	protected:
		// Load schema, must be overriden to perform loading schema appropriate to client.
		// Default implementation: assert(false);
		//
		virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId);

		// Signals
		//
	signals:
		void schemasWereReseted();

	public:
		// User must provide GlobalScript
		//
		const QString& globalScript() const;
		void setGlobalScript(const QString& value);

	private:
		std::map<QString, std::shared_ptr<VFrame30::Schema>> m_schemas;		// Loaded schemas, map by SchemaID
		QString m_globalScript;
	};

}

#endif // SCHEMAMANAGER_H
