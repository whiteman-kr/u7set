#include "MacrosExpander.h"
#include "../lib/PropertyObject.h"
#include "Session.h"
#include "Schema.h"

namespace VFrame30
{

	MacrosExpander::MacrosExpander()
	{

	}

	QString MacrosExpander::parse(const QString& str,
								  const Session& session,
								  const Schema* schema,
								  const PropertyObject* thisObject)
	{
		if (schema == nullptr ||
			thisObject == nullptr)
		{
			assert(schema);
			assert(thisObject);
			return QString();
		}

		QString result = str;

		QRegExp reStartIndex("\\$\\([a-zA-Z0-9]+[\\.]?[a-zA-Z0-9]*");	// Search for $(SomeText[.][SomeText])

		int index = 0;
		while (index < result.size())
		{
			// Find macro bounds
			//
			int startIndexOfMacro = result.indexOf(reStartIndex, index);
			if (startIndexOfMacro == -1)
			{
				break;
			}

			int endIndexOfMacro = result.indexOf(')', startIndexOfMacro + 1);
			if (endIndexOfMacro == -1)
			{
				break;
			}

			// Extract macro string
			//
			QString macro = result.mid(startIndexOfMacro + 2, endIndexOfMacro - startIndexOfMacro - 2);		// +2 is $(, -2 is $()

			// Get actual text
			//
			const PropertyObject* object = nullptr;
			QString propName;

			do
			{
				if (macro.startsWith("this.", Qt::CaseInsensitive) == true ||
					macro.startsWith("item.", Qt::CaseInsensitive) == true)
				{
					object = thisObject;
					propName = macro.mid(macro.indexOf('.') + 1);
					break;
				}

				if (macro.startsWith("schema.", Qt::CaseInsensitive) == true)
				{
					object = schema;
					propName = macro.mid(macro.indexOf('.') + 1);
					break;
				}

				if (macro.startsWith("session.", Qt::CaseInsensitive) == true)
				{
					object = &session;
					propName = macro.mid(macro.indexOf('.') + 1);
					break;
				}
			}
			while (false);

			QString replaceText;

			if (object != nullptr &&
				propName.isEmpty() == false)
			{
				QVariant value = object->propertyValue(propName);

				if (value.isValid() == true)
				{
					replaceText = value.toString();
				}
				else
				{
					replaceText = QLatin1String("[UnknownProp]");
				}
			}
			else
			{
				replaceText = QLatin1String("[UnknownObject]");
			}

			// Replace text in result
			//
			result.replace(startIndexOfMacro, endIndexOfMacro - startIndexOfMacro + 1, replaceText);

			// Iterate
			//
			index = startIndexOfMacro + replaceText.size();
		}

		return result;
	}

}
