#include "TestFile.h"
#include <QDebug>

// -------------------------------------------------------------------------------------------------------------------
//
// TestFileLine class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TestFileLine::TestFileLine(int number, const QString& line)
	: m_number(number)
	, m_line(line)
{
}

TestFileLine::~TestFileLine()
{
}

int TestFileLine::getCmdType(const QString& line)
{
	int cmdType = TF_CMD_UNKNOWN;

	if (line.isEmpty() == true)
	{
		return TF_CMD_EMPTY;
	}

	QStringList args = line.split(' ');
	if (args.count() == 0)
	{
		return TF_CMD_UNKNOWN;
	}

	QString cmd = args[0];

	for(int type = 1; type < TF_CMD_COUNT; type++)
	{
		if (cmd.compare(TF_Cmd[type]) == 0)
		{
			cmdType = type;
			break;
		}
	}

	return cmdType;
}

QString TestFileLine::parse()
{
	m_errorStr.clear();
	//
	//
	m_line.remove('\r');
	m_line.remove('\n');
	m_line = m_line.simplified();

	//
	//
	// find comment

	//
	//
	m_cmdType = getCmdType(m_line);
	switch (m_cmdType)
	{
		case TF_CMD_EMPTY:		m_errorStr.clear();					break;
		case TF_CMD_TEST:		m_errorStr = parseCmdTest();		break;
		case TF_CMD_ENDTEST:	m_errorStr.clear();					break;
		case TF_CMD_SCHEMA:		m_errorStr = parseCmdSchema();		break;
		case TF_CMD_COMPATIBLE: m_errorStr = parseCmdCompatible();	break;
		case TF_CMD_CONST: break;
		case TF_CMD_VAR: break;
		case TF_CMD_SET: break;
		case TF_CMD_CHECK: break;
		case TF_CMD_APPLY:		m_errorStr.clear();					break;
		case TF_CMD_DELAY:		m_errorStr = parseCmdDelay();			break;
		default:				m_errorStr = QString("Error : line (%1) - Unknown command").arg(m_number); break;
	}

	return m_errorStr;
}

QString TestFileLine::parseCmdTest()
{
	QString errorStr;

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		errorStr = QString("Error : line (%1) - Incorrect command").arg(m_number);
		return errorStr;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() != 2)
	{
		errorStr = QString("Error : line (%1) - Incorrect argument list").arg(m_number);
		return errorStr;
	}

	m_testID = argList[0].simplified();
	if(m_testID.isEmpty() == true)
	{
		errorStr = QString("Error : line (%1) - Incorrect TestID").arg(m_number);
		return errorStr;
	}

	m_testDescription = argList[1].simplified();
	if(m_testDescription.isEmpty() == true)
	{
		errorStr = QString("Error : line (%1) - Incorrect test description").arg(m_number);
		return errorStr;
	}

	return errorStr;
}

QString TestFileLine::parseCmdSchema()
{
	QString errorStr;

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		errorStr = QString("Error : line (%1) - Incorrect command").arg(m_number);
		return errorStr;
	}

	m_schemaID = m_line.right(m_line.length() - spacePos).simplified();
	if(m_schemaID.isEmpty() == true)
	{
		errorStr = QString("Error : line (%1) - Incorrect schemaID").arg(m_number);
		return errorStr;
	}

	return errorStr;
}

QString TestFileLine::parseCmdCompatible()
{
	QString errorStr;

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		errorStr = QString("Error : line (%1) - Incorrect command").arg(m_number);
		return errorStr;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		errorStr = QString("Error : line (%1) - Incorrect argument list").arg(m_number);
		return errorStr;
	}

	int argCount = argList.count();
	for(int i = 0; i < argCount; i++)
	{
		QString preset = argList[i].simplified();
		if(preset.isEmpty() == true)
		{
			errorStr = QString("Error : line (%1) - Incorrect compatible preset").arg(m_number);
			return errorStr;
		}

		m_compatibleList.append(preset);
	}

	return errorStr;
}

QString TestFileLine::parseCmdSet()
{
	QString errorStr;

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		errorStr = QString("Error : line (%1) - Incorrect command").arg(m_number);
		return errorStr;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		errorStr = QString("Error : line (%1) - Incorrect argument list").arg(m_number);
		return errorStr;
	}

	int argCount = argList.count();
	for(int i = 0; i < argCount; i++)
	{
		QString param = argList[i].simplified();
		if(param.isEmpty() == true)
		{
			errorStr = QString("Error : line (%1) - Incorrect compatible preset").arg(m_number);
			return errorStr;
		}

		QStringList sv = param.split('=');
		if (sv.count() != 2)
		{
			errorStr = QString("Error : line (%1) - Incorrect argument list").arg(m_number);
			return errorStr;
		}

		QString signal = sv[0].simplified();
		if (signal[0] != '#')
		{
			errorStr = QString("Error : line (%1) - Incorrect argument list").arg(m_number);
			return errorStr;
		}

		// m_SignalValueList.append
	}

	return errorStr;
}

QString TestFileLine::parseCmdDelay()
{
	QString errorStr;

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		errorStr = QString("Error : line (%1) - Incorrect command").arg(m_number);
		return errorStr;
	}

	QString ms = m_line.right(m_line.length() - spacePos).simplified();
	if(ms.isEmpty() == true)
	{
		errorStr = QString("Error : line (%1) - Incorrect argument").arg(m_number);
		return errorStr;
	}

	m_delay_ms = ms.sprintf("%0.0f", ms.toDouble()).toInt();

	return errorStr;
}

// -------------------------------------------------------------------------------------------------------------------
//
// TestFile class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TestFile::TestFile(QObject *parent)
	: QObject(parent)
{
}

TestFile::~TestFile()
{
	close();
}

bool TestFile::open()
{
	if (m_fileName.isEmpty() == true)
	{
		qDebug() << "Error: Test file name is empty";
		return false;
	}

	m_file.setFileName(m_fileName);
	if (m_file.open(QIODevice::ReadOnly) == false)
	{
		qDebug() << "Error: Test file" << m_fileName << "is not open";
		return false;
	}

	return true;
}

bool TestFile::parse()
{
	if (m_file.isOpen() == false)
	{
		qDebug() << "Error: Test file is not open";
		return false;
	}

	//
	//

	m_lineNumber = 0;

	m_foundEndOfTest = true;
	m_foundEndOfComment = -1;

	//
	//

	QString line;

	while(m_file.atEnd() == false)
	{
		m_lineNumber++;
		line = m_file.readLine();

		TestFileLine testline(m_lineNumber, line);
		QString errorStr = testline.parse();

		if (errorStr.isEmpty() == false)
		{
			m_errorList.append(errorStr);
			qDebug() << errorStr;
			continue;
		}


		if (testline.cmdType() == TF_CMD_TEST)
		{
			if (m_foundEndOfTest == false)
			{
				errorStr = QString("Error : line (%1) - Found not finished test").arg(m_lineNumber);
				m_errorList.append(errorStr);
				qDebug() << errorStr;

				continue;
			}

			m_foundEndOfTest = false;
		}

		if (testline.cmdType() == TF_CMD_ENDTEST)
		{
			if (m_foundEndOfTest == false)
			{
				m_foundEndOfTest = true;
			}
			else
			{
				errorStr = QString("Error : line (%1) - Found command endtest bun not found command test").arg(m_lineNumber);
				m_errorList.append(errorStr);
				qDebug() << errorStr;

				continue;
			}
		}

		m_commandList.append(line);
	}

	if (m_foundEndOfTest == false)
	{
		QString errorStr = QString("Error : line (%1) - Not found command endtest").arg(m_lineNumber);
		m_errorList.append(errorStr);
		qDebug() << errorStr;
	}

	return true;
}

void TestFile::close()
{
	if (m_file.isOpen() == false)
	{
		return;
	}

	m_file.close();
}
