#pragma once

namespace Sim
{
	struct DeviceCommand
	{
		int m_offset = 0;           // Offset in Code Memory, words
		int m_size = 0;             // Command size in words. Set in parse script
		QString m_string;           // Set in parse script

		quint16 m_afbOpCode = 0;    // Set in parse script
		quint16 m_afbInstance = 0;  // Set in parse script
		quint16 m_afbPinOpCode = 0; // Set in parse script

		quint16 m_bitNo0 = 0;       // Set in parse script
		quint16 m_bitNo1 = 0;       // Set in parse script

		quint16 m_word0 = 0;        // Set in parse script
		quint16 m_word1 = 0;        // Set in parse script
		quint16 m_word2 = 0;        // Set in parse script

		quint32 m_dword0 = 0;       // Set in parse script
		quint32 m_dword1 = 0;       // Set in parse script
	};
} // namespace Sim