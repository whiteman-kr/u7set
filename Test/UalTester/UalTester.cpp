#include "UalTester.h"

#include <QDebug>

#include "../../lib/SocketIO.h"
#include "../../lib/XmlHelper.h"
#include "../../lib/Signal.h"


#include <QtCore/QCoreApplication>
#include <csignal>

// -------------------------------------------------------------------------------------------------------------------
//
// UalTester class implementation
//
// -------------------------------------------------------------------------------------------------------------------

UalTester::UalTester(int& argc, char** argv) :
	m_waitSocketsConnectionTimer(this)
{
	setlocale(LC_ALL,"Russian");

	m_cmdLineParam.getParams(argc, argv);
}

UalTester::~UalTester()
{
}

bool UalTester::start()
{
	// parse params of cmd line
	//
	if (m_cmdLineParam.parse() == false)
	{
		return false;
	}

	std::cout << "------ Parsing of command line is successfully!" << std::endl;

	// run loader to receive signals from CfgSrv
	//
	runCfgLoaderThread();

	return true;
}

void UalTester::stop()
{
	stopCfgLoaderThread();
	m_waitSocketsConnectionTimer.stop();
	stopSignalStateThread();
	stopTuningThread();
}

void UalTester::runCfgLoaderThread()
{
	m_softwareInfo.init(E::SoftwareType::TestClient, m_cmdLineParam.equipmentID(), 1, 0);

	m_cfgLoaderThread = new CfgLoaderThread(m_softwareInfo,
											1,
											m_cmdLineParam.cfgSocketAddress1(),
											m_cmdLineParam.cfgSocketAddress2(),
											false,
											nullptr);

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &UalTester::slot_loadConfiguration, Qt::QueuedConnection);
	connect(this, &UalTester::configurationLoaded, this, &UalTester::slot_parseTestFile, Qt::QueuedConnection);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	return;
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


void UalTester::slot_loadConfiguration(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	bool result = false;

	// load configuration
	//
	result = readConfiguration(configurationXmlData);		// read configuration
	if (result == false)
	{
		QCoreApplication::exit(-1);
		return;
	}

	// load signals
	//
	for(Builder::BuildFileInfo bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			std::cout << errStr.toLocal8Bit().constData() << std::endl;
			continue;
		}

		result = true;

		if (bfi.ID == CFG_FILE_ID_APP_SIGNAL_SET)
		{
			result &= readAppSignals(fileData);				// read signals
		}
	}

	stopCfgLoaderThread();

	// configuration and signals laoded successfully
	//
	if (result == false)
	{
		QCoreApplication::exit(-1);
		return;
	}

	emit configurationLoaded();	// this is signal call slot to parse test file

	return;
}

bool UalTester::readConfiguration(const QByteArray& cfgFileData)
{
	XmlReadHelper xml(cfgFileData);

	if (m_cfgSettings.readFromXml(xml) == false)
	{
		std::cout << "Error: Configuration settings are not valid" << std::endl;
		return false;
	}

	std::cout << "------ Configuration was read successfully!" << std::endl;

	return true;
}

bool UalTester::readAppSignals(const QByteArray& cfgFileData)
{
	::Proto::AppSignalSet signalSet;

	bool result = signalSet.ParseFromArray(cfgFileData.constData(), cfgFileData.size());

	if (result == false)
	{
		std::cout << "Error: Configuration can not read signal list";
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
		std::cout << "Error: Configuration does not contain any signals for AppDataSrv" << std::endl;
		return false;
	}

	std::cout << "------ Loaded signals: " << m_signalBase.signalCount() << " successfully!" << std::endl;

	return true;
}

void UalTester::slot_parseTestFile()
{
	// parse Test file(s)
	//
	for(const QString& testFileName : m_cmdLineParam.testFileNameList())
	{
		if (parseTestFile(testFileName) == false)
		{
			m_cmdLineParam.printToReportFile(QStringList(QString("Test File: %1").arg(testFileName).toUtf8()));	// print file name to report file
			m_cmdLineParam.printToReportFile(m_testfile.errorList());												// print errors to report file
			QCoreApplication::exit(-1);
			return;
		}
	}

	std::cout << "------ Test file(s) was parsed successfully!" << std::endl;

	// run sockets to get AppSignalState and to set state signals and data sources
	//
	if (runSockets() == false)
	{
		QCoreApplication::exit(-1);
		return;
	}
}

bool UalTester::parseTestFile(const QString& testFileName)
{
	if (testFileName.isEmpty() == true)
	{
		std::cout << "Error: Test file name is empty" << std::endl;
		return false;
	}

	std::cout << "Parse test file:" << testFileName.toLocal8Bit().constData() << std::endl;

	if (m_testfile.parse(testFileName, &m_signalBase) == false)
	{
		m_testfile.close();
		return false;
	}

	m_testfile.close();

	std::cout << "Test file: " << testFileName.toLocal8Bit().constData() <<  " - was parsed successfully" << std::endl;

	return true;
}

bool UalTester::runSockets()
{
	std::cout << "Waiting connect to AppDataSrv and TuningSrv" << std::endl;

	// run signal state socket thread - connect to AppDataSrv
	//
	std::cout << "Run connect to AppDataSrv" << std::endl;
	if (runSignalStateThread() == false)
	{
		std::cout << "Error: Signal state socket did not run" << std::endl;
		return false;
	}

	// run connect to TuningSrv
	//
	std::cout << "Run connect to TuningSrv" << std::endl;
	if (runTuningThread() == false)
	{
		std::cout << "Error: Tuning socket did not run" << std::endl;
		return false;
	}

	// run waitSocketsConnectionTimer
	//
	connect(&m_waitSocketsConnectionTimer, &QTimer::timeout, this, &UalTester::slot_waitSocketsReady);
	m_waitSocketsConnectionTimer.setInterval(100);
	m_waitSocketsConnectionTimer.start();

	// wait connections
	//
	connect(this, &UalTester::signal_socketsReady, this, &UalTester::slot_runTestFile);

	return true;
}

bool UalTester::runSignalStateThread()
{
	HostAddressPort addressToConnect = m_cfgSettings.appDataService_clientRequestIP;

	if (addressToConnect.isEmpty() == true)
	{
		return false;
	}

	m_pSignalStateSocket = new SignalStateSocket(m_softwareInfo, addressToConnect, &m_signalBase);
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
		return;
	}

	m_pSignalStateSocketThread->quitAndWait(10000);
	delete m_pSignalStateSocketThread;
	m_pSignalStateSocketThread = nullptr;
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
	HostAddressPort addressToConnect;

	// select ip address
	//
	if (m_cmdLineParam.packetSourceAddress().isEmpty() == true)
	{
		addressToConnect = m_cfgSettings.tuningService_clientRequestIP;
	}
	else
	{
		addressToConnect = m_cmdLineParam.packetSourceAddress();
	}

	if (addressToConnect.isEmpty() == true)
	{
		return false;
	}

	m_pTuningSocket = new TuningSocket(m_softwareInfo, addressToConnect, &m_tuningSourceBase);
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

void UalTester::slot_waitSocketsReady()
{
	if (signalStateSocketIsConnected() == false || tuningSocketIsConnected() == false)
	{
		return;
	}

	m_waitSocketsConnectionTimer.stop();

	std::cout << "------ Connect to AppDataSrv and TuningSrv is established successfully!" << std::endl;

	emit signal_socketsReady();  // this is signal call slot to parse test file
}

void UalTester::slot_runTestFile()
{
	 int errorCount = runTestFile();
	 QCoreApplication::exit(errorCount);

	 stopSignalStateThread();
	 stopTuningThread();
}

int UalTester::runTestFile()
{
	if (m_pTuningSocket == nullptr)
	{
		std::cout << "Error: TuningSocket is not valid" << std::endl;
		return -1;
	}

	int startTestIndex = m_cmdLineParam.getStartTestIndex(m_testfile.testList());	// for cmd line param -from
	if (startTestIndex == -1)
	{
		std::cout << "Error: TestID " << m_cmdLineParam.fromTestID().toLocal8Bit().constData() << " not found in the test file" << std::endl;
		return -1;
	}

	std::cout << std::endl << "Test file is running, wait for the end ..." << std::endl;

	int totalErrorCount = 0;

	int testCount = m_testfile.testList().count();
	for(int testIndex = startTestIndex; testIndex < testCount; testIndex++)
	{
		if (m_cmdLineParam.enableContinueTest() == false)							// for cmd line param -errignore
		{
			break;
		}

		TestItem test = m_testfile.testList().at(testIndex);

		if (m_cmdLineParam.enableExecuteTest(test.testID()) == false)				// check cmd line param -test
		{
			continue;
		}

		if (m_cmdLineParam.enableExecuteTestForLM(test) == false)					// check cmd line param -lm
		{
			continue;
		}

		// execute all commands of test
		//
		int cmdCount = test.cmdCount();
		for(int cmdIndex = 0; cmdIndex < cmdCount; cmdIndex++)
		{
			if (m_cmdLineParam.enableContinueTest() == false)						// for cmd line param -errignore
			{
				break;
			}

			TestCmd cmd = test.cmd(cmdIndex);

			if (cmd.comment().isEmpty() == false)
			{
				test.appendResult(TF_SPACE +  cmd.comment(), m_cmdLineParam.enableTrace());
			}

			switch (cmd.type())
			{
				case TF_CMD_TEST:
					{
						std::cout << std::endl;

						QString str = "Test " + test.name();

						if (test.compatibleList().isEmpty() == false)
						{
							str += " (" + test.compatibleList().join(",") + ")";
						}

						test.appendResult(str, m_cmdLineParam.enableTrace());
					}
					break;

				case TF_CMD_SET:
					{
						QVector<Hash> signalHashList;

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

							ChangeSignalState signalCmd;

							signalCmd.setSignalHash(signalHash);
							signalCmd.setType(tuningCmdType);
							signalCmd.setValue(param.value());

							m_pTuningSocket->writeCmd(signalCmd);
							QThread::msleep(TUNING_SOCKET_WAIT_RAPLY_TIMEOUT);
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
							if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == UNDEFINED_HASH)
							{
								QString str = TF_SPACE + "Signal " + param.name() + " not found in SignalBase";
								test.appendResult(str, true);
								continue;
							}

							if (signal.state().isValid() == true)
							{
								if (param.compare(signal.state()) == true)
								{
									QString str = TF_SPACE + "Set " + param.valueStr(true, signal.param().decimalPlaces());
									test.appendResult(str, m_cmdLineParam.enableTrace());
								}
								else
								{
									test.incErrorCount();

									QString str = TF_SPACE + "Set " + param.valueStr(true, signal.param().decimalPlaces()) + " - Fail";;

									if (param.isFlag() == false)
									{
										TestCmdParam realState = param;
										realState.setValue(signal.state().value());

										str += TF_SPACE + "[ received: " + realState.valueStr(false, signal.param().decimalPlaces()) + " ]";
									}

									test.appendResult(str, m_cmdLineParam.enableTrace());
								}
							}
							else
							{
								test.incErrorCount();

								QString str = TF_SPACE + "Set signal " + param.name() + " - No valid";
								test.appendResult(str, m_cmdLineParam.enableTrace());
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
							if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == UNDEFINED_HASH)
							{
								QString str = TF_SPACE + "Signal " + param.name() + " not found in SignalBase";
								test.appendResult(str, true);
								continue;
							}

							if (signal.state().isValid() == true)
							{
								if (param.compare(signal.state()) == true)
								{
									QString str = TF_SPACE + "Check " + param.valueStr(true, signal.param().decimalPlaces()) + " - Ok";
									test.appendResult(str, m_cmdLineParam.enableTrace());
								}
								else
								{
									test.incErrorCount();

									QString str = TF_SPACE + "Check " + param.valueStr(true, signal.param().decimalPlaces()) + " - Fail";

									if (param.isFlag() == false)
									{
										TestCmdParam realState = param;
										realState.setValue(signal.state().value());

										str += TF_SPACE + "[ received: " + realState.valueStr(false, signal.param().decimalPlaces()) + " ]";
									}

									test.appendResult(str, m_cmdLineParam.enableTrace());
								}
							}
							else
							{
								test.incErrorCount();

								QString str = TF_SPACE + "Check signal " + param.name() + " - No valid";
								test.appendResult(str, m_cmdLineParam.enableTrace());
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

						quint32 ms = param.value().toUInt();

						QString str = TF_SPACE + "Delay " + QString("%1").arg(ms) + " ms";
						test.appendResult(str, m_cmdLineParam.enableTrace());

						QThread::msleep(ms);
					}
					break;

				case TF_CMD_RUN_SOURCE:
					{
						if (m_cmdLineParam.packetSourceAddress().isEmpty() == true)	// only PacketSource if tuning then break
						{
							break;
						}

						if (cmd.paramList().count() != 1)
						{
							break;
						}

						TestCmdParam param = cmd.paramList().at(0);

						if (param.type() != TestCmdParamType::String)
						{
							break;
						}

						QString sourceID = param.value().toString();
						if (sourceID.isEmpty() == true)
						{
							break;
						}

						QString str = TF_SPACE + "Run source " + sourceID;
						test.appendResult(str, m_cmdLineParam.enableTrace());

						ChangeSourceState sourceCmd;

						sourceCmd.setSourceID(sourceID);
						sourceCmd.setState(true);

						m_pTuningSocket->writeCmd(sourceCmd);
						QThread::msleep(TUNING_SOCKET_WAIT_RAPLY_TIMEOUT);
					}
					break;

				case TF_CMD_STOP_SOURCE:
					{
						if (m_cmdLineParam.packetSourceAddress().isEmpty() == true)	// only PacketSource if tuning then break
						{
							break;
						}

						if (cmd.paramList().count() != 1)
						{
							break;
						}

						TestCmdParam param = cmd.paramList().at(0);

						if (param.type() != TestCmdParamType::String)
						{
							break;
						}

						QString sourceID = param.value().toString();
						if (sourceID.isEmpty() == true)
						{
							break;
						}

						QString str = TF_SPACE + "Stop source " + sourceID;
						test.appendResult(str, m_cmdLineParam.enableTrace());

						ChangeSourceState sourceCmd;

						sourceCmd.setSourceID(sourceID);
						sourceCmd.setState(false);

						m_pTuningSocket->writeCmd(sourceCmd);
						QThread::msleep(TUNING_SOCKET_WAIT_RAPLY_TIMEOUT);
					}
					break;

				case TF_CMD_EXIT_PS:
					{
						if (m_cmdLineParam.packetSourceAddress().isEmpty() == true)	// only PacketSource if tuning then break
						{
							break;
						}

						if (cmd.paramList().count() != 0)
						{
							break;
						}

						QString str = TF_SPACE + "Exit PacketSourceConsole";
						test.appendResult(str, m_cmdLineParam.enableTrace());

						m_pTuningSocket->writeCmd(true);
						QThread::msleep(TUNING_SOCKET_WAIT_RAPLY_TIMEOUT);
					}
					break;


				case TF_CMD_ENDTEST:
					{
						if (test.errorCount() == 0)
						{
							if (m_cmdLineParam.enableTrace() == true)
							{
								QString str = "Endtest " + test.testID()  + " - Ok";
								test.appendResult(str, true);
							}
							else
							{
								test.resultList().clear();

								QString str = "Test " + test.name()  + " - Ok";
								test.appendResult(str, true);
							}
						}
						else
						{
							QString str = QString("Endtest %1 - error(s): %2").arg(test.testID()).arg(test.errorCount());
							test.resultList().append(str);

							if (m_cmdLineParam.enableTrace() == true)
							{
								std::cout << str.toLocal8Bit().constData() << std::endl;
							}
							else
							{
								int resCount = test.resultList().count();
								for (int i = 0; i < resCount; i++)
								{
									std::cout << test.resultList().at(i).toLocal8Bit().constData() << std::endl;
								}
							}
						}

						if (m_cmdLineParam.errorIngnore() == false)				// check cmd line param -errignore
						{
							m_cmdLineParam.setEnableContinueTest(false);
						}

						test.resultList().append(QString());
						m_cmdLineParam.printToReportFile(test.resultList());	// print results to report file
					}
					break;
			}
		}

		totalErrorCount += test.errorCount();
	}

	std::cout << std::endl << "End of test file" << std::endl;

	return totalErrorCount;
}


