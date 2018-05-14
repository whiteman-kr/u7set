#ifndef SIMCOMMANDPROCESSOR_H
#define SIMCOMMANDPROCESSOR_H
#include <map>
#include <functional>
#include "QtCore"
#include "SimOutput.h"

namespace Sim
{
	class CommandProcessor : protected Output
	{
	protected:
		CommandProcessor();

	public:
		virtual ~CommandProcessor();
		static CommandProcessor* createInstance(QString logicModuleName, QString equipmentId);

	public:
		virtual QString logicModuleName() const = 0;

	private:
		static const std::map<QString, std::function<CommandProcessor*()>> m_lmToFactory;
	};

}

#endif // SIMCOMMANDPROCESSOR_H
