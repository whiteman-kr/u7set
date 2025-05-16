#pragma once
#include "SimLogicModuleImpl.h"
#include "SimScriptRamAddress.h"

namespace Sim
{

	/*! \class ScriptLogicModule
		\ingroup simulator
		\brief Represents class that represents a logic module. It allows to control it's power and tuning state.
	*/
	class ScriptLogicModule : public QObject
	{
		Q_OBJECT

		/// \brief Module EquipmentID.
		Q_PROPERTY(QString equipmentID READ equipmentId)

		/// \brief Switches module power on and off.
		Q_PROPERTY(bool powerOff READ isPowerOff WRITE setPowerOff)

		/// \brief Switches module Arming Key on and off.
		Q_PROPERTY(bool armingKey READ armingKey WRITE setArmingKey)

		/// \brief Switches module Tuning Key on and off.
		Q_PROPERTY(bool tuningKey READ tuningKey WRITE setTuningKey)

		Q_PROPERTY(quint32 regBufferStartAddress READ regBufferStartAddress)
		Q_PROPERTY(quint32 regBufferSize READ regBufferSize)

	public:
		ScriptLogicModule() = default;
		ScriptLogicModule(const ScriptLogicModule& src);
		explicit ScriptLogicModule(std::shared_ptr<LogicModuleImpl> logicModule);

		ScriptLogicModule& operator=(const ScriptLogicModule& src);

	public:
		bool isNull() const;

		QString equipmentId() const;

		bool isPowerOff() const;
		void setPowerOff(bool value);

		bool armingKey() const;
		void setArmingKey(bool value);

		bool tuningKey() const;
		void setTuningKey(bool value);

		quint32 regBufferStartAddress() const;
		quint32 regBufferSize() const;

		// Ram Access
		//
	public slots:
		quint16 readRamBit(RamAddress address, E::LogicModuleRamAccess access);
		quint16 readRamWord(RamAddress address, E::LogicModuleRamAccess access);
		quint16 readRamUnsignedInt16(RamAddress address, E::LogicModuleRamAccess access);
		qint16 readRamSignedInt16(RamAddress address, E::LogicModuleRamAccess access);
		quint32 readRamDword(RamAddress address, E::LogicModuleRamAccess access);
		qint32 readRamSignedInt(RamAddress address, E::LogicModuleRamAccess access);
		qint32 readRamSignedInt32(RamAddress address, E::LogicModuleRamAccess access);
		float readRamFloat(RamAddress address, E::LogicModuleRamAccess access);

		void writeRamBit(RamAddress address, quint16 value, E::LogicModuleRamAccess access);
		void writeRamWord(RamAddress address, quint16 value, E::LogicModuleRamAccess access);
		void writeRamDword(RamAddress address, quint32 value, E::LogicModuleRamAccess access);
		void writeRamSignedInt(RamAddress address, qint32 value, E::LogicModuleRamAccess access);
		void writeRamFloat(RamAddress address, float value, E::LogicModuleRamAccess access);

	private:
		QString rwError(const QString& function, const RamAddress& address, E::LogicModuleRamAccess access) const;

	private:
		std::shared_ptr<LogicModuleImpl> m_logicModule;
	};


}

Q_DECLARE_METATYPE(Sim::ScriptLogicModule);
