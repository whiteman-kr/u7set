#ifndef SIMCOMMANDPROCESSOR_H
#define SIMCOMMANDPROCESSOR_H
#include <map>
#include <functional>
#include "QtCore"
#include "SimOutput.h"
#include "SimDeviceEmulator.h"
#include "SimException.h"

namespace Sim
{
	class DeviceCommand;

	class CommandProcessor : public QObject, protected Output
	{
		Q_OBJECT

	protected:
		explicit CommandProcessor(DeviceEmulator* device);

	public:
		virtual ~CommandProcessor();
		static CommandProcessor* createInstance(DeviceEmulator* device);

	public:
		virtual QString logicModuleName() const;

		// Parse LM command, result written to deviceCommand, can throw SimException
		//
		bool parseFunc(QString parseFunc, DeviceCommand* command);

	protected:
		ScriptDeviceEmulator& device();
		const ScriptDeviceEmulator& device() const;

		// Check that such AFB does exist and it has pin  (optional)
		// Raise SimException if fail
		//
		AfbComponent checkAfb(int opCode, int instanceNo, int pinOpCode = -1) const;

		QString strCommand(QString command) const;
		QString strAfbInstPin(const DeviceCommand* command) const;
		QString strAddr(quint16 address) const;
		QString strWordConst(quint16 data) const;
		QString strDwordConst(quint32 data)const;

	private:
		static const std::map<QString, std::function<CommandProcessor*(DeviceEmulator*)>> m_lmToFactory;

		ScriptDeviceEmulator m_device;
	};
}

#endif // SIMCOMMANDPROCESSOR_H
