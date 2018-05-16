#include <cassert>
#include "SimCommandProcessor.h"
#include "SimCommandProcessor_LM1_SF00.h"

namespace Sim
{
	const std::map<QString, std::function<CommandProcessor*(DeviceEmulator*)>> CommandProcessor::m_lmToFactory
		{
			{"LM1_SF00", [](DeviceEmulator* device){	return new CommandProcessor_LM1_SF00(device);	}},
			//{"LM_SR02", [](auto device){	return new CommandProcessor_LM_SR02;	}},
			//{"LM_SR01", [](auto device){	return new CommandProcessor_LM_SR01;	}},
		};

	CommandProcessor::CommandProcessor(DeviceEmulator* device) :
		QObject(),
		m_device(device)
	{
		assert(device);
	}

	CommandProcessor::~CommandProcessor()
	{
	}

	CommandProcessor* CommandProcessor::createInstance(DeviceEmulator* device)
	{
		assert(device);

		QString logicModuleName = device->lmDescription().name();
		QString equipmentId = device->logicModuleInfo().equipmentId;

		if (auto it = m_lmToFactory.find(logicModuleName);
			it == m_lmToFactory.end())
		{
			return nullptr;
		}
		else
		{
			CommandProcessor* result = it->second(device);
			assert(result);

			result->setOutputScope(equipmentId);

			return result;
		}
	}

	QString CommandProcessor::logicModuleName() const
	{
		assert(false);	// Implement it in derived class
		return "";
	}

	bool CommandProcessor::parseFunc(QString parseFunc, DeviceCommand* command)
	{
		if (command == nullptr)
		{
			assert(command);
			return false;
		}

		bool retVal = false;
		bool ok = QMetaObject::invokeMethod(this,
											parseFunc.toStdString().data(),
											Qt::DirectConnection,
											Q_RETURN_ARG(bool, retVal),
											Q_ARG(DeviceCommand*, command));

		return ok;
	}

	ScriptDeviceEmulator& CommandProcessor::device()
	{
		return m_device;
	}

	const ScriptDeviceEmulator& CommandProcessor::device() const
	{
		return m_device;
	}

	AfbComponent CommandProcessor::checkAfb(int opCode, int instanceNo, int pinOpCode /*= -1*/) const
	{
		AfbComponent afb = device().afbComponent(opCode);
		if (afb.isNull() == true)
		{
			SimException::raise(QString("Cannot find AfbComponent with OpCode %1").arg(opCode));
		}

		if (instanceNo < 0 || instanceNo >= afb.maxInstCount())
		{
			SimException::raise(QString("AfbComponent.Instance (%1) is out of limits (0-%2]")
									.arg(instanceNo)
									.arg(afb.maxInstCount()));
		}

		if (pinOpCode != -1 && afb.pinExists(pinOpCode) == false)
		{
			 SimException::raise(QString("AfbComponent does not have pin %1").arg(pinOpCode));
		}

		return afb;
	}

	QString CommandProcessor::strCommand(QString command) const
	{
		return command.leftJustified(10);
	}

	QString CommandProcessor::strAfbInstPin(const DeviceCommand* command) const
	{
		AfbComponent afb = device().afbComponent(command->m_afbOpCode);
		if (afb.isNull() == true)
		{
			SimException::raise(QString("AFB with OpCode %1 does not exist.").arg(command->m_afbOpCode), "CommandProcessor::strAfbInstPin");
		}

		return QString("%1.%2[%3]")
					.arg(afb.caption())
					.arg(command->m_afbInstance)
					.arg(afb.pinCaption(command->m_afbPinOpCode));
	}

	QString CommandProcessor::strAddr(quint16 address) const
	{
		return QString().arg(static_cast<ushort>(address), 4, 16, QChar('0'));
	}

	QString CommandProcessor::strWordConst(quint16 data) const
	{
		return QString("#%1h").arg(static_cast<uint>(data), 4, 16, QChar('0'));
	}

	QString CommandProcessor::strDwordConst(quint32 data) const
	{
		return QString("#%1h").arg(static_cast<uint>(data), 8, 16, QChar('0'));
	}

}
