#include "UalTester.h"

#include <QDebug>
#include <QFileInfo>

#include "../../lib/CommandLineParser.h"
#include "../../lib/SocketIO.h"
#include "../../lib/XmlHelper.h"
#include "../../lib/Signal.h"

// -------------------------------------------------------------------------------------------------------------------
//
// UalTester class implementation
//
// -------------------------------------------------------------------------------------------------------------------

const char* const UalTester::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
const char* const UalTester::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";
const char* const UalTester::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const UalTester::SETTING_TEST_FILE_NAME  = "TestFileName";

const char* const UalTester::SETTING_ERROR_IGNORE = "ErrorIngnore";
const char* const UalTester::SETTING_TEST_ID = "TestID";
const char* const UalTester::SETTING_FROM_TEST_ID = "FromTestID";
const char* const UalTester::SETTING_TRACE = "Trace";
const char* const UalTester::SETTING_REPORT = "ReportFileName";
const char* const UalTester::SETTING_PRESET_LM = "TestFileName";

UalTester::UalTester(int& argc, char** argv) :
	m_waitSocketsConnectionTimer(this)
{
	getCmdLineParams(argc, argv);
}

UalTester::~UalTester()
{
}

bool UalTester::printToReportFile(const QString& msg)
{
	if (m_reportFileName.isEmpty() == true)
	{
		return false;
	}

	QFile reportFile(m_reportFileName);
	if (reportFile.open(QIODevice::Append) == false)
	{
		return false;
	}

	qint64 written = reportFile.write(msg.toUtf8() + "\r\n");
	if (written != msg.length())
	{
		return false;
	}

	reportFile.close();

	return true;
}


void UalTester::getCmdLineParams(int& argc, char** argv)
{
	CommandLineParser cmdLineParser(argc, argv);

	// main settings
	//
	cmdLineParser.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	cmdLineParser.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");
	cmdLineParser.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "EquipmentID.", "EQUIPMENT_ID");
	cmdLineParser.addSingleValueOption("f", SETTING_TEST_FILE_NAME, "Test file name", "TestFileName");

	// optional settings
	//
	cmdLineParser.addSingleValueOption("errignore", SETTING_ERROR_IGNORE, "Stop testing if errors are found.", "No");
	cmdLineParser.addSingleValueOption("test", SETTING_TEST_ID, "Run a specific test", "TEST_ID");
	cmdLineParser.addSingleValueOption("from", SETTING_FROM_TEST_ID, "Starting from the specified test", "TEST_ID");
	cmdLineParser.addSingleValueOption("trace", SETTING_TRACE, "Line test execution report", "No");
	cmdLineParser.addSingleValueOption("report", SETTING_REPORT, "Report file name", "ReportFileName");
	cmdLineParser.addSingleValueOption("lm", SETTING_PRESET_LM, "Run only tests compatible with the specified LM preset", "LM_ID");

	cmdLineParser.parse();

	// main settings
	//
	m_cfgServiceIP1 = cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP1);
	m_cfgServiceIP2 = cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP2);
	m_equipmentID = cmdLineParser.settingValue(SETTING_EQUIPMENT_ID);
	m_testFileName = cmdLineParser.settingValue(SETTING_TEST_FILE_NAME);

	// optional settings
	//
	m_errorIngnoreStr = cmdLineParser.settingValue(SETTING_ERROR_IGNORE);
	m_testID = cmdLineParser.settingValue(SETTING_TEST_ID);
	m_fromTestID = cmdLineParser.settingValue(SETTING_FROM_TEST_ID);
	m_traceStr = cmdLineParser.settingValue(SETTING_TRACE);
	m_reportFileName = cmdLineParser.settingValue(SETTING_REPORT);
	m_presetLM = cmdLineParser.settingValue(SETTING_PRESET_LM);
}

bool UalTester::cmdLineParamsIsValid()
{
	// main settings
	//

	if (m_cfgServiceIP1.isEmpty() == true && m_cfgServiceIP2.isEmpty() == true )
	{
		qDebug() << "Error: IP-addres of Configuration Service is empty";
		return false;
	}

	m_cfgSocketAddress1.setAddressPortStr(m_cfgServiceIP1, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	m_cfgSocketAddress2.setAddressPortStr(m_cfgServiceIP2, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	if (m_cfgSocketAddress1.isValidIPv4(m_cfgSocketAddress1.addressStr()) == false)
	{
		qDebug() << "Error: IP-addres of first Configuration Service is not valid";
		return false;
	}

	if (m_cfgSocketAddress2.isValidIPv4(m_cfgSocketAddress2.addressStr()) == false)
	{
		qDebug() << "Error: IP-addres of second Configuration Service is not valid";
		return false;
	}

	if (m_equipmentID.isEmpty() == true)
	{
		qDebug() << "Error: EquipmentID is epmpty";
		return false;
	}

	if (m_testFileName.isEmpty() == true)
	{
		qDebug() << "Error: Test file name is empty";
		return false;
	}

	if (QFileInfo::exists(m_testFileName) == false)
	{
		qDebug() << "Error: Tets file" << m_testFileName << "is not exist";
		return false;
	}

	// optional settings
	//

	if (m_errorIngnoreStr.isEmpty() == false)
	{
		bool paramOk = false;

		if (m_errorIngnoreStr.compare("yes", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_errorIngnore = true;
		}

		if (m_errorIngnoreStr.compare("no", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_errorIngnore = false;
		}

		if (paramOk == false)
		{
			qDebug() << "Error: Invalid value of the parameter \"errignore\", specify \"yes\" or \"no\"";
			return false;
		}
	}

	if (m_traceStr.isEmpty() == false)
	{
		bool paramOk = false;

		if (m_traceStr.compare("yes", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_enableTrace = true;
		}

		if (m_traceStr.compare("no", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_enableTrace = false;
		}

		if (paramOk == false)
		{
			qDebug() << "Error: Invalid value of the parameter \"trace\", specify \"yes\" or \"no\"";
			return false;
		}
	}

	if (m_reportFileName.isEmpty() == false)
	{
		QFile reportFile(m_reportFileName);
		if (reportFile.open(QIODevice::WriteOnly) == false)
		{
			qDebug() << "Error: Report file" << m_reportFileName << "has not been created";
			return false;
		}

		reportFile.write(QString("Report from: %1\r\n\r\n").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss")).toUtf8());

		reportFile.close();
	}

	return true;
}

bool UalTester::runCfgLoaderThread()
{

	m_softwareInfo.init(E::SoftwareType::TestClient, m_equipmentID, 1, 0);

	m_cfgLoaderThread = new CfgLoaderThread(m_softwareInfo,
											1,
											m_cfgSocketAddress1,
											m_cfgSocketAddress2,
											false,
											nullptr);

	if (m_cfgLoaderThread == nullptr)
	{
		return false;
	}

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &UalTester::slot_configurationReceived);

	connect(this, &UalTester::signal_configurationParsed, this, &UalTester::slot_parseTestFile);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	return true;
}

void UalTester::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;
	m_cfgLoaderThread = nullptr;
}

bool UalTester::start()
{
	if (cmdLineParamsIsValid() == false)
	{
		return false;
	}

	if (runCfgLoaderThread() == false)
	{
		qDebug() << "Error: CfgLoaderThread is not  running";
		return false;
	}

	return true;
}

void UalTester::stop()
{
	stopCfgLoaderThread();
	m_waitSocketsConnectionTimer.stop();
	stopSignalStateThread();
	stopTuningThread();
}

void UalTester::slot_configurationReceived(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	// load configuration
	//

	bool result = false;

	result = readConfiguration(configurationXmlData);
	if (result == false)
	{
		return;
	}

	// load signal list
	//

	for(Builder::BuildFileInfo bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			continue;
		}

		result = true;

		if (bfi.ID == CFG_FILE_ID_APP_SIGNAL_SET)
		{
			result &= readAppSignals(fileData);				// fill UalTesterSignals
		}
	}

	stopCfgLoaderThread();

	if (result == false)
	{
		return;
	}

	emit signal_configurationParsed();

	return;
}

bool UalTester::readConfiguration(const QByteArray& cfgFileData)
{
	XmlReadHelper xml(cfgFileData);

	if (m_cfgSettings.readFromXml(xml) == false)
	{
		qDebug() << "Error: Configuration settings are not valid";
		return false;
	}

	qDebug() << "Configuration was read successfully";

	return true;
}

bool UalTester::readAppSignals(const QByteArray& cfgFileData)
{
	::Proto::AppSignalSet signalSet;

	bool result = signalSet.ParseFromArray(cfgFileData.constData(), cfgFileData.size());

	if (result == false)
	{
		qDebug() << "Error: Configuration can not read signal list";
		return false;
	}

	int signalCount = signalSet.appsignal_size();

	for(int i = 0; i < signalCount; i++)
	{
		const ::Proto::AppSignal& appSignal = signalSet.appsignal(i);

		Signal signal;
		signal.serializeFrom(appSignal);

		if (signal.appSignalID().isEmpty() == true || signal.hash() == UNDEFINED_HASH)
		{
			assert(false);
			continue;
		}

		m_signalBase.appendSignal(signal);
	}

	if (m_signalBase.signalCount() == 0)
	{
		qDebug() << "Error: Configuration does not contain any signals for AppDataSrv";
		return false;
	}

	qDebug() << "Loaded signals:" << m_signalBase.signalCount();

	return true;
}

bool UalTester::parseTestFile()
{
	qDebug() << "Parse test file:" << m_testFileName;

	printToReportFile("Open test file: " + m_testFileName + "\r\n");

	m_testfile.setFileName(m_testFileName);
	if (m_testfile.open() == false)
	{
		printToReportFile("Can not open test file for read");
		return false;
	}

	m_testfile.setSignalBase(&m_signalBase);

	if (m_testfile.parse() == false)
	{
		m_testfile.close();
		return false;
	}

	m_testfile.close();

	qDebug() << "Test file was parsed successfully";

	return true;
}

void UalTester::slot_parseTestFile()
{
	// parse Test file
	//
	if (parseTestFile() == false)
	{
		// print error list to file
		//
		int errorCount = m_testfile.errorList().count();
		for(int i = 0; i < errorCount; i++)
		{
			printToReportFile(m_testfile.errorList().at(i));
		}

		printToReportFile(QString("Found error(s): %1").arg(errorCount));
		return;
	}

	// run sockets to get AppSignalState and to set TuningState
	//
	if (runSockets() == false)
	{
		return;
	}

	connect(this, &UalTester::signal_socketsReady, this, &UalTester::slot_socketsReady);

	runWaitSocketsConnectionTimer();
}

bool UalTester::runSignalStateThread()
{
	m_pSignalStateSocket = new SignalStateSocket(m_softwareInfo, m_cfgSettings.appDataService_clientRequestIP, &m_signalBase);
	if (m_pSignalStateSocket == nullptr)
	{
		return false;
	}

	m_pSignalStateSocketThread = new SimpleThread(m_pSignalStateSocket);
	if (m_pSignalStateSocketThread == nullptr)
	{
		return false;
	}

	m_pSignalStateSocketThread->start();

	return true;
}

void UalTester::stopSignalStateThread()
{
	if (m_pSignalStateSocketThread != nullptr)
	{
		m_pSignalStateSocketThread->quitAndWait(10000);
		delete m_pSignalStateSocketThread;
		m_pSignalStateSocketThread = nullptr;
	}
}

bool UalTester::signalStateSocketIsConnected()
{
	if (m_pSignalStateSocket == nullptr || m_pSignalStateSocketThread == nullptr)
	{
		return false;
	}

	if (m_pSignalStateSocket->isConnected() == false)
	{
		return false;
	}

	return true;
}

bool UalTester::runTuningThread()
{
	// init tuning socket thread - TuningSrv
	//
	m_pTuningSocket = new TuningSocket(m_softwareInfo, m_cfgSettings.tuningService_clientRequestIP, &m_tuningBase);
	if (m_pTuningSocket == nullptr)
	{
		return false;
	}

	m_pTuningSocketThread = new SimpleThread(m_pTuningSocket);
	if (m_pTuningSocketThread == nullptr)
	{
		return false;
	}

	m_pTuningSocketThread->start();

	return true;
}

void UalTester::stopTuningThread()
{
	if (m_pTuningSocketThread != nullptr)
	{
		m_pTuningSocketThread->quitAndWait(10000);
		delete m_pTuningSocketThread;
		m_pTuningSocketThread = nullptr;
	}
}

bool UalTester::tuningSocketIsConnected()
{
	if (m_pTuningSocket == nullptr || m_pTuningSocketThread == nullptr)
	{
		return false;
	}

	if (m_pTuningSocket->isConnected() == false)
	{
		return false;
	}

	return true;
}

bool UalTester::runSockets()
{
	qDebug() << "Waiting connect to AppDataSrv and TuningSrv";

	// run signal state socket thread - connect to AppDataSrv
	//
	qDebug() << "Run connect to AppDataSrv";
	if (runSignalStateThread() == false)
	{
		qDebug() << "Error: Signal state socket did not run";
		return false;
	}

	// run connect to TuningSrv
	//
	qDebug() << "Run connect to TuningSrv";
	if (runTuningThread() == false)
	{
		qDebug() << "Error: Tuning socket did not run";
		return false;
	}

	return true;
}

void UalTester::runWaitSocketsConnectionTimer()
{
	connect(&m_waitSocketsConnectionTimer, &QTimer::timeout, this, &UalTester::slot_waitSocketsConnection);
	m_waitSocketsConnectionTimer.setInterval(100);
	m_waitSocketsConnectionTimer.start();
}

void UalTester::slot_waitSocketsConnection()
{
	if (signalStateSocketIsConnected() == false || tuningSocketIsConnected() == false)
	{
		return;
	}

	m_waitSocketsConnectionTimer.stop();

	emit signal_socketsReady();
}

void UalTester::slot_socketsReady()
{
	 runTestFile();

	 stopSignalStateThread();
	 stopTuningThread();
}

void UalTester::runTestFile()
{
	qDebug() << "Test file is running, wait for the end ...\n";

	int startTestIndex = 0;				// for cmd line param -from
	bool enableContinueTest = true;		// for cmd line param -errignore

	int testCount = m_testfile.testList().count();
	if (testCount == 0)
	{
		qDebug() << "Error: Test file don't have any test";
		return;
	}

	// check cmd line param -from
	//
	if (m_fromTestID.isEmpty() == false)
	{
		startTestIndex = -1;

		for(int testIndex = 0; testIndex < testCount; testIndex++)
		{
			TestItem test = m_testfile.testList().at(testIndex);
			if (m_fromTestID != test.testID())
			{
				continue;
			}

			startTestIndex = testIndex;
			break;
		}
	}

	if (startTestIndex == -1)
	{
		qDebug() << "Error: TestID" << m_fromTestID << "not found in the test file";
		return;
	}

	for(int testIndex = startTestIndex; testIndex < testCount; testIndex++)
	{
		if (enableContinueTest == false)
		{
			break;
		}

		TestItem test = m_testfile.testList().at(testIndex);

		// check cmd line param -test
		//
		if (m_testID.isEmpty() == false)
		{
			if (m_testID != test.testID())
			{
				continue;
			}
		}

		// execute commands
		//
		int cmdCount = test.cmdCount();
		for(int cmdIndex = 0; cmdIndex < cmdCount; cmdIndex++)
		{
			if (enableContinueTest == false)
			{
				break;
			}

			TestCommand cmd = test.cmd(cmdIndex);

			switch (cmd.type())
			{
				case TF_CMD_TEST:
					{
						if (m_enableTrace == true)
						{
							qDebug() << test.index() + 1 << "Test" << test.name();
							printToReportFile(QString("%1. Test - %2").arg(test.index() + 1).arg(test.name()));
						}
					}
					break;

				case TF_CMD_ENDTEST:
					{
						if (test.errorCount() == 0)
						{
							if (m_enableTrace == true)
							{
								qDebug() << "Endtest - Ok" << "\n";
								printToReportFile("Endtest - Ok\r\n");
							}
							else
							{
								qDebug() << test.index() + 1 << "Test" << test.name() << "- Ok";
								printToReportFile(QString("%1. Test - %2 - Ok").arg(test.index() + 1).arg(test.name()));
							}
						}
						else
						{
							if (m_enableTrace == true)
							{
								qDebug() << "Endtest - error(s):" << test.errorCount() << "\n";
								printToReportFile(QString("Endtest - error(s): %1\r\n").arg(test.errorCount()));
							}
							else
							{
								qDebug() << test.index() + 1 << "Test" << test.name() << "- error(s):" << test.errorCount();
								printToReportFile(QString("%1. Test - %2 - error(s): %3").arg(test.index() + 1).arg(test.name()).arg(test.errorCount()));
							}

							if (m_errorIngnore == false)
							{
								enableContinueTest = false;
							}
						}
					}
					break;

				case TF_CMD_SET:
					{
						QVector<Hash> signalHashList;
						TuningWriteCmd tuningCmd;

						//
						//
						int paramCount = cmd.paramList().count();
						for(int i = 0; i < paramCount; i++)
						{
							TestCmdParam param = cmd.paramList().at(i);
							if (param.isEmtpy() == true)
							{
								continue;
							}

							TuningValueType tuningCmdType;

							switch (param.type())
							{
								case TestCmdParamType::Discrete:	tuningCmdType = TuningValueType::Discrete;		break;
								case TestCmdParamType::SignedInt32:	tuningCmdType = TuningValueType::SignedInt32;	break;
								case TestCmdParamType::SignedInt64:	tuningCmdType = TuningValueType::SignedInt64;	break;
								case TestCmdParamType::Float:		tuningCmdType = TuningValueType::Float;			break;
								case TestCmdParamType::Double:		tuningCmdType = TuningValueType::Double;		break;
								default:							continue;										break;
							}

							Hash signalHash = calcHash(param.name());
							if (signalHash == UNDEFINED_HASH)
							{
								continue;
							}

							signalHashList.append(signalHash);

							tuningCmd.setSignalHash(signalHash);
							tuningCmd.setType(tuningCmdType);
							tuningCmd.setValue(param.value());

							m_tuningBase.appendCmdFowWrite(tuningCmd);
						}

						//
						//
						m_signalBase.appendHashForRequestState(signalHashList);
						QThread::msleep(1000);

						//
						//
						for(int i = 0; i < paramCount; i++)
						{
							TestCmdParam param = cmd.paramList().at(i);
							if (param.isEmtpy() == true)
							{
								continue;
							}

							TestSignal signal = m_signalBase.signal(param.name());
							if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
							{
								qDebug() << "Signal" << param.name() << "not found in SignalBase";
								continue;
							}

							if (signal.state().isValid() == true)
							{
								if (signal.state().value() == param.value())
								{
									if (m_enableTrace == true)
									{
										qDebug() << "\tSet" << param.valueStr() << "- Ok";
										printToReportFile(QString("    Set %1 - Ok").arg(param.valueStr()));
									}
								}
								else
								{
									test.incErrorCount();

									if (m_enableTrace == true)
									{
										qDebug() << "\tSet" << param.valueStr() << "- Fail";
										printToReportFile(QString("    Set %1 - Fail").arg(param.valueStr()));
									}
								}
							}
							else
							{
								test.incErrorCount();

								if (m_enableTrace == true)
								{
									qDebug() << "\tSet signal" << param.name() << "- No valid";
									printToReportFile(QString("    Set signal %1 - No valid").arg(param.name()));
								}
							}
						}

						m_signalBase.clearHashForRequestState();
					}
					break;


				case TF_CMD_CHECK:
					{
						QVector<Hash> signalHashList;

						int paramCount = cmd.paramList().count();
						for(int i = 0; i < paramCount; i++)
						{
							TestCmdParam param = cmd.paramList().at(i);
							if (param.isEmtpy() == true)
							{
								continue;
							}

							signalHashList.append(calcHash(param.name()));
						}

						m_signalBase.appendHashForRequestState(signalHashList);
						QThread::msleep(100);

						for(int i = 0; i < paramCount; i++)
						{
							TestCmdParam param = cmd.paramList().at(i);
							if (param.isEmtpy() == true)
							{
								continue;
							}

							TestSignal signal = m_signalBase.signal(param.name());
							if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
							{
								qDebug() << "Signal" << param.name() << "not found in SignalBase";
								continue;
							}

							if (signal.state().isValid() == true)
							{
								if (signal.state().value() == param.value())
								{
									if (m_enableTrace == true)
									{
										qDebug() << "\tCheck" << param.valueStr() << "- Ok";
										printToReportFile(QString("    Check %1 - Ok").arg(param.valueStr()));
									}
								}
								else
								{
									test.incErrorCount();

									if (m_enableTrace == true)
									{
										qDebug() << "\tCheck" << param.valueStr() << "- Fail";
										printToReportFile(QString("    Check %1 - Fail").arg(param.valueStr()));
									}
								}
							}
							else
							{
								test.incErrorCount();

								if (m_enableTrace == true)
								{
									qDebug() << "\tCheck signal" << param.name() << "- No valid";
									printToReportFile(QString("    Check signal %1 - No valid").arg(param.name()));
								}
							}
						}

						m_signalBase.clearHashForRequestState();
					}
					break;

				case TF_CMD_DELAY:
					{
						if (cmd.paramList().count() != 1)
						{
							break;
						}

						TestCmdParam param = cmd.paramList().at(0);

						if (param.type() != TestCmdParamType::SignedInt32)
						{
							break;
						}

						int ms = param.value().toInt();

						if (m_enableTrace == true)
						{
							qDebug() << "\tDelay" << ms << "ms";
							printToReportFile(QString("    Delay %1 ms").arg(ms));
						}

						QThread::msleep(ms);
					}
					break;
			}
		}
	}

	qDebug() << "End of test file";
}


