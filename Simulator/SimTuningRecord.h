#pragma once
#include <variant>
#include "SimRam.h"

namespace Sim
{

	struct TuningRecord
	{
		QString lmEquipmentId;
		QString portEquipmentId;

		quint32 offsetW;
		std::variant<quint32, qint32, float> value;
		quint32 dwordMask;

		static const size_t ValueDwordIndex = 0;		// Index in std::variant<quint32, qint32, float> value
		static const size_t ValueSignedInt32Index = 1;
		static const size_t ValueFloatIndex = 2;

		static TuningRecord createDword(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 offsetW,
										quint32 value,
										quint32 mask)
		{
			return TuningRecord{lmEquipmentId, portEquipmentId, offsetW, value, ~mask};		// Mask is flipped here, to be classical mask (1s is what to leave as is)
		}

		static TuningRecord createSignedInt32(const QString& lmEquipmentId,
											  const QString& portEquipmentId,
											  quint32 offsetW,
											  qint32 value)
		{
			return TuningRecord{lmEquipmentId, portEquipmentId, offsetW, value, 0xFFFFFFFF};
		}

		static TuningRecord createFloat(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 offsetW,
										float value)
		{
			return TuningRecord{lmEquipmentId, portEquipmentId, offsetW, value, 0xFFFFFFFF};
		}

		bool writeToRam(Sim::Ram& ram)
		{
			bool ok = true;

			switch (value.index())
			{
			case TuningRecord::ValueDwordIndex:
				{
					quint32 v = std::get<quint32>(value);

					quint32 ramDword = 0;
					ok &= ram.readDword(offsetW, &ramDword, E::ByteOrder::BigEndian);

					ramDword &= dwordMask;
					ramDword |= v;

					ok &= ram.writeDword(offsetW, ramDword, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
				}
				break;
			case TuningRecord::ValueSignedInt32Index:
				{
					qint32 v = std::get<qint32>(value);
					ok &= ram.writeSignedInt(offsetW, v, E::ByteOrder::BigEndian, E::LogicModuleRamAccess::Read);
				}
				break;
			case TuningRecord::ValueFloatIndex:
				{
					float v = std::get<float>(value);
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
