#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <map>
#include "Output.h"
#include "Subsystem.h"

class QTextStream;

namespace Sim
{
	class Simulator : protected Output
	{
	public:
		explicit Simulator(QTextStream* outputStream);
		virtual ~Simulator();

	public:
		bool load(QString buildPath);

	private:
		std::map<QString, Subsystem> m_subsystems;		// Key is subsystemId;
	};

}

#endif // SIMULATOR_H
