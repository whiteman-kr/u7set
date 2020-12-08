#include "SimTuningRecord.h"

namespace Sim
{
	qint64 TuningRecord::s_indexCounter = 0;

	TuningRecord TuningRecord::createApplyChanges(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		return TuningRecord{RecordType::ApplyChanges,
					lmEquipmentId,
					portEquipmentId,
					0,
					0,
					0,
					++s_indexCounter};
	}

	TuningRecord TuningRecord::createDword(const QString& lmEquipmentId,
										   const QString& portEquipmentId,
										   quint32 offsetW,
										   quint32 value,
										   quint32 mask)
	{
		return TuningRecord{RecordType::WriteDword,
					lmEquipmentId,
					portEquipmentId,
					offsetW,
					value,
					~mask,				// Mask is flipped here, to be classical mask (1s is what to leave as is)
					++s_indexCounter};

	}

	TuningRecord TuningRecord::createSignedInt32(const QString& lmEquipmentId,
												 const QString& portEquipmentId,
												 quint32 offsetW,
												 qint32 value)
	{
		return TuningRecord{RecordType::WriteSignedInt32,
					lmEquipmentId,
					portEquipmentId,
					offsetW,
					value,
					0xFFFFFFFF,
					++s_indexCounter};
	}

	TuningRecord TuningRecord::createFloat(const QString& lmEquipmentId,
										   const QString& portEquipmentId,
										   quint32 offsetW,
										   float value)
	{
		return TuningRecord{RecordType::WriteFloat,
					lmEquipmentId,
					portEquipmentId,
					offsetW,
					value,
					0xFFFFFFFF,
					++s_indexCounter};
	}

	bool TuningRecord::writeToRam(Sim::Ram& ram) const
	{
		bool ok = true;

		switch (type)
		{
		case TuningRecord::RecordType::WriteDword:
			{
				quint32 v = 0;

				try
				{
					v = std::get<quint32>(value);
				}
				catch (const std::bad_variant_access&)
				{
					Q_ASSERT(false);
					ok = false;
				}

				quint32 ramDword = 0;
				ok &= ram.readDword(offsetW, &ramDword, E::ByteOrder::BigEndian);

				ramDword &= dwordMask;
				ramDword |= v;

				ok &= ram.writeDword(offsetW, ramDword, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
			}
			break;
		case TuningRecord::RecordType::WriteSignedInt32:
			{
				qint32 v = 0;

				try
				{
					v = std::get<qint32>(value);
				}
				catch (const std::bad_variant_access&)
				{
					Q_ASSERT(false);
					ok = false;
				}

				ok &= ram.writeSignedInt(offsetW, v, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
			}
			break;
		case TuningRecord::RecordType::WriteFloat:
			{
				float v = 0;

				try
				{
					v = std::get<float>(value);
				}
				catch (const std::bad_variant_access&)
				{
					Q_ASSERT(false);
					ok = false;
				}

				ok &= ram.writeFloat(offsetW, v, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
			}
			break;
		default:
			Q_ASSERT(false);
			ok = false;
		}

		return ok;
	}
}
