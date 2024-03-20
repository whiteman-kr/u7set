#pragma once

namespace ClientLib
{
	struct TuningWriteCommand
	{
		enum class TuningWriteCommandType
		{
			WriteValue,
			Apply,
			ActivateLm
		};

		// Data

		Hash appSignalHash = 0;
		Hash equipmentHash = 0; // Used only for activation/deactivation LM
		TuningValue value;

		TuningWriteCommandType type = TuningWriteCommandType::WriteValue;

		bool enableControl = false;
		bool forceTakeControl = false;

		// Write constructor
		//
		TuningWriteCommand(const QString& appSignalId, const TuningValue& value) :
			TuningWriteCommand(::calcHash(appSignalId), value)
		{
		}

		TuningWriteCommand(Hash appSignalHash, const TuningValue& value)
		{
			type = TuningWriteCommandType::WriteValue;
			this->appSignalHash = appSignalHash;
			this->value = value;
		}

		// Apply constructor
		//
		TuningWriteCommand(bool apply)
		{
			Q_UNUSED(apply);
			this->type = TuningWriteCommandType::Apply;
		}

		// Activate LM constructor
		//
		TuningWriteCommand(Hash equipmentHash, bool enableControl, bool forceTakeControl)
		{
			type = TuningWriteCommandType::ActivateLm;
			this->equipmentHash = equipmentHash;
			this->enableControl = enableControl;
			this->forceTakeControl = forceTakeControl;
		}

		// Serializing
		//
		bool toProtoWriteCommand(Network::TuningWriteCommand* message) const;
	};
} // namespace ClientLib
