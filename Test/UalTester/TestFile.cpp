#include "TestFile.h"
#include <QDebug>

// -------------------------------------------------------------------------------------------------------------------
//
// TestFileLine class implementation
//
// -------------------------------------------------------------------------------------------------------------------

const char* const TestCommand::PARAM_TEST_ID = "TestID";
const char* const TestCommand::PARAM_TEST_DESCRIPTION = "TestDescription";
const char* const TestCommand::PARAM_SCHEMA_ID = "SchemaID";

TestCommand::TestCommand()
{
}

TestCommand::TestCommand(SignalBase* pSignalBase)
	: m_pSignalBase(pSignalBase)
	, m_lineIndex(0)
	, m_foundEndOfTest(true)
{
}

TestCommand::~TestCommand()
{
}

int TestCommand::getCmdType(const QString& line)
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
		if (cmd.compare(TestFileCmd[type]) == 0)
		{
			cmdType = type;
			break;
		}
	}

	return cmdType;
}

bool TestCommand::parse(const QString& line)
{
	m_line = line;
	m_errorList.clear();
	m_lineIndex ++;

	// remove unnecessary characters
	//
	m_line.remove('\r');
	m_line.remove('\n');
	m_line = m_line.simplified();

	// remove all after comment "//"
	//
	int pos = m_line.indexOf("//");
	if (pos != -1)
	{
		m_line.remove(pos, m_line.length() - pos);
	}

	// parse line
	//
	m_type = getCmdType(m_line);
	if (m_type < 0 || m_type >= TF_CMD_COUNT)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	bool resultCmd = true;

	switch (m_type)
	{
		case TF_CMD_TEST:		resultCmd = parseCmdTest();			break;
		case TF_CMD_ENDTEST:	resultCmd = parseCmdEndtest();		break;
		case TF_CMD_SCHEMA:		resultCmd = parseCmdSchema();		break;
		case TF_CMD_COMPATIBLE: resultCmd = parseCmdCompatible();	break;
		case TF_CMD_CONST:		resultCmd = parseCmdConst();		break;
		case TF_CMD_VAR:		resultCmd = parseCmdVar();			break;
		case TF_CMD_SET:		resultCmd = parseCmdSet();			break;
		case TF_CMD_CHECK:		resultCmd = parseCmdCheck();		break;
		case TF_CMD_DELAY:		resultCmd = parseCmdDelay();		break;
	}

	return resultCmd;
}

bool TestCommand::parseCmdTest()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() != 2)
	{
		QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;

	QString testID = argList[0].simplified();
	if(testID.isEmpty() == true)
	{
		QString errorStr = QString("(%1) Error : Failed TestID").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	param.setName(PARAM_TEST_ID);
	param.setType(TestCmdParamType::String);
	param.setValue(testID);
	m_paramList.append(param);

	QString testDescription = argList[1].simplified();
	if(testDescription.isEmpty() == true)
	{
		QString errorStr = QString("(%1) Error : Failed test description").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	param.setName(PARAM_TEST_DESCRIPTION);
	param.setType(TestCmdParamType::String);
	param.setValue(testDescription);
	m_paramList.append(param);

	if (m_foundEndOfTest == false)
	{
		QString errorStr = QString("(%1) Error : Found not finished test").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	m_foundEndOfTest = false;

	return true;
}

bool TestCommand::parseCmdEndtest()
{
	if (m_foundEndOfTest == true)
	{
		QString errorStr = QString("(%1) Error : Found command endtest bun not found command test").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	m_foundEndOfTest = true;

	return true;
}

bool TestCommand::parseCmdSchema()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString schemaID = m_line.right(m_line.length() - spacePos).simplified();
	if(schemaID.isEmpty() == true)
	{
		QString errorStr = QString("(%1) Error : Failed schemaID").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;
	param.setName(PARAM_SCHEMA_ID);
	param.setType(TestCmdParamType::String);
	param.setValue(schemaID);
	m_paramList.append(param);

	return true;
}

bool TestCommand::parseCmdCompatible()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;

	int argCount = argList.count();
	for(int i = 0; i < argCount; i++)
	{
		QString preset = argList[i].simplified();
		if(preset.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed compatible preset").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		param.setName(QString("Preset%1").arg(i));
		param.setType(TestCmdParamType::String);
		param.setValue(preset);
		m_paramList.append(param);
	}

	if (m_errorList.count() != 0)
	{
		return false;
	}

	return true;
}

bool TestCommand::parseCmdConst()
{
	return true;
}

bool TestCommand::parseCmdVar()
{
	return true;
}

bool TestCommand::parseCmdSet()
{
	if (m_pSignalBase == nullptr)
	{
		QString errorStr = QString("(%1) Error : Failed SignalBase").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;

	int argCount = argList.count();
	for(int i = 0; i < argCount; i++)
	{
		QString arg = argList[i].simplified();
		if(arg.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		QStringList sv = arg.split('=');
		if (sv.count() != 2)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		// signal ID
		//
		QString signalID = sv[0].simplified();
		if (signalID.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		if (signalID[0] != '#')
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		TestSignal signal = m_pSignalBase->signal(signalID);
		if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
		{
			QString errorStr = QString("(%1) Error : Signal %2 has not found in the signal base").arg(m_lineIndex).arg(signalID);
			m_errorList.append(errorStr);
			continue;
		}

		if (signal.param().enableTuning() == false)
		{
			QString errorStr = QString("(%1) Error : Signal %2 is not tuning signal").arg(m_lineIndex).arg(signalID);
			m_errorList.append(errorStr);
			continue;
		}

		param.setName(signalID);

		// signal Value
		//
		QString signalValue =  sv[1].simplified();
		if (signalValue.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		switch (signal.param().signalType())
		{
			case E::SignalType::Analog:
				{
					E::AnalogAppSignalFormat type = signal.param().analogSignalFormat();

					switch(type)
					{
						case E::AnalogAppSignalFormat::Float32:
							{
								bool isTypefloat = false;

								param.setType(TestCmdParamType::Float);
								param.setValue(signalValue.toFloat(&isTypefloat));

								if(isTypefloat == false)
								{
									QString errorStr = QString("(%1) Error : Signal %2 failed value of signal").arg(m_lineIndex).arg(signalID);
									m_errorList.append(errorStr);
									continue;
								}
							}
							break;

						case E::AnalogAppSignalFormat::SignedInt32:
							{
								bool isTypeInt = false;

								param.setType(TestCmdParamType::SignedInt32);
								param.setValue(signalValue.toInt(&isTypeInt));

								if(isTypeInt == false)
								{
									QString errorStr = QString("(%1) Error : Signal %2 failed value of signal (type float instead int)").arg(m_lineIndex).arg(signalID);
									m_errorList.append(errorStr);
									continue;
								}
							}
							break;

						default:

							QString errorStr = QString("(%1) Error : Signal %2 failed type of signal (int or float)").arg(m_lineIndex).arg(signalID);
							m_errorList.append(errorStr);
							continue;
					}
				}
				break;

			case E::SignalType::Discrete:
				{
					if (signalValue != "0" &&  signalValue != "1")
					{
						QString errorStr = QString("(%1) Error : Signal %2 failed value (0 or 1)").arg(m_lineIndex).arg(signalID);
						m_errorList.append(errorStr);
						continue;
					}

					param.setType(TestCmdParamType::Discrete);
					param.setValue(signalValue.toInt());
				}
				break;

			default:

				QString errorStr = QString("(%1) Error : Signal %2 failed type of signal (analog or discrete)").arg(m_lineIndex).arg(signalID);
				m_errorList.append(errorStr);
				continue;
		}

		m_paramList.append(param);
	}

	if (m_errorList.count() != 0)
	{
		return false;
	}

	return true;
}

bool TestCommand::parseCmdCheck()
{
	if (m_pSignalBase == nullptr)
	{
		QString errorStr = QString("(%1) Error : Failed SignalBase").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;

	int argCount = argList.count();
	for(int i = 0; i < argCount; i++)
	{
		QString arg = argList[i].simplified();
		if(arg.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		QStringList sv = arg.split("==");
		if (sv.count() != 2)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		// signal ID
		//
		QString signalID = sv[0].simplified();
		if (signalID.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		if (signalID[0] != '#')
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		TestSignal signal = m_pSignalBase->signal(signalID);
		if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
		{
			QString errorStr = QString("(%1) Error : Signal %2 has not found in the signal base").arg(m_lineIndex).arg(signalID);
			m_errorList.append(errorStr);
			continue;
		}

		param.setName(signalID);

		// signal Value
		//
		QString signalValue =  sv[1].simplified();
		if (signalValue.isEmpty() == true)
		{
			QString errorStr = QString("(%1) Error : Failed argument list").arg(m_lineIndex);
			m_errorList.append(errorStr);
			continue;
		}

		switch (signal.param().signalType())
		{
			case E::SignalType::Analog:
				{
					E::AnalogAppSignalFormat type = signal.param().analogSignalFormat();

					switch(type)
					{
						case E::AnalogAppSignalFormat::Float32:
							{
								bool isTypefloat = false;

								param.setType(TestCmdParamType::Float);
								param.setValue(signalValue.toFloat(&isTypefloat));

								if(isTypefloat == false)
								{
									QString errorStr = QString("(%1) Error : Signal %2 failed value of signal").arg(m_lineIndex).arg(signalID);
									m_errorList.append(errorStr);
									continue;
								}
							}
							break;

						case E::AnalogAppSignalFormat::SignedInt32:
							{
								bool isTypeInt = false;

								param.setType(TestCmdParamType::SignedInt32);
								param.setValue(signalValue.toInt(&isTypeInt));

								if(isTypeInt == false)
								{
									QString errorStr = QString("(%1) Error : Signal %2 failed value of signal (type float instead int)").arg(m_lineIndex).arg(signalID);
									m_errorList.append(errorStr);
									continue;
								}
							}
							break;

						default:

							QString errorStr = QString("(%1) Error : Signal %2 failed type of signal (int or float) ").arg(m_lineIndex).arg(signalID);
							m_errorList.append(errorStr);
							continue;
					}
				}
				break;

			case E::SignalType::Discrete:
				{
					if (signalValue != "0" &&  signalValue != "1")
					{
						QString errorStr = QString("(%1) Error : Signal %2 failed value (0 or 1)").arg(m_lineIndex).arg(signalID);
						m_errorList.append(errorStr);
						continue;
					}

					param.setType(TestCmdParamType::Discrete);
					param.setValue(signalValue.toInt());
				}
				break;

			default:

				QString errorStr = QString("(%1) Error : Signal %2 failed type of signal (analog or discrete)").arg(m_lineIndex).arg(signalID);
				m_errorList.append(errorStr);
				continue;
		}

		m_paramList.append(param);
	}

	if (m_errorList.count() != 0)
	{
		return false;
	}

	return true;
}

bool TestCommand::parseCmdDelay()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(%1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString ms = m_line.right(m_line.length() - spacePos).simplified();
	if(ms.isEmpty() == true)
	{
		QString errorStr = QString("(%1) Error : Failed argument").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int delay_ms = ms.sprintf("%0.0f", ms.toDouble()).toInt();

	TestCmdParam param;
	param.setName("Delay(ms)");
	param.setType(TestCmdParamType::SignedInt32);
	param.setValue(delay_ms);
	m_paramList.append(param);

	return true;
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

void TestFile::printErrorlist()
{
	// print error list
	//
	int errorCount = m_errorList.count();
	for(int i = 0; i < errorCount; i++)
	{
		qDebug() << m_errorList[i];
	}

	qDebug() << "Found error(s):" << errorCount;
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

	if (m_pSignalBase == nullptr)
	{
		qDebug() << "Error: Failed SignalBase";
		return false;
	}

	//
	//
	TestCommand testCmd(m_pSignalBase);

	// parse file
	//
	while(m_file.atEnd() == false)
	{
		QString line = m_file.readLine();
		if (testCmd.parse(line) == false)
		{
			m_errorList.append(testCmd.errorList());
			continue;
		}

		m_commandList.append(testCmd);
	}

	if (testCmd.foundEndOfTest() == false)
	{
		QString errorStr = QString("(%1) Error : Found not finished test").arg(testCmd.lineIndex());
		m_errorList.append(errorStr);
	}

	// print error list
	//
	if (m_errorList.count() != 0)
	{
		printErrorlist();
		return false;
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

int TestFile::cmdCount()
{
	int count = 0;

	m_mutex.lock();

		count = m_commandList.count();

	m_mutex.unlock();

	return count;
}

TestCommand TestFile::cmd(int index)
{
	TestCommand cmd;

	m_mutex.lock();

		if (index >= 0 && index < m_commandList.count())
		{
			cmd = m_commandList[index];
		}

	m_mutex.unlock();

	return cmd;

}
