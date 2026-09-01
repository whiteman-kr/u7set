#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace SimVduSharedData
{
	constexpr size_t OptoBufferMaxSize = 2560 * 2;
	constexpr size_t OptoPortCount = 8;

	constexpr size_t DiscreteInputCount = 6;
	constexpr size_t DiscreteOutputCount = 6;

	struct SimulatorState
	{
		uint64_t updateCounter; // The update counter, incremented by simulator on each update
		int32_t deviceState;    // enum class DeviceState (0 - Off, Start, Fault, Operate)
		int32_t reserved0;      // Reserved for future use (padding to 8 bytes)
		uint64_t plantTime;     // The plant time in microseconds
	};

	struct OptoBuffer
	{
		uint64_t timeout;       //
		uint64_t occupied;      // The number of bytes occupied in the buffer
		uint8_t data[OptoBufferMaxSize];
	};

	struct OptoPorts
	{
		uint64_t updateCounter;
		OptoBuffer portData[OptoPortCount]; // Data from simulator to VDU
	};

	struct DiscreteInputs
	{
		uint64_t updateCounter;
		uint8_t data[DiscreteInputCount];
		uint8_t validity[DiscreteInputCount];
	};

	struct DiscreteOutputs
	{
		uint64_t updateCounter;
		uint8_t data[DiscreteOutputCount];
	};

	struct SharedData
	{
		uint64_t size;                   // The size of this structure, must be filled first

		SimulatorState simulatorState;   // State of the simulator

		OptoPorts toVdu;                 // Data from simulator to VDU
		OptoPorts fromVdu;               // Data from VDU to simulator

		DiscreteInputs discreteInputs;   // Discrete inputs, simulator to VDU
		DiscreteOutputs discreteOutputs; // Discrete outputs, VDU to simulator
	};

	static_assert(std::is_standard_layout_v<SharedData> && std::is_trivial_v<SharedData>, "SharedData must be POD type");
} // namespace SimVduSharedData