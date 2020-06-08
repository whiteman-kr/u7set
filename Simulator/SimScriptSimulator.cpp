#include "SimScriptSimulator.h"
#include "Simulator.h"
#include <QJSValueIterator>

namespace Sim
{

	RamAddress::RamAddress(const Address16& addr16) :
		m_offset(addr16.isValid() ? addr16.offset() : BadAddress),
		m_bit(addr16.isValid() ? addr16.bit() : BadAddress)
	{
	}

	RamAddress::RamAddress(quint32 offset, quint32 bit) :
		m_offset(offset),
		m_bit(bit)
	{
	}

	bool RamAddress::isValid() const
	{
		return m_offset != BadAddress && m_bit != BadAddress;
	}

	quint32 RamAddress::offset() const
	{
		return m_offset;
	}

	void RamAddress::setOffset(quint32 value)
	{
		m_offset = value;
	}

	quint32 RamAddress::bit() const
	{
		return m_bit;
	}

	void RamAddress::setBit(quint32 value)
	{
		m_bit = value;
	}

	QString RamAddress::toString() const
	{
		return QString("Offset: %1 (%2), bit: %3, Access: %4");
	}

	ScriptWorkerThread::ScriptWorkerThread(ScriptSimulator* scriptSimulator) :
		QThread(),
		m_scriptSimulator{scriptSimulator}
	{
		assert(m_scriptSimulator);

		setObjectName("Sim::ScriptWorkerThread");

		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		return;
	}

	void ScriptWorkerThread::run()
	{
        m_result = true;

		if (m_scriptSimulator == nullptr)
		{
			writeError(tr("Internal error: m_scriptSimulator == nullptr"));
			m_result = false;
			return;
		}

		writeMessage(tr("********** Start testing of %1 **********").arg(m_testName));

        m_jsThis = m_jsEngine.newQObject(m_scriptSimulator);
        QQmlEngine::setObjectOwnership(m_scriptSimulator, QQmlEngine::CppOwnership);

		// Evaluate script
		//
		QJSValue scriptValue = m_jsEngine.evaluate(m_script);

		if (scriptValue.isError() == true)
		{
			writeError(tr("Script evaluate error at line %1\n"
						 "\tClass: %2\n"
						 "\tStack: %3\n"
						 "\tMessage: %4")
					  .arg(scriptValue.property("lineNumber").toInt())
					  .arg(metaObject()->className())
					  .arg(scriptValue.property("stack").toString())
					  .arg(scriptValue.toString()));

			m_result = false;
			return;
		}

        // initTestCase() - will be called before the first test function is executed.
        // cleanupTestCase() - will be called after the last test function was executed.
        // init() - will be called before each test function is executed.
        // cleanup() - will be called after every test function.
        //

		QElapsedTimer timer;
		timer.start();

        // initTestCase() - will be called before the first test function is executed.
        //
        runScriptFunction("initTestCase");

        // Call all functions which starts from 'test', like 'testAfbNot()'
        //
        QStringList testList;

        QJSValueIterator it(m_jsEngine.globalObject());
        while (it.hasNext() == true)
        {
            it.next();

			if (it.name().startsWith("test"))
            {
				testList.push_back(it.name());
            }
        }

		std::sort(testList.begin(), testList.end());

		int failed = 0;
		for (const QString& testFunc : testList)
		{
			// init() - called before each test function is executed.
			//
			runScriptFunction("init");

			if (bool testOk = runScriptFunction(testFunc);
				testOk == true)
			{
				writeMessage(testFunc + ": ok");
			}
			else
			{
				failed ++;
				writeError(testFunc + ": FAILED");
			}

			// cleanup() - called after every test function.
			//
			runScriptFunction("cleanup");
		}

		// cleanup() - will be called after every test function.
		//
		runScriptFunction("cleanupTestCase");

		qint64 elapsedMsTotal = timer.elapsed();

		if (failed != 0)
		{
			writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
		}
		else
		{
			writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
		}

		writeMessage(tr("********** Finished testing of %1 **********").arg(m_testName));

		m_result = (failed == 0);
		return;
	}

    bool ScriptWorkerThread::runScriptFunction(const QString& functionName)
    {
        QJSValue funcProp = m_jsEngine.globalObject().property(functionName);
        if (funcProp.isUndefined() == true)
        {
            return false;
        }

        if (funcProp.isCallable() == false)
        {
            writeError(tr("%1 is callable function").arg(functionName));
            return false;
        }

        Q_ASSERT(m_jsThis.isUndefined() == false && m_jsThis.isObject() == true);

        // Run script function
        //
        QJSValue result = funcProp.call(QJSValueList{} << m_jsThis);

        // Log errors and exit
        //
		if (result.isError() == true)
        {
			if (result.errorType() == QJSValue::ErrorType::GenericError)
			{
				// Assume that JS code must report about the error
				//
				writeError(tr("Error, stack trace: %1\n\tMessage: %2")
						   .arg(result.property("stack").toString())
						   .arg(result.toString()));
			}
			else
			{
				writeError(tr("Error at line %1\n"
							 "\tStack: %2\n"
							 "\tMessage: %3")
						  .arg(result.property("lineNumber").toInt())
						  .arg(result.property("stack").toString())
						  .arg(result.toString()));
			}

            return false;
        }

        return true;
    }

	void ScriptWorkerThread::start(QThread::Priority priority/* = InheritPriority*/)
	{
		m_jsEngine.moveToThread(this);
		m_result = true;

		QThread::start(priority);
		return;
	}

	bool ScriptWorkerThread::interruptScript()
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		m_jsEngine.setInterrupted(true);
		return true;
#else
		return false;
#endif
	}

	bool ScriptWorkerThread::result() const
	{
		return m_result;
	}

	void ScriptWorkerThread::setScript(QString value)
	{
		m_script = value;
	}

	void ScriptWorkerThread::setTestName(QString value)
	{
		m_testName = value;
	}

	ScriptSimulator::ScriptSimulator(Simulator* simulator, QObject* parent) :
		QObject(parent),
		Output(),
		m_simulator(simulator)
	{
		assert(m_simulator);
	}

	ScriptSimulator::~ScriptSimulator()
	{
	}

	bool ScriptSimulator::runScript(QString script, QString testName)
	{
		if (m_simulator == nullptr)
		{
			assert(m_simulator);
			return false;
		}

		if (m_simulator->isLoaded() == false)
		{
			writeError(tr("Script start error: project is not loaded."));
			return false;
		}

		if (script.isEmpty())
		{
			// It is not an error, just warn user about empty script and return true
			//
			writeWaning(tr("Script is empty."));
			return true;
		}

		writeDebug(tr("Start script"));

		m_workerThread.setScript(script);
		m_workerThread.setTestName(testName);

		m_workerThread.start();

		return true;
	}

	bool ScriptSimulator::stopScript()
	{
		if (isRunning() == false)
		{
			return true;
		}

		return m_workerThread.interruptScript();
	}

	bool ScriptSimulator::isRunning() const
	{
		return m_workerThread.isRunning();
	}

	bool ScriptSimulator::wait(unsigned long msecs /*= ULONG_MAX*/)
	{
		bool ok = m_workerThread.wait(msecs);
        return ok && m_workerThread.result();
	}

	bool ScriptSimulator::result() const
	{
		if (isRunning() == true)
		{
			writeError("Cannot get script result, script is still running.");
			return false;
		}

		return m_workerThread.result();
	}

	void ScriptSimulator::debugOutput(QString str)
	{
		qDebug() << str;
	}

	bool ScriptSimulator::startForMs(int msecs)
	{
		using namespace std::chrono;

        QJSEngine* jsEngine = qjsEngine(this);
        if (jsEngine == nullptr)
        {
            assert(jsEngine);
        }

		if (m_simulator->isRunning() == true)
		{
			writeWaning(tr("Simulation already running"));
			return false;
		}

		milliseconds durationMs{msecs};
		microseconds durationUs = duration_cast<microseconds>(durationMs);

        bool ok = m_simulator->control().startSimulation(durationUs);
        if (ok == false)
        {
            jsEngine->throwError(tr("Start simulation error."));
            return false;
        }

        // This is blocking call, wait for finishing simulation
        //
        while (m_simulator->control().isRunning() == true)
        {
			QThread::msleep(0);
        }

        return ok;
	}

	bool ScriptSimulator::reset()
	{
		if (m_simulator->isRunning() == true)
		{
			m_simulator->control().stop();
		}

		m_simulator->control().reset();
		m_simulator->control().setRunList({});

		return true;
	}

	double ScriptSimulator::signalValue(QString appSignalId)
	{
		bool ok = false;
		AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, &ok, true);

		if (ok == false)
		{
			throwScriptException(tr("signalValue(%1), signal not found.").arg(appSignalId));
			return -1;
		}

		return state.value();
	}

	bool ScriptSimulator::overrideSignalValue(QString appSignalId, double value)
	{
		if (m_simulator->overrideSignals().isSignalInOverrideList(appSignalId) == false)
		{
			int count = m_simulator->overrideSignals().addSignals(QStringList{} << appSignalId);
			if (count != 1)
			{
				return false;
			}
		}

		m_simulator->overrideSignals().setValue(appSignalId, OverrideSignalMethod::Value, value);
		return true;
	}

	void ScriptSimulator::overridesReset()
	{
		m_simulator->overrideSignals().clear();
	}

	bool ScriptSimulator::isLmExists(QString lmEquipmentId) const
	{
		std::vector<std::shared_ptr<LogicModule>> lms = m_simulator->logicModules();

		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		return lm != nullptr;
	}

	bool ScriptSimulator::isSignalExists(QString appSignalId) const
	{
		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);

		return signal.has_value();
	}

	RamAddress ScriptSimulator::signalUalAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->ualAddr();
		return result;
	}

	RamAddress ScriptSimulator::signalIoAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->ioBufAddr();
		return result;
	}

	RamAddress ScriptSimulator::signalTuningAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->tuningAddr();
		return result;
	}

	RamAddress ScriptSimulator::signalTuningAbsAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->tuningAbsAddr();
		return result;
	}

	RamAddress ScriptSimulator::signalRegBufAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->regBufAddr();
		return result;
	}

	RamAddress ScriptSimulator::signalRegValueAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->regValueAddr();
		return result;
	}

	RamAddress ScriptSimulator::signalRegValidityAddr(QString appSignalId) const
	{
		RamAddress result;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->regValidityAddr();
		return result;
	}

	quint32 ScriptSimulator::signalSizeW(QString appSignalId) const
	{
		quint32 result = 0;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);

		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->sizeW();
		return result;
	}

	quint32 ScriptSimulator::signalSizeBit(QString appSignalId) const
	{
		quint32 result = 0;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);

		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->sizeBit();
		return result;
	}

	bool ScriptSimulator::signalIsAcquired(QString appSignalId) const
	{
		quint32 result = false;

		std::optional<Signal> signal = m_simulator->appSignalManager().signalParamExt(appSignalId);

		if (signal.has_value() == false)
		{
			return result;
		}

		result = signal->isAcquired();

		return result;
	}

	bool ScriptSimulator::addrInIoModuleBuf(QString lmEquipmentId, quint32 modulePlace, RamAddress addr) const
	{
		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return false;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		if (modulePlace < 1 || modulePlace > lmDescription.memory().m_moduleCount)
		{
			return false;
		}

		return	addr.offset() >= (lmDescription.memory().m_moduleDataOffset + (modulePlace - 1) * lmDescription.memory().m_moduleDataSize) &&
				addr.offset() < (lmDescription.memory().m_moduleDataOffset + modulePlace * lmDescription.memory().m_moduleDataSize);
	}

	bool ScriptSimulator::addrInRegBuf(QString lmEquipmentId, RamAddress addr) const
	{
		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return false;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		return	addr.offset() >= lmDescription.memory().m_appLogicWordDataOffset &&
				addr.offset() < lmDescription.memory().m_appLogicWordDataOffset + lmDescription.memory().m_appLogicWordDataSize;
	}

	quint32 ScriptSimulator::regBufStartAddr(QString lmEquipmentId) const
	{
		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(lmEquipmentId);

		if (lm == nullptr)
		{
			return RamAddress::BadAddress;
		}

		const LmDescription& lmDescription = lm->lmDescription();

		return	lmDescription.memory().m_appLogicWordDataOffset;
	}


	quint16 ScriptSimulator::readRamBit(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception, but it will not throw c++ exception
		if (lm == nullptr)
		{
			return {};
		}

		quint16 result = {};

		bool ok = lm->ram().readBit(address.offset(), address.bit(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			throwScriptException(tr("readRamBit error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	quint16 ScriptSimulator::readRamWord(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception
		if (lm == nullptr)
		{
			return {};
		}

		quint16 result = {};

		bool ok = lm->ram().readWord(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			throwScriptException(tr("readRamWord error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	quint32 ScriptSimulator::readRamDword(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception
		if (lm == nullptr)
		{
			return {};
		}

		quint32 result = {};

		bool ok = lm->ram().readDword(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			throwScriptException(tr("readRamDword error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	qint32 ScriptSimulator::readRamSignedInt(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception
		if (lm == nullptr)
		{
			return {};
		}

		qint32 result = {};

		bool ok = lm->ram().readSignedInt(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			throwScriptException(tr("readRamSignedInt error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	float ScriptSimulator::readRamFloat(QString lmEquipmentId, RamAddress address, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception
		if (lm == nullptr)
		{
			return {};
		}

		float result = {};

		bool ok = lm->ram().readFloat(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			throwScriptException(tr("readRamFloat error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}


	void ScriptSimulator::writeRamBit(QString lmEquipmentId, RamAddress address, quint16 value, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception, but it will not throw c++ exception
		if (lm == nullptr)
		{
			return;
		}

		bool ok = lm->mutableRam().writeBit(address.offset(), address.bit(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			throwScriptException(tr("writeRamBit error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptSimulator::writeRamWord(QString lmEquipmentId, RamAddress address, quint16 value, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception, but it will not throw c++ exception
		if (lm == nullptr)
		{
			return;
		}

		bool ok = lm->mutableRam().writeWord(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			throwScriptException(tr("writeRamWord error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptSimulator::writeRamDword(QString lmEquipmentId, RamAddress address, quint32 value, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception, but it will not throw c++ exception
		if (lm == nullptr)
		{
			return;
		}

		bool ok = lm->mutableRam().writeDword(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			throwScriptException(tr("writeRamDword error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptSimulator::writeRamSignedInt(QString lmEquipmentId, RamAddress address, qint32 value, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception, but it will not throw c++ exception
		if (lm == nullptr)
		{
			return;
		}

		bool ok = lm->mutableRam().writeSignedInt(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			throwScriptException(tr("writeRamSignedInt error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptSimulator::writeRamFloat(QString lmEquipmentId, RamAddress address, float value, E::LogicModuleRamAccess access)
	{
		auto lm = logicModule(lmEquipmentId);	// Throws script exception, but it will not throw c++ exception
		if (lm == nullptr)
		{
			return;
		}

		bool ok = lm->mutableRam().writeFloat(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			throwScriptException(tr("writeRamFloat error, address %1").arg(address.toString()));
		}

		return;
	}


	std::shared_ptr<LogicModule> ScriptSimulator::logicModule(QString lmEquipmentId)
	{
		auto lm = m_simulator->logicModule(lmEquipmentId);
		if (lm == nullptr)
		{
			throwScriptException(tr("readRamBit LogicModule %1 not found").arg(lmEquipmentId));
		}

		return lm;
	}

	void ScriptSimulator::throwScriptException(QString text)
	{
		QJSEngine* jsEngine = qjsEngine(this);

		Q_ASSERT(jsEngine);
		if (jsEngine != nullptr)
		{
			jsEngine->throwError(QJSValue::ErrorType::GenericError, text);
		}
	}

	QString ScriptSimulator::buildPath() const
	{
		return m_simulator->buildPath();
	}

	qint64 ScriptSimulator::executionTimeOut() const
	{
		return m_executionTimeOut;
	}

	void ScriptSimulator::setExecutionTimeOut(qint64 value)
	{
		m_executionTimeOut = value;
	}



}
