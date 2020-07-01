#include "SimScriptLogicModule.h"
#include "SimScriptSimulator.h"

namespace Sim
{
	ScriptLogicModule::ScriptLogicModule(const ScriptLogicModule& src) :
		m_logicModule(src.m_logicModule)
	{
	}

	ScriptLogicModule::ScriptLogicModule(std::shared_ptr<LogicModule> logicModule) :
		m_logicModule(logicModule)
	{
	}

	ScriptLogicModule& ScriptLogicModule::operator=(const ScriptLogicModule& src)
	{
		m_logicModule = src.m_logicModule;
		return *this;
	}

	bool ScriptLogicModule::isNull() const
	{
		return m_logicModule == nullptr;
	}

	QString ScriptLogicModule::equipmentId() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("ScriptLogicModule is null"));
			return {};
		}

		return m_logicModule->equipmentId();
	}

	quint32 ScriptLogicModule::regBufferStartAddress() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("ScriptLogicModule is null"));
			return {};
		}

		return m_logicModule->lmDescription().memory().m_appLogicWordDataOffset;
	}

	quint32 ScriptLogicModule::regBufferSize() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("ScriptLogicModule is null"));
			return {};
		}

		return m_logicModule->logicModuleExtraInfo().appDataSizeBytes / 2;
	}

	bool ScriptLogicModule::isPowerOff() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("ScriptLogicModule is null"));
			return {};
		}

		return m_logicModule->isPowerOff();
	}

	void ScriptLogicModule::setPowerOff(bool value)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("ScriptLogicModule is null"));
			return;
		}

		return m_logicModule->setPowerOff(value);
	}

	quint16 ScriptLogicModule::readRamBit(RamAddress address, E::LogicModuleRamAccess access)
	{
		quint16 result = {};

		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamBit error, address %1").arg(address.toString()));
			return std::numeric_limits<decltype(result)>::max();
		}

		bool ok = m_logicModule->ram().readBit(address.offset(), address.bit(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamBit error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	quint16 ScriptLogicModule::readRamWord(RamAddress address, E::LogicModuleRamAccess access)
	{
		quint16 result = {};

		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamWord error, address %1").arg(address.toString()));
			return std::numeric_limits<decltype(result)>::max();
		}

		bool ok = m_logicModule->ram().readWord(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamWord error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	quint32 ScriptLogicModule::readRamDword(RamAddress address, E::LogicModuleRamAccess access)
	{
		quint32 result = {};

		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamDword error, address %1").arg(address.toString()));
			return std::numeric_limits<decltype(result)>::max();
		}

		bool ok = m_logicModule->ram().readDword(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamDword error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	qint32 ScriptLogicModule::readRamSignedInt(RamAddress address, E::LogicModuleRamAccess access)
	{
		qint32 result = {};

		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamSignedInt error, address %1").arg(address.toString()));
			return std::numeric_limits<decltype(result)>::max();
		}

		bool ok = m_logicModule->ram().readSignedInt(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamSignedInt error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}

	float ScriptLogicModule::readRamFloat(RamAddress address, E::LogicModuleRamAccess access)
	{
		float result = {};

		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamFloat error, address %1").arg(address.toString()));
			return std::numeric_limits<decltype(result)>::quiet_NaN();
		}

		bool ok = m_logicModule->ram().readFloat(address.offset(), &result, E::ByteOrder::BigEndian, access, true);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("readRamFloat error, address %1").arg(address.toString()));
			return {};
		}

		return result;
	}


	void ScriptLogicModule::writeRamBit(RamAddress address, quint16 value, E::LogicModuleRamAccess access)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamBit error, address %1").arg(address.toString()));
			return;
		}

		bool ok = m_logicModule->mutableRam().writeBit(address.offset(), address.bit(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamBit error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptLogicModule::writeRamWord(RamAddress address, quint16 value, E::LogicModuleRamAccess access)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamWord error, address %1").arg(address.toString()));
			return;
		}

		bool ok = m_logicModule->mutableRam().writeWord(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamWord error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptLogicModule::writeRamDword(RamAddress address, quint32 value, E::LogicModuleRamAccess access)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamDword error, address %1").arg(address.toString()));
			return;
		}

		bool ok = m_logicModule->mutableRam().writeDword(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamDword error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptLogicModule::writeRamSignedInt(RamAddress address, qint32 value, E::LogicModuleRamAccess access)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamSignedInt error, address %1").arg(address.toString()));
			return;
		}

		bool ok = m_logicModule->mutableRam().writeSignedInt(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamSignedInt error, address %1").arg(address.toString()));
		}

		return;
	}

	void ScriptLogicModule::writeRamFloat(RamAddress address, float value, E::LogicModuleRamAccess access)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamFloat error, address %1").arg(address.toString()));
			return;
		}

		bool ok = m_logicModule->mutableRam().writeFloat(address.offset(), value, E::ByteOrder::BigEndian, access);
		if (ok == false)
		{
			ScriptSimulator::throwScriptException(this, tr("writeRamFloat error, address %1").arg(address.toString()));
		}

		return;
	}

}
