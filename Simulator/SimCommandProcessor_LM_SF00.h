#ifndef SIMCOMMANDPROCESSOR_LM_SF00_H
#define SIMCOMMANDPROCESSOR_LM_SF00_H
#include <SimCommandProcessor.h>

namespace Sim
{

	class CommandProcessor_LM_SF00 : public CommandProcessor
	{
	public:
		CommandProcessor_LM_SF00();
		virtual ~CommandProcessor_LM_SF00();

	public:
		virtual QString logicModuleName() const override;

	private:
		inline static const QString m_logicModuleName{"LM-SF00"};
	};

}

#endif // SIMCOMMANDPROCESSOR_LM_SF00_H
