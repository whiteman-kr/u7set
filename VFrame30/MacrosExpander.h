#ifndef MACROSEXPANDER_H
#define MACROSEXPANDER_H

class PropertyObject;

namespace VFrame30
{
	class Session;
	class Schema;

	class MacrosExpander
	{
	public:
		MacrosExpander();

		static QString parse(const QString& str, const Session& session,  const VFrame30::Schema* schema, const PropertyObject* schemaItem);
	};

}

#endif // MACROSEXPANDER_H
