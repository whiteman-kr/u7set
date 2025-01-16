#pragma once

#include "TestScriptSelection.h"

#include <QStringList>


namespace TestSuite
{
	struct ControlParams
	{
		ControlParams() = default;

		ControlParams(const QStringList& scriptsFiles,
					  const QString& reportsPath,
					  const TestScriptSelection& testsFilter,
					  const QString& userName,
					  const QString& password) :
			scriptsFiles{scriptsFiles},
			reportsPath{reportsPath},
			testsFilter{testsFilter},
			userName{userName},
			password{password}
		{
		}

		QStringList scriptsFiles;        // List of script files for execution, if empty then exec all.
		QString reportsPath;             // Save reports to disk if path is not empty
		TestScriptSelection testsFilter; // Tests filter
		QString userName;
		QString password;
	};
} // namespace TestSuite