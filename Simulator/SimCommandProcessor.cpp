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

	bool CommandProcessor::runCommand([[maybe_unused]]const DeviceCommand& command)
	{
		// Must be implemented in derived class
		//
		assert(false);
		return false;
	}

	AfbComponent CommandProcessor::checkAfb(int opCode, int instanceNo, int pinOpCode /*= -1*/) const
	{
		AfbComponent afb = m_device.afbComponent(opCode);
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

	void CommandProcessor::checkParamRange(int paramValue, int minValue, int maxValue, QString param) const
	{
		if (paramValue < minValue || paramValue > maxValue)
		{
			QString message = QString("Param %1 is out of range, value = %2, range [%3, %4]")
								.arg(param)
								.arg(paramValue)
								.arg(minValue)
								.arg(maxValue);

			SimException::raise(message);
		}

		return;
	}

	void CommandProcessor::checkParamExists(const AfbComponentInstance* afbInstance, int paramOpIndex, QString paramName) const
	{
		if (afbInstance == nullptr)
		{
			assert(afbInstance);
			return;
		}

		if (afbInstance->paramExists(paramOpIndex) == false)
		{
			if (paramName.isEmpty() == true)
			{
				SimException::raise(QString("Param %1 is not found.")
										.arg(paramOpIndex));
			}
			else
			{
				SimException::raise(QString("Param %1 is not found.")
										.arg(paramName));
			}
		}

		return;
	}

	QString CommandProcessor::strCommand(QString command) const
	{
		return command.leftJustified(10);
	}

	QString CommandProcessor::strAfbInst(const DeviceCommand* command) const
	{
		AfbComponent afb = m_device.afbComponent(command->m_afbOpCode);
		if (afb.isNull() == true)
		{
			SimException::raise(QString("AFB with OpCode %1 does not exist.").arg(command->m_afbOpCode), "CommandProcessor::strAfbInst");
		}

		return QString("%1.%2")
					.arg(afb.caption())
					.arg(command->m_afbInstance);
	}

	QString CommandProcessor::strAfbInstPin(const DeviceCommand* command) const
	{
		AfbComponent afb = m_device.afbComponent(command->m_afbOpCode);
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
		QString hexAddr = QString("%1").arg(static_cast<ushort>(address), 4, 16, QChar('0'));
		if (hexAddr.at(0).isDigit() == false)
		{
			hexAddr.prepend('0');
		}

		return hexAddr;
	}

	QString CommandProcessor::strBitAddr(quint16 address, quint16 bitNo) const
	{
		QString hexAddr = QString("%1").arg(static_cast<ushort>(address), 4, 16, QChar('0'));
		if (hexAddr.at(0).isDigit() == false)
		{
			hexAddr.prepend('0');
		}

		return QString("%1[%2]")
					.arg(hexAddr)
					.arg(bitNo);
	}

	QString CommandProcessor::strBitConst(quint16 data) const
	{
		return QString("#%1").arg(data);
	}


	QString CommandProcessor::strWordConst(quint16 data) const
	{
		QString dataStr = QString("%1").arg(static_cast<ushort>(data), 4, 16, QChar('0'));
		if (dataStr.at(0).isDigit() == false)
		{
			dataStr.prepend('0');
		}

		return "#" + dataStr;
	}

	QString CommandProcessor::strDwordConst(quint32 data) const
	{
		QString dataStr = QString("%1").arg(static_cast<quint32>(data), 8, 16, QChar('0'));
		if (dataStr.at(0).isDigit() == false)
		{
			dataStr.prepend('0');
		}

		return "#" + dataStr;
	}

}
