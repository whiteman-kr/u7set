#include "ScriptChecker.h"

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
} // namespace Builder