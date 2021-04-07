#pragma once

#include "Schema.h"

namespace VFrame30
{
	class SchemaManager : public QObject
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

		virtual int schemaCount() const;
		virtual std::shared_ptr<VFrame30::Schema> schemaByIndex(int schemaIndex);

		virtual QString schemaCaptionById(const QString& schemaId) const;
		virtual QString schemaCaptionByIndex(int schemaIndex) const;
		virtual QString schemaIdByIndex(int schemaIndex) const;

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
		QString m_globalScript;
	};

}


