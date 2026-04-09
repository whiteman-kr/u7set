#include "AdsBridgeResources.h"

#include <mutex>

namespace AdsBridge
{
	const char* Resources::getString(const std::string& string) const
	{
		// Shared lock for reading. If the string is found, return the pointer.
		//
		{
			std::shared_lock lock(m_stringTableMutex);

			auto it = m_stringTable.find(string);
			if (it != m_stringTable.end())
			{
				return it->c_str();
			}
		}

		// Th string was not found, insert new string with unique lock
		//
		std::unique_lock lock{m_stringTableMutex};
		auto r = m_stringTable.insert(string);
		return r.first->c_str();
	}

	const char* Resources::getString(std::string&& string) const
	{
		// Shared lock for reading. If the string is found, return the pointer.
		//
		{
			std::shared_lock lock(m_stringTableMutex);

			auto it = m_stringTable.find(string);
			if (it != m_stringTable.end())
			{
				return it->c_str();
			}
		}

		// Th string was not found, insert new string with unique lock
		//
		std::unique_lock lock{m_stringTableMutex};
		auto r = m_stringTable.insert(std::move(string));
		return r.first->c_str();
	}
} // namespace AdsBridge