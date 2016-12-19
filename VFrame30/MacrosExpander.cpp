#include "MacrosExpander.h"
#include "../lib/PropertyObject.h"

namespace VFrame30
{

	MacrosExpander::MacrosExpander()
	{

	}

	QString MacrosExpander::parse(const QString& str, const PropertyObject* thisObject) const
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

			QString replaceText = "ReplaceDText";

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
