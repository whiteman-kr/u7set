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

		qint64 recordIndex;

		static qint64 s_indexCounter;

		// Create Recrd functions
		//
		static TuningRecord createApplyChanges(const QString& lmEquipmentId, const QString& portEquipmentId);

		static TuningRecord createDword(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 offsetW,
										quint32 value,
										quint32 mask);

		static TuningRecord createSignedInt32(const QString& lmEquipmentId,
											  const QString& portEquipmentId,
											  quint32 offsetW,
											  qint32 value);

		static TuningRecord createFloat(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 offsetW,
										float value);

		// Write this record to RAM, just helper
		//
		bool writeToRam(Sim::Ram& ram) const;
	};

}
