#include "TestFile.h"
#include <QDebug>

// -------------------------------------------------------------------------------------------------------------------
//
// TestCmdParam class implementation
//
// -------------------------------------------------------------------------------------------------------------------

bool TestCmdParam::isEmtpy()
{
	if (m_name.isEmpty() == true)
	{
		return true;
	}

	if (m_type == TestCmdParamType::Undefined)
	{
		return true;
	}

	return false;
}

void TestCmdParam::clear()
{
	m_name.clear();
	m_flag = TF_UNDEFINED_FLAG;
	m_type = TestCmdParamType::Undefined;
	m_value.clear();
}

bool TestCmdParam::getFlag(QString& signalID)
{
	m_flag = TF_UNDEFINED_FLAG;

	if (signalID.indexOf(":") == -1)
	{
		return true;
	}

	QStringList signalFlags = signalID.split(":");

	// update signalID
	//
	signalID = signalFlags[0].simplified();

	// get name of flag
	//
	QString flagName = signalFlags[1].simplified();

	QMetaEnum asf = QMetaEnum::fromType<E::AppSignalStateFlagType>();
	for(int f = 0; f < asf.keyCount(); f++)
	{
		if (asf.key(f) == flagName)
		{
			m_flag = f;
			break;
		}
	}

	if (m_flag == TF_UNDEFINED_FLAG)
	{
		return false;
	}

	return true;
}

QString TestCmdParam::valueStr(bool addParamName, int precise)
{
	QString str;

	switch (m_type)
	{
		case TestCmdParamType::Undefined:

			str = "???";
			break;

		case TestCmdParamType::Discrete:
		case TestCmdParamType::SignedInt32:
		case TestCmdParamType::SignedInt64:

			str = QString("%1").arg(m_value.toInt());
			break;

		case TestCmdParamType::Float:
		case TestCmdParamType::Double:
			{
				QString formatStr;
				formatStr.sprintf(("%%.%df"), precise);

				str.sprintf(formatStr.toUtf8(), m_value.toDouble());

				// remove unnecessary 0
				//
				int pos = str.indexOf('.');
				if (pos == -1)
				{
					break;
				}

				while(str[str.length() - 1] == '0')
				{
					if (str.length() - 2 == pos)
					{
						break;
					}

					str.remove(str.length() - 1, 1);
				}
			}
			break;
		case TestCmdParamType::String:

			str = m_value.toString();
			break;

		default:

			assert(false);
			break;
	}

	if (addParamName == true)
	{
		if (m_flag == TF_UNDEFINED_FLAG)
		{
			str = m_name + "=" + str;
		}
		else
		{
			QMetaEnum asf = QMetaEnum::fromType<E::AppSignalStateFlagType>();

			str = m_name + ":" + asf.key(m_flag) + "=" + str;
		}

	}

	return str;
}

bool TestCmdParam::compare(const AppSignalState& state)
{
	bool result = false;

	if (isFlag() == false)
	{
		QVariant cmpValue = state.value();

		switch (m_type)
		{
			case TestCmdParamType::Undefined:

				result = false;
				break;

			case TestCmdParamType::Discrete:
			case TestCmdParamType::SignedInt32:
			case TestCmdParamType::SignedInt64:

				result = m_value.toInt() == cmpValue.toInt();
				break;

			case TestCmdParamType::Float:
				{
					float lFloat = m_value.toFloat();
					float rFloat = cmpValue.toFloat();

					// nan
					//
					if (std::isnan(lFloat) == true && std::isnan(rFloat) == true)
					{
						result = true;
						break;
					}

					// inf
					//
					if (std::isinf(lFloat) == true && std::isinf(rFloat) == true)
					{
						result = true;
						break;
					}

					// simple float digit
					//
					result = std::nextafter(lFloat, std::numeric_limits<float>::lowest()) <= rFloat && std::nextafter(lFloat, std::numeric_limits<float>::max()) >= rFloat;
				}
				break;

			case TestCmdParamType::Double:
				{
					double lDouble = m_value.toDouble();
					double rDouble = cmpValue.toDouble();

					// nan
					//
					if (std::isnan(lDouble) == true && std::isnan(rDouble) == true)
					{
						result = true;
						break;
					}

					// inf
					//
					if (std::isinf(lDouble) == true && std::isinf(rDouble) == true)
					{
						result = true;
						break;
					}

					// simple double digit
					//
					result = std::nextafter(lDouble, std::numeric_limits<double>::lowest()) <= rDouble && std::nextafter(lDouble, std::numeric_limits<double>::max()) >= rDouble;
				}
				break;

			case TestCmdParamType::String:

				result = m_value.toString() == cmpValue.toString();
				break;

			default:

				assert(false);
				break;
		}
	}
	else
	{
		if (m_type == TestCmdParamType::SignedInt32)
		{
			switch (static_cast<E::AppSignalStateFlagType>(m_flag))
			{
				case E::AppSignalStateFlagType::Validity:		result = m_value.toBool() == state.isValid();			break;
				case E::AppSignalStateFlagType::StateAvailable:	result = m_value.toBool() == state.isStateAvailable();	break;
				case E::AppSignalStateFlagType::Simulated:		result = m_value.toBool() == state.isSimulated();		break;
				case E::AppSignalStateFlagType::Blocked:		result = m_value.toBool() == state.isBlocked();			break;
				case E::AppSignalStateFlagType::Mismatch:		result = m_value.toBool() == state.isMismatch();		break;
				case E::AppSignalStateFlagType::AboveHighLimit:	result = m_value.toBool() == state.isAboveHighLimit();	break;
				case E::AppSignalStateFlagType::BelowLowLimit:	result = m_value.toBool() == state.isBelowLowLimit();	break;
				default:										assert(0); result = false;								break;
			}

		}
	}

	return result;
}

TestCmdParam& TestCmdParam::operator=(const TestCmdParam& from)
{
	m_name = from.m_name;
	m_flag = from.m_flag;
	m_type = from.m_type;
	m_value = from.m_value;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
//
// TestFileLine class implementation
//
// -------------------------------------------------------------------------------------------------------------------

const char* const TestCmd::PARAM_TEST_ID = "TestID";
const char* const TestCmd::PARAM_TEST_DESCRIPTION = "TestDescription";
const char* const TestCmd::PARAM_SCHEMA_ID = "SchemaID";
const char* const TestCmd::PARAM_COMPATIBLE = "Compatible";
const char* const TestCmd::PARAM_SOURCE_ID = "SourceID";

TestCmd::TestCmd()
{
}

TestCmd::TestCmd(TestFile* pTestFile, SignalBase* pSignalBase)
	: m_pTestFile(pTestFile)
	, m_pSignalBase(pSignalBase)
	, m_lineIndex(0)
	, m_foundEndOfTest(true)
{
}

TestCmd::~TestCmd()
{
}

int TestCmd::getCmdType(const QString& line)
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
		if (cmd == TestFileCmd[type])
		{
			cmdType = type;
			break;
		}
	}

	return cmdType;
}

bool TestCmd::parse(const QString& line)
{
	m_line = line;
	m_comment.clear();
	m_paramList.clear();
	m_errorList.clear();
	m_lineIndex ++;

	// remove unnecessary characters
	//
	m_line.remove('\r');
	m_line.remove('\n');
	m_line.replace('\t', ' ');
	m_line = m_line.simplified();

	// remove all after comment "//"
	//
	int pos = m_line.indexOf("//");
	if (pos != -1)
	{
		m_comment = m_line.right(m_line.length() - pos);
		m_line.remove(pos, m_line.length() - pos);
	}

	// parse line
	//
	m_type = getCmdType(m_line);
	if (m_type < 0 || m_type >= TF_CMD_COUNT)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	bool resultCmd = true;

	switch (m_type)
	{
		case TF_CMD_TEST:			resultCmd = parseCmdTest();			break;
		case TF_CMD_ENDTEST:		resultCmd = parseCmdEndtest();		break;
		case TF_CMD_SCHEMA:			resultCmd = parseCmdSchema();		break;
		case TF_CMD_COMPATIBLE:		resultCmd = parseCmdCompatible();	break;
		case TF_CMD_CONST:			resultCmd = parseCmdConst();		break;
		case TF_CMD_VAR:			resultCmd = parseCmdVar();			break;
		case TF_CMD_SET:			resultCmd = parseCmdSet();			break;
		case TF_CMD_CHECK:			resultCmd = parseCmdCheck();		break;
		case TF_CMD_DELAY:			resultCmd = parseCmdDelay();		break;
		case TF_CMD_RUN_SOURCE:		resultCmd = parseCmdRunSource();	break;
		case TF_CMD_STOP_SOURCE:	resultCmd = parseCmdStopSource();	break;
		case TF_CMD_EXIT_PS:		resultCmd = parseCmdExitPS();		break;
	}

	return resultCmd;
}

bool TestCmd::parseCmdTest()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() != 2)
	{
		QString errorStr = QString("(line %1) Error : Failed argument list").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;

	QString testID = argList[0].simplified();
	if(testID.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Failed TestID").arg(m_lineIndex);
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
		QString errorStr = QString("(line %1) Error : Failed test description").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	param.setName(PARAM_TEST_DESCRIPTION);
	param.setType(TestCmdParamType::String);
	param.setValue(testDescription);
	m_paramList.append(param);

	if (m_foundEndOfTest == false)
	{
		QString errorStr = QString("(line %1) Error : Found not finished test").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	m_foundEndOfTest = false;

	if (isUniqueTestID(testID) == false) // check unique TestID
	{
		return false;
	}

	return true;
}

bool TestCmd::parseCmdEndtest()
{
	if (m_foundEndOfTest == true)
	{
		QString errorStr = QString("(line %1) Error : Found command \"endtest\", bun not found command \"test\"").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	m_foundEndOfTest = true;

	return true;
}

bool TestCmd::parseCmdSchema()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString schemaID = m_line.right(m_line.length() - spacePos).simplified();
	if(schemaID.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Failed schemaID").arg(m_lineIndex);
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

bool TestCmd::parseCmdCompatible()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(line %1) Error : Argument list is empty").arg(m_lineIndex);
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
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		param.setName(PARAM_COMPATIBLE);
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

bool TestCmd::parseCmdConst()
{
	int space1Pos = m_line.indexOf(' ');
	if (space1Pos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	int space2Pos = m_line.indexOf(' ', space1Pos + 1);
	if (space2Pos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed const type").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	QString constTypeStr = m_line;
	constTypeStr.remove(space2Pos, m_line.length() - space2Pos);
	constTypeStr = constTypeStr.right(constTypeStr.length() - space1Pos).simplified();

	TestCmdParamType constType = TestCmdParamType::Undefined;

	if (constTypeStr == "int")
	{
		constType = TestCmdParamType::SignedInt32;
	}

	if (constTypeStr == "float")
	{
		constType = TestCmdParamType::Float;
	}

	if (constType == TestCmdParamType::Undefined)
	{
		QString errorStr = QString("(line %1) Error : Failed const type").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - space2Pos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(line %1) Error : Argument list is empty").arg(m_lineIndex);
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
			QString errorStr = QString("(line %1) Error : Argument %2 is empty").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		QStringList sv = arg.split('=');
		if (sv.count() != 2)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		// const name
		//
		QString constName = sv[0].simplified();
		if (constName.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		if (isUniqueConstOrVarName(constName, m_paramList) == false)
		{
			continue;
		}

		param.setName(constName);

		// const Value
		//
		QString constValue = sv[1].simplified();
		if (constValue.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		param.setType(constType);

		switch (constType)
		{
			case TestCmdParamType::Float:
				{
					bool isTypefloat = false;

					param.setValue(constValue.toFloat(&isTypefloat));

					if(isTypefloat == false)
					{
						QString errorStr = QString("(line %1) Error : Const %2 failed value").arg(m_lineIndex).arg(constName);
						m_errorList.append(errorStr);
						continue;
					}
				}
				break;

			case TestCmdParamType::SignedInt32:
				{
					bool isTypeInt = false;

					param.setValue(constValue.toInt(&isTypeInt));

					if(isTypeInt == false)
					{
						QString errorStr = QString("(line %1) Error : Const %2 failed value").arg(m_lineIndex).arg(constName);
						m_errorList.append(errorStr);
						continue;
					}
				}
				break;

			default:

				QString errorStr = QString("(line %1) Error : Const %2 failed type (int or float)").arg(m_lineIndex).arg(constName);
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

bool TestCmd::parseCmdVar()
{
	int space1Pos = m_line.indexOf(' ');
	if (space1Pos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	int space2Pos = m_line.indexOf(' ', space1Pos + 1);
	if (space2Pos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed variable type").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	QString varTypeStr = m_line;
	varTypeStr.remove(space2Pos, m_line.length() - space2Pos);
	varTypeStr = varTypeStr.right(varTypeStr.length() - space1Pos).simplified();

	TestCmdParamType varType = TestCmdParamType::Undefined;

	if (varTypeStr == "int")
	{
		varType = TestCmdParamType::SignedInt32;
	}

	if (varTypeStr == "float")
	{
		varType = TestCmdParamType::Float;
	}

	if (varType == TestCmdParamType::Undefined)
	{
		QString errorStr = QString("(line %1) Error : Failed variable type").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - space2Pos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(line %1) Error : Argument list is empty").arg(m_lineIndex);
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
			QString errorStr = QString("(line %1) Error : Argument %2 is empty").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		QStringList sv = arg.split('=');
		if (sv.count() != 2)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		// variable name
		//
		QString varName = sv[0].simplified();
		if (varName.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		if (isUniqueConstOrVarName(varName, m_paramList) == false)
		{
			continue;
		}

		param.setName(varName);

		// variable Value
		//
		QString varValue =  sv[1].simplified();
		if (varValue.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		param.setType(varType);

		switch (varType)
		{
			case TestCmdParamType::Float:
				{
					bool isTypefloat = false;

					param.setValue(varValue.toFloat(&isTypefloat));

					if(isTypefloat == false)
					{
						QString errorStr = QString("(line %1) Error : Variable %2 failed value").arg(m_lineIndex).arg(varName);
						m_errorList.append(errorStr);
						continue;
					}
				}
				break;

			case TestCmdParamType::SignedInt32:
				{
					bool isTypeInt = false;

					param.setValue(varValue.toInt(&isTypeInt));

					if(isTypeInt == false)
					{
						QString errorStr = QString("(line %1) Error : Variable %2 failed value").arg(m_lineIndex).arg(varName);
						m_errorList.append(errorStr);
						continue;
					}
				}
				break;

			default:

				QString errorStr = QString("(line %1) Error : Variable %2 failed type (int or float)").arg(m_lineIndex).arg(varName);
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

bool TestCmd::parseCmdSet()
{
	if (m_pSignalBase == nullptr)
	{
		QString errorStr = QString("(line %1) Error : Failed SignalBase").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(line %1) Error : Argument list is empty").arg(m_lineIndex);
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
			QString errorStr = QString("(line %1) Error : Argument %2 is empty").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		QStringList sv = arg.split('=');
		if (sv.count() != 2)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		// signal ID
		//
		QString signalID = sv[0].simplified();
		if (signalID.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		if (signalID[0] != '#')
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		TestSignal signal = m_pSignalBase->signal(signalID);
		if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == UNDEFINED_HASH)
		{
			QString errorStr = QString("(line %1) Error : Signal %2 has not found in the signal base").arg(m_lineIndex).arg(signalID);
			m_errorList.append(errorStr);
			continue;
		}

//		if (signal.param().enableTuning() == false)
//		{
//			QString errorStr = QString("(line %1) Error : Signal %2 is not tuning signal").arg(m_lineIndex).arg(signalID);
//			m_errorList.append(errorStr);
//			continue;
//		}

		// signal Value
		//
		QString signalValue =  sv[1].simplified();
		if (signalValue.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		//
		//
		bool signalValueIsDigit = false;
		signalValue.toFloat(&signalValueIsDigit);

		if (signalValueIsDigit == true)
		{
			param = paramFromSignal(signalID, signalValue, signal.param());
		}
		else
		{
			param = paramFromConstOrVar(signalID, signalValue, signal.param());
		}

		if (param.isEmtpy() == true)
		{
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

bool TestCmd::parseCmdCheck()
{
	if (m_pSignalBase == nullptr)
	{
		QString errorStr = QString("(line %1) Error : Failed SignalBase").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString args = m_line.right(m_line.length() - spacePos).simplified();
	QStringList argList = args.split(',');

	if (argList.count() == 0)
	{
		QString errorStr = QString("(line %1) Error : Argument list is empty").arg(m_lineIndex);
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
			QString errorStr = QString("(line %1) Error : Argument %2 is empty").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		QStringList sv = arg.split("==");
		if (sv.count() != 2)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		// signal ID
		//
		QString signalID = sv[0].simplified();
		if (signalID.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		if (signalID[0] != '#')
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		// if signal ID has flags
		//
		if (param.getFlag(signalID) == false)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		// find signal by signalID
		//
		TestSignal signal = m_pSignalBase->signal(signalID);
		if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == UNDEFINED_HASH)
		{
			QString errorStr = QString("(line %1) Error : Signal %2 has not found in the signal base").arg(m_lineIndex).arg(signalID);
			m_errorList.append(errorStr);
			continue;
		}

		// signal Value
		//
		QString signalValue = sv[1].simplified();
		if (signalValue.isEmpty() == true)
		{
			QString errorStr = QString("(line %1) Error : Failed argument: %2").arg(m_lineIndex).arg(i+1);
			m_errorList.append(errorStr);
			continue;
		}

		//
		//
		if (param.isFlag() == false)
		{
			bool signalValueIsDigit = false;
			signalValue.toFloat(&signalValueIsDigit);

			if (signalValueIsDigit == true)
			{
				param = paramFromSignal(signalID, signalValue, signal.param());
			}
			else
			{
				param = paramFromConstOrVar(signalID, signalValue, signal.param());
			}
		}
		else
		{
			if (signalValue != "0" && signalValue != "1")
			{
				QString errorStr = QString("(line %1) Error : Signal %2 failed value of flag (0 or 1)").arg(m_lineIndex).arg(signal.param().appSignalID());
				m_errorList.append(errorStr);
				continue;
			}

			param.setName(signal.param().appSignalID());
			param.setType(TestCmdParamType::SignedInt32);
			param.setValue(signalValue.toInt());
		}

		if (param.isEmtpy() == true)
		{
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

bool TestCmd::parseCmdDelay()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString ms = m_line.right(m_line.length() - spacePos).simplified();
	if(ms.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Failed argument").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	quint32 delay_ms = ms.sprintf("%0.0f", ms.toDouble()).toUInt(); // float is rounded to int

	TestCmdParam param;

	param.setName("Delay");
	param.setType(TestCmdParamType::SignedInt32);
	param.setValue(delay_ms);

	m_paramList.append(param);

	return true;
}

bool TestCmd::parseCmdRunSource()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString sourceID = m_line.right(m_line.length() - spacePos).simplified();
	if(sourceID.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Failed sourceID").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;
	param.setName(PARAM_SOURCE_ID);
	param.setType(TestCmdParamType::String);
	param.setValue(sourceID);
	m_paramList.append(param);

	return true;
}

bool TestCmd::parseCmdStopSource()
{
	int spacePos = m_line.indexOf(' ');
	if (spacePos == -1)
	{
		QString errorStr = QString("(line %1) Error : Failed command - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	QString sourceID = m_line.right(m_line.length() - spacePos).simplified();
	if(sourceID.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Failed sourceID").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	TestCmdParam param;
	param.setName(PARAM_SOURCE_ID);
	param.setType(TestCmdParamType::String);
	param.setValue(sourceID);
	m_paramList.append(param);

	return true;
}

bool TestCmd::parseCmdExitPS()
{
	if (m_line.length() > static_cast<int>(strlen(TestFileCmd[TF_CMD_EXIT_PS])))
	{
		QString errorStr = QString("(line %1) Error : Failed params - %2").arg(m_lineIndex).arg(m_line);
		m_errorList.append(errorStr);
		return false;
	}

	return true;
}

bool TestCmd::isUniqueTestID(const QString& testID)
{
	if (testID.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : TestID is empty").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	if (m_pTestFile == nullptr)
	{
		QString errorStr = QString("(line %1) Error : Failed test file").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int cmdCount = m_pTestFile->commandList().count();
	for(int c = 0; c < cmdCount; c++)
	{
		TestCmd cmd = m_pTestFile->commandList().at(c);
		if (cmd.type() != TF_CMD_TEST)
		{
			continue;
		}

		int paramCount = cmd.paramList().count();
		for(int p = 0; p < paramCount; p++)
		{
			TestCmdParam param = cmd.paramList().at(p);
			if (param.name() != PARAM_TEST_ID)
			{
				continue;
			}

			if (param.value() == testID)
			{
				QString errorStr = QString("(line %1) Error : TestID %2 is not unique in the test file").arg(m_lineIndex).arg(testID);
				m_errorList.append(errorStr);
				return false;
			}
		}
	}

	return true;
}

bool TestCmd::isUniqueConstOrVarName(const QString& name, const QVector<TestCmdParam>& paramList)
{
	if (m_pTestFile == nullptr)
	{
		QString errorStr = QString("(line %1) Error : Failed test file").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	if (name.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Const or variable name is empty").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return false;
	}

	int paramCount = paramList.count();
	for(int i = 0; i < paramCount; i++)
	{
		if (paramList[i].name() == name)
		{
			QString errorStr = QString("(line %1) Error : Const or variable %2 is not unique in the test file").arg(m_lineIndex).arg(name);
			m_errorList.append(errorStr);
			return false;
		}
	}

	int cmdCount = m_pTestFile->commandList().count();
	for(int c = 0; c < cmdCount; c++)
	{
		TestCmd cmd = m_pTestFile->commandList().at(c);
		if (cmd.type() != TF_CMD_CONST && cmd.type() != TF_CMD_VAR)
		{
			continue;
		}

		int prmCount = cmd.paramList().count();
		for(int p = 0; p < prmCount; p++)
		{
			if (cmd.paramList().at(p).name() == name)
			{
				QString errorStr = QString("(line %1) Error : Const or variable %2 is not unique in the test file").arg(m_lineIndex).arg(name);
				m_errorList.append(errorStr);
				return false;
			}
		}
	}

	return true;
}

TestCmdParam TestCmd::paramFromConstOrVar(const QString& name, const QString& value, const Signal& signal)
{
	if (m_pTestFile == nullptr)
	{
		QString errorStr = QString("(line %1) Error : Failed test file").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	if (name.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Param name is empty").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	if (value.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Param value is empty").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	TestCmdParam param;

	int cmdCount = m_pTestFile->commandList().count();
	for(int c = 0; c < cmdCount; c++)
	{
		TestCmd cmd = m_pTestFile->commandList().at(c);
		if (cmd.type() != TF_CMD_CONST && cmd.type() != TF_CMD_VAR)
		{
			continue;
		}

		int paramCount = cmd.paramList().count();
		for(int p = 0; p < paramCount; p++)
		{
			if (cmd.paramList().at(p).name() == value)
			{
				param = cmd.paramList().at(p);
				break;
			}
		}
	}

	if (param.isEmtpy() == true)
	{
		QString errorStr = QString("(line %1) Error : Unknown name of const or var: %2").arg(m_lineIndex).arg(value);
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	//
	//
	bool typeOk = false;

	switch (signal.signalType())
	{
		case E::SignalType::Analog:
			{
				switch(signal.analogSignalFormat())
				{
					case E::AnalogAppSignalFormat::SignedInt32:

						if (param.type() != TestCmdParamType::SignedInt32)
						{
							break;
						}

						typeOk = true;

						break;

					case E::AnalogAppSignalFormat::Float32:

						if (param.type() != TestCmdParamType::Float)
						{
							break;
						}

						typeOk = true;

						break;
				}
			}
			break;

		case E::SignalType::Discrete:

			if (param.type() != TestCmdParamType::SignedInt32)
			{
				break;
			}

			if (param.value() != 0 && param.value() != 1)
			{
				break;
			}

			typeOk = true;

			break;
	}

	if (typeOk == false)
	{
		QString errorStr = QString("(line %1) Error : Non-combination of types for const or variable: %2").arg(m_lineIndex).arg(param.name());
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	param.setName(name);

	return param;
}

TestCmdParam TestCmd::paramFromSignal(const QString& name, const QString& value, const Signal& signal)
{
	if (name.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Param name is empty").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	if (value.isEmpty() == true)
	{
		QString errorStr = QString("(line %1) Error : Param value is empty").arg(m_lineIndex);
		m_errorList.append(errorStr);
		return TestCmdParam();
	}

	TestCmdParam param;

	switch (signal.signalType())
	{
		case E::SignalType::Analog:
			{
				switch(signal.analogSignalFormat())
				{
					case E::AnalogAppSignalFormat::SignedInt32:
						{
							bool isTypeInt = false;

							param.setType(TestCmdParamType::SignedInt32);
							param.setValue(value.toInt(&isTypeInt));

							if(isTypeInt == false)
							{
								QString errorStr = QString("(line %1) Error : Signal %2 failed value of signal (type Float32 instead SignedInt32)").arg(m_lineIndex).arg(signal.appSignalID());
								m_errorList.append(errorStr);
								return TestCmdParam();
							}
						}
						break;

					case E::AnalogAppSignalFormat::Float32:
						{
							bool isTypefloat = false;

							param.setType(TestCmdParamType::Float);
							param.setValue(value.toFloat(&isTypefloat));

							if(isTypefloat == false)
							{
								QString errorStr = QString("(line %1) Error : Signal %2 failed value of signal").arg(m_lineIndex).arg(signal.appSignalID());
								m_errorList.append(errorStr);
								return TestCmdParam();
							}
						}
						break;

					default:

						QString errorStr = QString("(line %1) Error : Signal %2 failed type of signal (SignedInt32 or Float32)").arg(m_lineIndex).arg(signal.appSignalID());
						m_errorList.append(errorStr);
						return TestCmdParam();
				}
			}
			break;

		case E::SignalType::Discrete:
			{
				if (value != "0" &&  value != "1")
				{
					QString errorStr = QString("(line %1) Error : Signal %2 failed value (0 or 1)").arg(m_lineIndex).arg(signal.appSignalID());
					m_errorList.append(errorStr);
					return TestCmdParam();
				}

				param.setType(TestCmdParamType::Discrete);
				param.setValue(value.toInt());
			}
			break;

		default:

			QString errorStr = QString("(line %1) Error : Signal %2 failed type of signal (analog or discrete)").arg(m_lineIndex).arg(signal.appSignalID());
			m_errorList.append(errorStr);
			return TestCmdParam();
	}

	param.setName(name);

	return param;
}

// -------------------------------------------------------------------------------------------------------------------
//
// TestItem class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TestItem::TestItem()
{
}

TestItem::TestItem(const TestItem& from)
{
	*this = from;
}

TestItem::~TestItem()
{
}

int TestItem::cmdCount()
{
	int count = 0;

	m_mutex.lock();

		count = m_commandList.count();

	m_mutex.unlock();

	return count;
}

TestCmd TestItem::cmd(int index)
{
	TestCmd cmd;

	m_mutex.lock();

		if (index >= 0 && index < m_commandList.count())
		{
			cmd = m_commandList[index];
		}

	m_mutex.unlock();

	return cmd;
}

void TestItem::appendCmd(const TestCmd& cmd)
{
	m_mutex.lock();

		m_commandList.append(cmd);

	m_mutex.unlock();
}

void TestItem::appendResult(const QString& str, bool printDebugMsg)
{
	m_resultList.append(str);

	if (printDebugMsg == true)
	{
		std::cout << str.toLocal8Bit().constData() << std::endl;
	}
}

TestItem& TestItem::operator=(const TestItem& from)
{
	m_testID = from.m_testID;
	m_name = from.m_name;
	m_compatibleList = from.m_compatibleList;
	m_errorCount = from.m_errorCount;

	m_commandList = from.m_commandList;
	m_resultList = from.m_resultList;

	return *this;
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
	if (m_errorList.isEmpty() == true)
	{
		return;
	}

	m_errorList.append(QString("Found error(s): %1").arg(m_errorList.count()));

	int errorCount = m_errorList.count();
	for(int i = 0; i < errorCount; i++)
	{
		std::cout << m_errorList[i].toLocal8Bit().constData() << std::endl;
	}
}

bool TestFile::parse(const QString& fileName, SignalBase* pSignalBase)
{
	if (fileName.isEmpty() == true)
	{
		std::cout << "Error: Test file name is empty" << std::endl;
		return false;
	}

	if (pSignalBase == nullptr)
	{
		std::cout << "Error: Failed SignalBase" << std::endl;
		return false;
	}

	m_file.setFileName(fileName);
	if (m_file.open(QIODevice::ReadOnly) == false)
	{
		std::cout << "Error: Test file " << m_fileName.toLocal8Bit().constData() << " is not open" << std::endl;
		m_errorList.append("Error: Test file " + m_fileName + " is not open\n");
		return false;
	}

	m_fileName = fileName;
	m_pSignalBase = pSignalBase;

	//
	//
	TestCmd testCmd(this, m_pSignalBase);

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
		QString errorStr = QString("(line %1) Error : Found not finished test").arg(testCmd.lineIndex());
		m_errorList.append(errorStr);
	}

	// print error list
	//
	if (m_errorList.count() != 0)
	{
		printErrorlist();
		return false;
	}

	createTestList();

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

void TestFile::createTestList()
{
	int cmdIndex = 0;
	int cmdCount = m_commandList.count();

	while(cmdIndex < cmdCount)
	{
		TestItem test;

		TestCmd& cmd = m_commandList[cmdIndex];
		if (cmd.type() == TF_CMD_TEST)
		{
			//
			//
			QString testName;

			int paramCount = cmd.paramList().count();
			for(int i = 0; i < paramCount; i++)
			{
				TestCmdParam param = cmd.paramList().at(i);
				if (param.type() != TestCmdParamType::String)
				{
					continue;
				}

				testName.append(cmd.paramList().at(i).value().toString());

				if (i == 0)
				{
					test.setTestID(cmd.paramList().at(i).value().toString());
					testName.append(", ");
				}
			}

			test.setName(testName);

			cmd.setComment(QString());

			//
			//
			while (cmdIndex < cmdCount)
			{
				cmd = m_commandList[cmdIndex];
				test.appendCmd(cmd);

				if (cmd.type() == TF_CMD_COMPATIBLE)
				{
					int prmCount = cmd.paramList().count();
					for(int i = 0; i < prmCount; i++)
					{
						test.compatibleList().append(cmd.paramList().at(i).value().toString());
					}
				}

				if (cmd.type() == TF_CMD_ENDTEST)
				{
					m_testList.append(test);
					break;
				}

				cmdIndex++;
			}
		}
		cmdIndex++;
	}
}
