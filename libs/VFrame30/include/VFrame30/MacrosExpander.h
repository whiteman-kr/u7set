#pragma once
#include <VFrame30/Context.h>
#include <VFrame30/Session.h>

namespace VFrame30
{
	class Schema;
	class SchemaItem;

	class MacrosExpander
	{
	public:
		MacrosExpander() = delete;

		static QStringList parse(const QStringList& stringList,
								 const Context* context,
								 const Session* session,
								 const VFrame30::SchemaItem* schemaItem);

		static QString parse(const QString& str,
							 const Context* context,
							 const Session* session,
							 const VFrame30::SchemaItem* schemaItem);

		static QStringList parse(const QStringList& stringList,
								 const IViewVariables* viewVariables,
								 const Session* session,
								 const VFrame30::Schema* schema,
								 const PropertyObject* thisObject);

		static QString parse(const QString& str,
							 const IViewVariables* viewVariables,
							 const Session* session,
							 const VFrame30::Schema* schema,
							 const PropertyObject* thisObject);
	};

}

