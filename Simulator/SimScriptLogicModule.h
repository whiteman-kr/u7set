#pragma once
#include "SimLogicModule.h"
#include "SimScriptRamAddress.h"

namespace Sim
{

	class ScriptLogicModule : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString equipmentID READ equipmentId)

		Q_PROPERTY(quint32 regBufferStartAddress READ regBufferStartAddress)
		Q_PROPERTY(quint32 regBufferSize READ regBufferSize)

		Q_PROPERTY(bool powerOff READ isPowerOff WRITE setPowerOff)

	public:
		ScriptLogicModule() = default;
		ScriptLogicModule(const ScriptLogicModule& src);
		explicit ScriptLogicModule(std::shared_ptr<LogicModule> logicModule);

		ScriptLogicModule& operator=(const ScriptLogicModule& src);

	public:
		bool isNull() const;

		QString equipmentId() const;

		quint32 regBufferStartAddress() const;
		quint32 regBufferSize() const;

		bool isPowerOff() const;
		void setPowerOff(bool value);

		// Ram Access
		//
	public slots:
		quint16 readRamBit(RamAddress address, E::LogicModuleRamAccess access);
		quint16 readRamWord(RamAddress address, E::LogicModuleRamAccess access);
		quint32 readRamDword(RamAddress address, E::LogicModuleRamAccess access);
		qint32 readRamSignedInt(RamAddress address, E::LogicModuleRamAccess access);
		float readRamFloat(RamAddress address, E::LogicModuleRamAccess access);

		void writeRamBit(RamAddress address, quint16 value, E::LogicModuleRamAccess access);
		void writeRamWord(RamAddress address, quint16 value, E::LogicModuleRamAccess access);
		void writeRamDword(RamAddress address, quint32 value, E::LogicModuleRamAccess access);
		void writeRamSignedInt(RamAddress address, qint32 value, E::LogicModuleRamAccess access);
		void writeRamFloat(RamAddress address, float value, E::LogicModuleRamAccess access);

	private:
		QString rwError(const QString& function, const RamAddress& address, E::LogicModuleRamAccess access) const;

	private:
		std::shared_ptr<LogicModule> m_logicModule;
	};


}

Q_DECLARE_METATYPE(Sim::ScriptLogicModule);
