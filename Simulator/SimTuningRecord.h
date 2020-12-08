#pragma once
#include <variant>
#include "SimRam.h"

namespace Sim
{

	struct TuningRecord
	{
		enum class RecordType
		{
			ApplyChanges,
			WriteDword,
			WriteSignedInt32,
			WriteFloat
		};

		RecordType type;
		QString lmEquipmentId;
		QString portEquipmentId;

		quint32 offsetW;
		std::variant<quint32, qint32, float> value;
		quint32 dwordMask;

		static TuningRecord createApplyChanges(const QString& lmEquipmentId,
											   const QString& portEquipmentId)
		{
			return TuningRecord{RecordType::ApplyChanges, lmEquipmentId, portEquipmentId, 0, 0, 0};
		}

		static TuningRecord createDword(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 offsetW,
										quint32 value,
										quint32 mask)
		{
			return TuningRecord{RecordType::WriteDword, lmEquipmentId, portEquipmentId, offsetW, value, ~mask};		// Mask is flipped here, to be classical mask (1s is what to leave as is)
		}

		static TuningRecord createSignedInt32(const QString& lmEquipmentId,
											  const QString& portEquipmentId,
											  quint32 offsetW,
											  qint32 value)
		{
			return TuningRecord{RecordType::WriteSignedInt32, lmEquipmentId, portEquipmentId, offsetW, value, 0xFFFFFFFF};
		}

		static TuningRecord createFloat(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 offsetW,
										float value)
		{
			return TuningRecord{RecordType::WriteFloat, lmEquipmentId, portEquipmentId, offsetW, value, 0xFFFFFFFF};
		}

		bool writeToRam(Sim::Ram& ram) const
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

	};

}
