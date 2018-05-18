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
		// Parse LM command, result written to deviceCommand, can throw SimException
		//
		bool parseFunc(QString parseFunc, DeviceCommand* command);

		// Run simulation LM command, can throw SimException
		//
		virtual bool runCommand(const DeviceCommand& command);

	protected:
		inline ScriptDeviceEmulator& device()
		{
			return m_device;
		}

		inline const ScriptDeviceEmulator& device() const
		{
			return m_device;
		}

		// Check that such AFB does exist and it has pin  (optional)
		// Raise SimException if fail
		//
		AfbComponent checkAfb(int opCode, int instanceNo, int pinOpCode = -1) const;

		// Check if data in specific range
		// Raise SimException if fail
		//
		void checkParamRange(int paramValue, int minValue, int maxValue, QString param) const;

		// Check if data in specific range
		// Raise SimException if fail
		//
		void checkParamExists(const AfbComponentInstance* afbInstance, int paramOpIndex, QString paramName = QString()) const;

		// Generation command string functions (helpers)
		//
		QString strCommand(QString command) const;
		QString strAfbInst(const DeviceCommand* command) const;
		QString strAfbInstPin(const DeviceCommand* command) const;

		QString strAddr(quint16 address) const;
		QString strBitAddr(quint16 address, quint16 bitNo) const;

		QString strBitConst(quint16 data) const;
		QString strWordConst(quint16 data) const;
		QString strDwordConst(quint32 data)const;

	private:
		static const std::map<QString, std::function<CommandProcessor*(DeviceEmulator*)>> m_lmToFactory;

		ScriptDeviceEmulator m_device;
	};
}

#endif // SIMCOMMANDPROCESSOR_H
