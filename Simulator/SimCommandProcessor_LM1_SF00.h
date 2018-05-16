#ifndef SIMCOMMANDPROCESSOR_LM1_SF00_H
#define SIMCOMMANDPROCESSOR_LM1_SF00_H
#include <SimCommandProcessor.h>
#include <SimDeviceEmulator.h>

namespace Sim
{

	class CommandProcessor_LM1_SF00 : public CommandProcessor
	{
		Q_OBJECT

	public:
		explicit CommandProcessor_LM1_SF00(DeviceEmulator* device);
		virtual ~CommandProcessor_LM1_SF00();

	public:
		virtual QString logicModuleName() const override;

	public slots:

		// Command: wrfbc
		// Code: 10
		// Description: Write constant word to AFB input
		//
		bool parse_wrfbc(DeviceCommand* command) const;
		bool command_wrfbc(const DeviceCommand& command);

		// Command: appstart
		// Code: 17
		// Description: Save ALP phase start address
		//
		bool parse_appstart(DeviceCommand* command) const;
		bool command_appstart(const DeviceCommand& command);

	private:
		inline static const QString m_logicModuleName{"LM-SF00"};
	};

}

#endif // SIMCOMMANDPROCESSOR_LM_SF00_H
