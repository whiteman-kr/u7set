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
		//MacrosExpander(std::shared_ptr<Schema> schema);

		QString parse(const QString& str, const Session& session,  const VFrame30::Schema* schema, const PropertyObject* schemaItem) const;

	private:
		//std::shared_ptr<Schema> m_schema;
	};

}

#endif // MACROSEXPANDER_H
