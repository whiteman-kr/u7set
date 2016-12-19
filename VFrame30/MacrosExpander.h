#ifndef MACROSEXPANDER_H
#define MACROSEXPANDER_H

class PropertyObject;

namespace VFrame30
{

	class MacrosExpander
	{
	public:
		MacrosExpander();
		//MacrosExpander(std::shared_ptr<Schema> schema);

		QString parse(const QString& str, const PropertyObject* schemaItem) const;

	private:
		//std::shared_ptr<Schema> m_schema;
	};

}

#endif // MACROSEXPANDER_H
