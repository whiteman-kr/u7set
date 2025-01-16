#include "ScriptChecker.h"
#include "IssueLogger.h"

#include <VFrame30/Schema.h>

namespace Builder
{
	bool ScriptChecker::checkFile(const QString& script, const QString& fileName, IssueLogger& log)
	{
		if (script.isEmpty() == true)
		{
			return true;
		}

		QJSEngine jsEngine;
		QJSValue jsValue = jsEngine.evaluate(script, fileName);

		if (jsValue.isError() == true)
		{
			int line = jsValue.property("lineNumber").toInt();
			QString message = jsValue.toString();

			log.errEQP6300(fileName, line, message);
		}

		return jsValue.isError() == false;
	}

	bool ScriptChecker::checkEquipmentProperty(const QString& script, const QString& equipmentId, const QString& property, IssueLogger& log)
	{
		if (script.isEmpty() == true)
		{
			return true;
		}

		QJSEngine jsEngine;
		QJSValue jsValue = jsEngine.evaluate(script, QString("%1.%2").arg(equipmentId).arg(property));

		if (jsValue.isError() == true)
		{
			int line = jsValue.property("lineNumber").toInt();
			QString message = jsValue.toString();

			log.errEQP6301(equipmentId, property, line, message);
		}

		return jsValue.isError() == false;
	}

	bool ScriptChecker::checkSchema(const VFrame30::Schema* schema, IssueLogger& log)
	{
		if (schema == nullptr)
		{
			assert(schema);
			return false;
		}

		if (schema->isVduSchema() == true)
		{
			// Lua? script check.
			//
			return true;
		}

		// java script check.
		//
		QJSEngine jsEngine;

		// Function for: Check script property of the schema.
		//
		auto checkSchemaProperty = [&log, &jsEngine](const VFrame30::Schema* schema, const Property* property)
		{
			assert(schema);
			assert(property);

			QString script = property->value().toString();

			if (script.isEmpty() == true || schema->excludeFromBuild() == true)
			{
				return true;
			}

			QJSValue jsValue = jsEngine.evaluate(script, QString("%1.%2").arg(schema->schemaId()).arg(property->caption()));
			if (jsValue.isError() == true)
			{
				int line = jsValue.property("lineNumber").toInt();
				QString message = jsValue.toString();

				log.errEQP6302(schema->schemaId(), property->caption(), line, message);
			}

			return jsValue.isError() == false;
		};

		// Function for: Check script property of the SchemaItem.
		//
		auto checkSchemaItemProperty = [&log, &jsEngine](const VFrame30::SchemaItem* schemaItem, const Property* property)
		{
			assert(schemaItem);
			assert(property);

			QString script = property->value().toString();
			if (script.isEmpty() == true)
			{
				return true;
			}

			QJSValue jsValue = jsEngine.evaluate(script,
												 QString("%1.%2.%3").arg(schemaItem->parentSchema()->schemaId()).arg(schemaItem->label()).arg(property->caption()));
			if (jsValue.isError() == true)
			{
				int line = jsValue.property("lineNumber").toInt();
				QString message = jsValue.toString();

				log.errEQP6303(schemaItem->parentSchema()->schemaId(),
							   schemaItem->label(),
							   schemaItem->guid(),
							   property->caption(),
							   line,
							   message);
			}

			return jsValue.isError() == false;
		};

		bool result = true;

		// Check script property of the schema.
		//
		for (auto schemaProperties = schema->properties();
			 auto schemaProperty : schemaProperties)
		{
			if (schemaProperty->isScript() == true)
			{
				result &= checkSchemaProperty(schema, schemaProperty.get());
			}
		}

		// Check script property of the SchemaItem.
		//
		for (const auto& layer : schema->layers())
		{
			assert(layer);

			for (const auto& schemaItem : layer->items())
			{
				assert(schemaItem);

				if (schemaItem->isCommented() == true)
				{
					continue;
				}

				for (auto properties = schemaItem->properties();
					 auto property : properties)
				{
					if (property->isScript() == true)
					{
						result &= checkSchemaItemProperty(schemaItem.get(), property.get());
					}
				}
			}
		}

		return result;
	}
} // namespace Builder