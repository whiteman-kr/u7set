#include <VFrame30/MacrosExpander.h>
#include <VFrame30/Schema.h>

namespace VFrame30
{
	QStringList MacrosExpander::parse(const QStringList& stringList,
									  const Context* context,
									  const Session* session,
									  const VFrame30::SchemaItem* schemaItem)
	{
		QStringList resultList;
		resultList.reserve(stringList.size());

		if (context == nullptr)
		{
			Q_ASSERT(context);
			return resultList;
		}

		for (const QString& str : stringList)
		{
			QString parsedString = parse(str, context, session, schemaItem);
			resultList.push_back(parsedString);
		}

		return resultList;
	}

	QString MacrosExpander::parse(const QString& str,
								  const Context* context,
								  const Session* session,
								  const VFrame30::SchemaItem* schemaItem)
	{
		if (context == nullptr)
		{
			Q_ASSERT(context);
			return str;
		}

		const Schema* schema = schemaItem->parentSchema();

		return parse(str,
					 context->viewVariables(),
					 session,
					 schema,
					 schemaItem);
	}

	QStringList MacrosExpander::parse(const QStringList& stringList,
									  const IViewVariables* viewVariables,
									  const Session* session,
									  const VFrame30::Schema* schema,
									  const PropertyObject* thisObject)
	{
		QStringList resultList;
		resultList.reserve(stringList.size());

		for (const QString& str : stringList)
		{
			QString parsedString = parse(str, viewVariables, session, schema, thisObject);
			resultList.push_back(parsedString);
		}

		return resultList;
	}

	QString MacrosExpander::parse(const QString& str,
								  const IViewVariables* viewVariables,
								  const Session* session,
								  const VFrame30::Schema* schema,
								  const PropertyObject* thisObject)
	{
		QString result = str;

		qsizetype index = 0;
		while (index < result.size())
		{
			// Find macro bounds, $(SomeText[.][SomeText])
			//
			qsizetype startIndexOfMacro = result.indexOf("$(", index);
			if (startIndexOfMacro == -1)
			{
				break;
			}

			qsizetype endIndexOfMacro = result.indexOf(')', startIndexOfMacro + 1);
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

			do  // while (false);
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

			do  // while (false);
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
				if (viewVariables != nullptr)
				{
					QVariant var = viewVariables->viewVariable(macro);

					if (var.isValid() == true)
					{
						replaceText = var.toString();
						break;
					}
				}

				// Look for environment variables.
				//
				{
#ifdef Q_OS_WIN32
					// Windows specific code, use qEnvironmentVariable.
					//
					if (QString envVar = qEnvironmentVariable(macro.toUtf8());
						envVar.isEmpty() == false)
					{
						replaceText = envVar;
						break;
					}
#endif // Q_OS_WIN32
#ifdef Q_OS_LINUX
					// Linux specific code, use qgetenv().
					//
					if (QByteArray envVar = qgetenv(macro.toUtf8());
						envVar.isEmpty() == false)
					{
						replaceText = QString::fromLocal8Bit(envVar);
						break;
					}
#endif // Q_OS_UNIX	
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
