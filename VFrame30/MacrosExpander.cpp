#include "MacrosExpander.h"
#include "../lib/PropertyObject.h"
#include "Session.h"
#include "Schema.h"
#include "DrawParam.h"
#include "ClientSchemaView.h"

namespace VFrame30
{
	QStringList MacrosExpander::parse(const QStringList& stringList, const CDrawParam* drawParam, const PropertyObject* schemaItem)
	{
		QStringList resultList;
		resultList.reserve(stringList.size());

		for (const QString& str : stringList)
		{
			QString parsedString = parse(str, drawParam, schemaItem);
			resultList.push_back(parsedString);
		}

		return resultList;
	}

	QString MacrosExpander::parse(const QString& str, const CDrawParam* drawParam, const PropertyObject* schemaItem)
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return str;
		}

		return parse(str,
					 drawParam->isMonitorMode() ? drawParam->clientSchemaView() : nullptr,
					 &drawParam->session(),
					 drawParam->schema(),
					 schemaItem);
	}

	QStringList MacrosExpander::parse(const QStringList& stringList,
									  const ClientSchemaView* clientView,
									  const Session* session,
									  const VFrame30::Schema* schema,
									  const PropertyObject* thisObject)
	{
		QStringList resultList;
		resultList.reserve(stringList.size());

		for (const QString& str : stringList)
		{
			QString parsedString = parse(str, clientView, session, schema, thisObject);
			resultList.push_back(parsedString);
		}

		return resultList;
	}

	QString MacrosExpander::parse(const QString& str,
								  const ClientSchemaView* clientView,
								  const Session* session,
								  const VFrame30::Schema* schema,
								  const PropertyObject* thisObject)
	{
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
				// Look for property assigned to object
				//
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
					object = session;
					propName = macro.mid(macro.indexOf('.') + 1);
					break;
				}
			}
			while (false);

			QString replaceText;

			do
			{
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
						replaceText = QString("[unk_prop: %1]").arg(macro);
					}
					break;
				}


				// Look for variables
				//
				if (clientView != nullptr)
				{
					QVariant var = clientView->variable(macro);

					if (var.isValid() == true)
					{
						replaceText = var.toString();
						break;
					}
				}

				// Total else
				//
				replaceText = QString("[unk_obj_or_var: %1]").arg(macro);
			}
			while (false);

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
