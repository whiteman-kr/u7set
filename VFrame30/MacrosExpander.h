#pragma once
#include <QVariant>

class PropertyObject;

namespace VFrame30
{
	class CDrawParam;
	class Session;
	class Schema;
	class ClientSchemaView;

	class MacrosExpander
	{
	public:
		MacrosExpander() = delete;

		static QStringList parse(const QStringList& stringList, const CDrawParam* drawParam, const PropertyObject* schemaItem);
		static QString parse(const QString& str, const CDrawParam* drawParam, const PropertyObject* schemaItem);

		static QStringList parse(const QStringList& stringList,
								 const ClientSchemaView* clientView,
								 const Session* session,
								 const VFrame30::Schema* schema,
								 const PropertyObject* thisObject);

		static QString parse(const QString& str,
							 const ClientSchemaView* clientView,
							 const Session* session,
							 const VFrame30::Schema* schema,
							 const PropertyObject* thisObject);
	};

}

